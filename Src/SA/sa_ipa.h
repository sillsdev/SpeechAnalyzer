/////////////////////////////////////////////////////////////////////////////
// sa_ipa.h:
// Definition of the CFontTable, CFontTableIPA, CFontTableANSI
// Author: Steve MacLean
// copyright 1999 JAARS Inc. SIL
//
// Revision History
// 1.06.1.4
//      SDM Original
// 1.06.5
//      SDM Added function IsIPA()
/////////////////////////////////////////////////////////////////////////////
#ifndef SA_IPA_H
#define SA_IPA_H

class CFontTable : public CFont {
public:
    CFontTable();

    enum tUnit {
        BYTE,
        CHARACTER,
        DELIMITEDWORD,
        NONE
    };

    int GetLength(tUnit nInUnits, const CString & szString) const;
    virtual CString GetNext(tUnit nInUnits, int & nIndex, const CString & szString) const = 0;
    CString GetRemainder(tUnit nInUnits, int nIndex, const CString & szString) const;
    void RemoveWordDelimiters(CString & szString) const;
    virtual BOOL IsIPA() const = 0;

    char m_wordDelimiter;
};

class CFontTableIPA : public CFontTable {
public:
    CFontTableIPA();

private:
    enum tGlyphType {
        ENDofSTRING,
        INDEPENDENT,
        PREFIX,
        POSTFIX,
        BREAK,
        LINK
    };

    enum tPhoneticClassification {
        UNDEFINED,
        CONSONANT,
        VOWEL,
        DIACRITIC,
        SUPRASEGMENTAL,
        PUNCTUATION
    };

    class {
    public:
        tGlyphType glyphType;
        tPhoneticClassification phoneticType;
    } m_pChar[256];

public:
    CString GetNext(tUnit nInUnits, int & nIndex, const CString & szString) const;
    virtual BOOL IsIPA() const {
        return TRUE;
    };

private:
    void AddChar(int nAccessCode,
                 int nIPAClassification,
                 char * szIPADescription,
                 tPhoneticClassification nClassification,
                 char * szClassSub1,
                 char * szClassSub2,
                 char * szClassSub3,
                 char * szClassSub4,
                 tGlyphType nType);

    tGlyphType GlyphType(int nAccessCode) const {
        if ((nAccessCode < 256)&&(nAccessCode >=0)) {
            return m_pChar[nAccessCode].glyphType;
        } else {
            return INDEPENDENT;
        }
    };
};


class CFontTableANSI : public CFontTable {

public:
    CFontTableANSI() {};

    int GetLength(tUnit nInUnits, const CString & szString) const;
    CString GetNext(tUnit nInUnits, int & nIndex, const CString & szString) const;
    virtual BOOL IsIPA() const {
        return FALSE;
    };

};

#endif
