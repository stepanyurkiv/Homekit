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
#include <Webserver.h>
#include <functional>

#include "HAPGlobals.hpp"
#include "HAPLogger.hpp"

class HAPWebServer {
public:
	HAPWebServer();
	// ~HAPWebServer();
	
	void begin();

	void handle();

private:
	static WebServer* _webserver;
	String _test;
};

#endif /* HAPWEBSERVER_HPP_ */
