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

void HAPPairings::add(uint8_t* id, uint8_t* key){    

	struct HAPPairing item;
	memcpy(item.id, id, HAP_PAIRINGS_ID_SIZE);
    memcpy(item.key, key, HAP_PAIRINGS_LTPK_SIZE);
	
    LogD("### Save pairing:", true);

    LogD("### - ID: ", false);
    HAPHelper::arrayPrint(item.id, HAP_PAIRINGS_ID_SIZE);

    LogD("### - KEY: ", false);
    HAPHelper::arrayPrint(item.key, HAP_PAIRINGS_LTPK_SIZE);

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

        LogD("### - ID: ", false);
        HAPHelper::arrayPrint(item.id, HAP_PAIRINGS_ID_SIZE);

        if ( memcmp(item.id, id, HAP_PAIRINGS_ID_SIZE) == 0) {
        
            LogD("### - KEY found: ", false);
            HAPHelper::arrayPrint(item.key, HAP_PAIRINGS_LTPK_SIZE);
            
            if (outkey != NULL)
                memcpy(outkey, item.key, HAP_PAIRINGS_LTPK_SIZE);
            return 0;
        }

    }
    return -1;
}


uint8_t HAPPairings::size(){
    return _pairings.size();
}
