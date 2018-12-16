//
// HAPServer.hpp
// Homekit
//
//  Created on: 08.08.2017
//      Author: michael
//

#ifndef HAPSERVER_HPP_
#define HAPSERVER_HPP_

#include <Arduino.h>
#include <WiFiClient.h>
#include <WiFiServer.h>
#include <Preferences.h>
#include <vector>
#include <map>

#include <ArduinoJson.h>


#include "HAPGlobals.hpp"
#include "HAPClient.hpp"
#include "HAPAccessorySet.hpp"

#include "HAPVerifyContext.hpp"

#include "HAPLimits.hpp"
#include "HAPVersion.hpp"
#include "HAPTypes.hpp"
#include "HAPPairings.hpp"

#include "HAPPlugins.hpp"
#include "Plugins/Plugins.hpp"

#include "EventManager.h"

#include "HAPWebServer.hpp"

#define Homekit_setFirmware(name, version) const char* __FLAGGED_FW_NAME = "\xbf\x84\xe4\x13\x54" name "\x93\x44\x6b\xa7\x75"; const char* __FLAGGED_FW_VERSION = "\x6a\x3f\x3e\x0e\xe1" version "\xb0\x30\x48\xd4\x1a"; hap.__setFirmware(__FLAGGED_FW_NAME, __FLAGGED_FW_VERSION);
#define Homekit_setBrand(brand) const char* __FLAGGED_BRAND = "\xfb\x2a\xf5\x68\xc0" brand "\x6e\x2f\x0f\xeb\x2d"; hap.__setBrand(__FLAGGED_BRAND);

#if HAP_UPDATE_SERVER
#include "HAPUpdate.hpp"
#endif

#if HAP_PRINT_QRCODE
#if HAP_GENERATE_XHM
#else
#error HAP_GENERATE_XHM must be enabled!
#endif
#endif	

#if HAP_GENERATE_XHM
#include "qrcode.h"
#endif

static const char HTTP_200[] PROGMEM 					= "HTTP/1.1 200 OK\r\n";
static const char HTTP_204[] PROGMEM 					= "HTTP/1.1 204 No Content\r\n";
static const char HTTP_207[] PROGMEM 					= "HTTP/1.1 207 Multi-Status\r\n";
static const char HTTP_400[] PROGMEM 					= "HTTP/1.1 400 Bad Request\r\n";

static const char HTTP_CONTENT_TYPE_HAPJSON[] PROGMEM 	= "Content-Type: application/hap+json\r\n";
static const char HTTP_CONTENT_TYPE_TLV8[] PROGMEM 		= "Content-Type: application/pairing+tlv8\r\n";

static const char HTTP_KEEP_ALIVE[] PROGMEM		 		= "Connection: keep-alive\r\n";
static const char HTTP_TRANSFER_ENCODING[] PROGMEM		= "Transfer-Encoding: chunked\r\n";

static const char HTTP_CRLF[] PROGMEM 					= "\r\n";

static const char EVENT_200[] PROGMEM 					= "EVENT/1.0 200 OK\r\n";


enum HAP_ERROR_TYPE {
	HAP_ERROR_NONE					= 0x00,
	HAP_ERROR_UNKNOWN				= 0x01,
	HAP_ERROR_AUTHENTICATON			= 0x02,
	HAP_ERROR_BACKOFF				= 0x03,
	HAP_ERROR_MAX_PEERS				= 0x04,
	HAP_ERROR_MAX_TRIES				= 0x05,
	HAP_ERROR_UNAVAILABLE			= 0x06,
	HAP_ERROR_BUSY					= 0x07,
};

enum HAP_STATUS_CODE {
	HAP_STATUS_SUCCESS				= 0,
	HAP_STATUS_INUSUFFICIENT_PRIV	= -70401,
	HAP_STATUS_INTERNAL_COMM_ERROR	= -70402,
	HAP_STATUS_BUSY					= -70403,
	HAP_STATUS_READONLY_WRITE		= -70404,
	HAP_STATUS_WRITEONLY_READ		= -70405,
	HAP_STATUS_NO_NOTIFICATION		= -70406,
	HAP_STATUS_OUT_OF_RESSOURCES	= -70407,
	HAP_STATUS_TIMED_OUT			= -70408,
	HAP_STATUS_RESOURCE_NOT_FOUND	= -70409,
	HAP_STATUS_INVALID_VALUE		= -70410,
	HAP_STATUS_AUTH_FAILED			= -70411,
};


