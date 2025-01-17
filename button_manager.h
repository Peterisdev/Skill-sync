#ifndef BUTTON_MANAGER_H
#define BUTTON_MANAGER_H

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
    const uint8_t UP_PIN;
    const uint8_t DOWN_PIN;
    const uint8_t SELECT_PIN;
    
    // Button states
    bool lastButtonStates[3] = {HIGH, HIGH, HIGH};
    bool currentButtonStates[3] = {HIGH, HIGH, HIGH};
    bool buttonPressed[3] = {false, false, false};
    unsigned long lastDebounceTime[3] = {0, 0, 0};
    unsigned long buttonPressTime[3] = {0, 0, 0};
    bool buttonActive[3] = {false, false, false};
    
    // Timing constants
    const unsigned long DEBOUNCE_DELAY = 150;    // Reduced for better responsiveness
    const unsigned long REPEAT_DELAY = 800;      // Longer initial delay
    const unsigned long REPEAT_INTERVAL = 300;   // Slower repeat rate
    const unsigned long MIN_PRESS_TIME = 50;     // Minimum time for a valid press

public:
    ButtonManager() : 
        UP_PIN(BUTTON_UP),
        DOWN_PIN(BUTTON_DOWN),
        SELECT_PIN(BUTTON_SELECT) {}

    void begin() {
        pinMode(UP_PIN, INPUT_PULLUP);
        pinMode(DOWN_PIN, INPUT_PULLUP);
        pinMode(SELECT_PIN, INPUT_PULLUP);
    }

    void update() {
        unsigned long currentTime = millis();
        
        // Update each button
        updateButton(0, UP_PIN, currentTime);
        updateButton(1, DOWN_PIN, currentTime);
        updateButton(2, SELECT_PIN, currentTime);
    }

    bool isUpPressed() {
        return checkAndClearButton(0);
    }

    bool isDownPressed() {
        return checkAndClearButton(1);
    }

    bool isSelectPressed() {
        return checkAndClearButton(2);
    }

private:
    void updateButton(int index, uint8_t pin, unsigned long currentTime) {
        bool reading = !digitalRead(pin);

        // If reading changed, reset debounce timer
        if (reading != lastButtonStates[index]) {
            lastDebounceTime[index] = currentTime;
        }

        // If enough time has passed for debounce
        if ((currentTime - lastDebounceTime[index]) > DEBOUNCE_DELAY) {
            // If button state has changed
            if (reading != currentButtonStates[index]) {
                currentButtonStates[index] = reading;
                
                // Button pressed
                if (reading == true) {
                    if (!buttonActive[index]) {
                        buttonPressTime[index] = currentTime;
                        buttonActive[index] = true;
                    }
                }
                // Button released
                else {
                    if (buttonActive[index]) {
                        if ((currentTime - buttonPressTime[index]) > MIN_PRESS_TIME) {
                            buttonPressed[index] = true;
                        }
                        buttonActive[index] = false;
                    }
                }
            }
            // Handle button repeat for held buttons
            else if (reading == true && buttonActive[index]) {
                unsigned long holdTime = currentTime - buttonPressTime[index];
                if (holdTime > REPEAT_DELAY) {
                    if ((currentTime - lastDebounceTime[index]) > REPEAT_INTERVAL) {
                        buttonPressed[index] = true;
                        lastDebounceTime[index] = currentTime;
                    }
                }
            }
        }

        lastButtonStates[index] = reading;
    }

    bool checkAndClearButton(int index) {
        if (buttonPressed[index]) {
            buttonPressed[index] = false;
            return true;
        }
        return false;
    }
};

extern ButtonManager buttonManager;

#endif