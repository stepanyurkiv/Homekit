//
// HAPServer.cpp
// Homekit
//
//  Created on: 08.08.2017
//      Author: michael
//

#include <WiFi.h>
#include <WString.h>
#include <algorithm>

#include "HAPServer.hpp"
#include "HAPLogger.hpp"
#include "HAPHelper.hpp"
#include "HAPBonjour.hpp"
#include "HAPDeviceID.hpp"

#include "HAPEncryption.hpp"

#include "srp.h"
#include "chacha20_poly1305.h"
#include "concat.h"

#if HAP_DEBUG
#include "esp_system.h"
#endif

#if HAP_USE_WOLFSSL_HKDF
#include "hkdf.h"
#else
#include "m_hkdf.h"
#endif

#include <mbedtls/version.h>
#include <sodium.h>

#include "curve25519.h"
#include "ed25519.h"


#define IS_BIG_ENDIAN (*(uint16_t *)"\0\xff" < 0x100)

#define HAP_RESET_EEPROM 1


//
// init static variables
//
struct tm HAPServer::_timeinfo;




HAPServer::HAPServer(uint16_t port, uint8_t maxClients)
:  _server(port)
, __HOMEKIT_SIGNATURE("\x25\x48\x4f\x4d\x45\x4b\x49\x54\x5f\x45\x53\x50\x33\x32\x5f\x46\x57\x25")
{
	_port = port;
	//	_clients.resize(maxClients);
	_firmwareSet = false;

	_previousMillis = 0;
	_minimalPluginInterval = HAP_MINIMAL_PLUGIN_INTERVAL;
	_accessorySet = new HAPAccessorySet();
}

HAPServer::~HAPServer() {
	// TODO Auto-generated destructor stub
	delete _accessorySet;
}

bool HAPServer::begin() {


	int error_code = 0;

	HAPDeviceID::generateID();

#if HAP_DEBUG
	LogD( F("\nRunning on:"), true);
	LogD("===================================================", true);	
	LogD("Device ID:    " + HAPDeviceID::deviceID(), true);	
	LogD("Chip ID:      " + HAPDeviceID::chipID(), true);
	LogD("MAC address:  " + WiFi.macAddress(), true);


	esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);

	LogD("", true);	
	LogD("ESP32:", true);
    LogD("   features:  " + String(chip_info.features), true); 
    LogD("   cores:     " + String(chip_info.cores), true); 
    LogD("   revision:  " + String(chip_info.revision), true); 

    LogD("", true);	
	LogD("Flash chip:", true);
	LogD("   size:      " + String(ESP.getFlashChipSize()), true);
	LogD("   speed:     " + String(ESP.getFlashChipSpeed()), true);
	LogD("   mode:      " + String(ESP.getFlashChipMode()), true);

    LogD("", true);	
    LogD("Endian:       ", false);
    LogD(IS_BIG_ENDIAN ? "BIG" : "little", true);


    char mbedtlsVersion[32];
    mbedtls_version_get_string_full(mbedtlsVersion);

    LogD("", true);	
    LogD("Versions:", true);    
	LogD("   SDK:       " + String(ESP.getSdkVersion()), true);
	LogD("   mbedtls:   " + String(mbedtlsVersion), true);
	LogD("   libsodium: " + String(sodium_version_string()), true);	
	LogD("===================================================", false);
	LogD("", true);
	LogD("Now starting ...", true);
#endif			


	LogV("Loading settings ...", false);

	LogV("OK", true);


	// Todo: Move pairings to accessorySet
	LogV("Loading pairings ...", false);	
	if (_pairings.begin()) {
#if HAP_RESET_EEPROM
		LogV("OK", true);

		LogW("Reset EEPROM ...", false);
		_pairings.resetEEPROM();
		LogW("OK", true);
#endif
		_pairings.load();		
	} 

#if HAP_RESET_EEPROM
#else	
	LogV("OK", true);
#endif

	LogI("Loaded " + String(_pairings.size()) + " pairings from EEPROM!", true);

#if HAP_DEBUG
	_pairings.print();
#endif


	if ( isPaired() ){
		LogV("Loading long term keys ...", false);	
		_longTermContext = (struct HAPLongTermContext*) calloc(1, sizeof(struct HAPLongTermContext));
		if (_longTermContext == NULL) {
			LogE( F("[ERROR] Initializing struct _longTermContext failed!"), true);
			return false;
		}
	
		_longTermContext->publicKey = (uint8_t*) malloc(sizeof(uint8_t) * ED25519_PUBLIC_KEY_LENGTH);
		_longTermContext->publicKeyLength = ED25519_PUBLIC_KEY_LENGTH;
		_longTermContext->privateKey = (uint8_t*) malloc(sizeof(uint8_t) * ED25519_PRIVATE_KEY_LENGTH);
		_longTermContext->privateKeyLength = ED25519_PRIVATE_KEY_LENGTH;

		_pairings.saveLTPK(_longTermContext->publicKey);
 		_pairings.saveLTSK(_longTermContext->privateKey);

		LogV("OK", true);
	}




	LogV( F("\nStarting event manager ..."), false);
  	listenerMemberFunction2.mObj = this;
  	listenerMemberFunction2.mf = &HAPServer::handleEvents;
  	
  	_eventManager.addListener( EventManager::kEventEvent, &listenerMemberFunction2 );
  	LogD( "\nNumber of listeners is:  ", false );
  	LogD( String(_eventManager.numListeners()), true );
	LogV(F("OK"), true);


	

	LogV("Starting encrpytion engine ...", false);
	error_code = HAPEncryption::begin();
	if (error_code != 0){
		LogE(F("[ERROR] Failed to initialize libsodium!"), true);		
	} else {
		LogV("OK", true);
	}




	LogV( F("Setup accessory ..."), false);
	_accessorySet->setModelName(HAP_HOSTNAME);
	_accessorySet->setConfigurationNumber(HAP_CONFIGURATION_NUMBER);
	_accessorySet->setAccessoryType(HAP_ACCESSORY_TYPE_BRIDGE);
	_accessorySet->setPinCode(HAP_PIN_CODE);
	_accessorySet->begin();
	LogV("OK", true);




#if HAP_UPDATE_SERVER

	//
	// Starting Arduino OTA
	//
	LogV( F("Starting Arduino OTA ..."), false);
	_updater.begin();
	LogV( F("OK"), true);

	//
	// Looking for Update on AWS
	//
	LogV( F("Check for web update ..."), false);
	//_updater.setLocalVersion( &_firmware.version );
#if SSL_ENABLED
	_updater.setHostAndPort("https://192.168.178.151", HAP_UPDATE_SERVER_PORT);
#else
	_updater.setHostAndPort(HAP_UPDATE_SERVER_URL, HAP_UPDATE_SERVER_PORT);
#endif

	if ( _updater.checkUpdateAvailable(&_firmware.version) ) {
		_updater.execOTA();
	}
	LogV( F("OK - No update available"), true);

#endif




	
#if HAP_NTP_ENABLED
	LogV( F("Starting NTP client ..."), false);
	configTzTime(HAP_NTP_TZ_INFO, HAP_NTP_SERVER_URL);

	if (getLocalTime(&_timeinfo, 10000)) {  // wait up to 10sec to sync
		//Serial.println(&_timeinfo, "Time set: %B %d %Y %H:%M:%S (%A)");		
		LogV(F("OK"), true);
		LogV("Set time to: " + timeString(), true);
	} else {
		LogV(F("[ERROR] Time not set!"), true);	
  	}


#endif




  	// 
  	// Loading plugins
  	// 
  	LogI( F("Loading plugins ..."), true);

	auto &factory = HAPPluginFactory::Instance();        
    std::vector<String> names = factory.names();    

    for(std::vector<String>::iterator it = names.begin(); it != names.end(); ++it) {
    	//Serial.println(*it);
    	auto plugin = factory.getPlugin(*it);

    	if ( plugin->isEnabled()) {

			LogI("   - ENABLED  " + plugin->name(), false);
    		LogD(" (v" + String(plugin->version()) + ")", false);	
    		LogD(" of type: " + String(plugin->type()), false);
    		LogI("", true);

	    	if ( plugin->type() == HAP_PLUGIN_TYPE_ACCESSORY) {
	    		HAPAccessory *accessory = plugin->init();
	    		_accessorySet->addAccessory(accessory);    		
	    	}

	    	if (plugin->interval() > 0 && plugin->interval() < _minimalPluginInterval) {
	    		_minimalPluginInterval = plugin->interval();	
	    	}
	    	
	    	_plugins.push_back(std::move(plugin));
    	} else {
    		LogW("   - DISABLED " + plugin->name(), false);
    		LogD(" (v" + String(plugin->version()) + ")", false);	
    		LogD(" of type: " + String(plugin->type()), false);
    		LogW("", true);
    	}
	}





	//
	// Starting HAP server
	// 
	LogV( F("Starting HAP server ..."), false);
	_server.begin();
	_server.setNoDelay(true);
	LogV(F("OK"), true);
  	


	// 
	// Bonjour
	// 

	// Set up mDNS responder:
	// - first argument is the domain name, in this example
	//   the fully-qualified domain name is "esp8266.local"
	// - second argument is the IP address to advertise
	//   we send our IP address on the WiFi network
	LogV( F("Advertising Bonjour service ..."), false);
	if (!MDNS.begin(HAP_HOSTNAME)) {
		LogE( F("Error setting up MDNS responder!"), true);
		return false;
	}

	// Add service to MDNS-SD
	MDNS.addService("_hap", "_tcp", _port);

	if ( !updateServiceTxt() ){
		LogE( F("Error advertising HAP service!"), true);
		return false;
	}

	LogV(F("OK"), true);



	// 
	// QR Code generation
	// 

#if HAP_GENERATE_XHM	

	/*
	 * Generate setupID and xmi uri
	 */
	_accessorySet->generateSetupID();
	
	//LogD("Homekit setupID: ", false);
	//LogD(_accessorySet->et.setupID(), true);


	LogV("Homekit X-HM URI: ", false);
	LogV(_accessorySet->xhm(), true);

	/*
	 * Generate QR Code
	 */
	uint8_t qrcodeData[qrcode_getBufferSize(3)];
	qrcode_initText(&_qrcode, qrcodeData, 3, ECC_HIGH, _accessorySet->xhm());


#if HAP_PRINT_QRCODE

	for (uint8_t y = 0; y < _qrcode.size; y++) {
		// Left quiet zone
		Serial.print("        ");
		// Each horizontal module
		for (uint8_t x = 0; x < _qrcode.size; x++) {
            // Print each module (UTF-8 \u2588 is a solid block)
			Serial.print(qrcode_getModule(&_qrcode, x, y) ? "\u2588\u2588": "  ");
		}
		Serial.print("\n");
	}
#endif

#endif


	// 
	// Startup completed
	// 
	// LogI( F("OK"), true);
	LogI("Homekit pin code: ", false);
	LogI(_accessorySet->pinCode(), true);

	
#if HAP_DEBUG	
	LogD(_accessorySet->describe(), true);    
#endif


	return true;
}


