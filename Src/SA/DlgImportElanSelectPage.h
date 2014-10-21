#ifndef DLGIMPORTELANSELECTPAGE_H
#define DLGIMPORTELANSELECTPAGE_H

#include "AppDefs.h"
#include "ElanUtils.h"

class CSaDoc;

class CDlgImportElanSelectPage : public CPropertyPage
{
public:
    CDlgImportElanSelectPage();
    virtual ~CDlgImportElanSelectPage();

    virtual BOOL OnSetActive();
    virtual LRESULT OnWizardNext();
    virtual LRESULT OnWizardBack();
	virtual BOOL OnInitDialog();

    enum { IDD = IDD_IMPORT_ELAN_SELECT_PAGE };

protected:
    DECLARE_MESSAGE_MAP()

private:
	virtual void DoDataExchange(CDataExchange* pDX);
	CComboBox refnum;
	CComboBox phonetic;
	CComboBox phonemic;
	CComboBox gloss;
	CComboBox glossNat;
	CComboBox ortho;
	CComboBox pl1;
	CComboBox pl2;
	CComboBox pl3;
	CComboBox pl4;
};

#endif
