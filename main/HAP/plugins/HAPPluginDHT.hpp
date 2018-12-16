//
// HAPPluginDHT.hpp
// Homekit
//
//  Created on: 22.04.2018
//      Author: michael
//

#ifndef HAPPLUGINDHT_HPP_
#define HAPPLUGINDHT_HPP_

#include <Arduino.h>
#include "HAPPlugins.hpp"
#include "HAPLogger.hpp"
#include "HAPAccessory.hpp"

class HAPPluginDHT: public HAPPlugin {
public:

	HAPPluginDHT();

	HAPAccessory* init(EventManager* eventManager = nullptr);
	void setValue(String oldValue, String newValue);
	void setValue(uint8_t type, String oldValue, String newValue);

	String getValue();
	String getValue(uint8_t type);

    void handle(HAPAccessorySet* accessorySet, bool forced = false);	
	void handleEvents(int eventCode, struct HAPEvent eventParam);
private:
	intCharacteristics *humValue;
	floatCharacteristics *tempValue;
};

REGISTER_PLUGIN(HAPPluginDHT)

#endif /* HAPPLUGINS_HPP_ */ 