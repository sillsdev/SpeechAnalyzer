#include "stdafx.h"
#include "DlgExportTimeTable.h"
#include "SaString.h"
#include "Sa_Doc.h"
#include "SA_View.h"
#include "Segment.h"
#include "MainFrm.h"
#include "GlossSegment.h"
#include "GlossNatSegment.h"
#include "PhoneticSegment.h"
#include "DlgExportFW.h"
#include "FileUtils.h"
#include "Process\Process.h"
#include "Process\sa_p_lou.h"
#include "Process\sa_p_grappl.h"
#include "Process\sa_p_custompitch.h"
#include "Process\sa_p_smoothedpitch.h"
#include "Process\sa_p_melogram.h"
#include "Process\sa_p_zcr.h"
#include "Process\sa_p_fra.h"
#include "Process\sa_p_spu.h"
#include "Process\sa_p_spg.h"
#include "Process\FormantTracker.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgExportTimeTable dialog

CDlgExportTimeTable::CDlgExportTimeTable( CWnd * pParent /*=NULL*/) :
    CDialog(CDlgExportTimeTable::IDD, pParent) {

    m_bAnnotations = TRUE;
    m_bCalculations = FALSE;
    m_bF1 = FALSE;
    m_bF2 = FALSE;
    m_bF3 = FALSE;
    m_bF4 = FALSE;
    m_bFormants = FALSE;
    m_bGloss = TRUE;
    m_bGlossNat = TRUE;
    m_szIntervalTime = "20";
    m_bSegmentLength = TRUE;
    m_bMagnitude = TRUE;
    m_bOrtho = TRUE;
    m_bPhonemic = TRUE;
    m_bPhonetic = TRUE;
    m_bPitch = TRUE;
    m_bReference = FALSE;
    m_bSegmentStart = TRUE;
    m_bSampleTime = TRUE;
    m_bTone = FALSE;
    m_bZeroCrossings = TRUE;
    m_nSampleRate = 0;
    m_nCalculationMethod = 1;
    m_bPhonetic2 = TRUE;
    m_nRegion = 0;
    m_bMelogram = TRUE;

    CSaDoc * pDoc = (CSaDoc *)((CMainFrame *)AfxGetMainWnd())->GetCurrSaView()->GetDocument();

	// no annotations
    if (pDoc->GetSegment(PHONETIC)->IsEmpty()) { 
        m_bReference = m_bPhonetic = m_bTone = m_bPhonemic = m_bOrtho = m_bGloss = m_bGlossNat = FALSE;
		// no segments
        m_bSegmentStart = m_bSegmentLength = FALSE; 
        m_nSampleRate = 1;
    }
}

void CDlgExportTimeTable::DoDataExchange(CDataExchange * pDX) {

    CDialog::DoDataExchange(pDX);

    DDX_Check(pDX, IDC_EXTAB_ANNOTATIONS, m_bAnnotations);
    DDX_Check(pDX, IDC_EXTAB_CALC, m_bCalculations);
    DDX_Check(pDX, IDC_EXTAB_F1, m_bF1);
    DDX_Check(pDX, IDC_EXTAB_F2, m_bF2);
    DDX_Check(pDX, IDC_EXTAB_F3, m_bF3);
    DDX_Check(pDX, IDC_EXTAB_F4, m_bF4);
    DDX_Check(pDX, IDC_EXTAB_FORMANTS, m_bFormants);
    DDX_Check(pDX, IDC_EXTAB_GLOSS, m_bGloss);
    DDX_Check(pDX, IDC_EXTAB_GLOSS_NAT, m_bGlossNat);
    DDX_Text(pDX, IDC_EXTAB_INTERVAL_TIME, m_szIntervalTime);
    DDV_MaxChars(pDX, m_szIntervalTime, 4);
    DDX_Check(pDX, IDC_EXTAB_LENGTH, m_bSegmentLength);
    DDX_Check(pDX, IDC_EXTAB_MAGNITUDE, m_bMagnitude);
    DDX_Check(pDX, IDC_EXTAB_ORTHO, m_bOrtho);
    DDX_Check(pDX, IDC_EXTAB_PHONEMIC, m_bPhonemic);
    DDX_Check(pDX, IDC_EXTAB_PHONETIC, m_bPhonetic);
    DDX_Check(pDX, IDC_EXTAB_PITCH, m_bPitch);
    DDX_Check(pDX, IDC_EXTAB_REFERENCE, m_bReference);
    DDX_Check(pDX, IDC_EXTAB_START, m_bSegmentStart);
    DDX_Check(pDX, IDC_EXTAB_TIME, m_bSampleTime);
    DDX_Check(pDX, IDC_EXTAB_TONE, m_bTone);
    DDX_Check(pDX, IDC_EXTAB_ZERO, m_bZeroCrossings);
    DDX_Radio(pDX, IDC_EXTAB_SEGMENT, m_nSampleRate);
    DDX_Radio(pDX, IDC_EXTAB_MIDPOINT, m_nCalculationMethod);
    DDX_Check(pDX, IDC_EXTAB_PHONETIC2, m_bPhonetic2);
    DDX_Radio(pDX, IDC_REGION_BETWEEN, m_nRegion);
    DDX_Check(pDX, IDC_EXTAB_MELOGRAM, m_bMelogram);
}

