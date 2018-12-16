//
// HAPHelper.cpp
// HomekitAccessoryProtocol
//
//  Created on: 06.08.2017
//      Author: michael
//

#include "HAPHelper.hpp"
#include <Print.h>
//#include <stdlib.h>

//convert hexstring to unsigned char
//returns 0 on success, -1 on error
//data is a buffer of at least len bytes
//hexstring is upper or lower case hexadecimal, NOT prepended with "0x"
byte* HAPHelper::hexToBin(const char* string)
{

	if(string == nullptr)
	{
	   return nullptr;
	}

	size_t slength = strlen(string);

	if(slength % 2 != 0) // must be even
	{
	   return nullptr;
	}
	size_t dlength = slength / 2;

	unsigned char* data = (unsigned char*) malloc(sizeof(unsigned char) * dlength);
	memset(data, 0, dlength);

	size_t index = 0;

	while (index < slength)
	{
		char c = string[index];

		int value = 0;
		if(c >= '0' && c <= '9')
		{
			value = (c - '0');
		}
		else if (c >= 'A' && c <= 'F')
		{
			value = (10 + (c - 'A'));
		}
		else if (c >= 'a' && c <= 'f')
		{
			 value = (10 + (c - 'a'));
		}
		else
		{
			free(data);
			return nullptr;
		}

		data[(index/2)] += value << (((index + 1) % 2) * 4);

		index++;
	}

	return data;
}

void HAPHelper::binToHex(const unsigned char * in, size_t insz, char * out, size_t outsz)
{
	unsigned char * pin = (unsigned char *)in;
	const char * hex = "0123456789ABCDEF";
	char * pout = out;
	for(; pin < in+insz; pout +=2, pin++){
		pout[0] = hex[(*pin>>4) & 0xF];
		pout[1] = hex[ *pin     & 0xF];
		//pout[2] = ':';
		if (pout + 2 - out > outsz){
			/* Better to truncate output string than overflow buffer */
			/* it would be still better to either return a status */
			/* or ensure the target buffer is large enough and it never happen */
			break;
		}
	}
	pout[0] = 0;
}

char* HAPHelper::toHex(const unsigned char* in, size_t insz) {
	char *out;
	out = (char*)malloc(sizeof(char) * (insz * 2) + 1);

	HAPHelper::binToHex(in, insz, out, (insz * 2) + 1);
	return out;
}

// This Chunk of code takes a string and separates it based on a given character 
// and returns The item between the separating character
// String HAPHelper::getValue(String data, char separator, int index)
// {
// 	int found = 0;
// 	int strIndex[] = { 0, -1 };
// 	int maxIndex = data.length() - 1;

// 	for (int i = 0; i <= maxIndex && found <= index; i++) {
// 		if (data.charAt(i) == separator || i == maxIndex) {
// 			found++;
// 			strIndex[0] = strIndex[1] + 1;
// 			strIndex[1] = (i == maxIndex) ? i+1 : i;
// 		}
// 	}
// 	return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
// }

uint8_t HAPHelper::numDigits(const size_t n) {
	if (n < 10) return 1;
	return 1 + numDigits(n / 10);
}


void HAPHelper::arrayPrint(uint8_t* a, int len)
{
	for (int i=0; i<len; i++) {
		if (i != 0 && (i % 0x10) == 0) {
			printf("\n");
		}
		printf("%02X ", a[i]);
	}
	printf("\n");
}

void HAPHelper::prependZeros(char *dest, const char *src, uint8_t width) {
	size_t len = strlen(src);
	size_t zeros = (len > width) ? 0 : width - len;
	memset(dest, '0', zeros);
	strcpy(dest + zeros, src);
}



String HAPHelper::wrap(const char *str) { 
	return String("\"" + String(str) + "\""); 
}

String HAPHelper::arrayWrap(String *s, unsigned short len) {
	String result;
	
	result += "[";
	
	for (int i = 0; i < len; i++) {
		result += s[i]+",";
	}
	result = result.substring(0, result.length()-1);
	
	result += "]";
	
	return result;
}


String HAPHelper::dictionaryWrap(String *key, String *value, unsigned short len) {
	String result;
	
	result += "{";
	
	for (int i = 0; i < len; i++) {
		result += wrap(key[i].c_str())+":"+value[i]+",";
	}
	result = result.substring(0, result.length()-1);
	
	result += "}";
	
	return result;
}


bool HAPHelper::containsNestedKey(const JsonObject& obj, const char* key) {
	for (const JsonPair& pair : obj) {
		if (!strcmp(pair.key, key))
			return true;

		if (containsNestedKey(pair.value.as<JsonObject>(), key)) 
			return true;
	}

	return false;
}


String HAPHelper::printUnescaped(String str) {
	String result;
	result = str;
	result.replace('\n', '\\ n\n');
	result.replace('\r', '\\ r');
	return result;
}