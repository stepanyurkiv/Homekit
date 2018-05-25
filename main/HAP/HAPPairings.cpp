//
// HAPPairings.cpp
// Homekit
//
//  Created on: 13.04.2018
//      Author: michael
//

#include "HAPPairings.hpp"
#include <algorithm>
#include "HAPHelper.hpp"
#include "HAPLogger.hpp"

HAPPairings::HAPPairings(){

}


bool HAPPairings::begin(){
	if (!EEPROM.begin(HAP_EEPROM_SIZE)) {
		LogE("[ERROR] Failed to initialise EEPROM", true); 
		return false;
	}    
	return true;
}


void HAPPairings::save(){
	for (int i=0; i < _pairings.size(); i++){
		EEPROM.writeBytes( HAP_EEPROM_OFFSET_PAIRINGS + ( i * sizeof(HAPPairing) ), &_pairings[i], sizeof(HAPPairing));
	}
	EEPROM.commit();
}

void HAPPairings::load(){
	
	for (int i=0; i < HAP_PAIRINGS_MAX; i++){
		HAPPairing tmp;

		// uint8_t id[HAP_PAIRINGS_ID_SIZE];
		// uint8_t key[HAP_PAIRINGS_LTPK_SIZE];

		// Serial.printf("i: %d . i * sizeof(HAPPairing): %d\n", i, i * sizeof(HAPPairing));
		// Serial.printf("i: %d . (i * sizeof(HAPPairing)) + HAP_PAIRINGS_ID_SIZE: %d\n", i, (i * sizeof(HAPPairing)) + HAP_PAIRINGS_ID_SIZE);

		// EEPROM.readBytes( i * sizeof(HAPPairing), id, HAP_PAIRINGS_ID_SIZE);
		// EEPROM.readBytes( (i * sizeof(HAPPairing)) + HAP_PAIRINGS_ID_SIZE, key, HAP_PAIRINGS_LTPK_SIZE);

		EEPROM.readBytes( HAP_EEPROM_OFFSET_PAIRINGS + ( i * sizeof(HAPPairing) ), &tmp, sizeof(HAPPairing));

		// if (id[0] != '0' && id[1] != '0'){
		// 	Serial.println("BREAK!!!!!!!!!!!!!!!!!!!!");
		// 	break;
		// }
		if (tmp.id[0] == 0xFF && tmp.id[1] == 0xFF && tmp.key[0] == 0xFF &&tmp.key[1] == 0xFF ){
			return;
		} 
		_pairings.push_back(tmp);
	}	
}


void HAPPairings::resetEEPROM(){
	for (int i=0 ; i < HAP_EEPROM_SIZE; i++){
		EEPROM.write(i, 0xFF);
	}
	EEPROM.commit();
}



void HAPPairings::add(uint8_t* id, uint8_t* key){    

	struct HAPPairing item;
	memcpy(item.id, id, HAP_PAIRINGS_ID_LENGTH);
	memcpy(item.key, key, HAP_PAIRINGS_LTPK_LENGTH);
	
	// LogD("### Save pairing:", true);

	// LogD("### - ID: ", false);
	// HAPHelper::arrayPrint(item.id, HAP_PAIRINGS_ID_SIZE);

	// LogD("### - KEY: ", false);
	// HAPHelper::arrayPrint(item.key, HAP_PAIRINGS_LTPK_SIZE);

	_pairings.push_back(item);
}

/*
struct HAPPairing HAPPairings::get(uint8_t* id) {
	for(size_t i = 0; i < _pairings.size(); i++)
	{
		if (memcmp(_pairings[i].id, id, HAP_PAIRINGS_ID_SIZE) ) {
			return _pairings[i];
		}
	}
	return struct HAPPairing;
}
*/

int HAPPairings::getKey(const uint8_t* id, uint8_t* outkey) {
	LogD("Get iOS DeviceID LTPK: ", true);
	for(size_t i = 0; i < _pairings.size(); i++) {
		struct HAPPairing item = _pairings[i];

		// LogD("### - ID: ", false);
		// HAPHelper::arrayPrint(item.id, HAP_PAIRINGS_ID_SIZE);

		if ( memcmp(item.id, id, HAP_PAIRINGS_ID_LENGTH) == 0) {
		
			// LogD("### - KEY found: ", false);
			// HAPHelper::arrayPrint(item.key, HAP_PAIRINGS_LTPK_SIZE);
			
			if (outkey != NULL)
				memcpy(outkey, item.key, HAP_PAIRINGS_LTPK_LENGTH);
			return 0;
		}

	}
	return -1;
}


uint8_t HAPPairings::size(){
	return _pairings.size();
}

void HAPPairings::print(){
	for (int i=0; i < _pairings.size(); i++){
		Serial.println("id: ");
		HAPHelper::arrayPrint(_pairings[i].id, 36);

		Serial.println("key: ");
		HAPHelper::arrayPrint(_pairings[i].key, 32);
	}
}


void HAPPairings::loadLTPK(uint8_t *ltpk){
	EEPROM.readBytes( 0 , &ltpk, HAP_PAIRINGS_LTPK_LENGTH);
}

void HAPPairings::loadLTSK(uint8_t *ltsk){
	EEPROM.readBytes( HAP_PAIRINGS_LTPK_LENGTH, &ltsk, HAP_PAIRINGS_LTSK_LENGTH);
}


void HAPPairings::saveLTPK(uint8_t *ltpk){
	EEPROM.writeBytes( 0 , &ltpk, HAP_PAIRINGS_LTPK_LENGTH);
}

void HAPPairings::saveLTSK(uint8_t *ltsk){
	EEPROM.writeBytes( HAP_PAIRINGS_LTPK_LENGTH, &ltsk, HAP_PAIRINGS_LTSK_LENGTH);
}

