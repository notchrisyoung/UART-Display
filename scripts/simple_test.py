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
END_BYTE = 0x03

def calc_checksum(data):
    """Calculate XOR checksum"""
    checksum = 0
    for byte in data:
        checksum ^= byte
    return checksum & 0xFF

def send_message(ser, json_data):
    """Send JSON message using UART protocol"""
    json_str = json.dumps(json_data, separators=(',', ':'))
    data = json_str.encode('utf-8')
    length = len(data)
    checksum = calc_checksum(data)
    
    # Build frame
    frame = bytearray([START_BYTE, length])
    frame.extend(data)
    frame.append(checksum)
    frame.append(END_BYTE)
    
    print(f"Sending: {json_str}")
    ser.write(frame)
    ser.flush()

def receive_message(ser, timeout=5):
    """Receive JSON message from serial port"""
    start_time = time.time()
    
    # Wait for start byte
    while (time.time() - start_time) < timeout:
        if ser.in_waiting > 0:
            byte = ser.read(1)
            if byte[0] == START_BYTE:
                break
    else:
        print("Timeout waiting for start byte")
        return None
    
    # Read length byte
    length_byte = ser.read(1)
    if len(length_byte) == 0:
        print("No length byte received")
        return None
    
    data_length = length_byte[0]
    
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
    
    # Parse JSON
    try:
        json_str = data.decode('utf-8')
        json_data = json.loads(json_str)
        print(f"Received: {json_str}")
        return json_data
    except Exception as e:
        print(f"Error parsing JSON: {e}")
        return None

def main():
    try:
        # Connect to Arduino
        print(f"Connecting to {SERIAL_PORT}...")
        ser = serial.Serial(SERIAL_PORT, BAUDRATE, timeout=1)
        time.sleep(2)  # Allow Arduino to reset
        print("Connected!")
        
        # Test sequence
        print("\n=== Quick Test Sequence ===")
        
        # 1. Switch to main menu
        print("\n1. Switching to main menu...")
        send_message(ser, {
            "type": "screen",
            "id": "main_menu",
            "title": "Main Menu",
            "menuItems": [
                "Menu Screen",
                "Selection Screen",
                "Info Screen",
                "Loading Screen",
                "Splash Screen"
            ]
        })
        
        # Wait for response from Arduino
        print("\nWaiting for response from Arduino...")
        while True:
            response = receive_message(ser, timeout=30)
            if response is None:
                print("No response received. Exiting...")
                break
            
            # Process the response
            if response.get("type") == "button":
                button = response.get("button")
                print(f"\n>>> Button pressed: {button}")
                
                # Handle different button presses
                if button == "select":
                    selected = response.get("selected", 0)
                    print(f"Selected menu item: {selected}")
                    # Here you can decide which menu to display next based on selection
                    # For example:
                    if selected == 0:
                        print("Loading 'Connect to ZC' screen...")
                        # send_message(ser, {...})
                    elif selected == 1:
                        print("Loading 'Connect to RC' screen...")
                        # send_message(ser, {...})
                    elif selected == 2:
                        print("Loading 'Settings' screen...")
                        # send_message(ser, {...})
                    elif selected == 3:
                        print("Loading 'System Info' screen...")
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