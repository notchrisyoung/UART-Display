#ifndef BUTTONS_H
#define BUTTONS_H

#include <Arduino.h>
#include "epd_driver.h"

// --- Hardware pin definitions ---
#define UP_BUTTON_PIN     35
#define DOWN_BUTTON_PIN   12
#define LEFT_BUTTON_PIN   13
#define RIGHT_BUTTON_PIN  14
#define SELECT_BUTTON_PIN 34
#define BACK_BUTTON_PIN   15
#define HOME_BUTTON_PIN   39
// --------------------------------

#define DEBOUNCE_DELAY    50 // milliseconds
#define LONG_PRESS_DELAY  1000 // milliseconds
#define LONG_PRESS_THRESHOLD 500 // milliseconds


enum class Button {
    UP,
    DOWN,
    LEFT,
    RIGHT,
    SELECT,
    BACK,
    HOME,
    COUNT
};

// Pin mapping array to get correct pin for each button
static const uint8_t buttonPins[] = {
    UP_BUTTON_PIN,     
    DOWN_BUTTON_PIN,   
    LEFT_BUTTON_PIN,   
    RIGHT_BUTTON_PIN,  
    SELECT_BUTTON_PIN, 
    BACK_BUTTON_PIN,   
    HOME_BUTTON_PIN    
};

class Buttons {
public:

    static void begin();
    static void update();

    // Register callbacks
    static void setButtonCallback(Button button, void (*onPress)());
    static void setLongPressCallback(Button button, void (*onLongPress)());

    // Adjust long-press threshold (milliseconds)
    static void setLongPressDuration(unsigned long duration);

private:
    static constexpr unsigned long DEBOUNCE_MS = 50;

    static volatile bool pressedStates[(int)Button::COUNT];
    static volatile bool pressedActive[(int)Button::COUNT]; // true while held
    static unsigned long lastTimes[(int)Button::COUNT];
    static unsigned long pressStartTimes[(int)Button::COUNT];

    static void (*pressCallbacks[(int)Button::COUNT])();
    static void (*longPressCallbacks[(int)Button::COUNT])();

    static unsigned long longPressDuration;

    static void IRAM_ATTR handleISR(Button button);

    static void IRAM_ATTR handleUpISR();
    static void IRAM_ATTR handleDownISR();
    static void IRAM_ATTR handleLeftISR();
    static void IRAM_ATTR handleRightISR();
    static void IRAM_ATTR handleSelectISR();
    static void IRAM_ATTR handleBackISR();
    static void IRAM_ATTR handleHomeISR();
};

#endif