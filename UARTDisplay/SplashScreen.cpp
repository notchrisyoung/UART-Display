#include "SplashScreen.h"

SplashScreen::SplashScreen() {}
void SplashScreen::init() {}
void SplashScreen::draw() {
    Display::clear();
    Display::drawGrayScale((DISPLAY_WIDTH/2 - splashScreenWidth/2), 0, splashScreenWidth, splashScreenHeight, splashScreenData);
}
void SplashScreen::update() {}
void SplashScreen::processUartData(JsonDocument& doc) {}
void SplashScreen::upPress() {}
void SplashScreen::downPress() {}
void SplashScreen::leftPress() {}
void SplashScreen::rightPress() {}
void SplashScreen::selectPress() {}
void SplashScreen::backPress() {}