#pragma once
// defs.h

// Common definitions.

#ifndef _defs_h
#define _defs_h

typedef size_t Length;  // Hungarian: len
// Length of a string

typedef unsigned short ID;  // Hungarian: id
// Identifier of a Windows resource
typedef unsigned short IDString;  // Hungarian: ids
// A resource identifier (also known as a symbol) consists of a name
// mapped to an integer value. It provides a descriptive way of
// referring to a resource in source code.
// More details on string resources appear at the end of this file.

typedef int ItemIndex;  // Hungarian: itm
// Index of a combo or list box item.

static const Length maxsizMString = 32767;

inline BOOL bEqual(LPCSTR pszA, LPCSTR pszB)
{
    return (strcmp(pszA, pszB) == 0);
}
inline BOOL bEqualDisregardingCase(LPCSTR pszA, LPCSTR pszB)
{
    return (_stricmp(pszA, pszB) == 0);
}

inline BOOL bMemberOfCharSet(char ch, LPCSTR pszCharSet)
{
    return (strchr(pszCharSet, ch) != NULL);
}


#endif
