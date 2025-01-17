#ifndef UI_H
#define UI_H

#include <U8g2lib.h>
#include <vector>
#include "wifi_structs.h"
#include "attacks.h"
#include "templates.h"
#include "settings_manager.h"

class UIManager {
private:
    U8G2* display;
    int menuPosition = 0;
    int settingsMenuPosition = 0;
    int settingsSubMenuPosition = 0;
    int attackMenuPosition = 0;
    SettingsManager* settingsManager;
    size_t selectedNetwork = 0;
    std::vector<WiFiNetwork> networks;

    // Helper methods
    void drawTitleBar(const char* title);
    void drawMenuItem(const char* text, int y, bool selected);
    String getCurrentSettingValue(MenuState state);

    // Menu item name getters
    const char* getSettingsMenuName(int pos) {
        static const char* items[] = {
            "Attack Settings",
            "Network Settings",
            "Display Settings",
            "Reset Settings"
        };
        return items[pos % 4];
    }

    const char* getAttackSettingName(int pos) {
        static const char* items[] = {
            "Deauth Packets",
            "Channel Hopping",
            "Beacon Interval"
        };
        return items[pos % 3];
    }

    const char* getNetworkSettingName(int pos) {
        static const char* items[] = {
            "TX Power",
            "Channel",
            "Hidden AP"
        };
        return items[pos % 3];
    }

    const char* getDisplaySettingName(int pos) {
        static const char* items[] = {
            "Contrast",
            "Flip Display",
            "Screen Timeout"
        };
        return items[pos % 3];
    }

    static const int MAIN_MENU_ITEMS = 3;      // Attack, Settings, Credentials
    static const int ATTACK_MENU_ITEMS = 5;    // Different attack types
    static const int SETTINGS_MENU_ITEMS = 3;  // Attack, Network, Display settings

public:
    UIManager(U8G2* disp) : display(disp) {}
    void begin();
    void setSettingsManager(SettingsManager* manager) { settingsManager = manager; }
    
    // Menu navigation
    void nextMenuItem() { menuPosition = (menuPosition + 1) % 4; }
    void nextAttackMenuItem() { attackMenuPosition = (attackMenuPosition + 1) % 6; }
    void nextSettingsMenuItem() { settingsMenuPosition = (settingsMenuPosition + 1) % 4; }
    void nextSettingsSubMenuItem() { settingsSubMenuPosition = (settingsSubMenuPosition + 1) % 3; }
    
    void previousMenuItem() { 
        menuPosition = (menuPosition - 1 + MAIN_MENU_ITEMS) % MAIN_MENU_ITEMS; 
    }
    
    void previousAttackMenuItem() { 
        attackMenuPosition = (attackMenuPosition - 1 + ATTACK_MENU_ITEMS) % ATTACK_MENU_ITEMS; 
    }
    
    void previousSettingsMenuItem() { 
        settingsMenuPosition = (settingsMenuPosition - 1 + SETTINGS_MENU_ITEMS) % SETTINGS_MENU_ITEMS; 
    }
    
    // Getters
    int getMenuPosition() const { return menuPosition; }
    int getAttackMenuPosition() const { return attackMenuPosition; }
    int getSettingsMenuPosition() const { return settingsMenuPosition; }
    int getSettingsSubMenuPosition() const { return settingsSubMenuPosition; }
    
    // Drawing methods
    void drawMainMenu();
    void drawAttackMenu(AttackType currentAttack, bool isRunning = false);
    void drawNetworkScan();
    void drawNetworkList(const std::vector<WiFiNetwork>& networks, int selected);
    void drawAttackStatus(const String& status, int progress, AttackType currentAttack);
    void drawSettingsMenu();
    void drawAttackSettings();
    void drawNetworkSettings();
    void drawDisplaySettings();
    void drawCredentials(const String& creds, int scrollPos);
    void drawTemplateSelect(PortalTemplate currentTemplate);
    void showBootSplash();

    // Add these methods for network list navigation
    void nextNetwork() { 
        if (!networks.empty()) {
            selectedNetwork = (selectedNetwork + 1) % networks.size(); 
        }
    }
    
    void previousNetwork() {
        if (!networks.empty()) {
            selectedNetwork = (selectedNetwork - 1 + networks.size()) % networks.size();
        }
    }

    String getAttackName(AttackType type) const;
};

#endif 