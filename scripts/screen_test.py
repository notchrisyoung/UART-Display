#!/usr/bin/env python3
"""
Screen tester for NERD (Networked E-paper Remote Display).

Connects to the display over serial and lets you drive every screen by hand:
pick a screen from the menu, choose its contents, and watch what the display
sends back (acknowledgements, menu selections and any debug text).

    pip install pyserial
    python screen_test.py                 # pick the port from a list
    python screen_test.py --port COM17    # or give it directly
    python screen_test.py --port /dev/ttyUSB0 --demo   # run every screen once, then exit

The protocol is described in the README: each frame is
0x02 | length (2 bytes, big-endian) | JSON | CRC-8 (poly 0x9B) | 0xFF,
and the display answers every frame with 0x10 (OK) or 0x11 (error).
"""

import argparse
import json
import queue
import sys
import threading
import time

try:
    import serial
    from serial.tools import list_ports
except ImportError:
    sys.exit("pyserial is not installed. Run: pip install pyserial")

BAUDRATE = 115200
START_BYTE = 0x02
END_BYTE = 0xFF
MESSAGE_OK = 0x10
MESSAGE_ERR = 0x11
ACK_TIMEOUT = 2.0  # seconds to wait for the display to acknowledge a frame

MAIN_MENU_ITEMS = ["Selection Screen", "Loading Screen", "Info Screen", "Splash Screen"]


# --------------------------------------------------------------------------- protocol

def _crc_table(poly=0x9B):
    table = []
    for i in range(256):
        curr = i
        for _ in range(8):
            curr = ((curr << 1) ^ poly) & 0xFF if curr & 0x80 else (curr << 1) & 0xFF
        table.append(curr)
    return table


CRC_TABLE = _crc_table()


def crc8(data: bytes) -> int:
    crc = 0
    for b in data:
        crc = CRC_TABLE[crc ^ b]
    return crc


def build_frame(payload: bytes, corrupt: bool = False) -> bytes:
    """Wrap a payload in the display's framing. corrupt=True sends a wrong CRC on purpose."""
    if len(payload) > 0xFFFF:
        raise ValueError("payload too long for a 2-byte length field")
    checksum = crc8(payload) ^ (0xFF if corrupt else 0x00)
    return bytes([START_BYTE]) + len(payload).to_bytes(2, "big") + payload + bytes([checksum, END_BYTE])


class FrameReader:
    """Splits the display's output into acks, framed JSON messages and plain text.

    The display writes single ack bytes, framed replies and the odd Serial.println()
    to the same port, so this is a small state machine fed one byte at a time.
    """

    def __init__(self):
        self.state = "idle"
        self.buf = bytearray()
        self.length = 0
        self.text = bytearray()

    def feed(self, byte: int):
        """Returns a list of events: ("ack", ok), ("message", dict|str), ("text", str)."""
        events = []
        if self.state == "idle":
            if byte == START_BYTE:
                events += self._flush_text()
                self.state, self.buf = "length", bytearray()
            elif byte in (MESSAGE_OK, MESSAGE_ERR):
                events += self._flush_text()
                events.append(("ack", byte == MESSAGE_OK))
            elif byte in (0x0A, 0x0D):
                events += self._flush_text()
            else:
                self.text.append(byte)
        elif self.state == "length":
            self.buf.append(byte)
            if len(self.buf) == 2:
                self.length = int.from_bytes(self.buf, "big")
                self.buf = bytearray()
                self.state = "data" if self.length else "crc"
        elif self.state == "data":
            self.buf.append(byte)
            if len(self.buf) >= self.length:
                self.state = "crc"
        elif self.state == "crc":
            self.crc = byte
            self.state = "end"
        elif self.state == "end":
            self.state = "idle"
            if byte != END_BYTE or self.crc != crc8(self.buf):
                events.append(("bad_frame", bytes(self.buf)))
            else:
                raw = self.buf.decode("utf-8", errors="replace")
                try:
                    events.append(("message", json.loads(raw)))
                except json.JSONDecodeError:
                    events.append(("message", raw))
        return events

    def _flush_text(self):
        if not self.text:
            return []
        line = self.text.decode("utf-8", errors="replace").strip()
        self.text = bytearray()
        return [("text", line)] if line else []


# --------------------------------------------------------------------------- connection

