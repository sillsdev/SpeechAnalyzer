/////////////////////////////////////////////////////////////////////////////
// ch_table.cpp:
// Implementation of the CCharTable
//             CIPATable  classes.
// Author: Urs Ruchti
// copyright 1997 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ch_table.h"
#include "ch_dlwnd.h"
#include "resource.h"
#include "CSaString.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

//###########################################################################
// CCharTable
// Character table base class. Sets up and maintains a character table for
// the character chart. The characters are assigned to the static dialog
// windows.

/////////////////////////////////////////////////////////////////////////////
// CCharTable construction/destruction/creation

/***************************************************************************/
// CCharTable::CCharTable Constructor
/***************************************************************************/
CCharTable::CCharTable()
{

    m_pFont = NULL;
    m_pCaller = NULL;
    m_apParent[VOWEL] = NULL;
    m_apParent[CONSONANT] = NULL;
    m_apParent[DIACRITIC] = NULL;
    m_apParent[SUPRASEG] = NULL;
}

/***************************************************************************/
// CCharTable::~CCharTable Destructor
/***************************************************************************/
CCharTable::~CCharTable()
{

    CleanUp();
}

/////////////////////////////////////////////////////////////////////////////
// CCharTable helper functions

/***************************************************************************/
// CCharTable::CleanUp
/***************************************************************************/
void CCharTable::CleanUp()
{

    // delete all the objects in the array and empty the array
    int nArraySize = m_CharOb.GetCount();
    for (int nLoop = 0; nLoop < nArraySize; nLoop++)
    {
        if (m_CharOb.GetAt(nLoop))
        {
            delete m_CharOb.GetAt(nLoop);
        }
    }
    m_CharOb.RemoveAll();
}

/***************************************************************************/
// CCharTable::AddChar Adds a character to the table
/***************************************************************************/
void CCharTable::AddChar(LPSTR pszChar, LPSTR pszSoundFile, UINT nID, int nType, int nMode)
{

    CChartChar * pChar = new CChartChar;
    CSaString szTemp, szTemp2;
    szTemp.setUtf8(pszChar);
    szTemp2 = pszSoundFile;
    pChar->Setup(&szTemp, &szTemp2, nID, nType, nMode, m_pFont, m_apParent[nType], m_pCaller);
    m_CharOb.Add(pChar);
}

/***************************************************************************/
// CCharTable::SelectChars Select characters from the table
// The pointer pszChars points to a string of characters, that should change
// their display mode to nMode. If nType contains -1, all types will be
// affected, otherwise only the type given. Only the characters that are not
// ICH_DISABLED will change their mode. If pszChars contains NULL, all the
// characters (but not the ICH_DISABLED ones) will changes the mode.
/***************************************************************************/
void CCharTable::SelectChars(CString * pszChars, int nMode, int nType)
{

    int nSearchLoopLimit = 0;
    if (pszChars)
    {
        nSearchLoopLimit = pszChars->GetLength();
    }
    int nArraySize = m_CharOb.GetCount();
    for (int nLoop = 0; nLoop < nArraySize; nLoop++)
    {
        CChartChar * pChar = (CChartChar *)m_CharOb.GetAt(nLoop);
        if (pChar)
        {
            // check if type correct
            if ((nType == -1) || (pChar->GetType() == nType))
            {
                // check if character disabled
                if ((pChar->GetMode() & ICH_DISABLED) == 0)
                {
                    // check character string against search string
                    CString * pszChar = pChar->GetChar();
                    int nSearchLoop = 0;
                    for (nSearchLoop; nSearchLoop < nSearchLoopLimit; nSearchLoop++)
                    {
                        CString szSearch = pszChars->Mid(nSearchLoop, 1);
                        if (*pszChar == szSearch)
                        {
                            break;    // this is it
                        }
                    }
                    if ((nSearchLoop < nSearchLoopLimit) || (!nSearchLoopLimit))
                    {
                        pChar->ChangeMode(nMode);
                    }
                }
            }
        }
    }
}

/***************************************************************************/
// CCharTable::InitPage Initializes a page of characters
/***************************************************************************/
void CCharTable::InitPage(int nType)
{

    int nArraySize = m_CharOb.GetCount();
    for (int nLoop = 0; nLoop < nArraySize; nLoop++)
    {
        CChartChar * pChar = (CChartChar *)m_CharOb.GetAt(nLoop);
        if (pChar->GetType() == nType)
        {
            pChar->Init();
        }
    }
}

//###########################################################################
// CIPATable
// IPA character table class. Sets up and maintains a character table for the
// character chart based on the IPA font.

