/////////////////////////////////////////////////////////////////////////////
// DlgAnnotation.cpp:
// Implementation of the CDlgImportSFM class.
// Author: Steve MacLean
// copyright 2014 JAARS Inc. SIL
//
// Revision History
// 1.5Test8.3
//         SDM Original version
// 1.5Test8.5
//         SDM Changed Table headings to use Initial Uppercase ("Emic" & "Etic")
// 1.5Test10.7
//         SDM Changed Import to not pad extra phonetic with spaces
// 1.5Test11.0
//         SDM replaced GetOffset() + GetDuration() with CSegment::GetStop()
//         SDM fixed bug in export which crashed if phonetic segment empty
//         SDM changed export to export AutoPitch (Grappl)
// 1.5Test11.3
//         SDM changed CSegment::AdjustPositionAll to CSegment::Adjust
//
//    07/27/2000
//         DDO Changed so these dialogs display before the user is asked
//             what file to export. Therefore, I had to move the get filename
//             dialog call into this module instead of doing it in the view
//             class when the user picks one of the export menu items.
//
/////////////////////////////////////////////////////////////////////////////
#ifndef DLGIMPORTSFM_H
#define DLGIMPORTSFM_H

/////////////////////////////////////////////////////////////////////////////
// CDlgImportSFM private dialog

class CDlgImportSFM : public CDialog
{
    // Construction
public:
    CDlgImportSFM(BOOL bPhonetic, BOOL bPhonemic, BOOL bOrtho, CWnd * pParent = NULL);  // standard constructor

    // Dialog Data
    enum { IDD = IDD_ANNOTATION_IMPORT_SFM };
    BOOL m_bGloss;
    BOOL m_bGlossNat;
    BOOL m_bPhonemic;
    BOOL m_bPhonetic;
    BOOL m_bOrthographic;
    CSaString m_szPhonemic;
    CSaString m_szGloss;
	CSaString m_szGlossNat;
    CSaString m_szPhonetic;
    CSaString m_szOrthographic;

    // Implementation
protected:
    virtual void DoDataExchange(CDataExchange * pDX);   // DDX/DDV support
    void SetEnable(int nItem, BOOL bEnable);

    // Generated message map functions
    afx_msg void OnImportPlainText();
    virtual BOOL OnInitDialog();
    DECLARE_MESSAGE_MAP()
};
#endif
