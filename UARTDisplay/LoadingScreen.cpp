#include "LoadingScreen.h"

LoadingScreen::LoadingScreen() {
    // Initialize menu items
    menuItems = nullptr;
    menuItemCount = 0;
    selectedIndex = 0;
    currentSelection = 0;
    menuTitle = "";
}
void LoadingScreen::init() {
    // Initialize the screen (e.g., setup menu items)
}
void LoadingScreen::update() {
  // No periodic updates needed for static menu
}
void LoadingScreen::draw() {
    Display::clear();
    Display::setFont(OpenSans24);
    Display::drawString((DISPLAY_WIDTH/2)-5, MENU_SCREEN_OFFSET, menuTitle, Display::CENTER);
    for (size_t i = 1; i < menuItemCount; i++) {
        Display::drawString(50, MENU_SCREEN_OFFSET + (MENU_SCREEN_SPACING * (i + 1)), menuItems[i], Display::LEFT);
    }
    highlight(selectedIndex);
}
void LoadingScreen::processUartData(JsonDocument& doc) {
    // Process the message and update the menu items if needed
    // Add logic to handle incoming UART data and update menu items
    draw(); // draw the menu after processing
}
void LoadingScreen::sendUartData(const String& message) {
    // Send data over UART if needed
    // Add logic to send data over UART
}
void LoadingScreen::highlight(int item) {
    if (item != currentSelection) {
        clearHighlight();
    }
    int y = MENU_SCREEN_SPACING * item; 
    int x1 = 12;
    int x2 = 40; 
    Display::fillTriangle(x1, y + 10, x1, y + 50, x2, y + 30, BLACK);
    currentSelection = item;
    Display::update(); 
}
void LoadingScreen::clearHighlight() {
	Rect_t rect = {0, 0, 52, EPD_HEIGHT};
	Display::fillRect(rect.x, rect.y, rect.width, rect.height, WHITE);
	for (int16_t  i = 0; i < 4; i++){
        Display::pushPixels(rect,50,1);
    }
}
void LoadingScreen::upPress() {
    if (selectedIndex > 0) {
        selectedIndex--;
        highlight(selectedIndex + 1);
    }
}
void LoadingScreen::downPress() {
    if (selectedIndex < menuItemCount - 1) {
        selectedIndex++;
        highlight(selectedIndex + 1);
    }
}
void LoadingScreen::leftPress() {
    // Handle left button press if needed
}
void LoadingScreen::rightPress() {
    // Handle right button press if needed
}
void LoadingScreen::selectPress() {
    // Handle select button press, e.g., execute action for selected menu item
    if (selectedIndex < menuItemCount) {
        // Add logic to handle the selected menu item
    }
}
void LoadingScreen::backPress() {
    // Handle back button press, e.g., return to the previous screen
    Serial.println("Back pressed");
    // Add logic to handle back navigation
}