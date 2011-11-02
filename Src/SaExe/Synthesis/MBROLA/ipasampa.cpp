//#include "stdMbrola.h"
#include <string.h>
#include <search.h>
//#include "dsp\dspTypes.h"
#include "ipasampa.h"

//CCharConverter::CCharConverter()
//{
//}

//CCharConverter::~CCharConverter()
//{
//}

/*------------------------------------------------------------------------*\
|                        Translation Tables                              |  
\*------------------------------------------------------------------------*/
static char *BaseIpa[] = {
#include "ussampa1.h"   //lookup table for single-byte
	//IPA strings (base characters)
}; 
static char *xIpa[] = {   
#include "ussampa2.h"   //lookup table for multiple-byte IPA 
	//strings (base plus diacritics)
};


/*------------------------------------------------------------------------*\
|                             IpaToSampa                                 |  
\*------------------------------------------------------------------------*/
/*                               Lookup                                   */ 
//int CCharConverter::Lookup(const void *Key, const void *TblEntry)

extern "C" int Lookup(const void *Key, const void *TblEntry)
{
	return(strncmp((char *)Key, *(char **)TblEntry, strcspn(*(char **)TblEntry, " "))); 
}

//char * CCharConverter::IpaToSampa(const char *IpaString)  //translate from IPA to Sampa

extern "C" char * IpaToSampa(const char *IpaString)  //translate from IPA to Sampa
{                          
	char *Found;                


	// Validate pointer to IPA string.
	if (!IpaString) return(NULL);

	switch(strlen(IpaString))
	{

		// Indicate IPA string is invalid if there are no characters.    
	case 0:  
		return(NULL);

		// If IPA string consists of only one character, do a simple table lookup.
	case 1:   
		if (*BaseIpa[(unsigned char)IpaString[0]] == 0) return(NULL);
		else return(BaseIpa[(unsigned char)IpaString[0]]);

		// Otherwise, do a binary search through the multi-byte table.         
	default:
		Found = (char *)bsearch((void *)IpaString, (void *)xIpa, 
			sizeof(xIpa)/sizeof(xIpa[0]), 
			sizeof(xIpa[0]), Lookup);
		if (Found) return(strpbrk(*(char **)Found, " ")+1);  
		else return(NULL);
	}      
}



/*------------------------------------------------------------------------*\
|                            QuerySymbol                                 |  
\*------------------------------------------------------------------------*/
//bool CCharConverter::QuerySymbol(const char IpaChar)

extern "C" bool QuerySymbol(const char IpaChar)   //check if symbol in translation table
{
	// Attempt to look up in single-byte table. 
	if (*BaseIpa[(unsigned char)IpaChar]) return(true);  

	// Attempt to find somewhere in multi-byte table.                                           
	for (unsigned short i = 0; i < sizeof(xIpa)/sizeof(xIpa[0]); i++)
		for (unsigned short j = 0; xIpa[i][j] != (char)' '; j++)
			if (IpaChar == xIpa[i][j]) return(true);
	return(false);          
}