

#include "HAPWebserverAPI.hpp"


void HAPWebserverAPI::handleAPI(AsyncWebServerRequest *request){
    LogV("Webserver request URL: " + request->url(), true);
                
    std::vector<String> urlSegements;
    char* ptr = strtok((char*)request->url().c_str(), "/");
        
    while(ptr){
        urlSegements.push_back(ptr);
        ptr = strtok(NULL, "/");
    }

    if ( urlSegements[1] == "debug" ){

        if ( request->method() == HTTP_GET && urlSegements[2] == "wifi" ){
            handleGetDebugWifi(request);
        }

    }

    request->send(200, "text/plain", "api: " + urlSegements[0] + " mod: " + urlSegements[1] + " op: " + urlSegements[2]);
    urlSegements.clear();
}

void HAPWebserverAPI::handleGetDebugWifi(AsyncWebServerRequest *request){
    _callbackDebugWiFiClients();
    
    request->send(200, "text/plain", request->url());
}