/////////////////////////////////////////////////////////////////////////////
// sa_ipa.cpp:
// Implementation of the CFontTable, CFontTableIPA, CFontTableANSI classes
// Author: Steve MacLean
// copyright 1999 JAARS Inc. SIL
//
// Revision History
// 1.06.1.4
//      SDM Original
// 1.06.8
//      SDM changed word delimiter
// 1.5Test8.2
//      SDM removed word delimiters from IPA::GetNext - NONE
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "sa_ipa.h"
#include "appdefs.h"


CFontTable::CFontTable() {
    m_wordDelimiter = EDIT_WORD_DELIMITER; //SDM 1.06.8
};

void CFontTable::RemoveWordDelimiters(CString & szString) const {
    CString szProcessed = "";

    for (int nIndex = 0; nIndex < szString.GetLength(); nIndex++) {
        if (szString[nIndex]!=m_wordDelimiter) {
            szProcessed += szString[nIndex];
        }
    }
    szString = szProcessed;
}

/***************************************************************************/
// CFontTable::GetLength Get length of string in words, characters, bytes
/***************************************************************************/
int CFontTable::GetLength(tUnit nInUnits, const CString & szString) const {
    int nCount = 0;
    int nIndex = 0;

    switch (nInUnits) {
    case BYTE:
        return szString.GetLength();
        break;
    case CHARACTER: {
        CString szWorking = szString;
        // Don't include Word Delimiters
        RemoveWordDelimiters(szWorking);
        while (nIndex < szWorking.GetLength()) {
            if (GetNext(CHARACTER,nIndex,szWorking).GetLength() > 0) {
                nCount++;
            }
        }
        return nCount;
        break;
    }
    case DELIMITEDWORD:
        while (nIndex < szString.GetLength()) {
            nCount++;
            GetNext(DELIMITEDWORD,nIndex,szString).GetLength();
        }
        return nCount;
        break;
    case NONE:
        return (szString.IsEmpty()) ? 0 : 1;
        break;
    default:
        return 0;
    }
};

/***************************************************************************/
// CFontTable::GetRemainder Get remainder of string in words, characters, bytes
/***************************************************************************/
CString CFontTable::GetRemainder(tUnit nInUnits, int nStringIndex, const CString & szString) const {
    int nIndex = 0;
    CString szReturn = "";

    switch (nInUnits) {
    case BYTE:
        return szString.Mid(nStringIndex);
        break;
    case CHARACTER: {
        CString szWorking = szString.Mid(nStringIndex);
        if (nStringIndex != 0) {
            // Prepend a dummy character to properly handle mid string processing
            szWorking = "a" + szWorking;
            nIndex = 1;
            RemoveWordDelimiters(szWorking);
        }
        while (nIndex < szWorking.GetLength()) {
            szReturn += GetNext(CHARACTER,nIndex,szWorking);
        }
        return szReturn;
        break;
    }
    case DELIMITEDWORD:
        nIndex = nStringIndex;
        while (nIndex < szString.GetLength()) {
            szReturn += GetNext(DELIMITEDWORD,nIndex,szString);
        }
        return szReturn;
        break;
    case NONE:
        szReturn = szString.Mid(nStringIndex);
        //      RemoveWordDelimiters(szReturn);
        return szReturn;
        break;
    default:
        return szReturn;
    }
};

void CFontTableIPA::AddChar(int nAccessCode, int , char * , tPhoneticClassification nClassification,
                            char * , char * , char * , char * ,tGlyphType nType) {
    if ((nAccessCode <= 255)&&(nAccessCode>=0)) {
        m_pChar[nAccessCode].phoneticType = nClassification;
        m_pChar[nAccessCode].glyphType = nType;
    }
}

