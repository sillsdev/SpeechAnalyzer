/////////////////////////////////////////////////////////////////////////////
// UnicodeString.h
// Implementation of various unicode functions
// Author: Steve MacLean
// copyright 2002 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////
#ifndef _UNICODE_STRING_H
#define _UNICODE_STRING_H

#ifndef WM_UNICHAR
#define WM_UNICHAR                      0x0109
#endif

#ifndef UNICODE_NOCHAR
#define UNICODE_NOCHAR                  0xFFFF
#endif

class CUnicodeString;
class CUtf32String;
class CUtf16String;
class CUtf8String;
class CFont;

class CUnicodeString {
public:
    virtual ~CUnicodeString() {
        ;
    }

    virtual operator CUtf8String() const = 0;
    virtual operator CUtf16String() const = 0;
    virtual operator CUtf32String() const = 0;

    static CUtf16String Decode(CFont * pFont, const std::string * szEncoded);
    virtual std::string Encode(CFont * pFont) const;

    CUtf32String Left(int nChars) const;

    virtual CUnicodeString & operator=(const CUnicodeString &) = 0;
    CUtf32String operator+(const CUnicodeString & szString) const;
    virtual BOOL IsEmpty() const = 0;
};

class CUtf32String : public CUnicodeString {
public:
    CUtf32String() {
        ;
    }
    explicit CUtf32String(unsigned long lChar) {
        *this += lChar;
    }
    virtual ~CUtf32String() {
        ;
    }

    virtual operator CUtf8String() const;
    virtual operator CUtf16String() const;
    virtual operator CUtf32String() const {
        return *this;
    }

    virtual CUnicodeString & operator=(const CUtf32String & szNew) {
        m_szContents = szNew.m_szContents;
        return *this;
    }
    virtual CUnicodeString & operator=(const CUnicodeString & szNew) {
        m_szContents = CUtf32String(szNew).m_szContents;
        return *this;
    }
    CUtf32String & operator+=(const unsigned long lChar) {
        if (IsValidUnicode(lChar)) {
            m_szContents+=lChar;
        }
        return *this;
    }
    static BOOL IsValidUnicode(unsigned long lChar) {
        return (lChar < 0x110000);
    }

    CUtf32String Left(int nChars) const;
    CUtf32String Mid(int nStart) const;
    CUtf32String Mid(int nStart, int nChars) const;
    CUtf32String & operator+=(const CUnicodeString & szString) {
        m_szContents += CUtf32String(szString).m_szContents;
        return *this;
    }

    virtual BOOL IsEmpty() const {
        return size() == 0;
    }
    int size() const {
        return m_szContents.size();
    }

private:
    std::basic_string<unsigned long> m_szContents;
};

class CUtf16String : public CUnicodeString {
public:
    CUtf16String() {
        ;
    }
    explicit CUtf16String(LPCWSTR szValue) : m_szContents(szValue) {
        ;
    }
    virtual ~CUtf16String() {
        ;
    }

    virtual operator CUtf8String() const;
    virtual operator CUtf16String() const {
        return *this;
    }
    virtual operator CUtf32String() const;

    std::wstring getUtf16() const {
        return m_szContents;
    }
    virtual std::string Encode(CFont * pFont) const;

    virtual CUnicodeString & operator=(const CUtf16String & szNew) {
        m_szContents = szNew.m_szContents;
        return *this;
    }
    virtual CUnicodeString & operator=(const CUnicodeString & szNew) {
        m_szContents = CUtf16String(szNew).m_szContents;
        return *this;
    }
    CUtf16String & operator+=(const CUnicodeString & szString) {
        m_szContents += CUtf16String(szString).m_szContents;
        return *this;
    }

    virtual BOOL IsEmpty() const {
        return size() == 0;
    }
    int size() const {
        return m_szContents.size();
    }
private:
    enum Kef16 { kef16HighSurrogate, kef16LowSurrogate, kef16BMP, kef16BOM, kef16Error};
    Kef16 classifyChar(unsigned short ucValue) const;

    CUtf16String & operator+=(unsigned long);

    friend CUtf32String::operator CUtf16String() const;

    std::wstring m_szContents;
};

class CUtf8String : public CUnicodeString {
public:
    CUtf8String() {
        ;
    }
    explicit CUtf8String(LPCSTR szValue) : m_szContents(szValue) {
        ;
    }
    virtual ~CUtf8String() {
        ;
    }

    virtual operator CUtf8String() const {
        return *this;
    }
    virtual operator CUtf16String() const;
    virtual operator CUtf32String() const;

    std::string getUtf8() const {
        return m_szContents;
    }

    virtual CUnicodeString & operator=(const CUtf8String & szNew) {
        m_szContents = szNew.m_szContents;
        return *this;
    }
    virtual CUnicodeString & operator=(const CUnicodeString & szNew) {
        m_szContents = CUtf8String(szNew).m_szContents;
        return *this;
    }
    CUtf8String & operator+=(const CUnicodeString & szString) {
        m_szContents += CUtf8String(szString).m_szContents;
        return *this;
    }

    virtual BOOL IsEmpty() const {
        return size() == 0;
    }
    int size() const {
        return m_szContents.size();
    }
private:
    enum Kef8 { kef8Lead1,kef8Lead2,kef8Lead3,kef8Lead4,kef8Trail,kef8FF,kef8FE,kef8Other};
    Kef8 classifyChar(unsigned char ucValue) const;

    CUtf8String & operator+=(unsigned long);

    friend CUtf32String::operator CUtf8String() const;

    std::string m_szContents;
};

#endif