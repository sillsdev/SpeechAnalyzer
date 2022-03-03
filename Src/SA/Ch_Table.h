/////////////////////////////////////////////////////////////////////////////
// ch_table.h:
// Interface of the CCharTable
//          CIPATable  classes.
// Author: Urs Ruchti
// copyright 1997 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////
#ifndef _CH_TABLE_H_
#define _CH_TABLE_H_

class CDlgCharChart;
class CCharTable {
public:
    CCharTable();
    virtual ~CCharTable();

    virtual void SetupTable(CFont*, CDlgCharChart*, CWnd*, CWnd*, CWnd*, CWnd*) = 0; // pure virtual
    virtual void SelectChars(CString*, int nMode, int nType = -1); // select characters from the table
    virtual void InitPage(int nType); // initialize a character page

protected:
    void CleanUp();
    void AddChar(LPSTR pszChar, LPSTR pszSoundFile, UINT nID, int nType, int nMode = 0); // add a character to the table

    CFont * m_pFont;  // font for characters
    CDlgCharChart * m_pCaller; // window to call for character selection
    CWnd * m_apParent[4]; // parent character pages
    CObArray m_CharOb;

};

class CIPATable : public CCharTable {

public:
    CIPATable();
    virtual ~CIPATable();

    virtual void SetupTable(CFont *, CDlgCharChart * pCaller, CWnd * pVowelPage, CWnd * pConsPage, CWnd * pDiacPage, CWnd * pSupraPage); // set up the IPA character table
};

#endif
