//
// HAPAccessorySet.hpp
// Homekit
//
//  Created on: 18.08.2017
//      Author: michael
//

#ifndef HAPACCESSORYSET_HPP_
#define HAPACCESSORYSET_HPP_

#include <Arduino.h>
#include <vector>

#include "HAPAccessory.hpp"

enum HAP_ACCESSORY_TYPE{
    HAP_ACCESSORY_TYPE_OTHER 				= 0,
    HAP_ACCESSORY_TYPE_BRIDGE				= 2,
    HAP_ACCESSORY_TYPE_FAN				 	= 3,
    HAP_ACCESSORY_TYPE_GARAGEDOOROPENER		= 4,
    HAP_ACCESSORY_TYPE_LIGHTBULB			= 5,
    HAP_ACCESSORY_TYPE_DOORLOCK		 		= 6,
    HAP_ACCESSORY_TYPE_OUTLET 				= 7,
    HAP_ACCESSORY_TYPE_SWITCH 				= 8,
    HAP_ACCESSORY_TYPE_THERMOSTAT	 		= 9,
    HAP_ACCESSORY_TYPE_SENSOR 				= 10,
    HAP_ACCESSORY_TYPE_ALARMSYSTEM 			= 11,
    HAP_ACCESSORY_TYPE_DOOR 				= 12,
    HAP_ACCESSORY_TYPE_WINDOW 				= 13,
    HAP_ACCESSORY_TYPE_WINDOWCOVER 			= 14,
    HAP_ACCESSORY_TYPE_PROGRAMMABLESWITCH 	= 15,
};

class HAPAccessorySet {
public:
	HAPAccessorySet();
	~HAPAccessorySet();


	void begin();
	bool isPaired;

	uint32_t configurationNumber();
	void setConfigurationNumber(uint32_t number);

	uint8_t accessoryType();
	void setAccessoryType(enum HAP_ACCESSORY_TYPE accessoryType);
	
	void addAccessoryInfo();

	const char* setupID();
	void generateSetupID();

	void setModelName(String name);
	const char* modelName();

	const char* setupHash();

	void setPinCode(const char* pinCode);
	const char* pinCode();
	const char* xhm();

	String describe();
	bool removeAccessory(HAPAccessory *acc);
	void addAccessory(HAPAccessory *acc);
	HAPAccessory* accessoryAtIndex(uint8_t index);
	HAPAccessory* accessoryWithAID(uint8_t aid);

	int32_t getValueForCharacteristics(int aid, int iid, char* out, size_t* outSize);
	characteristics* getCharacteristics(int aid, int iid);

	uint8_t numberOfAccessory();

protected:
	uint32_t 	_configurationNumber;
	enum HAP_ACCESSORY_TYPE 	_accessoryType;

	// Setup ID can be provided, although, per spec, should be random
	// every time the instance is started. If not provided on init, will be random.
	// 4 digit string 0-9 A-Z
	String 		_setupID;
	String 		_setupHash;
	String		_xhm;

	String 		_modelName;	
	String 		_pinCode;	// xxx-xx-xxx

private:	
	void computeSetupHash();
	char randomChar(char* letters);

	void generateXMI();

	std::vector<HAPAccessory *> _accessories;
    int _aid = 0;

    //AccessorySet(AccessorySet const&);
    //void operator=(AccessorySet const&);
};

#endif /* HAPACCESSORYSET_HPP_ */
