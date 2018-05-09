//
// HAPPluginSwitch.cpp
// Homekit
//
//  Created on: 18.08.2017
//      Author: michael
//

#include "HAPPluginSwitch.hpp"

#ifndef BUILTIN_LED
#define BUILTIN_LED 13
#endif

HAPPluginSwitch::HAPPluginSwitch(){
    _type = HAP_PLUGIN_TYPE_ACCESSORY;
    _name = "HAPPluginSwitch";
    _isEnabled = true;
    _interval = 0;
    _previousMillis = 0;
    _isOn = true;
}

void identifySwitch(bool oldValue, bool newValue) {
    printf("Start Identify Switch\n");
}

void changeState(bool oldValue, bool newValue) {
    printf("New state: %d\n", newValue);
    if (newValue == true) {
        digitalWrite(BUILTIN_LED, HIGH);    
    } else {
        digitalWrite(BUILTIN_LED, LOW);
    }           
}

void HAPPluginSwitch::handle(HAPAccessorySet* accessorySet, bool forced){


}

HAPAccessory* HAPPluginSwitch::init(){
	LogD("\nInitializing plugin: HAPPluginSwitch ...", false);

	HAPAccessory *accessory = new HAPAccessory();
	HAPAccessory::addInfoServiceToAccessory(accessory, "Builtin Switch", "ACME", "Swtich 1", "123123123", &identifySwitch);

    service = new HAPService(serviceType_switch);
    accessory->addService(service);

    stringCharacteristics *lightServiceName = new stringCharacteristics(charType_serviceName, permission_read, 32);
    lightServiceName->setValue("Schalter 1");
    accessory->addCharacteristics(service, lightServiceName);

    powerState = new boolCharacteristics(charType_on, permission_read|permission_write|permission_notify);
    if (_isOn)
        powerState->setValue("true");
    else
        powerState->setValue("false");
    powerState->valueChangeFunctionCall = &changeState;
    accessory->addCharacteristics(service, powerState);

	LogD("OK", true);

	return accessory;
}


void HAPPluginSwitch::setValue(String oldValue, String newValue){
    LogW("Setting Switch oldValue: " + oldValue + " -> newValue: " + newValue, true);
    powerState->setValue(newValue);

    if (newValue == "true"){
        _isOn = true;
    } else {
        _isOn = false;
    }
}

void HAPPluginSwitch::setValue(uint8_t type, String oldValue, String newValue){
    LogW("Setting Switch oldValue: " + oldValue + " -> newValue: " + newValue, true);
    if (type == charType_on) {
        powerState->setValue(newValue);

        if (newValue == "true"){
            _isOn = true;
        } else {
            _isOn = false;
        }    
    } 

}


String HAPPluginSwitch::getValue(){
    return powerState->value();
}

String HAPPluginSwitch::getValue(uint8_t type){
    if (type == charType_on) {
        return powerState->value();
    } 
    return "";
}