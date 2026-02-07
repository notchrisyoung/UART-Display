#include <Arduino.h>
#include <String.h>
#include "ScreenManager.h"
#include "UARTProtocol.h"

ScreenManager sm;
UARTProtocol uart;

void setup() {
    // Initialize UART communication
    uart.begin(115200);
    delay(1000);
    // Initialize the display
    Display::begin();
    // Initialize the buttons
    Buttons::begin();
    // Initialize the screen manager
    sm.init();
    uart.setCallback([](const uint8_t* data, uint8_t len) {
        sm.processUartData(const_cast<uint8_t*>(data), len);
    });
    sm.setUARTcallback([](const String& message) {
        uart.sendMessage((uint8_t*)message.c_str(), message.length());
    });
}

void loop() {
    Buttons::update();
    sm.update();
    uart.process();
    delay(10);
}








/*
    //################ Draw FUNCTIONS ##################################################

    //Drawing zc menu
    void DrawZCMenu() {
        epd_clear();
        memset(framebuffer, 0xFF, EPD_WIDTH * EPD_HEIGHT / 2);
        setFont(OpenSans14);
        drawString((EPD_WIDTH/2)-5, 28, TXT_SCANNING_FOR_ZC, CENTER);
        drawFastHLine(0, 39, EPD_WIDTH, Black);
        currentScreen = SCANNING_FOR_ZC;
        edp_update(); 
        DrawZCScannedMenuItems(ZCMenuItems, 100); 
        currentSelection = 1;
        ZCMenuHighlight(1);
    }
    void DrawZCScannedMenuItems(const char* menuItems[], int itemCount) {
        setFont(OpenSans12);
        for (int i = scanZCCount; i < scanZCCount+itemCount; i++) {
            int column = i / ZC_ITEM_PER_COLUMN;
            int row = i % ZC_ITEM_PER_COLUMN;
            int x = 35 + column * ZC_COLUMN_WIDTH;
            int y = SCAN_ZC_MENU_OFFSET + (SCAN_ZC_MENU_SPACING * (row + 1));
            drawString(x, y, menuItems[i], LEFT);
        }
        scanZCCount += itemCount;
        edp_update(); 
    }
    void ZCMenuHighlight(int menu) {
        int column = (currentSelection - 1) / ZC_ITEM_PER_COLUMN;
        int row = (currentSelection - 1) % ZC_ITEM_PER_COLUMN;
        int x = 3 + column * ZC_COLUMN_WIDTH;
        int y = 9 + SCAN_ZC_MENU_OFFSET + (SCAN_ZC_MENU_SPACING * (row));
        if (menu != currentSelection) {
            Rect_t rect = {x, y, 31, 32};
            epd_fill_rect(rect.x, rect.y, rect.width, rect.height, White, framebuffer);
            for (int16_t  i = 0; i < 10; i++){
                epd_push_pixels(rect,50,1);
            }
        }
        column = (menu - 1) / ZC_ITEM_PER_COLUMN;
        row = (menu - 1) % ZC_ITEM_PER_COLUMN;
        x = 12 + column * ZC_COLUMN_WIDTH;
        y = 15 + SCAN_ZC_MENU_OFFSET + (SCAN_ZC_MENU_SPACING * (row));
        fillTriangle(x, y , x, y + 15, x + 16, y + 7, Black);
        currentSelection = menu;
        edp_update();
    }
    void DrawZCConnectingScreen() {
        epd_clear();
        memset(framebuffer, 0xFF, EPD_WIDTH * EPD_HEIGHT / 2);
        setFont(OpenSans14);
        drawString((EPD_WIDTH/2)-5, 28, TXT_CONNECTING_TO_ZC, CENTER);
        drawFastHLine(0, 39, EPD_WIDTH, Black);
        currentScreen = CONNECTING_TO_ZC;
        edp_update();
    }

    //Drawing rc menu
    void DrawRCMenu() {
        epd_clear();
        memset(framebuffer, 0xFF, EPD_WIDTH * EPD_HEIGHT / 2);
        setFont(OpenSans14);
        drawString((EPD_WIDTH/2)-5, 28, TXT_SCANNING_FOR_RC, CENTER);
        drawFastHLine(0, 39, EPD_WIDTH, Black);
        currentScreen = SCANNING_FOR_RC;
        edp_update(); 
        DrawRCScannedMenuItems(RCMenuItems, 100); 
        currentSelection = 1;
        RCMenuHighlight(1);
    }
    void DrawRCScannedMenuItems(const char* menuItems[], int itemCount) {
        setFont(OpenSans12);
        for (int i = scanRCCount; i < scanRCCount+itemCount; i++) {
            int column = i / ZC_ITEM_PER_COLUMN;
            int row = i % ZC_ITEM_PER_COLUMN;
            int x = 35 + column * ZC_COLUMN_WIDTH;
            int y = SCAN_ZC_MENU_OFFSET + (SCAN_ZC_MENU_SPACING * (row + 1));
            drawString(x, y, menuItems[i], LEFT);
        }
        scanRCCount += itemCount;
        edp_update(); 
    }
    void RCMenuHighlight(int menu) {
        int column = (currentSelection - 1) / ZC_ITEM_PER_COLUMN;
        int row = (currentSelection - 1) % ZC_ITEM_PER_COLUMN;
        int x = 3 + column * ZC_COLUMN_WIDTH;
        int y = 9 + SCAN_ZC_MENU_OFFSET + (SCAN_ZC_MENU_SPACING * (row));
        if (menu != currentSelection) {
            Rect_t rect = {x, y, 31, 32};
            epd_fill_rect(rect.x, rect.y, rect.width, rect.height, White, framebuffer);
            for (int16_t  i = 0; i < 10; i++){
                epd_push_pixels(rect,50,1);
            }
        }
        column = (menu - 1) / ZC_ITEM_PER_COLUMN;
        row = (menu - 1) % ZC_ITEM_PER_COLUMN;
        x = 12 + column * ZC_COLUMN_WIDTH;
        y = 15 + SCAN_ZC_MENU_OFFSET + (SCAN_ZC_MENU_SPACING * (row));
        fillTriangle(x, y , x, y + 15, x + 16, y + 7, Black);
        currentSelection = menu;
        edp_update();
    }

    //Draw Rc info screen
    void DrawRCInfoScreen() {
        epd_clear();
        memset(framebuffer, 0xFF, EPD_WIDTH * EPD_HEIGHT / 2);
        setFont(OpenSans14);
        String title = String(TXT_RC_INFO) + " - " + RCMenuItems[currentSelection - 1];
        drawString((EPD_WIDTH/2)-5, 28, title, CENTER);
        drawFastHLine(0, 39, EPD_WIDTH, Black);
        currentScreen = RC_INFO_SCREEN;
        edp_update();
    }
*/
