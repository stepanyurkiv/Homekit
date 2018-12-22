//
// HAPClient.cpp
// Homekit
//
//  Created on: 12.08.2017
//      Author: michael
//

#include "HAPClient.hpp"
#include "HAPHelper.hpp"

HAPClient::HAPClient()
: state(CLIENT_STATE_DISCONNECTED)
, pairState(PAIR_STATE_RESERVED)
, verifyState(VERIFY_STATE_RESERVED)
, isEncrypted(false)
// , shouldNotify(false)
{

	verifyContext = new struct HAPVerifyContext();
	encryptionContext = new struct HAPEncryptionContext();
}

HAPClient::~HAPClient() {
	
	//delete [] verifyContext;
	//delete [] encryptionContext;
	subscribtions.clear();
}

String HAPClient::getClientState() const {
	switch(state) {
		case CLIENT_STATE_DISCONNECTED:
			return F("CLIENT_STATE_DISCONNECTED");
		case CLIENT_STATE_CONNECTED:
			return F("CLIENT_STATE_CONNECTED");
		case CLIENT_STATE_AVAILABLE:
			return F("CLIENT_STATE_AVAILABLE");
		case CLIENT_STATE_SENT:
			return F("CLIENT_STATE_SENT");
		case CLIENT_STATE_RECEIVED:
			return F("CLIENT_STATE_RECEIVED");
		case CLIENT_STATE_IDLE:
			return F("CLIENT_STATE_IDLE");
		default:
			return F("CLIENT_STATE_DISCONNECTED");
	}
}


String HAPClient::getPairState() const {
	switch(pairState) {
		case PAIR_STATE_RESERVED:
			return F("PAIR_STATE_RESERVED");
		case PAIR_STATE_M1:
			return F("PAIR_STATE_M1");
		case PAIR_STATE_M2:
			return F("PAIR_STATE_M2");
		case PAIR_STATE_M3:
			return F("PAIR_STATE_M3");
		case PAIR_STATE_M4:
			return F("PAIR_STATE_M4");
		case PAIR_STATE_M5:
			return F("PAIR_STATE_M5");
		case PAIR_STATE_M6:
			return F("PAIR_STATE_M6");
		default:
			return F("PAIR_STATE_RESERVED");
	}
}


String HAPClient::getVerifyState() const {
	switch(verifyState) {
		case VERIFY_STATE_RESERVED:
			return F("VERIFY_STATE_RESERVED");
		case VERIFY_STATE_M1:
			return F("VERIFY_STATE_M1");
		case VERIFY_STATE_M2:
			return F("VERIFY_STATE_M2");
		case VERIFY_STATE_M3:
			return F("VERIFY_STATE_M3");
		case VERIFY_STATE_M4:
			return F("VERIFY_STATE_M4");
		default:
			return F("VERIFY_STATE_RESERVED");
	}
}

bool HAPClient::operator==(const HAPClient &hap) const {
	return hap.client.fd() == client.fd();
}

void HAPClient::subscribe(int aid, int iid, bool value){
	struct HAPSubscribtionItem item = HAPSubscribtionItem(aid, iid);
	
	if (value){
		subscribtions.insert(item);
	} else {
		subscribtions.erase(item);
	}
	
}

bool HAPClient::isSubscribed(int aid, int iid) const {
	struct HAPSubscribtionItem item = HAPSubscribtionItem(aid, iid);
	return subscribtions.find(item) != subscribtions.end();
}


#if HAP_API_ADMIN_MODE

String HAPClient::describe() const {
	
	String keys[4];
    String values[4];
    int i=0;
    {
        keys[i] = "isEncrypted";        
        values[i++] = String(isEncrypted);
    }
	{
        keys[i] = "state";        
        values[i++] = String(state);
    }

	{
        keys[i] = "ip";        
        values[i++] = HAPHelper::wrap(client.remoteIP().toString());
    }

	{
        //Form subscribtions list
        int j=0;
		int noOfSubscribtions = subscribtions.size();
        String *subs = new String[noOfSubscribtions];
		for (auto &sub : subscribtions ) {
			subs[j++] = sub.describe();
		}
        keys[i] = "subscribtions";
        values[i++] = HAPHelper::arrayWrap(subs, noOfSubscribtions);
        delete [] subs;
    }
	
    return HAPHelper::dictionaryWrap(keys, values, i);
}


#endif