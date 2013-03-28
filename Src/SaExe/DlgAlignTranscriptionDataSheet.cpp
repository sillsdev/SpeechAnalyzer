// DlgAlignTranscriptionDataSheet.cpp : implementation file
//

#include "stdafx.h"
#include "sa_doc.h"
#include "DlgAlignTranscriptionDataSheet.h"
#include "resource.h"
#include "Segment.h"
#include "TranscriptionDataSettings.h"

// CDlgAlignTranscriptionDataSheet

IMPLEMENT_DYNAMIC(CDlgAlignTranscriptionDataSheet, CPropertySheet)

CDlgAlignTranscriptionDataSheet::CDlgAlignTranscriptionDataSheet(CWnd * pParentWnd, CSaDoc * pSaDoc) :
    gloss(pSaDoc,GLOSS,IDD_ANNOTATION_GLOSS_PAGE),
    ortho(pSaDoc,ORTHO,IDD_ANNOTATION_ORTHOGRAPHIC_PAGE),
    phonemic(pSaDoc,PHONEMIC,IDD_ANNOTATION_PHONEMIC_PAGE),
    phonetic(pSaDoc,PHONETIC,IDD_ANNOTATION_PHONETIC_PAGE),
    import(pSaDoc),
    finish(pSaDoc),
    CPropertySheet(IDS_AWIZ_CAPTION_ALIGN, pParentWnd, 0)
{
    AddPage(&init);
    AddPage(&import);
    AddPage(&gloss);
    AddPage(&ortho);
    AddPage(&phonetic);
    AddPage(&phonemic);
    AddPage(&align);
    AddPage(&segment);
    AddPage(&finish);
}

CDlgAlignTranscriptionDataSheet::~CDlgAlignTranscriptionDataSheet()
{
}

BEGIN_MESSAGE_MAP(CDlgAlignTranscriptionDataSheet, CPropertySheet)
END_MESSAGE_MAP()

CTranscriptionDataSettings CDlgAlignTranscriptionDataSheet::GetSettings()
{
    CTranscriptionDataSettings result;

    result.m_bUseReference = (init.m_bUseReference!=FALSE);
    result.m_bReference = (init.m_bReference!=FALSE);
    result.m_bPhonetic = (init.m_bPhonetic!=FALSE);
    result.m_bPhonemic = (init.m_bPhonemic!=FALSE);
    result.m_bOrthographic = (init.m_bOrthographic!=FALSE);
    result.m_bGloss = (init.m_bGloss!=FALSE);

    if (result.m_bUseReference)
    {
        result.m_TranscriptionData = import.m_TranscriptionData;
    }
    else
    {
        result.m_bPhoneticModified = phonetic.m_bModified;
        result.m_szPhonetic = phonetic.m_szText;

        result.m_bPhonemicModified = phonemic.m_bModified;
        result.m_szPhonemic = phonemic.m_szText;

        result.m_bOrthographicModified = ortho.m_bModified;
        result.m_szOrthographic = ortho.m_szText;

        result.m_bGlossModified = gloss.m_bModified;
        result.m_szGloss = gloss.m_szText;
    }

    result.m_nAlignBy = align.m_nAlignBy;
    result.m_nSegmentBy = segment.m_nSegmentBy;

    return result;
}

BOOL CDlgAlignTranscriptionDataSheet::OnInitDialog()
{
    BOOL bResult = CPropertySheet::OnInitDialog();
    SendMessage(DM_SETDEFID, PSBTN_NEXT);
    return bResult;
}

LRESULT CDlgAlignTranscriptionDataSheet::CalculateNext(int currentIDD)
{

    if (init.m_bUseReference)
    {
        switch (currentIDD)
        {
        case IDD_ANNOTATION_INIT_PAGE:
            return import.IDD;
        }
        return finish.IDD;
    }
    else
    {
        switch (currentIDD)
        {
        case IDD_ANNOTATION_INIT_PAGE:
            if (init.m_bPhonetic)
            {
                return phonetic.IDD;
            }
        case IDD_ANNOTATION_PHONETIC_PAGE:
            if (init.m_bPhonemic)
            {
                return phonemic.IDD;
            }
        case IDD_ANNOTATION_PHONEMIC_PAGE:
            if (init.m_bOrthographic)
            {
                return ortho.IDD;
            }
        case IDD_ANNOTATION_ORTHOGRAPHIC_PAGE:
            if (init.m_bGloss)
            {
                return gloss.IDD;
            }
        }
        return align.IDD;
    }
}

LRESULT CDlgAlignTranscriptionDataSheet::CalculateBack(int currentIDD)
{

    if (init.m_bUseReference)
    {
        switch (currentIDD)
        {
        case IDD_ANNOTATION_FINISH_PAGE:
            return import.IDD;
        }
        return init.IDD;
    }
    else
    {
        switch (currentIDD)
        {
        case IDD_ANNOTATION_ALIGN_PAGE:
            if (init.m_bGloss)
            {
                return gloss.IDD;
            }
        case IDD_ANNOTATION_GLOSS_PAGE:
            if (init.m_bOrthographic)
            {
                return ortho.IDD;
            }
        case IDD_ANNOTATION_ORTHOGRAPHIC_PAGE:
            if (init.m_bPhonemic)
            {
                return phonemic.IDD;
            }
        case IDD_ANNOTATION_PHONEMIC_PAGE:
            if (init.m_bPhonetic)
            {
                return phonetic.IDD;
            }
        }
        return init.IDD;
    }
}
