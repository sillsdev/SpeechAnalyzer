#ifndef DLGIMPORTELANFINISHPAGE_H
#define DLGIMPORTELANFINISHPAGE_H

class CDlgImportElanSheet;
class CSaDoc;

class CDlgImportElanFinishPage : public CPropertyPage
{

public:
    CDlgImportElanFinishPage();
    virtual ~CDlgImportElanFinishPage();

    virtual BOOL OnWizardFinish();
    virtual BOOL OnSetActive();
    virtual LRESULT OnWizardBack();

    enum { IDD = IDD_IMPORT_ELAN_FINISH_PAGE };

protected:
    DECLARE_MESSAGE_MAP()

private:
    CDlgImportElanSheet * GetParent();
};

#endif
