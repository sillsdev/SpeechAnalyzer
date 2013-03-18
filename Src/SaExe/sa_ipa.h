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

#ifndef INCLUDED_SA_IPA
#define INCLUDED_SA_IPA

class CFontTable : public CFont {
    // Constructor/Destructor
public:
    CFontTable();
    //  virtual ~CFontTable() = 0; // Pure Virtual Class

    // Attributes
public:
    enum tUnit {
        BYTE,
        CHARACTER,
        DELIMITEDWORD,
        NONE
    };

    char m_wordDelimiter;

    // Operations
public:
    int GetLength(tUnit nInUnits, const CString & szString) const;
    virtual CString GetNext(tUnit nInUnits, int & nIndex, const CString & szString) const = 0;
    CString GetRemainder(tUnit nInUnits, int nIndex, const CString & szString) const;
    void RemoveWordDelimiters(CString & szString) const;

    virtual BOOL IsIPA() const = 0;
};

class CFontTableIPA : public CFontTable {
    // Constructor/Destructor
public:
    CFontTableIPA();

    // Attributes
public:

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

    // Operations
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
    // Constructor/Destructor
public:
    CFontTableANSI() {};

    // Attributes
public:

private:

    // Operations
public:
    int GetLength(tUnit nInUnits, const CString & szString) const;
    CString GetNext(tUnit nInUnits, int & nIndex, const CString & szString) const;
    virtual BOOL IsIPA() const {
        return FALSE;
    };

};

#endif
