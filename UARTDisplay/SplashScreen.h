#ifndef SPLASH_SCREEN_H
#define SPLASH_SCREEN_H

#include "BaseScreen.h"
#include "SplashData.h"

class SplashScreen : public BaseScreen {
    public:
        SplashScreen();
        void init() override;
        void draw() override;
        void update() override;
        void processUartData(JsonDocument& doc) override;
        void sendUartData(const String& message) override;
        void setUARTcallback(void (*callback)(const String& message)) override;
        void upPress() override;
        void downPress() override;
        void leftPress() override;
        void rightPress() override;
        void selectPress() override;
        void backPress() override;
};

#endif // SPLASH_SCREEN_H