//
// HAPPluginLED.hpp
// Homekit
//
//  Created on: 22.04.2018
//      Author: michael
//

#ifndef HAPPLUGINLED_HPP_
#define HAPPLUGINLED_HPP_

#include <Arduino.h>
#include "HAPPlugins.hpp"
#include "HAPLogger.hpp"
#include "HAPAccessory.hpp"

#define HAP_BLINK_INTERVAL 1000

class HAPPluginLED: public HAPPlugin {
public:

	HAPPluginLED();
	HAPAccessory* init(EventManager* eventManager = nullptr);

	void setValue(String oldValue, String newValue);
	void setValue(uint8_t type, String oldValue, String newValue);

	String getValue();
	String getValue(uint8_t type);

	void handle(HAPAccessorySet* accessorySet, bool forced=false);
	void handleEvents(int eventCode, struct HAPEvent eventParam);
private:

	HAPService *service;	
	boolCharacteristics *powerState;
	intCharacteristics *brightnessState;

	bool _isOn;
};

REGISTER_PLUGIN(HAPPluginLED)

#endif /* HAPPLUGINLED_HPP_ */ 