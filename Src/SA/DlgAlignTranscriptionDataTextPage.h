#ifndef DLGALIGNTRANSCRIPTIONDATATEXTPAGE_H
#define DLGALIGNTRANSCRIPTIONDATATEXTPAGE_H

#include "AppDefs.h"
#include "SaString.h"

class CSaDoc;

class CDlgAlignTranscriptionDataTextPage : public CPropertyPage
{

public:
    CDlgAlignTranscriptionDataTextPage(CSaDoc * pSaDoc, EAnnotation type, int IDD);
    virtual ~CDlgAlignTranscriptionDataTextPage();

private:
    CSaDoc * m_pSaDoc;

protected:
    DECLARE_MESSAGE_MAP()

public:
    virtual BOOL OnSetActive();
    afx_msg void OnClickedImport();
    afx_msg void OnClickedRevert();

public:
    bool m_bModified;
    CSaString m_szText;
    EAnnotation m_Type;
    int IDD;

private:
    void SaveAnnotation();
    void OnUpdateAnnotation();
    void SetEnable(int nItem, BOOL bEnable);
    void SetAnnotation();
    void SetText(int nItem, CSaString szText);
public:
    virtual LRESULT OnWizardNext();
    virtual LRESULT OnWizardBack();
};

#endif
