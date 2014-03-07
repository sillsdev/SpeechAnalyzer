#ifndef DLGALIGNTRANSCRIPTIONDATAFINISHPAGE_H
#define DLGALIGNTRANSCRIPTIONDATAFINISHPAGE_H

class CDlgAlignTranscriptionDataSheet;
class CSaDoc;

class CDlgAlignTranscriptionDataFinishPage : public CPropertyPage
{

public:
    CDlgAlignTranscriptionDataFinishPage(CSaDoc * pSaDoc);
    virtual ~CDlgAlignTranscriptionDataFinishPage();

    enum { IDD = IDD_ANNOTATION_FINISH_PAGE };

private:
    CDlgAlignTranscriptionDataSheet * GetParent();
    CSaDoc * m_pSaDoc;

protected:
    DECLARE_MESSAGE_MAP()

public:
    virtual BOOL OnWizardFinish();
    virtual BOOL OnSetActive();
    virtual LRESULT OnWizardBack();
};

#endif
