//
// HAPPluginSwitch.hpp
// Homekit
//
//  Created on: 22.04.2018
//      Author: michael
//

#ifndef HAPPLUGINSWITCH_HPP_
#define HAPPLUGINSWITCH_HPP_

#include <Arduino.h>
#include "HAPPlugins.hpp"
#include "HAPLogger.hpp"
#include "HAPAccessory.hpp"

class HAPPluginSwitch: public HAPPlugin {
public:

	HAPPluginSwitch();
	HAPAccessory* init();

	void setValue(String oldValue, String newValue);
	void setValue(uint8_t type, String oldValue, String newValue);

	String getValue();
	String getValue(uint8_t type);

	void handle(HAPAccessorySet* accessorySet, bool forced=false);

private:

	HAPService *service;	
	boolCharacteristics *powerState;

	bool _isOn;
};

REGISTER_PLUGIN(HAPPluginSwitch)

#endif /* HAPPLUGINSWITCH_HPP_ */ 