/////////////////////////////////////////////////////////////////////////////
// fileInformation.h
// Interface of the CDlgInformationFilePage (property page)
//                  CDlgInformationWavePage (property page)
//                  CDlgInformationSourcePage (property page)
//                  CDlgInformationUserPage (property page)
//                  CDlgFileInformation (property sheet)
//
// Author: Urs Ruchti
// copyright 2000 JAARS Inc. SIL
//
// Revision History
//
// 01/12/2000
//    SDM   Extract from sa_dlg2.h
//
/////////////////////////////////////////////////////////////////////////////
#ifndef _FILE_INFORMATION_H

#include "CSaString.h"
#include "sa_dlwnd.h"
#include "resource.h"

#define _FILE_INFORMATION_H


//###########################################################################
// CDlgInformationFilePage property page

class CDlgInformationFilePage : public CPropertyPage {

    // Construction/destruction/creation
public:
    CDlgInformationFilePage(); // standard constructor

    // Attributes
private:

    // Dialog Data
    //{{AFX_DATA(CDlgInformationFilePage)
    enum { IDD = IDD_INFORMATIONFILEPAGE };
    // NOTE: the ClassWizard will add data members here
    //}}AFX_DATA

    // Operations
protected:
    virtual void DoDataExchange(CDataExchange * pDX); // DDX/DDV support

    // Generated message map functions
protected:
    //{{AFX_MSG(CDlgInformationFilePage)
    virtual BOOL OnInitDialog();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

//###########################################################################
// CDlgInformationWavePage property page

class CDlgInformationWavePage : public CPropertyPage {

    // Construction/destruction/creation
public:
    CDlgInformationWavePage(); // standard constructor

    // Attributes
private:

    // Dialog Data
    //{{AFX_DATA(CDlgInformationWavePage)
    enum { IDD = IDD_INFORMATIONWAVEPAGE };
    // NOTE: the ClassWizard will add data members here
    //}}AFX_DATA

    // Operations
protected:
    virtual void DoDataExchange(CDataExchange * pDX); // DDX/DDV support

    // Generated message map functions
protected:
    //{{AFX_MSG(CDlgInformationWavePage)
    virtual BOOL OnInitDialog();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

//###########################################################################
// CDlgInformationSourcePage property page

class CDlgInformationSourcePage : public CPropertyPage {

    // Construction/destruction/creation
public:
    CDlgInformationSourcePage();    // standard constructor

    // Attributes
private:

public:
    // Dialog Data
    //{{AFX_DATA(CDlgInformationSourcePage)
    enum { IDD = IDD_INFORMATIONSOURCEPAGE };
    CSaString m_szCountry;
    CSaString m_szDialect;
    CSaString m_szEthnoID;
    CSaString m_szFamily;
    CSaString m_szLanguage;
    int     m_nGender;
    CSaString m_szRegion;
    CSaString m_szSpeaker;
    CSaString m_szReference;
    CSaString m_szTranscriber;
    //}}AFX_DATA

    // Operations
protected:
    virtual void DoDataExchange(CDataExchange * pDX); // DDX/DDV support

    // Generated message map functions
protected:
    //{{AFX_MSG(CDlgInformationSourcePage)
    virtual BOOL OnInitDialog();
    virtual void OnOK();
    afx_msg void OnChangeEthnoid();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

//###########################################################################
// CDlgInformationUserPage property page

class CDlgInformationUserPage : public CPropertyPage {

    // Construction/destruction/creation
public:
    CDlgInformationUserPage(); // standard constructor

    // Attributes
private:
    CTranscriptionDisp m_TranscriptionDisp;

    // Dialog Data
public:
    //{{AFX_DATA(CDlgInformationUserPage)
    enum { IDD = IDD_INFORMATIONUSERPAGE };
    CSaString m_szFileDesc;
    CSaString m_szFreeTranslation;
    //}}AFX_DATA

    // Operations
protected:
    virtual void DoDataExchange(CDataExchange * pDX); // DDX/DDV support

    // Generated message map functions
protected:
    //{{AFX_MSG(CDlgInformationUserPage)
    virtual BOOL OnInitDialog();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};
//###########################################################################
// CDlgFileInformation property sheet

class CDlgFileInformation : public CPropertySheet {
    DECLARE_DYNAMIC(CDlgFileInformation)

// Construction/destruction/creation
public:
    CDlgFileInformation(LPCTSTR pszCaption, CWnd * pParent, UINT iSelectPage, BOOL bRecorder = FALSE);

    // Attributes
public:
    CDlgInformationFilePage   m_dlgFilePage;
    CDlgInformationWavePage   m_dlgWavePage;
    CDlgInformationSourcePage m_dlgSourcePage;
    CDlgInformationUserPage   m_dlgUserPage;

    // Operations
public:
    void OnHelpInformation();

protected:
    virtual void DoDataExchange(CDataExchange * pDX); // DDX/DDV support

private:
    void ChangeButtons(); // delete Apply button, move other buttons
    CButton m_cHelp;

    // Generated message map functions
protected:
    //{{AFX_MSG(CDlgFileInformation)
    virtual BOOL OnInitDialog();
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};


#endif //_FILE_INFORMATION_H