BEGIN_MESSAGE_MAP(CDlgExportTimeTable, CDialog)
    ON_BN_CLICKED(IDC_EXTAB_ANNOTATIONS, OnAllAnnotations)
    ON_BN_CLICKED(IDC_EXTAB_FORMANTS, OnAllFormants)
    ON_BN_CLICKED(IDC_EXTAB_CALC, OnAllCalculations)
    ON_EN_UPDATE(IDC_EXTAB_INTERVAL_TIME, OnUpdateIntervalTime)
    ON_BN_CLICKED(IDC_EXTAB_INTERVAL, OnSample)
    ON_BN_CLICKED(IDC_EXTAB_SEGMENT, OnSample)
    ON_BN_CLICKED(IDC_EXTAB_PHONETIC2, OnPhonetic)
    ON_BN_CLICKED(IDC_EXTAB_PHONETIC, OnPhonetic)
    ON_COMMAND(IDHELP, OnHelpExportTable)
END_MESSAGE_MAP()

void CDlgExportTimeTable::OnAllAnnotations() {

    UpdateData(TRUE);
    if (m_nSampleRate == 0) {
        SetVisible(IDC_EXTAB_TIME, FALSE);
        SetVisible(IDC_EXTAB_START, TRUE);
        SetVisible(IDC_EXTAB_LENGTH, TRUE);
        SetVisible(IDC_EXTAB_PHONETIC2, FALSE);
        SetVisible(IDC_EXTAB_ANNOTATIONS, TRUE, TRUE);
        BOOL bEnable = !m_bAnnotations;
        SetVisible(IDC_EXTAB_PHONETIC, TRUE, bEnable);
        SetVisible(IDC_EXTAB_TONE, TRUE, bEnable);
        SetVisible(IDC_EXTAB_PHONEMIC, TRUE, bEnable);
        SetVisible(IDC_EXTAB_ORTHO, TRUE, bEnable);
        SetVisible(IDC_EXTAB_GLOSS, TRUE, bEnable);
        SetVisible(IDC_EXTAB_GLOSS_NAT, TRUE, bEnable);
        SetVisible(IDC_EXTAB_REFERENCE, TRUE, bEnable);
    } else {
        SetVisible(IDC_EXTAB_TIME, TRUE);
        SetVisible(IDC_EXTAB_START, FALSE);
        SetVisible(IDC_EXTAB_LENGTH, FALSE);
        SetVisible(IDC_EXTAB_PHONETIC2, TRUE);
        SetVisible(IDC_EXTAB_ANNOTATIONS, FALSE);
        SetVisible(IDC_EXTAB_PHONETIC, FALSE);
        SetVisible(IDC_EXTAB_TONE, FALSE);
        SetVisible(IDC_EXTAB_PHONEMIC, FALSE);
        SetVisible(IDC_EXTAB_ORTHO, FALSE);
        SetVisible(IDC_EXTAB_GLOSS, FALSE);
        SetVisible(IDC_EXTAB_GLOSS_NAT, FALSE);
        SetVisible(IDC_EXTAB_REFERENCE, FALSE);
    }
    if (m_bAnnotations) {
        m_bReference = m_bPhonetic = m_bTone = m_bPhonemic = m_bOrtho = m_bGloss = m_bGlossNat = TRUE;
        UpdateData(FALSE);
    }
}

