#ifndef DLGIMPORTELANSHEET_H
#define DLGIMPORTELANSHEET_H

#include "DlgImportElanIntroPage.h"
#include "DlgImportElanSelectPage.h"
#include "DlgImportElanFinishPage.h"
#include "ElanUtils.h"

class CSaDoc;

typedef map<EAnnotation,wstring> ElanMap;

class CDlgImportElanSheet : public CPropertySheet {
public:
    CDlgImportElanSheet(CWnd * pParentWnd, Elan::CAnnotationDocument & document);
    virtual ~CDlgImportElanSheet();
    virtual BOOL OnInitDialog();

    map<EAnnotation,wstring> getAssignments();

protected:
    DECLARE_MESSAGE_MAP()

private:
    DECLARE_DYNAMIC(CDlgImportElanSheet)

    CDlgImportElanIntroPage pg1;
    CDlgImportElanSelectPage pg2;
    CDlgImportElanFinishPage pg3;

    int selection;
    Elan::CAnnotationDocument & document;
    ElanMap assignments;

};

#endif
