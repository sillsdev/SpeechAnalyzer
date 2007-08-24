//#ifndef CONVERT_H
//#define CONVERT_H
/*
  class CCharConverter
        {
          public:
            static char *Copyright(void);
            static float Version(void);
	        CCharConverter();
	        ~CCharConverter();
	        static char *IpaToSampa(const char *IpaString);
	        static bool QuerySymbol(const char IpaChar);
 
          private:
	        int Lookup(const void *Key, const void *TblEntry); 
        }; 
*/ 
#include <TCHAR.h>

extern "C" int Lookup(const void *Key, const void *TblEntry);
extern "C" TCHAR * IpaToSampa(const TCHAR *IpaString);
extern "C" bool QuerySymbol(const TCHAR IpaChar);
//#endif
