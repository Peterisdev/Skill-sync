#include "ui.h"

// Helper function for consistent title bars
void UIManager::drawTitleBar(const char* title) {
    display->drawBox(0, 0, 128, 12);
    display->setDrawColor(0);
    display->drawStr(2, 2, title);
    display->setDrawColor(1);
}

// Helper function for consistent menu items
void UIManager::drawMenuItem(const char* text, int y, bool selected) {
    if(selected) {
        display->drawBox(0, y, 128, 15);
        display->setDrawColor(0);
    }
    display->drawStr(4, y + 3, text);
    display->setDrawColor(1);
}

void UIManager::drawMainMenu() {
    display->clearBuffer();
    drawTitleBar("Main Menu");
    
    const char* menuItems[] = {
        "WiFi Attacks",
        "Settings",
        "View Logs",
        "About"
    };
    const int NUM_ITEMS = 4;
    
    int startY = 20;
    int itemHeight = 14;
    
    display->setFont(u8g2_font_7x14_tr);
    
    for(int i = 0; i < NUM_ITEMS; i++) {
        bool isSelected = (i == menuPosition);
        
        if (isSelected) {
            display->drawBox(2, startY + (i * itemHeight) - 1, 124, itemHeight);
            display->setDrawColor(0);
        }
        
        display->drawStr(6, startY + (i * itemHeight) + 2, menuItems[i]);
        
        if (isSelected) {
            display->setDrawColor(1);
        }
    }
    
    display->setDrawColor(1);
    display->drawBox(0, 53, 128, 11);
    display->setDrawColor(0);
    display->drawStr(4, 62, "UP/DN:Move  OK:Select");
    
    display->sendBuffer();
}

void UIManager::drawAttackMenu(AttackType currentAttack, bool isRunning) {
    display->clearBuffer();
    drawTitleBar("WiFi Attacks");
    
    const char* attacks[] = {
        "Deauth Attack",
        "Evil Twin",
        "Beacon Spam",
        "Probe Sniff",
        "Rickroll"
    };
    const int NUM_ATTACKS = 5;
    
    int startY = 16;
    int itemHeight = 12;
    
    for(int i = 0; i < NUM_ATTACKS; i++) {
        bool isSelected = (i == attackMenuPosition);
        bool isActive = (isRunning && i == static_cast<int>(currentAttack));
        
        // Draw selection background
        if (isSelected) {
            display->drawBox(0, startY + (i * itemHeight), 128, itemHeight);
            display->setDrawColor(0);
        }
        
        // Draw attack name
        display->drawStr(4, startY + (i * itemHeight) + 2, attacks[i]);
        
        // Draw status indicator if attack is active
        if (isActive) {
            display->drawStr(110, startY + (i * itemHeight) + 2, "●");
        }
        
        if (isSelected) {
            display->setDrawColor(1);
        }
    }
    
    // Status bar
    display->drawHLine(0, 52, 128);
    if (isRunning) {
        display->drawStr(2, 54, "⏎ Stop  ← Back");
    } else {
        display->drawStr(2, 54, "⏎ Start  ← Back");
    }
    
    display->sendBuffer();
}

void UIManager::drawNetworkScan() {
    display->clearBuffer();
    drawTitleBar("Network Scan");
    
    static uint8_t animFrame = 0;
    const char* frames[] = {"|", "/", "-", "\\"};
    
    display->drawStr(4, 24, "Scanning Networks...");
    display->drawStr(4, 36, frames[animFrame]);
    animFrame = (animFrame + 1) % 4;
    
    display->drawStr(4, 54, "Please wait");
    
    display->sendBuffer();
}

void UIManager::drawNetworkList(const std::vector<WiFiNetwork>& networks, int selected) {
    display->clearBuffer();
    drawTitleBar("Networks");
    
    int start = max(0, selected - 2);
    int end = min((int)networks.size(), start + 4);
    
    for(int i = start; i < end; i++) {
        String networkInfo = networks[i].ssid + " (" + networks[i].rssi + "dB)";
        drawMenuItem(networkInfo.c_str(), 14 + ((i-start) * 12), i == selected);
    }
    
    if(networks.size() > 4) {
        display->drawFrame(124, 14, 4, 48);
        display->drawBox(124, 14 + ((selected * 48) / networks.size()), 4, 12);
    }
    
    display->sendBuffer();
}

