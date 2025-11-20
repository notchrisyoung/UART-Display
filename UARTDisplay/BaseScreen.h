#ifndef BASE_SCREEN_H
#define BASE_SCREEN_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include "Display.h"

class BaseScreen {
public:
  virtual void init() = 0;
  virtual void update() = 0;
  virtual void draw() = 0;
  virtual void processUartData(JsonDocument& doc) = 0;
  virtual void upPress() = 0;
  virtual void downPress() = 0;
  virtual void leftPress() = 0;
  virtual void rightPress() = 0;
  virtual void selectPress() = 0;
  virtual void backPress() = 0;
};

#endif // BASE_SCREEN_H

