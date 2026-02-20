#include "SelectionScreen.h"

SelectionScreen::SelectionScreen() {
    // Initialize menu items
    screenItems = nullptr;
    drawnItemCount = 0;
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
}

void SelectionScreen::processUartData(JsonDocument& doc) {
    //Serial.println("SelectionScreen processUartData");
    if (doc.containsKey("menuItems")) {
        JsonArray items = doc["menuItems"].as<JsonArray>();
        screenItemCount = items.size();
        screenTitle = doc["menuTitle"].as<String>();
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

void SelectionScreen::upPress() {
    // Calculate current column and row
    uint16_t currentColumn = selectedIndex / ITEM_PER_COLUMN;
    uint16_t currentRow = selectedIndex % ITEM_PER_COLUMN;
    
    // Move up, wrapping to bottom if at the top
    uint16_t newRow;
    if (currentRow > 0) {
        newRow = currentRow - 1;
    } else {
        newRow = ITEM_PER_COLUMN - 1;  // Wrap to bottom row
    }
    
    // Calculate new index
    uint16_t newIndex = (currentColumn * ITEM_PER_COLUMN) + newRow;
    
    // If the new index exceeds the item count, adjust to the last valid item in current column
    if (newIndex >= screenItemCount) {
        newIndex = (currentColumn * ITEM_PER_COLUMN) + ((screenItemCount - 1) % ITEM_PER_COLUMN);
    }
    
    // Ensure we don't go out of bounds
    if (newIndex < screenItemCount) {
        selectedIndex = newIndex;
        highlight(selectedIndex);
    }
}

void SelectionScreen::downPress() {
    // Calculate current column and row
    uint16_t currentColumn = selectedIndex / ITEM_PER_COLUMN;
    uint16_t currentRow = selectedIndex % ITEM_PER_COLUMN;
    
    // Move down, wrapping to top if at the bottom
    uint16_t newRow;
    if (currentRow < ITEM_PER_COLUMN - 1) {
        newRow = currentRow + 1;
    } else {
        newRow = 0;  // Wrap to top row
    }
    
    // Calculate new index
    uint16_t newIndex = (currentColumn * ITEM_PER_COLUMN) + newRow;
    
    // If the new index exceeds the item count, wrap to first item in current column
    if (newIndex >= screenItemCount) {
        newIndex = currentColumn * ITEM_PER_COLUMN;
    }
    
    // Ensure we don't go out of bounds
    if (newIndex < screenItemCount) {
        selectedIndex = newIndex;
        highlight(selectedIndex);
    }
}

void SelectionScreen::leftPress() {
    // Calculate current column and row
    uint16_t currentColumn = selectedIndex / ITEM_PER_COLUMN;
    uint16_t currentRow = selectedIndex % ITEM_PER_COLUMN;
    
    // Calculate total number of columns
    uint16_t totalColumns = (screenItemCount + ITEM_PER_COLUMN - 1) / ITEM_PER_COLUMN;
    
    // Move left, wrapping around to the rightmost column if at the left edge
    uint16_t newColumn;
    if (currentColumn > 0) {
        newColumn = currentColumn - 1;
    } else {
        newColumn = totalColumns - 1;  // Wrap to rightmost column
    }
    
    // Calculate new index, but make sure it doesn't exceed total items
    uint16_t newIndex = (newColumn * ITEM_PER_COLUMN) + currentRow;
    
    // If the new index exceeds the item count, adjust to the last valid item in that column
    if (newIndex >= screenItemCount) {
        newIndex = (newColumn * ITEM_PER_COLUMN) + ((screenItemCount - 1) % ITEM_PER_COLUMN);
    }
    
    // Ensure we don't go out of bounds
    if (newIndex < screenItemCount) {
        selectedIndex = newIndex;
        highlight(selectedIndex);
    }
}

void SelectionScreen::rightPress() {
    // Calculate current column and row
    uint16_t currentColumn = selectedIndex / ITEM_PER_COLUMN;
    uint16_t currentRow = selectedIndex % ITEM_PER_COLUMN;
    
    // Calculate total number of columns
    uint16_t totalColumns = (screenItemCount + ITEM_PER_COLUMN - 1) / ITEM_PER_COLUMN;
    
    // Move right, wrapping around to the leftmost column if at the right edge
    uint16_t newColumn;
    if (currentColumn < totalColumns - 1) {
        newColumn = currentColumn + 1;
    } else {
        newColumn = 0;  // Wrap to leftmost column
    }
    
    // Calculate new index, but make sure it doesn't exceed total items
    uint16_t newIndex = (newColumn * ITEM_PER_COLUMN) + currentRow;
    
    // If the new index exceeds the item count, adjust to the last valid item in that column
    if (newIndex >= screenItemCount) {
        newIndex = (newColumn * ITEM_PER_COLUMN) + ((screenItemCount - 1) % ITEM_PER_COLUMN);
    }
    
    // Ensure we don't go out of bounds
    if (newIndex < screenItemCount) {
        selectedIndex = newIndex;
        highlight(selectedIndex);
    }
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
void SelectionScreen::draw() {
    Display::clear();
    drawTitle();
    drawItems(screenItems, screenItemCount);
    highlight(selectedIndex);
}

void SelectionScreen::drawTitle() {
    Display::setFont(OpenSansB14);
    Display::drawString((DISPLAY_WIDTH/2)-5, SELECTION_SCREEN_TITLE_OFFSET, screenTitle, Display::CENTER);
    Display::drawFastHLine(0, TITLE_HIGHT, DISPLAY_WIDTH, BLACK);
}

void SelectionScreen::drawItems(String*& menuItems, uint16_t itemCount) {
    Display::setFont(OpenSans12);
    for (uint16_t i = 0; i < itemCount; i++) {
        uint16_t column = i / ITEM_PER_COLUMN;
        uint16_t row = i % ITEM_PER_COLUMN;
        uint32_t x = 35 + column * COLUMN_WIDTH;
        uint32_t y = ITEM_OFFSET + (ITEM_SPACING * (row + 1));
        Display::drawString(x, y, menuItems[i], Display::LEFT);
    }
    drawnItemCount += itemCount;
} 

void SelectionScreen::highlight(uint16_t item) {
    uint16_t column = (currentSelection) / ITEM_PER_COLUMN;
    uint16_t row = (currentSelection) % ITEM_PER_COLUMN;
    uint16_t x = 3 + column * COLUMN_WIDTH;
    uint16_t y = 9 + ITEM_OFFSET + (ITEM_SPACING * (row));
    if (item != currentSelection) {
        Rect_t rect = {x, y, 31, 32};
        Display::fillRect(rect.x, rect.y, rect.width, rect.height, WHITE);
        for (int16_t  i = 0; i < 10; i++){
            Display::pushPixels(rect,50,1);
        }
    }
    column = (item) / ITEM_PER_COLUMN;
    row = (item) % ITEM_PER_COLUMN;
    x = 12 + column * COLUMN_WIDTH;
    y = 15 + ITEM_OFFSET + (ITEM_SPACING * (row));
    Display::fillTriangle(x, y , x, y + 15, x + 16, y + 7, BLACK);
    currentSelection = item;
    Display::update();
}

void SelectionScreen::clearMenuArea(){
    Rect_t rect = {0, TITLE_HIGHT, EPD_WIDTH, EPD_HEIGHT - TITLE_HIGHT};
    Display::fillRect(rect.x, rect.y, rect.width, rect.height, WHITE);
}