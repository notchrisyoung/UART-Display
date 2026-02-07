#ifndef MENU_SCREEN_H
#define MENU_SCREEN_H

#include "BaseScreen.h"


#define MENU_SCREEN_SPACING  75
#define MENU_SCREEN_OFFSET   50

class MenuScreen : public BaseScreen {
    public:
        MenuScreen();
        void init() override;
        void update() override;
        void draw() override;
        void processUartData(JsonDocument& doc) override;
        void sendUartData(const String& message) override;
        void setUARTcallback(void (*callback)(const String& message)) override;
        void highlight(int item);
        void clearHighlight();
        void upPress() override; 
        void downPress() override;
        void leftPress() override;
        void rightPress() override;
        void selectPress() override;
        void backPress() override;
    private:
        String* menuItems;
        String menuTitle;
        uint8_t menuItemCount;
        uint8_t selectedIndex;
        uint8_t currentSelection;
        void (*uartSendCallback)(const String&);
};

#endif // MENU_SCREEN_H