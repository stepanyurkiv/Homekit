# Homekit

### This is work-in-progress!

This project aims to implement Homekit for the ESP32 using the Arduino framework where possible.


## Build instructions

This is a application to be used with `Espressif IoT Development Framework (ESP-IDF)` and `arduino-esp32`. 

Please check ESP-IDF docs for getting started instructions and install instructions.

[Espressif IoT Development Framework](https://github.com/espressif/esp-idf)

Once the ESP-IDF is installed:

```shell
$ git clone https://github.com/An00bIS47/Homekit
$ cd Homekit
$ git submodule update --init --recursive
$ make -j4
$ make flash monitor
```


## WiFi Settings

Add a file called ``` WiFiCredentials.hpp ``` in the ```main``` folder and edit the settings:
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

#define WIFI_SSID       "SSID"
#define WIFI_PASSWORD   "PASSWORD"

#endif /* WIFICREDENTIALS_HPP_ */
```


## EEPROM Structure - Not yet working as intended :(

Pairings and long term keys are stored in a EEPROM partition called `eeprom`with a size of `4096 Bytes`.
To reset the EEPROM partition call `make erase_flash`. 

```c++
| Address | Description            | Bytes              |  
| ------- | ---------------------- | -------------------|  
|       0 | Long Term Public Key   |                 32 |
|      32 | Long Term Private Key  |                 64 |  
|         |                        |                    |  
|      96 | 1. Pairing: id  	   |                 36 |  
|     132 | 1. Pairing: key  	   |                 36 |  
|         |                        |                    |  
|     168 | 2. Pairing: id  	   |                 36 |  
|     204 | 2. Pairing: key  	   |                 36 |
|         |                        |                    |  
```