class Display:
    def __init__(self, port: str, baud: int = BAUDRATE, verbose: bool = False):
        self.ser = serial.Serial(port, baud, timeout=0.1)
        self.verbose = verbose
        self.acks = queue.Queue()
        self.messages = queue.Queue()
        self.on_message = None
        self._stop = threading.Event()
        self._reader = threading.Thread(target=self._read_loop, daemon=True)
        self._reader.start()

    def close(self):
        self._stop.set()
        self._reader.join(timeout=1)
        self.ser.close()

    def _read_loop(self):
        parser = FrameReader()
        while not self._stop.is_set():
            try:
                chunk = self.ser.read(max(1, self.ser.in_waiting))
            except serial.SerialException as e:
                print(f"\n[serial error] {e}")
                return
            for b in chunk:
                for kind, value in parser.feed(b):
                    if kind == "ack":
                        self.acks.put(value)
                    elif kind == "message":
                        print(f"\n<-- display: {json.dumps(value) if isinstance(value, dict) else value}")
                        self.messages.put(value)
                        if self.on_message:
                            self.on_message(value)
                    elif kind == "bad_frame":
                        print(f"\n<-- bad frame from display (CRC or end byte wrong): {value!r}")
                    elif kind == "text":
                        print(f"\n<-- display log: {value}")

    def send(self, message: dict, corrupt: bool = False) -> bool:
        payload = json.dumps(message, separators=(",", ":")).encode("utf-8")
        frame = build_frame(payload, corrupt=corrupt)
        while not self.acks.empty():
            self.acks.get_nowait()
        print(f"--> {payload.decode()[:120]}{'...' if len(payload) > 120 else ''}  ({len(payload)} bytes)")
        if self.verbose:
            print("    frame:", " ".join(f"{b:02X}" for b in frame[:48]), "..." if len(frame) > 48 else "")
        start = time.time()
        self.ser.write(frame)
        self.ser.flush()
        try:
            ok = self.acks.get(timeout=ACK_TIMEOUT + len(frame) / (BAUDRATE / 10))
        except queue.Empty:
            print("    no acknowledgement (is the display running and the port right?)")
            return False
        print(f"    {'OK' if ok else 'ERROR'} from display in {(time.time() - start) * 1000:.0f} ms")
        return ok


# --------------------------------------------------------------------------- screens

def splash():
    return {"type": "screen", "id": "splash"}


def main_menu(title="Main Menu", items=None):
    return {"type": "screen", "id": "main_menu", "menuTitle": title, "menuItems": items or MAIN_MENU_ITEMS}


def selection(coordinators=6, routers=20, title="ZigBee scan"):
    items = [f"Coordinator {i}" for i in range(1, coordinators + 1)]
    items += [f"Router {i}" for i in range(1, routers + 1)]
    return {"type": "screen", "id": "selection_menu", "menuTitle": title, "menuItems": items}


def info_screen():
    return {"type": "screen", "id": "info_screen", "title": "System Info",
            "infoItems": ["Firmware: test", "Hardware: ESP32", "Display: e-paper", "Status: Running"]}


def loading_screen():
    return {"type": "screen", "id": "loading_screen", "title": "Loading", "message": "Please wait..."}


SCREENS_BY_MENU_ITEM = {
    "Selection Screen": lambda: selection(),
    "Loading Screen": loading_screen,
    "Info Screen": info_screen,
    "Splash Screen": splash,
}


# --------------------------------------------------------------------------- UI

def ask(prompt, default):
    value = input(f"{prompt} [{default}]: ").strip()
    return value or str(default)


def ask_int(prompt, default, lo=0, hi=10000):
    while True:
        try:
            n = int(ask(prompt, default))
            if lo <= n <= hi:
                return n
        except ValueError:
            pass
        print(f"  enter a number from {lo} to {hi}")


MENU = """
NERD screen tester
  1  Splash screen
  2  Main menu (default items)
  3  Main menu (your own title and items)
  4  Selection list: coordinators and routers
  5  Selection list: stress test (choose how many items)
  6  Info screen
  7  Loading screen
  8  Send your own JSON
  9  Send a frame with a bad checksum (display should answer ERROR)
  d  Demo: show every screen in turn
  f  Toggle follow mode (open the screen you pick on the display's main menu): {follow}
  q  Quit
"""


