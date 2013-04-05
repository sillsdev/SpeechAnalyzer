#ifndef DLGALIGNTRANSCRIPTIONDATAIMPORTREFPAGE_H
#define DLGALIGNTRANSCRIPTIONDATAIMPORTREFPAGE_H

#include "resource.h"
#include "AppDefs.h"
#include "SaString.h"
#include "TranscriptionHelper.h"
#include "TranscriptionData.h"

class CSaDoc;
class CDlgAlignTranscriptionDataSheet;

class CDlgAlignTranscriptionDataImportRefPage : public CPropertyPage
{

public:
    CDlgAlignTranscriptionDataImportRefPage(CSaDoc * pSaDoc);
    virtual ~CDlgAlignTranscriptionDataImportRefPage();

    enum { IDD = IDD_ANNOTATION_IMPORT_REF };

private:
    CDlgAlignTranscriptionDataSheet * GetParent();
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
    CTranscriptionData m_TranscriptionData;

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