void UIManager::drawAttackStatus(const String& status, int progress, AttackType currentAttack) {
    display->clearBuffer();
    drawTitleBar("Attack Status");
    
    // Show attack type
    display->drawStr(2, 16, getAttackName(currentAttack).c_str());
    
    // Show status
    display->drawStr(2, 28, status.c_str());
    
    // Draw progress bar
    int barWidth = 124;
    int barHeight = 8;
    int barX = 2;
    int barY = 38;
    
    display->drawFrame(barX, barY, barWidth, barHeight);
    if (progress > 0) {
        int fillWidth = (progress * barWidth) / 100;
        display->drawBox(barX, barY, fillWidth, barHeight);
    }
    
    // Show controls
    display->drawHLine(0, 52, 128);
    display->drawStr(2, 54, "⏎ Stop  ← Back");
    
    display->sendBuffer();
}

void UIManager::drawSettingsMenu() {
    display->clearBuffer();
    drawTitleBar("Settings");
    
    const char* settings[] = {
        "Display",
        "WiFi",
        "Attack Config",
        "Reset All"
    };
    const int NUM_SETTINGS = 4;
    
    int startY = 16;
    int itemHeight = 12;
    
    for(int i = 0; i < NUM_SETTINGS; i++) {
        bool isSelected = (i == settingsMenuPosition);
        
        if (isSelected) {
            display->drawBox(0, startY + (i * itemHeight), 128, itemHeight);
            display->setDrawColor(0);
        }
        
        display->drawStr(4, startY + (i * itemHeight) + 2, settings[i]);
        
        if (isSelected) {
            display->setDrawColor(1);
        }
    }
    
    display->drawHLine(0, 52, 128);
    display->drawStr(2, 54, "↑↓ Select  ⏎ Enter");
    
    display->sendBuffer();
}

void UIManager::drawAttackSettings() {
    display->clearBuffer();
    drawTitleBar("Attack Settings");
    
    for(int i = 0; i < 3; i++) {
        drawMenuItem(getAttackSettingName(i), 14 + (i * 16), i == settingsSubMenuPosition);
    }
    
    // Add current value display
    String value = getCurrentSettingValue(ATTACK_SETTINGS);
    display->drawStr(4, 54, value.c_str());
    
    display->sendBuffer();
}

void UIManager::drawNetworkSettings() {
    display->clearBuffer();
    drawTitleBar("Network Settings");
    
    for(int i = 0; i < 3; i++) {
        drawMenuItem(getNetworkSettingName(i), 14 + (i * 16), i == settingsSubMenuPosition);
    }
    
    // Add current value display
    String value = getCurrentSettingValue(NETWORK_SETTINGS);
    display->drawStr(4, 54, value.c_str());
    
    display->sendBuffer();
}

void UIManager::drawDisplaySettings() {
    display->clearBuffer();
    drawTitleBar("Display Settings");
    
    for(int i = 0; i < 3; i++) {
        drawMenuItem(getDisplaySettingName(i), 14 + (i * 16), i == settingsSubMenuPosition);
    }
    
    // Add current value display
    String value = getCurrentSettingValue(DISPLAY_SETTINGS);
    display->drawStr(4, 54, value.c_str());
    
    display->sendBuffer();
}

