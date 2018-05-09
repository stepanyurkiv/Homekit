//
// HAPPairings.hpp
// Homekit
//
//  Created on: 13.04.2018
//      Author: michael
//

#ifndef HAPPAIRINGS_HPP_
#define HAPPAIRINGS_HPP_

#include <Arduino.h>
#include <vector>
#include "ed25519.h"
#include "HAPGlobals.hpp"

#define HAP_PAIRINGS_ID_SIZE 		36
#define HAP_PAIRINGS_LTPK_SIZE 		ED25519_PUBLIC_KEY_LENGTH

#ifndef HAP_PAIRINGS_MAX
#define HAP_PAIRINGS_MAX			8
#endif


struct HAPPairing {
	uint8_t id[HAP_PAIRINGS_ID_SIZE];
	uint8_t key[HAP_PAIRINGS_LTPK_SIZE];
};

class HAPPairings {

public:

	HAPPairings();
	void add(uint8_t* id, uint8_t* key);
	//struct HAPPairing get(uint8_t* id);
	int getKey(const uint8_t* id, uint8_t* outkey);
	uint8_t size();

private:
	std::vector<HAPPairing> _pairings;

};

#endif /* HAPPAIRINGS_HPP_ */