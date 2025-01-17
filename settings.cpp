#include "settings.h"

void SettingsManager::saveSettings() {
    File f = SPIFFS.open("/settings.json", "w");
    if (!f) return;
    
    StaticJsonDocument<1024> doc;
    JsonObject network = doc.createNestedObject("network");
    JsonObject display = doc.createNestedObject("display");
    JsonObject attack = doc.createNestedObject("attack");
    
    // Save network settings
    network["hostname"] = networkSettings.hostname;
    network["txPower"] = networkSettings.txPower;
    network["defaultChannel"] = networkSettings.defaultChannel;
    network["hiddenAP"] = networkSettings.hiddenAP;
    
    // Save display settings
    display["contrast"] = displaySettings.contrast;
    display["flipDisplay"] = displaySettings.flipDisplay;
    display["screenTimeout"] = displaySettings.screenTimeout;
    
    // Save attack settings
    attack["deauthPacketsPerBurst"] = attackSettings.deauthPacketsPerBurst;
    attack["channelHopping"] = attackSettings.channelHopping;
    attack["beaconInterval"] = attackSettings.beaconInterval;
    
    serializeJson(doc, f);
    f.close();
}

void SettingsManager::loadSettings() {
    File f = SPIFFS.open("/settings.json", "r");
    if (!f) {
        resetToDefaults();
        return;
    }
    
    StaticJsonDocument<1024> doc;
    DeserializationError error = deserializeJson(doc, f);
    if (error) {
        resetToDefaults();
        return;
    }
    
    // Load network settings
    networkSettings.hostname = doc["network"]["hostname"] | "WiFi-Watchdog";
    networkSettings.txPower = doc["network"]["txPower"] | 20;
    networkSettings.defaultChannel = doc["network"]["defaultChannel"] | 1;
    networkSettings.hiddenAP = doc["network"]["hiddenAP"] | false;
    
    // Load display settings
    displaySettings.contrast = doc["display"]["contrast"] | 128;
    displaySettings.flipDisplay = doc["display"]["flipDisplay"] | false;
    displaySettings.screenTimeout = doc["display"]["screenTimeout"] | 60;
    
    // Load attack settings
    attackSettings.deauthPacketsPerBurst = doc["attack"]["deauthPacketsPerBurst"] | 10;
    attackSettings.channelHopping = doc["attack"]["channelHopping"] | true;
    attackSettings.beaconInterval = doc["attack"]["beaconInterval"] | 100;
    
    f.close();
}

void SettingsManager::resetToDefaults() {
    networkSettings = NetworkSettings();
    displaySettings = DisplaySettings();
    attackSettings = AttackSettings();
    saveSettings();
} 