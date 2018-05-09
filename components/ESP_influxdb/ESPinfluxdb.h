/* Influxdb library

   MIT license
   Written by HW Wong
 */

#ifndef INFLUXDB_H
#define INFLUXDB_H
#include <Arduino.h>

#if defined(ESP8266)
	#include <ESP8266HTTPClient.h>
#elif defined(ESP32)
	#include <HTTPClient.h>
#endif

enum DB_RESPONSE {DB_ERROR, DB_SUCCESS, DB_CONNECT_FAILED};

// Url encode function
String URLEncode(String msg);

class dbMeasurement {
public:
	dbMeasurement(String m);	

	void addField(String key, float value);
	void addTag(String key, String value);
	void empty();
	String postString();

	void setMeasurement(String m);

private:
	String _data;
	String _tag;
	String _measurement;
};

class Influxdb {
public:
	Influxdb(const char* host, uint16_t port);

	DB_RESPONSE opendb(String db);
	DB_RESPONSE opendb(String db, String user, String password);
	DB_RESPONSE write(dbMeasurement data);
	DB_RESPONSE write(String data);
	DB_RESPONSE query(String sql);
	//uint8_t createDatabase(char *dbname);
	DB_RESPONSE response();
	DB_RESPONSE ping();

private:
	String _port;
	String _host;
	String _db;
	HTTPClient _http;

	DB_RESPONSE _response = DB_ERROR;
};

#endif
