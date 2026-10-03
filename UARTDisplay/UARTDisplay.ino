#include <Arduino.h>
#include <String.h>
#include "ScreenManager.h"
#include "UARTProtocol.h"

ScreenManager sm;
UARTProtocol uart;

void setup() {
    // Initialize UART communication
    uart.begin(115200);
    delay(1000);
    // Initialize the display
    Display::begin();
    // Initialize the buttons
    Buttons::begin();
    // Initialize the screen manager
    sm.init();
    uart.setCallback([](const uint8_t* data, uint8_t len) {
        sm.processUartData(const_cast<uint8_t*>(data), len);
    });
    sm.setUARTcallback([](const String& message) {
        uart.sendMessage((uint8_t*)message.c_str(), message.length());
    });
}

void loop() {
    Buttons::update();
    sm.update();
    uart.process();
    delay(10);
}
