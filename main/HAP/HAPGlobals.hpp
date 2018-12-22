//
// HAPGlobals.hpp
// Homekit
//
//  Created on: 08.08.2017
//      Author: michael
//
#ifndef HAPGLOBALS_HPP_
#define HAPGLOBALS_HPP_

/**
 * -> Preferences
 ********************************************************************/
#define HAP_DEBUG 					1
#define HAP_PIN_CODE 				"031-45-712"
#define HAP_HOSTNAME				"esp32"
#define HAP_MANUFACTURER			"C4"
#define HAP_MODELL_NAME				"Huzzah32"
#define HAP_RESET_EEPROM 			1

/**
 * Compile 
 ********************************************************************/
#define HAP_USE_WOLFSSL_HKDF		1		// Use wolfssl
											// Default: 1

#define HAP_GENERATE_XHM			1		// Create hash and qr code
											// Default: 0

#define HAP_UPDATE_SERVER 			0		// Use HAP update server to check
											// if a update is available on the
											// provided webserver
											// Default: 1

#define HAP_ENABLE_WEBSERVER		1		// Enable Webinterface
											// Default: 1

#define HAP_NTP_ENABLED 			1		// Enable SNTP client
											// Default: 1


/**
 * Options
 ********************************************************************/
#define HAP_BUFFERED_SEND			1		// !!! Keep enabled !!!
											// Send all data via wifi in *one* response
											// Default: 1

#define HAP_PRINT_QRCODE			0		// !!! HAP_GENERATE_XHM must be enabled !!!
											// Print QR code on console
											// Default: 0			

#define HAP_LONG_UUID				0		// Use long uuid as type in accessory json
											// Default: 0


#define HAP_MINIMAL_PLUGIN_INTERVAL	1000	// Minimal plugin handle interval in ms
											// Default: 1000

#if HAP_UPDATE_SERVER
//#define HAP_UPDATE_SERVER_URL 	"192.168.178.151"	
#define HAP_UPDATE_SERVER_URL 		"192.168.178.103"	// HTTP Server url for updates
#define HAP_UPDATE_SERVER_PORT		5000				// Update Server port
#endif


#if HAP_NTP_ENABLED
#define HAP_NTP_SERVER_URL			"time.euro.apple.com"						// NTP server url
#define HAP_NTP_TIME_FORMAT			"%Y-%m-%d %H:%M:%S"							// strftime format
#define HAP_NTP_TZ_INFO     		"WET-1WEST,M3.5.0/01:00,M10.5.0/01:00"		// timezone for berlin
#endif


#define HAP_API_ADMIN_MODE			1	// Allows detailed information on api interface


/**
 * Timeouts
 ********************************************************************/
#define ESP_WIFI_CONNECTION_TIMEOUT	5000

/**
 * Other - Do not edit !!!
 ********************************************************************/
#define HAP_CONFIGURATION_NUMBER	1		// Internal
#define HAP_AAD_LENGTH				2

/**
 * Limits
 ********************************************************************/
#if HAP_BUFFERED_SEND
#define HAP_BUFFER_SEND_SIZE		2048
#endif

#define HAP_ARDUINOJSON_BUFFER_SIZE 2048
#define HAP_ENCRYPTION_BUFFER_SIZE 	2048

#define HAP_PAIRINGS_MAX			10		// Number of available pairings 
											// Default: 10

#define HAP_PLUGIN_MAX_PARAMETER	5		// Maximum parameter a plugin can have
											// Default: 5

#endif /* HAPGLOBALS_HPP_ */