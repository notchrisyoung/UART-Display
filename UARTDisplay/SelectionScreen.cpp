#include "SelectionScreen.h"

SelectionScreen::SelectionScreen() {
    // Initialize menu items
    screenItems = nullptr;
    drawnItemCount = 0;
    screenItemCount = 0;
    selectedIndex = 0;
    currentSelection = 0;
    screenTitle = "";
    itemsPerColumn = 1;
    totalColumns = 0;
    currentPage = 0;
    totalPages = 0;
    columnWidths = nullptr;
    columnX = nullptr;
    columnPage = nullptr;
    layoutDirty = true;
    selectionMode = SelectionMode::ITEMS;
    uartSendCallback = nullptr;
}

SelectionScreen::~SelectionScreen() {
    clearLayout();
    if (screenItems != nullptr) {
        delete[] screenItems;
        screenItems = nullptr;
    }
}

void SelectionScreen::init() {
}

void SelectionScreen::update() {    
}

void SelectionScreen::clearLayout() {
    if (columnWidths != nullptr) {
        delete[] columnWidths;
        columnWidths = nullptr;
    }
    if (columnX != nullptr) {
        delete[] columnX;
        columnX = nullptr;
    }
    if (columnPage != nullptr) {
        delete[] columnPage;
        columnPage = nullptr;
    }
}