bool HAPServer::updateServiceTxt() {
	/*
	mdns_txt_item_t hapTxtData[8] = {
        {(char*)"c#"    ,(char*)"2"},                   // c# - Configuration number
        {(char*)"ff"    ,(char*)"0"},                   // ff - feature flags
        {(char*)"id"    ,(char*)"11:22:33:44:55:66"},   				// id - unique identifier
        {(char*)"md"	,(char*)"Huzzah32"},            // md - model name
        {(char*)"pv"    ,(char*)"1.0"},                 // pv - protocol version
        {(char*)"s#"    ,(char*)"1"},                   // s# - state number
        {(char*)"sf"    ,(char*)"1"},                   // sf - Status flag
        {(char*)"ci"    ,(char*)"2"}                    // ci - Accessory category indicator
    }; 

    return MDNS.addServiceTxtSet((char*)"_hap", "_tcp", 8, hapTxtData);


    */
#if HAP_GENERATE_XHM
	mdns_txt_item_t hapTxtData[9];
#else
	mdns_txt_item_t hapTxtData[8];
#endif
	uint8_t *baseMac = HAPDeviceID::generateID();

	// c# - Configuration number
	// Current configuration number. Required.
	// Must update when an accessory, service, or characteristic is added or removed 
	// 
	// Accessories must increment the config number after a firmware update. 
	// This must have a range of 1-4294967295 and wrap to 1 when it overflows. 
	// This value must persist across reboots, power cycles, etc.
	hapTxtData[0].key 		= (char*) "c#";
	hapTxtData[0].value 	= (char*) malloc(sizeof(char) * HAPHelper::numDigits(_accessorySet->configurationNumber()) );
	sprintf(hapTxtData[0].value, "%d", _accessorySet->configurationNumber() );		

	// id - unique identifier
	hapTxtData[1].key 		= (char*) "id";
	hapTxtData[1].value 	= (char*) malloc(sizeof(char) * 18);
	sprintf(hapTxtData[1].value, "%02X:%02X:%02X:%02X:%02X:%02X", baseMac[0], baseMac[1], baseMac[2], baseMac[3], baseMac[4], baseMac[5]);
	
	// ff - feature flags
	// Supports HAP Pairing. This flag is required for all HomeKit accessories.
	hapTxtData[2].key 		= (char*) "ff";
	hapTxtData[2].value 	= (char*) malloc(sizeof(char));
	sprintf(hapTxtData[2].value, "%d", isPaired() );
	
	// md - model name	
	hapTxtData[3].key 		= (char*) "md";
	hapTxtData[3].value 	= (char*) malloc(sizeof(char) * strlen(_accessorySet->modelName()));
	sprintf(hapTxtData[3].value, "%s", _accessorySet->modelName());

	// pv - protocol version
	hapTxtData[4].key 		= (char*) "pv";
	hapTxtData[4].value 	= (char*) "0.0";
	
	// s# - state number
	// must have a value of "1"
	hapTxtData[5].key 		= (char*) "s#";
	hapTxtData[5].value 	= (char*) "1";

	// sf - feature flags
	// Status flags (e.g. "0x04" for bit 3). Value should be an unsigned integer. 
	// Required if non-zero.
	// 1 if not paired
	// 0 if paired ??
	hapTxtData[6].key 		= (char*) "sf";
	hapTxtData[6].value 	= (char*) malloc(sizeof(char));
	sprintf(hapTxtData[6].value, "%d", !isPaired() );

	 // ci - Accessory category indicator
	hapTxtData[7].key 		= (char*) "ci";
	hapTxtData[7].value 	= (char*) malloc(sizeof(char) * HAPHelper::numDigits(_accessorySet->accessoryType()) );
	sprintf(hapTxtData[7].value, "%d", _accessorySet->accessoryType() );


#if HAP_GENERATE_XHM
	// sh - Required for QR Code 
	hapTxtData[8].key 		= (char*) "sh";
	hapTxtData[8].value 	= (char*) malloc(sizeof(char) * strlen(_accessorySet->setupHash()) );
	sprintf(hapTxtData[8].value, "%s", _accessorySet->setupHash() );

    return MDNS.addServiceTxtSet((char*)"_hap", "_tcp", 9, hapTxtData);
#else
    return MDNS.addServiceTxtSet((char*)"_hap", "_tcp", 8, hapTxtData);
#endif  
 
}



void HAPServer::handle() {


	// Handle new clients
	WiFiClient client = _server.available();
	if (client) {
		HAPClient hapClient;

		// New client connected
		hapClient.client = client;
		hapClient.state = CLIENT_STATE_CONNECTED;

		handleClientState(&hapClient);
	}



	for (auto& hapClient : _clients) {

		// Connected
		if (hapClient.client.connected()) {

			// Available
			unsigned long timeout = 100;
			unsigned long previousMillis = millis();
			while ( millis() - previousMillis < timeout) {

				delay(1);

				if (hapClient.client.available()) {
					hapClient.state = CLIENT_STATE_AVAILABLE;
					handleClientState(&hapClient);
					break;
				}

				// Idle
				hapClient.state = CLIENT_STATE_IDLE;
			}


		} else {
			// Disconnected
			hapClient.state = CLIENT_STATE_DISCONNECTED;
			handleClientState(&hapClient);
		}

		// LogV( "HAPClient state " + hapClient.getClientState(), true );
		// delay(1);

	}

	// Handle ntp client
#if HAP_NTP_ENABLED
	getLocalTime(&_timeinfo);
    //Serial.println(&_timeinfo, HAP_NTP_TIME_FORMAT);  	
#endif



	// Handle Arduino OTA
#if HAP_UPDATE_SERVER	
	_updater.handle();
#endif	


	// Handle any events that are in the queue
	_eventManager.processEvent();

	// Handle plugins
	unsigned long currentMillis = millis();
	if (currentMillis - _previousMillis >= _minimalPluginInterval) {
		// save the last time you blinked the LED
		_previousMillis = currentMillis;

		for (auto & plugin : _plugins) {			
			plugin->handle(_accessorySet);					
		}

	}	
	
}

#if HAP_NTP_ENABLED

String HAPServer::timeString(){
	char ptr[64];
	strftime (ptr, 64, HAP_NTP_TIME_FORMAT, &_timeinfo );
	return String(ptr);
}

#endif


void HAPServer::handleClientDisconnect(HAPClient hapClient) {
	std::vector<HAPClient>::iterator position = std::find(_clients.begin(), _clients.end(), hapClient);
	if (position != _clients.end()) { // == myVector.end() means the element was not found

		if (position->client.connected() ) {

			LogW("Client disconnecting", true);
			position->client.stop();
		}

		_clients.erase(position);
		return;
	}
	//LogE( F( "FAILED"), true );
}

void HAPServer::handleClientState(HAPClient* hapClient) {
	switch(hapClient->state) {
		case CLIENT_STATE_DISCONNECTED:
			LogV( ">>> client [" + hapClient->client.remoteIP().toString() + "] disconnected", true );
			handleClientDisconnect( *hapClient );

			break;
		case CLIENT_STATE_CONNECTED:
			LogV( "<<< client [" + hapClient->client.remoteIP().toString() + "] connected", true );		
			_clients.push_back(*hapClient);

			break;
		case CLIENT_STATE_AVAILABLE:
			LogV( "<<< client [" + hapClient->client.remoteIP().toString() + "] available: ", false );
			LogD( String(hapClient->client.available()), true );

			handleClientAvailable(hapClient);

			break;
		case CLIENT_STATE_SENT:
			LogV( F("<<< client sent"), true );
			break;
		case CLIENT_STATE_RECEIVED:
			LogV( F("<<< client received"), true );
			break;
		case CLIENT_STATE_IDLE:
			LogV( "<<< client [" + hapClient->client.remoteIP().toString() + "] idle", true );
			break;
	}
}


void HAPServer::handleClientAvailable(HAPClient* hapClient) {
	_curLine = "";
	LogD( F("Handle client available"), true);
	while ( hapClient->client.available() ) {

		if (hapClient->isEncrypted) {
			processIncomingEncryptedRequest( hapClient );	
		} else {
			processIncomingRequest( hapClient );	
		}
		
		delay(1);
	}

#if HAP_DEBUG
	LogD(_curLine, true);
#endif

	if ( hapClient->client.connected() ) {
		hapClient->state = CLIENT_STATE_IDLE;
	} else {
		hapClient->state = CLIENT_STATE_DISCONNECTED;
	}

	// Update client state *print*
	handleClientState( hapClient );

	// clear request
	hapClient->request.clear();
}


void HAPServer::processIncomingEncryptedRequest(HAPClient* hapClient){
	LogD( F("<<< Handle encrypted request ..."), false);

	//
    // Each HTTP message is split into frames no larger than 1024 bytes
    // 
    // Each frame has the following format:
    // 
    // < 2:    AAD for little endian length of encrypted data (n) in bytes>
    // < n:    encrypted data according to AEAD algorithm, up to 1024 bytes>
    // <16:    authTag according to AEAD algorithm>
    // 

	String bodyData = "";

	while ( hapClient->client.available() )	{


		//
	    // AAD
	    //
	    // < 2:    AAD for little endian length of encrypted data (n) in bytes>
	    // uint8_t aad[HAP_AAD_LENGTH];
	    // aad[0] = (uint8_t) (length >> 8);   // Get upper byte of 16-bit var;
	    // aad[1] = (uint8_t) length;          // Get lower byte of 16-bit var;
		uint8_t AAD[HAP_AAD_LENGTH];		
		hapClient->client.readBytes(AAD, HAP_AAD_LENGTH);
		
		uint16_t trueLength = ((uint16_t)AAD[1] << 8) | AAD[0];
		int availableSize = hapClient->client.available() - HAP_ENCRYPTION_HMAC_SIZE;	// 16 is the size of the HMAC

		Serial.printf("AAD: %02X%02X - %d\n", AAD[0], AAD[1], trueLength);				
		Serial.printf("availableSize: %d\n", availableSize);

		LogD("<<< Need " + String(trueLength) + " bytes and have " + String(availableSize) + " bytes", true);
		while (trueLength > availableSize) {
			// The packet is bigger than the available data; wait till more comes in
			delay(1);
		}

	    // 
	    // nonce
	    //
	    // The 32-bit fixed-common part of the 96-bit ( 12? ) nonce 
	    // is all zeros: 00 00 00 00.
	    //
	    // length 8
	    //
	    // < n:    encrypted data according to AEAD algorithm, up to 1024 bytes>
	    //
	    // Needs to be incremented each time it is called after the 1st 4 bytes
		
    	LogW("VOR decryptCount: " + String(hapClient->encryptionContext->decryptCount), true);
		int nonce = hapClient->encryptionContext->decryptCount;

		//
		// cipherText
		uint8_t cipherText[trueLength];		
		hapClient->client.readBytes(cipherText, trueLength);
		
		Serial.println("cipherText:");
		HAPHelper::arrayPrint(cipherText, trueLength);


		// 
		// hmac
		uint8_t hmac[HAP_ENCRYPTION_HMAC_SIZE];	// 16 is the size of the HMAC
		availableSize = hapClient->client.available();
		LogD("<<< Need " + String(HAP_ENCRYPTION_HMAC_SIZE) + " bytes and have " + String(availableSize) + " bytes", true);
		while ( HAP_ENCRYPTION_HMAC_SIZE > availableSize ) {
			// The packet is bigger than the available data; wait till more comes in
			delay(1);
		}

		hapClient->client.readBytes(hmac, HAP_ENCRYPTION_HMAC_SIZE);

		uint8_t decrypted[trueLength];
		HAPEncryption::verifyAndDecrypt(decrypted, cipherText, trueLength, hmac, AAD, nonce, hapClient->encryptionContext->decryptKey);		

		// increment decrypt counter
		hapClient->encryptionContext->decryptCount++;

		bodyData = parseRequest(hapClient, (char*)decrypted, trueLength);
		handlePath(hapClient, bodyData);		
	}
	
	return;


#if 0	
	uint8_t buffer[HAP_ENCRYPTION_BUFFER_SIZE];
	int bufferLength = 0;

	while ( hapClient->client.available() )	{
		buffer[bufferLength] = hapClient->client.read();
		bufferLength++;

		// TODO:
		if (bufferLength == HAP_ENCRYPTION_BUFFER_SIZE) {
			LogE("[ERROR] client data to big!!!", true);
			return;
		}
	}


#if HAP_DEBUG
	LogD( F("\nReceived encrypted message..."), true);
	HAPHelper::arrayPrint(buffer, bufferLength);
#endif	

	char* decrypted = (char*) calloc(1, bufferLength);
	if (decrypted == NULL) {
		LogE("[ERROR] calloc failed", true);
		return;
	}

	uint8_t* saveptr = NULL;
	int decrypted_len = 0;

	String bodyData = "";

	/*
	for (decrypted_len = decrypt(buffer, bufferLength, decrypted, &saveptr); decrypted_len; 
		decrypted_len = decrypt(buffer, bufferLength, decrypted, &saveptr)) {

		Serial.println("!!!!");
	}
	*/

	decrypted_len = decrypt(buffer, bufferLength, decrypted, &saveptr);


	Serial.printf("%d - %d: %s\n", strlen(decrypted), decrypted_len, decrypted);    

	bodyData = parseRequest(hapClient, decrypted, strlen(decrypted));

	free(decrypted);

#if HAP_DEBUG
	if (bodyData.length() > 0)
		LogD("bodyData: " + bodyData, true);
#endif

#endif

	

}

