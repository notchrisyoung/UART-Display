#include "MenuScreen.h"

MenuScreen::MenuScreen() {
    // Initialize menu items
    menuItems = nullptr;
    menuItemCount = 0;
    selectedIndex = 0;
    currentSelection = 0;
    menuTitle = "";
}
void MenuScreen::init() {
    // Initialize the screen (e.g., setup menu items)
    Serial.println("MenuScreen init");
}
void MenuScreen::update() {
  // No periodic updates needed for static menu
}
void MenuScreen::draw() {
    Serial.println("MenuScreen draw");
    Display::clear();
    Display::setFont(OpenSansB24);
    Display::drawString((DISPLAY_WIDTH/2)-5, MENU_SCREEN_OFFSET, menuTitle, Display::CENTER);
    for (size_t i = 0; i < menuItemCount; i++) {
        Display::drawString(50, MENU_SCREEN_OFFSET + (MENU_SCREEN_SPACING * (i + 1)), menuItems[i], Display::LEFT);
    }
    highlight(selectedIndex);
}
void MenuScreen::processUartData(JsonDocument& doc) {
    Serial.println("MenuScreen processUartData");
    // Process the message and update the menu items if needed
    // Add logic to handle incoming UART data and update menu items
    if (doc.containsKey("menuItems")) {
        JsonArray items = doc["menuItems"].as<JsonArray>();
        menuItemCount = items.size();
        menuTitle = doc["title"].as<String>();
        menuItems = new String[menuItemCount];
        for (size_t i = 0; i < menuItemCount; i++) {
            menuItems[i] = items[i].as<String>();
        }
    }
    draw(); // draw the menu after processing
}
void MenuScreen::sendUartData(const String& message) {
    // Send data over UART if needed
    // Add logic to send data over UART
    
}
void MenuScreen::highlight(int item) {
    if (item != currentSelection) {
        clearHighlight();
    }
    int y = MENU_SCREEN_SPACING * (item + 1);
    int x1 = 12;
    int x2 = 40; 
    Display::fillTriangle(x1, y + 10, x1, y + 50, x2, y + 30, BLACK);
    currentSelection = item;
    Display::update(); 
}
void MenuScreen::clearHighlight() {
	Rect_t rect = {0, 0, 52, EPD_HEIGHT};
	Display::fillRect(rect.x, rect.y, rect.width, rect.height, WHITE);
	for (int16_t  i = 0; i < 4; i++){
        Display::pushPixels(rect,50,1);
    }
}
void MenuScreen::upPress() {
    if (selectedIndex > 0) {
        selectedIndex--;
        highlight(selectedIndex);
    }
    else {
        selectedIndex = menuItemCount - 1;
        highlight(selectedIndex);
    }
}
void MenuScreen::downPress() {
    if (selectedIndex < menuItemCount - 1) {
        selectedIndex++;
        highlight(selectedIndex);
    }
    else {
        selectedIndex = 0;
        highlight(selectedIndex);
    }
}
void MenuScreen::leftPress() {
    // Handle left button press if needed
}
void MenuScreen::rightPress() {
    // Handle right button press if needed
}
void MenuScreen::selectPress() {
    
    sendUartData("select");
}
void MenuScreen::backPress() {
    // Handle back button press, e.g., return to the previous screen
    Serial.println("Back pressed");
    // Add logic to handle back navigation
}