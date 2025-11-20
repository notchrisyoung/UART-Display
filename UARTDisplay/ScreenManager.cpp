#include "ScreenManager.h"
#include <String.h>

ScreenManager::ScreenManager() {}
ScreenManager* ScreenManager::instance = nullptr;
void upPressCallback()    { if(ScreenManager::instance) ScreenManager::instance->upPress(); }
void downPressCallback()  { if(ScreenManager::instance) ScreenManager::instance->downPress(); }
void leftPressCallback()  { if(ScreenManager::instance) ScreenManager::instance->leftPress(); }
void rightPressCallback() { if(ScreenManager::instance) ScreenManager::instance->rightPress(); }
void selectPressCallback() { if(ScreenManager::instance) ScreenManager::instance->selectPress(); }
void backPressCallback()   { if(ScreenManager::instance) ScreenManager::instance->backPress(); }
void homePressCallback()   { if(ScreenManager::instance) ScreenManager::instance->homePress(); }

void ScreenManager::init() {
    instance = this; // assign global pointer
    currentScreen = new SplashScreen();
    currentScreenType = ScreenType::SPLASH_SCREEN;
    currentScreen->draw();
    Buttons::setButtonCallback(Button::UP, upPressCallback);
    Buttons::setButtonCallback(Button::DOWN, downPressCallback);
    Buttons::setButtonCallback(Button::LEFT, leftPressCallback);
    Buttons::setButtonCallback(Button::RIGHT, rightPressCallback);
    Buttons::setButtonCallback(Button::SELECT, selectPressCallback);
    Buttons::setButtonCallback(Button::BACK, backPressCallback);
    Buttons::setButtonCallback(Button::HOME, homePressCallback);
}

void ScreenManager::update() {
    if (currentScreen) {
        currentScreen->update();
    }
}

void ScreenManager::switchToScreen(ScreenType newScreen) {
    Serial.println("Switching screens");
    if(currentScreenType == newScreen) {
         return; // No need to switch if the screen is already the same
    }
    switch (newScreen) {
        case ScreenType::SPLASH_SCREEN:
            currentScreen = new SplashScreen();
            break;
        case ScreenType::MAIN_MENU:
            currentScreen = new MenuScreen();
            break;
        case ScreenType::SELECTION_MENU:
            currentScreen = new SelectionScreen();
            break;
        case ScreenType::INFO_SCREEN:
            currentScreen = new InfoScreen();
            break;
        case ScreenType::LOADING_SCREEN:
            currentScreen = new LoadingScreen();
            break;
    }
    currentScreen->init();
}

void ScreenManager::processUartData(uint8_t* data, uint8_t len) {
    Serial.println("Processing UART Data");
    // Process the message and update the current screen accordingly
    String message = String((char*)data);
    DeserializationError error = deserializeJson(doc, message);
    if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return;
    }
    String type = doc["type"];
    if (type == "screen") {
        String id = doc["id"];
        if (id == "splash") {
            switchToScreen(ScreenType::SPLASH_SCREEN);
        } else if (id == "main_menu") {
            switchToScreen(ScreenType::MAIN_MENU);
        } else if (id == "selection_menu") {
            switchToScreen(ScreenType::SELECTION_MENU);
        } else if (id == "loading_screen") {
            switchToScreen(ScreenType::LOADING_SCREEN);
        } else if (id == "info_screen") {
            switchToScreen(ScreenType::INFO_SCREEN);
        }
        if (currentScreen) {
            currentScreen->processUartData(doc);
        }
    }
    else if (type == "data") {
        if (currentScreen) {
            currentScreen->processUartData(doc);
        }
    }
}

void ScreenManager::setUARTcallback(void (*callback)(const String& message)) {
    // Set the UART callback to send data from screens
    // This function can be used to set a callback for sending data over UART
}

void ScreenManager::sendUartData(const String& message) {
    // Send data over UART if needed
    // Add logic to send data over UART
}

void ScreenManager::upPress() {
    if (currentScreen) {
        currentScreen->upPress();
    }
}

void ScreenManager::downPress() {
    if (currentScreen) {
        currentScreen->downPress();
    }
}

void ScreenManager::leftPress() {
    if (currentScreen) {
        currentScreen->leftPress();
    }
}

void ScreenManager::rightPress() {
    if (currentScreen) {
        currentScreen->rightPress();
    }
}

void ScreenManager::selectPress() {
    if (currentScreen) {
        currentScreen->selectPress();
    }
}

void ScreenManager::backPress() {
    if (currentScreen) {
        currentScreen->backPress();
    }
}

void ScreenManager::homePress() {
    Serial.println("Home Pressed");
    // Send home to controller
}