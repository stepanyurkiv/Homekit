//
// HAPWebServer.cpp
// Homekit
//
//  Created on: 14.12.2018
//      Author: michael
//

#include "HAPWebServer.hpp"
#include "ArduinoJson.h"

#include "HAPWebserverAPI.hpp"

#include "HAPDeviceID.hpp"

WebServer* HAPWebServer::_webserver;

HAPWebServer::HAPWebServer(){
	_webserver = new WebServer(80);
	_test = "TEST";
}

void HAPWebServer::handle(){
	_webserver->handleClient();	
}



void HAPWebServer::begin(){
	// _webserver->on("/", HTTP_ANY, std::bind(&HAPWebServer::handleRoot, this, std::placeholders::_1));

 	_webserver->on("/", []() {
    	_webserver->send(200, "text/plain", "hello from esp32!");
  	});

  	_webserver->on("/api/heap", HTTP_GET, []() {
    	_webserver->send(200, "text/plain", String(ESP.getFreeHeap()));

  //   	const size_t bufferSize = JSON_OBJECT_SIZE(1);
		// DynamicJsonBuffer jsonBuffer(bufferSize);

		// JsonObject& root = jsonBuffer.createObject();
		// root["heap"] = ESP.getFreeHeap();

		// String jsonResponse;
  // 		root.prettyPrintTo(jsonResponse);
  // 		_webserver->send ( 200, "application/json", jsonResponse );		

  	});

  	_webserver->on("/api/info", HTTP_GET, []() {
    	//_webserver->send(200, "text/plain", String(ESP.getFreeHeap()));

    	const size_t bufferSize = JSON_OBJECT_SIZE(2);
		DynamicJsonBuffer jsonBuffer(bufferSize);

		JsonObject& root = jsonBuffer.createObject();
		
		root["deviceId"] = HAPDeviceID::deviceID();
		root["heap"] = ESP.getFreeHeap();

		String jsonResponse;
  		root.prettyPrintTo(jsonResponse);
  		_webserver->send ( 200, "application/json", jsonResponse );		

  	});

	_webserver->on("/api/config", HTTP_GET, []() {    	
    	_webserver->send(200, "text/plain", "config: {\"config\": []}");
  	});
  
  	_webserver->on("/api/devices/{}/id/{}", []() {
    	String device = _webserver->pathArg(0);
    	String id = _webserver->pathArg(1);
    	_webserver->send(200, "text/plain", "devices: '" + device + "' and id: '" + id + "'");
  	});

	_webserver->addHandler(new HAPWebserverAPI("/index"));
	_webserver->begin();
}

