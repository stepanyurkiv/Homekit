//
// HAPAccessory.cpp
// Homekit
//
//  Created on: 18.08.2017
//      Author: michael
//

#include "HAPAccessorySet.hpp"
#include "HAPHelper.hpp"
#include "HAPLogger.hpp"
#include "HAPGlobals.hpp"
#include "HAPServer.hpp"

#include <WString.h>

#include <mbedtls/sha512.h>
#include <mbedtls/base64.h>
#include <mbedtls/bignum.h>

#include "base36.h"




HAPAccessorySet::HAPAccessorySet() 
: isPaired(false)
, _configurationNumber(1) 
, _accessoryType(HAP_ACCESSORY_TYPE_OTHER)
, _modelName("")
, _pinCode("000-00-000")
{
}


HAPAccessorySet::~HAPAccessorySet(){

}

void HAPAccessorySet::begin(){
	addAccessoryInfo();
}

void HAPAccessorySet::addAccessoryInfo(){
	HAPAccessory *accessory = new HAPAccessory();
	HAPService* infoService = HAPAccessory::addInfoServiceToAccessory(accessory, HAP_HOSTNAME, HAP_MANUFACTURER, HAP_MODELL_NAME, "44-22-777", NULL, hap.versionString());
	
	
	// stringCharacteristics *fwCha = new stringCharacteristics(charType_firmwareRevision, permission_read, 32);
	// fwCha->setValue(hap.versionString());
	// accessory->addCharacteristics(infoService, fwCha);
	
	addAccessory(accessory);
}



uint32_t HAPAccessorySet::configurationNumber(){
	return _configurationNumber;
}

void HAPAccessorySet::setConfigurationNumber(uint32_t number){
	_configurationNumber = number;
}

uint8_t HAPAccessorySet::accessoryType(){
	return _accessoryType;
}

void HAPAccessorySet::setAccessoryType(enum HAP_ACCESSORY_TYPE accessoryType){
	_accessoryType = accessoryType;
}

const char* HAPAccessorySet::setupID(){
	return _setupID.c_str();
}

void HAPAccessorySet::generateSetupID(){


	char setupID[4];

#if HAP_DEBUG	
	strcpy(setupID, "UPFT");
#else	  	
	const char* letters = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

	randomSeed(analogRead(0));

	int i = 0;
  	while(i < 4) {
    	setupID[i] = ( randomChar((char*)letters) );
    	i++;
  	}
  	setupID[4] = '\0';

#endif
  	_setupID = String(setupID);
  	computeSetupHash();

  	generateXMI();
}

char HAPAccessorySet::randomChar(char* letters) {
   return letters[random(0, strlen(letters)-1)];
}

void HAPAccessorySet::setModelName(String name){
	_modelName = name;
}

const char* HAPAccessorySet::modelName(){
	return _modelName.c_str();
}

const char* HAPAccessorySet::setupHash(){
	return _setupHash.c_str();
}

void HAPAccessorySet::computeSetupHash(){
	int len = 4 + _modelName.length();
	uint8_t setupHashMaterial[len];

	memcpy(setupHashMaterial, _setupID.c_str(), 4);
	memcpy(setupHashMaterial + 4, _modelName.c_str(), _modelName.length());

	uint8_t output[64];
	mbedtls_sha512(setupHashMaterial, len, output, 0);

	uint8_t sliced[4];
	memcpy(sliced, output, 4);

	// Doesn't work :(
	//_setupHash = base64::encode(sliced);	

	char setupHash[9];

	size_t olen;		
	if (mbedtls_base64_encode((uint8_t*)setupHash, 9, &olen, sliced, 4)) {
		LogE("[computeSetupHash] ERROR: mbedtls_base64_encode failed!", true);
	}

	setupHash[9] = '\0';
	_setupHash = String(setupHash);
}



void HAPAccessorySet::setPinCode(const char* pinCode){
	_pinCode = String(pinCode);
}

const char* HAPAccessorySet::pinCode(){
	return _pinCode.c_str();
}


