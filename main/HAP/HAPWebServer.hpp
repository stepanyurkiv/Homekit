//
// HAPWebServer.hpp
// Homekit
//
//  Created on: 14.12.2018
//      Author: michael
//

#ifndef HAPWEBSERVER_HPP_
#define HAPWEBSERVER_HPP_

#include <Arduino.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <functional>

// #include "HAPWebserverAPI.hpp"
#include "HAPGlobals.hpp"
#include "HAPLogger.hpp"

typedef std::function<String(void)> RestApiHandlerFunction;

class HAPWebServer {
public:
	HAPWebServer(uint16_t port = 80);
	// ~HAPWebServer();

	void begin();

	// callbacks
#if HAP_API_ADMIN_MODE	
	void setCallbackApiAccessories(RestApiHandlerFunction callback){ 
		_callbackApiAccessories = callback; 
	}
#endif

#if HAP_DEBUG
    void setCallbackApiDebugHapClients(RestApiHandlerFunction callback){ 
		_callbackApiDebugHapClients = callback; 
	}
#endif

	void notFound(AsyncWebServerRequest *request);

private:
	static AsyncWebServer*	 _webserver;



#if HAP_API_ADMIN_MODE
	RestApiHandlerFunction	 _callbackApiDebugHapClients;
	RestApiHandlerFunction	 _callbackApiAccessories;
#endif

	// HAPWebserverAPI			 _webserverAPI;
};

#endif /* HAPWEBSERVER_HPP_ */
