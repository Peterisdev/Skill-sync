#ifndef ATTACK_MANAGER_H
#define ATTACK_MANAGER_H

#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include "attacks.h"
#include "evil_twin.h"
#include "deauth.h"
#include "beacon_spam.h"
#include "probe_sniff.h"
#include "rickroll_beacon.h"
#include "settings_manager.h"
#include "storage.h"

class AttackManager {
private:
    DNSServer* dnsServer;
    ESP8266WebServer* webServer;
    SettingsManager* settingsManager;
    EvilTwinAttack evilTwin;
    DeauthAttack deauth;
    BeaconSpamAttack beaconSpam;
    ProbeSniffAttack probeSniff;
    RickrollBeaconAttack rickrollBeacon;
    
    AttackType currentAttack = NONE;
    bool attackRunning = false;
    unsigned long attackStartTime = 0;
    NetworkList networkList;
    String targetSSID;
    String targetBSSID;
    
    // Attack statistics
    struct AttackStats {
        uint32_t packetsTotal = 0;
        uint32_t clientsAffected = 0;
        uint32_t credentialsCaptured = 0;
        uint32_t probesCollected = 0;
        unsigned long duration = 0;
    } stats;

    PortalTemplate currentTemplate = INSTAGRAM;
    bool isAttackRunning = false;
    AttackType selectedAttack = NONE;

public:
    void begin(DNSServer* dns, ESP8266WebServer* web, SettingsManager* settings) {
        dnsServer = dns;
        webServer = web;
        settingsManager = settings;
        webServer->begin();
        evilTwin.begin(dnsServer, webServer);
        deauth.begin();
        beaconSpam.begin();
        probeSniff.begin();
        rickrollBeacon.begin();
    }

    void startAttack(AttackType type) {
        stopAttack();  // Stop any running attack first
        
        currentAttack = type;
        attackStartTime = millis();
        attackRunning = true;
        resetStats();

        switch(type) {
            case EVIL_TWIN:
                if(!targetSSID.isEmpty()) {
                    WiFiNetwork target;
                    target.ssid = targetSSID;
                    parseMAC(targetBSSID.c_str(), target.bssid);
                    evilTwin.setTarget(target);
                    evilTwin.start();
                }
                break;

            case DEAUTH:
                if(!targetBSSID.isEmpty()) {
                    deauth.setTarget(targetBSSID);
                    deauth.setPacketsPerBurst(settingsManager->getAttackSettings().deauthPacketsPerBurst);
                    deauth.start();
                }
                break;

            case BEACON_SPAM:
                beaconSpam.setInterval(settingsManager->getAttackSettings().beaconInterval);
                beaconSpam.setMaxSSIDs(settingsManager->getAttackSettings().maxBeacons);
                beaconSpam.start();
                break;

            case PROBE_SNIFF:
                probeSniff.setMaxProbes(settingsManager->getAttackSettings().maxProbes);
                probeSniff.setSaveToStorage(true);
                probeSniff.start();
                break;

            case RICKROLL_BEACON:
                rickrollBeacon.setSpeed(settingsManager->getAttackSettings().rickrollSpeed);
                rickrollBeacon.setChannelHopping(true);
                rickrollBeacon.start();
                break;

            default:
                break;
        }
    }

    void stopAttack() {
        if(!attackRunning) return;

        switch(currentAttack) {
            case EVIL_TWIN:
                evilTwin.stop();
                break;
            case DEAUTH:
                deauth.stop();
                break;
            case BEACON_SPAM:
                beaconSpam.stop();
                break;
            case PROBE_SNIFF:
                probeSniff.stop();
                break;
            case RICKROLL_BEACON:
                rickrollBeacon.stop();
                break;
            default:
                break;
        }

        attackRunning = false;
        currentAttack = NONE;
        updateStats();
    }

    void update() {
        if(!attackRunning) return;

        switch(currentAttack) {
            case EVIL_TWIN:
                evilTwin.update();
                dnsServer->processNextRequest();
                webServer->handleClient();
                break;
            case DEAUTH:
                deauth.update();
                break;
            case BEACON_SPAM:
                beaconSpam.update();
                break;
            case PROBE_SNIFF:
                probeSniff.update();
                break;
            case RICKROLL_BEACON:
                rickrollBeacon.update();
                break;
            default:
                break;
        }

        updateStats();
    }

    String getCurrentStatus() const {
        switch(currentAttack) {
            case DEAUTH:
                return "Deauth: " + String(stats.packetsTotal) + " pkts";
            case BEACON_SPAM:
                return "Beacon: " + String(stats.packetsTotal) + " SSIDs";
            case PROBE_SNIFF:
                return "Probes: " + String(stats.probesCollected);
            case EVIL_TWIN:
                return "Evil Twin: " + String(stats.credentialsCaptured) + " creds";
            case RICKROLL_BEACON:
                return "Rickroll: " + String(stats.packetsTotal) + " beacons";
            default:
                return "No attack running";
        }
    }

    String getAttackStatus() const {
        return getCurrentStatus();
    }