CFontTableIPA::CFontTableIPA() {
    // Initialize Table
    for (int nIndex=0; nIndex<256; nIndex++) {
        m_pChar[nIndex].phoneticType = UNDEFINED;
        m_pChar[nIndex].glyphType = INDEPENDENT;
    }

    // Fill Table
    AddChar(0,0,"Undefined",UNDEFINED,"","","","",ENDofSTRING);
    AddChar(1,0,"Undefined",UNDEFINED,"","","","",INDEPENDENT);
    AddChar(2,0,"Undefined",UNDEFINED,"","","","",INDEPENDENT);
    AddChar(3,0,"Undefined",UNDEFINED,"","","","",INDEPENDENT);
    AddChar(4,0,"Undefined",UNDEFINED,"","","","",INDEPENDENT);
    AddChar(5,0,"Undefined",UNDEFINED,"","","","",INDEPENDENT);
    AddChar(6,0,"Undefined",UNDEFINED,"","","","",INDEPENDENT);
    AddChar(7,0,"Undefined",UNDEFINED,"","","","",INDEPENDENT);
    AddChar(8,0,"Undefined",UNDEFINED,"","","","",INDEPENDENT);
    AddChar(9,0,"Undefined",UNDEFINED,"","","","",INDEPENDENT);
    AddChar(10,0,"Undefined",UNDEFINED,"","","","",INDEPENDENT);
    AddChar(11,0,"Undefined",UNDEFINED,"","","","",INDEPENDENT);
    AddChar(12,0,"Undefined",UNDEFINED,"","","","",INDEPENDENT);
    AddChar(13,0,"Undefined",UNDEFINED,"","","","",INDEPENDENT);
    AddChar(14,0,"Undefined",UNDEFINED,"","","","",INDEPENDENT);
    AddChar(15,0,"Undefined",UNDEFINED,"","","","",INDEPENDENT);
    AddChar(16,0,"Undefined",UNDEFINED,"","","","",INDEPENDENT);
    AddChar(17,0,"Undefined",UNDEFINED,"","","","",INDEPENDENT);
    AddChar(18,0,"Undefined",UNDEFINED,"","","","",INDEPENDENT);
    AddChar(19,0,"Undefined",UNDEFINED,"","","","",INDEPENDENT);
    AddChar(20,0,"Undefined",UNDEFINED,"","","","",INDEPENDENT);
    AddChar(21,0,"Undefined",UNDEFINED,"","","","",INDEPENDENT);
    AddChar(22,0,"Undefined",UNDEFINED,"","","","",INDEPENDENT);
    AddChar(23,0,"Undefined",UNDEFINED,"","","","",INDEPENDENT);
    AddChar(24,0,"Undefined",UNDEFINED,"","","","",INDEPENDENT);
    AddChar(25,0,"Undefined",UNDEFINED,"","","","",INDEPENDENT);
    AddChar(26,0,"Undefined",UNDEFINED,"","","","",INDEPENDENT);
    AddChar(27,0,"Undefined",UNDEFINED,"","","","",INDEPENDENT);
    AddChar(28,0,"Undefined",UNDEFINED,"","","","",INDEPENDENT);
    AddChar(29,0,"Undefined",UNDEFINED,"","","","",INDEPENDENT);
    AddChar(30,0,"Undefined",UNDEFINED,"","","","",INDEPENDENT);
    AddChar(31,0,"Undefined",UNDEFINED,"","","","",INDEPENDENT);
    AddChar(38,0,"Undefined",UNDEFINED,"","","","",INDEPENDENT);
    AddChar(127,0,"Undefined",UNDEFINED,"","","","",INDEPENDENT);
    AddChar(147,0,"Undefined",UNDEFINED,"","","","",INDEPENDENT);
    AddChar(152,0,"Undefined",UNDEFINED,"","","","",INDEPENDENT);
    AddChar(160,0,"Undefined",UNDEFINED,"","","","",INDEPENDENT);
    AddChar(165,0,"Undefined",UNDEFINED,"","","","",INDEPENDENT);
    AddChar(177,0,"Undefined",UNDEFINED,"","","","",INDEPENDENT);
    AddChar(182,0,"Undefined",UNDEFINED,"","","","",INDEPENDENT);
    AddChar(183,0,"Undefined",UNDEFINED,"","","","",INDEPENDENT);
    AddChar(187,0,"Undefined",UNDEFINED,"","","","",INDEPENDENT);
    AddChar(197,0,"Undefined",UNDEFINED,"","","","",INDEPENDENT);
    AddChar(208,0,"Undefined",UNDEFINED,"","","","",INDEPENDENT);
    AddChar(218,0,"Undefined",UNDEFINED,"","","","",INDEPENDENT);
    AddChar(219,0,"Undefined",UNDEFINED,"","","","",INDEPENDENT);
    AddChar(220,0,"Undefined",UNDEFINED,"","","","",INDEPENDENT);
    AddChar(221,0,"Undefined",UNDEFINED,"","","","",INDEPENDENT);
    AddChar(222,0,"Undefined",UNDEFINED,"","","","",INDEPENDENT);
    AddChar(223,0,"Undefined",UNDEFINED,"","","","",INDEPENDENT);
    AddChar(224,0,"Undefined",UNDEFINED,"","","","",INDEPENDENT);
    AddChar(226,0,"Undefined",UNDEFINED,"","","","",INDEPENDENT);
    AddChar(244,0,"Undefined",UNDEFINED,"","","","",INDEPENDENT);
    AddChar(173,100,"Capital C",CONSONANT,"Undefined","Undefined","Undefined","Undefined",INDEPENDENT);
    AddChar(174,100,"Capital V",CONSONANT,"Undefined","Undefined","Undefined","Undefined",INDEPENDENT);
    AddChar(112,101,"Lowercase P",CONSONANT,"Pulmonic","Plosive","Bilabial","Voiced",INDEPENDENT);
    AddChar(98,102,"Lowercase B",CONSONANT,"Pulmonic","Plosive","Bilabial","Voiceless",INDEPENDENT);
    AddChar(116,103,"Lowercase T",CONSONANT,"Pulmonic","Plosive","Dental/Alveolar","Voiced",INDEPENDENT);
    AddChar(100,104,"Lowercase D",CONSONANT,"Pulmonic","Plosive","Dental/Alveolar","Voiceless",INDEPENDENT);
    AddChar(255,105,"Right-tail T",CONSONANT,"Pulmonic","Plosive","Retroflex","Voiced",INDEPENDENT);
    AddChar(234,106,"Right-tail D",CONSONANT,"Pulmonic","Plosive","Retroflex","Voiceless",INDEPENDENT);
    AddChar(99,107,"Lowercase C",CONSONANT,"Pulmonic","Plosive","Palatal","Voiced",INDEPENDENT);
    AddChar(239,108,"Barred Dotless J",CONSONANT,"Pulmonic","Plosive","Palatal","Voiceless",INDEPENDENT);
    AddChar(107,109,"Lowercase K",CONSONANT,"Pulmonic","Plosive","Velar","Voiced",INDEPENDENT);
    AddChar(103,110,"Lowercase G",CONSONANT,"Pulmonic","Plosive","Velar","Voiceless",INDEPENDENT);
    AddChar(113,111,"Lowercase Q",CONSONANT,"Pulmonic","Plosive","Uvular","Voiced",INDEPENDENT);
    AddChar(71,112,"Small Capital G",CONSONANT,"Pulmonic","Plosive","Uvular","Voiceless",INDEPENDENT);
    AddChar(63,113,"Glottal Stop",CONSONANT,"Pulmonic","Plosive","Glottal","Voiced",INDEPENDENT);
    AddChar(109,114,"Lowercase M",CONSONANT,"Pulmonic","Nasal","Bilabial","Voiced",INDEPENDENT);
    AddChar(77,115,"Left-tail M (at right)",CONSONANT,"Pulmonic","Nasal","Labioden","Voiced",INDEPENDENT);
    AddChar(110,116,"Lowercase N",CONSONANT,"Pulmonic","Nasal","Dental/Alveolar","Voiced",INDEPENDENT);
    AddChar(247,117,"Right-tail N",CONSONANT,"Pulmonic","Nasal","Retroflex","Voiced",INDEPENDENT);
    AddChar(248,118,"Left-tail N (at left)",CONSONANT,"Pulmonic","Nasal","Palatal","Voiced",INDEPENDENT);
    AddChar(78,119,"Eng",CONSONANT,"Pulmonic","Nasal","Velar","Voiced",INDEPENDENT);
    AddChar(178,120,"Small Capital N",CONSONANT,"Pulmonic","Nasal","Uvular","Voiced",INDEPENDENT);
    AddChar(245,121,"Small Capital B",CONSONANT,"Pulmonic","Trill","Bilabial","Voiced",INDEPENDENT);
    AddChar(114,122,"Lowercase R",CONSONANT,"Pulmonic","Trill","Dental/Alveolar","Voiced",INDEPENDENT);
    AddChar(123,123,"Small Capital R",CONSONANT,"Pulmonic","Trill","Uvular","Voiced",INDEPENDENT);
    AddChar(82,124,"Fish-hook R",CONSONANT,"Pulmonic","Tap or Flap","Dental/Alveolar","Voiced",INDEPENDENT);
    AddChar(125,125,"Right-tail R",CONSONANT,"Pulmonic","Tap or Flap","Retroflex","Voiced",INDEPENDENT);
    AddChar(184,126,"Phi",CONSONANT,"Pulmonic","Fricative","Bilabial","Voiced",INDEPENDENT);
    AddChar(66,127,"Beta",CONSONANT,"Pulmonic","Fricative","Bilabial","Voiceless",INDEPENDENT);
    AddChar(102,128,"Lowercase F",CONSONANT,"Pulmonic","Fricative","Labioden","Voiced",INDEPENDENT);
    AddChar(118,129,"Lowercase V",CONSONANT,"Pulmonic","Fricative","Labioden","Voiceless",INDEPENDENT);
    AddChar(84,130,"Theta",CONSONANT,"Pulmonic","Fricative","Dental","Voiced",INDEPENDENT);
    AddChar(68,131,"Eth",CONSONANT,"Pulmonic","Fricative","Dental","Voiceless",INDEPENDENT);
    AddChar(115,132,"Lowercase S",CONSONANT,"Pulmonic","Fricative","Alveolar","Voiced",INDEPENDENT);
    AddChar(122,133,"Lowercase Z",CONSONANT,"Pulmonic","Fricative","Alveolar","Voiceless",INDEPENDENT);
    AddChar(83,134,"Esh",CONSONANT,"Pulmonic","Fricative","Post Alveolar","Voiced",INDEPENDENT);
    AddChar(90,135,"Yogh",CONSONANT,"Pulmonic","Fricative","Post Alveolar","Voiceless",INDEPENDENT);
    AddChar(167,136,"Right-Tail S (at left)",CONSONANT,"Pulmonic","Fricative","Retroflex","Voiced",INDEPENDENT);
    AddChar(189,137,"Right-tail Z",CONSONANT,"Pulmonic","Fricative","Retroflex","Voiceless",INDEPENDENT);
    AddChar(67,138,"C Cedilla",CONSONANT,"Pulmonic","Fricative","Palatal","Voiced",INDEPENDENT);
    AddChar(198,139,"Curly-tail J",CONSONANT,"Pulmonic","Fricative","Palatal","Voiceless",INDEPENDENT);
    AddChar(120,140,"Lowercase X",CONSONANT,"Pulmonic","Fricative","Velar","Voiced",INDEPENDENT);
    AddChar(196,141,"Gamma",CONSONANT,"Pulmonic","Fricative","Velar","Voiceless",INDEPENDENT);
    AddChar(88,142,"Chi",CONSONANT,"Pulmonic","Fricative","Uvular","Voiced",INDEPENDENT);
    AddChar(210,143,"Inverted Small Capital R",CONSONANT,"Pulmonic","Fricative","Uvular","Voiceless",INDEPENDENT);
    AddChar(240,144,"Crossed H",CONSONANT,"Pulmonic","Fricative","Pharyngl","Voiced",INDEPENDENT);
    AddChar(192,145,"Reversed Glottal Stop",CONSONANT,"Pulmonic","Fricative","Pharyngl","Voiceless",INDEPENDENT);
    AddChar(104,146,"Lowercase H",CONSONANT,"Pulmonic","Fricative","Glottal","Voiced",INDEPENDENT);
    AddChar(250,147,"Hooktop H",CONSONANT,"Pulmonic","Fricative","Glottal","Voiceless",INDEPENDENT);
    AddChar(194,148,"Belted L",CONSONANT,"Pulmonic","Lateral Fricative","Dental/Alveolar","Voiced",INDEPENDENT);
    AddChar(76,149,"L-Yogh Digraph",CONSONANT,"Pulmonic","Lateral Fricative","Dental/Alveolar","Voiceless",INDEPENDENT);
    AddChar(86,150,"Cursive V",CONSONANT,"Pulmonic","Approximant","Labioden","Voiced",INDEPENDENT);
    AddChar(168,151,"Turned R",CONSONANT,"Pulmonic","Approximant","Dental/Alveolar","Voiced",INDEPENDENT);
    AddChar(211,152,"Turned R, Right Tail",CONSONANT,"Pulmonic","Approximant","Retroflex","Voiced",INDEPENDENT);
    AddChar(106,153,"Lowercase J",CONSONANT,"Pulmonic","Approximant","Palatal","Voiced",INDEPENDENT);
    AddChar(190,153,"Dotless J",CONSONANT,"Pulmonic","Approximant","Palatal","Voiced",INDEPENDENT);
    AddChar(229,154,"Turned M, Right Leg",CONSONANT,"Pulmonic","Approximant","Velar","Voiced",INDEPENDENT);
    AddChar(108,155,"Lowercase L",CONSONANT,"Pulmonic","Lateral Approximant","Dental/Alveolar","Voiced",INDEPENDENT);
    AddChar(241,156,"Right-tail L",CONSONANT,"Pulmonic","Lateral Approximant","Retroflex","Voiced",INDEPENDENT);
    AddChar(180,157,"Turned Y",CONSONANT,"Pulmonic","Lateral Approximant","Palatal","Voiced",INDEPENDENT);
    AddChar(59,158,"Small Capital L",CONSONANT,"Pulmonic","Lateral Approximant","","Voiced",INDEPENDENT);
    AddChar(186,160,"Hooktop B",CONSONANT,"Non-Pulmonic","Imposive","Bilabial","Voiced",INDEPENDENT);
    AddChar(235,162,"Hooktop D",CONSONANT,"Non-Pulmonic","Imposive","Dental/Alveolar","Voiced",INDEPENDENT);
    AddChar(215,164,"Hooktop Barred Dotless J",CONSONANT,"Non-Pulmonic","Imposive","Palatal","Voiced",INDEPENDENT);
    AddChar(169,166,"Hooktop G",CONSONANT,"Non-Pulmonic","Imposive","Velar","Voiced",INDEPENDENT);
    AddChar(253,168,"Hooktop Small Capital G",CONSONANT,"Non-Pulmonic","Imposive","Uvular","Voiced",INDEPENDENT);
    AddChar(227,169,"Turned W",CONSONANT,"Pulmonic","Fricative","Labial-Velar","Voiceless",INDEPENDENT);
    AddChar(119,170,"Lowercase W",CONSONANT,"Pulmonic","Approximant","Labial-Velar","Voiced",INDEPENDENT);
    AddChar(231,171,"Turned H",CONSONANT,"Pulmonic","Approximant","Labial-Palatal","Voiced",INDEPENDENT);
    AddChar(251,172,"Barred Glottal Stop",CONSONANT,"Pulmonic","Plosive","Epiglottal","Voiceless",INDEPENDENT);
    AddChar(75,173,"Small Capital H",CONSONANT,"Pulmonic","Fricative","Epiglottal","Voiced",INDEPENDENT);
    AddChar(185,174,"Barred Reversed Glottal Stop",CONSONANT,"Pulmonic","Fricative","Epiglottal","Voiced",INDEPENDENT);
    AddChar(238,175,"Hooked Heng",CONSONANT,"Pulmonic","Fricative","Post Alveolar/Velar","Voiceless",INDEPENDENT);
    AddChar(135,176,"Bull's Eye",CONSONANT,"Non-Pulmonic","Click","Bilabial","",INDEPENDENT);
    AddChar(142,177,"Pipe",CONSONANT,"Non-Pulmonic","Click","Dental","",INDEPENDENT);
    AddChar(151,178,"Exclamation Point",CONSONANT,"Non-Pulmonic","Click","(Post) Alveolar","",INDEPENDENT);
    AddChar(156,179,"Double-barred Pipe",CONSONANT,"Non-Pulmonic","Click","Palatoalveolar","",INDEPENDENT);
    AddChar(146,180,"Double Pipe",CONSONANT,"Non-Pulmonic","Click","Alveolar Lateral","",INDEPENDENT);
    AddChar(228,181,"Turned Long-leg R",CONSONANT,"Pulmonic","Trill or Flap","Alveolar Lateral","Voiceless",INDEPENDENT);
    AddChar(254,182,"Curly-tail C",CONSONANT,"Pulmonic","Fricative","Alveolar Palatal","Voiced",INDEPENDENT);
    AddChar(252,183,"Curly-tail Z",CONSONANT,"Pulmonic","Fricative","Alveolar Palatal","Voiceless",INDEPENDENT);
    AddChar(105,301,"Lowercase I",VOWEL,"Front","Close","Unrounded","",INDEPENDENT);
    AddChar(101,302,"Lowercase E",VOWEL,"Front","Close-Mid","Unrounded","",INDEPENDENT);
    AddChar(69,303,"Epsilon",VOWEL,"Front","Open-Mid","Unrounded","",INDEPENDENT);
    AddChar(97,304,"Lowercase A",VOWEL,"Front","Open","Unrounded","",INDEPENDENT);
    AddChar(65,305,"Cursive A",VOWEL,"Back","Open","Unrounded","",INDEPENDENT);
    AddChar(141,306,"Open O",VOWEL,"Back","Open-Mid","Rounded","",INDEPENDENT);
    AddChar(111,307,"Lowercase O",VOWEL,"Back","Close-Mid","Rounded","",INDEPENDENT);
    AddChar(117,308,"Lowercase U",VOWEL,"Back","Close","Rounded","",INDEPENDENT);
    AddChar(121,309,"Lowercase Y",VOWEL,"Front","Close","Rounded","",INDEPENDENT);
    AddChar(79,310,"Slashed O",VOWEL,"Front","Close-Mid","Rounded","",INDEPENDENT);
    AddChar(191,311,"O-E Digraph",VOWEL,"Front","Open-Mid","Rounded","",INDEPENDENT);
    AddChar(175,312,"Small Capital O-E Digraph",VOWEL,"Front","Open","Rounded","",INDEPENDENT);
    AddChar(129,313,"Turned Cursive A",VOWEL,"Back","Open","Rounded","",INDEPENDENT);
    AddChar(195,314,"Turned V",VOWEL,"Back","Open-Mid","Unrounded","",INDEPENDENT);
    AddChar(70,315,"Ram's Horns",VOWEL,"Back","Close-Mid","Unrounded","",INDEPENDENT);
    AddChar(181,316,"Turned M",VOWEL,"Back","Close","Unrounded","",INDEPENDENT);
    AddChar(246,317,"Barred I",VOWEL,"Central","Close","Unrounded","",INDEPENDENT);
    AddChar(172,318,"Barred U",VOWEL,"Central","Close","Rounded","",INDEPENDENT);
    AddChar(73,319,"Small Capital I",VOWEL,"Near Front","Near Close","Unrounded","",INDEPENDENT);
    AddChar(89,320,"Small Capital Y",VOWEL,"Near Front","Near Close","Rounded","",INDEPENDENT);
    AddChar(85,321,"Upsilon",VOWEL,"Near Back","Near Close","Unrounded","",INDEPENDENT);
    AddChar(171,322,"Schwa",VOWEL,"Front","Close-Mid","Unrounded","",INDEPENDENT);
    AddChar(80,323,"Barred O",VOWEL,"Front","Close-Mid","Rounded","",INDEPENDENT);
    AddChar(140,324,"Turned A",VOWEL,"Central","Near Open","Unrounded","",INDEPENDENT);
    AddChar(81,325,"Ash Digraph",VOWEL,"Front","Near Open","Unrounded","",INDEPENDENT);
    AddChar(206,326,"Reversed Epsilon",VOWEL,"Central","Open-Mid","Unrounded","",INDEPENDENT);
    AddChar(207,396,"Closed Reversed Epsilon",VOWEL,"Central","Open-Mid","Rounded","",INDEPENDENT);
    AddChar(130,397,"Reversed E",VOWEL,"Central","Mid","Unrounded","",INDEPENDENT);
    AddChar(201,400,"Superior m",DIACRITIC,"Undefined","Undefined","Undefined","Undefined",POSTFIX);
    AddChar(203,400,"Superscript left-tail N (at left)",DIACRITIC,"Undefined","Undefined","Undefined","Undefined",POSTFIX);
    AddChar(212,400,"Superscript Eng",DIACRITIC,"Undefined","Undefined","Undefined","Undefined",POSTFIX);
    AddChar(39,401,"Apostrophe",DIACRITIC,"Ejective","","","",POSTFIX);
    AddChar(42,402,"Over-ring (o-width)",DIACRITIC,"Voiceless","","","",POSTFIX);
    AddChar(56,402,"Under-ring (o-width)",DIACRITIC,"Voiceless","","","",POSTFIX);
    AddChar(161,402,"Over Ring (i-width)",DIACRITIC,"Voiceless","","","",POSTFIX);
    AddChar(164,403,"Subscript Wedge",DIACRITIC,"Voiced","","","",POSTFIX);
    AddChar(72,404,"Superscript H",DIACRITIC,"Aspirated","","","",POSTFIX);
    AddChar(45,405,"Subscript Umlaut (o-width)",DIACRITIC,"Breathy voice","","","",POSTFIX);
    AddChar(188,406,"Subscript Tilde (i-width)",DIACRITIC,"Creaky voice","","","",POSTFIX);
    AddChar(209,407,"Subscript Seagull",DIACRITIC,"Linguolabial","","","",POSTFIX);
    AddChar(48,408,"Subscript Bridge",DIACRITIC,"Dental","","","",POSTFIX);
    AddChar(176,409,"Inverted Subscript Bridge",DIACRITIC,"Apical","","","",POSTFIX);
    AddChar(54,410,"Subscript Square",DIACRITIC,"Laminal","","","",POSTFIX);
    AddChar(166,411,"Subscript Right Half-ring",DIACRITIC,"More rounded","","","",POSTFIX);
    AddChar(55,412,"Subscript Left Half-ring",DIACRITIC,"Less rounded","","","",POSTFIX);
    AddChar(43,413,"Subscript Plus (o-width)",DIACRITIC,"Advanced","","","",POSTFIX);
    AddChar(61,414,"Under-bar (o-width)",DIACRITIC,"Retracted","","","",POSTFIX);
    AddChar(95,415,"Umlaut",DIACRITIC,"Centralized","","","",POSTFIX);
    AddChar(126,416,"Over-cross",DIACRITIC,"Mid-Centralized","","","",POSTFIX);
    AddChar(193,417,"Advancing Sign (i-width)",DIACRITIC,"Advanced Tongue Root","","","",POSTFIX);
    AddChar(170,418,"Retracting Sign (i-width)",DIACRITIC,"Retracted Tongue Root","","","",POSTFIX);
    AddChar(213,419,"Right Hook",DIACRITIC,"Rhoticity","","","",POSTFIX);
    AddChar(87,420,"Superscript W",DIACRITIC,"Labialized","","","",POSTFIX);
    AddChar(74,421,"Superscript J",DIACRITIC,"Palatalized","","","",POSTFIX);
    AddChar(236,422,"Superscript Gamma",DIACRITIC,"Velarized","","","",POSTFIX);
    AddChar(179,423,"Superscript Reversed Glottal Stop",DIACRITIC,"Pharangealized","","","",POSTFIX);
    AddChar(41,424,"Superscript Tilde (o-width)",DIACRITIC,"Nasalized","","","",POSTFIX);
    AddChar(60,425,"Superscript N",DIACRITIC,"Nasal release","","","",POSTFIX);
    AddChar(58,426,"Superscript L",DIACRITIC,"Lateral release","","","",POSTFIX);
    AddChar(124,427,"Corner",DIACRITIC,"No audible release","","","",POSTFIX);
    AddChar(242,428,"Superimposed Tilde",DIACRITIC,"Velarized or Pharangealized","","","",POSTFIX);
    AddChar(163,429,"Raising Sign (i-width)",DIACRITIC,"Raised","","","",POSTFIX);
    AddChar(162,430,"Lowering Sign (i-width)",DIACRITIC,"Lowered","","","",POSTFIX);
    AddChar(96,431,"Syllabicity Mark",DIACRITIC,"Syllabic","","","",POSTFIX);
    AddChar(57,432,"Subscript Arch (o-width)",DIACRITIC,"Non-syllabic","","","",POSTFIX);
    AddChar(200,501,"Vertical Stroke (Superior)",SUPRASEGMENTAL,"Primary Stress","","","",PREFIX);
    AddChar(199,502,"Vertical Stroke (Inferior)",SUPRASEGMENTAL,"Secondary Stress","","","",PREFIX);
    AddChar(249,503,"Length Mark",SUPRASEGMENTAL,"Long","","","",POSTFIX);
    AddChar(62,504,"Half-length Mark",SUPRASEGMENTAL,"Half-Long","","","",POSTFIX);
    AddChar(225,505,"Breve (i-width)",SUPRASEGMENTAL,"Extra-Short","","","",POSTFIX);
    AddChar(46,506,"Period",SUPRASEGMENTAL,"Syllable BREAK","","","",BREAK);
    AddChar(150,507,"Vertical Line",SUPRASEGMENTAL,"Minor (foot) group","","","",BREAK);
    AddChar(132,508,"Double Vertical Line",SUPRASEGMENTAL,"Major (intonation) group","","","",BREAK);
    AddChar(131,509,"Top Tie Bar",SUPRASEGMENTAL,"Linking","","","",LINK);
    AddChar(233,509,"Long Bottom Tie Bar",SUPRASEGMENTAL,"Linking","","","",LINK);
    AddChar(204,510,"Upward Diagonal Arrow",SUPRASEGMENTAL,"Accent","Global Rise","","",POSTFIX);
    AddChar(205,511,"Downward Diagonal Arrow",SUPRASEGMENTAL,"Accent","Global Fall","","",POSTFIX);
    AddChar(155,517,"Down Arrow",SUPRASEGMENTAL,"Accent","Downstep","","",POSTFIX);
    AddChar(139,518,"Up Arrow",SUPRASEGMENTAL,"Accent","Upstep","","",POSTFIX);
    AddChar(49,601,"Superscript 1",SUPRASEGMENTAL,"Tone","1","","",POSTFIX);
    AddChar(158,601,"Double Grave Accent (high i-width)",SUPRASEGMENTAL,"Tone","1","","",POSTFIX);
    AddChar(159,601,"Extra-low Tone Bar",SUPRASEGMENTAL,"Tone","1","","",POSTFIX);
    AddChar(50,602,"Superscript 2",SUPRASEGMENTAL,"Tone","2","","",POSTFIX);
    AddChar(153,602,"Grave Accent (high i-width)",SUPRASEGMENTAL,"Tone","2","","",POSTFIX);
    AddChar(154,602,"Low Tone Bar",SUPRASEGMENTAL,"Tone","2","","",POSTFIX);
    AddChar(51,603,"Superscript 3",SUPRASEGMENTAL,"Tone","3","","",POSTFIX);
    AddChar(148,603,"Macron (high i-width)",SUPRASEGMENTAL,"Tone","3","","",POSTFIX);
    AddChar(149,603,"Mid Tone Bar",SUPRASEGMENTAL,"Tone","3","","",POSTFIX);
    AddChar(52,604,"Superscript 4",SUPRASEGMENTAL,"Tone","4","","",POSTFIX);
    AddChar(143,604,"Acute Accent (high o-width)",SUPRASEGMENTAL,"Tone","4","","",POSTFIX);
    AddChar(145,604,"High Tone Bar",SUPRASEGMENTAL,"Tone","4","","",POSTFIX);
    AddChar(53,605,"Superscript 5",SUPRASEGMENTAL,"Tone","5","","",POSTFIX);
    AddChar(137,605,"Double Acute Accent (high i-width)",SUPRASEGMENTAL,"Tone","5","","",POSTFIX);
    AddChar(138,605,"Extra-high Tone Bar",SUPRASEGMENTAL,"Tone","5","","",POSTFIX);
    AddChar(128,613,"Right Bar 13",SUPRASEGMENTAL,"Accent","13","","",POSTFIX);
    AddChar(136,613,"Grave-macron accent",SUPRASEGMENTAL,"Accent","13","","",POSTFIX);
    AddChar(232,615,"Right Bar 15",SUPRASEGMENTAL,"Accent","15","","",POSTFIX);
    AddChar(243,615,"Wedge (high i-width)",SUPRASEGMENTAL,"Accent","15","","",POSTFIX);
    AddChar(217,631,"Right Bar 31",SUPRASEGMENTAL,"Accent","31","","",POSTFIX);
    AddChar(157,635,"Macron-Acute",SUPRASEGMENTAL,"Accent","35","","",POSTFIX);
    AddChar(216,635,"Right Bar 35",SUPRASEGMENTAL,"Accent","35","","",POSTFIX);
    AddChar(134,651,"Right Bar 51",SUPRASEGMENTAL,"Accent","51","","",POSTFIX);
    AddChar(230,651,"Circumflex (high i-width)",SUPRASEGMENTAL,"Accent","51","","",POSTFIX);
    AddChar(133,653,"Right Bar 53",SUPRASEGMENTAL,"Accent","53","","",POSTFIX);
    AddChar(40,694,"Contour Mark",SUPRASEGMENTAL,"Accent","454","","",POSTFIX);
    AddChar(202,694,"Right Bar 454",SUPRASEGMENTAL,"Accent","454","","",POSTFIX);
    AddChar(91,901,"Left Square Bracket",PUNCTUATION,"Left Square Bracket","","","",BREAK);
    AddChar(93,902,"Right Square Bracket",PUNCTUATION,"Right Square Bracket","","","",BREAK);
    AddChar(32,903,"Space",PUNCTUATION,"Space","","","",BREAK);
    AddChar(33,903,"Space",PUNCTUATION,"Space","","","",BREAK);
    AddChar(34,903,"Space",PUNCTUATION,"Space","","","",BREAK);
    AddChar(35,904,"Pound",PUNCTUATION,"Pound","","","",BREAK);
    AddChar(36,905,"Less Than",PUNCTUATION,"Less Than","","","",BREAK);
    AddChar(37,906,"Greater Than",PUNCTUATION,"Greater Than","","","",BREAK);
    AddChar(47,907,"Forward Slash",PUNCTUATION,"Forward Slash","","","",BREAK);
    AddChar(64,908,"Question Mark",PUNCTUATION,"Question Mark","","","",BREAK);
    AddChar(92,909,"Backward Slash",PUNCTUATION,"Backward Slash","","","",BREAK);
    AddChar(144,910,"Star",PUNCTUATION,"Star","","","",BREAK);
    AddChar(44,911,"Comma",PUNCTUATION,"Comma","","","",BREAK);
    AddChar(214,912,"Hyphen Dash",PUNCTUATION,"Hyphen Dash","","","",BREAK);
    AddChar(94,913,"Underbar",PUNCTUATION,"Underbar","","","",BREAK);
    AddChar(237,914,"Underscore Short",PUNCTUATION,"Underscore Short","","","",BREAK);
};

