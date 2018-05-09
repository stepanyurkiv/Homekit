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
#define HAP_HOSTNAME				"ESP32 Homekit"
#define HAP_MANUFACTURER			"C4"
#define HAP_MODELL_NAME				"Huzzah32"

/**
 * Compile 
 ********************************************************************/
#define HAP_USE_WOLFSSL_HKDF		1		// Use wolfssl

#define HAP_GENERATE_XHM			0		// Create hash and qr code

#define HAP_UPDATE_SERVER 			1		// Use HAP update server to check
											// if a update is available on the
											// provided webserver

#define HAP_NTP_ENABLED 			1		// Enable SNTP client



/**
 * Options
 ********************************************************************/
#define HAP_BUFFERED_SEND			1		// !!! Keep enabled !!!
											// Send all data via wifi in *one* response

#define HAP_PRINT_QRCODE			0		// !!! HAP_GENERATE_XHM must be enabled !!!
											// Print QR code on console

#define HAP_LONG_UUID				0		// Use long uuid as type in accessory json


#define HAP_MINIMAL_PLUGIN_INTERVAL	5000	// Minimal plugin handle interval in ms


#if HAP_UPDATE_SERVER
//#define HAP_UPDATE_SERVER_URL 	"192.168.178.151"
#define HAP_UPDATE_SERVER_URL 		"192.168.178.103"
#define HAP_UPDATE_SERVER_PORT		5000
#endif


#if HAP_NTP_ENABLED
#define HAP_NTP_SERVER_URL			"time.euro.apple.com"						// NTP server url
#define HAP_NTP_TIME_FORMAT			"%Y-%m-%d %H:%M:%S"							// strftime format
#define HAP_NTP_TZ_INFO     		"WET-1WEST,M3.5.0/01:00,M10.5.0/01:00"		// timezone for berlin
#endif


/**
 * Other
 ********************************************************************/
#define HAP_CONFIGURATION_NUMBER	1		// Internal

/**
 * Sizes
 ********************************************************************/
#if HAP_BUFFERED_SEND
#define HAP_BUFFER_SEND_SIZE		2048
#endif

#define HAP_ARDUINOJSON_BUFFER_SIZE 2048
#define HAP_ENCRYPTION_BUFFER_SIZE 	2048

#define HAP_PAIRINGS_MAX			8		// Number of available pairings 


#endif /* HAPGLOBALS_HPP_ */