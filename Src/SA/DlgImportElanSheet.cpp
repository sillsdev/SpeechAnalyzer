// DlgAlignTranscriptionDataSheet.cpp : implementation file
//

#include "stdafx.h"
#include "sa_doc.h"
#include "DlgImportElanSheet.h"

IMPLEMENT_DYNAMIC(CDlgImportElanSheet, CPropertySheet)

CDlgImportElanSheet::CDlgImportElanSheet(CWnd * pParentWnd, Elan::CAnnotationDocument & aDocument) :
    pg1(),
    pg2(),
    pg3(),
    CPropertySheet(IDS_AWIZ_CAPTION_IMPORT_ELAN, pParentWnd, 0),
    document(aDocument),
    selection(0) {
    AddPage(&pg1);
    AddPage(&pg2);
    AddPage(&pg3);
}

CDlgImportElanSheet::~CDlgImportElanSheet() {
}

BEGIN_MESSAGE_MAP(CDlgImportElanSheet, CPropertySheet)
END_MESSAGE_MAP()

BOOL CDlgImportElanSheet::OnInitDialog() {
    BOOL bResult = CPropertySheet::OnInitDialog();
    SendMessage(DM_SETDEFID, PSBTN_NEXT);
    return bResult;
}

map<EAnnotation,wstring> CDlgImportElanSheet::getAssignments() {
    return assignments;
}

