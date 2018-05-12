

#include "HAPCharacteristics.hpp"
#include "HAPHelper.hpp"

//Wrap to JSON
inline String attribute(unsigned short type, unsigned short acclaim, int p, bool value) {
    String result;
    if (p & permission_read) {
        result += HAPHelper::wrap("value")+":";
        if (value) result += true;
        else result += false;
        result += ",";
    }
    
    result += HAPHelper::wrap("perms")+":";
    result += "[";
    if (p & permission_read) result += HAPHelper::wrap("pr")+",";
    if (p & permission_write) result += HAPHelper::wrap("pw")+",";
    if (p & permission_notify) result += HAPHelper::wrap("ev")+",";
    result = result.substring(0, result.length()-1);
    result += "]";
    result += ",";
    
    char tempStr[4];

#if HAP_LONG_UUID    
    char uuidStr[36];
    snprintf(uuidStr, 36, HAP_UUID, type);
#else    
    char uuidStr[8];
    snprintf(uuidStr, 8, "\"%X\"", type);    
#endif

    result += HAPHelper::wrap("type")+":"+uuidStr;
    result += ",";
    
    snprintf(tempStr, 4, "%hd", acclaim);
    result += HAPHelper::wrap("iid")+":"+tempStr;
    result += ",";
    
    result += "\"format\":\"bool\"";
    
    return "{"+result+"}";
}



inline String attribute(unsigned short type, unsigned short acclaim, int p, int value, int minVal, int maxVal, int step, unit valueUnit) {
    String result;
    char tempStr[4];
    
    snprintf(tempStr, 4, "%d", value);
    
    if (p & permission_read) {
        result += HAPHelper::wrap("value")+":"+tempStr;
        result += ",";
    }
    
    snprintf(tempStr, 4, "%d", minVal);
    if (minVal != INT32_MIN)
        result += HAPHelper::wrap("minValue")+":"+tempStr+",";
    
    snprintf(tempStr, 4, "%d", maxVal);
    if (maxVal != INT32_MAX)
        result += HAPHelper::wrap("maxValue")+":"+tempStr+",";
    
    snprintf(tempStr, 4, "%d", step);
    if (step > 0)
        result += HAPHelper::wrap("minStep")+":"+tempStr+",";
    
    result += HAPHelper::wrap("perms")+":";
    result += "[";
    if (p & permission_read) result += HAPHelper::wrap("pr")+",";
    if (p & permission_write) result += HAPHelper::wrap("pw")+",";
    if (p & permission_notify) result += HAPHelper::wrap("ev")+",";
    result = result.substring(0, result.length()-1);
    result += "]";
    result += ",";

#if HAP_LONG_UUID    
    char uuidStr[36];
    snprintf(uuidStr, 36, HAP_UUID, type);
#else    
    char uuidStr[8];
    snprintf(uuidStr, 8, "\"%X\"", type);    
#endif

    result += HAPHelper::wrap("type")+":"+uuidStr;
    result += ",";
    
    snprintf(tempStr, 4, "%hd", acclaim);
    result += HAPHelper::wrap("iid")+":"+tempStr;
    result += ",";
    
    switch (valueUnit) {
        case unit_arcDegree:
            result += HAPHelper::wrap("unit")+":"+HAPHelper::wrap("arcdegrees")+",";
            break;
        case unit_celsius:
            result += HAPHelper::wrap("unit")+":"+HAPHelper::wrap("celsius")+",";
            break;
        case unit_percentage:
            result += HAPHelper::wrap("unit")+":"+HAPHelper::wrap("percentage")+",";
            break;
        case unit_none:
        	break;    
    }
    
    result += "\"format\":\"int\"";
    
    return "{"+result+"}";
}