bool HAPServer::handlePath(HAPClient* hapClient, String bodyData){
		// /accessories
	if ( (hapClient->request.path == "/accessories") && (hapClient->request.method == METHOD_GET) ) {
		handleAccessories( hapClient );
	} 
	// /characteristics
	else if ( hapClient->request.path == "/characteristics" ) {

		// GET
		if ( hapClient->request.method == METHOD_GET ) {
			handleCharacteristicsGet( hapClient );
		} 
		// PUT
		else if ( (hapClient->request.method == METHOD_PUT) && (hapClient->request.contentType == F("application/hap+json")) ) {
			handleCharacteristicsPut( hapClient, bodyData );
		}		
	} else {
		LogW("Not yet implemented! >>> client [" + hapClient->client.remoteIP().toString() + "] ", false);
		LogW("requested -> method: " + String(hapClient->request.method) + " - path: " + String(hapClient->request.path), true);

		return false;
	}

	hapClient->state = CLIENT_STATE_IDLE;
	return true;
}


String HAPServer::parseRequest(HAPClient* hapClient, char* msg, size_t msg_len){

	String bodyData;

	int curPos = 0;
	for (int i = 0; i < msg_len; i++) {
    		//Serial.print(decrypted[i]);
		if ( msg[i] == '\r' && msg[i + 1] == '\n' ) {
			processIncomingLine(hapClient, String(msg).substring(curPos, i));
			i++;
			if (i - curPos == 2) {
				curPos = i + 1;
				break;
			}
			curPos = i + 1;
		}

	}	

	if (curPos + hapClient->request.contentLength == msg_len) {
		bodyData += String(msg).substring(curPos);
	}

	return bodyData;
}





#if 0
int HAPServer::decrypt(uint8_t* encrypted, int len, char* decrypted, uint8_t** saveptr)
{
	

	uint64_t nonceEncryptCount = 0;

	//HAPEncryption::verifyAndDecrypt();

	return 0;

#if 0	
    uint8_t* ptr;
    if (*saveptr == NULL) {
        ptr = (uint8_t*)encrypted;
    }
    else if (*saveptr < encrypted + len) {
        ptr = *saveptr;
    }
    else if (*saveptr == encrypted + len){
        printf("[HAP][INFO] _decrypt end %d\n", (int)((uint8_t*)*saveptr - encrypted));
        LogD("[DECRYPT] Decryption complete!", true);
        return 0;
    }
    else {
        //printf("BUG? BUG? BUG?\n");
        LogE("[DECRYPT] [ERROR] Decryption failed! BUG BUG BUG ?", true);
        return -1;
    }

    int decrypted_len = ptr[1] * 256 + ptr[0];
    uint8_t nonce[12] = {0,};
    nonce[4] = _pairSetup->decryptCount % 256;
    nonce[5] = _pairSetup->decryptCount++ / 256;


#if 1
    Serial.print("nonce: ");
    HAPHelper::arrayPrint(nonce, 12);

    //Serial.print("saveptr: ");
	//HAPHelper::arrayPrint(*saveptr, len);

    Serial.print("ptr: ");
    HAPHelper::arrayPrint(ptr, len);

    Serial.print("encrypted: ");
    HAPHelper::arrayPrint(encrypted, len);
#endif

#if 0
	Serial.println("### decrypt: ");
	Serial.print("_pairSetup->sessionKey: ");
	HAPHelper::arrayPrint(_pairSetup->sessionKey, CURVE25519_SECRET_LENGTH);
	Serial.print("_pairSetup->encryptKey: ");
	HAPHelper::arrayPrint(_pairSetup->encryptKey, HKDF_KEY_LEN);
	Serial.print("_pairSetup->decryptKey: ");
	HAPHelper::arrayPrint(_pairSetup->decryptKey, HKDF_KEY_LEN);
#endif 


    if (chacha20_poly1305_decrypt_with_nonce(nonce, _pairSetup->decryptKey, ptr, AAD_LENGTH, 
                ptr+AAD_LENGTH, decrypted_len + CHACHA20_POLY1305_AUTH_TAG_LENGTH, (uint8_t*)decrypted) < 0) {
        LogW("[ERROR] chacha20_poly1305_decrypt_with_nonce failed\n", true);
        _pairSetup->decryptCount = 0;
        //decrypted_len = 0;
        return 0;
    }

    
    *saveptr = ptr + decrypted_len + CHACHA20_POLY1305_AUTH_TAG_LENGTH + AAD_LENGTH;

    return decrypted_len;

#endif

}
#endif


char* HAPServer::encrypt(uint8_t *message, size_t length, int* encrypted_len, uint8_t* key, uint16_t encryptCount) {

	char* encrypted = (char*) calloc(1, length + (length / HAP_ENCRYPTION_BUFFER_SIZE + 1) * (HAP_AAD_LENGTH + CHACHA20_POLY1305_AUTH_TAG_LENGTH) + 1);

	uint8_t nonce[12] = {0,};
	uint8_t* decrypted_ptr = (uint8_t*)message;
	uint8_t* encrypted_ptr = (uint8_t*)encrypted;

	int err_code = 0;

	while (length > 0) {
		int chunk_len = (length < HAP_ENCRYPTION_BUFFER_SIZE) ? length : HAP_ENCRYPTION_BUFFER_SIZE;
		length -= chunk_len;

		uint8_t aad[HAP_AAD_LENGTH];
		aad[0] = chunk_len % 256;
		aad[1] = chunk_len / 256;

		memcpy(encrypted_ptr, aad, HAP_AAD_LENGTH);
		encrypted_ptr += HAP_AAD_LENGTH;
		*encrypted_len += HAP_AAD_LENGTH;

		nonce[4] = encryptCount % 256;
		nonce[5] = encryptCount++ / 256;

		err_code = chacha20_poly1305_encrypt_with_nonce(nonce, key, aad, HAP_AAD_LENGTH, decrypted_ptr, chunk_len, encrypted_ptr);
		if (err_code != 0 ) {
			LogE("[ERROR] Encrypting failed!", true);
		}

		decrypted_ptr += chunk_len;
		encrypted_ptr += chunk_len + CHACHA20_POLY1305_AUTH_TAG_LENGTH;
		*encrypted_len += (chunk_len + CHACHA20_POLY1305_AUTH_TAG_LENGTH);
	}


	//_pairSetup->encryptCount = 0;
	return encrypted;
}


void HAPServer::processIncomingRequest(HAPClient* hapClient){

	const byte b = hapClient->client.read();

	if ( (char) b == '\n' ) {
		// if the current line is blank, you got two newline characters in a row.
		// that's the end of the client HTTP request, so send a response:
		if (_curLine.length() == 0) {


#if HAP_DEBUG
			// Handle data
			LogD( F("request: "), false);
			LogD(hapClient->request.toString(), true);
#endif

			// /identify
			if ( (hapClient->request.path == "/identify") && (hapClient->request.method == METHOD_POST) ) {
				handleIdentify(hapClient);
				hapClient->state = CLIENT_STATE_IDLE;
			}

			// /preferences
			else if (hapClient->request.path == "/preferences") {
				handlePreferences( hapClient );
				hapClient->state = CLIENT_STATE_IDLE;
			}


			// has content
			else if ( hapClient->request.contentLength > 0) {
				// encode tlv8
				if ( hapClient->request.contentType == F("application/pairing+tlv8") )  {
					if ( !encode(hapClient) ) {
						LogE( F("ERROR: Decoding pairing request failed!"), true);
						hapClient->state = CLIENT_STATE_DISCONNECTED;
						return;
					}

					// pair-setup M1
					if ( (hapClient->request.path == F("/pair-setup") ) && (hapClient->pairState == PAIR_STATE_M1) ) {
						if (!handlePairSetupM1( hapClient ) ) {
							LogE( F("ERROR: Pair-setup failed at M1!"), true);
							hapClient->state = CLIENT_STATE_DISCONNECTED;
						}
					}

					// pair-setup M3
					else if ( (hapClient->request.path == F("/pair-setup") ) && (hapClient->pairState == PAIR_STATE_M3) ) {
						if (!handlePairSetupM3( hapClient ) ) {
							LogE( F("ERROR: Pair-setup failed at M3!"), true);
							hapClient->state = CLIENT_STATE_DISCONNECTED;
						}
					}

					// pair-setup M5
					else if ( (hapClient->request.path == F("/pair-setup") ) && (hapClient->pairState == PAIR_STATE_M5) ) {
						if ( !handlePairSetupM5( hapClient ) ) {
							LogE( F("ERROR: Pair-setup failed at M5!"), true);
							hapClient->state = CLIENT_STATE_DISCONNECTED;
						}
					}

					// pair-verify M1
					else if ( (hapClient->request.path == F("/pair-verify") ) && (hapClient->verifyState == VERIFY_STATE_M1) ) {
						if ( !handlePairVerifyM1( hapClient ) ) {
							LogE( F("ERROR: Pair-verify failed at M1!"), true);
							hapClient->state = CLIENT_STATE_DISCONNECTED;
						}
					}

					// pair-verify M3
					else if ( (hapClient->request.path == F("/pair-verify") ) && (hapClient->verifyState == VERIFY_STATE_M3) ) {
						if ( !handlePairVerifyM3( hapClient ) ) {
							LogE( F("ERROR: Pair-verify failed at M3!"), true);
							hapClient->state = CLIENT_STATE_DISCONNECTED;
						}
					}
				}
			}

			_curLine = "";
			return;
		} else {    					// if you got a newline, then clear currentLine:
			// Handle lines
			processIncomingLine(hapClient, _curLine);
			_curLine = "";
		}
	} else if ( (char) b != '\r') {  	// if you got anything else but a carriage return character,		
		_curLine += (char) b;      		// add it to the end of the currentLine
	}
}


void HAPServer::processPathParameters(HAPClient* hapClient, String line, int curPos){
	
	int index = line.indexOf("?", curPos);

	if ( index == -1) {
		// no ? in request
		hapClient->request.path = line.substring(curPos, line.indexOf(" ", curPos));
		hapClient->request.params = std::map<String, String>();
	} else {
		hapClient->request.path = line.substring(curPos, index);
		
		//Serial.print("path: ");
		//Serial.println(hapClient->request.path);

		curPos = index + 1;
		String paramStr = line.substring(curPos, line.indexOf(" ", curPos));


		//Serial.println("paramStr:");
		//Serial.println(paramStr);
		

		do {
			curPos = 0;
			int endIndex = paramStr.indexOf("&");		
			if (endIndex == -1){
				endIndex = paramStr.length();		
			}

			String keyPair = paramStr.substring(curPos, endIndex); 
			//Serial.printf("tmp: %s\n", keyPair.c_str());

			int equalIndex = keyPair.indexOf("=");

			/*
			Serial.print("key: "); 
			Serial.print(keyPair.substring(0, equalIndex));
			Serial.print(" - value: "); 
			Serial.println(keyPair.substring(equalIndex + 1));
			*/

			hapClient->request.params[keyPair.substring(0, equalIndex)] = keyPair.substring(equalIndex + 1); 

			paramStr = paramStr.substring(endIndex + 1); 
		} while ( paramStr.length() > 0 );		
	}
}


void HAPServer::processIncomingLine(HAPClient* hapClient, String line){

	// Print Line
#if HAP_DEBUG
	LogD( line, true );
#endif

	int curPos = 0;

	// Method
	if ( line.startsWith("POST ") ) {
		hapClient->request.method = METHOD_POST;
		curPos = 5;
		// Path
		processPathParameters( hapClient, line, curPos);
		//hapClient->request.path = line.substring(curPos, line.indexOf(" ", curPos));
	} else if ( line.startsWith("GET ") ) {
		hapClient->request.method = METHOD_GET;
		curPos = 4;
		// Path
		processPathParameters( hapClient, line, curPos);
		//hapClient->request.path = line.substring(curPos, line.indexOf(" ", curPos));
	} else if ( line.startsWith("PUT ") ) {
		hapClient->request.method = METHOD_PUT;
		curPos = 4;
		// Path
		processPathParameters( hapClient, line, curPos);
		//hapClient->request.path = line.substring(curPos, line.indexOf(" ", curPos));
	} else if ( line.startsWith("DELETE ") ) {
		hapClient->request.method = METHOD_DELETE;
		curPos = 7;
		// Path
		processPathParameters( hapClient, line, curPos);
		//hapClient->request.path = line.substring(curPos, line.indexOf(" ", curPos));
	}

	if (line.length() == 0) {
		//Serial.println("END OF HEADERS!!!");



	} else {
		line.toLowerCase();

		// Content Type
		if ( line.startsWith("content-type: ") ) {
			curPos = 14;
			hapClient->request.contentType = line.substring(curPos);
		}

		// Content Length
		else if ( line.startsWith("content-length: ") ) {
			curPos = 16;
			hapClient->request.contentLength = line.substring(curPos).toInt();
		}


	}
}