/***************************************************************************/
// CFontTableIPA::GetNext Get next unit of string in words, characters, bytes
// updates nIndex to point to remainder of string for ASAP IPA fonts
/***************************************************************************/
CString CFontTableIPA::GetNext(tUnit nInUnits, int & nIndex, const CString & szString) const {
    CString szReturn = "";

    if ((nIndex < 0) || (nIndex >= szString.GetLength())) {
        return szReturn;
    }

    switch (nInUnits) {
    case BYTE:
        return szString[nIndex++];
        break;
    case CHARACTER: {
        CString szWorking = "";
        CString szPostfix = "";
        CString szPrefix = "";
        BOOL bIndependent = FALSE;
        BOOL bAppendPostfix = FALSE;
        BOOL bAppendPrefix = TRUE;

        int nIndependent = 0;
        int nWrkIndex;
        for (nWrkIndex = nIndex; nWrkIndex < szString.GetLength(); nWrkIndex++) {
            switch (GlyphType(szString[nWrkIndex])) {
            case ENDofSTRING:
                if (bIndependent==FALSE) {
                    if (nIndex == 0) {
                        szWorking+=szPostfix;
                    }
                    nIndex = nWrkIndex;
                    return szWorking;
                } else {
                    nIndex = nIndependent + 1;
                    szWorking += szPostfix;
                    return szWorking;
                }
                break;
            case INDEPENDENT:
            case BREAK:
                if ((bIndependent == FALSE) && (szPostfix.IsEmpty() || (nIndex>0))) {
                    bIndependent = TRUE;
                    nIndependent = nWrkIndex;
                    szWorking += szString[nWrkIndex];
                    szPostfix = "";
                    bAppendPostfix = TRUE;
                    bAppendPrefix = FALSE;
                } else {
                    nIndex = nIndependent + 1;
                    szWorking += szPostfix;
                    return szWorking;
                }
                break;
            case PREFIX:
                if (bAppendPrefix) {
                    szWorking += szString[nWrkIndex];
                } else {
                    szPrefix += szString[nWrkIndex];
                }
                break;
            case POSTFIX:
                if (bAppendPostfix) {
                    szWorking += szString[nWrkIndex];
                } else {
                    szPostfix += szString[nWrkIndex];
                }
                break;
            case LINK:
                bIndependent = FALSE;
                bAppendPrefix = TRUE;  // Include prefix for linked character
                szWorking += szPrefix;
                szPrefix = "";
                szWorking += szString[nWrkIndex]; // Insert link character
                break;
            }
        }

        if (bIndependent==FALSE) {
            if (nIndex == 0) {
                szWorking+=szPostfix;
            }
            nIndex = nWrkIndex;
            return szWorking;
        } else {
            nIndex = nIndependent + 1;
            szWorking += szPostfix;
            return szWorking;
        }
        return szString[nIndex++];
    }
    break;
    case DELIMITEDWORD: {
        szReturn = szString.Mid(nIndex);
        int nDelimiter = szReturn.Find(m_wordDelimiter);

        if (nDelimiter != -1) {
            szReturn = szReturn.Mid(0, nDelimiter);
            nIndex += nDelimiter + 1;
        } else {
            nIndex = szString.GetLength();
        }
        return szReturn;
    }
    break;
    case NONE:
    default:
        szReturn = szString.Mid(nIndex);
        nIndex = szString.GetLength();
        RemoveWordDelimiters(szReturn); // SDM 1.5Test8.2
        return szReturn;
    }
};

/***************************************************************************/
// CFontTableANSI::GetNext Get next unit of string in words, characters, bytes
// updates nIndex to point to remainder of string for ANSI access codes
/***************************************************************************/
CString CFontTableANSI::GetNext(tUnit nInUnits, int & nIndex, const CString & szString) const {
    CString szReturn = "";

    if ((nIndex < 0) || (nIndex >= szString.GetLength())) {
        return szReturn;
    }

    switch (nInUnits) {
    case BYTE:
        return szString[nIndex++];
        break;
    case CHARACTER:
        return szString[nIndex++];
        break;
    case DELIMITEDWORD: {
        szReturn = szString.Mid(nIndex);
        int nDelimiter = szReturn.Find(m_wordDelimiter);
        if (nDelimiter != -1) {
            szReturn = szReturn.Mid(0, nDelimiter);
            nIndex += nDelimiter + 1;
        } else {
            nIndex = szString.GetLength();
        }
        return szReturn;
    }
    break;
    case NONE:
    default:
        szReturn = szString.Mid(nIndex);
        nIndex = szString.GetLength();
        //      RemoveWordDelimiters(szReturn);
        return szReturn;
    }
};

