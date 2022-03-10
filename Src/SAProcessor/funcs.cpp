#include "pch.h"
#include "funcs.h"

int round2Int(double value) {
    return (int)floor(value + 0.5);
}

extern bool compare_no_case(LPCTSTR a, LPCTSTR b) {
    size_t sza = wcslen(a);
    size_t szb = wcslen(b);
    if (sza != szb) return false;
    for (size_t i = 0; i < sza; ++i) {
        if (tolower(a[i]) != tolower(b[i])) {
            return false;
        }
    }
    return true;
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

// convert wstring to UTF-8 string
string _to_utf8(LPCWSTR str) {
    std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
    return myconv.to_bytes(str);
}

string Utf8(LPCTSTR val) {
    wstring temp = val;
    return _to_utf8(temp.c_str());
}

string Utf8(wstring val) {
    LPCTSTR temp = (LPCTSTR)val.c_str();
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

long get_file_size(LPCTSTR filename) {
    ifstream in(filename, ifstream::ate | ifstream::binary);
    return (long)in.tellg();
}