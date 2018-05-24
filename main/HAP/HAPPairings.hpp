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
#include <EEPROM.h>


#define HAP_PAIRINGS_ID_SIZE 		36
#define HAP_PAIRINGS_LTPK_SIZE 		ED25519_PUBLIC_KEY_LENGTH

#ifndef HAP_PAIRINGS_MAX
#define HAP_PAIRINGS_MAX			16
#endif

#define HAP_EEPROM_SIZE				4096
// #define HAP_EEPROM_PARTITION		"eeprom"

//
// HAPPairing 
//	id 	=  36
//	key =  32
//	=========
//         68
// 

// 
//  EEPROM
//    size = 4096 
//  -----------------------------
// | id  |  key	| id  |  key  | ...
//  -----------------------------
//   36     32     36     32 
//		 68			   68
// 
//  max 60 key pairs available 
//
//	for 16 pairings -> 68 * 16 = 1088
// 
struct HAPPairing {
	uint8_t id[HAP_PAIRINGS_ID_SIZE];
	uint8_t key[HAP_PAIRINGS_LTPK_SIZE];
};

class HAPPairings {

public:

	HAPPairings();

	bool begin();

	void load();
	void save();
	void resetEEPROM();


	void loadLTPK(uint8_t *ltpk);
	void loadLTSK(uint8_t *ltsk);

	void saveLTPK(uint8_t *ltpk);
	void saveLTSK(uint8_t *ltsk);



	void print();

	void add(uint8_t* id, uint8_t* key);
	//struct HAPPairing get(uint8_t* id);
	int getKey(const uint8_t* id, uint8_t* outkey);
	uint8_t size();

private:
	std::vector<HAPPairing> _pairings;	

};

#endif /* HAPPAIRINGS_HPP_ */