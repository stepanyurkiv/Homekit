//
// HAPPairSetup.hpp
// Homekit
//
//  Created on: 31.03.2018
//      Author: michael
//

#ifndef HAPPAIRSETUP_HPP_
#define HAPPAIRSETUP_HPP_

#include <Arduino.h>
#include "curve25519.h"

struct HAPPairSetup {
    void* srp;
    //uint8_t sessionKey[CURVE25519_SECRET_LENGTH];
    

    // -> LongTermContext
    struct {
        uint8_t* publicKey;
        uint8_t* privateKey;
    } keys;

    
    // -> EncryptionContext
    //uint8_t encryptKey[CURVE25519_SECRET_LENGTH];
    //uint8_t decryptKey[CURVE25519_SECRET_LENGTH];

};

#endif /* HAPPAIRSETUP_HPP_ */
