# Homekit

### This is work-in-progress!

This project aims to implement Homekit for the ESP32 using the Arduino framework where possible.

## Build instructions

This is a application to be used with `Espressif IoT Development Framework (ESP-IDF)`. 

Please check ESP-IDF docs for getting started instructions and install instructions.

[Espressif IoT Development Framework](https://github.com/espressif/esp-idf)

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

#define WIFI_SSID		"SSID"
#define WIFI_PASSWORD	"PASSWORD"

#endif /* WIFICREDENTIALS_HPP_ */
```

## Tested with the following apps
- [hap-client-tool](https://github.com/forty2/hap-client-tool)
- Home app
- Elgato Eve app
