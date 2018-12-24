
#include "HAPAccessory.hpp"
#include "HAPHelper.hpp"

HAPAccessory::HAPAccessory() {

}

void HAPAccessory::addService(HAPService *ser) {
	ser->serviceID = ++numberOfInstance;
	_services.push_back(ser);
}

void HAPAccessory::addCharacteristics(HAPService *ser, characteristics *cha) {
	cha->iid = ++numberOfInstance;
	ser->_characteristics.push_back(cha);
}

bool HAPAccessory::removeService(HAPService *ser) {
	bool exist = false;
	for (std::vector<HAPService *>::iterator it = _services.begin(); it != _services.end(); it++) {
		if (*it == ser) {
			_services.erase(it);
			exist = true;
		}
	}
	return exist;
}

bool HAPAccessory::removeCharacteristics(characteristics *cha) {
	bool exist = false;
	for (std::vector<HAPService *>::iterator it = _services.begin(); it != _services.end(); it++) {
		for (std::vector<characteristics *>::iterator jt = (*it)->_characteristics.begin(); jt != (*it)->_characteristics.end(); jt++) {
			if (*jt == cha) {
				(*it)->_characteristics.erase(jt);
				exist = true;
			}
		}
	}
	return exist;
}



uint8_t HAPAccessory::numberOfService() const { 
	return _services.size(); 
}


HAPService *HAPAccessory::serviceAtIndex(uint8_t index) {
	for (std::vector<HAPService *>::iterator it = _services.begin(); it != _services.end(); it++) {
		if ((*it)->serviceID == index) {
			return *it;
		}
	}
	return NULL;
}

characteristics *HAPAccessory::characteristicsAtIndex(uint8_t index) {
	for (std::vector<HAPService *>::iterator it = _services.begin(); it != _services.end(); it++) {
		for (std::vector<characteristics *>::iterator jt = (*it)->_characteristics.begin(); jt != (*it)->_characteristics.end(); jt++) {
			if ((*jt)->iid == index) {
				return *jt;
			}
		}
	}
	return NULL;
}

characteristics *HAPAccessory::characteristicsOfType(uint8_t type) {
	for (std::vector<HAPService *>::iterator it = _services.begin(); it != _services.end(); it++) {
		for (std::vector<characteristics *>::iterator jt = (*it)->_characteristics.begin(); jt != (*it)->_characteristics.end(); jt++) {
			if ((*jt)->type == type) {
				return *jt;
			}
		}
	}
	return NULL;
}


String HAPAccessory::describe() const {

    String keys[2];
    String values[2];
    
    {
        keys[0] = "aid";
        char temp[8];
        sprintf(temp, "%d", aid);
        values[0] = temp;
    }
    
    {
        //Form services list
        int noOfService = numberOfService();
        String *services = new String[noOfService];
        for (int i = 0; i < noOfService; i++) {
            services[i] = _services[i]->describe();
        }
        keys[1] = "services";
        values[1] = HAPHelper::arrayWrap(services, noOfService);
        delete [] services;
    }
    
    
    return HAPHelper::dictionaryWrap(keys, values, 2);
}


// HAPService* HAPAccessory::addInfoServiceToAccessory(HAPAccessory *acc, String accName, String manufactuerName, String modelName, String serialNumber, identifyFunction identifyCallback, String firmwareRev) {
//     HAPService *infoService = new HAPService(serviceType_accessoryInfo);
//     acc->addService(infoService);
    
//     stringCharacteristics *accNameCha = new stringCharacteristics(charType_serviceName, permission_read, 32);
//     accNameCha->setValue(accName);
//     acc->addCharacteristics(infoService, accNameCha);
    
//     stringCharacteristics *manNameCha = new stringCharacteristics(charType_manufactuer, permission_read, 32);
//     manNameCha->setValue(manufactuerName);
//     acc->addCharacteristics(infoService, manNameCha);
    
//     stringCharacteristics *modelNameCha = new stringCharacteristics(charType_modelName, permission_read, 32);
//     modelNameCha->setValue(modelName);
//     acc->addCharacteristics(infoService, modelNameCha);
    
//     stringCharacteristics *serialNameCha = new stringCharacteristics(charType_serialNumber, permission_read, 32);
//     serialNameCha->setValue(serialNumber);
//     acc->addCharacteristics(infoService, serialNameCha);
    
//     boolCharacteristics *identify = new boolCharacteristics(charType_identify, permission_write);
//     identify->setValue("false");
//     identify->valueChangeFunctionCall = identifyCallback;
//     acc->addCharacteristics(infoService, identify);


//     if ( firmwareRev != "" ) {
//         stringCharacteristics *firmwareRevCha = new stringCharacteristics(charType_firmwareRevision, permission_read|permission_notify, 32);
//         firmwareRevCha->setValue(firmwareRev);
//         acc->addCharacteristics(infoService, firmwareRevCha);    
//     }
    


//     return infoService;
// }

HAPService* HAPAccessory::addInfoServiceToAccessory(HAPAccessory *acc, String accName, String manufactuerName, String modelName, String serialNumber, identifyFunctionCallback callback, String firmwareRev) {
    HAPService *infoService = new HAPService(serviceType_accessoryInfo);
    acc->addService(infoService);
    
    stringCharacteristics *accNameCha = new stringCharacteristics(charType_serviceName, permission_read, 32);
    accNameCha->setValue(accName);
    acc->addCharacteristics(infoService, accNameCha);
    
    stringCharacteristics *manNameCha = new stringCharacteristics(charType_manufactuer, permission_read, 32);
    manNameCha->setValue(manufactuerName);
    acc->addCharacteristics(infoService, manNameCha);
    
    stringCharacteristics *modelNameCha = new stringCharacteristics(charType_modelName, permission_read, 32);
    modelNameCha->setValue(modelName);
    acc->addCharacteristics(infoService, modelNameCha);
    
    stringCharacteristics *serialNameCha = new stringCharacteristics(charType_serialNumber, permission_read, 32);
    serialNameCha->setValue(serialNumber);
    acc->addCharacteristics(infoService, serialNameCha);
    
    boolCharacteristics *identify = new boolCharacteristics(charType_identify, permission_write);
    identify->setValue("false");
    identify->valueChangeFunctionCall = callback;
    acc->addCharacteristics(infoService, identify);


    if ( firmwareRev != "" ) {
        stringCharacteristics *firmwareRevCha = new stringCharacteristics(charType_firmwareRevision, permission_read|permission_notify, 32);
        firmwareRevCha->setValue(firmwareRev);
        acc->addCharacteristics(infoService, firmwareRevCha);    
    }
    


    return infoService;
}