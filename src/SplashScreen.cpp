#include "SplashScreen.h"

SplashScreen::SplashScreen() {}
void SplashScreen::init() {
    draw();
}
void SplashScreen::draw() {
    Display::clear();
    Display::drawGrayScale((DISPLAY_WIDTH/2 - splashScreenWidth/2), 0, splashScreenWidth, splashScreenHeight, splashScreenData);
}
void SplashScreen::update() {}
void SplashScreen::processUartData(JsonDocument& doc) {}
void SplashScreen::sendUartData(const String& message) {}
void SplashScreen::setUARTcallback(void (*callback)(const String& message)) {}
void SplashScreen::upPress() {}
void SplashScreen::downPress() {}
void SplashScreen::leftPress() {}
void SplashScreen::rightPress() {}
void SplashScreen::selectPress() {}
void SplashScreen::backPress() {}