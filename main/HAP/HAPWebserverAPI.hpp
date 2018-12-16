#ifndef HAPWEBSERVERAPI_HPP
#define HAPWEBSERVERAPI_HPP

#include <WebServer.h>

class HAPWebserverAPI : public RequestHandler {
public:
    HAPWebserverAPI(const char* uri = "api") : _uri(uri){}

    bool handle(WebServer& server, HTTPMethod requestMethod, String requestUri) override;    
    bool canHandle(HTTPMethod method, String uri) override;

protected:
    String _uri;  
};


#endif //HAPWEBSERVERAPI_HPP