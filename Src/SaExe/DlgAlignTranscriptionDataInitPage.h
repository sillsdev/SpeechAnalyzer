#ifndef DLGALIGNTRANSCRIPTIONDATAINITPAGE_H
#define DLGALIGNTRANSCRIPTIONDATAINITPAGE_H

// CDlgAlignTranscriptionDataInitPage
#include "resource.h"

class CDlgAlignTranscriptionDataInitPage : public CPropertyPage
{
public:
    CDlgAlignTranscriptionDataInitPage();
    virtual ~CDlgAlignTranscriptionDataInitPage();

    enum { IDD = IDD_ANNOTATION_INIT_PAGE };

protected:
    DECLARE_MESSAGE_MAP()

public:
    afx_msg void OnBnClicked();
    virtual BOOL OnSetActive();
    virtual void DoDataExchange(CDataExchange * pDX);

    BOOL m_bGloss;
    BOOL m_bPhonemic;
    BOOL m_bPhonetic;
    BOOL m_bReference;
    BOOL m_bOrthographic;
    BOOL m_bUseReference;

private:
    void UpdateNext();
public:
    virtual LRESULT OnWizardNext();
};

#endif
