#ifndef BUTTONS_H
#define BUTTONS_H

#include <Arduino.h>
#include "config.h"
#include "button_actions.h"

enum ButtonState {
    IDLE,
    PRESSED,
    HELD,
    RELEASED
};

class ButtonManager {
private:
    static const uint8_t NUM_BUTTONS = 3;
    uint8_t buttonPins[NUM_BUTTONS];
    bool buttonStates[NUM_BUTTONS];
    bool lastButtonStates[NUM_BUTTONS];
    unsigned long lastDebounceTime[NUM_BUTTONS];
    unsigned long lastPressTime = 0;
    uint8_t clickCount = 0;
    static const unsigned long DEBOUNCE_DELAY = 50;
    static const unsigned long DOUBLE_CLICK_TIME = 300;

public:
    void begin() {
        buttonPins[0] = BUTTON_UP;
        buttonPins[1] = BUTTON_DOWN;
        buttonPins[2] = BUTTON_SELECT;
        
        for(uint8_t i = 0; i < NUM_BUTTONS; i++) {
            pinMode(buttonPins[i], INPUT_PULLUP);
            buttonStates[i] = false;
            lastButtonStates[i] = false;
            lastDebounceTime[i] = 0;
        }
    }

    bool update() {
        bool changed = false;
        for(uint8_t i = 0; i < NUM_BUTTONS; i++) {
            bool reading = !digitalRead(buttonPins[i]);
            
            if (reading != lastButtonStates[i]) {
                lastDebounceTime[i] = millis();
            }
            
            if ((millis() - lastDebounceTime[i]) > DEBOUNCE_DELAY) {
                if (reading != buttonStates[i]) {
                    buttonStates[i] = reading;
                    changed = true;
                }
            }
            
            lastButtonStates[i] = reading;
        }
        return changed;
    }

    bool isPressed(uint8_t button) const {
        return button < NUM_BUTTONS && buttonStates[button];
    }

    bool isUpPressed() const { return buttonStates[0]; }
    bool isDownPressed() const { return buttonStates[1]; }
    bool isSelectPressed() const { return buttonStates[2]; }
    
    // Long press detection for any button
    bool isLongPress(uint8_t button) const {
        return buttonStates[button] && 
               (millis() - lastDebounceTime[button] > 1000);
    }
};

extern ButtonManager buttonManager;

#endif 