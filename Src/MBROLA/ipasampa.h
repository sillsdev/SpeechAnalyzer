#ifndef _IPASAMPA_H
#define _IPASAMPA_H
#include <TCHAR.h>

extern "C" int Lookup(const void * Key, const void * TblEntry);
extern "C" TCHAR * IpaToSampa(const TCHAR * IpaString);
extern "C" bool QuerySymbol(const TCHAR IpaChar);

#endif
