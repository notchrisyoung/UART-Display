#ifndef SELECTION_SCREEN_H
#define SELECTION_SCREEN_H

#include "BaseScreen.h"

#define SELECTION_SCREEN_TITLE_OFFSET 28
#define TITLE_HIGHT 39

#define ARROW_Y0 6
#define ARROW_Y1 13
#define ARROW_Y2 20
#define PREV_ARROW_X0 12
#define PREV_ARROW_X1 17
#define PREV_ARROW_X2 17
#define NEXT_ARROW_X0 316
#define NEXT_ARROW_X1 321
#define NEXT_ARROW_X2 321

#define BOX_Y 4
#define PREV_BOX_X 10
#define NEXT_BOX_X 310
#define BOX_WIDTH 7
#define BOX_HEIGHT 20

#define PAGE_NUMBER_MARGIN 50


#define ITEM_OFFSET 38
#define ITEM_SPACING 32

#define COLUMN_LEFT_MARGIN 3
#define COLUMN_RIGHT_MARGIN 3
#define COLUMN_TEXT_OFFSET 32
#define COLUMN_RIGHT_PADDING 8
#define COLUMN_HIGHLIGHT_WIDTH 31

#define TRIANGLE_X_OFFSET 12

enum class SelectionMode {
    ITEMS,
    PREV_ARROW,
    NEXT_ARROW
};

class SelectionScreen : public BaseScreen {
    public:
        SelectionScreen();
        ~SelectionScreen();
        void init() override;
        void update() override;
        void processUartData(JsonDocument& doc) override;
        void sendUartData(const String& message);
        void setUARTcallback(void (*callback)(const String& message)) override;
        void draw() override;
        void drawTitle();
        void drawItems(String*& menuItems, uint16_t itemCount);
        void clearMenuArea();
        void upPress() override; 
        void downPress() override;
        void leftPress() override;
        void rightPress() override;
        void selectPress() override;
        void backPress() override;
    private:
        String* screenItems;
        String screenTitle;
        uint16_t drawnItemCount;
        uint16_t screenItemCount;
        uint16_t selectedIndex;
        uint16_t currentSelection;
        uint16_t itemsPerColumn;
        uint16_t totalColumns;
        uint16_t currentPage;
        uint16_t totalPages;
        uint16_t* columnWidths;
        uint16_t* columnX;
        uint16_t* columnPage;
        SelectionMode selectionMode;
        void computeLayout();
        void drawHighlightedArrow(uint16_t x, uint16_t y, bool pointRight);
        void setSelection(uint16_t newIndex);
        void clearLayout();
        void nextPage();
        void prevPage();
        void (*uartSendCallback)(const String&);
};

#endif // SELECTION_SCREEN_H