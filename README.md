# Homekit
====================

Homekit for ESP32 Arduino used as ESP-IDF component

# Work-in-progress

## Pair-Setup
- [X] setupM1
- [X] setupM3
- [X] setupM5

## Pair-Verify
- [X] verifyM1
- [X] verifyM3

## En-/Decrypt
- [X] decrypt
- [ ] encrypt

# ToDo
- Accessories
- Characteristics
- Services
- etc

# Bugs
- Disconnect while setupM5 with iOS Devices

# Tested with hap-client-tool
https://github.com/forty2/hap-client-tool


# Build instructions

This is a application to be used with `Espressif IoT Development Framework`_ (ESP-IDF). 

Please check ESP-IDF docs for getting started instructions.

Espressif IoT Development Framework: https://github.com/espressif/esp-idf


# WiFi Settings

Add a file called ``` WiFiCredentials.hpp ``` in the ```main``` folder and edit the settigs:
```
//
// WiFiCredentials.hpp
// Homekit
//
//  Created on: 
//      Author: 
//

#ifndef WIFICREDENTIALS_HPP_
#define WIFICREDENTIALS_HPP_

#define WIFI_SSID	"SSID"
#define WIFI_PASSWORD	"PASSWORD"

#endif /* WIFICREDENTIALS_HPP_ */
```