bool HAPServer::encode(HAPClient* hapClient){

	uint16_t written = 0;
	bool success = false;

	// Method not supported :(
	if ( hapClient->client.peek() == 0x00) {
		hapClient->client.read();
//		Serial.println(c, HEX);
		hapClient->client.read();
//		Serial.println(c, HEX);
		hapClient->client.read();
//		Serial.println(c, HEX);
		hapClient->request.contentLength = hapClient->request.contentLength - 3;
	}

	// Reset pairing state
	hapClient->pairState = PAIR_STATE_RESERVED;

	while (hapClient->client.available()) {            	// loop while the client's connected

		if ( TLV8::isValidTLVType( hapClient->client.peek()) ) {

			byte type = hapClient->client.read();
			byte length = hapClient->client.read();

#if HAP_DEBUG
			LogD( F("------------------------------------------"), true );
			LogD("type:    " + String(type, HEX), true);
			LogD("length:  " + String(length), true);
#endif
			byte* data = (byte*) malloc(sizeof(byte) * length);
			hapClient->client.readBytes(data, length);

			if (type == TLV_TYPE_STATE) {

				if (hapClient->request.path == F("/pair-verify")) {
					hapClient->verifyState = static_cast<HAPVerifyState>(data[0]);
				} else 
					hapClient->pairState = static_cast<HAPPairState>(data[0]);
			}

#if HAP_DEBUG
			LogD( F("value:   "), false );
			char* outData = HAPHelper::toHex(data, length);
			LogD(outData, true);
			free(outData);
#endif

			written += length + 2;


			if (!hapClient->request.tlv.encode(type, length, data)) {
				LogE( F("ERROR: Encoding TLV data failed!"), true );
				return false;
			}

			if ( written == hapClient->request.contentLength ) {

#if HAP_DEBUG
				hapClient->request.tlv.print();
#endif
				success = true;
				hapClient->state = CLIENT_STATE_IDLE;
				break;
			}
//			else {
//				LogE( F("ERROR: Encoding TLV data failed 2!"), true );
//				return false;
//			}
		}
		else {
			LogW( F("WARNING: Unsupported TLV data! Skipping ..."), true );
			hapClient->client.read();
			//			hapClient->client.read();
			//			hapClient->client.read();
			//			written = written + 1;
		}

	}

	return success;
}

void HAPServer::handlePreferences(HAPClient* hapClient){
	LogI( F("<<< Handle preferences: "), true );
	// Send 204

	String response = String(HTTP_204);
	response += String(HTTP_CRLF);
	response += String(HTTP_CRLF);
	
	// hapClient->client.write( HTTP_204 );
	// hapClient->client.write( HTTP_CRLF );
	// hapClient->client.write( HTTP_CRLF );

	hapClient->client.write(response.c_str(), response.length());
	hapClient->request.clear();
}

void HAPServer::handleIdentify(HAPClient* hapClient){
	LogI( F("<<< Handle /identify: "), true );

	if ( !isPaired() ) {
		// Send 204
		hapClient->client.write( HTTP_204 );
	} else {
		// Send 400
		hapClient->client.write( HTTP_400 );

		hapClient->client.write( HTTP_CONTENT_TYPE_HAPJSON );

		hapClient->client.print( F("Content-Length: 21") );
		hapClient->client.write( HTTP_CRLF );
		hapClient->client.write( HTTP_CRLF );

		hapClient->client.print( F("{ \"status\" : -70401 }") );
		hapClient->client.write( HTTP_CRLF );
	}

	hapClient->client.write( HTTP_CRLF );

	hapClient->request.clear();
}

/*
bool HAPServer::beginSRP(){	
	return true;
}
*/

bool HAPServer::sendEncrypt(HAPClient* hapClient, String httpStatus, String plainText, bool chunked){
	bool result = true;

	LogD("\nEncrpyting response ...", false);

	String response;

	response = httpStatus;
	//response += String( HTTP_CRLF );

	if (httpStatus == HTTP_204) {
		response += String( HTTP_CRLF );
	} else {
		response += String( HTTP_CONTENT_TYPE_HAPJSON );

		if ( httpStatus != EVENT_200 ) {
			response += String( HTTP_KEEP_ALIVE );	
		}
		

		if (chunked) {
			response += String( HTTP_TRANSFER_ENCODING );
			response += String( HTTP_CRLF );

			char chunkSize[10];
			sprintf(chunkSize, "%x%s", plainText.length(), HTTP_CRLF);
			response += String(chunkSize);

		} else {
			response += String("Content-Length: " + String(plainText.length()));
			response += String( HTTP_CRLF );
			response += String( HTTP_CRLF );
		}	
		
		
		response += plainText;
		response += String( HTTP_CRLF );

		//int bytesSent = hapClient->client.println(encrypted);
		//hapClient->client.write( HTTP_CRLF );
		//free(outResponse);

		if (chunked) {
			response += String(0);			
			response += String( HTTP_CRLF );		
		}
		response += String( HTTP_CRLF );
	}


#if 0
	Serial.println("response: ");
	HAPHelper::arrayPrint((uint8_t*)response.c_str(), response.length());
#endif


	int encryptedLen = 0;
    char* encrypted = encrypt((uint8_t*)response.c_str(), response.length(), &encryptedLen, hapClient->encryptionContext->encryptKey, hapClient->encryptionContext->encryptCount++);

    if (encryptedLen == 0) {
    	LogE("[ERROR] Encrpyting response failed!", true);
    	hapClient->request.clear();
    	return false;
    } else {
		LogD("OK", true);
    }
    

	LogD("\n>>> Sending " + String(encryptedLen) + " bytes encrypted response to client [" + hapClient->client.remoteIP().toString() + "]", true);

#if HAP_DEBUG
	LogD(response, true);	
	HAPHelper::arrayPrint((uint8_t*)encrypted, encryptedLen);
#endif


	int bytesSent = hapClient->client.write(encrypted, encryptedLen);
	// hapClient->client.flush();


	if (bytesSent < encryptedLen) {
		LogE( F("[ERROR] Encrypted bytes did not match the expected length"), true );
		result = false;
	} 

	hapClient->request.clear();
	
	free(encrypted);

	return result;
}

bool HAPServer::sendResponse(HAPClient* hapClient, TLV8* response, bool chunked){
	
	bool result = true;
	
#if HAP_BUFFERED_SEND

	uint8_t buffer[HAP_BUFFER_SEND_SIZE];
	int offset = 0;

	// HTTP 200
	memcpy(buffer, String(HTTP_200).c_str(), String(HTTP_200).length());
	offset = String(HTTP_200).length();
	// CR_LF
	//memcpy(buffer + offset, String(HTTP_CRLF).c_str(), String(HTTP_CRLF).length());
	//offset += String(HTTP_CRLF).length();

	// HTTP_CONTENT_TYPE_TLV8
	memcpy(buffer + offset, String(HTTP_CONTENT_TYPE_TLV8).c_str(), String(HTTP_CONTENT_TYPE_TLV8).length());
	offset += String(HTTP_CONTENT_TYPE_TLV8).length();
	// CR_LF
	//memcpy(buffer + offset, String(HTTP_CRLF).c_str(), String(HTTP_CRLF).length());
	//offset += String(HTTP_CRLF).length();

	// HTTP_KEEP_ALIVE
	memcpy(buffer + offset, String(HTTP_KEEP_ALIVE).c_str(), String(HTTP_KEEP_ALIVE).length());
	offset += String(HTTP_KEEP_ALIVE).length();
	// CR_LF
	//memcpy(buffer + offset, String(HTTP_CRLF).c_str(), String(HTTP_CRLF).length());
	//offset += String(HTTP_CRLF).length();


	if (chunked) {
		// HTTP_TRANSFER_ENCODING  
		memcpy(buffer + offset, String(HTTP_TRANSFER_ENCODING).c_str(), String(HTTP_TRANSFER_ENCODING).length());
		offset += String(HTTP_TRANSFER_ENCODING).length();
		// CR_LF
		//memcpy(buffer + offset, String(HTTP_CRLF).c_str(), String(HTTP_CRLF).length());
		//offset += String(HTTP_CRLF).length();


		// CR_LF
		memcpy(buffer + offset, String(HTTP_CRLF).c_str(), String(HTTP_CRLF).length());
		offset += String(HTTP_CRLF).length();


		char chunkSize[10];
		sprintf(chunkSize, "%x%s", (int)response->size(), HTTP_CRLF);

		// Chunk size
		memcpy(buffer + offset, chunkSize, String(chunkSize).length());
		offset += String(chunkSize).length();

	} else {
		memcpy(buffer + offset, "Content-Length: ", String("Content-Length: ").length());
		offset += String("Content-Length: ").length();

		memcpy(buffer + offset, String(response->size()).c_str(), sizeof(response->size()) );
		offset += String(response->size()).length();


		// CR_LF
		memcpy(buffer + offset, String(HTTP_CRLF).c_str(), String(HTTP_CRLF).length());
		offset += String(HTTP_CRLF).length();
	}



	uint8_t outResponse[response->size()];
	size_t written = 0;

	response->decode(outResponse, &written);
	if (written == 0) {
		LogE("[ERROR] Failed to decode tlv8!", true);
	}

	memcpy(buffer + offset, outResponse, written);
	offset += written;


	// CR_LF
	memcpy(buffer + offset, String(HTTP_CRLF).c_str(), String(HTTP_CRLF).length());
	offset += String(HTTP_CRLF).length();

	if (chunked) {

		char chunkSize[10];
		sprintf(chunkSize, "%x%s", 0, HTTP_CRLF);

		// Chunk size
		memcpy(buffer + offset, chunkSize, String(chunkSize).length());
		offset += String(chunkSize).length();

		memcpy(buffer + offset, String(HTTP_CRLF).c_str(), String(HTTP_CRLF).length());
		offset += String(HTTP_CRLF).length();
	}

	int bytesSent = hapClient->client.write(buffer, offset);
	// hapClient->client.flush();

	LogD("\nSent " + String(bytesSent) + "/" + String(offset) + " bytes", true);


	if (bytesSent < offset) {
		LogE( F("[ERROR] Sent bytes did not match the response length"), true );
		result = false;
	}

#else
		// Send 200
	hapClient->client.print( HTTP_200 );
	hapClient->client.print( HTTP_CONTENT_TYPE_TLV8 );
	hapClient->client.print( HTTP_KEEP_ALIVE );

#if HAP_DEBUG
	LogD(">>> Sending " + String(response->size()) + " bytes response to client [" + hapClient->client.remoteIP().toString() + "]", true);
	response->print();
#endif

	if (chunked) {
		hapClient->client.print( HTTP_TRANSFER_ENCODING );
		hapClient->client.write( HTTP_CRLF );

		char chunkSize[10];
		sprintf(chunkSize, "%x%s", (int)response->size(), HTTP_CRLF);
		hapClient->client.write(chunkSize);

		hapClient->client.write( HTTP_CRLF );

	} else {
		hapClient->client.println("Content-Length: " + String(response->size()));
		hapClient->client.write( HTTP_CRLF );
	}

	uint8_t *outResponse = response->decode();
	int bytesSent = hapClient->client.write(outResponse, response->size());
	hapClient->client.write( HTTP_CRLF );
	
	// hapClient->client.flush();
	
	free(outResponse);

	if (bytesSent < response->size()) {
		LogE( F("[ERROR] Sending failed. Sent bytes did not match the expected length"), true );
		result = false;
	}

	if (chunked) {
		hapClient->client.write((uint8_t) 0x30);		// 0x30 is needed
		hapClient->client.write( HTTP_CRLF );		
	}
	hapClient->client.write( HTTP_CRLF );
	
	
	
#endif
	
	
	response->clear();
	hapClient->request.clear();

	return result;
}



