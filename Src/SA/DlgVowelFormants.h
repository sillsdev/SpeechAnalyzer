#ifndef DLGVOWELFORMANTS_H
#define DLGVOWELFORMANTS_H

#include "flexeditgrid.h"

/////////////////////////////////////////////////////////////////////////////
// CDlgVowelFormants dialog

class CDlgVowelFormants : public CDialog {
    // Construction
public:
    CDlgVowelFormants(CVowelFormantSet & cVowelSet, CWnd * pParent = NULL); // standard constructor

    // Dialog Data
    CVowelFormantSet & m_cVowelSetOK;
    CVowelFormantSet m_cSet;
    CFlexEditGrid   m_cGrid;
    enum { IDD = IDD_VOWEL_FORMANTS };
    int     m_nGender;
    CSaString   m_szSetName;

protected:
    virtual void DoDataExchange(CDataExchange * pDX);   // DDX/DDV support

    // Implementation
    BOOL OnValidateGenderChange();
protected:

    afx_msg void OnGenderChange();
    virtual BOOL OnInitDialog();
    virtual void OnOK();
    virtual void OnCancel();
    DECLARE_MESSAGE_MAP()
};

#endif
