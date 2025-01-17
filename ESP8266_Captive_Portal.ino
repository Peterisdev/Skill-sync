#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <U8g2lib.h>
#include <vector>
#include <algorithm>
#include "config.h"
#include "buttons.h"
#include "storage.h"
#include "templates.h"
#include "ui.h"
#include "attack_manager.h"
#include "wifi_structs.h"
#include "menu_state.h"

// Global objects
U8G2_SH1106_128X64_NONAME_F_HW_I2C display(U8G2_R0, U8X8_PIN_NONE, OLED_SCL, OLED_SDA);
ESP8266WebServer webServer(WEB_PORT);
DNSServer dnsServer;
ButtonManager buttonManager;
StorageManager storageManager;
UIManager uiManager(&display);
AttackManager attackManager;
SettingsManager settingsManager;

// Global state
MenuState currentState = BOOT_SPLASH;
unsigned long lastUpdate = 0;
unsigned long lastDisplayUpdate = 0;
const unsigned long UPDATE_INTERVAL = 1000;
const unsigned long DISPLAY_UPDATE_INTERVAL = 1000;

void setup() {
    // Initialize Serial for debugging
    Serial.begin(115200);
    Serial.println("\nInitializing...");

    // Initialize display
    display.begin();
    display.setContrast(128);
    display.clearBuffer();
    
    // Initialize buttons
    buttonManager.begin();
    Serial.println("Buttons initialized");

    // Initialize storage
    if (!SPIFFS.begin()) {
        Serial.println("Failed to mount file system");
    }
    storageManager.begin();
    Serial.println("Storage initialized");

    // Initialize settings
    settingsManager.begin();
    Serial.println("Settings initialized");

    // Initialize WiFi
    WiFi.mode(WIFI_OFF);
    WiFi.disconnect();
    delay(100);
    WiFi.mode(WIFI_STA);
    Serial.println("WiFi initialized");

    // Initialize web server
    webServer.begin();
    Serial.println("Web server initialized");

    // Initialize DNS server
    dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());
    Serial.println("DNS server initialized");

    // Show boot splash screen
    uiManager.showBootSplash();
    delay(2000);  // Show splash for 2 seconds
    currentState = MAIN_MENU;

    Serial.println("Initialization complete!");
}

// Function declarations
void handleScrollButton();
void handleBackButton();
void handleSelectButton(ButtonAction action);
void updateDisplay();

void loop() {
    static unsigned long lastButtonDebugTime = 0;
    static unsigned long currentTime = 0;
    static bool buttonHandled = false;
    
    currentTime = millis();
    
    // Update button states
    buttonManager.update();

    // Handle button presses with rate limiting
    if (currentTime - lastButtonDebugTime > 50) {  // 50ms between button checks
        if (!buttonHandled) {  // Only handle button if not already handled
            if (buttonManager.isUpPressed()) {
                handleScrollButton();
                Serial.println("UP button pressed");
                buttonHandled = true;
                lastButtonDebugTime = currentTime;
            }
            else if (buttonManager.isDownPressed()) {
                handleBackButton();
                Serial.println("DOWN button pressed");
                buttonHandled = true;
                lastButtonDebugTime = currentTime;
            }
            else if (buttonManager.isSelectPressed()) {
                handleSelectButton(SINGLE_CLICK);
                Serial.println("SELECT button pressed");
                buttonHandled = true;
                lastButtonDebugTime = currentTime;
            }
        }
    } else {
        buttonHandled = false;  // Reset button handled flag after delay
    }

    // Update attack status if running
    if (attackManager.isRunning()) {
        attackManager.update();
        updateDisplay();
    }

    // Regular display updates
    if (currentTime - lastDisplayUpdate > DISPLAY_UPDATE_INTERVAL) {
        updateDisplay();
        lastDisplayUpdate = currentTime;
    }

    yield();
}

void handleScrollButton() {
    static unsigned long lastScrollTime = 0;
    unsigned long currentTime = millis();
    
    // Add time-based throttling for menu updates
    if (currentTime - lastScrollTime > 200) {  // 200ms between menu position updates
        switch (currentState) {
            case MAIN_MENU:
                uiManager.nextMenuItem();
                Serial.println("Main menu: " + String(uiManager.getMenuPosition()));
                break;
            case ATTACK_MENU:
                uiManager.nextAttackMenuItem();
                Serial.println("Attack menu: " + String(uiManager.getAttackMenuPosition()));
                break;
            case SETTINGS_MENU:
                uiManager.nextSettingsMenuItem();
                Serial.println("Settings menu: " + String(uiManager.getSettingsMenuPosition()));
                break;
        }
        updateDisplay();
        lastScrollTime = currentTime;
    }
}

void handleBackButton() {
    static unsigned long lastBackTime = 0;
    unsigned long currentTime = millis();
    
    // Add time-based throttling for back button
    if (currentTime - lastBackTime > 200) {
        switch (currentState) {
            case ATTACK_MENU:
                if (attackManager.isRunning()) {
                    attackManager.stopAttack();
                }
                currentState = MAIN_MENU;
                break;
            case SETTINGS_MENU:
                currentState = MAIN_MENU;
                break;
            case ATTACK_RUNNING:
                attackManager.stopAttack();
                currentState = ATTACK_MENU;
                break;
        }
        updateDisplay();
        lastBackTime = currentTime;
    }
}

void handleSelectButton(ButtonAction action) {
    static unsigned long lastSelectTime = 0;
    unsigned long currentTime = millis();
    
    // Add time-based throttling for select button
    if (currentTime - lastSelectTime > 200) {
        switch (currentState) {
            case MAIN_MENU:
                switch (uiManager.getMenuPosition()) {
                    case 0: 
                        currentState = ATTACK_MENU; 
                        break;
                    case 1: 
                        currentState = SETTINGS_MENU; 
                        break;
                }
                break;
            case ATTACK_MENU:
                if (action == SINGLE_CLICK) {
                    AttackType selected = static_cast<AttackType>(uiManager.getAttackMenuPosition());
                    if (attackManager.isRunning()) {
                        attackManager.stopAttack();
                        currentState = ATTACK_MENU;
                    } else {
                        attackManager.selectAttack(selected);
                        attackManager.toggleAttack();
                        currentState = ATTACK_RUNNING;
                    }
                }
                break;
        }
        updateDisplay();
        lastSelectTime = currentTime;
    }
}

void updateDisplay() {
    switch (currentState) {
        case BOOT_SPLASH:
            uiManager.showBootSplash();
            break;
        case MAIN_MENU:
            uiManager.drawMainMenu();
            break;
        case ATTACK_MENU:
            uiManager.drawAttackMenu(attackManager.getSelectedAttack(), attackManager.isRunning());
            break;
        case ATTACK_RUNNING:
            uiManager.drawAttackStatus("Running", attackManager.getAttackProgress(), attackManager.getSelectedAttack());
            break;
        case SETTINGS_MENU:
            uiManager.drawSettingsMenu();
            break;
    }
}

// ... rest of your code ...