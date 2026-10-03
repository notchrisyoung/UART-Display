# UART Display

> **Work in progress.** The menu and selection screens work, the info and loading screens are still being built, and the protocol may change.

**NERD: Networked E-paper Remote Display.** A general-purpose **e-paper menu terminal**. A LilyGo T5 4.7" e-ink board with seven physical buttons shows menus, selection lists, info and loading screens. **Whatever is plugged into its serial port decides what's on screen.** The host sends small JSON messages over UART, and the display sends back what the user picked. This lets any microcontroller, Raspberry Pi or PC get a crisp, low-power UI without having to drive a display itself.

| The handheld | Splash image |
|---|---|
| ![The NERD handheld showing its splash screen](docs/photos/nerd-front.jpg) | ![Splash image](docs/splash.jpg) |

## Screens

| Screen `id` | What it shows |
|---|---|
| `splash` | Boot splash image (`SplashData.h`) |
| `main_menu` | Titled list of menu items; scroll with Up/Down, confirm with Select |
| `selection_menu` | Multi-column list for long lists (e.g. 170+ ZigBee coordinators and routers found by a scan), navigated with all four arrows |
| `info_screen` | Title plus lines of information *(work in progress)* |
| `loading_screen` | "Please wait" style screen *(work in progress)* |

## Protocol

**Framing** (both directions, 115200 baud):

```
0x02 | length (2 bytes, big-endian) | JSON payload | CRC-8 | 0xFF
```

- CRC-8 uses polynomial `0x9B`, computed over the payload only.
- For every frame it receives, the display replies with one byte: `0x10` (OK) or `0x11` (bad checksum / framing).

**Host → display** (switch screens and fill them):

```json
{"type": "screen", "id": "main_menu", "menuTitle": "Main Menu",
 "menuItems": ["Selection Screen", "Loading Screen", "Info Screen", "Splash Screen"]}
```

```json
{"type": "screen", "id": "selection_menu", "menuTitle": "Scanning for coordinators",
 "menuItems": ["Coordinator 1", "Coordinator 2", "Coordinator 3"]}
```

`{"type": "data", ...}` sends updates to the screen that's already showing.

**Display → host** (when the user presses Select):

```json
{"type": "item_selected", "selectedItem": "Info Screen"}
```

## Testing the screens

`scripts/screen_test.py` drives the display from a PC so every screen can be checked by hand. It's built into the PlatformIO project as two tasks, which use PlatformIO's own Python (it already has `pyserial`):

```bash
pio run -t screentest    # interactive tester
pio run -t screendemo    # show every screen once, then report pass/fail
```

In VS Code they're under **PlatformIO > Project Tasks > t5-47 > Custom**. They use the port set by `monitor_port` or `upload_port` in `platformio.ini` (or `--upload-port COM17` on the command line). With no port set, the tester lists the serial ports and asks which one to use.

The script also runs on its own with any Python 3 that has `pyserial`:

```bash
pip install pyserial
python scripts/screen_test.py                    # pick the port from a list
python scripts/screen_test.py --port COM17       # or name it
python scripts/screen_test.py --port COM17 --demo   # show every screen once, then exit
```

The interactive menu can:
- open each screen (splash, main menu, selection list, info, loading)
- fill the main menu with your own title and items
- fill the selection list with coordinators and routers, or with hundreds of items as a stress test
- send any JSON you type
- send a frame with a bad checksum, which the display should reject

Everything the display sends back is printed as it arrives: the OK/ERROR acknowledgement for each frame, the item picked with Select, and any debug text. In **follow mode** (on by default), picking an entry on the display's main menu opens that screen, so you can test from the buttons alone. `--demo` exits with code 0 only if every screen was acknowledged and the bad frame was rejected, so it also works as a quick check after flashing.

Close the PlatformIO serial monitor first: only one program can use the port at a time.

`scripts/simple_test.py` is the older, minimal version: set `SERIAL_PORT` at the top and run it to walk through the screens.

## The handheld build

| | | |
|---|---|---|
| ![Opened up](docs/photos/opened.jpg) | ![Inside the back half](docs/photos/inside.jpg) | ![Power button and antenna](docs/photos/power-and-antenna.jpg) |
| Opened up: the case, the display and buttons on their carrier, the electronics tray and the back cover | The host side: a Raspberry Pi Zero, a Digi XBee radio for the ZigBee network, a power board and a LiPo pouch cell | A lit metal power button and an SMA antenna for the XBee on the top edge |

![The seven buttons](docs/photos/buttons.jpg)

In the handheld, NERD is the screen and buttons for a ZigBee network tool. The Raspberry Pi Zero is the host: it talks to the ZigBee network through the XBee and drives the display over UART with the protocol above, for example listing the coordinators and routers a scan finds. Everything runs from the LiPo cell, and the case is printed in clear filament with brass heat-set inserts. The seven buttons are printed caps over tactile switches.

## Hardware

- **LilyGo T5 4.7" EPD** (ESP32 + 960x540 e-paper)
- 7 push buttons:

| Button | GPIO |
|---|---|
| Up | 35 |
| Down | 12 |
| Left | 13 |
| Right | 14 |
| Select | 34 |
| Back | 15 |
| Home | 39 |

## Building

This is a [PlatformIO](https://platformio.org/) project (the VS Code extension is the easiest way in). `platformio.ini` pins everything, so there's nothing to install by hand:

| Dependency | Version |
|---|---|
| Arduino-ESP32 core | 2.0.6 (`espressif32@6.0.1`) |
| [LilyGo-EPD47](https://github.com/Xinyuan-LilyGO/LilyGo-EPD47/tree/v0.1.0) | v0.1.0, from GitHub |
| ArduinoJson | 7.3.1 |

```bash
pio run -t upload      # build and flash
pio device monitor     # serial monitor at 115200
```

## Code layout

| Path | Purpose |
|---|---|
| `platformio.ini` | Board, build settings and libraries |
| `src/main.cpp` | Setup and main loop |
| `src/UARTProtocol.*`, `PacketCRC.*` | Serial framing and CRC |
| `ScreenManager.*` | Parses incoming JSON and switches screens |
| `BaseScreen.*` and `*Screen.*` | One class per screen type |
| `Buttons.*` | Debounced buttons with short/long press callbacks |
| `Display.*` | E-paper drawing helpers |
| `Fonts/` | Pre-converted Open Sans fonts |
| `debug/` | JTAG debugger files from the Arduino IDE setup (OpenOCD config, SVD) |
| `scripts/fontconvert.py` | Convert a TTF to an EPD font header ([details](scripts/README.MD)) |
| `scripts/imgconvert.py` | Convert an image (e.g. `splash.jpg`) to a header |
| `scripts/screen_test.py` | Interactive screen tester (see [Testing the screens](#testing-the-screens)) |
| `scripts/pio_targets.py` | Adds the `screentest` and `screendemo` PlatformIO tasks |
| `scripts/simple_test.py` | Minimal example host |

## Version

0.3.0. MIT licensed, see [LICENSE](LICENSE).
