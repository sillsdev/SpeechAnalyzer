/////////////////////////////////////////////////////////////////////////////
// ch_table.h:
// Interface of the CCharTable
//          CIPATable  classes.
// Author: Urs Ruchti
// copyright 1997 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////
#ifndef _CH_TABLE_H_
#define _CH_TABLE_H_

//###########################################################################
// CCharTable

class CDlgCharChart;
class CCharTable
{

    // Construction/destruction/creation
public:
    CCharTable();
    ~CCharTable();

    // Attributes
protected:
    CFont  * m_pFont;  // font for characters
    CDlgCharChart  * m_pCaller; // window to call for character selection
    CWnd  * m_apParent[4]; // parent character pages
    CObArray m_CharOb;

    // Operations
protected:
    void CleanUp();
    void AddChar(LPSTR pszChar, LPSTR pszSoundFile, UINT nID, int nType, int nMode=0); // add a character to the table
public:
    virtual void SetupTable(CFont *, CDlgCharChart *, CWnd *, CWnd *, CWnd *, CWnd *) = 0; // pure virtual
    virtual void SelectChars(CString *, int nMode, int nType = -1); // select characters from the table
    virtual void InitPage(int nType); // initialize a character page
};

//###########################################################################
// CIPATable

class CIPATable : public CCharTable
{

    // Construction/destruction/creation
public:
    CIPATable();
    ~CIPATable();

    // Attributes
private:

    // Operations
public:
    virtual void SetupTable(CFont *, CDlgCharChart * pCaller, CWnd * pVowelPage, CWnd * pConsPage, CWnd * pDiacPage, CWnd * pSupraPage); // set up the IPA character table
};

#endif //_CH_TABLE_H_
