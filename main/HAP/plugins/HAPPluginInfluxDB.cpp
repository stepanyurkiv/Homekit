//
// HAPPluginInfluxDB.cpp
// Homekit
//
//  Created on: 29.04.2018
//      Author: michael
//

#include "HAPPluginInfluxDB.hpp"
#include "HAPHelper.hpp"
#include "HAPLogger.hpp"
#include "HAPCharacteristics.hpp"

#define HAP_PLUGIN_INTERVAL 		30000	// 30 sec
#define HAP_INFLUXDB_TIMEOUT 		10000	// 10 sec

#ifndef HAP_ARDUINOJSON_BUFFER_SIZE
#define HAP_ARDUINOJSON_BUFFER_SIZE 2048
#endif

#define VERSION_MAJOR       0
#define VERSION_MINOR       0
#define VERSION_REVISION    1
#define VERSION_BUILD       1



const char *INFLUXDB_HOST = "192.168.178.137";
const uint16_t INFLUXDB_PORT = 8086;

const char *DATABASE = "homekit";
const char *DB_USER = "admin";
const char *DB_PASSWORD = "admin";

const uint8_t INFLUXDB_IGNORED_SERVICE_TYPES[] = {
	serviceType_accessoryInfo
};

HAPPluginInfluxDB::HAPPluginInfluxDB() {
	
	_type = HAP_PLUGIN_TYPE_STORAGE;
	_name = "HAPPluginInfluxDB";
	_isEnabled = true;
	_interval = HAP_PLUGIN_INTERVAL;
	_previousMillis = 0;	
	_openedDb = false;
	_influxdb = new Influxdb(INFLUXDB_HOST, INFLUXDB_PORT);	

    _version.major      = VERSION_MAJOR;
    _version.minor      = VERSION_MINOR;
    _version.revision   = VERSION_REVISION;
    _version.build      = VERSION_BUILD;

	_eventManager		= nullptr;
}

HAPAccessory* HAPPluginInfluxDB::init(EventManager* eventManager){
	listenerMemberFunctionPlugin.mObj = this;
	listenerMemberFunctionPlugin.mf = &HAPPlugin::handleEvents;
	
	// Add listener to event manager
	_eventManager = eventManager;
	_eventManager->addListener( EventManager::kEventEvent, &listenerMemberFunctionPlugin );
	
	return nullptr;
}

bool HAPPluginInfluxDB::openDB(){	

	LogD("Pinging influxdb server ...", false);
	bool isOnline = _influxdb->ping();
	if (isOnline)
		LogD("OK - Online", true);
	else
		LogD("ERROR - Influx server is offline or not responding", true);
	
	_openedDb = true;
	if (isOnline) {
		LogD("Opening database ...", false);
		_influxdb->opendb(DATABASE, DB_USER, DB_PASSWORD);	
		unsigned long started = millis();
		while (_openedDb != DB_SUCCESS) {
			_openedDb = _influxdb->opendb(DATABASE);

			if (millis() == started + HAP_INFLUXDB_TIMEOUT) {
				LogW("[ERROR] Opend database failed! - Timed out", true);
				_openedDb = false;
				break;
			}
		}
	} else {
		_openedDb = false;
	}

	if (_openedDb)
		LogD("OK", true);
	else
		LogD("ERROR", true);

	return _openedDb;
}

void HAPPluginInfluxDB::handle(HAPAccessorySet* accessorySet, bool forced){	
	if (shouldHandle() || forced) {
		LogD("Handle InfluxDB", true);

		if (_openedDb == false) {
			_openedDb = openDB();
		}

		//LogD(accessorySet->describe(), true);

		DynamicJsonBuffer jsonBuffer(HAP_ARDUINOJSON_BUFFER_SIZE);

		JsonObject& root = jsonBuffer.parseObject(accessorySet->describe());
		if (!root.success()) {
			LogE("[ERROR] failed to parse accessorySet!", true);
			return;
		}
		//root.prettyPrintTo(Serial);

		JsonArray& accessories = root["accessories"];

		for (auto a : accessories){
			JsonArray& services = a["services"];
			//services.prettyPrintTo(Serial);
			for (auto s : services){
			
				uint8_t *sType = HAPHelper::hexToBin(s["type"].as<char*>());		
				if (*sType != serviceType_accessoryInfo) {
					handleService(s);			
				}	
				free(sType);	
			}
		}


	}
}



void HAPPluginInfluxDB::handleEvents(int eventCode, struct HAPEvent eventParam){
	LogE("<<< Handle plugin event: " + String(__PRETTY_FUNCTION__), false);
	LogE(" code: " + String(eventCode) + " - value: ", false);
	LogE(eventParam.value, true);
}

void HAPPluginInfluxDB::handleService(JsonObject& service){
	
	JsonArray& chrs = service["characteristics"];
	String sName = getServiceName(chrs);

	std::vector<dbMeasurement> vec;
	
	
	for (auto chr : chrs){

		uint8_t *cType = HAPHelper::hexToBin(chr["type"].as<char*>());		
		if (*cType != charType_serviceName) {

			dbMeasurement row = dbMeasurement("_tmp_");			
			handleCharacteristic(chr, &row);			

			vec.push_back(row);

		}
		free(cType);
		
	}
	

	for (dbMeasurement row : vec) {

    	row.setMeasurement(sName);
    	
    	LogD(">>> " + sName, false);
    	LogD(": " + row.postString(), true);
    	bool result = _influxdb->write(row) == DB_SUCCESS;
    	LogD( result ? "OK" : "[ERROR] Updating failed", true);

		// Empty field object.
		row.empty();
		
	}
	vec.clear();
	

	//LogD(">>> " + sName, true);

}

String HAPPluginInfluxDB::getServiceName(JsonArray& chrs){
	for (auto chr : chrs){
		uint8_t *cType = HAPHelper::hexToBin(chr["type"].as<char*>());
		if (*cType == charType_serviceName) {
			free(cType);
			return chr["value"].as<String>();
		}
		free(cType);
	}
	return "";
}

void HAPPluginInfluxDB::handleCharacteristic(JsonObject& chr, dbMeasurement* row){

	row->addTag("type", 	chr["type"].as<String>()); 			// Add tag
	row->addTag("format", 	chr["format"].as<String>()); 	// Add pin tag
	
	String format = chr["format"].as<String>();
	if ( format == "string" ) {

	} else if ( format == "bool" ) {
		row->addField("value", 	chr["value"].as<float>()); // Add value field
	} else if ( format == "float" ) { 
		row->addField("value", 	chr["value"].as<float>()); // Add value field
	} else if ( format == "int" ) { 
		row->addField("value", 	chr["value"].as<float>()); // Add value field
	} else {
		row->addField("value", 	0.0); // Add value field
	}


	/*
	LogD("Updating influxdb ...", false);
	// Create data object with measurment name=analog_read

	dbMeasurement row("analog_read");
	row.addTag("testTag", "Tag1"); // Add tag
	row.addTag("pin", "A0"); // Add pin tag
	row.addField("ADC", analogRead(A0)); // Add value field
	row.addField("random", random(100)); // Add random value

	LogD(_influxdb->write(row) == DB_SUCCESS ? "OK" : "[ERROR] Updating failed", true);

	// Empty field object.
	row.empty();
	*/
}