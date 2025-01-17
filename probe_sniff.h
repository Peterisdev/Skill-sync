#ifndef PROBE_SNIFF_H
#define PROBE_SNIFF_H

#include <ESP8266WiFi.h>
#include <map>
#include "wifi_structs.h"

class ProbeSniffAttack {
private:
    bool running = false;
    std::map<String, std::vector<String>> probeRequests;
    std::map<String, int32_t> signalStrengths;
    std::map<String, unsigned long> lastSeenTimes;
    unsigned long startTime = 0;
    uint8_t channel = 1;
    unsigned long lastChannelSwitch = 0;
    const unsigned long CHANNEL_HOP_INTERVAL = 500;
    
    // Settings
    uint16_t maxProbes;
    bool saveToStorage;
    bool filterDuplicates = true;
    int8_t minSignalStrength = -85;
    unsigned long deviceTimeout = 60000; // 60 seconds

    static void promiscuousCallback(uint8_t *buf, uint16_t len) {
        if (len < 28) return;
        
        // Get frame control field
        uint16_t frameControl = ((uint16_t)buf[0] | ((uint16_t)buf[1] << 8));
        
        // Check if it's a probe request
        if ((frameControl & 0xFC) != 0x40) return;
        
        // Extract MAC address (source)
        char mac[18];
        snprintf(mac, sizeof(mac), "%02X:%02X:%02X:%02X:%02X:%02X",
                buf[22], buf[23], buf[24], buf[25], buf[26], buf[27]);
                
        // Extract SSID if present
        if (len < 38) return;
        
        uint8_t ssidLength = buf[37];
        if (ssidLength > 32 || ssidLength == 0) return;
        
        char ssid[33];
        memcpy(ssid, &buf[38], ssidLength);
        ssid[ssidLength] = '\0';
        
        // Get signal strength
        int32_t rssi = (int32_t)buf[0];
        
        // Store the data
        instance->addProbeData(String(mac), String(ssid), rssi);
    }

    static ProbeSniffAttack* instance;

public:
    ProbeSniffAttack() {
        instance = this;
    }

    void begin() {
        WiFi.mode(WIFI_AP_STA);
        WiFi.disconnect();
    }

    void setMaxProbes(uint16_t max) {
        maxProbes = max;
    }

    void setSaveToStorage(bool save) {
        saveToStorage = save;
    }

    void setFilterDuplicates(bool filter) {
        filterDuplicates = filter;
    }

    void setMinSignalStrength(int8_t min) {
        minSignalStrength = min;
    }

    void start() {
        running = true;
        probeRequests.clear();
        signalStrengths.clear();
        lastSeenTimes.clear();
        startTime = millis();
        
        wifi_set_opmode(STATION_MODE);
        wifi_promiscuous_enable(0);
        wifi_set_promiscuous_rx_cb(promiscuousCallback);
        wifi_promiscuous_enable(1);
        wifi_set_channel(channel);
    }

    void stop() {
        running = false;
        wifi_promiscuous_enable(0);
        
        if (saveToStorage) {
            saveToFile();
        }
    }

    void update() {
        if (!running) return;

        unsigned long currentTime = millis();
        
        // Channel hopping
        if (currentTime - lastChannelSwitch >= CHANNEL_HOP_INTERVAL) {
            channel = (channel % 13) + 1;
            wifi_set_channel(channel);
            lastChannelSwitch = currentTime;
        }
        
        // Clean up old devices
        cleanupOldDevices(currentTime);
    }

    String getStatus() {
        String status = "Probe Sniffer:\n";
        status += "Devices: " + String(probeRequests.size()) + "\n";
        status += "Channel: " + String(channel) + "\n";
        status += "Time: " + formatTime((millis() - startTime) / 1000);
        return status;
    }

    String getDetailedStatus() {
        String status;
        unsigned long currentTime = millis();
        
        for (const auto& pair : probeRequests) {
            status += pair.first + " (";
            status += String(signalStrengths[pair.first]) + "dBm, ";
            status += formatTimeDiff(currentTime - lastSeenTimes[pair.first]) + " ago)\n";
            status += "  SSIDs: ";
            for (const String& ssid : pair.second) {
                status += ssid + ", ";
            }
            status += "\n";
        }
        return status;
    }

    uint32_t getProbeCount() const { return probeRequests.size(); }
    size_t getUniqueClientCount() const { return probeRequests.size(); }

private:
    void addProbeData(String mac, String ssid, int32_t rssi) {
        if (probeRequests.size() >= maxProbes) return;
        if (rssi < minSignalStrength) return;
        
        unsigned long currentTime = millis();
        
        // Update last seen time
        lastSeenTimes[mac] = currentTime;
        
        // Update signal strength with moving average
        if (signalStrengths.find(mac) == signalStrengths.end()) {
            signalStrengths[mac] = rssi;
        } else {
            signalStrengths[mac] = (signalStrengths[mac] * 7 + rssi) / 8;
        }
        
        // Add SSID if not already present
        if (probeRequests.find(mac) == probeRequests.end()) {
            probeRequests[mac] = std::vector<String>();
        }
        
        auto& ssids = probeRequests[mac];
        if (!filterDuplicates || std::find(ssids.begin(), ssids.end(), ssid) == ssids.end()) {
            ssids.push_back(ssid);
        }
    }

    void cleanupOldDevices(unsigned long currentTime) {
        for (auto it = lastSeenTimes.begin(); it != lastSeenTimes.end();) {
            if (currentTime - it->second > deviceTimeout) {
                String mac = it->first;
                probeRequests.erase(mac);
                signalStrengths.erase(mac);
                it = lastSeenTimes.erase(it);
            } else {
                ++it;
            }
        }
    }

    void saveToFile() {
        File f = SPIFFS.open("/probes.txt", "a");
        if (!f) return;
        
        for (const auto& pair : probeRequests) {
            String line = pair.first + "," + 
                         String(signalStrengths[pair.first]) + ",";
            for (const String& ssid : pair.second) {
                line += ssid + ";";
            }
            line += "\n";
            f.print(line);
        }
        f.close();
    }

    String formatTime(unsigned long seconds) {
        int hours = seconds / 3600;
        int mins = (seconds % 3600) / 60;
        int secs = seconds % 60;
        
        char buffer[9];
        snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d", hours, mins, secs);
        return String(buffer);
    }

    String formatTimeDiff(unsigned long ms) {
        if (ms < 1000) return String(ms) + "ms";
        if (ms < 60000) return String(ms / 1000) + "s";
        return String(ms / 60000) + "m";
    }
};

ProbeSniffAttack* ProbeSniffAttack::instance = nullptr;

#endif 