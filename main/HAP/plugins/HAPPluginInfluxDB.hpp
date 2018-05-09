//
// HAPPluginInfluxDB.hpp
// Homekit
//
//  Created on: 29.04.2018
//      Author: michael
//

#ifndef HAPPLUGININFLUXDB_HPP_
#define HAPPLUGININFLUXDB_HPP_

#include <Arduino.h>

#include <ESPinfluxdb.h>
#include <ArduinoJson.h>

#include "HAPPlugins.hpp"
#include "HAPLogger.hpp"
#include "HAPAccessory.hpp"


class HAPPluginInfluxDB: public HAPPlugin {
public:

	HAPPluginInfluxDB();

	HAPAccessory* init(){return nullptr;};
	void setValue(String oldValue, String newValue) {};
    void setValue(uint8_t type, String oldValue, String newValue){};

    String getValue() { return ""; };
    String getValue(uint8_t type) { return ""; };

    void handle(HAPAccessorySet* accessorySet, bool forced = false); 

private:
	Influxdb* _influxdb;
	bool _openedDb;

	bool openDB();
	void handleCharacteristic(JsonObject& chr, dbMeasurement* row);
	void handleService(JsonObject& service);
	String getServiceName(JsonArray& chrs);
};

REGISTER_PLUGIN(HAPPluginInfluxDB)

#endif /* HAPPLUGININFLUXDB_HPP_ */ 