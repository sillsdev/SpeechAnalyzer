// patch.cpp Implementation of Shoebox single char pattern classes (1995-02-27)


// Change history:
// 1995-02-28 0.07a MRP: actually distinction is between single and multi-char
// 1995-02-27 0.07  MRP: separate internal program patterns from end-user ones


#include "stdSettings.h"
#include "patch.h"



const char * Shw_pszWhiteSpace = " \t\n"; // white space: space, tab, newline
const char * Shw_pszOtherWhiteSpace = "\t\n"; // above, other than space itself

bool Shw_bAtWhiteSpace(const char * psz)
{
    return (strchr(Shw_pszWhiteSpace, *psz) != NULL);
}


