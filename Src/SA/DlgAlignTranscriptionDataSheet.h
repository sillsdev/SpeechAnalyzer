#ifndef DLGALIGNTRANSCRIPTIONDATASHEET_H
#define DLGALIGNTRANSCRIPTIONDATASHEET_H

#include "dlgaligntranscriptiondatainitpage.h"
#include "dlgaligntranscriptiondatatextpage.h"
#include "dlgaligntranscriptiondataalignbypage.h"
#include "dlgaligntranscriptiondatasegmentbypage.h"
#include "dlgaligntranscriptiondatafinishpage.h"
#include "DlgAlignTranscriptionDataImportRefPage.h"
#include "sa_doc.h"

class CTranscriptionDataSettings;

class CDlgAlignTranscriptionDataSheet : public CPropertySheet
{

    DECLARE_DYNAMIC(CDlgAlignTranscriptionDataSheet)

    CDlgAlignTranscriptionDataInitPage init;
    CDlgAlignTranscriptionDataImportRefPage import;
    CDlgAlignTranscriptionDataTextPage gloss;
    CDlgAlignTranscriptionDataTextPage glossNat;
    CDlgAlignTranscriptionDataTextPage phonetic;
    CDlgAlignTranscriptionDataTextPage phonemic;
    CDlgAlignTranscriptionDataTextPage ortho;
    CDlgAlignTranscriptionDataAlignByPage align;
    CDlgAlignTranscriptionDataSegmentByPage segment;
    CDlgAlignTranscriptionDataFinishPage finish;

public:
    CDlgAlignTranscriptionDataSheet(CWnd * pParentWnd = NULL, CSaDoc * pSaDoc = NULL);
    virtual ~CDlgAlignTranscriptionDataSheet();

protected:
    DECLARE_MESSAGE_MAP()

public:
    CTranscriptionDataSettings GetSettings();
    virtual BOOL OnInitDialog();
    LRESULT CalculateBack(int currentIDD);
    LRESULT CalculateNext(int currentIDD);
};

#endif
