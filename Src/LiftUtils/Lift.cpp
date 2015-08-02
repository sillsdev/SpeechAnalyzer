#include "Lift.h"

#include <Rpc.h>
#include <time.h>

#pragma comment( lib, "uriparser")
#pragma comment( lib, "rpcrt4")

void Lift::dumpChildren(Element & element) {
    if (element.elementCount()==0) {
        return;
    }
    //printf("dumpChildren children=%d\n",element.elementCount());
    for (size_t i=0; i<element.elementCount(); i++) {
        printf("losing %s\n",utf8(element.get(i)->localname.c_str()).c_str());
    }
    if (element.elementCount()>0) {
        ::DebugBreak();
    }
}

void Lift::removeElement(Element & parent, Element & element) {
    try {
        printf("removing %s (%p) from %s (%p)\n",utf8(element.localname).c_str(), &element, utf8(parent.localname).c_str(), &parent);
        dumpChildren(element);
        parent.removeElement(element.id);
    } catch (logic_error & e) {
        printf("failure:%s\n",e.what());
        throw e;
    } catch (...) {
        int i=0;
        throw;
    }
}

wstring Lift::createUUID() {
    UUID uuid;
    RPC_STATUS status = UuidCreate(&uuid);
    if (status!=RPC_S_OK) {
        return L"";
    }
    RPC_WSTR buffer = NULL;
    status = UuidToStringW(&uuid,&buffer);
    if (status!=RPC_S_OK) {
        return L"";
    }
    wstring result = (wchar_t *)buffer;
    status = RpcStringFree(&buffer);
    if (status!=RPC_S_OK) {
        return L"";
    }
    return result;
}

/*
* This is the same type as an XML Schema datetime type. See http://www.w3.org/TR/xmlschema-2/#dateTime for details.
* In summary a time is a string representing a date and time in the following format: yyyy-MM-ddThh:mm:sszzzz.
* Times are given relative to GMT, thus if no timezone information is included then the time is considered to be in GMT.
* Likewise if no time is included then it is assumed to be 0, i.e. Midnight GMT at the start of the given day.
*
* yyyy represents a 4 digit year relative to AD 0 (yes it can be negative)
* -separator
* MM represents the month as 2 digits from 01 to 12.
* -separator
* dd represents the day of the month as 2 digits from 01 to 31.
* TTime separator. This and all following it are optional as a single unit (i.e. if the T exists,
* so must all the time elements unless marked as optional).
* hh represents the hour as 2 digits from 00 to 23. The hour can be 24 if the rest of the time is 0.
* :separator
* mm represents the minutes of the hour as two digits from 00 to 59
* :separator
* ss represents the seconds as 2 digits from 00 to 59
* zzzz represents optional timezone information in the form:+|-hh:mm indicating the time zone is ahead or behind
* GMT by the given number of hours and minutes.
* Optionally a timezone value of Z1 indicates an explicit zero offset from GMT.



struct tm {
        int tm_sec;     seconds after the minute - [0,59]
        int tm_min;     minutes after the hour - [0,59]
        int tm_hour;    hours since midnight - [0,23]
        int tm_mday;    day of the month - [1,31]
        int tm_mon;     months since January - [0,11]
        int tm_year;    years since 1900
        int tm_wday;    days since Sunday - [0,6]
        int tm_yday;    days since January 1 - [0,365]
        int tm_isdst;   daylight savings time flag


*/

wstring Lift::createDate() {

    time_t ltime;
    struct tm today;

    _time64(&ltime);
    errno_t err = _gmtime64_s(&today, &ltime);
    if (err) {
        printf("_gmtime64_s failed due to an invalid argument.");
        exit(1);
    }

    wstring result;
    wchar_t buffer[256];
    wmemset(buffer,0,_countof(buffer));
    swprintf(buffer,_countof(buffer),L"%04d-%02d-%02dT%02d:%02d:%02dZ",(1900+today.tm_year),(today.tm_mon+1),today.tm_mday,today.tm_hour,today.tm_min,today.tm_sec);
    result = buffer;
    return result;

}