class HAPServer {
public:
	HAPServer(uint16_t port = 51628, uint8_t maxClients = 8);
	~HAPServer();

	bool begin();
	void handle();


	void __setFirmware(const char* name, const char* version);
	void __setBrand(const char* brand);



#if HAP_NTP_ENABLED
	static String timeString();
	static uint32_t timestamp();
#endif

	String versionString();
	bool isPaired();

protected:

	// struct HAPPairSetup* _pairSetup;
	HAPWebServer _webserver;


	void* _srp;
	struct HAPLongTermContext* _longTermContext;

	HAPAccessorySet* _accessorySet;
	std::vector<HAPClient> _clients;
	WiFiServer _server;
	Preferences _preferences;
	EventManager _eventManager;

#if HAP_NTP_ENABLED	
	static struct tm _timeinfo;
#endif

	std::vector<std::unique_ptr<HAPPlugin>> _plugins;

	unsigned long _minimalPluginInterval;
	unsigned long _previousMillis;

	void handleEvents( int eventCode, struct HAPEvent eventParam );
	// void handlePluginEvents( int eventCode, struct HAPEvent eventParam );

    MemberFunctionCallable<HAPServer> listenerMemberFunction2;	

#if HAP_GENERATE_XHM	
	QRCode _qrcode;
#endif	

	HAPPairings _pairings;

#if HAP_UPDATE_SERVER 	
	HAPUpdate _updater;
#endif

	String _curLine;
	uint16_t _port;

	HAPVersion version() {
		return _firmware.version;
	}



	//
	// HTTP Paths
	//

	// Preferences ?
	void handlePreferences(HAPClient* hapClient);

	// /accessories
	void handleAccessories(HAPClient* hapClient);

	// /characteristics
	void handleCharacteristicsGet(HAPClient* hapClient);	
	void handleCharacteristicsPut(HAPClient* hapClient, String body);	

	// Identify
	void handleIdentify(HAPClient* hapClient);





private:
	struct {
		char name[MAX_FIRMWARE_NAME_LENGTH];
		HAPVersion version;
	} _firmware;

	char _brand[MAX_BRAND_LENGTH];

	bool _firmwareSet;




	//
	// Bonjour
	//
	bool updateServiceTxt();


	//
	// HTTP
	// 

	// parsing
	void processIncomingRequest(HAPClient* hapClient);
	void processIncomingEncryptedRequest(HAPClient* hapClient);

	void processIncomingLine(HAPClient* hapClient, String line);
	void processPathParameters(HAPClient* hapClient, String line, int curPos);

	String parseRequest(HAPClient* hapClient, char* decrypted, size_t decrypted_len);
	bool handlePath(HAPClient* hapClient, String bodyData);

	// Connection states
	void handleClientState(HAPClient* hapClient);
	void handleClientAvailable(HAPClient* hapClient);
	void handleClientDisconnect(HAPClient hapClient);



	//
	// Pairing
	//

	// Pair-Setup states
	bool handlePairSetupM1(HAPClient* hapClient);
	bool handlePairSetupM3(HAPClient* hapClient);
	bool handlePairSetupM5(HAPClient* hapClient);


	// Pair-Verify states
	bool handlePairVerifyM1(HAPClient* hapClient);
	bool handlePairVerifyM3(HAPClient* hapClient);



	//
	// Sending responses
	// 
	bool sendResponse(HAPClient* hapClient, TLV8* response, bool chunked = true);
	bool sendEncrypt(HAPClient* hapClient, String httpStatus, String plainText, bool chunked = true);



	//
	// encrpytion / decryption
	//
	char* encrypt(uint8_t *message, size_t length, int* encrypted_len, uint8_t* key, uint16_t encryptCount);
	// int decryt(uint8_t* encrypted, int len, char* decrypted, uint8_t** saveptr);


	//
	// TLV8
	//
	static bool encode(HAPClient* hapClient);
	

	// int32_t getValueForCharacteristics(int aid, int iid, char* out, size_t* outSize);
	// characteristics* getCharacteristics(int aid, int iid);


	const char* __HOMEKIT_SIGNATURE;
};

extern HAPServer hap;

#endif /* HAPSERVER_HPP_ */
