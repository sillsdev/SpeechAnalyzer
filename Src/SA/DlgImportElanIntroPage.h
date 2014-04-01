#ifndef DLGIMPORTELANINTROPAGE_H
#define DLGIMPORTELANINTROPAGE_H

#include "AppDefs.h"
#include "SaString.h"

class CSaDoc;

class CDlgImportElanIntroPage : public CPropertyPage
{
public:
    CDlgImportElanIntroPage();
    virtual ~CDlgImportElanIntroPage();

    virtual LRESULT OnWizardNext();
    virtual LRESULT OnWizardBack();
    virtual BOOL OnSetActive();

    enum { IDD = IDD_IMPORT_ELAN_INTRO_PAGE };

protected:
    DECLARE_MESSAGE_MAP()

private:
	virtual void DoDataExchange(CDataExchange* pDX);

	int selection;
};

#endif
