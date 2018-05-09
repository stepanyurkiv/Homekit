//
// HAPHelper.hpp
// HomekitAccessoryProtocol
//
//  Created on: 06.08.2017
//      Author: michael
//

#ifndef HAPHELPER_HPP_
#define HAPHELPER_HPP_

#include <Arduino.h>

class HAPHelper {
public:
	HAPHelper();
	~HAPHelper();

	static union {
		uint32_t bit32;
		uint8_t bit8[4];
	} HAPBit32to8Converter;

	static String getValue(String data, char separator, int index);
	
	static byte* hexToBin(const char* string);
	static void binToHex(const unsigned char * in, size_t insz, char * out, size_t outsz);
	static char* toHex(const unsigned char * in, size_t insz);

	static uint8_t numDigits(const size_t n);
	static void arrayPrint(uint8_t* a, int len);
	static void prependZeros(char *dest, const char *src, uint8_t width);


	static String wrap(const char *str);
	static String arrayWrap(String *s, unsigned short len);

	static String dictionaryWrap(String *key, String *value, unsigned short len);
};

#endif /* HAPHELPER_HPP_ */