/////////////////////////////////////////////////////////////////////////////
// CIPATable construction/destruction/creation

/***************************************************************************/
// CIPATable::CIPATable Constructor
/***************************************************************************/
CIPATable::CIPATable()
{

}

/***************************************************************************/
// CIPATable::~CIPATable Destructor
/***************************************************************************/
CIPATable::~CIPATable()
{

}

/////////////////////////////////////////////////////////////////////////////
// CIPATable helper functions

/***************************************************************************/
// CIPATable::SetupTable Sets up a character table
// The parameter pFont contains a pointer to the font for the characters.
// pWCaller points to the window (dialog), that wants to receive the messages
// from the characters. The other pointers point to the parent dialog pages.
/***************************************************************************/

#define _AC(a,b) b

void CIPATable::SetupTable(CFont * pFont, CDlgCharChart * pCaller, CWnd * pVowelPage,
                           CWnd * pConsPage, CWnd * pDiacPage, CWnd * pSupraPage)
{

    if (pFont && pCaller && pVowelPage && pConsPage && pDiacPage)
    {
        m_pFont = pFont;
        m_pCaller = pCaller;
        m_apParent[VOWEL] = pVowelPage;
        m_apParent[CONSONANT] = pConsPage;
        m_apParent[DIACRITIC] = pDiacPage;
        m_apParent[SUPRASEG] = pSupraPage;
        CleanUp();
        // add all the characters for your table
        AddChar(_AC("i","i"), "VOW-00W", IDC_CHART_CV0, VOWEL);
        AddChar(_AC("e","e"), "VOW-09W", IDC_CHART_CV1, VOWEL);
        AddChar(_AC("E","ɛ"), "VOW-16W", IDC_CHART_CV2, VOWEL);
        AddChar(_AC("Q","æ"), "VOW-22W", IDC_CHART_CV3, VOWEL);
        AddChar(_AC("a","a"), "VOW-24W", IDC_CHART_CV4, VOWEL);
        AddChar(_AC("y","y"), "VOW-01W", IDC_CHART_CV100, VOWEL);
        AddChar(_AC("I","ɪ"), "VOW-06W", IDC_CHART_CV101, VOWEL);
        AddChar(_AC("Y","ʏ"), "VOW-07W", IDC_CHART_CV102, VOWEL);
        AddChar(_AC("O","ø"), "VOW-10W", IDC_CHART_CV103, VOWEL);
        AddChar(_AC("\xBF","œ"), "VOW-17W", IDC_CHART_CV104, VOWEL);
        AddChar(_AC("\xAF","ɶ"), "VOW-25W", IDC_CHART_CV105, VOWEL);
        AddChar(_AC("\xF6","ɨ"), "VOW-02W", IDC_CHART_CV200, VOWEL);
        AddChar(_AC("\x82","ɘ"), "VOW-11W", IDC_CHART_CV201, VOWEL);
        AddChar(_AC("\xCE","ɜ"), "VOW-18W", IDC_CHART_CV202, VOWEL);
        AddChar(_AC("\xAB","ə"), "VOW-15W", IDC_CHART_CV250, VOWEL);
        AddChar(_AC("\x8C","ɐ"), "VOW-23W", IDC_CHART_CV251, VOWEL);
        AddChar(_AC("\xAC","ʉ"), "VOW-03W", IDC_CHART_CV300, VOWEL);
        AddChar(_AC("P","ɵ"), "VOW-12W", IDC_CHART_CV301, VOWEL);
        AddChar(_AC("\xCF","ɞ"), "VOW-19W", IDC_CHART_CV302, VOWEL);
        AddChar(_AC("\xB5","ɯ"), "VOW-04W", IDC_CHART_CV400, VOWEL);
        AddChar(_AC("U","ʊ"), "VOW-08W", IDC_CHART_CV401, VOWEL);
        AddChar(_AC("F","ɤ"), "VOW-13W", IDC_CHART_CV402, VOWEL);
        AddChar(_AC("\xC3","ʌ"), "VOW-20W", IDC_CHART_CV403, VOWEL);
        AddChar(_AC("A","ɑ"), "VOW-26W", IDC_CHART_CV404, VOWEL);
        AddChar(_AC("u","u"), "VOW-05W", IDC_CHART_CV500, VOWEL);
        AddChar(_AC("o","o"), "VOW-14W", IDC_CHART_CV501, VOWEL);
        AddChar(_AC("\x8D","ɔ"), "VOW-21W", IDC_CHART_CV502, VOWEL);
        AddChar(_AC("\x81","ɒ"), "VOW-27W", IDC_CHART_CV503, VOWEL);
        // Consonants pulmonic
        AddChar(_AC("p","p"), "CON-00B", IDC_CHART_CV0, CONSONANT); // bilabial
        AddChar(_AC("\xB8","ɸ"), "CON-25B", IDC_CHART_CV1, CONSONANT);
        AddChar(_AC("b","b"), "CON-01B", IDC_CHART_CV10, CONSONANT);
        AddChar(_AC("m","m"), "CON-13B", IDC_CHART_CV11, CONSONANT);
        AddChar(_AC("\xF5","ʙ"), "CON-20B", IDC_CHART_CV12, CONSONANT);
        AddChar(_AC("B","β"), "CON-26B", IDC_CHART_CV13, CONSONANT);
        AddChar(_AC("f","f"), "CON-27B", IDC_CHART_CV100, CONSONANT); // labiodental
        AddChar(_AC("M","ɱ"), "CON-14B", IDC_CHART_CV110, CONSONANT);
        AddChar(_AC(" ",""), "", IDC_CHART_CV111, CONSONANT);
        AddChar(_AC("v","v"), "CON-28B", IDC_CHART_CV112, CONSONANT);
        AddChar(_AC("V","ʋ"), "CON-49B", IDC_CHART_CV113, CONSONANT);
        AddChar(_AC("T","θ"), "CON-29B", IDC_CHART_CV400, CONSONANT); // dental
        AddChar(_AC("D","ð"), "CON-30B", IDC_CHART_CV410, CONSONANT);
        AddChar(_AC("t","t"), "CON-02B", IDC_CHART_CV500, CONSONANT); // alveolar
        AddChar(_AC("s","s"), "CON-31B", IDC_CHART_CV501, CONSONANT);
        AddChar(_AC("\xC2","ɬ"), "CON-47B", IDC_CHART_CV502, CONSONANT);
        AddChar(_AC("d","d"), "CON-03B", IDC_CHART_CV510, CONSONANT);
        AddChar(_AC("n","n"), "CON-15B", IDC_CHART_CV511, CONSONANT);
        AddChar(_AC("r","r"), "CON-21B", IDC_CHART_CV512, CONSONANT);
        AddChar(_AC("R","ɾ"), "CON-23B", IDC_CHART_CV513, CONSONANT);
        AddChar(_AC("z","z"), "CON-32B", IDC_CHART_CV514, CONSONANT);
        AddChar(_AC("L","ɮ"), "CON-48B", IDC_CHART_CV515, CONSONANT);
        AddChar(_AC("\xA8","ɹ"), "CON-50B", IDC_CHART_CV516, CONSONANT);
        AddChar(_AC("l","l"), "CON-54B", IDC_CHART_CV517, CONSONANT);
        AddChar(_AC("S","ʃ"), "CON-33B", IDC_CHART_CV600, CONSONANT); // postalveolar
        AddChar(_AC("Z","ʒ"), "CON-34B", IDC_CHART_CV610, CONSONANT);
        AddChar(_AC("\xFF","ʈ"), "CON-04B", IDC_CHART_CV700, CONSONANT); // retroflex
        AddChar(_AC("\xA7","ʂ"), "CON-35B", IDC_CHART_CV701, CONSONANT);
        AddChar(_AC("\xEA","ɖ"), "CON-05B", IDC_CHART_CV710, CONSONANT);
        AddChar(_AC("\xF7","ɳ"), "CON-16B", IDC_CHART_CV711, CONSONANT);
        AddChar(_AC("}","ɽ"), "CON-24B", IDC_CHART_CV712, CONSONANT);
        AddChar(_AC("\xBD","ʐ"), "CON-36B", IDC_CHART_CV713, CONSONANT);
        AddChar(_AC("\xD3","ɻ"), "CON-51B", IDC_CHART_CV714, CONSONANT);
        AddChar(_AC("\xF1","ɭ"), "CON-55B", IDC_CHART_CV715, CONSONANT);
        AddChar(_AC("c","c"), "CON-06B", IDC_CHART_CV800, CONSONANT); // palatal
        AddChar(_AC("C","ç"), "CON-37B", IDC_CHART_CV801, CONSONANT);
        AddChar(_AC("\xEF","ɟ"), "CON-07B", IDC_CHART_CV810, CONSONANT);
        AddChar(_AC("\xF8","ɲ"), "CON-17B", IDC_CHART_CV811, CONSONANT);
        AddChar(_AC("\xC6","ʝ"), "CON-38B", IDC_CHART_CV812, CONSONANT);
        AddChar(_AC("j","j"), "CON-52B", IDC_CHART_CV813, CONSONANT);
        AddChar(_AC("\xB4","ʎ"), "CON-56B", IDC_CHART_CV814, CONSONANT);
        AddChar(_AC("k","k"), "CON-08B", IDC_CHART_CV900, CONSONANT); // velar
        AddChar(_AC("x","x"), "CON-39B", IDC_CHART_CV901, CONSONANT);
        AddChar(_AC("g","ɡ"), "CON-09B", IDC_CHART_CV910, CONSONANT);
        AddChar(_AC("N","ŋ"), "CON-18B", IDC_CHART_CV911, CONSONANT);
        AddChar(_AC("\xC4","ɣ"), "CON-40B", IDC_CHART_CV912, CONSONANT);
        AddChar(_AC("\xE5","ɰ"), "CON-53B", IDC_CHART_CV913, CONSONANT);
        AddChar(_AC(";","ʟ"), "CON-57B", IDC_CHART_CV914, CONSONANT);
        AddChar(_AC("q","q"), "CON-10B", IDC_CHART_CV1000, CONSONANT); // uvular
        AddChar(_AC("X","χ"), "CON-41B", IDC_CHART_CV1001, CONSONANT);
        AddChar(_AC("G","ɢ"), "CON-11B", IDC_CHART_CV1010, CONSONANT);
        AddChar(_AC("\xB2","ɴ"), "CON-19B", IDC_CHART_CV1011, CONSONANT);
        AddChar(_AC("{","ʀ"), "CON-22B", IDC_CHART_CV1012, CONSONANT);
        AddChar(_AC("\xD2","ʁ"), "CON-42B", IDC_CHART_CV1013, CONSONANT);
        AddChar(_AC("\xF0","ħ"), "CON-43B", IDC_CHART_CV1100, CONSONANT); // pharyngeal
        AddChar(_AC("\xC0","ʕ"), "CON-44B", IDC_CHART_CV1110, CONSONANT);
        AddChar(_AC("?","ʔ"), "CON-12B", IDC_CHART_CV1200, CONSONANT); // glottal
        AddChar(_AC("h","h"), "CON-45B", IDC_CHART_CV1201, CONSONANT);
        AddChar(_AC("\xFA","ɦ"), "CON-46B", IDC_CHART_CV1210, CONSONANT);
        // Consonants non-pulmonic
        AddChar(_AC("\x87","ʘ"), "CON-58B", IDC_CHART_CV50, CONSONANT); // clicks
        AddChar(_AC("\x96","ǀ"), "CON-59B", IDC_CHART_CV51, CONSONANT);
        AddChar(_AC("\x97","ǃ"), "CON-60B", IDC_CHART_CV52, CONSONANT);
        AddChar(_AC("\x9C","ǂ"), "CON-61B", IDC_CHART_CV53, CONSONANT);
        AddChar(_AC("\x84","ǁ"), "CON-62B", IDC_CHART_CV54, CONSONANT);
        AddChar(_AC("\xBA","ɓ"), "CON-63B", IDC_CHART_CV150, CONSONANT); // voiced implosives
        AddChar(_AC("\xEB","ɗ"), "CON-64B", IDC_CHART_CV151, CONSONANT);
        AddChar(_AC("\xD7","ʄ"), "CON-65B", IDC_CHART_CV152, CONSONANT);
        AddChar(_AC("\xA9","ɠ"), "CON-66B", IDC_CHART_CV153, CONSONANT);
        AddChar(_AC("\xFD","ʛ"), "CON-67B", IDC_CHART_CV154, CONSONANT);
        AddChar(_AC("\x27","ʼ"), NULL, IDC_CHART_CV250, CONSONANT); // ejectives
        AddChar(_AC("p\'","pʼ"),  "CON-68B", IDC_CHART_CV251, CONSONANT, ICH_DISABLED);
        AddChar(_AC("t\'","tʼ"),  "CON-69B", IDC_CHART_CV252, CONSONANT, ICH_DISABLED);
        AddChar(_AC("k\'","kʼ"),  "CON-70B", IDC_CHART_CV253, CONSONANT, ICH_DISABLED);
        AddChar(_AC("s\'","sʼ"),  NULL, IDC_CHART_CV254, CONSONANT, ICH_DISABLED);
        // Consonants other symbols
        AddChar(_AC("\xE3","ʍ"), "CON-72B", IDC_CHART_CV350, CONSONANT);
        AddChar(_AC("w","w"), "CON-73B", IDC_CHART_CV351, CONSONANT);
        AddChar(_AC("\xE7","ɥ"), "CON-74B", IDC_CHART_CV352, CONSONANT);
        AddChar(_AC("K","ʜ"), "CON-75B", IDC_CHART_CV353, CONSONANT);
        AddChar(_AC("\xB9","ʢ"), "CON-76B", IDC_CHART_CV354, CONSONANT);
        AddChar(_AC("\xFB","ʡ"), "CON-77B", IDC_CHART_CV355, CONSONANT);
        AddChar(_AC("\xFE","ɕ"), "CON-78B", IDC_CHART_CV450, CONSONANT);
        AddChar(_AC("\xFC","ʑ"), "CON-79B", IDC_CHART_CV451, CONSONANT);
        AddChar(_AC("\xE4","ɺ"), "CON-80B", IDC_CHART_CV452, CONSONANT);
        AddChar(_AC("\xEE","ɧ"), "CON-81B", IDC_CHART_CV453, CONSONANT);
        AddChar(_AC("S","ʃ"), NULL, IDC_CHART_CV454, CONSONANT, ICH_DISABLED);
        AddChar(_AC("x","x"), NULL, IDC_CHART_CV455, CONSONANT, ICH_DISABLED);
        AddChar(_AC("k\x83p","k͡p"), "CON-82B", IDC_CHART_CV458, CONSONANT, ICH_DISABLED);
        AddChar(_AC("t\xE9s","t͜s"), "CON-83B", IDC_CHART_CV459, CONSONANT, ICH_DISABLED);
        // Diacritics
        AddChar(_AC("N","ŋ"), NULL, IDC_CHART_CV0, DIACRITIC, ICH_GRAYED);
        AddChar(_AC("8","̥"), NULL, IDC_CHART_CV0_DIACRITIC, DIACRITIC, ICH_GRAYED);
        AddChar(_AC("8","̥"),  NULL, IDC_CHART_CV1, DIACRITIC);
        AddChar(_AC("n8","n̥"), "DIA-00B", IDC_CHART_CV100, DIACRITIC, ICH_DISABLED);
        AddChar(_AC("d8","d̥"), "DIA-01B", IDC_CHART_CV200, DIACRITIC, ICH_DISABLED);
        AddChar(_AC("\xA4","̬"),  NULL, IDC_CHART_CV2, DIACRITIC);
        AddChar(_AC("s\xA4","s̬"), "DIA-02B", IDC_CHART_CV101, DIACRITIC, ICH_DISABLED);
        AddChar(_AC("t\xA4","t̬"), "DIA-03B", IDC_CHART_CV201, DIACRITIC, ICH_DISABLED);
        AddChar(_AC("H","ʰ"),  NULL, IDC_CHART_CV3, DIACRITIC);
        AddChar(_AC("tH","tʰ"), "DIA-04B", IDC_CHART_CV102, DIACRITIC, ICH_DISABLED);
        AddChar(_AC("dH","dʰ"), "DIA-05B", IDC_CHART_CV202, DIACRITIC, ICH_DISABLED);
        AddChar(_AC("�","̹"),  NULL, IDC_CHART_CV4, DIACRITIC);
        AddChar(_AC("\x8D","ɔ̹"), "DIA-06A", IDC_CHART_CV103, DIACRITIC, ICH_DISABLED);
        AddChar(_AC("7","̜"),  NULL, IDC_CHART_CV5, DIACRITIC);
        AddChar(_AC("\x8D" "7","ɔ" "̜"), "DIA-07A", IDC_CHART_CV104, DIACRITIC, ICH_DISABLED);
        AddChar(_AC("+","̟"),  NULL, IDC_CHART_CV6, DIACRITIC);
        AddChar(_AC("u+","u̟"), "DIA-08A", IDC_CHART_CV105, DIACRITIC, ICH_DISABLED);
        AddChar(_AC("=","̠"),  NULL, IDC_CHART_CV7, DIACRITIC);
        AddChar(_AC("e=","e̠"), "DIA-09A", IDC_CHART_CV106, DIACRITIC, ICH_DISABLED);
        AddChar(_AC("_","̈"),  NULL, IDC_CHART_CV8, DIACRITIC);
        AddChar(_AC("e_","ë"), "DIA-10A", IDC_CHART_CV107, DIACRITIC, ICH_DISABLED);
        AddChar(_AC("~","̽"),  NULL, IDC_CHART_CV9, DIACRITIC);
        AddChar(_AC("e~","e̽"), "DIA-11A", IDC_CHART_CV108, DIACRITIC, ICH_DISABLED);
        AddChar(_AC("`","̩"),  NULL, IDC_CHART_CV10, DIACRITIC);
        AddChar(_AC("n`","n̩"), "DIA-12A", IDC_CHART_CV109, DIACRITIC, ICH_DISABLED);
        AddChar(_AC("9","̯"),  NULL, IDC_CHART_CV11, DIACRITIC);
        AddChar(_AC("e9","e̯"), "DIA-13A", IDC_CHART_CV110, DIACRITIC, ICH_DISABLED);
        AddChar(_AC("\xD5","˞"),  NULL, IDC_CHART_CV12, DIACRITIC);
        AddChar(_AC("P\xD5","ɵ˞"), "DIA-14A", IDC_CHART_CV111, DIACRITIC, ICH_DISABLED);
        AddChar(_AC("a\xD5","a˞"), NULL, IDC_CHART_CV203, DIACRITIC, ICH_DISABLED);
        AddChar(_AC("-","̤"),  NULL, IDC_CHART_CV300, DIACRITIC);
        AddChar(_AC("b-","b̤"), "DIA-15B", IDC_CHART_CV400, DIACRITIC, ICH_DISABLED);
        AddChar(_AC("a-","a̤"), "DIA-16A", IDC_CHART_CV500, DIACRITIC, ICH_DISABLED);
        AddChar(_AC("\xBC","̰"),  NULL, IDC_CHART_CV301, DIACRITIC);
        AddChar(_AC("b\xBC","b̰"), "DIA-17B", IDC_CHART_CV401, DIACRITIC, ICH_DISABLED);
        AddChar(_AC("a\xBC","a̰"), "DIA-18A", IDC_CHART_CV501, DIACRITIC, ICH_DISABLED);
        AddChar(_AC("\xD1","̼"),  NULL, IDC_CHART_CV302, DIACRITIC);
        AddChar(_AC("t\xD1","t̼"), "DIA-19B", IDC_CHART_CV402, DIACRITIC, ICH_DISABLED);
        AddChar(_AC("d\xD1","d̼"), "DIA-20B", IDC_CHART_CV502, DIACRITIC, ICH_DISABLED);
        AddChar(_AC("W","ʷ"),  NULL, IDC_CHART_CV303, DIACRITIC);
        AddChar(_AC("tW","tʷ"), "DIA-21B", IDC_CHART_CV403, DIACRITIC, ICH_DISABLED);
        AddChar(_AC("dW","dʷ"), "DIA-22B", IDC_CHART_CV503, DIACRITIC, ICH_DISABLED);
        AddChar(_AC("J","ʲ"),  NULL, IDC_CHART_CV304, DIACRITIC);
        AddChar(_AC("tJ","tʲ"), "DIA-23B", IDC_CHART_CV404, DIACRITIC, ICH_DISABLED);
        AddChar(_AC("dJ","dʲ"), "DIA-24B", IDC_CHART_CV504, DIACRITIC, ICH_DISABLED);
        AddChar(_AC("\xEC","ˠ"),  NULL, IDC_CHART_CV305, DIACRITIC);
        AddChar(_AC("t\xEC","tˠ"), "DIA-25B", IDC_CHART_CV405, DIACRITIC, ICH_DISABLED);
        AddChar(_AC("d\xEC","dˠ"), "DIA-26B", IDC_CHART_CV505, DIACRITIC, ICH_DISABLED);
        AddChar(_AC("\xB3","ˤ"),  NULL, IDC_CHART_CV306, DIACRITIC);
        AddChar(_AC("t\xB3","tˤ"), "DIA-27B", IDC_CHART_CV406, DIACRITIC, ICH_DISABLED);
        AddChar(_AC("d\xB3","dˤ"), "DIA-28B", IDC_CHART_CV506, DIACRITIC, ICH_DISABLED);
        AddChar(_AC("\xF2","̴"),  NULL, IDC_CHART_CV307, DIACRITIC);
        AddChar(_AC("l\xF2","ɫ"), "DIA-29B", IDC_CHART_CV407, DIACRITIC, ICH_DISABLED);
        AddChar(_AC("\xA3","̝"),  NULL, IDC_CHART_CV308, DIACRITIC);
        AddChar(_AC("e\xA3","e̝"), "DIA-30A", IDC_CHART_CV408, DIACRITIC, ICH_DISABLED);
        AddChar(_AC("\xA8\xA3","ɹ̝"), "DIA-31B", IDC_CHART_CV507, DIACRITIC, ICH_DISABLED);
        AddChar(_AC("\xA2","̞"),  NULL, IDC_CHART_CV309, DIACRITIC);
        AddChar(_AC("e\xA2","e̞"), "DIA-32A", IDC_CHART_CV409, DIACRITIC, ICH_DISABLED);
        AddChar(_AC("B\xA2","β̞"), "DIA-33B", IDC_CHART_CV508, DIACRITIC, ICH_DISABLED);
        AddChar(_AC("\xC1","̘"),  NULL, IDC_CHART_CV310, DIACRITIC);
        AddChar(_AC("e\xC1","e̘"), "DIA-34A", IDC_CHART_CV410, DIACRITIC, ICH_DISABLED);
        AddChar(_AC("\xAA","̙"),  NULL, IDC_CHART_CV311, DIACRITIC);
        AddChar(_AC("e\xAA","e̙"), "DIA-35A", IDC_CHART_CV411, DIACRITIC, ICH_DISABLED);
        AddChar(_AC("0","̪"),  NULL, IDC_CHART_CV600, DIACRITIC);
        AddChar(_AC("t0","t̪"), "DIA-36B", IDC_CHART_CV700, DIACRITIC, ICH_DISABLED);
        AddChar(_AC("d0","d̪"), "DIA-37B", IDC_CHART_CV800, DIACRITIC, ICH_DISABLED);
        AddChar(_AC("\xB0","̺"),  NULL, IDC_CHART_CV601, DIACRITIC);
        AddChar(_AC("t\xB0","t̺"), "DIA-38B", IDC_CHART_CV701, DIACRITIC, ICH_DISABLED);
        AddChar(_AC("d\xB0","d̺"), "DIA-39B", IDC_CHART_CV801, DIACRITIC, ICH_DISABLED);
        AddChar(_AC("6","̻"),  NULL, IDC_CHART_CV602, DIACRITIC);
        AddChar(_AC("t6","t̻"), "DIA-40B", IDC_CHART_CV702, DIACRITIC, ICH_DISABLED);
        AddChar(_AC("d6","d̻"), "DIA-41B", IDC_CHART_CV802, DIACRITIC, ICH_DISABLED);
        AddChar(_AC(")","̃"),  NULL, IDC_CHART_CV603, DIACRITIC);
        AddChar(_AC("e)","ẽ"), "DIA-42A", IDC_CHART_CV803, DIACRITIC, ICH_DISABLED);
        AddChar(_AC("<","ⁿ"),  NULL, IDC_CHART_CV604, DIACRITIC);
        AddChar(_AC("d<","dⁿ"), "DIA-43B", IDC_CHART_CV804, DIACRITIC, ICH_DISABLED);
        AddChar(_AC(":","ˡ"),  NULL, IDC_CHART_CV605, DIACRITIC);
        AddChar(_AC("d:","dˡ"), "DIA-44B", IDC_CHART_CV805, DIACRITIC, ICH_DISABLED);
        AddChar(_AC("|","̚"),  NULL, IDC_CHART_CV606, DIACRITIC);
        AddChar(_AC("d|","d̚"), "DIA-45A", IDC_CHART_CV806, DIACRITIC, ICH_DISABLED);
        // Suprasegmentals
        AddChar(_AC("\xC8","ˈ"),  NULL, IDC_CHART_CV0, SUPRASEG);
        AddChar(_AC("\xC7" "foUnP\xC8tISPn","ˌ" "foʊnɵˈtɪʃɵn"), "SSEG-00A", IDC_CHART_CV100, SUPRASEG, ICH_DISABLED);
        AddChar(_AC("\xC7","ˌ"),  NULL, IDC_CHART_CV1, SUPRASEG);
        AddChar(_AC("\xF9","ː"),  NULL, IDC_CHART_CV2, SUPRASEG);
        AddChar(_AC("e\xF9","eː"), "SSEG-01A", IDC_CHART_CV101, SUPRASEG, ICH_DISABLED);
        AddChar(_AC(">","ˑ"),  NULL, IDC_CHART_CV3, SUPRASEG);
        AddChar(_AC("e>","eˑ"), "SSEG-02A", IDC_CHART_CV102, SUPRASEG, ICH_DISABLED);
        AddChar(_AC("\xE1","̆"),  NULL, IDC_CHART_CV4, SUPRASEG);
        AddChar(_AC("e\xE1","ĕ"), "SSEG-03A", IDC_CHART_CV103, SUPRASEG, ICH_DISABLED);
        AddChar(_AC("\x8E","ǀ"),  NULL, IDC_CHART_CV5, SUPRASEG);
        AddChar(_AC("","ǁ"),  NULL, IDC_CHART_CV6, SUPRASEG);
        AddChar(_AC(".","."),  NULL, IDC_CHART_CV7, SUPRASEG);
        AddChar(_AC("\xA8i.Qkt","ɹi.ækt"), "SSEG-04A", IDC_CHART_CV104, SUPRASEG, ICH_DISABLED);
        AddChar(_AC("\xED","‿"),  NULL, IDC_CHART_CV8, SUPRASEG);
        //AddChar(_AC("\x83","͡"),  NULL, IDC_CHART_CV9, SUPRASEG);
        AddChar(_AC("\x89","̋"),  NULL, IDC_CHART_CV200, SUPRASEG);
        AddChar(_AC("e\x89","e̋"), "SSEG-05A", IDC_CHART_CV300, SUPRASEG, ICH_DISABLED);
        AddChar(_AC("\x8A","˥"),  NULL, IDC_CHART_CV400, SUPRASEG);
        AddChar(_AC("\x8F","́"),  NULL, IDC_CHART_CV201, SUPRASEG);
        AddChar(_AC("e\x8F","é"), "SSEG-06A", IDC_CHART_CV301, SUPRASEG, ICH_DISABLED);
        AddChar(_AC("\x91","˦"),  NULL, IDC_CHART_CV401, SUPRASEG);
        AddChar(_AC("\x94","̄"),  NULL, IDC_CHART_CV202, SUPRASEG);
        AddChar(_AC("e\x94","ē"), "SSEG-07A", IDC_CHART_CV302, SUPRASEG, ICH_DISABLED);
        AddChar(_AC("\x95","˧"),  NULL, IDC_CHART_CV402, SUPRASEG);
        AddChar(_AC("\x99","̀"),  NULL, IDC_CHART_CV203, SUPRASEG);
        AddChar(_AC("e\x99","è"), "SSEG-08A", IDC_CHART_CV303, SUPRASEG, ICH_DISABLED);
        AddChar(_AC("\x9A","˨"),  NULL, IDC_CHART_CV403, SUPRASEG);
        AddChar(_AC("\x9E","̏"),  NULL, IDC_CHART_CV204, SUPRASEG);
        AddChar(_AC("e\x9E","ȅ"), "SSEG-09A", IDC_CHART_CV304, SUPRASEG, ICH_DISABLED);
        AddChar(_AC("\x9F","˩"),  NULL, IDC_CHART_CV404, SUPRASEG);
        AddChar(_AC("\x9B","↓"),  NULL, IDC_CHART_CV205, SUPRASEG);
        AddChar(_AC("\x8B","↑"),  NULL, IDC_CHART_CV206, SUPRASEG);
        AddChar(_AC("\xF3","̌"),  NULL, IDC_CHART_CV500, SUPRASEG);
        AddChar(_AC("e\xF3","ě"), "SSEG-10A", IDC_CHART_CV600, SUPRASEG, ICH_DISABLED);
        AddChar(_AC("\xE8","˩˥"),  NULL, IDC_CHART_CV700, SUPRASEG);
        AddChar(_AC("\xE6","̂"),  NULL, IDC_CHART_CV501, SUPRASEG);
        AddChar(_AC("e\xE6","ê"), "SSEG-11A", IDC_CHART_CV601, SUPRASEG, ICH_DISABLED);
        AddChar(_AC("\x86","˥˩"),  NULL, IDC_CHART_CV701, SUPRASEG);
        AddChar(_AC("\x9D",""),  NULL, IDC_CHART_CV502, SUPRASEG);
        AddChar(_AC("e\x9D","e"), "SSEG-12A", IDC_CHART_CV602, SUPRASEG, ICH_DISABLED);
        AddChar(_AC("\xD8","˧˥"),  NULL, IDC_CHART_CV702, SUPRASEG);
        AddChar(_AC("\x88",""),  NULL, IDC_CHART_CV503, SUPRASEG);
        AddChar(_AC("e\x88","e"), "SSEG-13A", IDC_CHART_CV603, SUPRASEG, ICH_DISABLED);
        AddChar(_AC("\x80","˩˧"),  NULL, IDC_CHART_CV703, SUPRASEG);
        AddChar(_AC("(",""),  NULL, IDC_CHART_CV504, SUPRASEG);
        AddChar(_AC("e(","e"), "SSEG-14A", IDC_CHART_CV604, SUPRASEG, ICH_DISABLED);
        AddChar(_AC("\xCA","˦˥˦"),  NULL, IDC_CHART_CV704, SUPRASEG);
        AddChar(_AC("\xCC","↗"),  NULL, IDC_CHART_CV505, SUPRASEG);
        AddChar(_AC("\xCD","↘"),  NULL, IDC_CHART_CV506, SUPRASEG);
    }
}
