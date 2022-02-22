#include "pch.h"
#include "funcs.h"

int round2Int(double value) {
    return (int)floor(value + 0.5);
}

// convert UTF-8 string to wstring
wstring _to_wstring(const std::string& str) {
    std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
    return myconv.from_bytes(str);
}

// convert wstring to UTF-8 string
string _to_utf8(const std::wstring& str) {
    std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
    return myconv.to_bytes(str);
}

string Utf8(LPCTSTR val) {
    wstring temp = val;
    return _to_utf8(temp.c_str());
}

string Utf8(CString val) {
    LPCTSTR temp = (LPCTSTR)val;
    return _to_utf8(temp);
}

// trim from start (in place)
void ltrim(string& s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !isspace(ch);
    }));
}

// trim from start (in place)
void ltrim(wstring& s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](wchar_t ch) {
        return !isspace(ch);
    }));
}

// trim from end (in place)
void rtrim(string& s) {
    s.erase(find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !isspace(ch);
    }).base(), s.end());
}

// trim from end (in place)
void rtrim(wstring& s) {
    s.erase(find_if(s.rbegin(), s.rend(), [](wchar_t ch) {
        return !isspace(ch);
    }).base(), s.end());
}

// trim from both ends (in place)
void trim(string& s) {
    ltrim(s);
    rtrim(s);
}

// trim from both ends (in place)
void trim(wstring& s) {
    ltrim(s);
    rtrim(s);
}

// trim from start (copying)
string ltrim_copy(string s) {
    ltrim(s);
    return s;
}

// trim from start (copying)
wstring ltrim_copy(wstring s) {
    ltrim(s);
    return s;
}

// trim from end (copying)
string rtrim_copy(string s) {
    rtrim(s);
    return s;
}

// trim from end (copying)
wstring rtrim_copy(wstring s) {
    rtrim(s);
    return s;
}

// trim from both ends (copying)
string trim_copy(string s) {
    trim(s);
    return s;
}

// trim from both ends (copying)
wstring trim_copy(wstring s) {
    trim(s);
    return s;
}
