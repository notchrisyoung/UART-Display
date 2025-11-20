#ifndef SELECTION_SCREEN_H
#define SELECTION_SCREEN_H

#include "BaseScreen.h"



#define MENU_SCREEN_SPACING  75
#define MENU_SCREEN_OFFSET   50

class SelectionScreen : public BaseScreen {
    public:
        SelectionScreen();
        void init() override;
        void update() override;
        void draw() override;
        void processUartData(JsonDocument& doc) override;
        void sendUartData(const String& message);
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
};

#endif // SELECTION_SCREEN_H