// Helper function to get current setting value
String UIManager::getCurrentSettingValue(MenuState state) {
    switch(state) {
        case ATTACK_SETTINGS:
            switch(settingsSubMenuPosition) {
                case 0: return "Packets: " + String(settingsManager->getAttackSettings().deauthPacketsPerBurst);
                case 1: return "Hopping: " + String(settingsManager->getAttackSettings().channelHopping ? "ON" : "OFF");
                case 2: return "Interval: " + String(settingsManager->getAttackSettings().beaconInterval) + "ms";
                default: return "";
            }
        case NETWORK_SETTINGS:
            switch(settingsSubMenuPosition) {
                case 0: return "Power: " + String(settingsManager->getNetworkSettings().txPower) + "dBm";
                case 1: return "Channel: " + String(settingsManager->getNetworkSettings().defaultChannel);
                case 2: return "Hidden: " + String(settingsManager->getNetworkSettings().hiddenAP ? "YES" : "NO");
                default: return "";
            }
        case DISPLAY_SETTINGS:
            switch(settingsSubMenuPosition) {
                case 0: return "Contrast: " + String(settingsManager->getDisplaySettings().contrast);
                case 1: return "Flip: " + String(settingsManager->getDisplaySettings().flipDisplay ? "YES" : "NO");
                case 2: return "Timeout: " + String(settingsManager->getDisplaySettings().screenTimeout) + "s";
                default: return "";
            }
        default:
            return "";
    }
}

void UIManager::drawTemplateSelect(PortalTemplate currentTemplate) {
    display->clearBuffer();
    drawTitleBar("Select Template");
    
    const char* templates[] = {
        "Instagram",
        "Facebook",
        "Gmail",
        "LinkedIn",
        "Netflix",
        "PayPal",
        "Twitter",
        "Microsoft",
        "Apple ID",
        "Custom"
    };
    
    drawMenuItem(templates[static_cast<int>(currentTemplate)], 24, true);
    display->sendBuffer();
}

void UIManager::drawCredentials(const String& creds, int scrollPos) {
    display->clearBuffer();
    drawTitleBar("Captured Credentials");
    
    // Calculate max visible lines
    int lineHeight = 12;
    int maxLines = 4;
    int startY = 14;
    
    // Split credentials into lines
    std::vector<String> lines;
    int lastSpace = 0;
    int lineStart = 0;
    
    for (size_t i = 0; i < creds.length(); i++) {
        if (creds[i] == '\n' || creds[i] == ' ') {
            lines.push_back(creds.substring(lineStart, i));
            lineStart = i + 1;
        }
    }
    if (lineStart < creds.length()) {
        lines.push_back(creds.substring(lineStart));
    }
    
    // Draw visible lines with scrolling
    for (int i = 0; i < maxLines && i + scrollPos < lines.size(); i++) {
        display->drawStr(0, startY + (i * lineHeight), lines[i + scrollPos].c_str());
    }
    
    display->sendBuffer();
}

void UIManager::showBootSplash() {
    display->clearBuffer();
    
    // Draw logo or title
    display->setFont(u8g2_font_8x13B_tf);
    display->drawStr(10, 20, "WiFi Watchdog");
    display->setFont(u8g2_font_6x10_tf);
    display->drawStr(20, 40, "Initializing...");
    
    // Draw progress bar
    display->drawFrame(10, 50, 108, 8);
    display->drawBox(12, 52, 104, 4);
    
    display->sendBuffer();
    delay(1500); // Show splash for 1.5 seconds
}

void UIManager::begin() {
    display->begin();
    display->setContrast(settingsManager->getDisplaySettings().contrast);
    
    // Fix display orientation - force normal orientation regardless of settings
    display->setDisplayRotation(U8G2_R0);
    display->clearBuffer();
    
    // Set proper font and display settings
    display->setFont(u8g2_font_6x10_tf);
    display->setFontPosTop();
    display->setDrawColor(1);
}

String UIManager::getAttackName(AttackType type) const {
    switch(type) {
        case DEAUTH:
            return "Deauth Attack";
        case EVIL_TWIN:
            return "Evil Twin";
        case BEACON_SPAM:
            return "Beacon Spam";
        case PROBE_SNIFF:
            return "Probe Sniff";
        case RICKROLL_BEACON:
            return "Rickroll";
        case NONE:
        default:
            return "None";
    }
} 