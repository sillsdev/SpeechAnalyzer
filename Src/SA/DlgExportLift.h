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
using std::map;

class CSaDoc;

/////////////////////////////////////////////////////////////////////////////
// CDlgExportLift dialog
class CDlgExportLift : public CDialog {
public:
    CDlgExportLift(LPCTSTR szDocTitle,
                   LPCTSTR szPath,
                   BOOL gloss,
                   BOOL glossNat,
                   BOOL ortho,
                   BOOL phonemic,
                   BOOL phonetic,
                   BOOL reference,
                   map<wstring,wstring> contryCodes,
                   CWnd * pParent = NULL);

    CExportLiftSettings settings;

protected:
    virtual BOOL OnInitDialog();
    virtual void DoDataExchange(CDataExchange * pDX); // DDX/DDV support
    void SetEnable(int nItem, BOOL bEnable);
    void SetCheck(int nItem, BOOL bCheck);
    void UpdateButtonState();
    CString lookupCountryCode(LPCTSTR value);

    afx_msg void OnHelpExportBasic();
    afx_msg void OnClickedBrowse();

    enum { IDD = IDD_EXPORT_LIFT };

    CButton ctlButtonOK;
    CEdit ctlEditFolder;

    BOOL bGlossDflt;
    BOOL bGlossNatDflt;
    BOOL bOrthoDflt;
    BOOL bToneDflt;
    BOOL bPhonemicDflt;
    BOOL bPhoneticDflt;
    BOOL bPOSDflt;
    BOOL bReferenceDflt;

    CComboBox ctlReferenceList;
    CComboBox ctlGlossNatList;
    CComboBox ctlPhraseList1List;
    CComboBox ctlPhraseList2List;
    CComboBox ctlOrthoList;

    wstring refLang;
    wstring phonemicLang;
    wstring phoneticLang;
    wstring glossLang;
    wstring orthoLang;
    wstring posLang;

    map<wstring,wstring> countryCodes;

    DECLARE_MESSAGE_MAP()
};

#endif
