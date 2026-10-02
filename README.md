# UART Display

A general-purpose **e-paper menu terminal**. A LilyGo T5 4.7" e-ink board with seven physical buttons shows menus, selection lists, info and loading screens. **Whatever is plugged into its serial port decides what's on screen.** The host sends small JSON messages over UART, and the display sends back what the user picked. This lets any microcontroller, Raspberry Pi or PC get a crisp, low-power UI without having to drive a display itself.

![Splash image](scripts/splash.jpg)

<!-- PHOTOS: add photos of the device here, e.g. ![UART Display](docs/device.jpg) -->

## Screens

| Screen `id` | What it shows |
|---|---|
| `splash` | Boot splash image (`SplashData.h`) |
| `main_menu` | Titled list of menu items; scroll with Up/Down, confirm with Select |
| `selection_menu` | Multi-column list for long lists (e.g. 170+ scanned devices), navigated with all four arrows |
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
{"type": "screen", "id": "selection_menu", "menuTitle": "Scanning for ZC's",
 "menuItems": ["ZC1", "ZC2", "ZC3"]}
```

`{"type": "data", ...}` sends updates to the screen that's already showing.

**Display → host** (when the user presses Select):

```json
{"type": "item_selected", "selectedItem": "Info Screen"}
```

`scripts/simple_test.py` is a ready-to-run host that walks through every screen. Set `SERIAL_PORT` at the top and run `python simple_test.py` (needs `pyserial`).

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

- Arduino IDE, **ESP32 board package 2.0.6**
- [LilyGo-EPD47 v0.1.0](https://github.com/Xinyuan-LilyGO/LilyGo-EPD47/tree/v0.1.0)
- **ArduinoJson 7.3.1** (7.4.x works but throws a warning)

Open `UARTDisplay/UARTDisplay.ino` and upload.

## Code layout

| Path | Purpose |
|---|---|
| `UARTDisplay/UARTDisplay.ino` | Setup and main loop |
| `UARTProtocol.*`, `PacketCRC.*` | Serial framing and CRC |
| `ScreenManager.*` | Parses incoming JSON and switches screens |
| `BaseScreen.*` and `*Screen.*` | One class per screen type |
| `Buttons.*` | Debounced buttons with short/long press callbacks |
| `Display.*` | E-paper drawing helpers |
| `Fonts/` | Pre-converted Open Sans fonts |
| `debug.cfg`, `debug_custom.json`, `esp32.svd` | Hardware debugger configuration |
| `scripts/fontconvert.py` | Convert a TTF to an EPD font header ([details](scripts/README.MD)) |
| `scripts/imgconvert.py` | Convert an image (e.g. `splash.jpg`) to a header |
| `scripts/simple_test.py` | Example host / test script |

## Version

0.3.0. MIT licensed, see [LICENSE](LICENSE).
