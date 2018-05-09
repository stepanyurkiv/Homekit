
#include "HAPPluginLED.hpp"

#ifndef BUILTIN_LED
#define BUILTIN_LED 13
#endif

HAPPluginLED::HAPPluginLED(){
    _type = HAP_PLUGIN_TYPE_ACCESSORY;
    _name = "HAPPluginLED";
    _isEnabled = false;
    _interval = HAP_BLINK_INTERVAL;
    _previousMillis = 0;
    _isOn = true;
}

void identify(bool oldValue, bool newValue) {
    printf("Start Identify Light\n");
}

void changeLightPower(bool oldValue, bool newValue) {
    printf("New light power state: %d\n", newValue);

    if (newValue == true) {
        digitalWrite(BUILTIN_LED, HIGH);    
    } else {
        digitalWrite(BUILTIN_LED, LOW);
    }        
}

void changeBrightness(int oldValue, int newValue){
    //printf("New brightness state: %d\n", newValue);
}


void HAPPluginLED::handle(HAPAccessorySet* accessorySet, bool forced){

    if (shouldHandle() || forced) {
        
        /*
        LogW("Handle LED", true);
        if (_isOn)
            setValue("true", "false");
        else
            setValue("false", "true");
        */

        uint32_t freeMem = ESP.getFreeHeap();
        //Serial.println(freeMem);
        
        float percentage = 100 / 2;
        //Serial.printf("%f\n", percentage);
        brightnessState->setValue(String(percentage));
    }
}

HAPAccessory* HAPPluginLED::init(){
	LogD("\nInitializing plugin: HAPPluginLED ...", false);

    pinMode(BUILTIN_LED, OUTPUT);    
    digitalWrite(BUILTIN_LED, _isOn); 

	HAPAccessory *accessory = new HAPAccessory();
	HAPAccessory::addInfoServiceToAccessory(accessory, "Builtin LED", "ACME", "LED", "123123123", &identify);

    service = new HAPService(serviceType_lightBulb);
    accessory->addService(service);

    stringCharacteristics *lightServiceName = new stringCharacteristics(charType_serviceName, permission_read, 32);
    lightServiceName->setValue("Light");
    accessory->addCharacteristics(service, lightServiceName);

    powerState = new boolCharacteristics(charType_on, permission_read|permission_write|permission_notify);
    if (_isOn)
        powerState->setValue("true");
    else
        powerState->setValue("false");
    powerState->valueChangeFunctionCall = &changeLightPower;
    accessory->addCharacteristics(service, powerState);

    
    brightnessState = new intCharacteristics(charType_brightness, permission_read|permission_write, 0, 100, 1, unit_percentage);
    brightnessState->setValue("50");
    brightnessState->valueChangeFunctionCall = &changeBrightness;
    //accessory->addCharacteristics(service, brightnessState);    

	LogD("OK", true);

	return accessory;
}


void HAPPluginLED::setValue(String oldValue, String newValue){
    //LogW("Setting LED oldValue: " + oldValue + " -> newValue: " + newValue, true);
    powerState->setValue(newValue);

    if (newValue == "true"){
        _isOn = true;
    } else {
        _isOn = false;
    }
}

void HAPPluginLED::setValue(uint8_t type, String oldValue, String newValue){
    //LogW("Setting LED oldValue: " + oldValue + " -> newValue: " + newValue, true);
    if (type == charType_on) {
        powerState->setValue(newValue);

        if (newValue == "true"){
            _isOn = true;
        } else {
            _isOn = false;
        }    
    } else if (type == charType_brightness) {        
        brightnessState->setValue(newValue);
    }

}


String HAPPluginLED::getValue(){
    return powerState->value();
}

String HAPPluginLED::getValue(uint8_t type){
    if (type == charType_on) {
        return powerState->value();
    } else if (type == charType_brightness) {
        return brightnessState->value();
    }
    return "";
}