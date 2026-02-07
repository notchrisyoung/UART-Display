#include "SelectionScreen.h"

SelectionScreen::SelectionScreen() {
    // Initialize menu items
    screenItems = nullptr;
    screenItemCount = 0;
    selectedIndex = 0;
    currentSelection = 0;
    screenTitle = "";
}

void SelectionScreen::init() {
    // Initialize the screen (e.g., setup menu items)
    //Serial.println("SelectionScreen init");
}

void SelectionScreen::update() {
    // No periodic updates needed for static menu
}

void SelectionScreen::draw() {
    Display::clear();
    Display::setFont(OpenSansB14);
    Display::drawString((DISPLAY_WIDTH/2)-5, SELECTION_SCREEN_OFFSET, screenTitle, Display::CENTER);
    Display::drawFastHLine(0, 39, EPD_WIDTH, Black);
    Display::setFont(OpenSans12);
    for (size_t i = 0; i < screenItemCount; i++) {
        Display::drawString(50, SELECTION_SCREEN_OFFSET + (SELECTION_SCREEN_SPACING * (i + 1)), screenItems[i], Display::LEFT);
    }
    highlight(selectedIndex);
}

void SelectionScreen::processUartData(JsonDocument& doc) {
    //Serial.println("SelectionScreen processUartData");
    if (doc.containsKey("screenItems")) {
        JsonArray items = doc["screenItems"].as<JsonArray>();
        screenItemCount = items.size();
        screenTitle = doc["screenTitle"].as<String>();
        screenItems = new String[screenItemCount];
        for (size_t i = 0; i < screenItemCount; i++) {
            screenItems[i] = items[i].as<String>();
        }
    }
    draw(); // draw the menu after processing
}

void SelectionScreen::sendUartData(const String& message) {
    if (uartSendCallback) {
        uartSendCallback(message);
    }
    
}

void SelectionScreen::setUARTcallback(void (*callback)(const String& message)) {
    uartSendCallback = callback;
}

void SelectionScreen::highlight(int item) {
    if (item != currentSelection) {
        clearHighlight();
    }
    int y = SELECTION_SCREEN_SPACING * (item + 1);
    int x1 = 12;
    int x2 = 40; 
    Display::fillTriangle(x1, y + 10, x1, y + 50, x2, y + 30, BLACK);
    currentSelection = item;
    Display::update(); 
}

void SelectionScreen::clearHighlight() {
	Rect_t rect = {0, 0, 52, EPD_HEIGHT};
	Display::fillRect(rect.x, rect.y, rect.width, rect.height, WHITE);
	for (int16_t  i = 0; i < 4; i++){
        Display::pushPixels(rect,50,1);
    }
}

void SelectionScreen::upPress() {
    if (selectedIndex > 0) {
        selectedIndex--;
        highlight(selectedIndex);
    }
    else {
        selectedIndex = screenItemCount - 1;
        highlight(selectedIndex);
    }
}

void SelectionScreen::downPress() {
    if (selectedIndex < screenItemCount - 1) {
        selectedIndex++;
        highlight(selectedIndex);
    }
    else {
        selectedIndex = 0;
        highlight(selectedIndex);
    }
}

void SelectionScreen::leftPress() {
    // Handle left button press if needed
}

void SelectionScreen::rightPress() {
    // Handle right button press if needed
}

void SelectionScreen::selectPress() {
    JsonDocument doc;
    doc["type"] = "item_selected";
    doc["selectedItem"] = screenItems[selectedIndex];
    String message;
    serializeJson(doc, message);
    sendUartData(message);
}

void SelectionScreen::backPress() {
    // Handle back button press, e.g., return to the previous screen
    //Serial.println("Back pressed");
    // Add logic to handle back navigation
}

//################ Draw FUNCTIONS ##################################################
void SelectionScreen::DrawZCMenu() {
    setFont(OpenSans14);
    drawString((EPD_WIDTH/2)-5, 28, TXT_SCANNING_FOR_ZC, CENTER);
    drawFastHLine(0, 39, EPD_WIDTH, Black);
    edp_update(); 
    DrawZCScannedMenuItems(ZCMenuItems, 100); 
    currentSelection = 1;
    ZCMenuHighlight(1);
}

void SelectionScreen::DrawZCScannedMenuItems(const char* menuItems[], int itemCount) {
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

void SelectionScreen::ZCMenuHighlight(int menu) {
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

void SelectionScreen::DrawZCConnectingScreen() {
    epd_clear();
    memset(framebuffer, 0xFF, EPD_WIDTH * EPD_HEIGHT / 2);
    setFont(OpenSans14);
    drawString((EPD_WIDTH/2)-5, 28, TXT_CONNECTING_TO_ZC, CENTER);
    drawFastHLine(0, 39, EPD_WIDTH, Black);
    currentScreen = CONNECTING_TO_ZC;
    edp_update();
}

//Drawing rc menu
void SelectionScreen::DrawRCMenu() {
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
void SelectionScreen::DrawRCScannedMenuItems(const char* menuItems[], int itemCount) {
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
void SelectionScreen::RCMenuHighlight(int menu) {
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
void SelectionScreen::DrawRCInfoScreen() {
    epd_clear();
    memset(framebuffer, 0xFF, EPD_WIDTH * EPD_HEIGHT / 2);
    setFont(OpenSans14);
    String title = String(TXT_RC_INFO) + " - " + RCMenuItems[currentSelection - 1];
    drawString((EPD_WIDTH/2)-5, 28, title, CENTER);
    drawFastHLine(0, 39, EPD_WIDTH, Black);
    currentScreen = RC_INFO_SCREEN;
    edp_update();
}