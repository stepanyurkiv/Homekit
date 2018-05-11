//
// HAPWiFiHelper.cpp
// Homekit
//
//  Created on: 08.08.2017
//      Author: michael
//

#include "HAPWiFiHelper.hpp"

#include <WString.h>
#include <esp_wps.h>
#include <esp_wifi.h>

#include "HAPLogger.hpp"


#define ESP_WPS_MODE WPS_TYPE_PBC
//#define ESP_WPS_MODE WPS_TYPE_PIN

HAPWiFiHelper::HAPWiFiHelper() {
	WiFi.onEvent(eventHandler);
}

HAPWiFiHelper::~HAPWiFiHelper() {
	// TODO Auto-generated destructor stub
}

void HAPWiFiHelper::startWPS(){

	WiFi.mode(WIFI_STA);
	//ESP_ERROR_CHECK(esp_wifi_wps_enable(WPS_TYPE_PBC));
	ESP_ERROR_CHECK(esp_wifi_wps_start(0));

	LogV("Connecting via WPS ..", false);
	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		LogV(".", false);
	}
	LogV( F("OK"), true);
}

void HAPWiFiHelper::connect(const char* ssid, const char* password) {

	LogV( F("Connecting to SSID  *"), false);
	LogV(ssid, false);
	LogV( F("* .."), false);

	WiFi.mode(WIFI_STA);
	WiFi.begin(ssid, password);

	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		LogV( F("."), false);
	}
	LogFlush();
	// LogV( F("OK"), true); // is done through event
}

void HAPWiFiHelper::eventHandler(WiFiEvent_t event) {
	switch(event) {
		case SYSTEM_EVENT_STA_START:

			break;
		case SYSTEM_EVENT_STA_CONNECTED:
			//enable sta ipv6 here
            WiFi.enableIpV6();
			LogV( F("OK"), true);
			break;
		case SYSTEM_EVENT_STA_GOT_IP:
			LogV( F("Got IP address "), false);
			LogV(WiFi.localIP().toString().c_str(), true);
			break;

		case SYSTEM_EVENT_AP_STA_GOT_IP6:
			LogV( F("Got IPv6 address "), false);
			LogV(WiFi.localIPv6().toString().c_str(), true);
			
			break;	
		case SYSTEM_EVENT_STA_DISCONNECTED:
			LogW( F("WARNING: WiFi lost connection!"), true);
			break;
		case SYSTEM_EVENT_STA_WPS_ER_SUCCESS:
			/*point: the function esp_wifi_wps_start() only get ssid & password
			 * so call the function esp_wifi_connect() here
			 * */
			LogV( F("WPS succeeded"), true);
//			ESP_LOGI(TAG, "SYSTEM_EVENT_STA_WPS_ER_SUCCESS");
			ESP_ERROR_CHECK(esp_wifi_wps_disable());
			ESP_ERROR_CHECK(esp_wifi_connect());
			break;
		case SYSTEM_EVENT_STA_WPS_ER_FAILED:
			LogE( F("WPS failed"), true);
//			ESP_LOGI(TAG, "SYSTEM_EVENT_STA_WPS_ER_FAILED");
			/*
            ESP_ERROR_CHECK(esp_wifi_wps_disable());
			ESP_ERROR_CHECK(esp_wifi_wps_enable(WPS_TYPE_PBC));
			ESP_ERROR_CHECK(esp_wifi_wps_start(0));
             */
			break;
		case SYSTEM_EVENT_STA_WPS_ER_TIMEOUT:
			LogE( F("WPS timeout"), true);
//			ESP_LOGI(TAG, "SYSTEM_EVENT_STA_WPS_ER_TIMEOUT");
			/*
            ESP_ERROR_CHECK(esp_wifi_wps_disable());
			ESP_ERROR_CHECK(esp_wifi_wps_enable(WPS_TYPE_PBC));
			ESP_ERROR_CHECK(esp_wifi_wps_start(0));
             */
			break;
//		case SYSTEM_EVENT_STA_WPS_ER_PIN:
//			Log(COLOR_GREEN, "WPS PIN Code", true);
////			ESP_LOGI(TAG, "SYSTEM_EVENT_STA_WPS_ER_PIN");
//			/*show the PIN code here*/
////			ESP_LOGI(TAG, "WPS_PIN = "PINSTR, PIN2STR(event->event_info.sta_er_pin.pin_code));
//			Log(COLOR_GREEN, "WPS_PIN", false);
//			char str[12];
//			sprintf(str, PINSTR, PIN2STR(event.event_info.sta_er_pin.pin_code));
//			Log(COLOR_GREEN, str, true);
//			break;
		default:
			break;
	}
}
