# Homekit
====================

Homekit for ESP32 Arduino used as ESP-IDF component

# Work-in-progress

This project aims to implement Homekit for the esp32 using the arduino framework where possible.

Currently it uses WolfSSL for encrpytion and srp but it is planned to replace it with using libsodium and mbedtls. 

## Bugs / ToDo:
- [ ] Persistent pairings
- [ ] Events


# Tested with hap-client-tool
https://github.com/forty2/hap-client-tool
Home app
Elgato Eve app

# Build instructions

This is a application to be used with `Espressif IoT Development Framework`_ (ESP-IDF). 

Please check ESP-IDF docs for getting started instructions.

Espressif IoT Development Framework: https://github.com/espressif/esp-idf

```shell
$ git clone https://github.com/An00bIS47/Homekit
$ cd Homekit
$ git submodule update --init --recursive
$ make -j4
$ make flash monitor
```


# WiFi Settings

Add a file called ``` WiFiCredentials.hpp ``` in the ```main``` folder and edit the settigs:
```c++
//
// WiFiCredentials.hpp
// Homekit
//
//  Created on: 
//      Author: 
//

#ifndef WIFICREDENTIALS_HPP_
#define WIFICREDENTIALS_HPP_

#define WIFI_SSID		"SSID"
#define WIFI_PASSWORD	"PASSWORD"

#endif /* WIFICREDENTIALS_HPP_ */
```
