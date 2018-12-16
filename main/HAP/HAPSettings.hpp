//
// HAPSettings.hpp
// Homekit
//
//  Created on: 14.12.2018
//      Author: michael
//

#ifndef HAPSETTINGS_HPP_
#define HAPSETTINGS_HPP_

#include <Arduino.h>

#include "HAPGlobals.hpp"
#include "HAPHashMap.hpp"

class HAPSettingsPlugin {
public:        
    HAPSettingsPlugin(String _name, bool _enabled){
        name = _name;
        enabled = _enabled 
    }
    
    HAPHashMap<String, String, HAP_PLUGIN_MAX_PARAMETER> params;
    String name;
    bool enabled;    
}


class HAPSettings {
public:
	HAPSettings();
	// ~HAPWebServer();
	
    String deviceName;
    String pinCode;
};

#endif /* HAPSETTINGS_HPP_ */