bool HAPServer::handlePairSetupM1(HAPClient* hapClient){

	LogI( F("Homekit PIN: "), false);
	LogI( String(_accessorySet->pinCode()), true);

	LogV( "<<< Handle client [" + hapClient->client.remoteIP().toString() + "] -> /pair-setup Step 1/4 ...", false);	


	TLV8 response;

	_longTermContext = (struct HAPLongTermContext*) calloc(1, sizeof(struct HAPLongTermContext));
	if (_longTermContext == NULL) {
		LogE( F("[ERROR] Initializing struct _longTermContext failed!"), true);
		return false;
	}

	// generate keys if not stored
	// TODO:	
	LogD("\nGenerating key pairs ...", false);
	_longTermContext->publicKey = (uint8_t*) malloc(sizeof(uint8_t) * ED25519_PUBLIC_KEY_LENGTH);
	_longTermContext->publicKeyLength = ED25519_PUBLIC_KEY_LENGTH;
	_longTermContext->privateKey = (uint8_t*) malloc(sizeof(uint8_t) * ED25519_PRIVATE_KEY_LENGTH);
	_longTermContext->privateKeyLength = ED25519_PRIVATE_KEY_LENGTH;

 	ed25519_key_generate(_longTermContext->publicKey, _longTermContext->privateKey);
 	LogD("OK", true);


 	_pairings.saveLTPK(_longTermContext->publicKey);
 	_pairings.saveLTSK(_longTermContext->privateKey);



 	LogD("Initializing srp ...", false);
	if (_srp) {
		srp_cleanup(_srp);
	}

	_srp = srp_init(_accessorySet->pinCode());
	uint8_t host_public_key[SRP_PUBLIC_KEY_LENGTH] = {0,};
	LogD("OK", true);

	LogD("Generating srp key ...", false);
	if (srp_host_key_get(_srp, host_public_key) < 0) {
		LogE( F("[ERROR] srp_host_key_get failed"), true);
		//return HomekitHelper::pairError(HAP_TLV_ERROR_UNKNOWN, acc_msg, acc_msg_length);
		response.encode(TLV_TYPE_STATE, 1, PAIR_STATE_M2);
		response.encode(TLV_TYPE_ERROR, 1, HAP_TLV_ERROR_UNKNOWN);

		if (_srp) {
			srp_cleanup(_srp);
		}

		sendResponse(hapClient, &response);		
		return false;
	}
	LogD("OK", true);

	LogD("Generating salt ...", false);
	uint8_t salt[SRP_SALT_LENGTH] = {0,};
	if (srp_salt(_srp, salt) < 0) {
		LogE( F("[ERROR] srp_salt failed"), true);
		response.encode(TLV_TYPE_STATE, 1, PAIR_STATE_M2);
		response.encode(TLV_TYPE_ERROR, 1, HAP_TLV_ERROR_UNKNOWN);

		if (_srp) {
			srp_cleanup(_srp);
		}

		sendResponse(hapClient, &response);
		return false;
	}
	LogD("OK", true);
	

	LogD("Sending response ...", false);
	response.encode(TLV_TYPE_STATE, 1, PAIR_STATE_M2);
	response.encode(TLV_TYPE_SALT, SRP_SALT_LENGTH, salt);
	response.encode(TLV_TYPE_PUBLIC_KEY, SRP_PUBLIC_KEY_LENGTH, host_public_key);


	sendResponse(hapClient, &response);
	LogD("OK", true);


	hapClient->request.clear();
	response.clear();


	LogV("OK", true);
	return true;
}

bool HAPServer::handlePairSetupM3(HAPClient* hapClient) {
	LogV( "<<< Handle client [" + hapClient->client.remoteIP().toString() + "] -> /pair-setup Step 2/4 ...", false);

	int err_code = 0;
	TLV8 response;

	LogD( F("\nDecoding TLV ..."), false);

	// uint8_t *device_public_key = hapClient->request.tlv.decode(HAP_TLV_TYPE_PUBLICKEY);

	size_t decodedLen = 0;	
	uint8_t device_public_key[hapClient->request.tlv.size(HAP_TLV_TYPE_PUBLICKEY)];

	hapClient->request.tlv.decode(HAP_TLV_TYPE_PUBLICKEY, device_public_key, &decodedLen);

	if (decodedLen == 0) {
		LogE( F("[ERROR] Invalid payload: no client public key"), true);		
		response.encode(TLV_TYPE_STATE, 1, PAIR_STATE_M4);
		response.encode(TLV_TYPE_ERROR, 1, HAP_TLV_ERROR_AUTHENTICATION);

		sendResponse(hapClient, &response);	
		return false;
	}
	LogD("OK", true);
	

	LogD( F("Generating proof ..."), false);
	err_code = srp_client_key_set(_srp, device_public_key);
	if (err_code < 0) {
		LogE( F("[ERROR] srp_client_key_set failed"), true);		
		response.encode(TLV_TYPE_STATE, 1, PAIR_STATE_M4);
		response.encode(TLV_TYPE_ERROR, 1, HAP_TLV_ERROR_AUTHENTICATION);
        
        if (_srp) {
			srp_cleanup(_srp);
		}
        sendResponse(hapClient, &response);	
        return false;
    }

    decodedLen = 0;
    // uint8_t *proof = hapClient->request.tlv.decode(HAP_TLV_TYPE_PROOF);
    uint8_t proof[hapClient->request.tlv.size(HAP_TLV_TYPE_PROOF)];
	hapClient->request.tlv.decode(HAP_TLV_TYPE_PROOF, proof, &decodedLen);

    if (decodedLen == 0) {
    	LogE( F("[ERROR] Invalid payload: no device proof"), true);		    	
    	response.encode(TLV_TYPE_STATE, 1, PAIR_STATE_M4);
		response.encode(TLV_TYPE_ERROR, 1, HAP_TLV_ERROR_AUTHENTICATION);
        
        sendResponse(hapClient, &response);
    	return false;
    }
	LogD("OK", true);


    LogD( F("Verifying device proof ..."), false);
    err_code = srp_client_proof_verify(_srp, proof);
    if (err_code < 0) {        
        LogE( F("[ERROR] srp_client_proof_verify failed"), true);		    	
        response.encode(TLV_TYPE_STATE, 1, PAIR_STATE_M4);
		response.encode(TLV_TYPE_ERROR, 1, HAP_TLV_ERROR_AUTHENTICATION);
        

		if (_srp) {
			srp_cleanup(_srp);
		}

        sendResponse(hapClient, &response);
        return false; 
    }
    LogD("OK", true);


    LogD( F("Generating accessory proof ..."), false);
    uint8_t acc_srp_proof[SRP_PROOF_LENGTH] = {0,};
    err_code = srp_host_proof_get(_srp, acc_srp_proof);
    if (err_code < 0) {
        LogE( F("[ERROR] srp_host_proof_get failed"), true);		    	
        response.encode(TLV_TYPE_STATE, 1, PAIR_STATE_M4);
		response.encode(TLV_TYPE_ERROR, 1, HAP_TLV_ERROR_AUTHENTICATION);


		if (_srp) {
			srp_cleanup(_srp);
		}
        sendResponse(hapClient, &response);
        return false; 
    }
    LogD("OK", true);


    LogD("Sending response ...", false);
    response.encode(TLV_TYPE_STATE, 1, PAIR_STATE_M4);
    response.encode(HAP_TLV_TYPE_PROOF, SRP_PROOF_LENGTH, acc_srp_proof);

#if HAP_DEBUG
	response.print();
#endif    
	
	sendResponse(hapClient, &response);
	LogD("OK", true);
	
	hapClient->request.clear();
	response.clear();

	LogV("OK", true);
    return true;
}