//################ UART Communication Functions ##################################################
void SelectionScreen::processUartData(JsonDocument& doc) {
    if (doc.containsKey("menuItems")) {
        screenTitle = doc["menuTitle"].as<String>();
        JsonArray items = doc["menuItems"].as<JsonArray>();
        screenItemCount = items.size();
        screenItems = new String[screenItemCount];
        for (size_t i = 0; i < screenItemCount; i++) {
            screenItems[i] = items[i].as<String>();
        }
        selectedIndex = 0;
        currentSelection = 0;
        selectionMode = SelectionMode::ITEMS;
    }
    computeLayout();
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


//################ Input Handling FUNCTIONS ##################################################
void SelectionScreen::upPress() {
    if (screenItemCount != 0) {
        // If on arrow buttons and multi-page, stay in arrow mode
        if (selectionMode != SelectionMode::ITEMS && totalPages > 1) {
            return;
        }
        // If on items, move up within items
        if (selectionMode == SelectionMode::ITEMS) {
            uint16_t currentColumn = selectedIndex / itemsPerColumn;
            uint16_t currentRow = selectedIndex % itemsPerColumn;
            uint16_t newRow;
            if (currentRow > 0) {
                newRow = currentRow - 1;
            } else {
                newRow = itemsPerColumn - 1;
            }
            uint16_t newIndex = (currentColumn * itemsPerColumn) + newRow;
            
            if (newIndex >= screenItemCount) {
                newIndex = (currentColumn * itemsPerColumn) + ((screenItemCount - 1) % itemsPerColumn);
            }
            if (newIndex < screenItemCount) {
                setSelection(newIndex);
            }
        }
    }
}

void SelectionScreen::downPress() {
    if (screenItemCount != 0) {
        // If on arrow buttons, move down to first item
        if (selectionMode != SelectionMode::ITEMS) {
            selectionMode = SelectionMode::ITEMS;
            draw();
            return;
        }
        
        // Calculate current column and row
        uint16_t currentColumn = selectedIndex / itemsPerColumn;
        uint16_t currentRow = selectedIndex % itemsPerColumn;
        
        // Move down, wrapping to top if at the bottom
        uint16_t newRow;
        if (currentRow < itemsPerColumn - 1) {
            newRow = currentRow + 1;
        } else {
            newRow = 0;  // Wrap to top row
        }
        
        // Calculate new index
        uint16_t newIndex = (currentColumn * itemsPerColumn) + newRow;
        
        // If the new index exceeds the item count, wrap to first item in current column
        if (newIndex >= screenItemCount) {
            newIndex = currentColumn * itemsPerColumn;
        }
        
        // Ensure we don't go out of bounds
        if (newIndex < screenItemCount) {
            setSelection(newIndex);
        }
    }
}

void SelectionScreen::leftPress() {
    if (screenItemCount != 0) {
        // If on arrow buttons and multi-page, switch between arrows
        if (selectionMode != SelectionMode::ITEMS && totalPages > 1) {
            if (selectionMode == SelectionMode::NEXT_ARROW) {
                selectionMode = SelectionMode::PREV_ARROW;
            } else {
                selectionMode = SelectionMode::NEXT_ARROW;
            }
            draw();
            return;
        }
        
        // If on items and multi-page, move to prev arrow
        if (totalPages > 1) {
            selectionMode = SelectionMode::PREV_ARROW;
            draw();
            return;
        }
        
        // Calculate current column and row
        uint16_t currentColumn = selectedIndex / itemsPerColumn;
        uint16_t currentRow = selectedIndex % itemsPerColumn;
        
        // Calculate total number of columns
        uint16_t totalColumns = (screenItemCount + itemsPerColumn - 1) / itemsPerColumn;
        
        // Move left, wrapping around to the rightmost column if at the left edge
        uint16_t newColumn;
        if (currentColumn > 0) {
            newColumn = currentColumn - 1;
        } else {
            newColumn = totalColumns - 1;  // Wrap to rightmost column
        }
        
        // Calculate new index, but make sure it doesn't exceed total items
        uint16_t newIndex = (newColumn * itemsPerColumn) + currentRow;
        
        // If the new index exceeds the item count, adjust to the last valid item in that column
        if (newIndex >= screenItemCount) {
            newIndex = (newColumn * itemsPerColumn) + ((screenItemCount - 1) % itemsPerColumn);
        }
        
        // Ensure we don't go out of bounds
        if (newIndex < screenItemCount) {
            setSelection(newIndex);
        }
    }
}

void SelectionScreen::rightPress() {
    if (screenItemCount != 0) {
        // If on arrow buttons and multi-page, switch between arrows
        if (selectionMode != SelectionMode::ITEMS && totalPages > 1) {
            if (selectionMode == SelectionMode::PREV_ARROW) {
                selectionMode = SelectionMode::NEXT_ARROW;
            } else {
                selectionMode = SelectionMode::PREV_ARROW;
            }
            draw();
            return;
        }
        
        // If on items and multi-page, move to next arrow
        if (totalPages > 1) {
            selectionMode = SelectionMode::NEXT_ARROW;
            draw();
            return;
        }
        
        // Calculate current column and row
        uint16_t currentColumn = selectedIndex / itemsPerColumn;
        uint16_t currentRow = selectedIndex % itemsPerColumn;
        
        // Calculate total number of columns
        uint16_t totalColumns = (screenItemCount + itemsPerColumn - 1) / itemsPerColumn;
        
        // Move right, wrapping around to the leftmost column if at the right edge
        uint16_t newColumn;
        if (currentColumn < totalColumns - 1) {
            newColumn = currentColumn + 1;
        } else {
            newColumn = 0;  // Wrap to leftmost column
        }
        
        // Calculate new index, but make sure it doesn't exceed total items
        uint16_t newIndex = (newColumn * itemsPerColumn) + currentRow;
        
        // If the new index exceeds the item count, adjust to the last valid item in that column
        if (newIndex >= screenItemCount) {
            newIndex = (newColumn * itemsPerColumn) + ((screenItemCount - 1) % itemsPerColumn);
        }
        
        // Ensure we don't go out of bounds
        if (newIndex < screenItemCount) {
            setSelection(newIndex);
        }
    }
}

void SelectionScreen::selectPress() {
    if (screenItemCount != 0) {
        // Handle arrow button selection
        if (selectionMode == SelectionMode::PREV_ARROW) {
            prevPage();
            selectionMode = SelectionMode::ITEMS;
            return;
        } else if (selectionMode == SelectionMode::NEXT_ARROW) {
            nextPage();
            selectionMode = SelectionMode::ITEMS;
            return;
        }
        
        // Handle item selection
        JsonDocument doc;
        doc["type"] = "item_selected";
        doc["selectedItem"] = screenItems[selectedIndex];
        String message;
        serializeJson(doc, message);
        sendUartData(message);
    }
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
    if (screenItemCount > 0) {
        drawItems(screenItems, screenItemCount);
        currentSelection = selectedIndex;
        setSelection(selectedIndex);
        if (totalPages > 1) {
            drawPageArrows(false);
            drawPageNumber();
        }
    }
}

void SelectionScreen::drawTitle() {
    Display::setFont(OpenSansB14);
    uint16_t titleWidth = Display::getTextWidth(screenTitle);
    Display::drawString((DISPLAY_WIDTH / 2) - (titleWidth / 2), SELECTION_SCREEN_TITLE_OFFSET, screenTitle, Display::CENTER);
    Display::drawFastHLine(0, TITLE_HIGHT, DISPLAY_WIDTH, BLACK);
}

void SelectionScreen::drawItems(String*& menuItems, uint16_t itemCount) {
    Display::setFont(OpenSans12);
    for (uint16_t i = 0; i < itemCount; i++) {
        uint16_t column = i / itemsPerColumn;
        uint16_t row = i % itemsPerColumn;
        if (columnPage != nullptr && columnPage[column] == currentPage) {
            uint32_t x = columnX[column] + COLUMN_TEXT_OFFSET;
            uint32_t y = ITEM_OFFSET + (ITEM_SPACING * (row + 1));
            Display::drawString(x, y, menuItems[i], Display::LEFT);
        }
    }
    drawnItemCount += itemCount;
} 

void SelectionScreen::clearMenuArea(){
    Rect_t rect = {0, TITLE_HIGHT, EPD_WIDTH, EPD_HEIGHT - TITLE_HIGHT};
    Display::fillRect(rect.x, rect.y, rect.width, rect.height, WHITE);
}

void SelectionScreen::computeLayout() {
    clearLayout();
    itemsPerColumn = (DISPLAY_HEIGHT - ITEM_OFFSET) / ITEM_SPACING;
    if (itemsPerColumn == 0) {
        itemsPerColumn = 1;
    }
    totalColumns = (screenItemCount + itemsPerColumn - 1) / itemsPerColumn;
    columnWidths = new uint16_t[totalColumns];
    columnX = new uint16_t[totalColumns];
    columnPage = new uint16_t[totalColumns];

    uint16_t maxColumnWidth = DISPLAY_WIDTH - COLUMN_LEFT_MARGIN - COLUMN_RIGHT_MARGIN;
    for (uint16_t column = 0; column < totalColumns; column++) {
        uint16_t startIndex = column * itemsPerColumn;
        uint16_t endIndex = startIndex + itemsPerColumn;
        if (endIndex > screenItemCount) {
            endIndex = screenItemCount;
        }
        uint16_t maxWidth = 0;
        for (uint16_t i = startIndex; i < endIndex; i++) {
            uint16_t width = Display::getTextWidth(screenItems[i]);
            if (width > maxWidth) {
                maxWidth = width;
            }
        }
        uint16_t columnWidth = COLUMN_TEXT_OFFSET + maxWidth + COLUMN_RIGHT_PADDING;
        if (columnWidth > maxColumnWidth) {
            columnWidth = maxColumnWidth;
        }
        columnWidths[column] = columnWidth;
    }

    uint16_t page = 0;
    uint16_t currentX = COLUMN_LEFT_MARGIN;
    uint16_t rightEdge = DISPLAY_WIDTH - COLUMN_RIGHT_MARGIN;
    for (uint16_t column = 0; column < totalColumns; column++) {
        if (currentX != COLUMN_LEFT_MARGIN && (currentX + columnWidths[column]) > rightEdge) {
            page++;
            currentX = COLUMN_LEFT_MARGIN;
        }
        columnX[column] = currentX;
        columnPage[column] = page;
        currentX += columnWidths[column];
    }
    totalPages = page + 1;
    if (currentPage >= totalPages) {
        currentPage = 0;
    }
    layoutDirty = false;
}

void SelectionScreen::setSelection(uint16_t newIndex) {
    if (newIndex >= screenItemCount) {
        return;
    }
    if (layoutDirty) {
        computeLayout();
    }
    selectionMode = SelectionMode::ITEMS;
    selectedIndex = newIndex;
    uint16_t column = selectedIndex / itemsPerColumn;
    if (columnPage != nullptr && columnPage[column] != currentPage) {
        currentPage = columnPage[column];
        draw();
        return;
    }
    if (screenItemCount == 0 || columnPage == nullptr) {
        return;
    }
    column = (currentSelection) / itemsPerColumn;
    uint16_t row = (currentSelection) % itemsPerColumn;
    if (selectedIndex != currentSelection && columnPage[column] == currentPage) {
        uint16_t x = columnX[column];
        uint16_t y = 9 + ITEM_OFFSET + (ITEM_SPACING * (row));
        Rect_t rect = {x, y, COLUMN_HIGHLIGHT_WIDTH, 32};
        Display::fillRect(rect.x, rect.y, rect.width, rect.height, WHITE);
        Display::pushPixels(rect,50,10);
    }
    column = (selectedIndex) / itemsPerColumn;
    row = (selectedIndex) % itemsPerColumn;
    if (columnPage[column] == currentPage) {
        uint16_t x = columnX[column] + TRIANGLE_X_OFFSET;
        uint16_t y = 15 + ITEM_OFFSET + (ITEM_SPACING * (row));
        Display::fillTriangle(x, y , x, y + 15, x + 16, y + 7, BLACK);
        currentSelection = selectedIndex;
        Display::update();
    }
}

void SelectionScreen::drawPageArrows(bool clearFirst) {
    uint8_t prevBoxColor = (selectionMode == SelectionMode::PREV_ARROW) ? BLACK : WHITE;
    uint8_t nextBoxColor = (selectionMode == SelectionMode::NEXT_ARROW) ? BLACK : WHITE;
    uint8_t prevArrowColor = (selectionMode == SelectionMode::PREV_ARROW) ? WHITE : BLACK;
    uint8_t nextArrowColor = (selectionMode == SelectionMode::NEXT_ARROW) ? WHITE : BLACK;
    Rect_t prevRect = {PREV_BOX_X, BOX_Y, BOX_WIDTH, BOX_HEIGHT};
    Rect_t nextRect = {NEXT_BOX_X, BOX_Y, BOX_WIDTH, BOX_HEIGHT};
    if (clearFirst) {
        Display::pushPixels(prevRect, 50, 10);
        Display::pushPixels(nextRect, 50, 10);
    }
    Display::fillRect(prevRect.x, prevRect.y, prevRect.width, prevRect.height, prevBoxColor);
    Display::fillRect(nextRect.x, nextRect.y, nextRect.width, nextRect.height, nextBoxColor);
    Display::fillTriangle(PREV_ARROW_X0, ARROW_Y0, PREV_ARROW_X1, ARROW_Y1, PREV_ARROW_X2, ARROW_Y2, prevArrowColor);
    Display::fillTriangle(NEXT_ARROW_X0, ARROW_Y0, NEXT_ARROW_X1, ARROW_Y1, NEXT_ARROW_X2, ARROW_Y2, nextArrowColor);
}

void SelectionScreen::drawPageNumber(bool clearFirst) {
    Display::setFont(OpenSans12);
    String pageLabel = String(currentPage + 1) + "/" + String(totalPages);
    if (clearFirst) {
        uint16_t pageLabelWidth = Display::getTextWidth(pageLabel);
        Rect_t rect = {PAGE_NUMBER_MARGIN, SELECTION_SCREEN_TITLE_OFFSET, pageLabelWidth, 16};
        Display::fillRect(rect.x, rect.y, rect.width, rect.height, WHITE);
        Display::pushPixels(rect, 50, 10);
    }
    Display::drawString(PAGE_NUMBER_MARGIN, SELECTION_SCREEN_TITLE_OFFSET, pageLabel, Display::CENTER);
}

void SelectionScreen::nextPage() {
    if (totalPages <= 1) {
        return;
    }
    if (currentPage < totalPages - 1) {
        currentPage++;
    } else {
        currentPage = 0;  // Wrap to first page
    }
    
    // Find first item on the new page
    for (uint16_t i = 0; i < screenItemCount; i++) {
        uint16_t column = i / itemsPerColumn;
        if (columnPage != nullptr && columnPage[column] == currentPage) {
            selectedIndex = i;
            currentSelection = i;
            break;
        }
    }
    draw();
}

void SelectionScreen::prevPage() {
    if (totalPages <= 1) {
        return;
    }
    if (currentPage > 0) {
        currentPage--;
    } else {
        currentPage = totalPages - 1;  // Wrap to last page
    }
    
    // Find first item on the new page
    for (uint16_t i = 0; i < screenItemCount; i++) {
        uint16_t column = i / itemsPerColumn;
        if (columnPage != nullptr && columnPage[column] == currentPage) {
            selectedIndex = i;
            currentSelection = i;
            break;
        }
    }
    draw();
}