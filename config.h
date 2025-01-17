#ifndef CONFIG_H
#define CONFIG_H

// Pin Definitions
#define BUTTON_UP D5
#define BUTTON_DOWN D6
#define BUTTON_SELECT D7
#define OLED_SDA D1
#define OLED_SCL D2

// Network Configuration
#define WEB_PORT 80
#define MAX_NETWORKS 15

// Storage Configuration
#define STORAGE_SIZE 1024

// Display Configuration
#define DISPLAY_TIMEOUT 30000
#define MENU_TIMEOUT 60000

// Add this line to the existing config.h
#define DNS_PORT 53

// Display pins
#define DISPLAY_CLK 5  // D1/GPIO5 for I2C clock
#define DISPLAY_DATA 4 // D2/GPIO4 for I2C data

#endif 