void CDlgExportTimeTable::OnAllFormants() {

    UpdateData(TRUE);
    BOOL bEnable = !m_bFormants;
    SetEnable(IDC_EXTAB_F1, bEnable && !m_bFormants);
    SetEnable(IDC_EXTAB_F2, bEnable && !m_bFormants);
    SetEnable(IDC_EXTAB_F3, bEnable && !m_bFormants);
    SetEnable(IDC_EXTAB_F4, bEnable && !m_bFormants);
    if (m_bFormants) {
        m_bF1 = m_bF2 = m_bF3 = m_bF4 = TRUE;
        UpdateData(FALSE);
    }
}

void CDlgExportTimeTable::OnAllCalculations() {

    UpdateData(TRUE);
    BOOL bEnable = !m_bCalculations;
    SetEnable(IDC_EXTAB_FORMANTS, bEnable);
    SetEnable(IDC_EXTAB_F1, bEnable && !m_bFormants);
    SetEnable(IDC_EXTAB_F2, bEnable && !m_bFormants);
    SetEnable(IDC_EXTAB_F3, bEnable && !m_bFormants);
    SetEnable(IDC_EXTAB_F4, bEnable && !m_bFormants);
    SetEnable(IDC_EXTAB_MAGNITUDE, bEnable);
    SetEnable(IDC_EXTAB_PITCH, bEnable);
    SetEnable(IDC_EXTAB_MELOGRAM, bEnable);
    SetEnable(IDC_EXTAB_ZERO, bEnable);
    if (m_bCalculations) {
        m_bMagnitude = m_bZeroCrossings = m_bPitch = m_bMelogram  = m_bFormants = TRUE;
        UpdateData(FALSE);
        OnAllFormants();
    }
}

void CDlgExportTimeTable::OnOK() {
	UpdateData(TRUE);
    CDialog::OnOK();
}

/***************************************************************************/
// CDlgExportTimeTable::OnHelpExportTable Call Export Table help
/***************************************************************************/
void CDlgExportTimeTable::OnHelpExportTable() {

    // create the pathname
    CString szPath = AfxGetApp()->m_pszHelpFilePath;
    szPath = szPath + "::/User_Interface/Menus/File/Export/SFM_time_table.htm";
    ::HtmlHelp(NULL, szPath, HH_DISPLAY_TOPIC, NULL);
}

BOOL CDlgExportTimeTable::OnInitDialog() {

    CDialog::OnInitDialog();

    CSaDoc * pDoc = (CSaDoc *)((CMainFrame *)AfxGetMainWnd())->GetCurrSaView()->GetDocument();

    if (pDoc->GetSegment(PHONETIC)->IsEmpty()) { // no annotations
        BOOL bEnable = FALSE;
        SetEnable(IDC_EXTAB_PHONETIC, bEnable);
        SetEnable(IDC_EXTAB_TONE, bEnable);
        SetEnable(IDC_EXTAB_PHONEMIC, bEnable);
        SetEnable(IDC_EXTAB_ORTHO, bEnable);
        SetEnable(IDC_EXTAB_GLOSS, bEnable);
        SetEnable(IDC_EXTAB_GLOSS_NAT, bEnable);
        SetEnable(IDC_EXTAB_REFERENCE, bEnable);
        SetEnable(IDC_EXTAB_ANNOTATIONS, bEnable);
        SetEnable(IDC_EXTAB_LENGTH, bEnable);
        SetEnable(IDC_EXTAB_START, bEnable);
    }

    OnAllAnnotations();
    OnAllCalculations();
    OnAllFormants();

    CenterWindow();

    return TRUE;  // return TRUE  unless you set the focus to a control
}

/***************************************************************************/
// CDlgExportTimeTable::SetEnable Enables/Disables controls
/***************************************************************************/
void CDlgExportTimeTable::SetEnable(int nItem, BOOL bEnable) {

    CWnd * pWnd = GetDlgItem(nItem);
    if (pWnd) {
        pWnd->EnableWindow(bEnable);
    }
}

