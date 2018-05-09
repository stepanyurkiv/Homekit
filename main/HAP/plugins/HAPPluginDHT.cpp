
#include "HAPPluginDHT.hpp"


#define HAP_PLUGIN_INTERVAL 2000

HAPPluginDHT::HAPPluginDHT(){
	_type = HAP_PLUGIN_TYPE_ACCESSORY;
	_name = "HAPPluginDHT";
	_isEnabled = false;
	_interval = HAP_PLUGIN_INTERVAL;
	_previousMillis = 0;

	randomSeed(analogRead(0));
}

void identifyDHT(bool oldValue, bool newValue) {
	Serial.printf("Start Identify DHT22\n");
}

void changeTemp(float oldValue, float newValue) {
	//Serial.printf("New temperature %f\n", newValue);
}

void changeHum(int oldValue, int newValue) {
	//Serial.printf("New humidity %d\n", newValue);
}


void HAPPluginDHT::handle(HAPAccessorySet* accessorySet, bool forced){	
	if (shouldHandle() || forced) {		
		setValue(charType_currentTemperature, getValue(charType_currentTemperature), String(random(0,10)));
		setValue(charType_currentHumidity, getValue(charType_currentHumidity), String(random(0,100)));
	}
}

void HAPPluginDHT::setValue(String oldValue, String newValue){

}

void HAPPluginDHT::setValue(uint8_t type, String oldValue, String newValue){
	if (type == charType_currentTemperature) {
		//LogW("Setting DHT TEMPERATURE oldValue: " + oldValue + " -> newValue: " + newValue, true);
		tempValue->setValue(newValue);
	} else if (type == charType_currentHumidity) {
		//LogW("Setting DHT HUMIDITY oldValue: " + oldValue + " -> newValue: " + newValue, true);
		humValue->setValue(newValue);
	}
}

String HAPPluginDHT::getValue(){
	return "";
}

String HAPPluginDHT::getValue(uint8_t type){
	if (type == charType_currentTemperature) {		
		return tempValue->value();
	} else if (type == charType_currentHumidity) {
		return humValue->value();
	}
	return "";
}

HAPAccessory* HAPPluginDHT::init(){
	LogD("\nInitializing plugin: " + _name + " ...", false);

	HAPAccessory *accessory = new HAPAccessory();
	HAPAccessory::addInfoServiceToAccessory(accessory, "DHT 1", "ET", "DHT", "12345678", &identifyDHT);


	HAPService *tempService = new HAPService(serviceType_temperatureSensor);
	accessory->addService(tempService);

	stringCharacteristics *tempServiceName = new stringCharacteristics(charType_serviceName, permission_read, 0);
	tempServiceName->setValue("Temperature Sensor");
	accessory->addCharacteristics(tempService, tempServiceName);

	//floatCharacteristics(uint8_t _type, int _permission, float minVal, float maxVal, float step, unit charUnit): characteristics(_type, _permission), _minVal(minVal), _maxVal(maxVal), _step(step), _unit(charUnit)

	tempValue = new floatCharacteristics(charType_currentTemperature, permission_read, -50, 100, 0.1, unit_celsius);
	tempValue->setValue("0.0");
	tempValue->valueChangeFunctionCall = &changeTemp;
	accessory->addCharacteristics(tempService, tempValue);



	HAPService *humService = new HAPService(serviceType_humiditySensor);
	accessory->addService(humService);

	stringCharacteristics *humServiceName = new stringCharacteristics(charType_serviceName, permission_read, 0);
	humServiceName->setValue("Humidity Sensor");
	accessory->addCharacteristics(humService, humServiceName);

	//intCharacteristics(uint8_t _type, int _permission, int minVal, int maxVal, int step, unit charUnit): characteristics(_type, _permission), _minVal(minVal), _maxVal(maxVal), _step(step), _unit(charUnit)

	humValue = new intCharacteristics(charType_currentHumidity, permission_read, 0, 100, 0.1, unit_percentage);
	humValue->setValue("0");
	humValue->valueChangeFunctionCall = &changeHum;
	accessory->addCharacteristics(humService, humValue);


	LogD("OK", true);

	return accessory;
}