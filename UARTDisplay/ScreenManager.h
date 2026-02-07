#ifndef SCREEN_MANAGER_H
#define SCREEN_MANAGER_H

#include "BaseScreen.h"
#include "Buttons.h"
#include "MenuScreen.h"
#include "SplashScreen.h"
#include "LoadingScreen.h"
#include "InfoScreen.h"
#include "SelectionScreen.h"
#include "UARTProtocol.h"
#include <ArduinoJson.h>


class ScreenManager {
public:
  static ScreenManager* instance; 
  enum ScreenType {
		SPLASH_SCREEN,
		MAIN_MENU,
		SELECTION_MENU,
		INFO_SCREEN,
		LOADING_SCREEN
  };
  ScreenManager();
  void init();
	void update();
	void switchToScreen(ScreenType newScreen);
	//void setButtonCallback(Button button, void (*callback)());
	void setUARTcallback(void (*callback)(const String& message));
	void processUartData(uint8_t* data, uint8_t len);
	void sendUartData(const String& message);
	void upPress();
	void downPress();
	void leftPress();
	void rightPress();
	void selectPress();
	void backPress();
	void homePress();

private:
  	JsonDocument doc;
  	BaseScreen* currentScreen;
	ScreenType currentScreenType;
	void (*uartSendCallback)(const String& message);
};

#endif // SCREEN_MANAGER_H
