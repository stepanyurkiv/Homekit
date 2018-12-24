
#include "HAPPluginDHT.hpp"


#define HAP_PLUGIN_INTERVAL 5000

#define VERSION_MAJOR       0
#define VERSION_MINOR       0
#define VERSION_REVISION    1
#define VERSION_BUILD       1


HAPPluginDHT::HAPPluginDHT(){
	_type = HAP_PLUGIN_TYPE_ACCESSORY;
	_name = "HAPPluginDHT";
	_isEnabled = true;
	_interval = HAP_PLUGIN_INTERVAL;
	_previousMillis = 0;

    _version.major      = VERSION_MAJOR;
    _version.minor      = VERSION_MINOR;
    _version.revision   = VERSION_REVISION;
    _version.build      = VERSION_BUILD;

	randomSeed(analogRead(0));
}

void HAPPluginDHT::identify(bool oldValue, bool newValue) {
	Serial.printf("Start Identify DHT22\n");
}

void HAPPluginDHT::changeTemp(float oldValue, float newValue) {
	Serial.printf("New temperature: %f\n", newValue);
}

void HAPPluginDHT::changeHum(float oldValue, float newValue) {
	Serial.printf("New humidity: %f\n", newValue);
}


void HAPPluginDHT::handle(bool forced){	
	if (shouldHandle() || forced) {		
		LogD("Handle " + String(__PRETTY_FUNCTION__) + " - interval: " + String(interval()), true);

		setValue(charType_currentTemperature, getValue(charType_currentTemperature), String(random(20,70)) + "." + String(random(0,9)));
		setValue(charType_currentHumidity, getValue(charType_currentHumidity), String(random(20,70)) + "." + String(random(0,9)));

		// Add event
		struct HAPEvent event = HAPEvent(nullptr, _accessory->aid, _humidityValue->iid, _humidityValue->value());							
		_eventManager->queueEvent( EventManager::kEventFromController, event);

		event = HAPEvent(nullptr, _accessory->aid, _temperatureValue->iid, _temperatureValue->value());							
		_eventManager->queueEvent( EventManager::kEventFromController, event);
	}
}

// void HAPPluginDHT::handleEvents(int eventCode, struct HAPEvent eventParam){
// 	LogE("!!!!!!!!!!! HANDLE PLUGIN EVENT !!!!!!!!!!!!!!!", true);
// }

void HAPPluginDHT::setValue(String oldValue, String newValue){

}

void HAPPluginDHT::setValue(uint8_t type, String oldValue, String newValue){
	if (type == charType_currentTemperature) {		
		_temperatureValue->setValue(newValue);
	} else if (type == charType_currentHumidity) {
		//LogW("Setting DHT HUMIDITY oldValue: " + oldValue + " -> newValue: " + newValue, true);
		_humidityValue->setValue(newValue);
	}
}

String HAPPluginDHT::getValue(){
	return "";
}

String HAPPluginDHT::getValue(uint8_t type){
	if (type == charType_currentTemperature) {		
		return _temperatureValue->value();
	} else if (type == charType_currentHumidity) {
		return _humidityValue->value();
	}
	return "";
}

HAPAccessory* HAPPluginDHT::initAccessory(){
	LogD("\nInitializing plugin: " + _name + " ...", false);

	_accessory = new HAPAccessory();
	//HAPAccessory::addInfoServiceToAccessory(_accessory, "DHT 1", "ET", "DHT", "12345678", std::bind(&identifyDHT), version() );
	auto callbackIdentify = std::bind(&HAPPlugin::identify, this, std::placeholders::_1, std::placeholders::_2);
    HAPAccessory::addInfoServiceToAccessory(_accessory, "DHT Sensor", "ACME", "LED", "123123123", callbackIdentify, version());

	_temperatureService = new HAPService(serviceType_temperatureSensor);
	_accessory->addService(_temperatureService);

	stringCharacteristics *tempServiceName = new stringCharacteristics(charType_serviceName, permission_read, 0);
	tempServiceName->setValue("Temperature Sensor");
	_accessory->addCharacteristics(_temperatureService, tempServiceName);

	//floatCharacteristics(uint8_t _type, int _permission, float minVal, float maxVal, float step, unit charUnit): characteristics(_type, _permission), _minVal(minVal), _maxVal(maxVal), _step(step), _unit(charUnit)
	_temperatureValue = new floatCharacteristics(charType_currentTemperature, permission_read|permission_notify, -50, 100, 0.1, unit_celsius);
	_temperatureValue->setValue("0.0");
	auto callbackChangeTemp = std::bind(&HAPPluginDHT::changeTemp, this, std::placeholders::_1, std::placeholders::_2);
	//_temperatureValue->valueChangeFunctionCall = std::bind(&changeTemp);
	_temperatureValue->valueChangeFunctionCall = callbackChangeTemp;
	_accessory->addCharacteristics(_temperatureService, _temperatureValue);

	_humidityService = new HAPService(serviceType_humiditySensor);
	_accessory->addService(_humidityService);

	stringCharacteristics *humServiceName = new stringCharacteristics(charType_serviceName, permission_read, 0);
	humServiceName->setValue("Humidity Sensor");
	_accessory->addCharacteristics(_humidityService, humServiceName);

	//intCharacteristics(uint8_t _type, int _permission, int minVal, int maxVal, int step, unit charUnit): characteristics(_type, _permission), _minVal(minVal), _maxVal(maxVal), _step(step), _unit(charUnit)
	_humidityValue = new floatCharacteristics(charType_currentHumidity, permission_read|permission_notify, 0, 100, 0.1, unit_percentage);
	_humidityValue->setValue("0.0");

	auto callbackChangeHum = std::bind(&HAPPluginDHT::changeHum, this, std::placeholders::_1, std::placeholders::_2);
	//_humidityValue->valueChangeFunctionCall = std::bind(&changeHum);
	_humidityValue->valueChangeFunctionCall = callbackChangeHum;
	_accessory->addCharacteristics(_humidityService, _humidityValue);


	LogD("OK", true);

	return _accessory;
}