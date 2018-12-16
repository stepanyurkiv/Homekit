

#include "HAPWebserverAPI.hpp"

bool HAPWebserverAPI::handle(WebServer& server, HTTPMethod requestMethod, String requestUri) { 
	if (requestMethod != HTTP_GET || requestUri != _uri) {
        return false;
    }

    server.send(200, "text/plain", "This is an api page");
    return true;
}

bool HAPWebserverAPI::canHandle(HTTPMethod method, String uri) {
	if (method != HTTP_GET || uri != _uri) {
        return false;
    }
	return true;
}