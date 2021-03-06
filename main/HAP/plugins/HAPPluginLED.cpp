
#include "HAPPluginLED.hpp"

#ifndef BUILTIN_LED
#define BUILTIN_LED 13
#endif

#define VERSION_MAJOR       0
#define VERSION_MINOR       0
#define VERSION_REVISION    5
#define VERSION_BUILD       0

#define ENABLE_BRIGHTNESS   0

HAPPluginLED::HAPPluginLED(){
    _type = HAP_PLUGIN_TYPE_ACCESSORY;
    _name = "HAPPluginLED";
    _isEnabled = false;
    _interval = HAP_BLINK_INTERVAL;
    _previousMillis = 0;
    _isOn = true;

    _version.major      = VERSION_MAJOR;
    _version.minor      = VERSION_MINOR;
    _version.revision   = VERSION_REVISION;
    _version.build      = VERSION_BUILD;
}

void HAPPluginLED::changePower(bool oldValue, bool newValue) {
    printf("New light power state: %d\n", newValue);

    if (newValue == true) {
        digitalWrite(BUILTIN_LED, HIGH);    
    } else {
        digitalWrite(BUILTIN_LED, LOW);
    }      
}

void HAPPluginLED::changeBrightness(int oldValue, int newValue){
    printf("New brightness state: %d\n", newValue);
}

void HAPPluginLED::handle(bool forced){
    
    if (shouldHandle() || forced) {        

        LogD("Handle " + String(__PRETTY_FUNCTION__) + " - interval: " + String(interval()), true);

        if (_isOn)
            setValue("true", "false");
        else
            setValue("false", "true");

        // Add event
		struct HAPEvent event = HAPEvent(nullptr, _accessory->aid, _powerState->iid, _powerState->value());							
		_eventManager->queueEvent( EventManager::kEventFromController, event);

        uint32_t freeMem = ESP.getFreeHeap();        
        uint8_t percentage = ( freeMem * 100) / 245000;        
        _brightnessState->setValue(String(percentage));

        // Add event
		struct HAPEvent eventB = HAPEvent(nullptr, _accessory->aid, _brightnessState->iid, _brightnessState->value());							
		_eventManager->queueEvent( EventManager::kEventFromController, eventB);
    }
}

HAPAccessory* HAPPluginLED::initAccessory(){
	LogD("\nInitializing plugin: HAPPluginLED ...", false);

    pinMode(BUILTIN_LED, OUTPUT);    
    digitalWrite(BUILTIN_LED, _isOn); 

	_accessory = new HAPAccessory();
	//HAPAccessory::addInfoServiceToAccessory(_accessory, "Builtin LED", "ACME", "LED", "123123123", &identify);
    auto callbackIdentify = std::bind(&HAPPlugin::identify, this, std::placeholders::_1, std::placeholders::_2);
    HAPAccessory::addInfoServiceToAccessory(_accessory, "Builtin LED", "ACME", "LED", "123123123", callbackIdentify, version());

    _service = new HAPService(serviceType_lightBulb);
    _accessory->addService(_service);

    stringCharacteristics *lightServiceName = new stringCharacteristics(charType_serviceName, permission_read, 32);
    lightServiceName->setValue("Light");
    _accessory->addCharacteristics(_service, lightServiceName);

    _powerState = new boolCharacteristics(charType_on, permission_read|permission_write|permission_notify);
    if (_isOn)
        _powerState->setValue("true");
    else
        _powerState->setValue("false");
    
    auto callbackPowerState = std::bind(&HAPPluginLED::changePower, this, std::placeholders::_1, std::placeholders::_2);        
    _powerState->valueChangeFunctionCall = callbackPowerState;
    _accessory->addCharacteristics(_service, _powerState);

    
    _brightnessState = new intCharacteristics(charType_brightness, permission_read|permission_write|permission_notify, 0, 100, 1, unit_percentage);
        //_brightnessState->valueChangeFunctionCall = &changeBrightness;

#if ENABLE_BRIGHTNESS   
    _brightnessState->setValue("50");
    auto callbackBrightness = std::bind(&HAPPluginLED::changeBrightness, this, std::placeholders::_1, std::placeholders::_2);        
    _brightnessState->valueChangeFunctionCall = callbackBrightness;
    _accessory->addCharacteristics(_service, _brightnessState);    
#endif

	LogD("OK", true);

	return _accessory;
}


void HAPPluginLED::setValue(String oldValue, String newValue){
    //LogW("Setting LED oldValue: " + oldValue + " -> newValue: " + newValue, true);
    _powerState->setValue(newValue);

    if (newValue == "true"){
        _isOn = true;
    } else {
        _isOn = false;
    }
}

void HAPPluginLED::setValue(uint8_t type, String oldValue, String newValue){
    LogW("-> Setting LED oldValue: " + oldValue + " -> newValue: " + newValue, true);
    if (type == charType_on) {
        _powerState->setValue(newValue);

        if (newValue == "true"){
            _isOn = true;
        } else {
            _isOn = false;
        }    

        struct HAPEvent event = HAPEvent(nullptr, _accessory->aid, _powerState->iid, _powerState->value());							
	    _eventManager->queueEvent( EventManager::kEventFromController, event);
 
    } else if (type == charType_brightness) {        
        _brightnessState->setValue(newValue);
    }

}


String HAPPluginLED::getValue(){
    return _powerState->value();
}

String HAPPluginLED::getValue(uint8_t type){
    if (type == charType_on) {
        return _powerState->value();
    } else if (type == charType_brightness) {
        return _brightnessState->value();
    }
    return "";
}

void HAPPluginLED::identify(bool oldValue, bool newValue) {
    printf("Start Identify Light from member\n");
}