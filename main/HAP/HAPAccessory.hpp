//
// HAPAccessory.hpp
// Homekit
//
//  Created on: 22.04.2018
//      Author: michael
//

#ifndef HAPACCESSORY_HPP_
#define HAPACCESSORY_HPP_

#include <Arduino.h>
#include <vector>

#include "HAPService.hpp"
#include "HAPCharacteristics.hpp"

typedef void (*identifyFunction)(bool oldValue, bool newValue);

class HAPAccessory {
public:

    int aid;    
    uint8_t numberOfInstance = 0;
    
    std::vector<HAPService *>_services;

    void addService(HAPService *ser);   
    void addCharacteristics(HAPService *ser, characteristics *cha);
    bool removeService(HAPService *ser);
    bool removeCharacteristics(characteristics *cha);

    HAPAccessory();

    uint8_t numberOfService();
    HAPService *serviceAtIndex(uint8_t index);
    
    characteristics *characteristicsAtIndex(uint8_t index);
    characteristics *characteristicsOfType(uint8_t type);

    String describe();

    static HAPService* addInfoServiceToAccessory(HAPAccessory *acc, String accName, String manufactuerName, String modelName, String serialNumber, identifyFunction identifyCallback, String firmwareRev = "");
    


private:
    
};


#endif /* HAPACCESSORY_HPP_ */