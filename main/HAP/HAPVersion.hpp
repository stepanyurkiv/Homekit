//
// HAPVersion.hpp
// Homekit
//
//  Created on: 20.08.2017
//      Author: michael
//

#ifndef HAPVERSION_HPP_
#define HAPVERSION_HPP_

#include <Arduino.h>
#include "HAPBuildnumber.hpp"

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

/* Build automated generated version number */
#define HOMEKIT_VERSION \
	STR(HOMEKIT_VERSION_MAJOR) \
"." STR(HOMEKIT_VERSION_MINOR) \
"." STR(HOMEKIT_VERSION_REVISION) \
"." STR(HOMEKIT_VERSION_BUILD) \

#define HOMEKIT_VERSION_MAJOR 0
#define HOMEKIT_VERSION_MINOR 1
#define HOMEKIT_VERSION_REVISION 0


struct HAPVersion {

	int major, minor, revision, build;

	HAPVersion(){
		major 		= 0;
		minor 		= 0;
		revision 	= 0;
		build 		= 0;
	}

	HAPVersion(const char* version)
	{
		sscanf(version, "%d.%d.%d.%d", &major, &minor, &revision, &build);
		if (major < 0) major = 0;
		if (minor < 0) minor = 0;
		if (revision < 0) revision = 0;
		if (build < 0) build = 0;
	}

	bool operator < (const HAPVersion& other)
	{
		if (major < other.major)
			return true;
		if (minor < other.minor)
			return true;
		if (revision < other.revision)
			return true;
		if (build < other.build)
			return true;
		return false;
	}

	bool operator == (const HAPVersion& other)
	{
		return major == other.major
			&& minor == other.minor
			&& revision == other.revision
			&& build == other.build;
	}

	void operator = (const HAPVersion& other)
	{
		major = other.major;
		minor = other.minor;
		revision = other.revision;
		build = other.build;
	}

	String toString(){
		char str[32];
		sprintf(str, "%d.%d.%d.%d", major, minor, revision, build);
		return str;
	}
};

#endif /* HAPVERSION_HPP_ */