def demo(display: Display, pause: float):
    steps = [("Splash", splash()), ("Main menu", main_menu()), ("Selection list", selection()),
             ("Info", info_screen()), ("Loading", loading_screen()), ("Back to main menu", main_menu())]
    passed = 0
    for name, msg in steps:
        print(f"\n== {name}")
        passed += display.send(msg)
        time.sleep(pause)
    print("\n== Bad checksum (should be rejected)")
    rejected = not display.send(main_menu(), corrupt=True)
    print(f"\nDemo done: {passed}/{len(steps)} screens acknowledged, "
          f"bad frame {'rejected as expected' if rejected else 'was NOT rejected'}.")
    return passed == len(steps) and rejected


def interactive(display: Display):
    follow = {"on": True}

    def on_message(msg):
        if follow["on"] and isinstance(msg, dict) and msg.get("type") == "item_selected":
            builder = SCREENS_BY_MENU_ITEM.get(msg.get("selectedItem"))
            if builder:
                print("    (follow mode) opening that screen")
                # Send from another thread: this callback runs on the reader thread,
                # which has to stay free to receive the acknowledgement.
                threading.Thread(target=display.send, args=(builder(),), daemon=True).start()

    display.on_message = on_message
    while True:
        print(MENU.format(follow="on" if follow["on"] else "off"))
        choice = input("> ").strip().lower()
        if choice == "1":
            display.send(splash())
        elif choice == "2":
            display.send(main_menu())
        elif choice == "3":
            title = ask("Title", "Main Menu")
            items = [s.strip() for s in ask("Items, comma separated", ", ".join(MAIN_MENU_ITEMS)).split(",") if s.strip()]
            display.send(main_menu(title, items))
        elif choice == "4":
            display.send(selection(ask_int("Coordinators", 6), ask_int("Routers", 20)))
        elif choice == "5":
            n = ask_int("Number of items", 174, 1, 2000)
            display.send(selection(coordinators=0, routers=n, title=f"{n} items"))
        elif choice == "6":
            display.send(info_screen())
        elif choice == "7":
            display.send(loading_screen())
        elif choice == "8":
            raw = input("JSON: ").strip()
            try:
                display.send(json.loads(raw))
            except json.JSONDecodeError as e:
                print(f"  not valid JSON: {e}")
        elif choice == "9":
            display.send(main_menu(), corrupt=True)
        elif choice == "d":
            demo(display, pause=3)
        elif choice == "f":
            follow["on"] = not follow["on"]
        elif choice in ("q", "quit", "exit"):
            return
        else:
            print("  unknown choice")


def pick_port():
    ports = list(list_ports.comports())
    if not ports:
        sys.exit("No serial ports found. Plug in the display, or pass --port.")
    if len(ports) == 1:
        print(f"Using {ports[0].device} ({ports[0].description})")
        return ports[0].device
    for i, p in enumerate(ports, 1):
        print(f"  {i}  {p.device}  {p.description}")
    return ports[ask_int("Port", 1, 1, len(ports)) - 1].device


def main():
    ap = argparse.ArgumentParser(description="Interactive screen tester for the NERD e-paper display.")
    ap.add_argument("--port", help="serial port, e.g. COM17 or /dev/ttyUSB0 (asks if omitted)")
    ap.add_argument("--baud", type=int, default=BAUDRATE)
    ap.add_argument("--demo", action="store_true", help="show every screen once, report the results and exit")
    ap.add_argument("--pause", type=float, default=3.0, help="seconds between screens in --demo (default 3)")
    ap.add_argument("--verbose", action="store_true", help="print the raw frame bytes")
    args = ap.parse_args()

    port = args.port or pick_port()
    try:
        display = Display(port, args.baud, verbose=args.verbose)
    except serial.SerialException as e:
        sys.exit(f"Could not open {port}: {e}\n(Close the PlatformIO serial monitor or any other program using it.)")

    print("Waiting for the display to start...")
    time.sleep(2)  # opening the port resets most ESP32 boards
    try:
        if args.demo:
            sys.exit(0 if demo(display, args.pause) else 1)
        interactive(display)
    except (KeyboardInterrupt, EOFError):
        print()
    finally:
        display.close()


if __name__ == "__main__":
    main()