/***************************************************************************/
// CDlgExportTimeTable::SetVisible Sets the visibility of dialog items
/***************************************************************************/
void CDlgExportTimeTable::SetVisible(int nItem, BOOL bVisible, BOOL bEnable /*=TRUE*/) {

    CWnd * pWnd = GetDlgItem(nItem);
    if (pWnd) {
        pWnd->EnableWindow(bVisible && bEnable);// disable invisible items, enable on show
        if (bVisible) {
            pWnd->SetWindowPos(NULL, 0,0,0,0,SWP_NOMOVE+SWP_NOSIZE+SWP_NOZORDER+SWP_SHOWWINDOW);
        } else {
            pWnd->SetWindowPos(NULL, 0,0,0,0,SWP_NOMOVE+SWP_NOSIZE+SWP_NOZORDER+SWP_HIDEWINDOW);
        }
    }
}


/***************************************************************************/
// CDlgExportTimeTable::OnUpdateIntervalTime Filter edit box to only accept digits
/***************************************************************************/
void CDlgExportTimeTable::OnUpdateIntervalTime() {

    CWnd * pWnd = GetDlgItem(IDC_EXTAB_INTERVAL_TIME);
    if (pWnd!=NULL) {
        CSaString szText;
        BOOL bChanged = FALSE;

        pWnd->GetWindowText(szText);

        for (int nIndex=0; nIndex < szText.GetLength(); nIndex++) {
            if ((szText[nIndex] < '0') || (szText[nIndex] > '9')) {
                szText = szText.Left(nIndex) + szText.Mid(nIndex+1);
                bChanged = TRUE;
            }
            if (bChanged) { // only change if necessary (will cause infinite loop)
                pWnd->SetWindowText(szText);
            }
        }
    }
}


/***************************************************************************/
// CDlgExportTimeTable::OnSample 
// Disable Phonetic Sampling on empty phonetic segment and update visibility of check boxes
/***************************************************************************/
void CDlgExportTimeTable::OnSample() {

    UpdateData(TRUE);
    CSaDoc * pDoc = (CSaDoc *)((CMainFrame *)AfxGetMainWnd())->GetCurrSaView()->GetDocument();
	// no annotations
    if (pDoc->GetSegment(PHONETIC)->IsEmpty()) { 
        if (m_nSampleRate == 0) {
            AfxMessageBox(IDS_ERROR_NOSEGMENTS,MB_OK,0);
            m_nSampleRate = 1;
            UpdateData(FALSE);
        }
    }
    OnAllAnnotations();
}

/***************************************************************************/
// CDlgExportTimeTable::OnPhonetic keep invisible phonetic check box in sync with visible
/***************************************************************************/
void CDlgExportTimeTable::OnPhonetic() {

    UpdateData(TRUE);
    if (m_nSampleRate == 0) {
        m_bPhonetic2 = m_bPhonetic;
    } else {
        m_bPhonetic = m_bPhonetic2;
    }
    UpdateData(FALSE);
}

static LPCSTR psz_Phonemic = "pm";
static LPCSTR psz_Gloss = "gl";
static LPCSTR psz_GlossNat = "gn";
static LPCSTR psz_Phonetic = "ph";
static LPCSTR psz_Orthographic = "or";
static LPCSTR psz_ImportEnd = "import";

static LPCSTR psz_FreeTranslation = "ft"; // Free Translation
static LPCSTR psz_Language ="ln"; // Language Name
static LPCSTR psz_Dialect = "dlct"; // Dialect
static LPCSTR psz_Family = "fam"; // Family
static LPCSTR psz_Ethno = "id"; // Ethnologue ID number
static LPCSTR psz_Country = "cnt"; // Country
static LPCSTR psz_Region = "reg"; // Region
static LPCSTR psz_Speaker = "spkr"; // Speaker Name
static LPCSTR psz_Gender = "gen"; // Gender
static LPCSTR psz_NotebookReference = "nbr"; // Notebook Reference
static LPCSTR psz_Transcriber = "tr"; // Transcriber
static LPCSTR psz_Comments = "cmnt"; // Comments
static LPCSTR psz_Description = "desc"; // Description
static LPCSTR psz_Table = "table";

