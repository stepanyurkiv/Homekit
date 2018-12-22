#ifndef HAPWEBSERVERAPI_HPP
#define HAPWEBSERVERAPI_HPP

#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESPAsyncWebserver.h>
#include <vector>
#include <functional>

#include "HAPLogger.hpp"

typedef std::function<String(void)> RestApiHandlerFunction;

class HAPWebserverAPI
{
public :
	HAPWebserverAPI(){
	};

	void handleAPI (AsyncWebServerRequest *request);
    void handleGetDebugWifi(AsyncWebServerRequest *request);

    // callbacks
    void setCallbackDebugWiFiClients(RestApiHandlerFunction callback){ _callbackDebugWiFiClients = callback; }
private:    

    RestApiHandlerFunction _callbackDebugWiFiClients;
};

#endif //HAPWEBSERVERAPI_HPP