    void setTarget(const String& ssid, const String& bssid) {
        targetSSID = ssid;
        targetBSSID = bssid;
    }

    const AttackStats& getStats() const {
        return stats;
    }

    bool isRunning() const {
        return isAttackRunning;
    }

    AttackType getCurrentAttack() const {
        return currentAttack;
    }

    String getAttackName(AttackType type) const {
        switch(type) {
            case NONE: return "None";
            case EVIL_TWIN: return "Evil Twin";
            case DEAUTH: return "Deauth";
            case BEACON_SPAM: return "Beacon Spam";
            case PROBE_SNIFF: return "Probe Sniff";
            case RICKROLL_BEACON: return "Rickroll Beacon";
            default: return "Unknown";
        }
    }

    void scanNetworks() {
        WiFi.scanNetworks(true); // Start async scan
        networkList.clear();
    }

    const NetworkList& getScannedNetworks() const {
        return networkList;
    }

    void updateBeaconSpam() {
        if (currentAttack == BEACON_SPAM) {
            beaconSpam.update();
        }
    }

    void updateDeauth() {
        if (currentAttack == DEAUTH) {
            deauth.update();
        }
    }

    void updateProbeSniff() {
        if (currentAttack == PROBE_SNIFF) {
            probeSniff.update();
        }
    }

    void updateRickroll() {
        if (currentAttack == RICKROLL_BEACON) {
            rickrollBeacon.update();
        }
    }

    void stopCurrentAttack() {
        switch(currentAttack) {
            case EVIL_TWIN: evilTwin.stop(); break;
            case DEAUTH: deauth.stop(); break;
            case BEACON_SPAM: beaconSpam.stop(); break;
            case PROBE_SNIFF: probeSniff.stop(); break;
            case RICKROLL_BEACON: rickrollBeacon.stop(); break;
            default: break;
        }
        currentAttack = NONE;
        attackRunning = false;
    }

    void nextTemplate() {
        currentTemplate = static_cast<PortalTemplate>((static_cast<int>(currentTemplate) + 1) % 10);
    }

    PortalTemplate getCurrentTemplate() const {
        return currentTemplate;
    }

    void adjustRickrollSpeed() {
        if (currentAttack == RICKROLL_BEACON) {
            rickrollBeacon.adjustSpeed();
        }
    }

    int getAttackProgress() const {
        if (!attackRunning) return 0;
        
        switch(currentAttack) {
            case PROBE_SNIFF:
            case RICKROLL_BEACON:
            case EVIL_TWIN:
            case BEACON_SPAM:
                return 50; // Continuous attacks, show 50% progress
            case DEAUTH:
                return (deauth.getPacketCount() * 100) / 1000; // Arbitrary max of 1000 packets
            default:
                return 0;
        }
    }

    void updateNetworkList() {
        networkList.clear();
        int numNetworks = WiFi.scanComplete();
        for(int i = 0; i < numNetworks; i++) {
            WiFiNetwork network;
            network.ssid = WiFi.SSID(i);
            network.rssi = WiFi.RSSI(i);
            network.channel = WiFi.channel(i);
            memcpy(network.bssid, WiFi.BSSID(i), 6);
            networkList.push_back(network);
        }
    }

    void toggleAttack() {
        if (isAttackRunning) {
            stopAttack();
            isAttackRunning = false;
        } else if (selectedAttack != NONE) {
            startAttack(selectedAttack);
            isAttackRunning = true;
        }
    }

    void selectAttack(AttackType type) {
        selectedAttack = type;
    }

    AttackType getSelectedAttack() const { return selectedAttack; }

private:
    void resetStats() {
        stats = AttackStats();
    }

    void updateStats() {
        stats.duration = millis() - attackStartTime;
        
        switch(currentAttack) {
            case EVIL_TWIN:
                stats.credentialsCaptured = storageManager.getCredentialsCount();
                stats.clientsAffected = WiFi.softAPgetStationNum();
                break;
            case DEAUTH:
                stats.packetsTotal = deauth.getPacketCount();
                stats.clientsAffected = deauth.getTargetCount();
                break;
            case BEACON_SPAM:
                stats.packetsTotal = beaconSpam.getBeaconCount();
                break;
            case PROBE_SNIFF:
                stats.probesCollected = probeSniff.getProbeCount();
                stats.clientsAffected = probeSniff.getUniqueClientCount();
                break;
            default:
                break;
        }
    }

    String formatDuration(unsigned long ms) {
        unsigned long seconds = ms / 1000;
        unsigned long minutes = seconds / 60;
        unsigned long hours = minutes / 60;
        
        char buffer[12];
        sprintf(buffer, "%02lu:%02lu:%02lu", 
                hours, minutes % 60, seconds % 60);
        return String(buffer);
    }

    void parseMAC(const char* macStr, uint8_t* mac) {
        sscanf(macStr, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
               &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]);
    }

    String macToString(uint8_t* mac) {
        char macStr[18];
        snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
                mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
        return String(macStr);
    }
};

extern AttackManager attackManager;

#endif 