bool HAPServer::handlePairSetupM5(HAPClient* hapClient) {
	LogV( "<<< Handle client [" + hapClient->client.remoteIP().toString() + "] -> /pair-setup Step 3/4 ...", false);

	int err_code = 0;
	TLV8 response;

    uint8_t srp_key[SRP_SESSION_KEY_LENGTH] = {0,};
    srp_host_session_key(_srp, srp_key);
   
	if (_srp) {
		srp_cleanup(_srp);
	}


    LogD( F("\nDecoding TLV values ..."), false);
	uint8_t *encrypted_tlv = hapClient->request.tlv.decode(HAP_TLV_TYPE_ENCRYPTED_DATA);
	size_t encrypted_tlv_len = hapClient->request.tlv.size(HAP_TLV_TYPE_ENCRYPTED_DATA);

	 if (!encrypted_tlv) {
        LogE( F("[ERROR] Decrypting HAP_TLV_TYPE_ENCRYPTED_DATA failed"), true);		    	
    	response.encode(TLV_TYPE_STATE, 1, PAIR_STATE_M6);
		response.encode(TLV_TYPE_ERROR, 1, HAP_TLV_ERROR_AUTHENTICATION);
        
        sendResponse(hapClient, &response);        
    	return false;
    }
    LogD( F("OK"), true);

    LogD( F("Decrypting chacha20_poly1305 ..."), false);
#if HAP_USE_WOLFSSL_HKDF	
	uint8_t subtlv_key[HKDF_KEY_LEN] = {0,};
    hkdf_key_get(HKDF_KEY_TYPE_PAIR_SETUP_ENCRYPT, srp_key, SRP_SESSION_KEY_LENGTH, subtlv_key);
#else	    
	uint8_t subtlv_key[MBEDTLS_MD_MAX_SIZE] = {0,};
	const mbedtls_md_info_t *md = mbedtls_md_info_from_type( MBEDTLS_MD_SHA512 );

	err_code = mbedtls_hkdf(md, 
				(uint8_t*)"Pair-Setup-Encrypt-Salt", strlen("Pair-Setup-Encrypt-Salt"), 
				srp_key, SRP_SESSION_KEY_LENGTH,
            	(uint8_t*)"Pair-Setup-Encrypt-Info", strlen("Pair-Setup-Encrypt-Info"),
            	subtlv_key, MBEDTLS_MD_MAX_SIZE);
#endif

	if (err_code < 0) {
        LogE( F("[ERROR] Get HKDF Key failed"), true);		    	
        response.encode(TLV_TYPE_STATE, 1, PAIR_STATE_M6);
		response.encode(TLV_TYPE_ERROR, 1, HAP_TLV_ERROR_UNKNOWN);
        
        sendResponse(hapClient, &response);

        // free(encrypted_tlv);
        return false;
    }
    LogD( F("OK"), true);

    LogD( F("Decrypting chacha20_poly1305 ..."), false);
    // uint8_t *subtlv = (uint8_t*) malloc(sizeof(uint8_t) * encrypted_tlv_len);
    uint8_t subtlv[encrypted_tlv_len] = {0,};

    err_code = chacha20_poly1305_decrypt(CHACHA20_POLY1305_TYPE_PS05, subtlv_key, NULL, 0, encrypted_tlv, encrypted_tlv_len, subtlv);

    if (err_code < 0) {
        LogE( F("[PAIR-SETUP M5] [ERROR] Decrypting CHACHA20_POLY1305_TYPE_PS05 failed"), true);		    	
        response.encode(TLV_TYPE_STATE, 1, PAIR_STATE_M6);
		response.encode(TLV_TYPE_ERROR, 1, HAP_TLV_ERROR_AUTHENTICATION);
        
        sendResponse(hapClient, &response);

        // free(subtlv);
        // free(encrypted_tlv);
        return false;
    }
    LogD( F("OK"), true);

    
	TLV8 encTLV; 
	encTLV.encode(subtlv, strlen((char*)subtlv));
	
#if HAP_DEBUG	
	encTLV.print();
    //HAPHelper::arrayPrint(subtlv, strlen((char*)subtlv));
#endif

	// free(subtlv);
	// free(encrypted_tlv);

    uint8_t ios_devicex[HKDF_KEY_LEN] = {0,};
    hkdf_key_get(HKDF_KEY_TYPE_PAIR_SETUP_CONTROLLER, srp_key, SRP_SESSION_KEY_LENGTH, ios_devicex);

    uint8_t* ios_device_pairing_id 		= encTLV.decode(HAP_TLV_TYPE_IDENTIFIER);
    uint8_t ios_device_pairing_id_len 	= encTLV.size(HAP_TLV_TYPE_IDENTIFIER);

    uint8_t* ios_device_ltpk 			= encTLV.decode(HAP_TLV_TYPE_PUBLICKEY);
    uint8_t  ios_device_ltpk_len 		= encTLV.size(HAP_TLV_TYPE_PUBLICKEY);


    uint8_t* ios_device_signature 		= encTLV.decode(HAP_TLV_TYPE_SIGNATURE);
    uint8_t  ios_device_signature_len 	= encTLV.size(HAP_TLV_TYPE_SIGNATURE);


    int ios_device_info_len = 0;
    uint8_t* ios_device_info = concat3(ios_devicex, sizeof(ios_devicex), 
            ios_device_pairing_id, ios_device_pairing_id_len, 
            ios_device_ltpk, ios_device_ltpk_len,
            &ios_device_info_len);
    

    LogD( F("Verifying ED25519 ..."), false);
	int verified = ed25519_verify(ios_device_ltpk, ios_device_ltpk_len,
            ios_device_signature, ios_device_signature_len,
            ios_device_info, ios_device_info_len);
	
    concat_free(ios_device_info);

	if (verified < 0) {
        LogE( F("[ERROR] Verification failed"), true);		    	
        response.encode(TLV_TYPE_STATE, 1, PAIR_STATE_M6);
		response.encode(TLV_TYPE_ERROR, 1, HAP_TLV_ERROR_AUTHENTICATION);
        
        sendResponse(hapClient, &response);
        return false;
	}
	LogD( F("OK"), true);


	// Save to Pairings
	LogD( F("Saving pairing ..."), false);
// #if HAP_DEBUG
// 	LogD("\niOS ios_device_pairing_id: ", true);
// 	HAPHelper::arrayPrint(ios_device_pairing_id, ios_device_pairing_id_len );
// 	LogD("iOS ios_device_ltpk: ", true);
// 	HAPHelper::arrayPrint(ios_device_ltpk, ios_device_ltpk_len );

// 	LogD("_pairSetup->keys.privateKey: ", true);
// 	HAPHelper::arrayPrint(_longTermContext->privateKey, ED25519_PRIVATE_KEY_LENGTH);

// 	LogD("_pairSetup->keys.publicKey: ", true);
// 	HAPHelper::arrayPrint(_longTermContext->publicKey, ED25519_PUBLIC_KEY_LENGTH);
// #endif

	_pairings.add(ios_device_pairing_id, ios_device_ltpk);
	_pairings.save();	
	LogD( F("OK"), true);
	
	encTLV.clear();
	// delete enc_tlv;


	LogV( "<<< Handle [" + hapClient->client.remoteIP().toString() + "] -> /pair-setup Step 4/4 ...", true);
	//  _acc_m6_subtlv(srp_key, ps->acc_id, ps->keys.public, ps->keys.private, &acc_subtlv, &acc_subtlv_length);
	uint8_t accessoryx[HKDF_KEY_LEN] = {0,};
	hkdf_key_get(HKDF_KEY_TYPE_PAIR_SETUP_ACCESSORY, srp_key, SRP_SESSION_KEY_LENGTH, 
            accessoryx);

	
    int acc_info_len = 0;
    uint8_t* acc_info = concat3(accessoryx, sizeof(accessoryx), 
            (uint8_t*)HAPDeviceID::deviceID().c_str(), 17, 
            _longTermContext->publicKey, ED25519_PUBLIC_KEY_LENGTH, &acc_info_len);

    LogD( F("\nVerifying signature"), false);
	int acc_signature_length = ED25519_SIGN_LENGTH;
    uint8_t acc_signature[ED25519_SIGN_LENGTH] = {0,};
    err_code = ed25519_sign(_longTermContext->publicKey, _longTermContext->privateKey, acc_info, acc_info_len,
            acc_signature, &acc_signature_length);


    concat_free(acc_info);

    if (err_code != 0) {
        LogE( F("[ERROR] Verify signature failed"), true);		    	
        response.encode(TLV_TYPE_STATE, 1, PAIR_STATE_M6);
		response.encode(TLV_TYPE_ERROR, 1, HAP_TLV_ERROR_AUTHENTICATION);
        
        sendResponse(hapClient, &response);
        return false;
	}
	LogD( F("OK"), true);

	// Encrypt data
	TLV8 subTLV;
	subTLV.encode(HAP_TLV_TYPE_IDENTIFIER, 17, (uint8_t*)HAPDeviceID::deviceID().c_str()  );
	subTLV.encode(HAP_TLV_TYPE_PUBLICKEY, ED25519_PUBLIC_KEY_LENGTH, _longTermContext->publicKey);
	subTLV.encode(HAP_TLV_TYPE_SIGNATURE, ED25519_SIGN_LENGTH, acc_signature);


	// TODO NOW:
	// uint8_t* tlv8Data = subTLV->decode();
	size_t tlv8Len = subTLV.size();
	uint8_t tlv8Data[tlv8Len];
	size_t written = 0;

	subTLV.decode(tlv8Data, &written);
	if (written == 0) {
		LogE("[ERROR] Failed to decode subtlv8!", true);
	}
	

#if HAP_DEBUG	
	subTLV.print();
	//HAPHelper::arrayPrint(tlv8Data, tlv8Len);
#endif
		

	uint8_t encryptedData[tlv8Len + CHACHA20_POLY1305_AUTH_TAG_LENGTH];
	// encryptedData = (uint8_t*)malloc(sizeof(uint8_t) * (tlv8Len + CHACHA20_POLY1305_AUTH_TAG_LENGTH));
	// 
	// if (!encryptedData){
	// 	LogE( F("[ERROR] Malloc of encryptedData failed"), true);		    	
	// }

	LogD( F("Getting session key ..."), false);
	err_code = hkdf_key_get(HKDF_KEY_TYPE_PAIR_SETUP_ENCRYPT, srp_key, SRP_SESSION_KEY_LENGTH, subtlv_key);
	if (err_code != 0) {
        LogE( F("[ERROR] Verify signature failed"), true);		    	
        response.encode(TLV_TYPE_STATE, 1, PAIR_STATE_M6);
		response.encode(TLV_TYPE_ERROR, 1, HAP_TLV_ERROR_UNKNOWN);
        
        sendResponse(hapClient, &response);
        subTLV.clear();
        return false;
	}
	LogD( F("OK"), true);


	LogD( F("Encrypting Data ..."), false);
	err_code = chacha20_poly1305_encrypt(CHACHA20_POLY1305_TYPE_PS06, subtlv_key, NULL, 0, tlv8Data, tlv8Len, encryptedData);
	if (err_code != 0) {
        LogE( F("[ERROR] Verify signature failed"), true);		    	
        response.encode(TLV_TYPE_STATE, 1, PAIR_STATE_M6);
		response.encode(TLV_TYPE_ERROR, 1, HAP_TLV_ERROR_UNKNOWN);
        
        sendResponse(hapClient, &response);
        subTLV.clear();
        return false;
	}
	LogD( F("OK"), true);


	LogD( F("Sending response ..."), false);
	response.encode(TLV_TYPE_STATE, 1, PAIR_STATE_M6);
	response.encode(TLV_TYPE_ENCRYPTED_DATA, tlv8Len + CHACHA20_POLY1305_AUTH_TAG_LENGTH, encryptedData);

#if HAP_DEBUG	
	response.print();
	//HAPHelper::arrayPrint(encryptedData, tlv8Len + CHACHA20_POLY1305_AUTH_TAG_LENGTH);
#endif

	sendResponse(hapClient, &response);	
	LogD( F("OK"), true);

	response.clear();
	subTLV.clear();
	// delete subTLV;

	updateServiceTxt();
	
	LogV("OK", true);
	LogI(">>> Pairing with client [" + hapClient->client.remoteIP().toString() + "] complete!", true);

    return true;
}


bool HAPServer::handlePairVerifyM1(HAPClient* hapClient){
	LogV( "<<< Handle client [" + hapClient->client.remoteIP().toString() + "] -> /pair-verify Step 1/2 ...", false);

	int err_code = 0;
	TLV8 response;


	if ( !isPaired() ) {
		LogW( F("\n[WARNING] Attempt to verify unpaired accessory!"), true);
		response.encode(TLV_TYPE_STATE, 1, VERIFY_STATE_M2);
		response.encode(TLV_TYPE_ERROR, 1, HAP_TLV_ERROR_UNKNOWN);
        
        sendResponse(hapClient, &response);
		return false;
	}

	hapClient->encryptionContext->decryptCount = 0;
	hapClient->encryptionContext->encryptCount = 0;

	LogD("\nGenerating accessory curve25519 keys ...", false);

	uint8_t acc_curve_public_key[CURVE25519_KEY_LENGTH] = {0,};		// my_key_public
	uint8_t acc_curve_private_key[CURVE25519_KEY_LENGTH] = {0,};	// my_key	

	if (curve25519_key_generate(acc_curve_public_key, acc_curve_private_key) < 0) {
		LogE( F("[ERROR] curve25519_key_generate failed"), true);
		response.encode(TLV_TYPE_STATE, 1, VERIFY_STATE_M2);
		response.encode(TLV_TYPE_ERROR, 1, HAP_TLV_ERROR_UNKNOWN);
        
        sendResponse(hapClient, &response);
		return false;
	}
	LogD( F("OK"), true);


	uint8_t *ios_device_curve_key = hapClient->request.tlv.decode(HAP_TLV_TYPE_PUBLICKEY); 	// device_key
	uint8_t ios_device_curve_key_len = hapClient->request.tlv.size(HAP_TLV_TYPE_PUBLICKEY);
	
#if HAP_DEBUG
	LogD("acc_curve_public_key", true);
	HAPHelper::arrayPrint(acc_curve_public_key, CURVE25519_KEY_LENGTH);

	LogD("acc_curve_private_key", true);
	HAPHelper::arrayPrint(acc_curve_private_key, CURVE25519_KEY_LENGTH);


	LogD("ios_device_curve_key", true);
	HAPHelper::arrayPrint(ios_device_curve_key, ios_device_curve_key_len);
#endif

	LogD("Generating Curve25519 shared secret ...", false);
	uint8_t sharedSecret[CURVE25519_SECRET_LENGTH] = {0,};									// shared_secret
	int sharedSecretLength = CURVE25519_SECRET_LENGTH;		

	if (curve25519_shared_secret(ios_device_curve_key, acc_curve_private_key, sharedSecret, &sharedSecretLength) < 0) {
		LogE( F("[ERROR] curve25519_shared_secret failed"), true);

		response.encode(TLV_TYPE_STATE, 1, VERIFY_STATE_M2);
		response.encode(TLV_TYPE_ERROR, 1, HAP_TLV_ERROR_UNKNOWN);
        
        sendResponse(hapClient, &response);
		return false;
	}
	LogD( F("OK"), true);



	LogD("Generating signature ...", false);
	int acc_info_len;
	uint8_t* acc_info = concat3(acc_curve_public_key, CURVE25519_KEY_LENGTH,
		(uint8_t*)HAPDeviceID::deviceID().c_str(), 17,
		ios_device_curve_key, ios_device_curve_key_len,
		&acc_info_len);

	int acc_signature_length = ED25519_SIGN_LENGTH;
	uint8_t acc_signature[ED25519_SIGN_LENGTH] = {0,};
	err_code = ed25519_sign(_longTermContext->publicKey, _longTermContext->privateKey, 
		acc_info, acc_info_len,
		acc_signature, &acc_signature_length);



	if (err_code < 0){
		LogE( F("[ERROR] ed25519_sign failed"), true);
		response.encode(TLV_TYPE_STATE, 1, VERIFY_STATE_M2);
		response.encode(TLV_TYPE_ERROR, 1, HAP_TLV_ERROR_UNKNOWN);
        
        sendResponse(hapClient, &response);
		return false;
	}


	concat_free(acc_info);
	LogD( F("OK"), true);

	// Encrypt data

	LogD( F("Encoding into TLV ..."), false);
	TLV8 *subTLV = new TLV8();
	subTLV->encode(HAP_TLV_TYPE_IDENTIFIER, 17, (uint8_t*)HAPDeviceID::deviceID().c_str()  );
	subTLV->encode(HAP_TLV_TYPE_SIGNATURE, ED25519_SIGN_LENGTH, acc_signature);

	size_t tlv8Len = subTLV->size();

		// TODO NOW:
	// uint8_t* tlv8Data = subTLV->decode();
	uint8_t tlv8Data[subTLV->size()];
	size_t written = 0;

	subTLV->decode(tlv8Data, &written);
	if (written == 0) {
		LogE("[ERROR] Failed to decode subtlv8!", true);
	}

#if HAP_DEBUG	
	subTLV->print();
	//HAPHelper::arrayPrint(tlv8Data, tlv8Len);
#endif
	LogD( F("OK"), true);


	LogD("Generating proof ...", false);	
    uint8_t sessionKey[HKDF_KEY_LEN] = {0,};   		// session_key 
    err_code = hkdf_key_get(HKDF_KEY_TYPE_PAIR_VERIFY_ENCRYPT, sharedSecret, CURVE25519_SECRET_LENGTH, sessionKey);
	if (err_code != 0) {
        LogE( F("[ERROR] Verify signature failed"), true);		    	
        response.encode(TLV_TYPE_STATE, 1, VERIFY_STATE_M2);
		response.encode(TLV_TYPE_ERROR, 1, HAP_TLV_ERROR_UNKNOWN);
        
        sendResponse(hapClient, &response);
        return false;
	}
	LogD( F("OK"), true);


	LogD( F("Encrypting data ..."), false);
	uint8_t* encryptedData;
	encryptedData = (uint8_t*)malloc(sizeof(uint8_t) * (tlv8Len + CHACHA20_POLY1305_AUTH_TAG_LENGTH));
	
	if (!encryptedData){
		LogE( F("[ERROR] Malloc of encryptedData failed"), true);		    	
		response.encode(TLV_TYPE_STATE, 1, VERIFY_STATE_M2);
		response.encode(TLV_TYPE_ERROR, 1, HAP_TLV_ERROR_UNKNOWN);
        
        sendResponse(hapClient, &response);
        return false;
	}

   	err_code = chacha20_poly1305_encrypt(CHACHA20_POLY1305_TYPE_PV02, sessionKey, NULL, 0, tlv8Data, tlv8Len, encryptedData);
	if (err_code != 0) {
        LogE( F("[ERROR] Verify signature failed"), true);		    	
        response.encode(TLV_TYPE_STATE, 1, VERIFY_STATE_M2);
		response.encode(TLV_TYPE_ERROR, 1, HAP_TLV_ERROR_UNKNOWN);
        
        sendResponse(hapClient, &response);
        return false;
	}
	LogD( F("OK"), true);

	LogD("Saving context ...", false);
	

	hapClient->verifyContext = new struct HAPVerifyContext;	
	memcpy(hapClient->verifyContext->secret, sharedSecret, HKDF_KEY_LEN);
	memcpy(hapClient->verifyContext->sessionKey, sharedSecret, CURVE25519_SECRET_LENGTH);
	memcpy(hapClient->verifyContext->accessoryLTPK, acc_curve_public_key, ED25519_PUBLIC_KEY_LENGTH);
	memcpy(hapClient->verifyContext->deviceLTPK, ios_device_curve_key, ED25519_PUBLIC_KEY_LENGTH);

    
	LogD( F("Sending response ..."), false);
	response.encode(TLV_TYPE_STATE, 1, VERIFY_STATE_M2);
	response.encode(HAP_TLV_TYPE_PUBLICKEY, CURVE25519_KEY_LENGTH, acc_curve_public_key);
	response.encode(TLV_TYPE_ENCRYPTED_DATA, tlv8Len + CHACHA20_POLY1305_AUTH_TAG_LENGTH, encryptedData);

	//memcpy(_pairSetup->sessionKey, sharedSecret, CURVE25519_SECRET_LENGTH);

#if HAP_DEBUG	
	response.print();
	//HAPHelper::arrayPrint(encryptedData, tlv8Len + CHACHA20_POLY1305_AUTH_TAG_LENGTH);
#endif

	sendResponse(hapClient, &response);	
	LogD( F("OK"), true);
	response.clear();
	
	subTLV->clear();
	delete subTLV;



	LogI("OK", true);
	return true;
}


