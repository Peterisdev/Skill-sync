#ifndef LOGGER_H
#define LOGGER_H

#include <Arduino.h>
#include <vector>
#include <FS.h>
#include "settings.h"

enum LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    ATTACK
};

struct LogEntry {
    unsigned long timestamp;
    LogLevel level;
    String message;
    
    LogEntry(LogLevel lvl, const String& msg) 
        : timestamp(millis()), level(lvl), message(msg) {}
};

class Logger {
private:
    std::vector<LogEntry> memoryLogs;
    static const size_t MAX_MEMORY_LOGS = 100;
    bool serialEnabled = true;
    bool fileLoggingEnabled = true;
    LogLevel minimumLevel = DEBUG;
    
    const char* LOG_FILE = "/system.log";
    const char* ATTACK_LOG = "/attacks.log";
    const size_t MAX_FILE_SIZE = 1024 * 1024; // 1MB

    // ANSI color codes for serial output
    const char* COLORS[5] = {
        "\033[36m", // Cyan for DEBUG
        "\033[32m", // Green for INFO
        "\033[33m", // Yellow for WARNING
        "\033[31m", // Red for ERROR
        "\033[35m"  // Magenta for ATTACK
    };
    const char* RESET_COLOR = "\033[0m";

public:
    void begin() {
        if (serialEnabled) {
            Serial.begin(115200);
        }
        
        if (!SPIFFS.begin()) {
            debug("Failed to mount file system");
            fileLoggingEnabled = false;
        }
        
        // Rotate logs if they're too large
        rotateLogs();
        
        info("Logger initialized");
    }

    void debug(const String& message) {
        log(DEBUG, message);
    }

    void info(const String& message) {
        log(INFO, message);
    }

    void warning(const String& message) {
        log(WARNING, message);
    }

    void error(const String& message) {
        log(ERROR, message);
    }

    void attack(const String& message) {
        log(ATTACK, message);
        logToFile(ATTACK_LOG, formatAttackLog(message));
    }

    void log(LogLevel level, const String& message) {
        if (level < minimumLevel) return;

        // Add to memory buffer
        if (memoryLogs.size() >= MAX_MEMORY_LOGS) {
            memoryLogs.erase(memoryLogs.begin());
        }
        memoryLogs.emplace_back(level, message);

        // Print to serial if enabled
        if (serialEnabled) {
            Serial.print(COLORS[level]);
            Serial.print(formatLogEntry(level, message));
            Serial.println(RESET_COLOR);
        }

        // Save to file if enabled and not debug level
        if (fileLoggingEnabled && level > DEBUG) {
            logToFile(LOG_FILE, formatLogEntry(level, message));
        }
    }

    String getRecentLogs(size_t count = 10) {
        String output;
        size_t start = (memoryLogs.size() > count) ? 
                      memoryLogs.size() - count : 0;
        
        for (size_t i = start; i < memoryLogs.size(); i++) {
            output += formatLogEntry(memoryLogs[i].level, 
                                  memoryLogs[i].message);
            output += "\n";
        }
        return output;
    }

    String getAttackLogs() {
        if (!fileLoggingEnabled) return "File logging disabled";
        
        File file = SPIFFS.open(ATTACK_LOG, "r");
        if (!file) return "No attack logs found";
        
        String logs;
        while (file.available()) {
            logs += file.readStringUntil('\n') + "\n";
        }
        file.close();
        return logs;
    }

    void clearLogs() {
        memoryLogs.clear();
        if (fileLoggingEnabled) {
            SPIFFS.remove(LOG_FILE);
            SPIFFS.remove(ATTACK_LOG);
        }
        info("Logs cleared");
    }

    void setMinimumLevel(LogLevel level) {
        minimumLevel = level;
    }

    void enableSerial(bool enable) {
        serialEnabled = enable;
    }

    void enableFileLogging(bool enable) {
        fileLoggingEnabled = enable;
    }

private:
    void logToFile(const char* filename, const String& message) {
        if (!fileLoggingEnabled) return;

        File file = SPIFFS.open(filename, "a");
        if (!file) {
            Serial.println("Failed to open log file");
            return;
        }

        file.println(message);
        file.close();
    }

    String formatLogEntry(LogLevel level, const String& message) {
        char timestamp[32];
        unsigned long ms = millis();
        unsigned long seconds = ms / 1000;
        unsigned long minutes = seconds / 60;
        unsigned long hours = minutes / 60;
        
        sprintf(timestamp, "[%02lu:%02lu:%02lu.%03lu]", 
                hours, minutes % 60, seconds % 60, ms % 1000);
        
        const char* levelStr[] = {"DEBUG", "INFO", "WARN", "ERROR", "ATTACK"};
        return String(timestamp) + " " + levelStr[level] + ": " + message;
    }

    String formatAttackLog(const String& message) {
        char timestamp[20];
        time_t now = time(nullptr);
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localtime(&now));
        return String(timestamp) + " | " + message;
    }

    void rotateLogs() {
        if (!fileLoggingEnabled) return;

        // Check system log size
        File file = SPIFFS.open(LOG_FILE, "r");
        if (file && file.size() > MAX_FILE_SIZE) {
            file.close();
            String tempFile = "/temp.log";
            
            // Copy last portion of log to temp file
            File src = SPIFFS.open(LOG_FILE, "r");
            File dst = SPIFFS.open(tempFile, "w");
            
            if (src && dst) {
                src.seek(src.size() - MAX_FILE_SIZE/2);
                while (src.available()) {
                    dst.write(src.read());
                }
                src.close();
                dst.close();
                
                // Replace original with rotated log
                SPIFFS.remove(LOG_FILE);
                SPIFFS.rename(tempFile, LOG_FILE);
            }
        }

        // Similar rotation for attack log
        file = SPIFFS.open(ATTACK_LOG, "r");
        if (file && file.size() > MAX_FILE_SIZE) {
            // ... (similar rotation logic for attack log)
        }
    }
};

extern Logger logger;

#endif 