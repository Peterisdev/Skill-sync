#ifndef RICKROLL_BEACON_H
#define RICKROLL_BEACON_H

#include <ESP8266WiFi.h>
#include <vector>

class RickrollBeaconAttack {
private:
    bool running = false;
    int currentLine = 0;
    unsigned long lastChange = 0;
    unsigned long changeInterval = 2000;
    bool randomizeMac = true;
    uint8_t channel = 1;
    unsigned long lastChannelSwitch = 0;
    const unsigned long CHANNEL_HOP_INTERVAL = 500;
    bool channelHopping = true;
    uint32_t totalBeacons = 0;
    
    const std::vector<String> lyrics = {
        "Never gonna give you up",
        "Never gonna let you down",
        "Never gonna run around",
        "And desert you",
        "Never gonna make you cry",
        "Never gonna say goodbye",
        "Never gonna tell a lie",
        "And hurt you",
        "Rick Astley WiFi",
        "(Get Rickrolled)",
        "We're no strangers to love",
        "You know the rules",
        "And so do I",
        "A full commitment's",
        "What I'm thinking of",
        "You wouldn't get this",
        "From any other guy"
    };

    uint8_t beaconPacket[109] = {
        0x80, 0x00,                         // Frame Control
        0x00, 0x00,                         // Duration
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, // Destination
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, // Source
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, // BSSID
        0x00, 0x00,                         // Sequence Control
        // Fixed parameters
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Timestamp
        0x64, 0x00,                         // Beacon Interval
        0x31, 0x04,                         // Capability Info
        // Tagged parameters
        0x00                                // SSID parameter
    };

public:
    void begin() {
        WiFi.mode(WIFI_AP_STA);
        wifi_set_opmode(STATION_MODE);
    }

    void setSpeed(uint8_t speed) {
        // Speed 1-5, where 5 is fastest
        speed = constrain(speed, 1, 5);
        changeInterval = 3000 / speed;
    }

    void setRandomizeMac(bool random) {
        randomizeMac = random;
    }

    void setChannelHopping(bool enabled) {
        channelHopping = enabled;
    }

    void setChannel(uint8_t newChannel) {
        if (newChannel >= 1 && newChannel <= 14) {
            channel = newChannel;
            if (running) {
                wifi_set_channel(channel);
            }
        }
    }

    void start() {
        running = true;
        currentLine = 0;
        totalBeacons = 0;
        lastChange = millis();
        lastChannelSwitch = millis();
        wifi_promiscuous_enable(1);
        wifi_set_channel(channel);
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
            channel = (channel % 13) + 1;
            wifi_set_channel(channel);
            lastChannelSwitch = currentTime;
        }

        // Update beacon SSID
        if (currentTime - lastChange >= changeInterval) {
            broadcastCurrentLine();
            currentLine = (currentLine + 1) % lyrics.size();
            lastChange = currentTime;
        }
    }

    String getStatus() {
        String status = "Rickroll Beacon:\n";
        status += "Current: " + lyrics[currentLine] + "\n";
        status += "Speed: " + String(3000/changeInterval) + "\n";
        status += "Beacons: " + String(totalBeacons) + "\n";
        status += "Channel: " + String(channel);
        return status;
    }

    void adjustSpeed() {
        unsigned long newInterval = changeInterval - 500;
        changeInterval = (newInterval < 500) ? 2000 : newInterval;
    }

private:
    void broadcastCurrentLine() {
        String ssid = lyrics[currentLine];
        uint8_t packet[200];
        memcpy(packet, beaconPacket, sizeof(beaconPacket));

        // Randomize MAC if enabled
        if (randomizeMac) {
            for (int i = 10; i < 16; i++) {
                packet[i] = random(256);
                packet[i + 6] = packet[i];
            }
        }

        // Set SSID
        uint8_t ssidLen = ssid.length();
        packet[37] = 0x00;  // SSID parameter number
        packet[38] = ssidLen;  // SSID length
        memcpy(&packet[39], ssid.c_str(), ssidLen);

        // Add supported rates
        packet[39 + ssidLen] = 0x01;  // Supported rates parameter number
        packet[40 + ssidLen] = 0x08;  // Supported rates length
        packet[41 + ssidLen] = 0x82;  // 1 Mbps
        packet[42 + ssidLen] = 0x84;  // 2 Mbps
        packet[43 + ssidLen] = 0x8B;  // 5.5 Mbps
        packet[44 + ssidLen] = 0x96;  // 11 Mbps
        packet[45 + ssidLen] = 0x24;  // 18 Mbps
        packet[46 + ssidLen] = 0x30;  // 24 Mbps
        packet[47 + ssidLen] = 0x48;  // 36 Mbps
        packet[48 + ssidLen] = 0x6C;  // 54 Mbps

        // Add channel information
        packet[49 + ssidLen] = 0x03;  // Channel parameter number
        packet[50 + ssidLen] = 0x01;  // Channel length
        packet[51 + ssidLen] = channel;  // Current channel

        // Add RSN (WPA2) Information
        packet[52 + ssidLen] = 0x30;  // RSN parameter number
        packet[53 + ssidLen] = 0x14;  // RSN length
        packet[54 + ssidLen] = 0x01, 0x00;  // Version
        packet[56 + ssidLen] = 0x00, 0x0F, 0xAC, 0x04;  // Group cipher suite (CCMP)
        packet[60 + ssidLen] = 0x01, 0x00;  // Pairwise cipher suite count
        packet[62 + ssidLen] = 0x00, 0x0F, 0xAC, 0x04;  // Pairwise cipher suite (CCMP)
        packet[66 + ssidLen] = 0x01, 0x00;  // Authentication suite count
        packet[68 + ssidLen] = 0x00, 0x0F, 0xAC, 0x02;  // Authentication suite (PSK)
        packet[72 + ssidLen] = 0x00, 0x00;  // RSN capabilities

        uint16_t packetSize = 74 + ssidLen;
        wifi_send_pkt_freedom(packet, packetSize, 0);
        totalBeacons++;
        delay(1);
    }
};

#endif 