//
// HAPWiFiHelper.hpp
// Homekit
//
//  Created on: 08.08.2017
//      Author: michael
//

#ifndef HAPWIFIHELPER_HPP_
#define HAPWIFIHELPER_HPP_

#include <Arduino.h>
#include <WiFi.h>

class HAPWiFiHelper {
public:
	HAPWiFiHelper();
	~HAPWiFiHelper();

	static void connect(const char* ssid, const char* password);
	static void startWPS();

private:
	static void eventHandler(WiFiEvent_t event);
};

#endif /* HAPWIFIHELPER_HPP_ */
