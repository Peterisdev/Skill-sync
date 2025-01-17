#ifndef SETTINGS_H
#define SETTINGS_H

#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include "storage.h"

struct DisplaySettings {
    uint16_t timeout = 30000;
    uint8_t brightness = 128;
    uint8_t contrast = 128;
    bool invertDisplay = false;
    bool flipDisplay = false;
    bool showLogo = true;
    uint16_t screenTimeout = 60;
};

struct AttackSettings {
    uint8_t deauthPacketsPerBurst = 10;
    bool channelHopping = true;
    uint16_t beaconInterval = 100;
    uint8_t maxBeacons = 20;
    uint8_t maxProbes = 100;
    uint16_t rickrollSpeed = 2000;
    bool randomizeMac = true;
};

struct NetworkSettings {
    String hostname = "WiFi-Watchdog";
    String apPassword = "";
    uint8_t apChannel = 1;
    bool hiddenAP = false;
    IPAddress apIP;
    IPAddress apGateway;
    IPAddress apSubnet;
    uint8_t txPower = 20;
    uint8_t defaultChannel = 1;

    NetworkSettings() {
        apIP = IPAddress(192, 168, 4, 1);
        apGateway = IPAddress(192, 168, 4, 1);
        apSubnet = IPAddress(255, 255, 255, 0);
    }
};

class SettingsManager {
private:
    NetworkSettings networkSettings;
    DisplaySettings displaySettings;
    AttackSettings attackSettings;
    bool settingsChanged = false;

public:
    NetworkSettings& getNetworkSettings() { return networkSettings; }
    DisplaySettings& getDisplaySettings() { return displaySettings; }
    AttackSettings& getAttackSettings() { return attackSettings; }
    
    const NetworkSettings& getNetworkSettingsConst() const { return networkSettings; }
    const DisplaySettings& getDisplaySettingsConst() const { return displaySettings; }
    const AttackSettings& getAttackSettingsConst() const { return attackSettings; }

    void saveSettings();
    void loadSettings();
    void resetToDefaults();

    void begin() {
        loadSettings();
    }
};

extern SettingsManager settingsManager;

#endif