static void CreateWordSegments(const int nWord, int & nSegments) {

    CSaDoc * pDoc = (CSaDoc *)((CMainFrame *)AfxGetMainWnd())->GetCurrSaView()->GetDocument();

    if (pDoc->GetSegment(GLOSS)->GetOffsetSize() > nWord) {
        DWORD dwStart;
        DWORD dwStop;
        int nPhonetic;
        CPhoneticSegment * pPhonetic = (CPhoneticSegment *) pDoc->GetSegment(PHONETIC);

        if (nWord == -1) {
            dwStart = 0;
            if (pDoc->GetSegment(GLOSS)->IsEmpty()) {
                dwStop = pDoc->GetDataSize();
            } else {
                dwStop = pDoc->GetSegment(GLOSS)->GetOffset(0);
            }
            if (dwStart + pDoc->GetBytesFromTime(MIN_EDIT_SEGMENT_TIME) > dwStop) {
                return;
            }
            nPhonetic = 0;
        } else {
            ASSERT(nSegments);
            dwStart = pDoc->GetSegment(GLOSS)->GetOffset(nWord);
            dwStop = pDoc->GetSegment(GLOSS)->GetDuration(nWord) + dwStart;
            nPhonetic = pPhonetic->FindOffset(dwStart);
        }
        // Limit number of segments
        if (nSegments*pDoc->GetBytesFromTime(MIN_ADD_SEGMENT_TIME) > (dwStop -dwStart)) {
            nSegments = (int)((dwStop -dwStart)/pDoc->GetBytesFromTime(MIN_ADD_SEGMENT_TIME));
            if (!nSegments) {
                nSegments = 1;
            }
            if (nSegments*pDoc->GetBytesFromTime(MIN_EDIT_SEGMENT_TIME) > (dwStop -dwStart)) {
                return;
            }
        }
        // remove excess segments
        int nCount = 0;
        int nIndex = nPhonetic;
        while ((nIndex != -1) && (pPhonetic->GetOffset(nIndex) < dwStop)) {
            if (nCount >= nSegments) {
                // no checkpoint
                pPhonetic->Remove(pDoc, nIndex, FALSE);
                if (nIndex >= pPhonetic->GetOffsetSize()) {
                    break;
                }
            } else {
                DWORD dwBegin = dwStart + nCount;
                pPhonetic->Adjust(pDoc, nIndex, dwBegin, 1, false);
                nIndex = pPhonetic->GetNext(nIndex);
                nCount++;
            }
        }
        if (nSegments == 0) {
            return;    // done
        }
        // add segments
        while (nCount < nSegments) {
            if (nIndex == -1) {
                nIndex = pPhonetic->GetOffsetSize();
            }
            DWORD dwBegin = dwStart + nCount;
            CSaString szEmpty(SEGMENT_DEFAULT_CHAR);
            pPhonetic->Insert( nIndex, szEmpty, false, dwBegin, 1);
            nIndex = pPhonetic->GetNext(nIndex);
            nCount++;
        }
        // adjust segment spacing
        DWORD dwSize = (dwStop - dwStart)/nSegments;
        if (pDoc->Is16Bit()) {
            dwSize &= ~1;
        }
        dwSize += pDoc->GetBlockAlign();
        if (nIndex == -1) {
            nIndex = pPhonetic->GetOffsetSize();
        }
        nIndex = pPhonetic->GetPrevious(nIndex);
        while ((nIndex != -1)&&(pPhonetic->GetOffset(nIndex) >= dwStart)) {
            nCount--;
            DWORD dwBegin = dwStart+nCount*dwSize;
            DWORD dwDuration = dwSize;
            if ((dwBegin + dwDuration) > dwStop) {
                dwDuration = dwStop - dwBegin;
            }
            pPhonetic->Adjust(pDoc, nIndex, dwBegin, dwDuration, false);
            nIndex = pPhonetic->GetPrevious(nIndex);
        }
    }
    nSegments = 0;
}

