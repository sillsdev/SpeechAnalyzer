#ifndef _FUNCS_H
#define _FUNCS_H

#include <algorithm> 
#include <cctype>
#include <locale>

extern int round2Int(double value);

// convert UTF-8 string to wstring
extern wstring _to_wstring(const std::string& str);

// convert wstring to UTF-8 string
extern string _to_utf8(const std::wstring& str);

extern string Utf8(LPCTSTR val);

extern string Utf8(CString val);

// trim from start (in place)
extern void ltrim(string& s);
extern void ltrim(wstring& s);

// trim from end (in place)
extern void rtrim(string& s);
extern void rtrim(wstring& s);

// trim from both ends (in place)
extern void trim(string& s);
extern void trim(wstring& s);

// trim from start (copying)
extern string ltrim_copy(string s);
extern wstring ltrim_copy(wstring s);

// trim from end (copying)
extern string rtrim_copy(string s);
extern wstring rtrim_copy(wstring s);

// trim from both ends (copying)
extern string trim_copy(string s);
extern wstring trim_copy(wstring s);

#endif