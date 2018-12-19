//
// HAPClient.hpp
// Homekit
//
//  Created on: 12.08.2017
//      Author: michael
//

#ifndef HAPCLIENT_HPP_
#define HAPCLIENT_HPP_

#include <Arduino.h>
#include <WiFiClient.h>
#include <set>

#include "HAPRequest.hpp"
#include "HAPVerifyContext.hpp"


enum HAPClientState {
	CLIENT_STATE_DISCONNECTED = 0,
	CLIENT_STATE_CONNECTED,
	CLIENT_STATE_AVAILABLE,
	CLIENT_STATE_SENT,
	CLIENT_STATE_RECEIVED,
	CLIENT_STATE_IDLE,
};

enum HAPPairState {
	PAIR_STATE_RESERVED = 0,
	PAIR_STATE_M1,
	PAIR_STATE_M2,
	PAIR_STATE_M3,
	PAIR_STATE_M4,
	PAIR_STATE_M5,
	PAIR_STATE_M6,
};

enum HAPVerifyState {
	VERIFY_STATE_RESERVED = 0,
	VERIFY_STATE_M1,
	VERIFY_STATE_M2,
	VERIFY_STATE_M3,
	VERIFY_STATE_M4,
};

struct HAPSubscribtionItem {
	int aid;
	int iid;

	HAPSubscribtionItem(int aid_, int iid_) : aid(aid_), iid(iid_) {};
	bool operator<(const HAPSubscribtionItem& rhs) const {
		return rhs.aid < this->aid || (rhs.aid == this->aid && rhs.iid < this->iid);
  	};
};

class HAPClient {
public:
	HAPClient();
	~HAPClient();

	//struct HAPKeys {
	//	byte accessorySRPProof[SHA512_DIGEST_LENGTH];
	//} keys;

	HAPRequest		request;
	WiFiClient 		client;
	HAPClientState 	state;
	HAPPairState	pairState;
	HAPVerifyState	verifyState;	

	struct HAPVerifyContext* 		verifyContext;
	struct HAPEncryptionContext* 	encryptionContext;	

	bool			isEncrypted;
	bool			shouldNotify;

	bool operator==(const HAPClient &hap);

	String getPairState() const;
	String getVerifyState() const;
	String getClientState() const;

	void subscribe(int aid, int iid, bool value = true);
	bool isSubscribed(int aid, int iid);

	std::set<HAPSubscribtionItem> subscribtions;
};

#endif /* HAPCLIENT_HPP_ */
