//
// HAPService.hpp
// Homekit
//
//  Created on: 22.04.2018
//      Author: michael
//

#ifndef HAPSERVICE_HPP_
#define HAPSERVICE_HPP_

#include <Arduino.h>
#include <vector>

#include "HAPCharacteristics.hpp"

class HAPService {
public:

    HAPService();

    String describe();

    int serviceID;
    int uuid;
    std::vector<characteristics *> _characteristics;

    HAPService(int _uuid);

    virtual uint8_t numberOfCharacteristics() { return _characteristics.size(); }
    virtual characteristics *characteristicsAtIndex(uint8_t index) { return _characteristics[index]; }
    
};


#endif /* HAPSERVICE_HPP_ */