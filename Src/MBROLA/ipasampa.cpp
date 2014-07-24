#include <string.h>
#include <search.h>
#include "ipasampa.h"

static char * BaseIpa[] = {
#include "ussampa1.h"   //lookup table for single-byte
    //IPA strings (base characters)
};
static char * xIpa[] = {
#include "ussampa2.h"   //lookup table for multiple-byte IPA 
    //strings (base plus diacritics)
};

extern "C" int Lookup(const void * Key, const void * TblEntry) {
    return (strncmp((char *)Key, *(char **)TblEntry, strcspn(*(char **)TblEntry, " ")));
}

//translate from IPA to Sampa
extern "C" char * IpaToSampa(const TCHAR * IpaString) {

    char * Found = NULL;

    // Validate pointer to IPA string.
    if (!IpaString) {
        return (NULL);
    }

    switch (strlen(IpaString)) {

    // Indicate IPA string is invalid if there are no characters.
    case 0:
        return (NULL);

    // If IPA string consists of only one character, do a simple table lookup.
    case 1:
        if (*BaseIpa[(unsigned char)IpaString[0]] == 0) {
            return (NULL);
        } else {
            return (BaseIpa[(unsigned char)IpaString[0]]);
        }

    // Otherwise, do a binary search through the multi-byte table.
    default:
        Found = (char *)bsearch((void *)IpaString, (void *)xIpa,
                                sizeof(xIpa)/sizeof(xIpa[0]),
                                sizeof(xIpa[0]), Lookup);
        if (Found) {
            return (strpbrk(*(char **)Found, " ")+1);
        } else {
            return (NULL);
        }
    }
}

//check if symbol in translation table
extern "C" bool QuerySymbol(const TCHAR IpaChar) {
    // Attempt to look up in single-byte table.
    if (*BaseIpa[(unsigned char)IpaChar]) {
        return (true);
    }
    // Attempt to find somewhere in multi-byte table.
    for (unsigned short i = 0; i < sizeof(xIpa)/sizeof(xIpa[0]); i++)
        for (unsigned short j = 0; xIpa[i][j] != (char)' '; j++)
            if (IpaChar == xIpa[i][j]) {
                return (true);
            }
    return (false);
}