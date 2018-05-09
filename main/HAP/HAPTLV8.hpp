//
// TLV8.hpp
// HAP
//
//  Created on: 18.06.2017
//      Author: michael
//

#ifndef HAPTLV8_HPP_
#define HAPTLV8_HPP_

#include <Arduino.h>


enum HAP_TLV_Type {
	TLV_TYPE_METHOD					= 0x00,
	TLV_TYPE_IDENTIFIER				= 0x01,
	TLV_TYPE_SALT					= 0x02,
	TLV_TYPE_PUBLIC_KEY				= 0x03,
	TLV_TYPE_PROOF					= 0x04,
	TLV_TYPE_ENCRYPTED_DATA			= 0x05,
	TLV_TYPE_STATE					= 0x06,
	TLV_TYPE_ERROR					= 0x07,
	TLV_TYPE_RETRY_DELAY			= 0x08,
	TLV_TYPE_CERTIFICATE			= 0x09,
	TLV_TYPE_SIGNATURE				= 0x0A,
	TLV_TYPE_PERMISSIONS			= 0x0B,
	TLV_TYPE_FRAGMENT_DATA			= 0x0C,
	TLV_TYPE_FRAGMENT_LAST			= 0x0D,

	TLV_TYPE_SEPERATOR				= 0xFF,
};


struct TLV8Entry {
	uint8_t type;
	uint8_t length;
	byte* value;

	uint8_t id;

	TLV8Entry* next;
};


class TLV8 {
public:
	TLV8();
	~TLV8();

	TLV8Entry* searchType(TLV8Entry* ptr, byte type);
	TLV8Entry* searchId(TLV8Entry* ptr, uint8_t id);

	bool encode(byte type, size_t length, const byte data);
	bool encode(byte type, size_t length, const byte* rawData);
	bool encode(byte* rawData, size_t dataLen);

	byte* decode();
	byte* decode(byte type);

	void addNode( TLV8Entry* ptr);
	void insertNode( TLV8Entry* ptr);
	void deleteNode( TLV8Entry* ptr);


	void clear();
	void deleteList( TLV8Entry* ptr);

	void print();
	static void printList( TLV8Entry* ptr);
	static void printNode( TLV8Entry* ptr);

	TLV8Entry* initNode(const byte type, const byte length, const byte* value);
	TLV8Entry* initNode(const byte* rawData);

	TLV8Entry *_head;


	// Size of data values + TYPE + LENGTH
	size_t size();
	size_t size( byte type );
	static size_t size( TLV8Entry *ptr );
	static size_t size( TLV8Entry *ptr, byte type );

//	// Length of data + type + length values
//	size_t length();
//	static size_t length( TLV8Entry *ptr );

	// Number of entries
	uint8_t count();
	static uint8_t count(TLV8Entry* ptr);

	static bool isValidTLVType(byte type);
private:

	TLV8Entry *_tail;
	TLV8Entry *_ptr;
};

#endif /* HAPTLV8_HPP_ */
