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
#include <ArduinoJson.h>


// #if defined(__GNUC__) || defined(__clang__)
// 	#define DEPRECATED __attribute__((deprecated))
// #elif defined(_MSC_VER)
// 	#define DEPRECATED __declspec(deprecated)
// #else
// 	#pragma message("WARNING: You need to implement DEPRECATED for this compiler")
// 	#define DEPRECATED
// #endif


class HAPHelper {
public:
	HAPHelper();
	~HAPHelper();

	// static union {
	// 	uint32_t bit32;
	// 	uint8_t bit8[4];
	// } HAPBit32to8Converter;

	// DEPRECATED
	 static String getValue(String data, char separator, int index);	
	 static byte* hexToBin(const char* string);
	 static void binToHex(const unsigned char * in, size_t insz, char * out, size_t outsz);
	 static char* toHex(const unsigned char * in, size_t insz);
	 static void prependZeros(char *dest, const char *src, uint8_t width); // -> moved to encryption
	


	static uint8_t numDigits(const size_t n);
	static void arrayPrint(uint8_t* a, int len);
	
	static String wrap(const char *str);
	static String arrayWrap(String *s, unsigned short len);
	static String dictionaryWrap(String *key, String *value, unsigned short len);



	static bool containsNestedKey(const JsonObject& obj, const char* key);
};

#endif /* HAPHELPER_HPP_ */
