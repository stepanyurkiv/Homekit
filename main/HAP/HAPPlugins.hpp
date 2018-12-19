//
// HAPPlugins.hpp
// Homekit
//
//  Created on: 18.08.2017
//      Author: michael
//

#ifndef HAPPLUGINS_HPP_
#define HAPPLUGINS_HPP_

#include <Arduino.h>
#include <list>
#include <string>
#include <map>
#include <memory>
#include <vector>

#include "HAPAccessory.hpp"
#include "HAPAccessorySet.hpp"
#include "HAPLogger.hpp"
#include "HAPVersion.hpp"

#include "EventManager.h"

//namespace HAPPluginSystem {

enum HAP_PLUGIN_TYPE {
	HAP_PLUGIN_TYPE_OTHER = 0,
	HAP_PLUGIN_TYPE_ACCESSORY,
	HAP_PLUGIN_TYPE_STORAGE
};

/* Base class for plugins */
class HAPPlugin {

public:
	//virtual void doSomething() = 0;
	//virtual HAPPlugin() = 0;
	virtual HAPAccessory* initAccessory() = 0;
	virtual void setValue(String oldValue, String newValue) = 0;
	virtual void setValue(uint8_t type, String oldValue, String newValue) = 0;

	virtual String getValue() = 0;

	virtual void handle(bool forced = false) {};
	virtual void handleEvents(int eventCode, struct HAPEvent eventParam) {};	
	
	// inline void addEvent(int eventCode, int aid, int iid, String value){
	// 	struct HAPEvent event = HAPEvent(nullptr, aid, iid, value);							
	// 	_eventManager->queueEvent( eventCode, event);
	// }

	inline enum HAP_PLUGIN_TYPE type(){
		return _type;
	}

	inline String version(){
		return _version.toString();
	}

	inline String name(){
		return _name;
	}

	inline bool isEnabled(){
		return _isEnabled;
	}

	inline void enable(bool mode){
		_isEnabled = mode;
	}

	inline unsigned long interval(){
		return _interval;
	}	

	inline void setInterval(unsigned long interval){
		_interval = interval;
	}

	inline bool shouldHandle(){

		if (_isEnabled) {
			unsigned long currentMillis = millis(); // grab current time

			if ((unsigned long)(currentMillis - _previousMillis) >= _interval) {

				// save the last time you blinked the LED
				_previousMillis = currentMillis;

				//LogD("Handle plugin: " + String(_name), true);			
				return true;			
			}
		}

		return false;
	}

	inline void addEventListener(EventManager* eventManager){
		_listenerMemberFunctionPlugin.mObj = this;
		_listenerMemberFunctionPlugin.mf = &HAPPlugin::handleEvents;
	
		// Add listener to event manager
		_eventManager = eventManager;
		_eventManager->addListener( EventManager::kEventFromController, &_listenerMemberFunctionPlugin );
	}

	inline void setAccessorySet(HAPAccessorySet* accessorySet){
		_accessorySet = accessorySet;
	}	

protected:
	enum HAP_PLUGIN_TYPE _type;
	String 				_name;
	bool 				_isEnabled;

	unsigned long 		_interval;
	unsigned long 		_previousMillis;

	HAPVersion 			_version;

	EventManager*		_eventManager;
	HAPAccessorySet*	_accessorySet;
	MemberFunctionCallable<HAPPlugin> _listenerMemberFunctionPlugin;
};

	/* 
	 * Base class for PluginRegistrar
	 * See PluginRegistrar below for explanations
	 */
class IPluginRegistrar {
public:
	virtual std::unique_ptr<HAPPlugin> getPlugin() = 0;
};

	/* 
	 * This is the factory, the common interface to "plugins".
	 * Plugins registers themselves here and the factory can serve them on
	 * demand.
	 * It is a Singleton
	 */
class HAPPluginFactory {
public:
		/* Get Singleton instance */
	static HAPPluginFactory& Instance();
		/* Register a new plugin */
	void registerPlugin(IPluginRegistrar* registrar, String name);
		/* Get an instance of a plugin based on its name */
		/* throws out_of_range if plugin not found */
	std::unique_ptr<HAPPlugin> getPlugin(String name);
	std::vector<String> names();

	void loadPlugins();

private:
		/* Holds pointers to plugin registrars */
	std::map<String, IPluginRegistrar*> _registry;
		/* Make constructors private and forbid cloning */
	HAPPluginFactory(): _registry() {};
	HAPPluginFactory(HAPPluginFactory const&) = delete;
	void operator=(HAPPluginFactory const&) = delete;
};

	/* 
	 * Helper class that registers a plugin upon construction.
	 * Actually, the registrar registers itself, and the proxied plugin is only
	 * created on-demand. This mechanism can be shortened by directly 
	 * registering and instance of the plugin, but the assumption here is that
	 * instanciating the plugin can be heavy and not necessary.
	 */
	template<class TPlugin>
class PluginRegistrar: public IPluginRegistrar {
public:
	PluginRegistrar(String classname);
	std::unique_ptr<HAPPlugin> getPlugin();
private:
		/* That is not really used there, but could be useful */
	String _classname;
};

/* template functions in header */
template<class TPlugin>
PluginRegistrar<TPlugin>::PluginRegistrar(String classname): _classname(classname) {
	HAPPluginFactory &factory = HAPPluginFactory::Instance();
	factory.registerPlugin(this, classname);
}

template<class TPlugin>
std::unique_ptr<HAPPlugin>
PluginRegistrar<TPlugin>::getPlugin() {
	std::unique_ptr<HAPPlugin> plugin(new TPlugin());
	return plugin;
}
//}

/*
void HAPPluginFactory::loadPlugins(){
	for(std::map<String, IPluginRegistrar*>::iterator it = _registry.begin(); it != _registry.end(); ++it) {
		Serial.println(it->first);
		auto plugin = getPlugin(it->first);
		plugin->init();
	}
}
*/


#define REGISTER_PLUGIN(CLASSNAME) \
static PluginRegistrar<CLASSNAME> \
CLASSNAME##_registrar( #CLASSNAME ); 


#endif /* HAPPLUGINS_HPP_ */    