#ifndef SELECTION_SCREEN_H
#define SELECTION_SCREEN_H

#include "BaseScreen.h"



#define SELECTION_SCREEN_SPACING  75
#define SELECTION_SCREEN_TITLE_OFFSET 28
#define TITLE_HIGHT 39
#define ITEM_PER_COLUMN 15
#define COLUMN_WIDTH 125
#define ITEM_OFFSET 38
#define ITEM_SPACING 32


class SelectionScreen : public BaseScreen {
    public:
        SelectionScreen();
        void init() override;
        void update() override;
        void processUartData(JsonDocument& doc) override;
        void sendUartData(const String& message);
        void setUARTcallback(void (*callback)(const String& message)) override;
        void draw() override;
        void drawTitle();
        void drawItems(String*& menuItems, uint16_t itemCount);
        void highlight(uint16_t item);
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
        void (*uartSendCallback)(const String&);
};

#endif // SELECTION_SCREEN_H