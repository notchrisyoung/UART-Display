#include "Buttons.h"

volatile bool Buttons::pressedStates[(int)Button::COUNT] = {false};
volatile bool Buttons::pressedActive[(int)Button::COUNT] = {false};
unsigned long Buttons::lastTimes[(int)Button::COUNT] = {0};
unsigned long Buttons::pressStartTimes[(int)Button::COUNT] = {0};
void (*Buttons::pressCallbacks[(int)Button::COUNT])() = {nullptr};
void (*Buttons::longPressCallbacks[(int)Button::COUNT])() = {nullptr};
unsigned long Buttons::longPressDuration = 1000; // default: 1 second

#define DEFINE_ISR(name, btn) \
    void IRAM_ATTR Buttons::handle##name##ISR() { handleISR(Button::btn); }

DEFINE_ISR(Up, UP)
DEFINE_ISR(Down, DOWN)
DEFINE_ISR(Left, LEFT)
DEFINE_ISR(Right, RIGHT)
DEFINE_ISR(Select, SELECT)
DEFINE_ISR(Back, BACK)
DEFINE_ISR(Home, HOME)

void Buttons::begin() {
    static const struct {
        Button button;
        uint8_t pin;
        void (*isr)();
    } buttonMap[] = {
        {Button::UP,     UP_BUTTON_PIN,     handleUpISR},
        {Button::DOWN,   DOWN_BUTTON_PIN,   handleDownISR},
        {Button::LEFT,   LEFT_BUTTON_PIN,   handleLeftISR},
        {Button::RIGHT,  RIGHT_BUTTON_PIN,  handleRightISR},
        {Button::SELECT, SELECT_BUTTON_PIN, handleSelectISR},
        {Button::BACK,   BACK_BUTTON_PIN,   handleBackISR},
        {Button::HOME,   HOME_BUTTON_PIN,   handleHomeISR}
    };

    for (const auto& b : buttonMap) {
        pinMode(b.pin, INPUT_PULLUP);
        attachInterrupt(digitalPinToInterrupt(b.pin), b.isr, FALLING);
        pressedStates[(int)b.button] = false;
        pressedActive[(int)b.button] = false;
        lastTimes[(int)b.button] = 0;
        pressStartTimes[(int)b.button] = 0;
    }
}

void IRAM_ATTR Buttons::handleISR(Button button) {
    unsigned long now = millis();
    int idx = (int)button;

    if (now - lastTimes[idx] < DEBOUNCE_MS) return;
    lastTimes[idx] = now;

    // If not already held, start timing the press
    if (!pressedActive[idx]) {
        pressedActive[idx] = true;
        pressStartTimes[idx] = now;
    }
    pressedStates[idx] = true;
}

void Buttons::update() {
    unsigned long now = millis();
    for (int i = 0; i < (int)Button::COUNT; i++) {
        if (pressedActive[i]) {
            // If button is still physically held down
            if (digitalRead(buttonPins[i]) == LOW) {
                // Check for long press
                if (pressStartTimes[i] &&
                    (now - pressStartTimes[i] > longPressDuration)) {
                    if (longPressCallbacks[i]) {
                        longPressCallbacks[i]();
                    }
                    pressedActive[i] = false;
                    pressStartTimes[i] = 0;
                }
            } else {
                // Button released → treat as short press
                if (pressedStates[i]) {
                    if (pressCallbacks[i] && (now - pressStartTimes[i]) <= longPressDuration) {
                        pressCallbacks[i]();
                    }
                }
                pressedActive[i] = false;
                pressedStates[i] = false;
                pressStartTimes[i] = 0;
            }
        }
    }
}

void Buttons::setButtonCallback(Button button, void (*onPress)()) {
    pressCallbacks[(int)button] = onPress;
}

void Buttons::setLongPressCallback(Button button, void (*onLongPress)()) {
    longPressCallbacks[(int)button] = onLongPress;
}

void Buttons::setLongPressDuration(unsigned long duration) {
    longPressDuration = duration;
}
