/////////////////////////////////////////////////////////////////////////////
// DlgExportFW.h:
// Interface of the DlgExportFW class.
//
// Author: Steve MacLean
// copyright 1999 JAARS Inc. SIL
//
// Revision History
// 1.5Test8.3
//         SDM Original version
//
/////////////////////////////////////////////////////////////////////////////
#ifndef _DGLEXPORTLIFT_H
#define _DGLEXPORTLIFT_H

#include "objectostream.h"
#include "SaString.h"
#include "ExportLiftSettings.h"
#include <list>

using std::list;

class CSaDoc;

/////////////////////////////////////////////////////////////////////////////
// CDlgExportLift dialog
class CDlgExportLift : public CDialog {
public:
    CDlgExportLift(LPCTSTR szDocTitle,
                   BOOL gloss,
                   BOOL glossNat,
                   BOOL ortho,
                   BOOL phonemic,
                   BOOL phonetic,
                   BOOL reference,
                   list<wstring> iso,
                   CWnd * pParent = NULL);

    CExportLiftSettings settings;

protected:
    virtual BOOL OnInitDialog();
    virtual void DoDataExchange(CDataExchange * pDX); // DDX/DDV support
    void SetEnable(int nItem, BOOL bEnable);
    void SetCheck(int nItem, BOOL bCheck);
    void GetCurrentPath(LPTSTR szBuffer, size_t size);
    void UpdateButtonState();
    void WriteFileUtf8(CFile * pFile, const CSaString szString);
    CSaString GetFieldWorksProjectDirectory();
    bool SearchForValue(HKEY root, DWORD sam, wstring keyName, LPCTSTR valueName, wstring & value);

    afx_msg void OnClickedExSfmInterlinear();
    afx_msg void OnClickedExSfmMultirecord();
    afx_msg void OnClickedBrowseFieldworks();
    afx_msg void OnHelpExportBasic();
    afx_msg void OnSelchangeComboFieldworksProject();
    afx_msg void OnKillfocusComboFieldworksProject();

    enum { IDD = IDD_EXPORT_LIFT };
    CEdit ctlEditFieldWorksFolder;

    CComboBox ctlComboFieldWorksProject;
    CStatic ctlStaticFieldWorksProject;
    CButton ctlButtonOK;

    BOOL bGlossDflt;
    BOOL bGlossNatDflt;
    BOOL bOrthoDflt;
    BOOL bToneDflt;
    BOOL bPhonemicDflt;
    BOOL bPhoneticDflt;
    BOOL bPOSDflt;
    BOOL bReferenceDflt;

    CComboBox ctlReferenceList;
    CComboBox ctlPhonemicList;
    CComboBox ctlPhoneticList;
    CComboBox ctlGlossList;
    CComboBox ctlGlossNatList;
    CComboBox ctlOrthoList;

    wstring refLang;
    wstring phonemicLang;
    wstring phoneticLang;
    wstring glossLang;
    wstring orthoLang;
    wstring posLang;

    list<wstring> iso;

    DECLARE_MESSAGE_MAP()
};

#endif
