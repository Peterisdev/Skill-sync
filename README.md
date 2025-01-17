# ESP8266 WiFi Pentesting Tool

A powerful WiFi security testing toolkit built for ESP8266, featuring an OLED interface and multiple attack vectors. This tool is designed for network security research and penetration testing.

## Features

### Attack Modules
- **Deauthentication Attack**: Disconnect devices from target networks
- **Evil Twin Attack**: Create fake access points to capture credentials
- **Beacon Spam**: Flood area with custom WiFi networks
- **Probe Sniffing**: Monitor probe requests from nearby devices
- **Rickroll Beacon**: Create fun WiFi networks using Rick Astley lyrics

### Hardware Interface
- OLED Display Support (SSD1306/SH1106)
- 3-Button Navigation System
- Intuitive Menu Interface

### Additional Features
- Captive Portal with Multiple Templates
- Channel Hopping
- MAC Address Randomization
- Credential Harvesting
- Settings Persistence
- Real-time Attack Status Display

## Hardware Requirements

- ESP8266 Development Board
- OLED Display (SSD1106 128x64)
- 3x Push Buttons
- USB Cable for Programming

## Pin Configuration

cpp
// Button Pins
#define BUTTON_UP D5
#define BUTTON_DOWN D6
#define BUTTON_SELECT D7
// Display Pins
#define OLED_SDA D1
#define OLED_SCL D2



## Building and Installation

1. Install Arduino IDE
2. Add ESP8266 board support
3. Install Required Libraries:
   - U8g2lib
   - ESP8266WiFi
   - DNSServer
   - ESP8266WebServer
   - ArduinoJson
4. Clone this repository
5. Open `ESP8266_Captive_Portal.ino`
6. Select your board and upload

## Usage

1. Power on the device
2. Use UP/DOWN buttons to navigate menus
3. Use SELECT button to choose options
4. Monitor attack status on the OLED display

## Legal Disclaimer

This tool is designed for:
- Security research
- Network testing
- Educational purposes

**You are responsible for how you use this tool. Only use it on networks you own or have explicit permission to test.**

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Acknowledgments

- ESP8266 Community
- Arduino Community
- Security Researchers

## Warning

Using this tool to attack networks without permission may be illegal in your country. Always obtain proper authorization before testing any network security tools.