inline String attribute(unsigned short type, unsigned short acclaim, int p, float value, float minVal, float maxVal, float step, unit valueUnit) {
    String result;
    char tempStr[4];
    
    snprintf(tempStr, 4, "%f", value);
    
    if (p & permission_read) {
        result += HAPHelper::wrap("value")+":"+tempStr;
        result += ",";
    }
    
    snprintf(tempStr, 4, "%f", minVal);
    if (minVal != INT32_MIN)
        result += HAPHelper::wrap("minValue")+":"+tempStr+",";
    
    snprintf(tempStr, 4, "%f", maxVal);
    if (maxVal != INT32_MAX)
        result += HAPHelper::wrap("maxValue")+":"+tempStr+",";
    
    snprintf(tempStr, 4, "%f", step);
    if (step > 0)
        result += HAPHelper::wrap("minStep")+":"+tempStr+",";
    
    result += HAPHelper::wrap("perms")+":";
    result += "[";
    if (p & permission_read) result += HAPHelper::wrap("pr")+",";
    if (p & permission_write) result += HAPHelper::wrap("pw")+",";
    if (p & permission_notify) result += HAPHelper::wrap("ev")+",";
    result = result.substring(0, result.length()-1);
    result += "]";
    result += ",";
    

#if HAP_LONG_UUID    
    char uuidStr[36];
    snprintf(uuidStr, 36, HAP_UUID, type);
#else    
    char uuidStr[8];
    snprintf(uuidStr, 8, "\"%X\"", type);    
#endif

    result += HAPHelper::wrap("type")+":"+uuidStr;
    result += ",";
    
    snprintf(tempStr, 4, "%hd", acclaim);
    result += HAPHelper::wrap("iid")+":"+tempStr;
    result += ",";
    
    switch (valueUnit) {
        case unit_arcDegree:
            result += HAPHelper::wrap("unit")+":"+HAPHelper::wrap("arcdegrees")+",";
            break;
        case unit_celsius:
            result += HAPHelper::wrap("unit")+":"+HAPHelper::wrap("celsius")+",";
            break;
        case unit_percentage:
            result += HAPHelper::wrap("unit")+":"+HAPHelper::wrap("percentage")+",";
            break;
        case unit_none:
        	break;
    }
    
    result += "\"format\":\"float\"";
    
    return "{"+result+"}";
}



inline String attribute(unsigned short type, unsigned short acclaim, int p, String value, unsigned short len) {
    String result;
    char tempStr[4];
    
    if (p & permission_read) {
        result += HAPHelper::wrap("value")+":"+HAPHelper::wrap(value.c_str());
        result += ",";
    }
    
    result += HAPHelper::wrap("perms")+":";
    result += "[";
    if (p & permission_read) result += HAPHelper::wrap("pr")+",";
    if (p & permission_write) result += HAPHelper::wrap("pw")+",";
    if (p & permission_notify) result += HAPHelper::wrap("ev")+",";
    result = result.substring(0, result.length()-1);
    result += "]";
    result += ",";
    

#if HAP_LONG_UUID    
    char uuidStr[36];
    snprintf(uuidStr, 36, HAP_UUID, type);
#else    
    char uuidStr[8];
    snprintf(uuidStr, 8, "\"%X\"", type);    
#endif
    result += HAPHelper::wrap("type")+":"+uuidStr;
    result += ",";
    
    snprintf(tempStr, 4, "%hd", acclaim);
    result += HAPHelper::wrap("iid")+":"+tempStr;
    result += ",";
    
    if (len > 0) {
        snprintf(tempStr, 4, "%hd", len);
        result += HAPHelper::wrap("maxLen")+":"+tempStr;
        result += ",";
    }
    
    result += "\"format\":\"string\"";
    
    return "{"+result+"}";
}


String boolCharacteristics::describe() {
    return attribute(type, iid, permission, _value);
}

String floatCharacteristics::describe() {
    return attribute(type, iid, permission, _value, _minVal, _maxVal, _step, _unit);
}

String intCharacteristics::describe() {
    return attribute(type, iid, permission, _value, _minVal, _maxVal, _step, _unit);
}

String stringCharacteristics::describe() {
    return attribute(type, iid, permission, _value, maxLen);
}