bool HAPServer::handlePairVerifyM3(HAPClient* hapClient){
	LogV( "<<< Handle client [" + hapClient->client.remoteIP().toString() + "] -> /pair-verify Step 2/2 ...", false);

	int err_code = 0;
	TLV8 response;

	uint8_t* encryptedData = hapClient->request.tlv.decode(HAP_TLV_TYPE_ENCRYPTED_DATA);
	int encryptedDataLen = hapClient->request.tlv.size(HAP_TLV_TYPE_ENCRYPTED_DATA);


	if (!encryptedData) {
		LogE( F("[PAIR-VERIFY M3] [ERROR] HAP_TLV_TYPE_ENCRYPTED_DATA failed "), true);		    	
		response.encode(TLV_TYPE_STATE, 1, VERIFY_STATE_M4);
		response.encode(TLV_TYPE_ERROR, 1, HAP_ERROR_AUTHENTICATON);

		sendResponse(hapClient, &response);
		return false;
	}



	LogD("\nGenerating decrpytion key ...", true);
	uint8_t subtlv_key[HKDF_KEY_LEN] = {0,};
	err_code = hkdf_key_get(HKDF_KEY_TYPE_PAIR_VERIFY_ENCRYPT, hapClient->verifyContext->secret, CURVE25519_SECRET_LENGTH, subtlv_key);
	if (err_code != 0) {
		LogE( F("[PAIR-VERIFY M3] [ERROR] hkdf_key_get failed"), true);		    	
		response.encode(TLV_TYPE_STATE, 1, VERIFY_STATE_M4);
		response.encode(TLV_TYPE_ERROR, 1, HAP_ERROR_AUTHENTICATON);

		sendResponse(hapClient, &response);
		return false;
	}


	LogD("Decrypting data ...", true);
	uint8_t* subtlv = (uint8_t*)malloc(sizeof(uint8_t) * encryptedDataLen);
	err_code = chacha20_poly1305_decrypt(CHACHA20_POLY1305_TYPE_PV03, subtlv_key, NULL, 0, encryptedData, encryptedDataLen, subtlv);
	if (err_code != 0) {
		LogE( F("[ERROR] Decrypting failed"), true);		    	
		response.encode(TLV_TYPE_STATE, 1, VERIFY_STATE_M4);
		response.encode(TLV_TYPE_ERROR, 1, HAP_ERROR_AUTHENTICATON);

		sendResponse(hapClient, &response);
		return false;
	}	

	TLV8 subTlv;
	subTlv.encode(subtlv, strlen((char*)subtlv));

#if HAP_DEBUG
	LogD("subTLV: ", true);
	subTlv.print();
#endif


	uint8_t ios_device_pairing_id_len 	= subTlv.size(HAP_TLV_TYPE_IDENTIFIER);
	uint8_t *ios_device_pairing_id 		= subTlv.decode(HAP_TLV_TYPE_IDENTIFIER);		


	if (!ios_device_pairing_id) {
		LogE( F("[ERROR] HAP_TLV_TYPE_IDENTIFIER failed "), true);		    	
		response.encode(TLV_TYPE_STATE, 1, VERIFY_STATE_M4);
		response.encode(TLV_TYPE_ERROR, 1, HAP_ERROR_AUTHENTICATON);

		sendResponse(hapClient, &response);
		return false;
	}


#if HAP_DEBUG
	LogD("iOS device_pairing_id: ", true);
	HAPHelper::arrayPrint(ios_device_pairing_id, ios_device_pairing_id_len);	
#endif


	LogD("Looking up iOS device LTPK for client: ", true);
	HAPHelper::arrayPrint(ios_device_pairing_id, ios_device_pairing_id_len);	


	uint8_t ios_device_ltpk[ED25519_PUBLIC_KEY_LENGTH];
	err_code = _pairings.getKey(ios_device_pairing_id, ios_device_ltpk);
	
	if (err_code == -1) {
		LogE( F("[ERROR] No iOS Device LTPK found!"), true);		    	
		response.encode(TLV_TYPE_STATE, 1, VERIFY_STATE_M4);
		response.encode(TLV_TYPE_ERROR, 1, HAP_ERROR_AUTHENTICATON);

		sendResponse(hapClient, &response);
		return false;
	}
		

	LogD("Found LTPK: ", true);	
	HAPHelper::arrayPrint(ios_device_ltpk, ED25519_PUBLIC_KEY_LENGTH);

	uint8_t *ios_device_signature = subTlv.decode(HAP_TLV_TYPE_SIGNATURE);
	uint8_t ios_device_signature_len = subTlv.size(HAP_TLV_TYPE_SIGNATURE);

	LogD("Found Signature: ", false);
	HAPHelper::arrayPrint(ios_device_signature, ios_device_signature_len);


	if (!ios_device_signature) {
		LogE( F("[ERROR] HAP_TLV_TYPE_SIGNATURE failed "), true);		    	
		response.encode(TLV_TYPE_STATE, 1, VERIFY_STATE_M4);
		response.encode(TLV_TYPE_ERROR, 1, HAP_ERROR_AUTHENTICATON);

		sendResponse(hapClient, &response);
		return false;
	}

    
	int ios_device_info_len = 0;
    uint8_t* ios_device_info = concat3(hapClient->verifyContext->deviceLTPK, HKDF_KEY_LEN, 
            ios_device_pairing_id, ios_device_pairing_id_len, 
            hapClient->verifyContext->accessoryLTPK, ED25519_PUBLIC_KEY_LENGTH, &ios_device_info_len);



    LogD( F("Verifying Signature ..."), true);
	int verified = ed25519_verify(ios_device_ltpk, ED25519_PUBLIC_KEY_LENGTH,
            ios_device_signature, ios_device_signature_len,
            ios_device_info, ios_device_info_len);
	
    concat_free(ios_device_info);

	if (verified < 0) {
        LogE( F("[ERROR] Signature verification failed"), true);		    	
        response.encode(TLV_TYPE_STATE, 1, PAIR_STATE_M6);
		response.encode(TLV_TYPE_ERROR, 1, HAP_TLV_ERROR_AUTHENTICATION);
        
        sendResponse(hapClient, &response);
        return false;
	}
    

	// TODO Move ecrypt & decrypt keys out of pairSetup!
    err_code = hkdf_key_get(HKDF_KEY_TYPE_CONTROL_READ, hapClient->verifyContext->secret, CURVE25519_SECRET_LENGTH, hapClient->encryptionContext->encryptKey);
	if (err_code != 0) {
		LogE( F("[ERROR] hkdf_key_get failed"), true);		    	
		response.encode(TLV_TYPE_STATE, 1, VERIFY_STATE_M4);
		response.encode(TLV_TYPE_ERROR, 1, HAP_ERROR_AUTHENTICATON);

		sendResponse(hapClient, &response);
		return false;
	}


	err_code = hkdf_key_get(HKDF_KEY_TYPE_CONTROL_WRITE, hapClient->verifyContext->secret, CURVE25519_SECRET_LENGTH, hapClient->encryptionContext->decryptKey);
	if (err_code != 0) {
		LogE( F("[ERROR] hkdf_key_get failed"), true);		    	
		response.encode(TLV_TYPE_STATE, 1, VERIFY_STATE_M4);
		response.encode(TLV_TYPE_ERROR, 1, HAP_ERROR_AUTHENTICATON);

		sendResponse(hapClient, &response);
		return false;
	}


	// FREE CONTEXT !!!

	LogD( F("Sending response ..."), true);
	response.encode(TLV_TYPE_STATE, 1, VERIFY_STATE_M4);

#if HAP_DEBUG
	response.print();
#endif

	sendResponse(hapClient, &response);	
	response.clear();
	// following messages from this socket will be encrypted
	hapClient->isEncrypted = true;
	
	LogV("OK", true);
	LogI(">>> Verification with client [" + hapClient->client.remoteIP().toString() + "] complete!", true);

	return true;
}


void HAPServer::handleAccessories(HAPClient* hapClient) {
	LogV( "<<< Handle [" + hapClient->client.remoteIP().toString() + "] -> /accessories ...", false);
	sendEncrypt(hapClient, HTTP_200, _accessorySet->describe());	

	LogV("OK", true);
}



