#!/usr/bin/env python3
"""
Simple UART Test Script for Arduino NERD Display
Quick and easy testing without command line arguments
"""

import serial
import json
import time

# Configuration - CHANGE THESE AS NEEDED
SERIAL_PORT = "COM17"  # Change to your Arduino's serial port
BAUDRATE = 115200

# Protocol constants
START_BYTE = 0x02
END_BYTE = 0xFF
MESSAGE_OK = 0x10
MESSAGE_ERR = 0x11

class PacketCRC:
    def __init__(self, polynomial=0x9B, crc_len=8):
        self.poly = polynomial
        self.crc_len = crc_len
        self.table_len = 2 ** crc_len
        self.cs_table = []
        self.generate_table()
    
    def generate_table(self):
        for i in range(self.table_len):
            curr = i
            for j in range(8):
                if (curr & 0x80) != 0:
                    curr = ((curr << 1) ^ self.poly) & 0xFF
                else:
                    curr = (curr << 1) & 0xFF
            self.cs_table.append(curr)
    
    def calculate(self, data):
        crc = 0
        for byte in data:
            crc = self.cs_table[crc ^ byte]
        return crc

# Initialize CRC calculator
crc_calculator = PacketCRC(0x9B, 8)

def calc_checksum(data):
    return crc_calculator.calculate(data)

def send_message(ser, json_data):
    json_str = json.dumps(json_data, separators=(',', ':'))
    data = json_str.encode('utf-8')
    length = len(data)
    checksum = calc_checksum(data)
    frame = bytearray([START_BYTE])
    frame.extend(length.to_bytes(2, byteorder='big'))  # 2 bytes for length
    frame.extend(data)
    frame.append(checksum)
    frame.append(END_BYTE)

    print(f"Sending: {json_str}")
    print(f"Frame length: {len(frame)} bytes")
    print(f"Frame bytes: {' '.join(f'{b:02X}' for b in frame)}")
    
    for byte in frame:
        ser.write(bytes([byte]))
    ser.flush()

def receive_message(ser, timeout=5):
    start_time = time.time()
    # Wait for start byte
    while (time.time() - start_time) < timeout:
        if ser.in_waiting > 0:
            byte = ser.read(1)
            print(byte.decode('utf-8', errors='ignore'), end='')
            if byte[0] == START_BYTE:
                break
            
    else:
        print("Timeout waiting for start byte")
        return None
    
    # Read length (2 bytes)
    length_bytes = ser.read(2)
    if len(length_bytes) != 2:
        print("No length bytes received")
        return None
    
    data_length = int.from_bytes(length_bytes, byteorder='big')
    
    # Read data
    data = ser.read(data_length)
    if len(data) != data_length:
        print(f"Expected {data_length} bytes, got {len(data)}")
        return None
    
    # Read checksum
    checksum_byte = ser.read(1)
    if len(checksum_byte) == 0:
        print("No checksum received")
        return None
    
    # Read end byte
    end_byte = ser.read(1)
    if len(end_byte) == 0 or end_byte[0] != END_BYTE:
        print("Invalid end byte")
        return None
    
    # Verify checksum
    expected_checksum = calc_checksum(data)
    if checksum_byte[0] != expected_checksum:
        print(f"Checksum mismatch: expected {expected_checksum}, got {checksum_byte[0]}")
        return None
    return json.loads(data.decode('utf-8'))


def main():
    try:
        print(f"Connecting to {SERIAL_PORT}...")
        ser = serial.Serial(SERIAL_PORT, BAUDRATE, timeout=1)
        time.sleep(2)  
        print("Connected!")
        print("\nReading initial output from Arduino...")
        time.sleep(0.5)
        while ser.in_waiting > 0:
            output = ser.readline().decode('utf-8', errors='ignore').strip()
            if output:
                print(f"Arduino: {output}")
        print("\n1. Switching to main menu...")
        send_message(ser, {
            "type": "screen",
            "id": "main_menu",
            "menuTitle": "Main Menu",
            "menuItems": [
                "Selection Screen",
                "Loading Screen",
                "Info Screen",
                "Splash Screen"
            ],
        })
        print("\nWaiting for response from Arduino...")
        while True:
            response = receive_message(ser, timeout=60)
            if response is None:
                print("No response received. Exiting...")
                break
            
            # Process the response
            if response.get("type") == "item_selected":
                menu_item = response.get("selectedItem")
                print(f"\n>>> Menu item selected: {menu_item}")
                if menu_item == "Menu Screen":
                    print("Loading 'Menu Screen'...")
                    send_message(ser, {
                        "type": "screen",
                        "id": "main_menu",
                        "menuTitle": "Main Menu",
                        "menuItems": [
                            "Selection Screen",
                            "Loading Screen",
                            "Info Screen",
                            "Splash Screen"
                        ],
                    })
                elif menu_item == "Selection Screen":
                    print("Loading 'Selection Screen'...")
                    send_message(ser, {
                        "type": "screen",
                        "id": "selection_menu",
                        "menuTitle": "Scanning for ZC's",
                        "menuItems": [f"ZC{i}" for i in range(1, 175)]
                    })
                elif menu_item == "Info Screen":
                    print("Loading 'Info Screen'...")
                    send_message(ser, {
                        "type": "screen",
                        "id": "info_screen",
                        "title": "System Info",
                        "infoItems": [
                            "Firmware: v1.0.0", 
                            "Hardware: ESP32",
                            "Display: e-Paper",
                            "Status: Running"
                        ]
                    })
                elif menu_item == "Loading Screen":
                    print("Loading 'Loading Screen'...")
                    send_message(ser, {
                        "type": "screen",
                        "id": "loading_screen",
                        "title": "Loading",
                        "message": "Please wait..."
                    })
                elif menu_item == "Splash Screen":
                    print("Loading 'Splash Screen'...")
                    send_message(ser, {
                        "type": "screen",
                        "id": "splash"
                    })
                else:
                    print(f"Unknown menu item: {menu_item}")
            
            elif response.get("type") == "status":
                status = response.get("status")
                print(f"Status update: {status}")
            
            else:
                print(f"Unknown response type: {response}")
        
        
    except serial.SerialException as e:
        print(f"Serial error: {e}")
        print("Make sure:")
        print("1. Arduino is connected")
        print("2. Correct COM port is specified")
        print("3. Arduino Serial Monitor is closed")
    except Exception as e:
        print(f"Error: {e}")
    finally:
        if 'ser' in locals() and ser.is_open:
            ser.close()
            print("Serial connection closed")

if __name__ == "__main__":
    main()