void HAPAccessorySet::generateXMI(){
	
	String tmp = _pinCode;	
	tmp.replace("-", "");	

	int lowValue = atoi(tmp.c_str());

	lowValue |= 1 << 28;

	mbedtls_mpi bignumLow;
	mbedtls_mpi bignumHigh;

	mbedtls_mpi_init(&bignumLow);
	mbedtls_mpi_init(&bignumHigh);


	int error_code = mbedtls_mpi_lset(&bignumLow, lowValue);
	if (error_code != 0){
		Serial.println("ERROR 1!!!");
	}

	
	size_t olenLow = 0;
	mbedtls_mpi_write_string(&bignumLow, 16, NULL, 0, &olenLow);
	
	char dstLow[olenLow];
	int dstLowLen = olenLow;
	error_code = mbedtls_mpi_write_string(&bignumLow, 16, dstLow, dstLowLen, &olenLow);
	if (error_code != 0){
		Serial.println("ERROR 2!!!");
	}


	int valueHigh = _accessoryType >> 1;

	char dst2[olenLow + 1];
	sprintf(dst2, "%d%s", valueHigh, dstLow);	

	error_code = mbedtls_mpi_read_string(&bignumHigh, 16, dst2);
	if (error_code != 0){
		Serial.println("ERROR 3!!!");
	}


	size_t olen2 = 0;
	mbedtls_mpi_write_string(&bignumHigh, 16, NULL, 0, &olen2);
	
	char dst3[olen2];
	int dstlen3 = olen2;
	error_code = mbedtls_mpi_write_string(&bignumHigh, 16, dst3, dstlen3, &olen2);
	if (error_code != 0){
		Serial.println("ERROR 4!!!");
	}

	char dest[9];
	str16_to_str36(dest, dst3);
	
	char finalDest[9];	
	HAPHelper::prependZeros(finalDest, dest, 9);
	
	char xhm[20];

	memcpy(xhm, "X-HM://", 7);
	memcpy(xhm + 7, finalDest, 9);
	memcpy(xhm + 7 + 9, _setupID.c_str(), 4);
	xhm[20] = '\0';

	_xhm = String(xhm);

	mbedtls_mpi_free(&bignumLow);
	mbedtls_mpi_free(&bignumHigh);
}

const char* HAPAccessorySet::xhm(){
	return _xhm.c_str();
}


uint8_t HAPAccessorySet::numberOfAccessory() {
	return _accessories.size();
}

HAPAccessory* HAPAccessorySet::accessoryWithAID(uint8_t aid) {
	for (std::vector<HAPAccessory *>::iterator it = _accessories.begin(); it != _accessories.end(); it++) {
		if ((*it)->aid == aid) {
			return *it;
		}
	}
	return nullptr;
}

HAPAccessory* HAPAccessorySet::accessoryAtIndex(uint8_t index){
	uint8_t count = 0;
	for (std::vector<HAPAccessory *>::iterator it = _accessories.begin(); it != _accessories.end(); it++) {
		if (count == index) {
			return *it;
		}
		count++;
	}
	return nullptr;
}


void HAPAccessorySet::addAccessory(HAPAccessory *acc) {
	acc->aid = ++_aid;
	_accessories.push_back(acc);
	_configurationNumber++;
}


bool HAPAccessorySet::removeAccessory(HAPAccessory *acc) {
	bool exist = false;
	for (std::vector<HAPAccessory *>::iterator it = _accessories.begin(); it != _accessories.end(); it++) {
		if (*it == acc) {
			_accessories.erase(it);
			_configurationNumber--;
			exist = true;
		}
	}
	return exist;
}


String HAPAccessorySet::describe() {
    int numberOfAcc = numberOfAccessory();
    String *desc = new String[numberOfAcc];

    for (int i = 0; i < numberOfAcc; i++) {
        desc[i] = _accessories[i]->describe();
    }

    
    String result = HAPHelper::arrayWrap(desc, numberOfAcc);
    delete [] desc;
    String key = "accessories";
    result = HAPHelper::dictionaryWrap(&key, &result, 1);
    
    return result;
}