void HAPServer::handleCharacteristicsGet(HAPClient* hapClient){
	LogV( "<<< Handle client [" + hapClient->client.remoteIP().toString() + "] -> GET /characteristics ...", false);

	String idStr = hapClient->request.params["id"];
	//LogE(idStr, true);

	//String result = "[";
	DynamicJsonBuffer jsonBuffer(HAP_ARDUINOJSON_BUFFER_SIZE);

	JsonObject& root = jsonBuffer.createObject();
	JsonArray& characteristics = root.createNestedArray("characteristics");

	bool errorOccured = false;

	do {
		int curPos = 0;
		int endIndex = idStr.indexOf(",");		
		if (endIndex == -1){
			endIndex = idStr.length();		
		}

		String keyPair = idStr.substring(curPos, endIndex); 
		//Serial.printf("keyPair: %s\n", keyPair.c_str());

		int equalIndex = keyPair.indexOf(".");

		int aid = keyPair.substring(0, equalIndex).toInt();
		int iid = keyPair.substring(equalIndex + 1).toInt();

		
		// Serial.print(">>>>>>>>>>>>>>>>>>< aid: "); 
		// Serial.print(aid);
		// Serial.print(" - iid: ");
		// Serial.println(iid);
		
		//int error_code = HAP_STATUS_SUCCESS;
		//

		// String value = getValueForCharacteristics(aid, iid);


		JsonObject& characteristics_0 = characteristics.createNestedObject();
		characteristics_0["aid"] = aid;
		characteristics_0["iid"] = iid;

		size_t outSize = 0;
		int32_t errorCode = getValueForCharacteristics(aid, iid, NULL, &outSize);

#if HAP_DEBUG
			LogD( "errorCode: " + String(errorCode), true );
			LogD( "outSize:   " + String(outSize), true );
#endif

		if ( errorCode != 0){		

			characteristics_0["status"] = errorCode;
			errorOccured = true;
		} else {
			// outSize = outSize + 1;
			char value[outSize];
			getValueForCharacteristics(aid, iid, value, &outSize);

#if HAP_DEBUG
			LogD( "value: " + String(value), true );
#endif

			if ( strcmp(value, "true") == 0 ) {
				characteristics_0["value"] = 1;
			} else if ( strcmp(value, "false") == 0 ) {
				characteristics_0["value"] = 0;
			} else {
				characteristics_0["value"] = value;	
			}
		}
		
		idStr = idStr.substring(endIndex + 1); 
	} while ( idStr.length() > 0 );

	String response;			
	root.printTo(response);

#if HAP_DEBUG

	root.prettyPrintTo(Serial);

#endif


	if (errorOccured){
		sendEncrypt(hapClient, HTTP_400, response, false);
	} else {
		sendEncrypt(hapClient, HTTP_200, response, false);	
	}
    


	LogV("OK", true);

}


// String HAPServer::getValueForCharacteristics(int aid, int iid){		
// 	characteristics *c = getCharacteristics(aid, iid);

// 	if (c != nullptr) {
// 		return String(c->value());
// 	}
// 	return "--ERROR_OCCURED--";
// }

int32_t HAPServer::getValueForCharacteristics(int aid, int iid, char* out, size_t* outSize){
	characteristics *c = getCharacteristics(aid, iid);
	if (c != nullptr) {		
		*outSize = c->value().length() + 1;
		if (out != NULL){
			 c->value().toCharArray(out, *outSize);	
		}		
		return 0;
	}
	return HAP_STATUS_RESOURCE_NOT_FOUND;
}


characteristics* HAPServer::getCharacteristics(int aid, int iid){
	HAPAccessory *a = _accessorySet->accessoryWithAID(aid);		
		
		if (a == NULL) {

			LogE("[ERROR] Accessory with aid: ", false);
    		LogE(String(aid), false);
    		LogE(" not found! - ErrorCode: ", false);
    		LogE(String(HAP_STATUS_RESOURCE_NOT_FOUND), true);

    		//error_code = HAP_STATUS_RESOURCE_NOT_FOUND;
    		//errorOccured = true;	
    		return nullptr;	    			
		} 
		else {

			characteristics *c = a->characteristicsAtIndex(iid);

			if (c == NULL) {
				LogE("[ERROR] Characteristics with aid: ", false);
	    		LogE(String(aid), false);
	    		LogE(" - iid: ", false);
	    		LogE(String(iid), false);
				LogE(" not found! - ErrorCode: ", false);
    			LogE(String(HAP_STATUS_RESOURCE_NOT_FOUND), true);
    			return nullptr;
			} else {

				return c;
			}			

		}

	return nullptr;
}


void HAPServer::handleCharacteristicsPut(HAPClient* hapClient, String body){
	LogV( "<<< Handle client [" + hapClient->client.remoteIP().toString() + "] ->  PUT /characteristics ...", true);

	DynamicJsonBuffer jsonBuffer(HAP_ARDUINOJSON_BUFFER_SIZE);
	JsonObject& root = jsonBuffer.parseObject(body);

	if (!root.success()) {
    	LogE("[ERROR] Parsing characteristics failed!", true);
    	return;
  	}

	//root.prettyPrintTo(Serial);
	//Serial.println();

	//Serial.println(root["characteristics"][0].as<String>() );

	JsonArray& charArray = root["characteristics"];
	
	charArray.prettyPrintTo(Serial);	
	Serial.println();
	
	String result = "[";
	String httpStatusStr;
	

	bool onlyOne = charArray.size() > 0;	
	bool errorOccured = false;

	for( const auto& chr : charArray) {
    	int aid = chr["aid"].as<int>();
    	int iid = chr["iid"].as<int>();

    	bool isEvent = HAPHelper::containsNestedKey(chr, "ev");
    	String value = "";

    	int error_code = HAP_STATUS_SUCCESS;

    	
		Serial.println(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>><");
    	Serial.printf("aid: %d\n", aid);
		Serial.println(_accessorySet->describe());
		Serial.println(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>><");

    	HAPAccessory *a = _accessorySet->accessoryWithAID(aid);
    	if (a == NULL) {
    		LogE("[ERROR] Accessory with aid: ", false);
    		LogE(String(aid), false);
    		LogE(" not found! - ErrorCode: ", false);
    		LogE(String(HAP_STATUS_RESOURCE_NOT_FOUND), true);

    		error_code = HAP_STATUS_RESOURCE_NOT_FOUND;
    		errorOccured = true;
    	} else {
    		characteristics *c = a->characteristicsAtIndex(iid);


			if (c == NULL) {

	    		LogE("[ERROR] Characteristics with aid: ", false);
	    		LogE(String(aid), false);
	    		LogE(" - iid: ", false);
	    		LogE(String(iid), false);
				LogE(" not found! - ErrorCode: ", false);
    			LogE(String(HAP_STATUS_RESOURCE_NOT_FOUND), true);

    			error_code = HAP_STATUS_RESOURCE_NOT_FOUND;
    			errorOccured = true;
			} else {


				if ( isEvent ) {
					isEvent = true;


					Serial.print(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> NESTED EVENT: ");
					Serial.println(isEvent);
					Serial.println(chr["ev"].as<String>());

					LogE("!!!!!!!!!!!!!! ADDiNG EVENT 1!!!", true);
					LogE("!!!!!!!!!!!!!! ADDiNG EVENT 2!!!", true);
					LogE("!!!!!!!!!!!!!! ADDiNG EVENT 3!!!", true);
					LogE("!!!!!!!!!!!!!! ADDiNG EVENT 4!!!", true);
					LogE("!!!!!!!!!!!!!! ADDiNG EVENT 5!!!", true);
					LogE("!!!!!!!!!!!!!! ADDiNG EVENT 6!!!", true);


					struct HAPEvent event = HAPEvent(hapClient, aid, iid, chr["ev"].as<String>());					


					LogI("HAPCLIENT IP: " + hapClient->client.remoteIP().toString(), true);
					LogI("EVENT IP:     " + event.hapClient->client.remoteIP().toString(), true);


					_eventManager.queueEvent( EventManager::kEventEvent, event);
				} else {
					isEvent = false;
						
					value = chr["value"].as<String>();

					if (c->writable()) {
						LogV("<<< Change value of aid: ", false);
						LogV(String(aid), false);
						LogV(" - iid: ", false);
						LogV(String(iid), false);
						LogV(" to value: " + value, true);

						c->setValue(value);

						error_code = HAP_STATUS_SUCCESS;
						errorOccured = false;

					} else {
						LogE("[ERROR] Characteristics with aid: ", false);
						LogE(String(aid), false);
						LogE(" - iid: ", false);
						LogE(String(iid), false);
						LogE(" not writable! - ErrorCode: ", false);
						LogE(String(HAP_STATUS_READONLY_WRITE), true);

						error_code = HAP_STATUS_READONLY_WRITE;
						errorOccured = true;
					}

				}
	    	}


			if ( onlyOne && (errorOccured == false) ) {
				httpStatusStr = HTTP_204;
			} else {

				httpStatusStr = HTTP_207;

				char c1[3], c2[3];
				sprintf(c1, "%d", aid);
				sprintf(c2, "%d", iid);
				String s[3] = {String(c1), String(c2), String(error_code)};
				String k[3] = {"aid", "iid", "status"};
				if (result.length() != 1) {
					result += ",";
				}

				String _result = HAPHelper::dictionaryWrap(k, s, 3);
				result += _result;
			}

	   	}
		result += "]";

		String d = "characteristics";
		result = HAPHelper::dictionaryWrap(&d, &result, 1);

		if (errorOccured == false) {
			result = "";
		}

		LogD(">>> Sending result: ", false);
		LogD(httpStatusStr, true);
		LogD(result, true);

		sendEncrypt(hapClient, httpStatusStr, result);
	}
}

void HAPServer::handleEvents( int eventCode, struct HAPEvent eventParam )
{
        LogI("<<< Handle event: code: " + String(eventCode) + " - value: ", false);
        LogI(eventParam.value, true);

        if (eventCode == EventManager::kEventEvent) {
#if 0
        	Serial.println("aid: " + String(eventParam.aid));
        	Serial.println("iid: " + String(eventParam.iid));        	
        	Serial.println("value: " + String(eventParam.value));
#endif
        	


        	const size_t bufferSize = JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(1) + JSON_OBJECT_SIZE(3);
			DynamicJsonBuffer jsonBuffer(bufferSize);

			JsonObject& root = jsonBuffer.createObject();

			JsonArray& characteristics = root.createNestedArray("characteristics");

			JsonObject& characteristics_0 = characteristics.createNestedObject();
			characteristics_0["aid"] = eventParam.aid;
			characteristics_0["iid"] = eventParam.iid;


        	// String value = getValueForCharacteristics(eventParam.aid, eventParam.iid);        	

        	size_t outSize = 0;
        	int32_t errorCode = 0;

        	errorCode = getValueForCharacteristics(eventParam.aid, eventParam.iid, NULL, &outSize);

#if HAP_DEBUG
			LogD( "errorCode: " + String(errorCode), true );
			LogD( "outSize: " + String(outSize), true );
#endif

        	if ( errorCode != 0){
        		// TODO:
        		characteristics_0["status"] = errorCode;
        	} else {

        		// outSize = outSize + 1;
				char value[outSize];
				getValueForCharacteristics(eventParam.aid, eventParam.iid, value, &outSize);


#if HAP_DEBUG
				LogD( "value: " + String(value), true );
#endif


				if ( strcmp(value, "true") == 0 ) {
					characteristics_0["value"] = 1;
				} else if ( strcmp(value, "false") == 0 ) {
					characteristics_0["value"] = 0;
				} else {
					characteristics_0["value"] = value;	
				}
        	}
			

			String response;			
			root.printTo(response);

#if HAP_DEBUG
			root.prettyPrintTo(Serial);
#endif

			LogV("Sending EVENT to client [" + eventParam.hapClient->client.remoteIP().toString() + "] ...", true);
			if ( eventParam.hapClient->client.connected() ){				
				sendEncrypt(eventParam.hapClient, EVENT_200, response, false);	
			} else {
				LogW("[WARNING] No client available to send the event to!", true);
			}
        	
        }
};

bool HAPServer::isPaired(){
	return _pairings.size() > 0;
}


String HAPServer::versionString(){
	return _firmware.version.toString();
}


void HAPServer::__setFirmware(const char* name, const char* version) {

	if (strlen(name) + 1 - 10 > MAX_FIRMWARE_NAME_LENGTH || strlen(version) + 1 - 10 > MAX_FIRMWARE_VERSION_LENGTH) {
		LogE( F("[ERROR] Either the name or version string is too long"), true);
		return;  // never reached, here for clarity
	}

	// Remove flags
	char ver[20];
	strncpy(ver, version + 5, strlen(version) - 5);
	ver[strlen(version) - 5] = '\0';

	_firmware.version = HAPVersion(ver);
	_firmwareSet = true;
}

void HAPServer::__setBrand(const char* brand) {

	if (strlen(brand) + 1 - 10 > MAX_BRAND_LENGTH) {
		LogE(F("[ERROR] The brand string is too long"), true);
		return;  // never reached, here for clarity
	}

	strncpy(_brand, brand + 5, strlen(brand) - 10);
	_brand[strlen(brand) - 10] = '\0';
}



HAPServer hap;
