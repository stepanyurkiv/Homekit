//
// main.cpp
// Homekit
//
//  Created on: 08.08.2017
//      Author: michael
//
#include <Arduino.h>

#include "HAP/HAPLogger.hpp"
#include "HAP/HAPServer.hpp"
#include "HAP/HAPWiFiHelper.hpp"
#include "HAP/HAPHelper.hpp"

#include "WiFiCredentials.hpp"

HAPWiFiHelper wifi;

unsigned long previousMillis = 0;

const long interval = 1000;

void setup(){
	Serial.begin(115200);


	Homekit_setFirmware("Homekit", HOMEKIT_VERSION);
	Homekit_setBrand("An00bIS47");

	/*
	 * Logging
	 */
	HAPLogger::setPrinter(&Serial);
	HAPLogger::setLogLevel(LogLevel::DEBUG);

	HAPLogger::printInfo();

	LogI( F("Starting Homekit "), false);
	LogI( hap.versionString() + String( " ..."), true);
	LogI( F("Log level: "), false);
	LogI( String(HAPLogger::getLogLevel() ), true);


	/*
	 * WiFi
	 */
	wifi.connect(WIFI_SSID, WIFI_PASSWORD);


	/*
	 * Homekit
	 */
	hap.begin();
}

void loop(){

	hap.handle();

}


