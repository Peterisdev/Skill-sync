#ifndef WIFI_STRUCTS_H
#define WIFI_STRUCTS_H

#include <ESP8266WiFi.h>
#include <vector>
#include <algorithm>

struct WiFiNetwork {
    String ssid;
    int32_t rssi;
    uint8_t channel;
    uint8_t bssid[6];
    uint8_t encType;
    bool hidden;
    unsigned long lastSeen;
    std::vector<String> clients;
    uint16_t beaconInterval;
    bool isTarget;

    // Constructor
    WiFiNetwork() : rssi(0), channel(0), hidden(false), lastSeen(0), 
                   beaconInterval(100), isTarget(false) {
        memset(bssid, 0, 6);
    }

    // Helper methods
    String getBSSIDString() const {
        char bssidStr[18];
        snprintf(bssidStr, sizeof(bssidStr), "%02X:%02X:%02X:%02X:%02X:%02X",
                bssid[0], bssid[1], bssid[2], bssid[3], bssid[4], bssid[5]);
        return String(bssidStr);
    }

    String getEncryptionType() const {
        switch(encType) {
            case ENC_TYPE_NONE: return "Open";
            case ENC_TYPE_WEP: return "WEP";
            case ENC_TYPE_TKIP: return "WPA";
            case ENC_TYPE_CCMP: return "WPA2";
            case ENC_TYPE_AUTO: return "WPA*";
            default: return "Unknown";
        }
    }

    int getSignalQuality() const {
        return constrain(2 * (rssi + 100), 0, 100);
    }

    String getSignalBars() const {
        int quality = getSignalQuality();
        if (quality >= 80) return "████";
        if (quality >= 60) return "███░";
        if (quality >= 40) return "██░░";
        if (quality >= 20) return "█░░░";
        return "░░░░";
    }

    void addClient(const String& clientMac) {
        if (std::find(clients.begin(), clients.end(), clientMac) == clients.end()) {
            clients.push_back(clientMac);
        }
    }

    void removeClient(const String& clientMac) {
        clients.erase(
            std::remove(clients.begin(), clients.end(), clientMac),
            clients.end()
        );
    }

    bool hasClient(const String& clientMac) const {
        return std::find(clients.begin(), clients.end(), clientMac) != clients.end();
    }

    String getDetailedInfo() const {
        String info = "SSID: " + (hidden ? "<Hidden>" : ssid) + "\n";
        info += "BSSID: " + getBSSIDString() + "\n";
        info += "Channel: " + String(channel) + "\n";
        info += "Signal: " + String(rssi) + "dBm " + getSignalBars() + "\n";
        info += "Security: " + getEncryptionType() + "\n";
        info += "Clients: " + String(clients.size()) + "\n";
        return info;
    }
};

struct DeauthTarget {
    String bssid;
    String client;
    uint32_t packets;
    
    DeauthTarget(String b, String c, uint32_t p = 0) 
        : bssid(b), client(c), packets(p) {}
};

struct ProbeRequest {
    String clientMac;
    String ssid;
    int32_t rssi;
    unsigned long timestamp;
    uint8_t channel;

    // Constructor
    ProbeRequest() : rssi(0), timestamp(0), channel(0) {}

    String getInfo() const {
        String info = "Client: " + clientMac + "\n";
        info += "SSID: " + (ssid.length() > 0 ? ssid : "<broadcast>") + "\n";
        info += "Signal: " + String(rssi) + "dBm\n";
        info += "Channel: " + String(channel) + "\n";
        return info;
    }
};

typedef std::vector<WiFiNetwork> NetworkList;

#endif 