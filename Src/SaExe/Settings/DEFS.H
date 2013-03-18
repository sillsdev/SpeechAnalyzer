// defs.h

// Common definitions.

#ifndef _defs_h
#define _defs_h

typedef size_t Length;  // Hungarian: len
// Length of a string

typedef unsigned long NumberOf;  // Hungarian: num
// Count of the number of some kind of objects.

typedef unsigned short ID;  // Hungarian: id
// Identifier of a Windows resource
typedef unsigned short IDString;  // Hungarian: ids
// A resource identifier (also known as a symbol) consists of a name
// mapped to an integer value. It provides a descriptive way of
// referring to a resource in source code.
// More details on string resources appear at the end of this file.

typedef int ItemIndex;  // Hungarian: itm
// Index of a combo or list box item.


inline BOOL bEqual(const char * pszA, const char * pszB) {
    return (strcmp(pszA, pszB) == 0);
}
inline BOOL bEqualDisregardingCase(const char * pszA, const char * pszB) {
    return (_stricmp(pszA, pszB) == 0);
}

inline BOOL bMemberOfCharSet(char ch, const char * pszCharSet) {
    return (strchr(pszCharSet, ch) != NULL);
}


#endif  // _defs_h
