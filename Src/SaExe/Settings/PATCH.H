// patch.h Interface for Shoebox single char pattern classes (1995-02-27)


// Change history:
// 1995-02-28 0.07a MRP: actually distinction is between single and multi-char
// 1995-02-27 0.07  MRP: separate internal program patterns from end-user ones


#ifndef PATCH_H
#define PATCH_H

extern const char* Shw_pszWhiteSpace;  // white space: space, tab, newline
extern const char* Shw_pszOtherWhiteSpace;  // above, other than space itself

bool Shw_bAtWhiteSpace(const char* psz);

#endif  // PATCH_H