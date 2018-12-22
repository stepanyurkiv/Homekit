//
// HAPWebServer.cpp
// Homekit
//
//  Created on: 14.12.2018
//      Author: michael
//

#include <ArduinoJson.h>
#include "HAPWebServer.hpp"
#include "HAPDeviceID.hpp"
#include "HAPHelper.hpp"

AsyncWebServer* HAPWebServer::_webserver;

HAPWebServer::HAPWebServer(uint16_t port){
	_webserver = new AsyncWebServer(port);
}

void HAPWebServer::begin(){
	
	_webserver->on("/", HTTP_GET, [](AsyncWebServerRequest *request){
		LogD(__PRETTY_FUNCTION__,false);		
		LogD( " url: " + request->url(),true);		
        request->send(200, "text/plain", "Hello, world");
    });

#if HAP_API_ADMIN_MODE
	_webserver->on("/api/accessories", HTTP_GET, [this](AsyncWebServerRequest *request){
		LogD(__PRETTY_FUNCTION__,false);		
		LogD( " url: " + request->url(),true);		
		if (request->url() == "/api/accessories")	{
    		request->send(200, "application/json", _callbackApiAccessories());
		} else {
			notFound(request);
		}    	
    });

#endif


#if HAP_DEBUG	
	_webserver->on("/api/debug/clients", HTTP_GET, [this](AsyncWebServerRequest *request){
		LogD(__PRETTY_FUNCTION__,false);		
		LogD( " url: " + request->url(),true);	
		if (request->url() == "/api/debug/clients")	{
    		request->send(200, "application/json", _callbackApiDebugHapClients());
		} else {
			notFound(request);
		}			
    });

	_webserver->on("/api/debug", HTTP_GET, [this](AsyncWebServerRequest *request){
		LogD(__PRETTY_FUNCTION__,false);		
		LogD( " url: " + request->url(),true);
		
		if (request->url() == "/api/debug"){
			String key = "debug";			
			String value = "{";
			
			value += HAPHelper::removeBrackets(_callbackApiDebugHapClients());			
			
			value += ",";	
			
			value += HAPHelper::removeBrackets(_callbackApiAccessories());
			
			value += "}";

			request->send(200, "application/json", HAPHelper::dictionaryWrap(&key, &value, 1));
		} else {
			notFound(request);
		}

    });
#endif



	_webserver->onNotFound(std::bind(&HAPWebServer::notFound, this, std::placeholders::_1));

	// _webserver->on("/api", HTTP_ANY, std::bind(&HAPWebserverAPI::handleAPI, _webserverAPI, std::placeholders::_1));

	_webserver->begin();
	
}


void HAPWebServer::notFound(AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Not found");
}

