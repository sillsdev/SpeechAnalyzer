/////////////////////////////////////////////////////////////////////////////
// graphsParameters.cpp:
// Implementation of the CDlgParametersRawdataPage (property page)
//                       CDlgParametersPitchPage (property page)
//                       CDlgParametersSpectroPage (property page)
//                       CDlgParametersSpectrumPage (property page)
//                       CDlgParametersFormantsPage (property page)
//                       CDlgParametersMusicPage (property page)
//                       CDlgParametersSDPPage (property page)
//                       CDlgGraphsParameters (property sheet)
//
// Author: Urs Ruchti
// copyright 2000 JAARS Inc. SIL
//
// Revision History
//
// 01/12/2000
//    SDM   Extract from sa_dlg.cpp
//
/////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "graphsParameters.h"
#include "sa_doc.h"
#include "sa.h"
#include "sa_graph.h"
#include "sa_view.h"
#include "sa_plot.h"
#include "mainfrm.h"
#include "sa_g_spu.h"
#include <math.h>
#include "PrivateCursorWnd.h"
#include "DlgVowelFormants.h"
#include "Process\Process.h"
#include "Process\sa_p_spg.h"
#include "Process\sa_p_sfmt.h"
#include "Process\sa_p_cha.h"
#include "Process\sa_p_lou.h"
#include "Process\sa_p_pit.h"
#include "Process\sa_p_spi.h"
#include "Process\sa_p_cpi.h"
#include "Process\sa_p_spu.h"
#include "Process\sa_p_fmt.h"
#include "Process\FormantTracker.h"
#include "dsp\lpc.h"
#include "dsp\ZTransform.h"

//###########################################################################
// CDlgParametersRawdataPage property page
// Displays all controls to change raw data parameters.

BEGIN_MESSAGE_MAP(CDlgParametersRawdataPage, CPropertyPage)
    ON_CBN_SELCHANGE(IDC_RAWDATA_PROCESS, OnProcess)
END_MESSAGE_MAP()

CDlgParametersRawdataPage::CDlgParametersRawdataPage() : CPropertyPage(CDlgParametersRawdataPage::IDD) {

    m_nProcess = 0;
}

void CDlgParametersRawdataPage::DoDataExchange(CDataExchange * pDX) {

    CPropertyPage::DoDataExchange(pDX);
    DDX_CBIndex(pDX, IDC_RAWDATA_PROCESS, m_nProcess);
}

BOOL CDlgParametersRawdataPage::OnInitDialog() {

    CPropertyPage::OnInitDialog();

    CSaApp * pApp = static_cast<CSaApp *>(AfxGetApp());

    CWnd * pWnd = GetDlgItem(IDC_RAWDATA_PROCESS);
    pWnd->EnableWindow(pApp->GetWbDoc() != NULL);
    UpdateData(TRUE);
    return TRUE;
}

/***************************************************************************/
// CDlgParametersRawdataPage::OnProcess Process changed
/***************************************************************************/
void CDlgParametersRawdataPage::OnProcess() {

    UpdateData(TRUE);
    // check if this process already used
    if ((m_nProcess) && ((CMainFrame *)AfxGetMainWnd())->IsProcessUsed(m_nProcess)) {
        // already used, switch back to plain
        m_nProcess = 0;
        AfxMessageBox(IDS_ERROR_MULTIPLEPROCESS,MB_OK,0);
        UpdateData(FALSE);
    }
}

//###########################################################################
// CDlgParametersPitchPage property page
// Displays all controls to change pitch calculation parameters.

BEGIN_MESSAGE_MAP(CDlgParametersPitchPage, CPropertyPage)
    ON_COMMAND(IDC_PITCH_CHANGESCROLL, OnChangeScroll)
    ON_EN_KILLFOCUS(IDC_PITCH_CHANGE, OnKillfocusChange)
    ON_COMMAND(IDC_PITCH_GROUPSCROLL, OnGroupScroll)
    ON_EN_KILLFOCUS(IDC_PITCH_GROUP, OnKillfocusGroup)
    ON_COMMAND(IDC_PITCH_INTERPOLATIONSCROLL, OnInterpolScroll)
    ON_EN_KILLFOCUS(IDC_PITCH_INTERPOLATION, OnKillfocusInterpol)
    ON_COMMAND(IDC_PITCH_LOUDNESSSCROLL, OnVoicingScroll)
    ON_EN_KILLFOCUS(IDC_PITCH_LOUDNESS, OnKillfocusVoicing)
    ON_COMMAND(IDC_PITCH_MAXFREQSCROLL, OnMaxFreqScroll)
    ON_EN_KILLFOCUS(IDC_PITCH_MAXFREQ, OnKillfocusMaxfreq)
    ON_COMMAND(IDC_PITCH_MINFREQSCROLL, OnMinFreqScroll)
    ON_EN_KILLFOCUS(IDC_PITCH_MINFREQ, OnKillfocusMinfreq)
    ON_COMMAND(IDC_PITCH_UPPERBOUNDSCROLL, OnUpperBoundScroll)
    ON_EN_KILLFOCUS(IDC_PITCH_UPPERBOUNDEDIT, OnKillfocusUpperBound)
    ON_COMMAND(IDC_PITCH_LOWERBOUNDSCROLL, OnLowerBoundScroll)
    ON_EN_KILLFOCUS(IDC_PITCH_LOWERBOUNDEDIT, OnKillfocusLowerBound)
    ON_CBN_SELCHANGE(IDC_PITCH_RANGE, OnRange)
    ON_CBN_SELCHANGE(IDC_PITCH_SCALING, OnScaling)
    ON_BN_CLICKED(IDC_USEMEDIANFILTER, OnUsemedianfilter)
    ON_EN_KILLFOCUS(IDC_MEDIANFILTERSIZE, OnKillfocusMedianFilter)
    ON_EN_CHANGE(IDC_PITCH_CHANGE, OnChange)
    ON_EN_CHANGE(IDC_PITCH_GROUP, OnChange)
    ON_EN_CHANGE(IDC_PITCH_INTERPOLATION, OnChange)
    ON_EN_CHANGE(IDC_PITCH_LOUDNESS, OnChange)
    ON_EN_CHANGE(IDC_PITCH_MAXFREQ, OnChange)
    ON_EN_CHANGE(IDC_PITCH_MINFREQ, OnChange)
    ON_EN_CHANGE(IDC_PITCH_UPPERBOUNDEDIT, OnChange)
    ON_EN_CHANGE(IDC_PITCH_LOWERBOUNDEDIT, OnChange)
    ON_EN_CHANGE(IDC_MEDIANFILTERSIZE, OnChange)
    ON_BN_CLICKED(IDC_PITCH_MANUAL_DEFAULT_APP, OnPitchManualDefaultApp)
    ON_BN_CLICKED(IDC_PITCH_MANUAL_DEFAULT_FILE, OnPitchManualDefaultFile)
END_MESSAGE_MAP()

CDlgParametersPitchPage::CDlgParametersPitchPage() : CPropertyPage(CDlgParametersPitchPage::IDD) {

    m_bModified = FALSE;
}

void CDlgParametersPitchPage::DoDataExchange(CDataExchange * pDX) {

    CPropertyPage::DoDataExchange(pDX);
    CString szVoicing("");
    szVoicing.Format(_T("%5.1f"), m_fVoicing);
    szVoicing.TrimLeft();
    DDV_MinMaxInt(pDX, (int)m_fVoicing, 0, 99);
    DDX_Text(pDX, IDC_PITCH_CHANGE, m_nChange);
    DDV_MinMaxInt(pDX, m_nChange, 1, 20);
    DDX_Text(pDX, IDC_PITCH_GROUP, m_nGroup);
    DDV_MinMaxInt(pDX, m_nGroup, 1, 20);
    DDX_Text(pDX, IDC_PITCH_INTERPOLATION, m_nInterpol);
    DDV_MinMaxInt(pDX, m_nInterpol, 0, 10);
    DDX_Text(pDX, IDC_PITCH_LOUDNESS, szVoicing);
    DDX_Text(pDX, IDC_PITCH_MAXFREQ, m_nMaxFreq);
    DDV_MinMaxInt(pDX, m_nMaxFreq, 80, 2047); //CLW 10/4/99
    DDX_Text(pDX, IDC_PITCH_MINFREQ, m_nMinFreq);
    DDV_MinMaxInt(pDX, m_nMinFreq, 40, 460);
    DDX_Text(pDX, IDC_PITCH_LOWERBOUNDEDIT, m_nLowerBound);
    DDX_Text(pDX, IDC_PITCH_UPPERBOUNDEDIT, m_nUpperBound);
    DDX_CBIndex(pDX, IDC_PITCH_RANGE, m_nPitchRange);
    DDX_CBIndex(pDX, IDC_PITCH_SCALING, m_nPitchScaling);
    DDX_Text(pDX, IDC_MEDIANFILTERSIZE, m_nCepMedianFilterSize);
    DDV_MinMaxInt(pDX, m_nCepMedianFilterSize, 1, 250);
}

/***************************************************************************/
// CDlgParametersPitchPage::Apply Apply button hit
/***************************************************************************/
void CDlgParametersPitchPage::Apply() {

    if (m_bModified) { // retrieve data
        // get pointer to document
        CMainFrame * pMDIFrameWnd = (CMainFrame *)AfxGetMainWnd();
        ASSERT(pMDIFrameWnd->IsKindOf(RUNTIME_CLASS(CMainFrame)));
        CSaView * pView = (CSaView *)pMDIFrameWnd->GetCurrSaView();
        CSaDoc * pDoc = (CSaDoc *)pView->GetDocument();
        // get utt, pitch and format parameters document member data
        UttParm myUttParm;
        UttParm * pUttParm = &myUttParm;
        pDoc->GetUttParm(pUttParm); // get sa parameters utterance member data
        FmtParm * pFmtParm = pDoc->GetFmtParm();
        // save member data
        int nVoicingRaw16 = (int)(m_fVoicing * 16. * .01 * pow(2.F, pFmtParm->wBitsPerSample - 1) + 0.5);
        if (((UINT)m_nMaxFreq != pUttParm->nMaxFreq) ||
                ((UINT)m_nMinFreq != pUttParm->nMinFreq) ||
                (nVoicingRaw16 != pUttParm->nCritLoud)) {
            pUttParm->nMaxFreq = m_nMaxFreq;
            pUttParm->nMinFreq = m_nMinFreq;
            pUttParm->nCritLoud = nVoicingRaw16;
            pDoc->SetUttParm(pUttParm);
            // processed data is invalid
            pDoc->GetChange()->SetDataInvalid();
            pDoc->GetLoudness()->SetDataInvalid();
            pDoc->GetSmoothLoudness()->SetDataInvalid();
            pDoc->GetPitch()->SetDataInvalid();
            pDoc->GetCustomPitch()->SetDataInvalid();
            pDoc->GetSmoothedPitch()->SetDataInvalid();
            pDoc->SetModifiedFlag(); // document has been modified
            pDoc->SetTransModifiedFlag(TRUE); // transcription data has been modified
        }
        if ((m_nChange != pUttParm->nMaxChange) || (m_nGroup != pUttParm->nMinGroup)
                || (m_nInterpol != pUttParm->nMaxInterp)) {
            pUttParm->nMaxChange = m_nChange;
            pUttParm->nMinGroup = m_nGroup;
            pUttParm->nMaxInterp = m_nInterpol;
            pDoc->SetUttParm(pUttParm);
            pDoc->SetModifiedFlag(); // document has been modified
            pDoc->SetTransModifiedFlag(TRUE); // transcription data has been modified
            // processed data is invalid
            pDoc->GetSmoothedPitch()->SetDataInvalid();
        }
        PitchParm cPitchParm = *pDoc->GetPitchParm();
        PitchParm * pPitchParm = &cPitchParm;
        pPitchParm->nUpperBound = m_nUpperBound;
        pPitchParm->nLowerBound = m_nLowerBound;
        pPitchParm->nRangeMode = m_nPitchRange;
        if (m_nPitchRange) {
            CMainFrame * pMainFrame = (CMainFrame *)AfxGetMainWnd();
            ASSERT(pMainFrame->IsKindOf(RUNTIME_CLASS(CMainFrame)));
            PitchParm cParm = *pMainFrame->GetPitchParmDefaults();
            cParm.nManualPitchLower = m_nLowerBound;
            cParm.nManualPitchUpper = m_nUpperBound;
            pMainFrame->SetPitchParmDefaults(cParm);
        }
        pPitchParm->nScaleMode = m_nPitchScaling;
        pPitchParm->nCepMedianFilterSize = (BYTE) m_nCepMedianFilterSize;
        pPitchParm->bUseCepMedianFilter  = m_bUseCepMedianFilter;
        pDoc->SetPitchParm(cPitchParm);
        m_bModified = FALSE;
        SetModified(FALSE);
        // refresh all graphs
        pView->RefreshGraphs(TRUE, TRUE);
    }
}

BOOL CDlgParametersPitchPage::OnInitDialog() {

    CPropertyPage::OnInitDialog();
    CRect rWnd;
    // build and place the min frequency spin control
    m_SpinMinFreq.Init(IDC_PITCH_MINFREQSCROLL, this);
    // build and place the max frequency spin control
    m_SpinMaxFreq.Init(IDC_PITCH_MAXFREQSCROLL, this);
    // build and place the voicing spin control
    m_SpinVoicing.Init(IDC_PITCH_LOUDNESSSCROLL, this);
    // build and place the change spin control
    m_SpinChange.Init(IDC_PITCH_CHANGESCROLL, this);
    // build and place the group spin control
    m_SpinGroup.Init(IDC_PITCH_GROUPSCROLL, this);
    // build and place the interpolation spin control
    m_SpinInterpol.Init(IDC_PITCH_INTERPOLATIONSCROLL, this);
    // build and place the upper boundary spin control
    m_SpinUpperBound.Init(IDC_PITCH_UPPERBOUNDSCROLL, this);
    // build and place the lower boundary spin control
    m_SpinLowerBound.Init(IDC_PITCH_LOWERBOUNDSCROLL, this);
    // build an place the cepstral median filter size spin control

    /*
    // Remove the following until cepstral processing is taken care of.
    m_SpinMedianFilterSize.Init(IDC_MEDIANFILTERSIZE_SPIN, this);
    */
    // get pointer to document
    CMainFrame * pMDIFrameWnd = (CMainFrame *)AfxGetMainWnd();
    ASSERT(pMDIFrameWnd->IsKindOf(RUNTIME_CLASS(CMainFrame)));
    CSaView * pView = (CSaView *)(pMDIFrameWnd->GetCurrSaView());
    CSaDoc * pDoc = (CSaDoc *)pView->GetDocument();
    m_nFreqLimit = 0x7FFF / PRECISION_MULTIPLIER;
    // get utt, pitch and format parameters document member data
    const UttParm * pUttParm = pDoc->GetUttParm();
    const PitchParm * pPitchParm = pDoc->GetPitchParm();
    FmtParm * pFmtParm = pDoc->GetFmtParm();
    // initialize member data
    m_nCepMedianFilterSize = pPitchParm->nCepMedianFilterSize;
    m_bUseCepMedianFilter  = pPitchParm->bUseCepMedianFilter;
    m_nChange = pUttParm->nMaxChange;
    if (m_nChange > 20) {
        m_nChange = 20;
    }
    if (m_nChange < 1) {
        m_nChange = 1;
    }
    m_nGroup = pUttParm->nMinGroup;
    if (m_nGroup > 20) {
        m_nGroup = 20;
    }
    if (m_nGroup < 1) {
        m_nGroup = 1;
    }
    m_nInterpol = pUttParm->nMaxInterp;
    if (m_nInterpol > 10) {
        m_nInterpol = 10;
    }
    if (m_nInterpol < 0) {
        m_nInterpol = 0;
    }
    m_fVoicing = (double)pUttParm->nCritLoud / 16. / pow(2.F, pFmtParm->wBitsPerSample - 1) * 100.;
    if (m_fVoicing > 5.0) {
        m_fVoicing = 5.0;
    }
    if (m_fVoicing < 0.) {
        m_fVoicing = 0.;
    }
    int nVoicingRaw16 = (int)(m_fVoicing * 16. * .01 * pow(2.F, pFmtParm->wBitsPerSample - 1) + 0.5);
    m_nMaxFreq = pUttParm->nMaxFreq;
    if (m_nMaxFreq > 2047) {
        m_nMaxFreq = 2047;    //CLW 10/4/99
    }
    if (m_nMaxFreq < 80) {
        m_nMaxFreq = 80;
    }
    m_nMinFreq = pUttParm->nMinFreq;
    if (m_nMinFreq > 460) {
        m_nMinFreq = 460;
    }
    if (m_nMinFreq < 40) {
        m_nMinFreq = 40;
    }
    if (m_nMinFreq > (m_nMaxFreq - 40)) {
        m_nMinFreq = m_nMaxFreq - 40;
    }
    if (m_nMaxFreq < (m_nMinFreq + 40)) {
        m_nMaxFreq = m_nMinFreq + 40;
    }
    if ((m_nChange != pUttParm->nMaxChange) || (m_nGroup != pUttParm->nMinGroup)
            || (m_nInterpol != pUttParm->nMaxInterp) || (nVoicingRaw16 != pUttParm->nCritLoud)
            || (m_nMaxFreq != (int)pUttParm->nMaxFreq) || (m_nMinFreq != (int)pUttParm->nMinFreq)) {
        m_bModified = TRUE;
        SetModified(TRUE); // data modified, enable apply button
    }
    m_nUpperBound = pPitchParm->nUpperBound;
    m_nLowerBound = pPitchParm->nLowerBound;

    CMainFrame * pMainFrame = (CMainFrame *)AfxGetMainWnd();
    ASSERT(pMainFrame->IsKindOf(RUNTIME_CLASS(CMainFrame)));
    m_nManualLowerBound = pMainFrame->GetPitchParmDefaults()->nManualPitchLower;
    m_nManualUpperBound = pMainFrame->GetPitchParmDefaults()->nManualPitchUpper;

    m_nPitchRange = pPitchParm->nRangeMode;
    m_nPitchScaling = pPitchParm->nScaleMode;
    if (!m_nPitchRange) {
        PitchParm::GetAutoRange(pDoc, m_nUpperBound, m_nLowerBound);

        // auto range, disable boundaries
        GetDlgItem(IDC_PITCH_UPPERBOUNDTITLE)->EnableWindow(FALSE);
        GetDlgItem(IDC_PITCH_UPPERBOUNDEDIT)->EnableWindow(FALSE);
        GetDlgItem(IDC_PITCH_UPPERBOUNDTEXT)->EnableWindow(FALSE);
        m_SpinUpperBound.EnableWindow(FALSE);
        GetDlgItem(IDC_PITCH_LOWERBOUNDTITLE)->EnableWindow(FALSE);
        GetDlgItem(IDC_PITCH_LOWERBOUNDEDIT)->EnableWindow(FALSE);
        GetDlgItem(IDC_PITCH_LOWERBOUNDTEXT)->EnableWindow(FALSE);
        m_SpinLowerBound.EnableWindow(FALSE);

    }
    // Enable manual analysis control for custom or smooth pitch graphs.
    UINT nFocusedID = pView->GetFocusedGraphID();

    if (nFocusedID != IDD_CEPPITCH && nFocusedID != IDD_SMPITCH
            && nFocusedID != IDD_CHPITCH && nFocusedID != IDD_PITCH) {
        GetDlgItem(IDC_PITCH_MANUAL_GROUP)->EnableWindow(FALSE);
        GetDlgItem(IDC_PITCH_MINFREQTXT)->EnableWindow(FALSE);
        GetDlgItem(IDC_PITCH_MINFREQ)->EnableWindow(FALSE);
        GetDlgItem(IDC_PITCH_MINFREQDIM)->EnableWindow(FALSE);
        m_SpinMinFreq.EnableWindow(FALSE);
        GetDlgItem(IDC_PITCH_MAXFREQTXT)->EnableWindow(FALSE);
        GetDlgItem(IDC_PITCH_MAXFREQ)->EnableWindow(FALSE);
        GetDlgItem(IDC_PITCH_MAXFREQDIM)->EnableWindow(FALSE);
        m_SpinMaxFreq.EnableWindow(FALSE);
        GetDlgItem(IDC_PITCH_LOUDNESSTXT)->EnableWindow(FALSE);
        GetDlgItem(IDC_PITCH_LOUDNESS)->EnableWindow(FALSE);
        m_SpinVoicing.EnableWindow(FALSE);
        GetDlgItem(IDC_PITCH_MANUAL_DEFAULT_FILE)->EnableWindow(FALSE);
        GetDlgItem(IDC_PITCH_MANUAL_DEFAULT_APP)->EnableWindow(FALSE);
    }

    if (nFocusedID != IDD_CEPPITCH && nFocusedID != IDD_SMPITCH && nFocusedID != IDD_CHPITCH) {
        GetDlgItem(IDC_PITCH_CHANGETXT)->EnableWindow(FALSE);
        GetDlgItem(IDC_PITCH_CHANGE)->EnableWindow(FALSE);
        GetDlgItem(IDC_PITCH_CHANGEDIM)->EnableWindow(FALSE);
        m_SpinChange.EnableWindow(FALSE);
        GetDlgItem(IDC_PITCH_GROUPTXT)->EnableWindow(FALSE);
        GetDlgItem(IDC_PITCH_GROUP)->EnableWindow(FALSE);
        m_SpinGroup.EnableWindow(FALSE);
        GetDlgItem(IDC_PITCH_INTERPOLATIONTXT)->EnableWindow(FALSE);
        GetDlgItem(IDC_PITCH_INTERPOLATION)->EnableWindow(FALSE);
        m_SpinInterpol.EnableWindow(FALSE);
    }

    UpdateData(FALSE);
    return TRUE;
}

/***************************************************************************/
// CDlgParametersPitchPage::OnSetActive  RLJ 1.5Test10.9B
/***************************************************************************/
BOOL CDlgParametersPitchPage::OnSetActive() {

    CPropertyPage::OnSetActive();
    return TRUE;
}

/***************************************************************************/
// CDlgParametersPitchPage::OnChangeScroll Change spin control hit
/***************************************************************************/
void CDlgParametersPitchPage::OnChangeScroll() {

    int nData = GetDlgItemInt(IDC_PITCH_CHANGE, NULL, TRUE);
    if (m_SpinChange.UpperButtonClicked()) {
        nData++;
    } else {
        nData--;
    }
    if (nData > 20) {
        nData = 20;
    }
    if (nData < 1) {
        nData = 1;
    }
    if (nData != m_nChange) {
        m_nChange = nData;
        m_bModified = TRUE;
        SetModified(TRUE); // data modified, enable apply button
        SetDlgItemInt(IDC_PITCH_CHANGE, m_nChange, TRUE);
    }
}

/***************************************************************************/
// CDlgParametersPitchPage::OnChange page changed
/***************************************************************************/
void CDlgParametersPitchPage::OnChange() {

    m_bModified = TRUE;
    SetModified(TRUE); // data modified, enable apply button
}

/***************************************************************************/
// CDlgParametersPitchPage::OnKillfocusChange Change changed
/***************************************************************************/
void CDlgParametersPitchPage::OnKillfocusChange() {

    int nData = GetDlgItemInt(IDC_PITCH_CHANGE, NULL, TRUE);
    if (nData > 20) {
        nData = 20;
    }
    if (nData < 1) {
        nData = 1;
    }
    if (nData != m_nChange) {
        m_nChange = nData;
        m_bModified = TRUE;
        SetModified(TRUE); // data modified, enable apply button
    }
    SetDlgItemInt(IDC_PITCH_CHANGE, m_nChange, TRUE);
}

/***************************************************************************/
// CDlgParametersPitchPage::OnGroupScroll Group spin control hit
/***************************************************************************/
void CDlgParametersPitchPage::OnGroupScroll() {

    int nData = GetDlgItemInt(IDC_PITCH_GROUP, NULL, TRUE);
    if (m_SpinGroup.UpperButtonClicked()) {
        nData++;
    } else {
        nData--;
    }
    if (nData > 20) {
        nData = 20;
    }
    if (nData < 1) {
        nData = 1;
    }
    if (nData != m_nGroup) {
        m_nGroup = nData;
        m_bModified = TRUE;
        SetModified(TRUE); // data modified, enable apply button
        SetDlgItemInt(IDC_PITCH_GROUP, m_nGroup, TRUE);
    }
}

/***************************************************************************/
// CDlgParametersPitchPage::OnKillfocusGroup Group changed
/***************************************************************************/
void CDlgParametersPitchPage::OnKillfocusGroup() {

    int nData = GetDlgItemInt(IDC_PITCH_GROUP, NULL, TRUE);
    if (nData > 20) {
        nData = 20;
    }
    if (nData < 1) {
        nData = 1;
    }
    if (nData != m_nGroup) {
        m_nGroup = nData;
        m_bModified = TRUE;
        SetModified(TRUE); // data modified, enable apply button
    }
    SetDlgItemInt(IDC_PITCH_GROUP, m_nGroup, TRUE);
}

/***************************************************************************/
// CDlgParametersPitchPage::OnInterpolScroll Interpol spin control hit
/***************************************************************************/
void CDlgParametersPitchPage::OnInterpolScroll() {

    int nData = GetDlgItemInt(IDC_PITCH_INTERPOLATION, NULL, TRUE);
    if (m_SpinInterpol.UpperButtonClicked()) {
        nData++;
    } else {
        nData--;
    }
    if (nData > 10) {
        nData = 10;
    }
    if (nData < 0) {
        nData = 0;
    }
    if (nData != m_nInterpol) {
        m_nInterpol = nData;
        m_bModified = TRUE;
        SetModified(TRUE); // data modified, enable apply button
        SetDlgItemInt(IDC_PITCH_INTERPOLATION, m_nInterpol, TRUE);
    }
}

/***************************************************************************/
// CDlgParametersPitchPage::OnKillfocusInterpol Interpol changed
/***************************************************************************/
void CDlgParametersPitchPage::OnKillfocusInterpol() {

    int nData = GetDlgItemInt(IDC_PITCH_INTERPOLATION, NULL, TRUE);
    if (nData > 10) {
        nData = 10;
    }
    if (nData < 0) {
        nData = 0;
    }
    if (nData != m_nInterpol) {
        m_nInterpol = nData;
        m_bModified = TRUE;
        SetModified(TRUE); // data modified, enable apply button
    }
    SetDlgItemInt(IDC_PITCH_INTERPOLATION, m_nInterpol, TRUE);
}

/***************************************************************************/
// CDlgParametersPitchPage::OnVoicingScroll Voicing spin control hit
/***************************************************************************/
void CDlgParametersPitchPage::OnVoicingScroll() {

    CString szData("");
    GetDlgItemText(IDC_PITCH_LOUDNESS, szData.GetBuffer(_MAX_PATH), 5);
    double fData = 0;

    swscanf_s(szData.GetBuffer(_MAX_PATH), _T("%lf"), &fData);

    if (m_SpinVoicing.UpperButtonClicked()) {
        fData += 0.1;
    } else {
        fData -= 0.1;
    }
    if (fData > 5.0) {
        fData = 5.0;
    }
    if (fData < 0.) {
        fData = 0.;
    }
    if (fData != m_fVoicing) {
        m_fVoicing = fData;
        m_bModified = TRUE;
        SetModified(TRUE); // data modified, enable apply button
        szData.Format(_T("%5.1f"), fData);
        szData.TrimLeft();
        SetDlgItemText(IDC_PITCH_LOUDNESS, szData);
    }
}

/***************************************************************************/
// CDlgParametersPitchPage::OnKillfocusVoicing Voicing changed
/***************************************************************************/
void CDlgParametersPitchPage::OnKillfocusVoicing() {

    CString szData("");
    GetDlgItemText(IDC_PITCH_LOUDNESS, szData.GetBuffer(_MAX_PATH), 5);
    double fData = 0;

    swscanf_s(szData.GetBuffer(_MAX_PATH), _T("%lf"), &fData);

    if (fData > 5.0) {
        fData = 5.0;
    }
    if (fData < 0.) {
        fData = 0.;
    }
    if (fData != m_fVoicing) {
        m_fVoicing = fData;
        m_bModified = TRUE;
        SetModified(TRUE); // data modified, enable apply button
    }
    szData.Format(_T("%5.1f"), fData);
    szData.TrimLeft();
    SetDlgItemText(IDC_PITCH_LOUDNESS, szData);
}

/***************************************************************************/
// CDlgParametersPitchPage::OnMaxFreqScroll Max frequency spin control hit
/***************************************************************************/
void CDlgParametersPitchPage::OnMaxFreqScroll() {

    int nData = GetDlgItemInt(IDC_PITCH_MAXFREQ, NULL, TRUE);
    if (m_SpinMaxFreq.UpperButtonClicked()) {
        nData++;
    } else {
        nData--;
    }
    if (nData > 2047) {
        nData = 2047;    //CLW 10/4/99
    }
    if (nData < 80) {
        nData = 80;
    }
    if (nData != m_nMaxFreq) {
        m_nMaxFreq = nData;
        m_bModified = TRUE;
        SetModified(TRUE); // data modified, enable apply button
        SetDlgItemInt(IDC_PITCH_MAXFREQ, m_nMaxFreq, TRUE);
        if (m_nMinFreq > (m_nMaxFreq * 4 / 5)) {
            m_nMinFreq = m_nMaxFreq * 4 / 5;
            SetDlgItemInt(IDC_PITCH_MINFREQ, m_nMinFreq, TRUE);
        }
    }
}

/***************************************************************************/
// CDlgParametersPitchPage::OnKillfocusMaxfreq Max frequency changed
/***************************************************************************/
void CDlgParametersPitchPage::OnKillfocusMaxfreq() {

    BOOL bTrans; //CLW 8/27/99
    int nData = (int)GetDlgItemInt(IDC_PITCH_MAXFREQ, &bTrans, TRUE); //CLW 8/27/99
    if (!bTrans && (nData==0)) {
        nData = 460;    //CLW 8/27/99
    }
    if (nData > 460) {
        nData = 460;
    }
    if (nData < 40) {
        nData = 40;
    }
    if (nData != m_nMaxFreq) {
        m_nMaxFreq = nData;
        m_bModified = TRUE;
        SetModified(TRUE); // data modified, enable apply button
        if (m_nMinFreq > (m_nMaxFreq * 4 / 5)) {
            m_nMinFreq = m_nMaxFreq * 4 / 5;
            SetDlgItemInt(IDC_PITCH_MINFREQ, m_nMinFreq, TRUE);
        }
    }
    SetDlgItemInt(IDC_PITCH_MAXFREQ, m_nMaxFreq, TRUE);
}

/***************************************************************************/
// CDlgParametersPitchPage::OnMinFreqScroll Min frequency spin control hit
/***************************************************************************/
void CDlgParametersPitchPage::OnMinFreqScroll() {

    int nData = GetDlgItemInt(IDC_PITCH_MINFREQ, NULL, TRUE);
    if (m_SpinMinFreq.UpperButtonClicked()) {
        nData++;
    } else {
        nData--;
    }
    if (nData > 460) {
        nData = 460;
    }
    if (nData < 40) {
        nData = 40;
    }
    if (nData != m_nMinFreq) {
        m_nMinFreq = nData;
        m_bModified = TRUE;
        SetModified(TRUE); // data modified, enable apply button
        SetDlgItemInt(IDC_PITCH_MINFREQ, m_nMinFreq, TRUE);
        if (m_nMaxFreq < ceil(m_nMinFreq * 5 / 4.)) {
            m_nMaxFreq = (int)(ceil(m_nMinFreq * 5 / 4.));
            SetDlgItemInt(IDC_PITCH_MAXFREQ, m_nMaxFreq, TRUE);
        }
    }
}

/***************************************************************************/
// CDlgParametersPitchPage::OnKillfocusMinfreq Min frequency changed
/***************************************************************************/
void CDlgParametersPitchPage::OnKillfocusMinfreq() {

    BOOL bTrans; //CLW 8/27/99
    int nData = (int)GetDlgItemInt(IDC_PITCH_MINFREQ, &bTrans, TRUE); //CLW 8/27/99
    if (!bTrans && (nData==0)) {
        nData = 460;    //CLW 8/27/99
    }
    if (nData > 460) {
        nData = 460;
    }
    if (nData < 40) {
        nData = 40;
    }
    if (nData != m_nMinFreq) {
        m_nMinFreq = nData;
        m_bModified = TRUE;
        SetModified(TRUE); // data modified, enable apply button
        if (m_nMaxFreq < ceil(m_nMinFreq * 5 / 4.)) {
            m_nMaxFreq = (int)(ceil(m_nMinFreq * 5 / 4.));
            SetDlgItemInt(IDC_PITCH_MAXFREQ, m_nMaxFreq, TRUE);
        }
    }
    SetDlgItemInt(IDC_PITCH_MINFREQ, m_nMinFreq, TRUE);
}




/***************************************************************************/
// CDlgParametersPitchPage::OnKillfocusMedianFilter
/***************************************************************************/
void CDlgParametersPitchPage::OnKillfocusMedianFilter() {

    int nData = GetDlgItemInt(IDC_MEDIANFILTERSIZE, NULL, TRUE);

    if (nData > 250) {
        nData = 250;
    }
    if (nData < 1) {
        nData = 1;
    }

    if (nData != m_nCepMedianFilterSize) {
        m_nCepMedianFilterSize = nData;
        m_bModified = TRUE;
        SetModified(TRUE); // data modified, enable apply button
    }
    SetDlgItemInt(IDC_MEDIANFILTERSIZE, m_nCepMedianFilterSize, TRUE);
}



/***************************************************************************/
// CDlgParametersPitchPage::OnUpperBoundScroll Upper boundary spin control hit
/***************************************************************************/
void CDlgParametersPitchPage::OnUpperBoundScroll() {

    if (m_nPitchRange) {
        int nData = GetDlgItemInt(IDC_PITCH_UPPERBOUNDEDIT, NULL, TRUE);
        if (m_SpinUpperBound.UpperButtonClicked()) {
            nData++;
        } else {
            nData--;
        }
        // RLJ 09/21/2000: Bug GPI-03
        //      if (nData > m_nFreqLimit) nData = m_nFreqLimit;
        if (nData > CECIL_PITCH_MAXFREQ) {
            nData = CECIL_PITCH_MAXFREQ;
        }
        if (nData < 100) {
            nData = 100;
        }
        if (nData != m_nUpperBound) {
            m_nUpperBound = nData;
            m_bModified = TRUE;
            SetModified(TRUE); // data modified, enable apply button
            SetDlgItemInt(IDC_PITCH_UPPERBOUNDEDIT, m_nUpperBound, TRUE);
            if (m_nLowerBound >= m_nUpperBound) {
                m_nLowerBound = m_nUpperBound - 1;
                SetDlgItemInt(IDC_PITCH_LOWERBOUNDEDIT, m_nLowerBound, TRUE);
            }
        }
    }
}

/***************************************************************************/
// CDlgParametersPitchPage::OnKillfocusUpperBound Upper boundary changed
/***************************************************************************/
void CDlgParametersPitchPage::OnKillfocusUpperBound() {

    BOOL bTrans; //CLW 8/27/99
    int nData = (int)GetDlgItemInt(IDC_PITCH_UPPERBOUNDEDIT, &bTrans, TRUE); //CLW 8/27/99
    // RLJ 09/21/2000: Bug GPI-03
    //    if (!bTrans && (nData==0)) nData = m_nFreqLimit; //CLW 8/27/99
    //    if (nData > m_nFreqLimit) nData = m_nFreqLimit;
    if (!bTrans && (nData==0)) {
        nData = CECIL_PITCH_MAXFREQ;    //CLW 8/27/99
    }
    if (nData > CECIL_PITCH_MAXFREQ) {
        nData = CECIL_PITCH_MAXFREQ;
    }
    if (nData < 100) {
        nData = 100;
    }
    if (nData != m_nUpperBound) {
        m_nUpperBound = nData;
        m_bModified = TRUE;
        SetModified(TRUE); // data modified, enable apply button
        if (m_nLowerBound >= m_nUpperBound) {
            m_nLowerBound = m_nUpperBound - 1;
            SetDlgItemInt(IDC_PITCH_LOWERBOUNDEDIT, m_nLowerBound, TRUE);
        }
    }
    SetDlgItemInt(IDC_PITCH_UPPERBOUNDEDIT, m_nUpperBound, TRUE);
}

/***************************************************************************/
// CDlgParametersPitchPage::OnLowerBoundScroll Lower boundary spin control hit
/***************************************************************************/
void CDlgParametersPitchPage::OnLowerBoundScroll() {

    if (m_nPitchRange) {
        int nData = GetDlgItemInt(IDC_PITCH_LOWERBOUNDEDIT, NULL, TRUE);
        if (m_SpinLowerBound.UpperButtonClicked()) {
            nData++;
        } else {
            nData--;
        }
        // RLJ 09/21/2000: Bug GPI-03
        //      if (nData > (m_nFreqLimit - 1)) nData = m_nFreqLimit - 1;
        if (nData > (CECIL_PITCH_MAXFREQ - 1)) {
            nData = CECIL_PITCH_MAXFREQ - 1;
        }
        if (nData < 10) {
            nData = 10;
        }
        if (nData != m_nLowerBound) {
            m_nLowerBound = nData;
            m_bModified = TRUE;
            SetModified(TRUE); // data modified, enable apply button
            SetDlgItemInt(IDC_PITCH_LOWERBOUNDEDIT, m_nLowerBound, TRUE);
            if (m_nUpperBound <= m_nLowerBound) {
                m_nUpperBound = m_nLowerBound + 1;
                SetDlgItemInt(IDC_PITCH_UPPERBOUNDEDIT, m_nUpperBound, TRUE);
            }
        }
    }
}



/***************************************************************************/
// CDlgParametersPitchPage::OnMedianFilterSizeSpin size spin hit.
/***************************************************************************/
void CDlgParametersPitchPage::OnMedianFilterSizeSpin() {

    int nData = GetDlgItemInt(IDC_MEDIANFILTERSIZE, NULL, TRUE);

    if (m_SpinMedianFilterSize.UpperButtonClicked()) {
        nData++;
    } else {
        nData--;
    }

    if (nData > 250) {
        nData = 250;
    }
    if (nData < 1) {
        nData = 1;
    }

    if (nData != m_nCepMedianFilterSize) {
        m_nCepMedianFilterSize = nData;
        m_bModified = TRUE;
        SetModified(TRUE); // data modified, enable apply button
        SetDlgItemInt(IDC_MEDIANFILTERSIZE, m_nCepMedianFilterSize, TRUE);
    }
}

/***************************************************************************/
// CDlgParametersPitchPage::OnKillfocusLowerBound Lower boundary changed
/***************************************************************************/
void CDlgParametersPitchPage::OnKillfocusLowerBound() {

    int nData = GetDlgItemInt(IDC_PITCH_LOWERBOUNDEDIT, NULL, TRUE);
    // RLJ 09/21/2000: Bug GPI-03CECIL_PITCH_MAXFREQ
    //    if (nData > (m_nFreqLimit - 1)) nData = m_nFreqLimit - 1;
    if (nData > (CECIL_PITCH_MAXFREQ - 1)) {
        nData = CECIL_PITCH_MAXFREQ - 1;
    }
    if (nData < 10) {
        nData = 10;
    }
    if (nData != m_nLowerBound) {
        m_nLowerBound = nData;
        m_bModified = TRUE;
        SetModified(TRUE); // data modified, enable apply button
        if (m_nUpperBound <= m_nLowerBound) {
            m_nUpperBound = m_nLowerBound + 1;
            SetDlgItemInt(IDC_PITCH_UPPERBOUNDEDIT, m_nUpperBound, TRUE);
        }
    }
    SetDlgItemInt(IDC_PITCH_LOWERBOUNDEDIT, m_nLowerBound, TRUE);
}

/***************************************************************************/
// CDlgParametersPitchPage::OnRange Range selection changed
/***************************************************************************/
void CDlgParametersPitchPage::OnRange() {

    int nOldPitchRange = m_nPitchRange;
    UpdateData(TRUE);

    if (nOldPitchRange == m_nPitchRange) {
        return;    // no change
    }

    if (!m_nPitchRange) {
        m_nManualLowerBound = m_nLowerBound;
        m_nManualUpperBound = m_nUpperBound;

        CMainFrame * pMDIFrameWnd = (CMainFrame *)AfxGetMainWnd();
        ASSERT(pMDIFrameWnd->IsKindOf(RUNTIME_CLASS(CMainFrame)));
        CSaView * pView = (CSaView *)(pMDIFrameWnd->GetCurrSaView());
        CSaDoc * pDoc = (CSaDoc *)pView->GetDocument();

        PitchParm::GetAutoRange(pDoc, m_nUpperBound, m_nLowerBound);

        GetDlgItem(IDC_PITCH_UPPERBOUNDTITLE)->EnableWindow(FALSE);
        GetDlgItem(IDC_PITCH_UPPERBOUNDEDIT)->EnableWindow(FALSE);
        GetDlgItem(IDC_PITCH_UPPERBOUNDTEXT)->EnableWindow(FALSE);
        m_SpinUpperBound.EnableWindow(FALSE);

        GetDlgItem(IDC_PITCH_LOWERBOUNDTITLE)->EnableWindow(FALSE);
        GetDlgItem(IDC_PITCH_LOWERBOUNDEDIT)->EnableWindow(FALSE);
        GetDlgItem(IDC_PITCH_LOWERBOUNDTEXT)->EnableWindow(FALSE);
        m_SpinLowerBound.EnableWindow(FALSE);
    } else {
        // manual range, enable boundaries
        GetDlgItem(IDC_PITCH_UPPERBOUNDTITLE)->EnableWindow(TRUE);
        GetDlgItem(IDC_PITCH_UPPERBOUNDEDIT)->EnableWindow(TRUE);
        GetDlgItem(IDC_PITCH_UPPERBOUNDTEXT)->EnableWindow(TRUE);
        m_SpinUpperBound.EnableWindow(TRUE);
        GetDlgItem(IDC_PITCH_LOWERBOUNDTITLE)->EnableWindow(TRUE);
        GetDlgItem(IDC_PITCH_LOWERBOUNDEDIT)->EnableWindow(TRUE);
        GetDlgItem(IDC_PITCH_LOWERBOUNDTEXT)->EnableWindow(TRUE);
        m_SpinLowerBound.EnableWindow(TRUE);
        m_nLowerBound = m_nManualLowerBound;
        m_nUpperBound = m_nManualUpperBound;
    }
    m_bModified = TRUE;
    SetModified(TRUE); // data modified, enable apply button
    UpdateData(FALSE);
}

/***************************************************************************/
// CDlgParametersPitchPage::OnScaling Scale mode changed
/***************************************************************************/
void CDlgParametersPitchPage::OnScaling() {

    UpdateData(TRUE);
    m_bModified = TRUE;
    SetModified(TRUE); // data modified, enable apply button
}

/***************************************************************************/
// CDlgParametersPitchPage::EnableCepstralSettings Enable cepstral pitch settings
/***************************************************************************/
void CDlgParametersPitchPage::EnableCepstralSettings(BOOL enable) {

    GetDlgItem(IDC_CEPSTRAL_BOX)->EnableWindow(enable);
    GetDlgItem(IDC_USEMEDIANFILTER)->EnableWindow(enable);
    EnableCepMedianSize(enable && m_bUseCepMedianFilter);
}

/***************************************************************************/
// CDlgParametersPitchPage::EnableCepMedianSize Enable median filter spin control
/***************************************************************************/
void CDlgParametersPitchPage::EnableCepMedianSize(BOOL enable) {

    m_SpinMedianFilterSize.EnableWindow(enable);
    GetDlgItem(IDC_MEDIANFILTERSIZE)->EnableWindow(enable);
    GetDlgItem(IDC_MEDIANFILTERSIZE_TEXT)->EnableWindow(enable);
}

/***************************************************************************/
// CDlgParametersPitchPage::OnUsemedianFilter Handle median filter checkbox
/***************************************************************************/
void CDlgParametersPitchPage::OnUsemedianfilter() {

    m_bUseCepMedianFilter = (BOOL)IsDlgButtonChecked(IDC_USEMEDIANFILTER);
    SetModified(TRUE); // data modified, enable apply button
    m_bModified = TRUE;
    EnableCepMedianSize(m_bUseCepMedianFilter);
}



/***************************************************************************/
// OnPitchManualDefaultFile::OnPitchManualDefaultFile Set items to file values
/***************************************************************************/
void CDlgParametersPitchPage::OnPitchManualDefaultFile() {

    OnPitchManualDefault(FALSE);
}

/***************************************************************************/
// CDlgParametersPitchPage::OnPitchManualDefaultApp items to default app values
/***************************************************************************/
void CDlgParametersPitchPage::OnPitchManualDefaultApp() {

    OnPitchManualDefault(TRUE);
}

void CDlgParametersPitchPage::OnPitchManualDefault(BOOL bAppDefaults) {
    UttParm UttDefault;
    UttParm * pUttParm = &UttDefault;

    CMainFrame * pMDIFrameWnd = (CMainFrame *)AfxGetMainWnd();
    ASSERT(pMDIFrameWnd->IsKindOf(RUNTIME_CLASS(CMainFrame)));
    CSaView * pView = (CSaView *)pMDIFrameWnd->GetCurrSaView();
    CSaDoc * pDoc = (CSaDoc *)pView->GetDocument();
    // get format parameters
    FmtParm * pFmtParm = pDoc->GetFmtParm();
    if (bAppDefaults) {
        // initialize utterance parameters
        pUttParm->Init(pFmtParm->wBitsPerSample);
    } else {
        pDoc->GetUttParm(pUttParm, TRUE);  // Get file original utt parm settings
    }

    // calculate raw voicing value
    int nVoicingRaw16 = (int)(m_fVoicing * 16. * .01 * pow(2.F, pFmtParm->wBitsPerSample - 1) + 0.5);

    if ((m_nChange != pUttParm->nMaxChange) || (m_nGroup != pUttParm->nMinGroup)
            || (m_nInterpol != pUttParm->nMaxInterp) || (nVoicingRaw16 != pUttParm->nCritLoud)
            || (m_nMaxFreq != (int)pUttParm->nMaxFreq) || (m_nMinFreq != (int)pUttParm->nMinFreq)) {
        m_bModified = TRUE;
        SetModified(TRUE); // data modified, enable apply button
        m_nChange = pUttParm->nMaxChange;
        m_nGroup = pUttParm->nMinGroup;
        m_nInterpol = pUttParm->nMaxInterp;
        m_fVoicing = pUttParm->nCritLoud / 16. / pow(2.F, pFmtParm->wBitsPerSample - 1) * 100.;
        m_nMaxFreq = pUttParm->nMaxFreq;
        m_nMinFreq = pUttParm->nMinFreq;
        UpdateData(FALSE);
    }
}

//###########################################################################
// CDlgParametersSpectroPage property page
// Displays all controls to change spectrogram calculation parameters.

IMPLEMENT_DYNAMIC(CDlgParametersSpectroPage, CPropertyPage)

BEGIN_MESSAGE_MAP(CDlgParametersSpectroPage, CPropertyPage)
    ON_BN_CLICKED(IDC_COLOR, OnModifiedColor)
    ON_BN_CLICKED(IDC_COARSE, OnModifiedResolution)
    ON_BN_CLICKED(IDC_OVERLAID, OnModifiedOverlay)
    ON_BN_CLICKED(IDC_SMOOTH_SPECTRA, OnChange)
    ON_WM_HSCROLL()
    ON_BN_CLICKED(IDC_F1, OnModifiedFormants)
    ON_BN_CLICKED(IDC_F2, OnModifiedFormants)
    ON_BN_CLICKED(IDC_F3, OnModifiedFormants)
    ON_BN_CLICKED(IDC_F4, OnModifiedFormants)
    ON_BN_CLICKED(IDC_MONO, OnModifiedColor)
    ON_BN_CLICKED(IDC_FINE, OnModifiedResolution)
    ON_BN_CLICKED(IDC_ONLY, OnModifiedOverlay)
    ON_BN_CLICKED(IDC_MEDIUM, OnModifiedResolution)
    ON_BN_CLICKED(IDC_FORMANTS, OnModifiedFormants)
    ON_BN_CLICKED(IDC_MORE_INFO, OnMoreInfo)
    ON_BN_CLICKED(IDC_FORMANT_COLOR, OnChange)
    ON_BN_CLICKED(IDC_SHOW_PITCH, OnModifiedFormants)
END_MESSAGE_MAP()

CDlgParametersSpectroPage::CDlgParametersSpectroPage(UINT nID, UINT graphId, const CSpectroParm * pSpectroParmDefaults) : 
CPropertyPage(CDlgParametersSpectroPage::IDD, nID) {

    m_GraphId = graphId;
    m_bShowPitch = FALSE;
    m_nResolution = (UINT) pSpectroParmDefaults->nResolution;
    m_nColor = (UINT) pSpectroParmDefaults->nColor;
    m_bF1 = (UINT) pSpectroParmDefaults->bShowF1;
    m_bF2 = (UINT) pSpectroParmDefaults->bShowF2;
    m_bF3 = (UINT) pSpectroParmDefaults->bShowF3;
    m_bF4 = (UINT) pSpectroParmDefaults->bShowF4;
    m_bSmoothFormantTracks = (UINT) pSpectroParmDefaults->bSmoothFormantTracks;
    m_nOverlay = (UINT) pSpectroParmDefaults->nOverlay;
    m_bSmoothSpectra = pSpectroParmDefaults->bSmoothSpectra;
    m_bFormantColor = pSpectroParmDefaults->bFormantColor;
    m_bShowPitch = pSpectroParmDefaults->bShowPitch;
    m_bFormants = m_bShowPitch || m_bF1 || m_bF2 || m_bF3 || m_bF4;
    m_nFrequency = (UINT) pSpectroParmDefaults->nFrequency;
    m_nMaxThreshold = (UINT) pSpectroParmDefaults->nMaxThreshold;
    m_nMinThreshold = (UINT) pSpectroParmDefaults->nMinThreshold;
    m_bModified = FALSE;
}

/*
Do the work of applying the settings in the dialog page.
m_GrapId should be the graph to operate on (which spectrogram).
Rationale: Simplify the Apply() function in SpectroPageA&B.
*/
void CDlgParametersSpectroPage::Apply() {

    if (m_bModified) { // retrieve data
        UpdateData(TRUE); // Get Data from dialog
        CMainFrame * pMDIFrameWnd = (CMainFrame *)AfxGetMainWnd();
        ASSERT(pMDIFrameWnd->IsKindOf(RUNTIME_CLASS(CMainFrame)));
        CSaView * pView = (CSaView *)pMDIFrameWnd->GetCurrSaView();
        CSaDoc * pDoc = (CSaDoc *)pView->GetDocument();

        // Find the proper graph
        UINT * pnID = pView->GetGraphIDs();
        CGraphWnd * pGraph = NULL;
        for (int nLoop = 0; nLoop < MAX_GRAPHS_NUMBER; nLoop++) {
            if (pnID[nLoop] == m_GraphId) {
                pGraph = pView->GetGraph(nLoop);
            }
        }
        // get spectrogram parameters
        CSpectroParm cSpectroParm = pDoc->GetSpectrogram(m_GraphId==IDD_SPECTROGRAM)->GetSpectroParm();
        CSpectroParm * pSpectroParm = &cSpectroParm;
        // save member data
        if (pSpectroParm->nResolution != m_nResolution) {
            // processed data is invalid
            pDoc->GetSpectrogram(m_GraphId==IDD_SPECTROGRAM)->SetDataInvalid();
        }

        if (pSpectroParm->bSmoothFormantTracks != m_bSmoothFormantTracks) {
            // processed data is invalid
            pDoc->GetSpectrogram(TRUE)->GetFormantProcess()->SetDataInvalid();
        }

        pSpectroParm->nFrequency = m_nFrequency;
        pSpectroParm->nMaxThreshold = m_nMaxThreshold;
        pSpectroParm->nMinThreshold = m_nMinThreshold;
        pSpectroParm->nResolution = m_nResolution;
        pSpectroParm->nColor = m_nColor;
        pSpectroParm->nOverlay = m_nOverlay;
        pSpectroParm->bShowF1 = m_bF1;
        pSpectroParm->bShowF2 = m_bF2;
        pSpectroParm->bShowF3 = m_bF3;
        pSpectroParm->bShowF4 = m_bF4;
        // if F4 is selected, also enable everything else
        pSpectroParm->bShowF5andUp = m_bF4;
        pSpectroParm->bSmoothFormantTracks = m_bSmoothFormantTracks;
        pSpectroParm->bSmoothSpectra = m_bSmoothSpectra;
        pSpectroParm->bFormantColor = m_bFormantColor;
        pSpectroParm->bShowPitch = m_bShowPitch;

        m_bModified = FALSE;
        SetModified(FALSE);
        // set the new spectrogram parameters
        pDoc->GetSpectrogram(m_GraphId==IDD_SPECTROGRAM)->SetSpectroParm(*pSpectroParm);
        pDoc->RestartAllProcesses();
        // refresh the spectrogram
        pGraph->RedrawGraph(TRUE, TRUE);
    }
}

void CDlgParametersSpectroPage::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar * pScrollBar) {

    int nCtrlID = pScrollBar->GetDlgCtrlID();

    switch (nCtrlID) {
    case IDC_SLIDER_MAX:
    case IDC_SLIDER_MIN:
    case IDC_SLIDER_FREQ:
        CSliderCtrl * pSlider = reinterpret_cast<CSliderCtrl *>(pScrollBar);
        nPos = pSlider->GetPos();
        break;
    }

    switch (nCtrlID) {
    case IDC_SLIDER_MAX:
        SetMaxThreshold(nPos);
        Apply();
        break;
    case IDC_SLIDER_MIN:
        SetMinThreshold(nPos);
        Apply();
        break;
    case IDC_SLIDER_FREQ:
        SetFrequency(nPos);
        Apply();
        break;
    default:
        ASSERT(FALSE);
        CPropertyPage::OnHScroll(nSBCode, nPos, pScrollBar);
    }
}

/*
Initialize this page (I supposes).
Rationale: Simplify the OnDialogInit function of SpectroPageA&B. -wgm
*/
BOOL CDlgParametersSpectroPage::OnInitDialog() {

    CPropertyPage::OnInitDialog();
    // get pointer to document
    CMainFrame * pMDIFrameWnd = (CMainFrame *)AfxGetMainWnd();
    ASSERT(pMDIFrameWnd->IsKindOf(RUNTIME_CLASS(CMainFrame)));
    CSaView * pView = (CSaView *)pMDIFrameWnd->GetCurrSaView();
    CSaDoc * pDoc = (CSaDoc *)pView->GetDocument();
    // get format parameters document member data
    FmtParm * pFmtParm = pDoc->GetFmtParm();
    m_nFreqLimit = (int)(pFmtParm->dwSamplesPerSec / 2); //CLW 8/25/99 ()'s
    // get spectrogram graph
    UINT * pnID = pView->GetGraphIDs();
    CGraphWnd * pGraph = NULL;
    for (int nLoop = 0; nLoop < MAX_GRAPHS_NUMBER; nLoop++) {
        if (pnID[nLoop] == m_GraphId) {
            pGraph = pView->GetGraph(nLoop);
        }
    }
    // get spectrogram parameters
    const CSpectroParm * pSpectroParm = &pDoc->GetSpectrogram(m_GraphId==IDD_SPECTROGRAM)->GetSpectroParm();
    // initialize member data
    m_nResolution = pSpectroParm->nResolution;
    m_nColor = pSpectroParm->nColor;
    m_nOverlay = pSpectroParm->nOverlay;
    m_bF1 = pSpectroParm->bShowF1;
    m_bF2 = pSpectroParm->bShowF2;
    m_bF3 = pSpectroParm->bShowF3;
    m_bF4 = pSpectroParm->bShowF4;
    m_bSmoothFormantTracks = pSpectroParm->bSmoothFormantTracks;
    m_bSmoothSpectra = pSpectroParm->bSmoothSpectra;
    m_bFormantColor = pSpectroParm->bFormantColor;
    m_bShowPitch = pSpectroParm->bShowPitch;
    m_bFormants = m_bShowPitch || m_bF1 || m_bF2 || m_bF3 || m_bF4;
    UpdateData(FALSE);
    m_cSliderFreq.SetRange(0,m_nFreqLimit, TRUE);
    m_cSliderFreq.SetLineSize(50);
    m_cSliderFreq.SetPageSize(500);
    SetFrequency(pSpectroParm->nFrequency);
    m_cSliderMax.SetRange(0, 234, TRUE);
    m_cSliderMax.SetLineSize(1);
    m_cSliderMax.SetPageSize(24);
    m_cSliderMin.SetRange(0, 234, TRUE);
    m_cSliderMin.SetLineSize(1);
    m_cSliderMin.SetPageSize(24);
    SetMaxThreshold(pSpectroParm->nMaxThreshold);
    SetMinThreshold(pSpectroParm->nMinThreshold);
    OnModifiedFormants(FALSE);
    return TRUE;  // return TRUE  unless you set the focus to a control
}

/***************************************************************************/
// CDlgParametersSpectroPage::DoDataExchange Data exchange
/***************************************************************************/
void CDlgParametersSpectroPage::DoDataExchange(CDataExchange * pDX) {

    CPropertyPage::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CDlgParametersSpectroPage)
    DDX_Control(pDX, IDC_MIN_STATIC, m_cMinStatic);
    DDX_Control(pDX, IDC_MAX_STATIC, m_cMaxStatic);
    DDX_Control(pDX, IDC_FREQ_STATIC, m_cFreqStatic);
    DDX_Control(pDX, IDC_SLIDER_MIN, m_cSliderMin);
    DDX_Control(pDX, IDC_SLIDER_MAX, m_cSliderMax);
    DDX_Control(pDX, IDC_SLIDER_FREQ, m_cSliderFreq);
    DDX_Radio(pDX, IDC_FINE, m_nResolution);
    DDX_Radio(pDX, IDC_COLOR, m_nColor);
    DDX_Check(pDX, IDC_F1, m_bF1);
    DDX_Check(pDX, IDC_F2, m_bF2);
    DDX_Check(pDX, IDC_F3, m_bF3);
    DDX_Check(pDX, IDC_F4, m_bF4);
    //DDX_Check(pDX, IDC_SMOOTH_FORMANTTRACKS, m_bSmoothFormantTracks);
    DDX_Radio(pDX, IDC_OVERLAID, m_nOverlay);
    DDX_Check(pDX, IDC_SMOOTH_SPECTRA, m_bSmoothSpectra);
    DDX_Check(pDX, IDC_FORMANTS, m_bFormants);
    DDX_Check(pDX, IDC_FORMANT_COLOR, m_bFormantColor);
    DDX_Check(pDX, IDC_SHOW_PITCH, m_bShowPitch);
    //}}AFX_DATA_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CDlgParametersSpectroPage message handlers

/***************************************************************************/
// CDlgParametersSpectroPage::OnChange page changed
/***************************************************************************/
void CDlgParametersSpectroPage::OnChange() {

    m_bModified = TRUE;
    SetModified(TRUE); // data modified, enable apply button
}

/***************************************************************************/
// CDlgParametersSpectroPage::SetFrequency Frequency changed
/***************************************************************************/
int CDlgParametersSpectroPage::SetFrequency(int nData) {

    int nReturn = m_nFrequency;
    int nLowerFreq = (int)CSpectroParm::Bandwidth(m_nResolution);
    if (nData + 25 > m_nFreqLimit) {
        nData = m_nFreqLimit;
    } else if (nData - 25 <= nLowerFreq) {
        nData = nLowerFreq + 1;
    } else {
        nData = ((nData+ 25)/50)*50;    // Round to nearest 50 Hz
    }
    if (nData != m_nFrequency) {
        m_nFrequency = nData;
        m_bModified = TRUE;
        SetModified(TRUE); // data modified, enable apply button
    }
    m_cSliderFreq.SetPos(nData);

    CString szText;
    szText.Format(_T("%d Hz"), nData);
    m_cFreqStatic.SetWindowText(szText);
    return nReturn;
}

/***************************************************************************/
// CDlgParametersSpectroPage::SetMaxThreshold
/***************************************************************************/
int CDlgParametersSpectroPage::SetMaxThreshold(int nData) {

    int nReturn = m_nMaxThreshold;
    if (nData <= m_nMinThreshold)
        // limit using other position
    {
        nData = m_nMinThreshold + 1;
    }
    if (nData > 233) {
        nData = 233;
    }
    if (nData < 1) {
        nData = 1;
    }

    if (nData != m_nMaxThreshold) {
        m_nMaxThreshold = nData;
        m_bModified = TRUE;
        SetModified(TRUE); // data modified, enable apply button
        if (m_nMaxThreshold <= m_nMinThreshold) {
            SetMinThreshold(m_nMaxThreshold - 1);
        }
    }
    m_cSliderMax.SetPos(nData);

    CString szText;
    szText.Format(_T("%0.1f dB"), 3. - (233 - nData)*0.25);
    m_cMaxStatic.SetWindowText(szText);
    return nReturn;
}

/***************************************************************************/
// CDlgParametersSpectroPage::SetMinThreshold
/***************************************************************************/
int CDlgParametersSpectroPage::SetMinThreshold(int nData) {

    int nReturn = m_nMinThreshold;
    if (nData >= m_nMaxThreshold)
        // limit using other position
    {
        nData = m_nMaxThreshold - 1;
    }
    if (nData > 234) {
        nData = 234;
    }
    if (nData < 1) {
        nData = 1;
    }
    if (nData != m_nMinThreshold) {
        m_nMinThreshold = nData;
        m_bModified = TRUE;
        SetModified(TRUE); // data modified, enable apply button
        if (m_nMaxThreshold <= m_nMinThreshold) {
            SetMaxThreshold(m_nMinThreshold + 1);
        }
    }
    m_cSliderMin.SetPos(nData);

    CString szText;
    szText.Format(_T("%0.1f dB"), 3. - (233 - nData)*0.25);
    m_cMinStatic.SetWindowText(szText);
    return nReturn;
}

/***************************************************************************/
// CDlgParametersSpectroPage::OnModifiedFormants Formants on/off changed
/***************************************************************************/
void CDlgParametersSpectroPage::OnModifiedFormants() {

    OnModifiedFormants(TRUE);
}

void CDlgParametersSpectroPage::OnModifiedFormants(BOOL bMessage) {
    BOOL bOldFormants = m_bFormants;
    BOOL bMasterEnableChange = !bMessage;

    if (bMessage) {
        UpdateData(TRUE);
        SetModified(TRUE); // data modified, enable apply button
        m_bModified = TRUE;
    }

    if ((m_bFormants != bOldFormants) ||
		(!m_bFormants && (m_bShowPitch || m_bF1 || m_bF2 || m_bF3 || m_bF4))) {
        m_bF1 = m_bF2 = m_bF3 = m_bF4 = m_bFormants;
        m_bShowPitch = FALSE;

        UpdateData(FALSE);

        bMasterEnableChange = TRUE;
    }

    if (bMasterEnableChange) {
        GetDlgItem(IDC_SHOW_PITCH)->EnableWindow(m_bFormants);
        GetDlgItem(IDC_F1)->EnableWindow(m_bFormants);
        GetDlgItem(IDC_F2)->EnableWindow(m_bFormants);
        GetDlgItem(IDC_F3)->EnableWindow(m_bFormants);
        GetDlgItem(IDC_F4)->EnableWindow(m_bFormants);
    }

    BOOL bShowFormants = m_bF1 || m_bF2 || m_bF3 || m_bF4;
    //GetDlgItem(IDC_SMOOTH_FORMANTTRACKS)->EnableWindow(bShowFormants);
    GetDlgItem(IDC_OVERLAID)->EnableWindow(bShowFormants);
    GetDlgItem(IDC_ONLY)->EnableWindow(bShowFormants);

    bShowFormants = bShowFormants || m_bShowPitch;
    GetDlgItem(IDC_FORMANT_COLOR)->EnableWindow((m_bShowPitch || bShowFormants) && m_nColor);

    GetDlgItem(IDC_FORMANT_NOTE)->EnableWindow(m_bFormants);
    GetDlgItem(IDC_MORE_INFO)->EnableWindow(m_bFormants);
}


/***************************************************************************/
// CDlgParametersSpectroPage::OnModifiedSmoothFormantTracks  Smooth formant tracks on/off changed
/***************************************************************************/
void CDlgParametersSpectroPage::OnModifiedSmoothFormantTracks() {

    //m_bSmoothFormantTracks = (BOOL)IsDlgButtonChecked(IDC_SMOOTH_FORMANTTRACKS);
    SetModified(TRUE); // data modified, enable apply button
    m_bModified = TRUE;
}


/***************************************************************************/
// CDlgParametersSpectroPage::OnModifiedColor Color mode modified
// In case of palette changes, the apply button must become disabled, because
// the dialog captures the palette and must be closed to allow the palette
// to be changed.
/***************************************************************************/
void CDlgParametersSpectroPage::OnModifiedColor() {

    int nData = (int)IsDlgButtonChecked(IDC_MONO);
    if (nData != m_nColor) {
        m_nColor = nData;
        m_bModified = TRUE;
        BOOL bShowFormants = m_bShowPitch || m_bF1 || m_bF2 || m_bF3 || m_bF4;
        GetDlgItem(IDC_FORMANT_COLOR)->EnableWindow(m_nColor && bShowFormants);
        SetModified(TRUE); // data modified, enable apply button
        Apply();
    }
}

/***************************************************************************/
// CDlgParametersSpectroPage::OnModifiedResolution Resolution modified
/***************************************************************************/
void CDlgParametersSpectroPage::OnModifiedResolution() {

    int nData = (int)m_nResolution;
    UpdateData(TRUE);
    if (nData != m_nResolution) {
        m_nResolution = nData;
        m_bModified = TRUE;
        SetModified(TRUE); // data modified, enable apply button
        // check, if frequency below limit now
        SetFrequency(m_nFrequency);
        Apply();
    }
}

/***************************************************************************/
// CDlgParametersSpectroPage::OnModifiedOverlay Overlay modified
// In case of palette changes, the apply button must become disabled, because
// the dialog captures the palette and must be closed to allow the palette
// to be changed.
/***************************************************************************/
void CDlgParametersSpectroPage::OnModifiedOverlay() {

    int nData = (int)IsDlgButtonChecked(IDC_ONLY);
    if (nData != m_nOverlay) {
        m_nOverlay = nData;
        m_bModified = TRUE;
        SetModified(TRUE); // data modified, enable apply button
    }
}

/***************************************************************************/
// CDlgParametersSpectroPage::OnMoreInfo More Info clicked
/***************************************************************************/
void CDlgParametersSpectroPage::OnMoreInfo() {

    // create the pathname
    CString szPath = AfxGetApp()->m_pszHelpFilePath;
    szPath += "::/User_Interface/Menus/Graphs/Graph_Types/Formants.htm";
    ::HtmlHelp(NULL, szPath, HH_DISPLAY_TOPIC, NULL);
}

//###########################################################################
// CDlgParametersSpectrumPage property page
// Displays all controls to change spectrum calculation parameters.

/////////////////////////////////////////////////////////////////////////////
// CDlgParametersSpectrumPage message map

BEGIN_MESSAGE_MAP(CDlgParametersSpectrumPage, CPropertyPage)
    //{{AFX_MSG_MAP(CDlgParametersSpectrumPage)
    ON_COMMAND(IDC_SPECTRUM_PWRUPPERSCROLL, OnPwrUpperScroll)
    ON_EN_KILLFOCUS(IDC_SPECTRUM_PWRUPPEREDIT, OnKillfocusPwrUpper)
    ON_COMMAND(IDC_SPECTRUM_PWRLOWERSCROLL, OnPwrLowerScroll)
    ON_EN_KILLFOCUS(IDC_SPECTRUM_PWRLOWEREDIT, OnKillfocusPwrLower)
    ON_COMMAND(IDC_SPECTRUM_FREQUPPERSCROLL, OnFreqUpperScroll)
    ON_EN_KILLFOCUS(IDC_SPECTRUM_FREQUPPEREDIT, OnKillfocusFreqUpper)
    ON_COMMAND(IDC_SPECTRUM_FREQLOWERSCROLL, OnFreqLowerScroll)
    ON_EN_KILLFOCUS(IDC_SPECTRUM_FREQLOWEREDIT, OnKillfocusFreqLower)
    ON_BN_CLICKED(IDC_SPECTRUM_FULLSCALE, OnModifiedFreqRange)
    ON_BN_CLICKED(IDC_SPECTRUM_HALFSCALE, OnModifiedFreqRange)
    ON_BN_CLICKED(IDC_SPECTRUM_THIRDSCALE, OnModifiedFreqRange)
    ON_BN_CLICKED(IDC_SPECTRUM_QUARTERSCALE, OnModifiedFreqRange)
    ON_COMMAND(IDC_SPECTRUM_SMOOTHSCROLL, OnSmoothScroll)
    ON_EN_KILLFOCUS(IDC_SPECTRUM_SMOOTHEDIT, OnKillfocusSmooth)
    ON_COMMAND(IDC_SPECTRUM_PEAKSCROLL, OnPeakScroll)
    ON_EN_KILLFOCUS(IDC_SPECTRUM_PEAKEDIT, OnKillfocusPeak)
    ON_CBN_SELCHANGE(IDC_SPECTRUM_SCALING, OnScaling)
    ON_EN_CHANGE(IDC_SPECTRUM_PWRUPPEREDIT, OnChange)
    ON_EN_CHANGE(IDC_SPECTRUM_PWRLOWEREDIT, OnChange)
    ON_EN_CHANGE(IDC_SPECTRUM_FREQUPPEREDIT, OnChange)
    ON_EN_CHANGE(IDC_SPECTRUM_FREQLOWEREDIT, OnChange)
    ON_BN_CLICKED(IDC_SPECTRUM_LPCSMOOTH, OnModifiedShowSmoothedPlots)
    ON_BN_CLICKED(IDC_SPECTRUM_CEPSMOOTH, OnModifiedShowSmoothedPlots)
    ON_EN_CHANGE(IDC_SPECTRUM_SMOOTHEDIT, OnChange)
    ON_EN_CHANGE(IDC_SPECTRUM_PEAKEDIT, OnChange)
    ON_BN_CLICKED(IDC_SPECTRUM_FORMANTFREQ, OnModifiedShowFormantFreq)
    ON_BN_CLICKED(IDC_SPECTRUM_FORMANTBANDWIDTH, OnModifiedShowFormantBandwidth)
    ON_BN_CLICKED(IDC_SPECTRUM_FORMANTPOWER, OnModifiedShowFormantPower)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgParametersSpectrumPage construction/destruction/creation

/***************************************************************************/
// CDlgParametersSpectrumPage::CDlgParametersSpectrumPage Constructor
/***************************************************************************/
CDlgParametersSpectrumPage::CDlgParametersSpectrumPage() : CPropertyPage(CDlgParametersSpectrumPage::IDD) {

    //{{AFX_DATA_INIT(CDlgParametersSpectrumPage)
    m_nFreqScaleRange = -1;
    //}}AFX_DATA_INIT
    m_bModified = FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// CDlgParametersSpectrumPage helper functions

/***************************************************************************/
// CDlgParametersSpectrumPage::DoDataExchange Data exchange
/***************************************************************************/
void CDlgParametersSpectrumPage::DoDataExchange(CDataExchange * pDX) {

    CPropertyPage::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CDlgParametersSpectrumPage)
    DDX_Text(pDX, IDC_SPECTRUM_PWRLOWEREDIT, m_nPwrLower);
    DDX_Text(pDX, IDC_SPECTRUM_PEAKEDIT, m_nPeak);
    DDX_Text(pDX, IDC_SPECTRUM_SMOOTHEDIT, m_nSmooth);
    DDX_Text(pDX, IDC_SPECTRUM_PWRUPPEREDIT, m_nPwrUpper);
    DDX_CBIndex(pDX, IDC_SPECTRUM_SCALING, m_nSpectrumScaling);
    DDX_Text(pDX, IDC_SPECTRUM_FREQLOWEREDIT, m_nFreqLower);
    DDX_Text(pDX, IDC_SPECTRUM_FREQUPPEREDIT, m_nFreqUpper);
    DDX_Check(pDX, IDC_SPECTRUM_LPCSMOOTH, m_bShowLpcSpectrum);
    DDX_Check(pDX, IDC_SPECTRUM_CEPSMOOTH, m_bShowCepSpectrum);
    DDX_Check(pDX, IDC_SPECTRUM_FORMANTFREQ, m_bShowFormantFreq);
    DDX_Check(pDX, IDC_SPECTRUM_FORMANTBANDWIDTH, m_bShowFormantBandwidth);
    DDX_Check(pDX, IDC_SPECTRUM_FORMANTPOWER, m_bShowFormantPower);
    DDX_Radio(pDX, IDC_SPECTRUM_FULLSCALE, m_nFreqScaleRange);
    //}}AFX_DATA_MAP
}

/***************************************************************************/
// CDlgParametersSpectrumPage::Apply Apply button hit
/***************************************************************************/
void CDlgParametersSpectrumPage::Apply() {

    if (m_bModified) { // retrieve data
        // get pointer to spectrum process
        CMainFrame * pMDIFrameWnd = (CMainFrame *)AfxGetMainWnd();
        ASSERT(pMDIFrameWnd->IsKindOf(RUNTIME_CLASS(CMainFrame)));
        CSaView * pView = (CSaView *)pMDIFrameWnd->GetCurrSaView();
        CSaDoc * pDoc = (CSaDoc *)pView->GetDocument();
        CProcessSpectrum * pSpectrum = pDoc->GetSpectrum();
        // get spectrum parameters data
        SpectrumParm * pSpecParm = pSpectrum->GetSpectrumParms();
        // save the parameters
        pSpecParm->nScaleMode = m_nSpectrumScaling;
        pSpecParm->nPwrUpperBound = m_nPwrUpper;
        pSpecParm->nPwrLowerBound = m_nPwrLower;
        pSpecParm->nFreqUpperBound = m_nFreqUpper;
        pSpecParm->nFreqLowerBound = m_nFreqLower;
        int nFreqScaleOld = pSpecParm->nFreqScaleRange;
        pSpecParm->nFreqScaleRange = m_nFreqScaleRange;
        pSpecParm->nSmoothLevel = m_nSmooth;
        pSpecParm->nPeakSharpFac = m_nPeak;
        pSpecParm->bShowLpcSpectrum = m_bShowLpcSpectrum;
        pSpecParm->bShowCepSpectrum = m_bShowCepSpectrum;
        pSpecParm->bShowFormantFreq = m_bShowFormantFreq;
        pSpecParm->bShowFormantBandwidth = m_bShowFormantBandwidth;
        pSpecParm->bShowFormantPower = m_bShowFormantPower;
        // invalidate processed data
        pSpectrum->SetDataInvalid();
        m_bModified = FALSE;
        SetModified(FALSE);
        // refresh the spectrum
        int nSpectrumGraph = pView->GetGraphIndexForIDD(IDD_SPECTRUM);
        CGraphWnd * pSpectrumGraph = pView->GetGraph(nSpectrumGraph);
        if (nFreqScaleOld != pSpecParm->nFreqScaleRange) {
            CPlotWnd * pSpectrumPlot = pSpectrumGraph->GetPlot();
            CPrivateCursorWnd * pFreqCursor = pSpectrumPlot->GetPrivateCursorWnd();
            CPoint point;
            point.x = pSpectrumPlot->GetPrivateCursorPosition();

            double ratio = (pSpecParm->nFreqScaleRange + 1)/double(nFreqScaleOld + 1);

            point.x = round(point.x *ratio);

            // full scale to half scale
            CRect rPlotWnd;
            pSpectrumPlot->GetClientRect(rPlotWnd);
            if (point.x >= rPlotWnd.right) {
                point.x = rPlotWnd.right - 1;
            }

            pFreqCursor->ChangeCursorPosition(point);
        }
        pSpectrumGraph->RedrawGraph(TRUE,TRUE);
        //pSpectrumGraph->RedrawGraph(FALSE,TRUE); //by AKE, to retain x-scale during animation
        int nFormantChart = pView->GetGraphIndexForIDD(IDD_F1F2);
        if (nFormantChart >= 0) {
            CGraphWnd * pFormantChart = pView->GetGraph(nFormantChart);
            pFormantChart->RedrawGraph(TRUE, TRUE);
        }
        nFormantChart = pView->GetGraphIndexForIDD(IDD_F2F1);
        if (nFormantChart >= 0) {
            CGraphWnd * pFormantChart = pView->GetGraph(nFormantChart);
            pFormantChart->RedrawGraph(TRUE, TRUE);
        }
        nFormantChart = pView->GetGraphIndexForIDD(IDD_F2F1F1);
        if (nFormantChart >= 0) {
            CGraphWnd * pFormantChart = pView->GetGraph(nFormantChart);
            pFormantChart->RedrawGraph(TRUE, TRUE);
        }
        nFormantChart = pView->GetGraphIndexForIDD(IDD_3D);
        if (nFormantChart >= 0) {
            CGraphWnd * pFormantChart = pView->GetGraph(nFormantChart);
            pFormantChart->RedrawGraph(TRUE, TRUE);
        }
    }
}

/////////////////////////////////////////////////////////////////////////////
// CDlgParametersSpectrumPage message handlers

/***************************************************************************/
// CDlgParametersSpectrumPage::OnInitDialog Dialog initialization
/***************************************************************************/
BOOL CDlgParametersSpectrumPage::OnInitDialog() {

    // get pointer to spectrum process
    CMainFrame * pMDIFrameWnd = (CMainFrame *)AfxGetMainWnd();
    ASSERT(pMDIFrameWnd->IsKindOf(RUNTIME_CLASS(CMainFrame)));
    CSaView * pView = (CSaView *)pMDIFrameWnd->GetCurrSaView();
    CSaDoc * pDoc = (CSaDoc *)pView->GetDocument();
    CProcessSpectrum * pSpectrum = pDoc->GetSpectrum();

    // get spectrum parameters data
    SpectrumParm * pSpecParm = pSpectrum->GetSpectrumParms();
    // initialize member data
    m_nSpectrumScaling = pSpecParm->nScaleMode;
    m_nPwrUpper = pSpecParm->nPwrUpperBound;
    m_nPwrLower = pSpecParm->nPwrLowerBound;
    m_nFreqUpper = pSpecParm->nFreqUpperBound;
    m_nFreqLower = pSpecParm->nFreqLowerBound;
    m_nFreqScaleRange = pSpecParm->nFreqScaleRange;
    // get format parameters document member data
    FmtParm * pFmtParm = pDoc->GetFmtParm();
    m_nFreqLimit = (int)(pFmtParm->dwSamplesPerSec / 2); //CLW 8/25/99 ()'s
    m_bShowLpcSpectrum = pSpecParm->bShowLpcSpectrum;
    m_bShowCepSpectrum = pSpecParm->bShowCepSpectrum;
    m_nSmooth = pSpecParm->nSmoothLevel;
    m_nPeak = pSpecParm->nPeakSharpFac;
    m_bShowFormantFreq = pSpecParm->bShowFormantFreq;
    m_bShowFormantBandwidth = pSpecParm->bShowFormantBandwidth;
    m_bShowFormantPower = pSpecParm->bShowFormantPower;

    CPropertyPage::OnInitDialog();
    CRect rWnd;
    // build and place the smooth frequency spin control
    m_SpinSmoothLevel.Init(IDC_SPECTRUM_SMOOTHSCROLL, this);
    // build and place the sharpening spin control
    m_SpinSharpening.Init(IDC_SPECTRUM_PEAKSCROLL, this);
    // build and place the power upper boundary spin control
    m_SpinPwrUpper.Init(IDC_SPECTRUM_PWRUPPERSCROLL, this);
    // build and place the power lower boundary spin control
    m_SpinPwrLower.Init(IDC_SPECTRUM_PWRLOWERSCROLL, this);
    // build and place the frequency upper boundary spin control
    // m_SpinFreqUpper.Init(IDC_SPECTRUM_FREQUPPERSCROLL, this);  //!!restore when code completed
    //m_SpinFreqUpper.EnableWindow(FALSE);  // ## temporary: disable the spin control
    // build and place the frequency lower boundary spin control
    // m_SpinFreqLower.Init(IDC_SPECTRUM_FREQLOWERSCROLL, this);  //!!restore when code completed
    //m_SpinFreqLower.EnableWindow(FALSE);  // ## temporary: disable the spin control
    CheckDlgButton(IDC_SPECTRUM_LPCSMOOTH, pSpecParm->bShowLpcSpectrum);
    GetDlgItem(IDC_SPECTRUM_PEAKFACTOR)->EnableWindow(pSpecParm->bShowLpcSpectrum || pSpecParm->bShowCepSpectrum);
    GetDlgItem(IDC_SPECTRUM_PEAKEDIT)->EnableWindow(pSpecParm->bShowLpcSpectrum || pSpecParm->bShowCepSpectrum);
    m_SpinSharpening.EnableWindow(pSpecParm->bShowLpcSpectrum || pSpecParm->bShowCepSpectrum);
    GetDlgItem(IDC_SPECTRUM_FORMANTFREQ)->EnableWindow(pSpecParm->bShowLpcSpectrum);
    GetDlgItem(IDC_SPECTRUM_FORMANTPOWER)->EnableWindow(pSpecParm->bShowLpcSpectrum);
    CheckDlgButton(IDC_SPECTRUM_CEPSMOOTH, pSpecParm->bShowCepSpectrum);
    GetDlgItem(IDC_SPECTRUM_SMOOTHLEVEL)->EnableWindow(pSpecParm->bShowCepSpectrum);
    GetDlgItem(IDC_SPECTRUM_SMOOTHEDIT)->EnableWindow(pSpecParm->bShowCepSpectrum);
    m_SpinSmoothLevel.EnableWindow(pSpecParm->bShowCepSpectrum);
    CheckDlgButton(IDC_SPECTRUM_FORMANTFREQ, pSpecParm->bShowFormantFreq);
    CheckDlgButton(IDC_SPECTRUM_FORMANTBANDWIDTH, pSpecParm->bShowFormantBandwidth);
    CheckDlgButton(IDC_SPECTRUM_FORMANTPOWER, pSpecParm->bShowFormantPower);
    // show or hide power range dimension
    if (m_nSpectrumScaling) {
        // hide power range dimension
        GetDlgItem(IDC_SPECTRUM_PWRUPPERDIM)->ShowWindow(SW_HIDE);
        GetDlgItem(IDC_SPECTRUM_PWRLOWERDIM)->ShowWindow(SW_HIDE);
    } else {
        // show power range dimension
        GetDlgItem(IDC_SPECTRUM_PWRUPPERDIM)->ShowWindow(SW_SHOW);
        GetDlgItem(IDC_SPECTRUM_PWRLOWERDIM)->ShowWindow(SW_SHOW);
    }
    UpdateData(FALSE);
    return TRUE;
}

/***************************************************************************/
// CDlgParametersSpectrumPage::OnPwrUpperScroll Upper power bnd spin control hit
/***************************************************************************/
void CDlgParametersSpectrumPage::OnPwrUpperScroll() {

    int nData = GetDlgItemInt(IDC_SPECTRUM_PWRUPPEREDIT, NULL, TRUE);
    if (m_SpinPwrUpper.UpperButtonClicked()) {
        nData++;
    } else {
        nData--;
    }
    if (nData > 0x7FFF) {
        nData = 0x7FFF;
    }
    if (nData < 0) {
        nData = 0;
    }
    if (nData != m_nPwrUpper) {
        m_nPwrUpper = nData;
        m_bModified = TRUE;
        SetModified(TRUE); // data modified, enable apply button
        SetDlgItemInt(IDC_SPECTRUM_PWRUPPEREDIT, m_nPwrUpper, TRUE);
        if (m_nPwrLower >= m_nPwrUpper) {
            m_nPwrLower = m_nPwrUpper - 1;
            SetDlgItemInt(IDC_SPECTRUM_PWRLOWEREDIT, m_nPwrLower, TRUE);
        }
    }
}

/***************************************************************************/
// CDlgParametersSpectrumPage::OnKillfocusPwrUpper Upper power boundary changed
/***************************************************************************/
void CDlgParametersSpectrumPage::OnKillfocusPwrUpper() {

    BOOL bTrans; //CLW 8/27/99
    int nData = (int)GetDlgItemInt(IDC_SPECTRUM_PWRUPPEREDIT, &bTrans, TRUE); //CLW 8/27/99
    if (!bTrans && (nData==0)) {
        nData = 0x7FFF;    //CLW 8/27/99
    }
    if (nData > 0x7FFF) {
        nData = 0x7FFF;
    }
    if (nData < 0) {
        nData = 0;
    }
    if (nData != m_nPwrUpper) {
        m_nPwrUpper = nData;
        m_bModified = TRUE;
        SetModified(TRUE); // data modified, enable apply button
        if (m_nPwrLower >= m_nPwrUpper) {
            m_nPwrLower = m_nPwrUpper - 1;
            SetDlgItemInt(IDC_SPECTRUM_PWRLOWEREDIT, m_nPwrLower, TRUE);
        }
    }
    SetDlgItemInt(IDC_SPECTRUM_PWRUPPEREDIT, m_nPwrUpper, TRUE);
}

/***************************************************************************/
// CDlgParametersSpectrumPage::OnModifiedFreqRange
/***************************************************************************/
void CDlgParametersSpectrumPage::OnModifiedFreqRange() {

    int nData = m_nFreqScaleRange;
    UpdateData(TRUE);
    if (nData != m_nFreqScaleRange) {
        m_bModified = TRUE;
        SetModified(TRUE); // data modified, enable apply button
    }
}

/***************************************************************************/
// CDlgParametersSpectrumPage::OnChange page changed
/***************************************************************************/
void CDlgParametersSpectrumPage::OnChange() {

    m_bModified = TRUE;
    SetModified(TRUE); // data modified, enable apply button
}

/***************************************************************************/
// CDlgParametersSpectrumPage::OnPwrLowerScroll Lower power bnd spin control hit
/***************************************************************************/
void CDlgParametersSpectrumPage::OnPwrLowerScroll() {

    int nData = GetDlgItemInt(IDC_SPECTRUM_PWRLOWEREDIT, NULL, TRUE);
    if (m_SpinPwrLower.UpperButtonClicked()) {
        nData++;
    } else {
        nData--;
    }
    if (nData > 0x7FFE) {
        nData = 0x7FFE;
    }
    if (nData < -200) {
        nData = -200;
    }
    if (nData != m_nPwrLower) {
        m_nPwrLower = nData;
        m_bModified = TRUE;
        SetModified(TRUE); // data modified, enable apply button
        SetDlgItemInt(IDC_SPECTRUM_PWRLOWEREDIT, m_nPwrLower, TRUE);
        if (m_nPwrUpper <= m_nPwrLower) {
            m_nPwrUpper = m_nPwrLower + 1;
            SetDlgItemInt(IDC_SPECTRUM_PWRUPPEREDIT, m_nPwrUpper, TRUE);
        }
    }
}

/***************************************************************************/
// CDlgParametersSpectrumPage::OnKillfocusPwrLower Lower power boundary changed
/***************************************************************************/
void CDlgParametersSpectrumPage::OnKillfocusPwrLower() {

    BOOL bTrans; //CLW 8/27/99
    int nData = (int)GetDlgItemInt(IDC_SPECTRUM_PWRLOWEREDIT, &bTrans, TRUE); //CLW 8/27/99
    if (!bTrans && (nData==0)) {
        nData = 0x7FFE;    //CLW 8/27/99
    }
    if (nData > 0x7FFE) {
        nData = 0x7FFE;
    }
    if (nData < -200) {
        nData = -200;
    }
    if (nData != m_nPwrLower) {
        m_nPwrLower = nData;
        m_bModified = TRUE;
        SetModified(TRUE); // data modified, enable apply button
        if (m_nPwrUpper <= m_nPwrLower) {
            m_nPwrUpper = m_nPwrLower + 1;
            SetDlgItemInt(IDC_SPECTRUM_PWRUPPEREDIT, m_nPwrUpper, TRUE);
        }
    }
    SetDlgItemInt(IDC_SPECTRUM_PWRLOWEREDIT, m_nPwrLower, TRUE);
}

/***************************************************************************/
// CDlgParametersSpectrumPage::OnFreqUpperScroll Upper freq. bnd spin cntrl hit
/***************************************************************************/
void CDlgParametersSpectrumPage::OnFreqUpperScroll() {

    int nData = GetDlgItemInt(IDC_SPECTRUM_FREQUPPEREDIT, NULL, TRUE);
    if (m_SpinFreqUpper.UpperButtonClicked()) {
        nData++;
    } else {
        nData--;
    }
    if (nData > m_nFreqLimit) {
        nData = m_nFreqLimit;
    }
    if (nData < 100) {
        nData = 100;
    }
    if (nData != m_nFreqUpper) {
        m_nFreqUpper = nData;
        m_bModified = TRUE;
        SetModified(TRUE); // data modified, enable apply button
        SetDlgItemInt(IDC_SPECTRUM_FREQUPPEREDIT, m_nFreqUpper, TRUE);
        if (m_nFreqLower >= (m_nFreqUpper - 99)) {
            m_nFreqLower = m_nFreqUpper - 100;
            SetDlgItemInt(IDC_SPECTRUM_FREQLOWEREDIT, m_nFreqLower, TRUE);
        }
    }
}

/***************************************************************************/
// CDlgParametersSpectrumPage::OnKillfocusFreqUpper Upper freq. boundary changed
/***************************************************************************/
void CDlgParametersSpectrumPage::OnKillfocusFreqUpper() {

    int nData = GetDlgItemInt(IDC_SPECTRUM_FREQUPPEREDIT, NULL, TRUE);
    if (nData > m_nFreqLimit) {
        nData = m_nFreqLimit;
    }
    if (nData < 100) {
        nData = 100;
    }
    if (nData != m_nFreqUpper) {
        m_nFreqUpper = nData;
        m_bModified = TRUE;
        SetModified(TRUE); // data modified, enable apply button
        if (m_nFreqLower >= (m_nFreqUpper - 99)) {
            m_nFreqLower = m_nFreqUpper - 100;
            SetDlgItemInt(IDC_SPECTRUM_FREQLOWEREDIT, m_nFreqLower, TRUE);
        }
    }
    SetDlgItemInt(IDC_SPECTRUM_FREQUPPEREDIT, m_nFreqUpper, TRUE);
}

/***************************************************************************/
// CDlgParametersSpectrumPage::OnFreqLowerScroll Lower freq. bnd spin control hit
/***************************************************************************/
void CDlgParametersSpectrumPage::OnFreqLowerScroll() {

    int nData = GetDlgItemInt(IDC_SPECTRUM_FREQLOWEREDIT, NULL, TRUE);
    if (m_SpinFreqLower.UpperButtonClicked()) {
        nData++;
    } else {
        nData--;
    }
    if (nData > (m_nFreqLimit - 100)) {
        nData = m_nFreqLimit - 100;
    }
    if (nData < 0) {
        nData = 0;
    }
    if (nData != m_nFreqLower) {
        m_nFreqLower = nData;
        m_bModified = TRUE;
        SetModified(TRUE); // data modified, enable apply button
        SetDlgItemInt(IDC_SPECTRUM_FREQLOWEREDIT, m_nFreqLower, TRUE);
        if (m_nFreqUpper <= (m_nFreqLower + 99)) {
            m_nFreqUpper = m_nFreqLower + 100;
            SetDlgItemInt(IDC_SPECTRUM_FREQUPPEREDIT, m_nFreqUpper, TRUE);
        }
    }
}

/***************************************************************************/
// CDlgParametersSpectrumPage::OnKillfocusFreqLower Lower freq. boundary changed
/***************************************************************************/
void CDlgParametersSpectrumPage::OnKillfocusFreqLower() {

    int nData = GetDlgItemInt(IDC_SPECTRUM_FREQLOWEREDIT, NULL, TRUE);
    if (nData > (m_nFreqLimit - 100)) {
        nData = m_nFreqLimit - 100;
    }
    if (nData < 0) {
        nData = 0;
    }
    if (nData != m_nFreqLower) {
        m_nFreqLower = nData;
        m_bModified = TRUE;
        SetModified(TRUE); // data modified, enable apply button
        if (m_nFreqUpper <= (m_nFreqLower + 99)) {
            m_nFreqUpper = m_nFreqLower + 100;
            SetDlgItemInt(IDC_SPECTRUM_FREQUPPEREDIT, m_nFreqUpper, TRUE);
        }
    }
    SetDlgItemInt(IDC_SPECTRUM_FREQLOWEREDIT, m_nFreqLower, TRUE);
}

/***************************************************************************/
// CDlgParametersSpectrumPage::OnSmoothScroll Smoothing spin control hit
/***************************************************************************/
void CDlgParametersSpectrumPage::OnSmoothScroll() {

    int nData = GetDlgItemInt(IDC_SPECTRUM_SMOOTHEDIT, NULL, TRUE);
    if (m_SpinSmoothLevel.UpperButtonClicked()) {
        nData++;
    } else {
        nData--;
    }
    if (nData > 9) {
        nData = 9;
    }
    if (nData < 0) {
        nData = 0;
    }
    if (nData != m_nSmooth) {
        m_nSmooth = nData;
        m_bModified = TRUE;
        SetModified(TRUE); // data modified, enable apply button
        SetDlgItemInt(IDC_SPECTRUM_SMOOTHEDIT, m_nSmooth, TRUE);
    }
}

/***************************************************************************/
// CDlgParametersSpectrumPage::OnKillfocusSmooth Smoothing changed
/***************************************************************************/
void CDlgParametersSpectrumPage::OnKillfocusSmooth() {

    int nData = GetDlgItemInt(IDC_SPECTRUM_SMOOTHEDIT, NULL, TRUE);
    if (nData > 9) {
        nData = 9;
    }
    if (nData < 0) {
        nData = 0;
    }
    if (nData != m_nSmooth) {
        m_nSmooth = nData;
        m_bModified = TRUE;
        SetModified(TRUE); // data modified, enable apply button
    }
    SetDlgItemInt(IDC_SPECTRUM_SMOOTHEDIT, m_nSmooth, TRUE);
}

/***************************************************************************/
// CDlgParametersSpectrumPage::OnPeakScroll Peaking spin control hit
/***************************************************************************/
void CDlgParametersSpectrumPage::OnPeakScroll() {

    int nData = GetDlgItemInt(IDC_SPECTRUM_PEAKEDIT, NULL, TRUE);
    if (m_SpinSharpening.UpperButtonClicked()) {
        nData++;
    } else {
        nData--;
    }
    if (nData > 9) {
        nData = 9;
    }
    if (nData < 0) {
        nData = 0;
    }
    if (nData != m_nPeak) {
        m_nPeak = nData;
        m_bModified = TRUE;
        SetModified(TRUE); // data modified, enable apply button
        SetDlgItemInt(IDC_SPECTRUM_PEAKEDIT, m_nPeak, TRUE);
    }
}

/***************************************************************************/
// CDlgParametersSpectrumPage::OnKillfocusPeak Peaking changed
/***************************************************************************/
void CDlgParametersSpectrumPage::OnKillfocusPeak() {

    int nData = GetDlgItemInt(IDC_SPECTRUM_PEAKEDIT, NULL, TRUE);
    if (nData > 9) {
        nData = 9;
    }
    if (nData < 0) {
        nData = 0;
    }
    if (nData != m_nPeak) {
        m_nPeak = nData;
        m_bModified = TRUE;
        SetModified(TRUE); // data modified, enable apply button
    }
    SetDlgItemInt(IDC_SPECTRUM_PEAKEDIT, m_nPeak, TRUE);
}

/***************************************************************************/
// CDlgParametersSpectrumPage::OnScaling Scale mode changed
// If the scaling is log (0), the power range dimension text has to be
// visible, otherwise it must be hidden.
/***************************************************************************/
void CDlgParametersSpectrumPage::OnScaling() {

    UpdateData(TRUE);
    m_bModified = TRUE;
    SetModified(TRUE); // data modified, enable apply button
    if (m_nSpectrumScaling) {
        // hide power range dimension
        GetDlgItem(IDC_SPECTRUM_PWRUPPERDIM)->ShowWindow(SW_HIDE);
        GetDlgItem(IDC_SPECTRUM_PWRLOWERDIM)->ShowWindow(SW_HIDE);
    } else {
        // show power range dimension
        GetDlgItem(IDC_SPECTRUM_PWRUPPERDIM)->ShowWindow(SW_SHOW);
        GetDlgItem(IDC_SPECTRUM_PWRLOWERDIM)->ShowWindow(SW_SHOW);
    }
}

/***************************************************************************/
// CDlgParametersSpectrumPage::OnModifiedShowSmoothedPlots  Lpc or Cepstral checkbox modified
/***************************************************************************/
void CDlgParametersSpectrumPage::OnModifiedShowSmoothedPlots() {

    BOOL bShowLpcSpectrum = (BOOL)IsDlgButtonChecked(IDC_SPECTRUM_LPCSMOOTH);
    if (bShowLpcSpectrum != m_bShowLpcSpectrum) {
        m_bShowLpcSpectrum = bShowLpcSpectrum;
        m_bModified = TRUE;
        SetModified(TRUE); // data modified, enable apply button
        GetDlgItem(IDC_SPECTRUM_FORMANTFREQ)->EnableWindow(bShowLpcSpectrum);
        GetDlgItem(IDC_SPECTRUM_FORMANTBANDWIDTH)->EnableWindow(bShowLpcSpectrum);
        GetDlgItem(IDC_SPECTRUM_FORMANTPOWER)->EnableWindow(bShowLpcSpectrum);
    }

    BOOL bShowCepSpectrum = (BOOL)IsDlgButtonChecked(IDC_SPECTRUM_CEPSMOOTH);
    if (bShowCepSpectrum != m_bShowCepSpectrum) {
        m_bShowCepSpectrum = bShowCepSpectrum;
        m_bModified = TRUE;
        SetModified(TRUE); // data modified, enable apply button
        GetDlgItem(IDC_SPECTRUM_SMOOTHLEVEL)->EnableWindow(bShowCepSpectrum);
        GetDlgItem(IDC_SPECTRUM_SMOOTHEDIT)->EnableWindow(bShowCepSpectrum);
        m_SpinSmoothLevel.EnableWindow(bShowCepSpectrum);
    }

    GetDlgItem(IDC_SPECTRUM_PEAKFACTOR)->EnableWindow(bShowLpcSpectrum || bShowCepSpectrum);
    GetDlgItem(IDC_SPECTRUM_PEAKEDIT)->EnableWindow(bShowLpcSpectrum || bShowCepSpectrum);
    m_SpinSharpening.EnableWindow(bShowLpcSpectrum || bShowCepSpectrum);
}

/***************************************************************************/
// CDlgParametersSpectrumPage::OnModifiedShowFormantFreq  Frequency checkbox modified
/***************************************************************************/
void CDlgParametersSpectrumPage::OnModifiedShowFormantFreq() {

    BOOL bShowFormantFreq = (BOOL)IsDlgButtonChecked(IDC_SPECTRUM_FORMANTFREQ);
    if (bShowFormantFreq != m_bShowFormantFreq) {
        m_bShowFormantFreq = bShowFormantFreq;
        m_bModified = TRUE;
        SetModified(TRUE); // data modified, enable apply button
    }
    if (bShowFormantFreq) {
        GetDlgItem(IDC_SPECTRUM_FORMANTBANDWIDTH)->EnableWindow(TRUE);
    } else {
        m_bShowFormantBandwidth = FALSE;
        CheckDlgButton(IDC_SPECTRUM_FORMANTBANDWIDTH, FALSE);
        GetDlgItem(IDC_SPECTRUM_FORMANTBANDWIDTH)->EnableWindow(FALSE);
    }
}

/***************************************************************************/
// CDlgParametersSpectrumPage::OnModifiedShowFormantBandwidth  Bandwidth checkbox modified
/***************************************************************************/
void CDlgParametersSpectrumPage::OnModifiedShowFormantBandwidth() {

    BOOL bShowFormantBandwidth = (BOOL)IsDlgButtonChecked(IDC_SPECTRUM_FORMANTBANDWIDTH);
    if (bShowFormantBandwidth != m_bShowFormantBandwidth) {
        m_bShowFormantBandwidth = bShowFormantBandwidth;
        m_bModified = TRUE;
        SetModified(TRUE); // data modified, enable apply button
    }
}

/***************************************************************************/
// CDlgParametersSpectrumPage::OnModifiedShowFormantPower  Power checkbox modified
/***************************************************************************/
void CDlgParametersSpectrumPage::OnModifiedShowFormantPower() {

    BOOL bShowFormantPower = (BOOL)IsDlgButtonChecked(IDC_SPECTRUM_FORMANTPOWER);
    if (bShowFormantPower != m_bShowFormantPower) {
        m_bShowFormantPower = bShowFormantPower;
        m_bModified = TRUE;
        SetModified(TRUE); // data modified, enable apply button
    }
}


//###########################################################################
// CDlgParametersFormantsPage property page
// Displays all controls to change formants calculation parameters.

/////////////////////////////////////////////////////////////////////////////
// CDlgParametersFormantsPage message map

BEGIN_MESSAGE_MAP(CDlgParametersFormantsPage, CPropertyPage)
    //{{AFX_MSG_MAP(CDlgParametersFormantsPage)
    ON_BN_CLICKED(IDC_FORMANTS_CEPSTRAL, OnModifiedSource)
    ON_BN_CLICKED(IDC_FORMANTS_TRACK, OnModifiedTrack)
    ON_BN_CLICKED(IDC_FORMANTS_TRACK_SMOOTH, OnModifiedSmooth)
    ON_BN_CLICKED(IDC_FORMANTS_MELSCALE, OnModifiedScale)
    ON_CBN_EDITCHANGE(IDC_FORMANT_VOWELS, OnEditChangeFormantVowels)
    ON_CBN_SELCHANGE(IDC_FORMANT_VOWELS, OnEditChangeFormantVowels)
    ON_BN_CLICKED(IDC_FORMANTS_EDIT, OnFormantVowelsEdit)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgParametersFormantsPage construction/destruction/creation

/***************************************************************************/
// CDlgParametersFormantsPage::CDlgParametersFormantsPage Constructor
/***************************************************************************/
CDlgParametersFormantsPage::CDlgParametersFormantsPage() : CPropertyPage(CDlgParametersFormantsPage::IDD) {

    //{{AFX_DATA_INIT(CDlgParametersFormantsPage)
    //}}AFX_DATA_INIT
    m_bModified = FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// CDlgParametersFormantsPage helper functions

/***************************************************************************/
// CDlgParametersFormantsPage::DoDataExchange Data exchange
/***************************************************************************/
void CDlgParametersFormantsPage::DoDataExchange(CDataExchange * pDX) {

    CPropertyPage::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CDlgParametersFormantsPage)
    DDX_Control(pDX, IDC_FORMANTS_EDIT, m_cFormantVowelEdit);
    DDX_Control(pDX, IDC_FORMANT_VOWELS, m_cVowelSet);
    DDX_Check(pDX, IDC_FORMANTS_CEPSTRAL, m_bCepstralFormants);
    DDX_Check(pDX, IDC_FORMANTS_TRACK, m_bTrackFormants);
    DDX_Check(pDX, IDC_FORMANTS_TRACK_SMOOTH, m_bSmoothFormants);
    DDX_Check(pDX, IDC_FORMANTS_MELSCALE, m_bMelScale);
    //}}AFX_DATA_MAP
}

/***************************************************************************/
// CDlgParametersFormantsPage::Apply Apply button hit
/***************************************************************************/
void CDlgParametersFormantsPage::Apply() {

    if (m_bModified) { // retrieve data
        // get pointer to document
        CMainFrame * pMDIFrameWnd = (CMainFrame *)AfxGetMainWnd();
        ASSERT(pMDIFrameWnd->IsKindOf(RUNTIME_CLASS(CMainFrame)));
        CSaView * pView = (CSaView *)pMDIFrameWnd->GetCurrSaView();
        CSaDoc * pDoc = (CSaDoc *)pView->GetDocument();
        CProcessFormants * pFormants = pDoc->GetFormants();
        // save member data
        FormantParm * pFormantParms = pFormants->GetFormantParms();
        /*
        if (m_nDataSource)
        {
        pFormantParms->bFromLpcSpectrum = FALSE;
        pFormantParms->bFromCepstralSpectrum = TRUE;
        }
        else
        {
        pFormantParms->bFromLpcSpectrum = TRUE;
        pFormantParms->bFromCepstralSpectrum = FALSE;
        }
        */
        pFormantParms->bFromCepstralSpectrum = m_bCepstralFormants;
        pFormantParms->bTrackFormants = m_bTrackFormants;
        if (pFormantParms->bMelScale != m_bMelScale ||
                pFormantParms->bSmoothFormants != m_bSmoothFormants) {
            pFormants->ResetTracking();
        }
        pFormantParms->bMelScale = m_bMelScale;
        pFormantParms->bSmoothFormants = m_bSmoothFormants;

        GetVowelSets().SetDefaultSet(m_cVowelSet.GetCurSel());

        // invalidate processed data
        pDoc->GetFormants()->SetDataInvalid();
        m_bModified = FALSE;
        SetModified(FALSE);
        // refresh all graphs
        pView->RefreshGraphs(TRUE, TRUE);
    }
}

BOOL CDlgParametersFormantsPage::OnInitDialog() {

    // get pointer to document
    CMainFrame * pMDIFrameWnd = (CMainFrame *)AfxGetMainWnd();
    ASSERT(pMDIFrameWnd->IsKindOf(RUNTIME_CLASS(CMainFrame)));
    CSaView * pView = (CSaView *)pMDIFrameWnd->GetCurrSaView();
    CSaDoc * pDoc = (CSaDoc *)pView->GetDocument();
    CProcessFormants * pFormants = pDoc->GetFormants();

    // initialize member data
    FormantParm * pFormantParms = pFormants->GetFormantParms();

    m_bCepstralFormants = pFormantParms->bFromCepstralSpectrum;
    m_bTrackFormants = pFormantParms->bTrackFormants;
    m_bSmoothFormants = pFormantParms->bSmoothFormants;
    m_bMelScale = pFormantParms->bMelScale;

    CPropertyPage::OnInitDialog();
    if (m_bAccess) {
        GetDlgItem(IDC_FORMANTS_CEPSTRAL)->ShowWindow(SW_SHOW);
        GetDlgItem(IDC_FORMANTS_TRACK)->ShowWindow(SW_SHOW);
        GetDlgItem(IDC_FORMANTS_TRACK_SMOOTH)->ShowWindow(SW_SHOW);
        GetDlgItem(IDC_STATIC_DATA_SET)->ShowWindow(SW_SHOW);
        GetDlgItem(IDC_FORMANT_VOWELS)->ShowWindow(SW_SHOW);
        GetDlgItem(IDC_FORMANTS_EDIT)->ShowWindow(SW_SHOW);
        GetDlgItem(IDC_STATIC_TEXT)->ShowWindow(SW_SHOW);
    }

    //  if (pFormantParms->bFromLpcSpectrum) m_nDataSource = 0;
    //  else m_nDataSource = 1;
    BOOL bStaticMode = (pView->GetGraphUpdateMode() == STATIC_UPDATE);
    GetDlgItem(IDC_FORMANTS_CEPSTRAL)->EnableWindow(bStaticMode);
    CheckDlgButton(IDC_FORMANTS_CEPSTRAL, pFormantParms->bFromCepstralSpectrum);
    GetDlgItem(IDC_FORMANTS_TRACK)->EnableWindow(pView->IsAnimationRequested());
    CheckDlgButton(IDC_FORMANTS_TRACK, pFormantParms->bTrackFormants);
    GetDlgItem(IDC_FORMANTS_TRACK_SMOOTH)->EnableWindow(pView->IsAnimationRequested());
    CheckDlgButton(IDC_FORMANTS_TRACK_SMOOTH, pFormantParms->bSmoothFormants);
    CheckDlgButton(IDC_FORMANTS_MELSCALE, pFormantParms->bMelScale);
    UpdateData(TRUE);
    PopulateVowelSetCombo(m_cVowelSet);

    int curSel = m_cVowelSet.GetCurSel();
    BOOL enable = GetVowelSets()[curSel].IsUser();
    m_cFormantVowelEdit.EnableWindow(TRUE);
    return TRUE;
}

void CDlgParametersFormantsPage::OnModifiedSource() {

    BOOL bCepstralFormants= (BOOL)IsDlgButtonChecked(IDC_FORMANTS_CEPSTRAL);
    if (bCepstralFormants != m_bCepstralFormants) {
        m_bCepstralFormants = bCepstralFormants;
        m_bModified = TRUE;
        SetModified(TRUE); // data modified, enable apply button
    }
}

void CDlgParametersFormantsPage::OnEditChangeFormantVowels() {

    m_bModified = TRUE;
    SetModified(TRUE); // data modified, enable apply button

    int curSel = m_cVowelSet.GetCurSel();
    BOOL enable = GetVowelSets()[curSel].IsUser();
    m_cFormantVowelEdit.EnableWindow(TRUE);
}


void CDlgParametersFormantsPage::OnFormantVowelsEdit() {

    GetVowelSets().SetDefaultSet(m_cVowelSet.GetCurSel());

    CDlgVowelFormants cEdit(GetVowelSets()[m_cVowelSet.GetCurSel()], this);

    if (cEdit.DoModal() == IDOK) { // Edited
        PopulateVowelSetCombo(m_cVowelSet);

        // get pointer to document
        CMainFrame * pMDIFrameWnd = (CMainFrame *)AfxGetMainWnd();
        ASSERT(pMDIFrameWnd->IsKindOf(RUNTIME_CLASS(CMainFrame)));
        CSaView * pView = (CSaView *)pMDIFrameWnd->GetCurrSaView();

        // refresh all graphs
        pView->RefreshGraphs(TRUE, TRUE);
    }
}

void CDlgParametersFormantsPage::PopulateVowelSetCombo(CComboBox & cBox) {

    cBox.ResetContent();
    CVowelFormantSets & cSets = GetVowelSets();

    for (unsigned int i=0; i < cSets.size(); i++) {
        cBox.AddString(cSets[i].GetName());
    }

    cBox.SetCurSel(cSets.GetDefaultSet());
}

/***************************************************************************/
// CDlgParametersFormantsPage::OnModifiedTrack  Formant tracking checkbox modified
/***************************************************************************/
void CDlgParametersFormantsPage::OnModifiedTrack() {

    BOOL bTrackFormants= (BOOL)IsDlgButtonChecked(IDC_FORMANTS_TRACK);
    if (bTrackFormants) {
        GetDlgItem(IDC_FORMANTS_TRACK_SMOOTH)->EnableWindow(TRUE);
    } else {
        GetDlgItem(IDC_FORMANTS_TRACK_SMOOTH)->EnableWindow(FALSE);
        CheckDlgButton(IDC_FORMANTS_TRACK_SMOOTH, FALSE);
    }
    if (bTrackFormants != m_bTrackFormants) {
        m_bTrackFormants = bTrackFormants;
        m_bModified = TRUE;
        SetModified(TRUE); // data modified, enable apply button
    }
}

/***************************************************************************/
// CDlgParametersFormantsPage::OnModifiedSmooth  Formant smoothing checkbox modified
/***************************************************************************/
void CDlgParametersFormantsPage::OnModifiedSmooth() {

    BOOL bSmoothFormants= (BOOL)IsDlgButtonChecked(IDC_FORMANTS_TRACK_SMOOTH);
    if (bSmoothFormants != m_bSmoothFormants) {
        m_bSmoothFormants = bSmoothFormants;
        m_bModified = TRUE;
        SetModified(TRUE); // data modified, enable apply button
    }
}
/***************************************************************************/
// CDlgParametersFormantsPage::OnModifiedScale  Mel scale checkbox modified
/***************************************************************************/
void CDlgParametersFormantsPage::OnModifiedScale() {

    BOOL bMelScale = (BOOL)IsDlgButtonChecked(IDC_FORMANTS_MELSCALE);
    if (bMelScale != m_bMelScale) {
        m_bMelScale = bMelScale;
        m_bModified = TRUE;
        SetModified(TRUE); // data modified, enable apply button
    }
}

//###########################################################################
// CDlgParametersSDPPage property page
// Displays all controls to change SDP calculation parameters.

BEGIN_MESSAGE_MAP(CDlgParametersSDPPage, CPropertyPage)
    ON_COMMAND(IDC_SDP_PANESSCROLL, OnPanesScroll)
    ON_EN_KILLFOCUS(IDC_SDP_PANESEDIT, OnKillfocusPanes)
    ON_COMMAND(IDC_SDP_STEPSSCROLL, OnStepsScroll)
    ON_EN_KILLFOCUS(IDC_SDP_STEPSEDIT, OnKillfocusSteps)
    ON_COMMAND(IDC_SDP_UPPERSCROLL, OnUpperScroll)
    ON_EN_KILLFOCUS(IDC_SDP_UPPEREDIT, OnKillfocusUpper)
    ON_CBN_SELCHANGE(IDC_SDP_STEPMODE, OnStepMode)
    ON_BN_CLICKED(IDC_SDP_AVERAGE, OnModified)
    ON_EN_CHANGE(IDC_SDP_PANESEDIT, OnChange)
    ON_EN_CHANGE(IDC_SDP_STEPSEDIT, OnChange)
    ON_EN_CHANGE(IDC_SDP_UPPEREDIT, OnChange)
END_MESSAGE_MAP()

CDlgParametersSDPPage::CDlgParametersSDPPage() : CPropertyPage(CDlgParametersSDPPage::IDD) {

    m_bAverage = FALSE;
    m_nStepMode = 0;
    m_bModified = FALSE;
}

void CDlgParametersSDPPage::DoDataExchange(CDataExchange * pDX) {

    CPropertyPage::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_SDP_PANESEDIT, m_nPanes);
    DDX_Text(pDX, IDC_SDP_STEPSEDIT, m_nSteps);
    DDX_Text(pDX, IDC_SDP_UPPEREDIT, m_nUpperBound);
    DDX_Check(pDX, IDC_SDP_AVERAGE, m_bAverage);
    DDX_CBIndex(pDX, IDC_SDP_STEPMODE, m_nStepMode);
}

void CDlgParametersSDPPage::Apply() {

    if (m_bModified) { // retrieve data
        // get pointer to SDP parameters
        CMainFrame * pMDIFrameWnd = (CMainFrame *)AfxGetMainWnd();
        ASSERT(pMDIFrameWnd->IsKindOf(RUNTIME_CLASS(CMainFrame)));
        CSaView * pView = (CSaView *)pMDIFrameWnd->GetCurrSaView();
        CSaDoc * pDoc = (CSaDoc *)pView->GetDocument();
        SDPParm * pSDPParm = pDoc->GetSDPParm();
        // save the parameters
        UpdateData(TRUE); // necessary for average check box
        pSDPParm->nPanes = m_nPanes;
        pSDPParm->nStepMode = m_nStepMode;
        pSDPParm->nSteps = m_nSteps;
        pSDPParm->nUpperBound = m_nUpperBound;
        pSDPParm->bAverage = m_bAverage;
        m_bModified = FALSE;
        SetModified(FALSE);
        // invalidate SDP data and refresh the SDP graphs
        UINT * pnID = pView->GetGraphIDs();
        for (int nLoop = 0; nLoop < MAX_GRAPHS_NUMBER; nLoop++) {
            if ((pnID[nLoop] == IDD_SDP_A) || (pnID[nLoop] == IDD_SDP_B)) {
                pDoc->GetSDP(pnID[nLoop] == IDD_SDP_A ? 0:1)->SetDataInvalid();
                pView->GetGraph(nLoop)->RedrawGraph(TRUE, TRUE);
            }
        }
    }
}

BOOL CDlgParametersSDPPage::OnInitDialog() {

    CPropertyPage::OnInitDialog();
    CRect rWnd;
    // build and place the panes spin control
    m_SpinPanes.Init(IDC_SDP_PANESSCROLL, this);
    // build and place the steps spin control
    m_SpinSteps.Init(IDC_SDP_STEPSSCROLL, this);
    // build and place the power upper boundary spin control
    m_SpinUpper.Init(IDC_SDP_UPPERSCROLL, this);
    // get the SDP parameters from mainframe
    CMainFrame * pMDIFrameWnd = (CMainFrame *)AfxGetMainWnd();
    ASSERT(pMDIFrameWnd->IsKindOf(RUNTIME_CLASS(CMainFrame)));
    CSaView * pView = (CSaView *)pMDIFrameWnd->GetCurrSaView();
    CSaDoc * pDoc = (CSaDoc *)pView->GetDocument();
    SDPParm * pSDPParm = pDoc->GetSDPParm();
    // initialize member data
    m_nPanes = pSDPParm->nPanes;
    m_nStepMode = pSDPParm->nStepMode;
    m_nSteps = pSDPParm->nSteps;
    m_nUpperBound = pSDPParm->nUpperBound;
    m_bAverage = pSDPParm->bAverage;
    if (!m_nStepMode) {
        // auto steps, disable steps
        GetDlgItem(IDC_SDP_STEPSTEXT)->EnableWindow(FALSE);
        GetDlgItem(IDC_SDP_STEPSEDIT)->EnableWindow(FALSE);
        m_SpinSteps.EnableWindow(FALSE);
        GetDlgItem(IDC_SDP_STEPSDIM)->EnableWindow(FALSE);
    }
    UpdateData(FALSE);
    return TRUE;
}

/***************************************************************************/
// CDlgParametersSDPPage::OnPanesScroll Panes spin control hit
/***************************************************************************/
void CDlgParametersSDPPage::OnPanesScroll() {

    UINT nData = GetDlgItemInt(IDC_SDP_PANESEDIT, NULL, TRUE);
    if (m_SpinPanes.UpperButtonClicked()) {
        if (nData < 8) {
            nData++;
        }
    } else {
        if (nData > 1) {
            nData--;
        }
    }
    if (nData != m_nPanes) {
        m_nPanes = nData;
        m_bModified = TRUE;
        SetModified(TRUE); // data modified, enable apply button
        SetDlgItemInt(IDC_SDP_PANESEDIT, m_nPanes, TRUE);
    }
}

/***************************************************************************/
// CDlgParametersSDPPage::OnChange page changed
/***************************************************************************/
void CDlgParametersSDPPage::OnChange() {

    m_bModified = TRUE;
    SetModified(TRUE); // data modified, enable apply button
}

/***************************************************************************/
// CDlgParametersSDPPage::OnKillfocusPanes Panes changed
/***************************************************************************/
void CDlgParametersSDPPage::OnKillfocusPanes() {

    UINT nData = GetDlgItemInt(IDC_SDP_PANESEDIT, NULL, TRUE);
    if (nData > 8) {
        nData = 8;
    }
    if (nData < 1) {
        nData = 1;
    }
    if (nData != m_nPanes) {
        m_nPanes = nData;
        m_bModified = TRUE;
        SetModified(TRUE); // data modified, enable apply button
    }
    SetDlgItemInt(IDC_SDP_PANESEDIT, m_nPanes, TRUE);
}

/***************************************************************************/
// CDlgParametersSDPPage::OnStepsScroll Steps spin control hit
/***************************************************************************/
void CDlgParametersSDPPage::OnStepsScroll() {

    UINT nData = GetDlgItemInt(IDC_SDP_STEPSEDIT, NULL, TRUE);
    if (m_SpinSteps.UpperButtonClicked()) {
        if (nData < 0xFFFF) {
            nData++;
        }
    } else {
        if (nData > 1) {
            nData--;
        }
    }
    if (nData != m_nSteps) {
        m_nSteps = nData;
        m_bModified = TRUE;
        SetModified(TRUE); // data modified, enable apply button
        SetDlgItemInt(IDC_SDP_STEPSEDIT, m_nSteps, TRUE);
    }
}

/***************************************************************************/
// CDlgParametersSDPPage::OnKillfocusSteps Steps changed
/***************************************************************************/
void CDlgParametersSDPPage::OnKillfocusSteps() {

    UINT nData = GetDlgItemInt(IDC_SDP_STEPSEDIT, NULL, TRUE);
    if (nData < 1) {
        nData = 1;
    }
    if (nData != m_nSteps) {
        m_nSteps = nData;
        m_bModified = TRUE;
        SetModified(TRUE); // data modified, enable apply button
    }
    SetDlgItemInt(IDC_SDP_STEPSEDIT, m_nSteps, TRUE);
}

/***************************************************************************/
// CDlgParametersSDPPage::OnUpperScroll Upper boundary spin control hit
/***************************************************************************/
void CDlgParametersSDPPage::OnUpperScroll() {

    UINT nData = GetDlgItemInt(IDC_SDP_UPPEREDIT, NULL, TRUE);
    if (m_SpinUpper.UpperButtonClicked()) {
        if (nData < 360) {
            nData++;
        }
    } else {
        if (nData > 1) {
            nData--;
        }
    }
    if (nData != m_nUpperBound) {
        m_nUpperBound = nData;
        m_bModified = TRUE;
        SetModified(TRUE); // data modified, enable apply button
        SetDlgItemInt(IDC_SDP_UPPEREDIT, m_nUpperBound, TRUE);
    }
}

/***************************************************************************/
// CDlgParametersSDPPage::OnKillfocusUpper Upper boundary changed
/***************************************************************************/
void CDlgParametersSDPPage::OnKillfocusUpper() {

    UINT nData = GetDlgItemInt(IDC_SDP_UPPEREDIT, NULL, TRUE);
    if (nData > 360) {
        nData = 360;
    }
    if (nData < 1) {
        nData = 1;
    }
    if (nData != m_nUpperBound) {
        m_nUpperBound = nData;
        m_bModified = TRUE;
        SetModified(TRUE); // data modified, enable apply button
    }
    SetDlgItemInt(IDC_SDP_UPPEREDIT, m_nUpperBound, TRUE);
}

/***************************************************************************/
// CDlgParametersSDPPage::OnStepMode Steps mode selection changed
/***************************************************************************/
void CDlgParametersSDPPage::OnStepMode() {

    UpdateData(TRUE);
    if (!m_nStepMode) {
        // auto steps, disable steps
        GetDlgItem(IDC_SDP_STEPSTEXT)->EnableWindow(FALSE);
        GetDlgItem(IDC_SDP_STEPSEDIT)->EnableWindow(FALSE);
        m_SpinSteps.EnableWindow(FALSE);
        GetDlgItem(IDC_SDP_STEPSDIM)->EnableWindow(FALSE);
    } else {
        // manual steps, enable steps
        GetDlgItem(IDC_SDP_STEPSTEXT)->EnableWindow(TRUE);
        GetDlgItem(IDC_SDP_STEPSEDIT)->EnableWindow(TRUE);
        m_SpinSteps.EnableWindow(TRUE);
        GetDlgItem(IDC_SDP_STEPSDIM)->EnableWindow(TRUE);
    }
    m_bModified = TRUE;
    SetModified(TRUE); // data modified, enable apply button
}

/***************************************************************************/
// CDlgParametersSDPPage::OnModified Data modified
/***************************************************************************/
void CDlgParametersSDPPage::OnModified() {

    m_bModified = TRUE;
    SetModified(TRUE); // data modified, enable apply button
}


/////////////////////////////////////////////////////////////////////////////
// CDlgParametersMusicPage dialog
CDlgParametersMusicPage::CDlgParametersMusicPage() : CPropertyPage(CDlgParametersMusicPage::IDD) {

    m_nDynamic = FALSE;
    m_nWeighted = FALSE;
}

void CDlgParametersMusicPage::DoDataExchange(CDataExchange * pDX) {

    CPropertyPage::DoDataExchange(pDX);
    DDX_Check(pDX, IDC_TWC_DYNAMIC, m_nDynamic);
    DDX_Check(pDX, IDC_TWC_WEIGHTED, m_nWeighted);
    DDX_Text(pDX, IDC_PITCH_LOWERBOUNDEDIT, m_nLowerBound);
    DDX_Text(pDX, IDC_PITCH_UPPERBOUNDEDIT, m_nUpperBound);
    DDX_CBIndex(pDX, IDC_PITCH_RANGE, m_nRange);
    DDX_Text(pDX, IDC_CALC_LOWERBOUNDEDIT, m_nCalcLowerBound);
    DDX_Text(pDX, IDC_CALC_UPPERBOUNDEDIT, m_nCalcUpperBound);
    DDX_CBIndex(pDX, IDC_CALC_RANGE, m_nCalcRange);
}

BEGIN_MESSAGE_MAP(CDlgParametersMusicPage, CPropertyPage)
    ON_BN_CLICKED(IDC_TWC_WEIGHTED, OnClickWeighted)
    ON_BN_CLICKED(IDC_TWC_DYNAMIC, OnClickDynamic)
    ON_COMMAND(IDC_PITCH_UPPERBOUNDSCROLL, OnUpperBoundScroll)
    ON_EN_KILLFOCUS(IDC_PITCH_UPPERBOUNDEDIT, OnKillfocusUpperBound)
    ON_COMMAND(IDC_PITCH_LOWERBOUNDSCROLL, OnLowerBoundScroll)
    ON_EN_KILLFOCUS(IDC_PITCH_LOWERBOUNDEDIT, OnKillfocusLowerBound)
    ON_CBN_SELCHANGE(IDC_PITCH_RANGE, OnRange)
    ON_EN_CHANGE(IDC_PITCH_UPPERBOUNDEDIT, OnChange)
    ON_EN_CHANGE(IDC_PITCH_LOWERBOUNDEDIT, OnChange)
    ON_COMMAND(IDC_CALC_UPPERBOUNDSCROLL, OnCalcUpperBoundScroll)
    ON_EN_KILLFOCUS(IDC_CALC_UPPERBOUNDEDIT, OnKillfocusCalcUpperBound)
    ON_COMMAND(IDC_CALC_LOWERBOUNDSCROLL, OnCalcLowerBoundScroll)
    ON_EN_KILLFOCUS(IDC_CALC_LOWERBOUNDEDIT, OnKillfocusCalcLowerBound)
    ON_CBN_SELCHANGE(IDC_CALC_RANGE, OnCalcRange)
    ON_EN_CHANGE(IDC_CALC_UPPERBOUNDEDIT, OnChange)
    ON_EN_CHANGE(IDC_CALC_LOWERBOUNDEDIT, OnChange)
END_MESSAGE_MAP()

BOOL CDlgParametersMusicPage::OnInitDialog() {

    CPropertyPage::OnInitDialog();

    // build and place the upper boundary spin control
    m_SpinUpperBound.Init(IDC_PITCH_UPPERBOUNDSCROLL, this);
    // build and place the lower boundary spin control
    m_SpinLowerBound.Init(IDC_PITCH_LOWERBOUNDSCROLL, this);

    // build and place the upper boundary spin control
    m_SpinCalcUpperBound.Init(IDC_CALC_UPPERBOUNDSCROLL, this);
    // build and place the lower boundary spin control
    m_SpinCalcLowerBound.Init(IDC_CALC_LOWERBOUNDSCROLL, this);

    CMainFrame * pMDIFrameWnd = (CMainFrame *)AfxGetMainWnd();
    ASSERT(pMDIFrameWnd->IsKindOf(RUNTIME_CLASS(CMainFrame)));
    CSaView * pView = (CSaView *)pMDIFrameWnd->GetCurrSaView();
    m_nDynamic  = !pView->GetStaticTWC();
    m_nWeighted = !pView->GetNormalMelogram();

    // get pointer to document
    CSaDoc * pDoc = (CSaDoc *)pView->GetDocument();
    const MusicParm * pParm = pDoc->GetMusicParm();
    m_nUpperBound = pParm->nUpperBound;
    m_nLowerBound = pParm->nLowerBound;

    CMainFrame * pMainFrame = (CMainFrame *)AfxGetMainWnd();
    ASSERT(pMainFrame->IsKindOf(RUNTIME_CLASS(CMainFrame)));
    m_nManualLowerBound = pMainFrame->GetMusicParmDefaults()->nManualLower;
    m_nManualUpperBound = pMainFrame->GetMusicParmDefaults()->nManualUpper;

    m_nRange = pParm->nRangeMode;
    if (!m_nRange) {
        MusicParm::GetAutoRange(pDoc, m_nUpperBound, m_nLowerBound);
        // auto range, disable boundaries
        GetDlgItem(IDC_PITCH_UPPERBOUNDTITLE)->EnableWindow(FALSE);
        GetDlgItem(IDC_PITCH_UPPERBOUNDEDIT)->EnableWindow(FALSE);
        GetDlgItem(IDC_PITCH_UPPERBOUNDTEXT)->EnableWindow(FALSE);
        m_SpinUpperBound.EnableWindow(FALSE);
        GetDlgItem(IDC_PITCH_LOWERBOUNDTITLE)->EnableWindow(FALSE);
        GetDlgItem(IDC_PITCH_LOWERBOUNDEDIT)->EnableWindow(FALSE);
        GetDlgItem(IDC_PITCH_LOWERBOUNDTEXT)->EnableWindow(FALSE);
        m_SpinLowerBound.EnableWindow(FALSE);

    }
    m_nCalcUpperBound = pParm->nCalcUpperBound;
    m_nCalcLowerBound = pParm->nCalcLowerBound;

    m_nManualCalcLowerBound = pMainFrame->GetMusicParmDefaults()->nManualCalcLower;
    m_nManualCalcUpperBound = pMainFrame->GetMusicParmDefaults()->nManualCalcUpper;

    m_nCalcRange = pParm->nCalcRangeMode;
    if (!m_nCalcRange) {
        // auto range, disable boundaries
        GetDlgItem(IDC_CALC_UPPERBOUNDTITLE)->EnableWindow(FALSE);
        GetDlgItem(IDC_CALC_UPPERBOUNDEDIT)->EnableWindow(FALSE);
        GetDlgItem(IDC_CALC_UPPERBOUNDTEXT)->EnableWindow(FALSE);
        m_SpinCalcUpperBound.EnableWindow(FALSE);
        GetDlgItem(IDC_CALC_LOWERBOUNDTITLE)->EnableWindow(FALSE);
        GetDlgItem(IDC_CALC_LOWERBOUNDEDIT)->EnableWindow(FALSE);
        GetDlgItem(IDC_CALC_LOWERBOUNDTEXT)->EnableWindow(FALSE);
        m_SpinCalcLowerBound.EnableWindow(FALSE);

    }
    m_bModified = FALSE;
    UpdateData(FALSE);
    return TRUE;  // return TRUE  unless you set the focus to a control
}

/***************************************************************************/
// CDlgParametersMusicPage::OnChange page changed
/***************************************************************************/
void CDlgParametersMusicPage::OnChange() {

    m_bModified = TRUE;
    SetModified(TRUE); // data modified, enable apply button
}

/***************************************************************************/
// 09/25/2000 - DDO
/***************************************************************************/
void CDlgParametersMusicPage::OnClickWeighted() {

    m_bModified = TRUE;
    SetModified(TRUE);
}

/***************************************************************************/
// 09/25/2000 - DDO
/***************************************************************************/
void CDlgParametersMusicPage::OnClickDynamic() {

    m_bModified = TRUE;
    SetModified(TRUE);
}

/***************************************************************************/
// CDlgParametersMusicPage::OnUpperBoundScroll Upper boundary spin control hit
/***************************************************************************/
void CDlgParametersMusicPage::OnUpperBoundScroll() {

    if (m_nRange) {
        int nData = GetDlgItemInt(IDC_PITCH_UPPERBOUNDEDIT, NULL, TRUE);
        if (m_SpinUpperBound.UpperButtonClicked()) {
            nData++;
        } else {
            nData--;
        }

        if (nData > 100) {
            nData = 100;
        }
        if (nData < 1) {
            nData = 1;
        }
        if (nData != m_nUpperBound) {
            m_nUpperBound = nData;
            m_bModified = TRUE;
            SetModified(TRUE); // data modified, enable apply button
            SetDlgItemInt(IDC_PITCH_UPPERBOUNDEDIT, m_nUpperBound, TRUE);
            if (m_nLowerBound >= m_nUpperBound) {
                m_nLowerBound = m_nUpperBound - 1;
                SetDlgItemInt(IDC_PITCH_LOWERBOUNDEDIT, m_nLowerBound, TRUE);
            }
        }
    }
}

/***************************************************************************/
// CDlgParametersMusicPage::OnKillfocusUpperBound Upper boundary changed
/***************************************************************************/
void CDlgParametersMusicPage::OnKillfocusUpperBound() {

    BOOL bTrans;
    int nData = (int)GetDlgItemInt(IDC_PITCH_UPPERBOUNDEDIT, &bTrans, TRUE); //CLW 8/27/99

    if (!bTrans && (nData==0)) {
        nData = 100;
    }
    if (nData > 100) {
        nData = 100;
    }
    if (nData < 1) {
        nData = 1;
    }
    if (nData != m_nUpperBound) {
        m_nUpperBound = nData;
        m_bModified = TRUE;
        SetModified(TRUE); // data modified, enable apply button
        if (m_nLowerBound >= m_nUpperBound) {
            m_nLowerBound = m_nUpperBound - 1;
            SetDlgItemInt(IDC_PITCH_LOWERBOUNDEDIT, m_nLowerBound, TRUE);
        }
    }
    SetDlgItemInt(IDC_PITCH_UPPERBOUNDEDIT, m_nUpperBound, TRUE);
}

/***************************************************************************/
// CDlgParametersMusicPage::OnLowerBoundScroll Lower boundary spin control hit
/***************************************************************************/
void CDlgParametersMusicPage::OnLowerBoundScroll() {

    if (m_nRange) {
        int nData = GetDlgItemInt(IDC_PITCH_LOWERBOUNDEDIT, NULL, TRUE);
        if (m_SpinLowerBound.UpperButtonClicked()) {
            nData++;
        } else {
            nData--;
        }

        if (nData > 99) {
            nData = 99;
        }
        if (nData < 0) {
            nData = 0;
        }
        if (nData != m_nLowerBound) {
            m_nLowerBound = nData;
            m_bModified = TRUE;
            SetModified(TRUE); // data modified, enable apply button
            SetDlgItemInt(IDC_PITCH_LOWERBOUNDEDIT, m_nLowerBound, TRUE);
            if (m_nUpperBound <= m_nLowerBound) {
                m_nUpperBound = m_nLowerBound + 1;
                SetDlgItemInt(IDC_PITCH_UPPERBOUNDEDIT, m_nUpperBound, TRUE);
            }
        }
    }
}

/***************************************************************************/
// CDlgParametersMusicPage::OnKillfocusLowerBound Lower boundary changed
/***************************************************************************/
void CDlgParametersMusicPage::OnKillfocusLowerBound() {

    int nData = GetDlgItemInt(IDC_PITCH_LOWERBOUNDEDIT, NULL, TRUE);

    if (nData > 99) {
        nData = 99;
    }
    if (nData < 0) {
        nData = 0;
    }
    if (nData != m_nLowerBound) {
        m_nLowerBound = nData;
        m_bModified = TRUE;
        SetModified(TRUE); // data modified, enable apply button
        if (m_nUpperBound <= m_nLowerBound) {
            m_nUpperBound = m_nLowerBound + 1;
            SetDlgItemInt(IDC_PITCH_UPPERBOUNDEDIT, m_nUpperBound, TRUE);
        }
    }
    SetDlgItemInt(IDC_PITCH_LOWERBOUNDEDIT, m_nLowerBound, TRUE);
}

/***************************************************************************/
// CDlgParametersMusicPage::OnRange Range selection changed
/***************************************************************************/
void CDlgParametersMusicPage::OnRange() {

    int nOldRange = m_nRange;
    UpdateData(TRUE);

    if (nOldRange == m_nRange) {
        return;    // no change
    }

    if (!m_nRange) {
        m_nManualLowerBound = m_nLowerBound;
        m_nManualUpperBound = m_nUpperBound;

        CMainFrame * pMDIFrameWnd = (CMainFrame *)AfxGetMainWnd();
        ASSERT(pMDIFrameWnd->IsKindOf(RUNTIME_CLASS(CMainFrame)));
        CSaView * pView = (CSaView *)(pMDIFrameWnd->GetCurrSaView());
        CSaDoc * pDoc = (CSaDoc *)pView->GetDocument();

        MusicParm::GetAutoRange(pDoc, m_nUpperBound, m_nLowerBound);

        GetDlgItem(IDC_PITCH_UPPERBOUNDTITLE)->EnableWindow(FALSE);
        GetDlgItem(IDC_PITCH_UPPERBOUNDEDIT)->EnableWindow(FALSE);
        GetDlgItem(IDC_PITCH_UPPERBOUNDTEXT)->EnableWindow(FALSE);
        m_SpinUpperBound.EnableWindow(FALSE);

        GetDlgItem(IDC_PITCH_LOWERBOUNDTITLE)->EnableWindow(FALSE);
        GetDlgItem(IDC_PITCH_LOWERBOUNDEDIT)->EnableWindow(FALSE);
        GetDlgItem(IDC_PITCH_LOWERBOUNDTEXT)->EnableWindow(FALSE);
        m_SpinLowerBound.EnableWindow(FALSE);
    } else {
        // manual range, enable boundaries
        GetDlgItem(IDC_PITCH_UPPERBOUNDTITLE)->EnableWindow(TRUE);
        GetDlgItem(IDC_PITCH_UPPERBOUNDEDIT)->EnableWindow(TRUE);
        GetDlgItem(IDC_PITCH_UPPERBOUNDTEXT)->EnableWindow(TRUE);
        m_SpinUpperBound.EnableWindow(TRUE);
        GetDlgItem(IDC_PITCH_LOWERBOUNDTITLE)->EnableWindow(TRUE);
        GetDlgItem(IDC_PITCH_LOWERBOUNDEDIT)->EnableWindow(TRUE);
        GetDlgItem(IDC_PITCH_LOWERBOUNDTEXT)->EnableWindow(TRUE);
        m_SpinLowerBound.EnableWindow(TRUE);
        m_nLowerBound = m_nManualLowerBound;
        m_nUpperBound = m_nManualUpperBound;
    }
    m_bModified = TRUE;
    SetModified(TRUE); // data modified, enable apply button
    UpdateData(FALSE);
}

/***************************************************************************/
// CDlgParametersMusicPage::OnCalcUpperBoundScroll Upper boundary spin control hit
/***************************************************************************/
void CDlgParametersMusicPage::OnCalcUpperBoundScroll() {

    if (m_nCalcRange) {
        int nData = GetDlgItemInt(IDC_CALC_UPPERBOUNDEDIT, NULL, TRUE);
        if (m_SpinCalcUpperBound.UpperButtonClicked()) {
            nData++;
        } else {
            nData--;
        }

        if (nData > 100) {
            nData = 100;
        }
        if (nData < 1) {
            nData = 1;
        }
        if (nData != m_nCalcUpperBound) {
            m_nCalcUpperBound = nData;
            m_bModified = TRUE;
            SetModified(TRUE); // data modified, enable apply button
            SetDlgItemInt(IDC_CALC_UPPERBOUNDEDIT, m_nCalcUpperBound, TRUE);
            if (m_nCalcLowerBound >= m_nCalcUpperBound) {
                m_nCalcLowerBound = m_nCalcUpperBound - 1;
                SetDlgItemInt(IDC_CALC_LOWERBOUNDEDIT, m_nCalcLowerBound, TRUE);
            }
        }
    }
}

/***************************************************************************/
// CDlgParametersMusicPage::OnKillfocusCalcUpperBound Upper boundary changed
/***************************************************************************/
void CDlgParametersMusicPage::OnKillfocusCalcUpperBound() {

    BOOL bTrans;
    int nData = (int)GetDlgItemInt(IDC_CALC_UPPERBOUNDEDIT, &bTrans, TRUE); //CLW 8/27/99

    if (!bTrans && (nData==0)) {
        nData = 100;
    }
    if (nData > 100) {
        nData = 100;
    }
    if (nData < 1) {
        nData = 1;
    }
    if (nData != m_nCalcUpperBound) {
        m_nCalcUpperBound = nData;
        m_bModified = TRUE;
        SetModified(TRUE); // data modified, enable apply button
        if (m_nCalcLowerBound >= m_nCalcUpperBound) {
            m_nCalcLowerBound = m_nCalcUpperBound - 1;
            SetDlgItemInt(IDC_CALC_LOWERBOUNDEDIT, m_nCalcLowerBound, TRUE);
        }
    }
    SetDlgItemInt(IDC_CALC_UPPERBOUNDEDIT, m_nCalcUpperBound, TRUE);
}

/***************************************************************************/
// CDlgParametersMusicPage::OnCalcLowerBoundScroll Lower boundary spin control hit
/***************************************************************************/
void CDlgParametersMusicPage::OnCalcLowerBoundScroll() {

    if (m_nCalcRange) {
        int nData = GetDlgItemInt(IDC_CALC_LOWERBOUNDEDIT, NULL, TRUE);
        if (m_SpinCalcLowerBound.UpperButtonClicked()) {
            nData++;
        } else {
            nData--;
        }

        if (nData > 99) {
            nData = 99;
        }
        if (nData < 0) {
            nData = 0;
        }
        if (nData != m_nCalcLowerBound) {
            m_nCalcLowerBound = nData;
            m_bModified = TRUE;
            SetModified(TRUE); // data modified, enable apply button
            SetDlgItemInt(IDC_CALC_LOWERBOUNDEDIT, m_nCalcLowerBound, TRUE);
            if (m_nCalcUpperBound <= m_nCalcLowerBound) {
                m_nCalcUpperBound = m_nCalcLowerBound + 1;
                SetDlgItemInt(IDC_CALC_UPPERBOUNDEDIT, m_nCalcUpperBound, TRUE);
            }
        }
    }
}

/***************************************************************************/
// CDlgParametersMusicPage::OnKillfocusCalcLowerBound Lower boundary changed
/***************************************************************************/
void CDlgParametersMusicPage::OnKillfocusCalcLowerBound() {

    int nData = GetDlgItemInt(IDC_CALC_LOWERBOUNDEDIT, NULL, TRUE);

    if (nData > 99) {
        nData = 99;
    }
    if (nData < 0) {
        nData = 0;
    }
    if (nData != m_nCalcLowerBound) {
        m_nCalcLowerBound = nData;
        m_bModified = TRUE;
        SetModified(TRUE); // data modified, enable apply button
        if (m_nCalcUpperBound <= m_nCalcLowerBound) {
            m_nCalcUpperBound = m_nCalcLowerBound + 1;
            SetDlgItemInt(IDC_CALC_UPPERBOUNDEDIT, m_nCalcUpperBound, TRUE);
        }
    }
    SetDlgItemInt(IDC_CALC_LOWERBOUNDEDIT, m_nCalcLowerBound, TRUE);
}

/***************************************************************************/
// CDlgParametersMusicPage::OnCalcRange Range selection changed
/***************************************************************************/
void CDlgParametersMusicPage::OnCalcRange() {

    int nOldRange = m_nCalcRange;
    UpdateData(TRUE);

    if (nOldRange == m_nCalcRange) {
        return;    // no change
    }

    if (!m_nCalcRange) {
        m_nManualCalcLowerBound = m_nCalcLowerBound;
        m_nManualCalcUpperBound = m_nCalcUpperBound;
        // RLJ, 09/21/2000: Bug GPI-01
        // RANGE in Auto Mode, reset boundaries to default values
        // and then disable spin controls for boundaries.
        MusicParm TempParm;
        TempParm.Init(); // Get hardcoded Parm defaults (NOT from *.PSA)
        m_nCalcUpperBound = TempParm.nCalcUpperBound;
        m_nCalcLowerBound = TempParm.nCalcLowerBound;

        GetDlgItem(IDC_CALC_UPPERBOUNDTITLE)->EnableWindow(FALSE);
        GetDlgItem(IDC_CALC_UPPERBOUNDEDIT)->EnableWindow(FALSE);
        GetDlgItem(IDC_CALC_UPPERBOUNDTEXT)->EnableWindow(FALSE);
        m_SpinCalcUpperBound.EnableWindow(FALSE);

        GetDlgItem(IDC_CALC_LOWERBOUNDTITLE)->EnableWindow(FALSE);
        GetDlgItem(IDC_CALC_LOWERBOUNDEDIT)->EnableWindow(FALSE);
        GetDlgItem(IDC_CALC_LOWERBOUNDTEXT)->EnableWindow(FALSE);
        m_SpinCalcLowerBound.EnableWindow(FALSE);
    } else {
        // manual range, enable boundaries
        GetDlgItem(IDC_CALC_UPPERBOUNDTITLE)->EnableWindow(TRUE);
        GetDlgItem(IDC_CALC_UPPERBOUNDEDIT)->EnableWindow(TRUE);
        GetDlgItem(IDC_CALC_UPPERBOUNDTEXT)->EnableWindow(TRUE);
        m_SpinCalcUpperBound.EnableWindow(TRUE);
        GetDlgItem(IDC_CALC_LOWERBOUNDTITLE)->EnableWindow(TRUE);
        GetDlgItem(IDC_CALC_LOWERBOUNDEDIT)->EnableWindow(TRUE);
        GetDlgItem(IDC_CALC_LOWERBOUNDTEXT)->EnableWindow(TRUE);
        m_SpinCalcLowerBound.EnableWindow(TRUE);
        m_nCalcLowerBound = m_nManualCalcLowerBound;
        m_nCalcUpperBound = m_nManualCalcUpperBound;
    }
    m_bModified = TRUE;
    SetModified(TRUE); // data modified, enable apply button
    UpdateData(FALSE);
}

/***************************************************************************/
// 09/25/2000 - DDO
/***************************************************************************/
void CDlgParametersMusicPage::Apply() {

    // CLW 10/26/00 m_hWnd is NULL if music page has never been accessed
    if (m_bModified && m_hWnd) {
        UpdateData(TRUE);
        CMainFrame * pMDIFrameWnd = (CMainFrame *)AfxGetMainWnd();
        ASSERT(pMDIFrameWnd->IsKindOf(RUNTIME_CLASS(CMainFrame)));
        CSaView * pView = (CSaView *)pMDIFrameWnd->GetCurrSaView();
        pView->SetStaticTWC(!m_nDynamic);
        pView->SetNormalMelogram(!m_nWeighted);
        // get pointer to document
        CSaDoc * pDoc = (CSaDoc *)pView->GetDocument();
        MusicParm cParm = *pDoc->GetMusicParm();
        MusicParm * pParm = &cParm;
        pParm->nUpperBound = m_nUpperBound;
        pParm->nLowerBound = m_nLowerBound;
        pParm->nRangeMode = m_nRange;
        if (m_nRange) {
            CMainFrame * pMainFrame = (CMainFrame *)AfxGetMainWnd();
            ASSERT(pMainFrame->IsKindOf(RUNTIME_CLASS(CMainFrame)));
            MusicParm cParm = *pMainFrame->GetMusicParmDefaults();
            cParm.nManualLower = m_nLowerBound;
            cParm.nManualUpper = m_nUpperBound;
            pMainFrame->SetMusicParmDefaults(cParm);
        }
        pParm->nCalcUpperBound = m_nCalcUpperBound;
        pParm->nCalcLowerBound = m_nCalcLowerBound;
        pParm->nCalcRangeMode = m_nCalcRange;
        pDoc->SetMusicParm(cParm);
        if (m_nCalcRange) {
            CMainFrame * pMainFrame = (CMainFrame *)AfxGetMainWnd();
            ASSERT(pMainFrame->IsKindOf(RUNTIME_CLASS(CMainFrame)));
            MusicParm cParm = *pMainFrame->GetMusicParmDefaults();
            cParm.nManualCalcLower = m_nCalcLowerBound;
            cParm.nManualCalcUpper = m_nCalcUpperBound;
            pMainFrame->SetMusicParmDefaults(cParm);
        }
        m_bModified = FALSE;
        SetModified(FALSE);
        // refresh all graphs
        pView->RefreshGraphs(TRUE, TRUE);
    }
}

/////////////////////////////////////////////////////////////////////////////
// CDlgParametersIntensityPage property page

CDlgParametersIntensityPage::CDlgParametersIntensityPage() : CPropertyPage(CDlgParametersIntensityPage::IDD) {

    //{{AFX_DATA_INIT(CDlgParametersIntensityPage)
    m_nScaleMode = -1;
    //}}AFX_DATA_INIT
}

CDlgParametersIntensityPage::~CDlgParametersIntensityPage() {

}

void CDlgParametersIntensityPage::DoDataExchange(CDataExchange * pDX) {

    CPropertyPage::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CDlgParametersIntensityPage)
    DDX_Radio(pDX, IDC_DISPLAY_SCALEMODE, m_nScaleMode);
    //}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgParametersIntensityPage, CPropertyPage)
    //{{AFX_MSG_MAP(CDlgParametersIntensityPage)
    ON_CBN_SELCHANGE(IDC_DISPLAY_SCALEMODE, OnChange)
    ON_BN_CLICKED(IDC_DISPLAY_SCALEMODE, OnChange)
    ON_BN_CLICKED(IDC_RADIO2, OnChange)
    ON_BN_CLICKED(IDC_RADIO3, OnChange)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()


/***************************************************************************/
// CDlgParametersIntensityPage::OnInitDialog
/***************************************************************************/
BOOL CDlgParametersIntensityPage::OnInitDialog() {

    CPropertyPage::OnInitDialog();

    CMainFrame * pMDIFrameWnd = (CMainFrame *)AfxGetMainWnd();
    ASSERT(pMDIFrameWnd->IsKindOf(RUNTIME_CLASS(CMainFrame)));
    CSaView * pView = (CSaView *)pMDIFrameWnd->GetCurrSaView();
    // get pointer to document
    CSaDoc * pDoc = (CSaDoc *)pView->GetDocument();

    const IntensityParm & cIntensity = pDoc->GetIntensityParm();
    m_nScaleMode = cIntensity.nScaleMode;

    UpdateData(FALSE);

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

/***************************************************************************/
// CDlgParametersIntensityPage::OnChange page changed
/***************************************************************************/
void CDlgParametersIntensityPage::OnChange() {

    m_bModified = TRUE;
    SetModified(TRUE); // data modified, enable apply button
}

/***************************************************************************/
// CDlgParametersIntensityPage::Apply
/***************************************************************************/
void CDlgParametersIntensityPage::Apply() {

    if (m_bModified && m_hWnd) {
        UpdateData(TRUE);
        CMainFrame * pMDIFrameWnd = (CMainFrame *)AfxGetMainWnd();
        ASSERT(pMDIFrameWnd->IsKindOf(RUNTIME_CLASS(CMainFrame)));
        CSaView * pView = (CSaView *)pMDIFrameWnd->GetCurrSaView();
        // get pointer to document
        CSaDoc * pDoc = (CSaDoc *)pView->GetDocument();
        IntensityParm cParm = pDoc->GetIntensityParm();

        cParm.nScaleMode = m_nScaleMode;

        pDoc->SetIntensityParm(cParm);
        pMDIFrameWnd->SetIntensityParmDefaults(cParm);

        m_bModified = FALSE;
        SetModified(FALSE);
        // refresh all graphs
        pView->RefreshGraphs(TRUE, TRUE);
    }
}


/////////////////////////////////////////////////////////////////////////////
#include "float.h"
// CDlgParametersIntensityPage message handlers

/////////////////////////////////////////////////////////////////////////////
// CDlgParametersResearchPage property page

IMPLEMENT_DYNCREATE(CDlgParametersResearchPage, CPropertyPage)

// Global Research Settings
CResearchSettings ResearchSettings;

int researchSmoothSettings[] = { -1, 40, 50, 60, 70, 85, 100, 120, 140, 170, 200, 240, 280, 340, 400, 480, 560, 680, 800, 960 };

void CResearchSettings::Init() {

    m_bSpectrogramConnectFormants = FALSE;
    m_bSpectrogramContrastEnhance = FALSE;
    m_bShowHilbertTransform = FALSE;
    m_bShowInstantaneousPower = FALSE;
    m_nSpectrumLpcMethod = LPC_COVAR_LATTICE;
    m_nSpectrumLpcOrderFsMult = 1;
    m_nSpectrumLpcOrderExtra = 1;
    m_nSpectrumLpcOrderAuxMax = 4;
    m_nLpcCepstralSmooth = -1;
    m_nLpcCepstralSharp = 0;
}

CDlgParametersResearchPage::CDlgParametersResearchPage() : CPropertyPage(CDlgParametersResearchPage::IDD) {

    m_workingSettings = ResearchSettings;
    //{{AFX_DATA_INIT(CDlgParametersResearchPage)
    //}}AFX_DATA_INIT
}


CDlgParametersResearchPage::~CDlgParametersResearchPage() {

}

BOOL CDlgParametersResearchPage::OnInitDialog() {

    CPropertyPage::OnInitDialog();


    m_cSmooth.AddString(_T("Def."));
    int nSelect = 0;
    for (int i = 1; i < sizeof(researchSmoothSettings)/sizeof(int); i++) {
        CString szString;
        szString.Format(_T("%d"), researchSmoothSettings[i]);
        m_cSmooth.AddString(szString);

        if (researchSmoothSettings[i] == ResearchSettings.m_nLpcCepstralSmooth) {
            nSelect = i;
        }
    }
    m_cSmooth.SetCurSel(nSelect);
    m_cWindowType.SetCurSel(ResearchSettings.m_cWindow.m_nType);
    m_cWindowReplication.SetCurSel(ResearchSettings.m_cWindow.m_nReplication);

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgParametersResearchPage::Apply() {

    if (m_bModified && m_hWnd) {
        UpdateData(TRUE); // Get Data from dialog

        CMainFrame * pMDIFrameWnd = (CMainFrame *)AfxGetMainWnd();
        ASSERT(pMDIFrameWnd->IsKindOf(RUNTIME_CLASS(CMainFrame)));
        CSaView * pView = (CSaView *)pMDIFrameWnd->GetCurrSaView();
        CSaDoc * pDoc = (CSaDoc *)pView->GetDocument();

        int nSmoothIndex = m_cSmooth.GetCurSel();
        m_workingSettings.m_nLpcCepstralSmooth = researchSmoothSettings[nSmoothIndex];
        m_workingSettings.m_cWindow.m_nType = m_cWindowType.GetCurSel();
        m_workingSettings.m_cWindow.m_nReplication = m_cWindowReplication.GetCurSel();

        BOOL bSpectrumSettingsChanged =
            ResearchSettings.m_nSpectrumLpcMethod != m_workingSettings.m_nSpectrumLpcMethod ||
            ResearchSettings.m_nSpectrumLpcOrderFsMult != m_workingSettings.m_nSpectrumLpcOrderFsMult ||
            ResearchSettings.m_nSpectrumLpcOrderExtra != m_workingSettings.m_nSpectrumLpcOrderExtra ||
            ResearchSettings.m_nSpectrumLpcOrderAuxMax != m_workingSettings.m_nSpectrumLpcOrderAuxMax ||
            ResearchSettings.m_cWindow != m_workingSettings.m_cWindow ||
            ((m_workingSettings.m_nSpectrumLpcMethod == LPC_CEPSTRAL) &&
             (ResearchSettings.m_nLpcCepstralSharp != m_workingSettings.m_nLpcCepstralSharp ||
              ResearchSettings.m_nLpcCepstralSmooth != m_workingSettings.m_nLpcCepstralSmooth));

        if (bSpectrumSettingsChanged) {
            pDoc->GetSpectrogram(TRUE)->GetFormantProcess()->SetDataInvalid();
            CProcessSpectrum * pSpectrum = pDoc->GetSpectrum();
            // invalidate processed data
            pSpectrum->SetDataInvalid();
        }

        if (ResearchSettings.m_cWindow.m_nType != m_workingSettings.m_cWindow.m_nType) {
            // processed data is invalid
            pDoc->GetSpectrogram(TRUE)->SetDataInvalid();
        }

        ResearchSettings = m_workingSettings;
        pView->RefreshGraphs(TRUE, TRUE);
        SetModified(FALSE);
    }
}

void CDlgParametersResearchPage::DoDataExchange(CDataExchange * pDX) {

    int temp;

    CPropertyPage::DoDataExchange(pDX);
    DDX_Check(pDX, IDC_RESEARCH_HILBERT, m_workingSettings.m_bShowHilbertTransform);
    DDX_Check(pDX, IDC_RESEARCH_INSTANTANEOUS_POWER, m_workingSettings.m_bShowInstantaneousPower);
    DDX_Check(pDX, IDC_RESEARCH_SPECTROGRAM_CONNECT_FORMANTS, m_workingSettings.m_bSpectrogramConnectFormants);
    DDX_Check(pDX, IDC_RESEARCH_SPECTROGRAM_CONTRAST, m_workingSettings.m_bSpectrogramContrastEnhance);
    DDX_Radio(pDX, IDC_RESEARCH_SPECTRUM_LPC_METHOD, m_workingSettings.m_nSpectrumLpcMethod);
    DDX_Text(pDX, IDC_RESEARCH_LPC_ORDER_FS_MULT_EDIT, m_workingSettings.m_nSpectrumLpcOrderFsMult);
    DDX_Text(pDX, IDC_RESEARCH_LPC_ORDER_EXTRA_EDIT, m_workingSettings.m_nSpectrumLpcOrderExtra);
    DDX_Text(pDX, IDC_RESEARCH_LPC_ORDER_AUX_MAX_EDIT, m_workingSettings.m_nSpectrumLpcOrderAuxMax);
    DDX_CBIndex(pDX, IDC_RESEARCH_LPC_SHARP, m_workingSettings.m_nLpcCepstralSharp);

    temp = m_workingSettings.m_cWindow.m_bEquivalentLength;
    DDX_Check(pDX, IDC_RESEARCH_EXTENDED, temp);
    m_workingSettings.m_cWindow.m_bEquivalentLength = (temp == TRUE);

    temp = m_workingSettings.m_cWindow.m_bCenter;
    DDX_Check(pDX, IDC_RESEARCH_WINDOW_CENTER, temp);
    m_workingSettings.m_cWindow.m_bCenter = (temp == TRUE);

    DDX_Text(pDX, IDC_RESEARCH_WINDOW_FRAGMENT_EDIT, m_workingSettings.m_cWindow.m_nFragments);
    DDX_Text(pDX, IDC_RESEARCH_WINDOW_TIME_EDIT, m_workingSettings.m_cWindow.m_dTime);
    DDX_Text(pDX, IDC_RESEARCH_WINDOW_BANDWIDTH_EDIT, m_workingSettings.m_cWindow.m_dBandwidth);
    DDX_Radio(pDX, IDC_RESEARCH_WINDOW_BETWEEN_CURSORS, m_workingSettings.m_cWindow.m_nLengthMode);
    //{{AFX_DATA_MAP(CDlgParametersResearchPage)
    DDX_Control(pDX, IDC_RESEARCH_WINDOW_REPLICATION, m_cWindowReplication);
    DDX_Control(pDX, IDC_RESEARCH_WINDOW_TYPE, m_cWindowType);
    DDX_Control(pDX, IDC_RESEARCH_LPC_SMOOTH, m_cSmooth);
    //}}AFX_DATA_MAP
}

void CDlgParametersResearchPage::OnModified() {

    SetModified(TRUE);
    Apply();
}

BEGIN_MESSAGE_MAP(CDlgParametersResearchPage, CPropertyPage)
    //{{AFX_MSG_MAP(CDlgParametersResearchPage)
    ON_BN_CLICKED(IDC_RADIO3, OnModified)
    ON_BN_CLICKED(IDC_RESEARCH_WINDOW_BANDWIDTH, OnModified)
    ON_BN_CLICKED(IDC_RADIO4, OnModified)
    ON_BN_CLICKED(IDC_RESEARCH_SPECTROGRAM_CONNECT_FORMANTS, OnModified)
    ON_BN_CLICKED(IDC_RESEARCH_SPECTROGRAM_CONTRAST, OnModified)
    ON_BN_CLICKED(IDC_RESEARCH_SPECTRUM_LPC_METHOD, OnModified)
    ON_EN_KILLFOCUS(IDC_RESEARCH_LPC_ORDER_FS_MULT_EDIT, OnModified)
    ON_EN_KILLFOCUS(IDC_RESEARCH_LPC_ORDER_EXTRA_EDIT, OnModified)
    ON_EN_KILLFOCUS(IDC_RESEARCH_LPC_ORDER_AUX_MAX_EDIT, OnModified)
    ON_CBN_SELCHANGE(IDC_RESEARCH_LPC_SHARP, OnModified)
    ON_CBN_SELCHANGE(IDC_RESEARCH_LPC_SMOOTH, OnModified)
    ON_BN_CLICKED(IDC_RESEARCH_WINDOW_TIME, OnModified)
    ON_BN_CLICKED(IDC_RESEARCH_WINDOW_FRAGMENTS, OnModified)
    ON_BN_CLICKED(IDC_RESEARCH_WINDOW_BETWEEN_CURSORS, OnModified)
    ON_CBN_SELCHANGE(IDC_RESEARCH_WINDOW_TYPE, OnModified)
    ON_BN_CLICKED(IDC_RESEARCH_EXTENDED, OnModified)
    ON_EN_KILLFOCUS(IDC_RESEARCH_WINDOW_BANDWIDTH_EDIT, OnModified)
    ON_BN_CLICKED(IDC_RESEARCH_WINDOW_CENTER, OnModified)
    ON_EN_KILLFOCUS(IDC_RESEARCH_WINDOW_TIME_EDIT, OnModified)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgParametersResearchPage message handlers

/////////////////////////////////////////////////////////////////////////////
// CDlgParametersFormantTracker property page

IMPLEMENT_DYNCREATE(CDlgParametersFormantTracker, CPropertyPage)

CFormantTrackerOptions FormantTrackerOptions;

void CFormantTrackerOptions::Init() {

    m_nWindowType = DspWin::kBlackman;
    m_dUpdateRate = 100;
    m_dWindowBandwidth = 125;
    m_dDtfMinBandwidth = 125;
    m_dAzfMinBandwidth = 125;
    m_bAzfAddConjugateZeroes = TRUE;
    m_bAzfMostRecent = TRUE;
    m_bAzfDiscardLpcBandwidth = TRUE;
    m_bLowPass = FALSE;
    m_bShowOriginalFormantTracks = FALSE;
}

BOOL CFormantTrackerOptions::operator !=(const CFormantTrackerOptions & ref) const {

    return
        (m_nWindowType != ref.m_nWindowType) ||
        (m_dWindowBandwidth != ref.m_dWindowBandwidth) ||
        (m_dUpdateRate != ref.m_dUpdateRate) ||
        (m_dDtfMinBandwidth != ref.m_dDtfMinBandwidth) ||
        (m_dAzfMinBandwidth != ref.m_dAzfMinBandwidth) ||
        (m_bAzfAddConjugateZeroes != ref.m_bAzfAddConjugateZeroes) ||
        (m_bAzfMostRecent != ref.m_bAzfMostRecent) ||
        (m_bAzfDiscardLpcBandwidth != ref.m_bAzfDiscardLpcBandwidth) ||
        (m_bLowPass != ref.m_bLowPass) ||
        (m_bShowOriginalFormantTracks != ref.m_bShowOriginalFormantTracks);
}

CDlgParametersFormantTracker::CDlgParametersFormantTracker() :
    CPropertyPage(CDlgParametersFormantTracker::IDD) {
    m_workingSettings = FormantTrackerOptions;

    //{{AFX_DATA_INIT(CDlgParametersFormantTracker)
    //}}AFX_DATA_INIT
}

CDlgParametersFormantTracker::~CDlgParametersFormantTracker() {

}

void CDlgParametersFormantTracker::DoDataExchange(CDataExchange * pDX) {

    CPropertyPage::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_FTRACKER_AZF_BANDWIDTH, m_workingSettings.m_dAzfMinBandwidth);
    DDX_Text(pDX, IDC_FTRACKER_DTF_BANDWIDTH, m_workingSettings.m_dDtfMinBandwidth);
    DDX_Text(pDX, IDC_FTRACKER_WINDOW_BANDWIDTH, m_workingSettings.m_dWindowBandwidth);
    DDX_Text(pDX, IDC_FTRACKER_UPDATE_RATE, m_workingSettings.m_dUpdateRate);
    DDX_Check(pDX, IDC_FTRACKER_LOWPASS, m_workingSettings.m_bLowPass);
    DDX_Check(pDX, IDC_FTRACKER_AZF_MOST_RECENT, m_workingSettings.m_bAzfMostRecent);
    DDX_Check(pDX, IDC_FTRACKER_AZF_DISCARD_LPC_BANDWIDTH, m_workingSettings.m_bAzfDiscardLpcBandwidth);
    DDX_CBIndex(pDX, IDC_RESEARCH_WINDOW_TYPE, m_workingSettings.m_nWindowType);
    DDX_Check(pDX, IDC_FTRACKER_AZF_ADD_CONJUGATE_ZERO, m_workingSettings.m_bAzfAddConjugateZeroes);
    DDX_Check(pDX, IDC_FTRACKER_ORIGINAL, m_workingSettings.m_bShowOriginalFormantTracks);
    //{{AFX_DATA_MAP(CDlgParametersFormantTracker)
    //}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgParametersFormantTracker, CPropertyPage)
    //{{AFX_MSG_MAP(CDlgParametersFormantTracker)
    ON_BN_CLICKED(IDC_FTRACKER_AZF_ADD_CONJUGATE_ZERO, OnModified)
    ON_EN_KILLFOCUS(IDC_FTRACKER_AZF_BANDWIDTH, OnModified)
    ON_BN_CLICKED(IDC_FTRACKER_AZF_MOST_RECENT, OnModified)
    ON_BN_CLICKED(IDC_FTRACKER_AZF_DISCARD_LPC_BANDWIDTH, OnModified)
    ON_EN_KILLFOCUS(IDC_FTRACKER_DTF_BANDWIDTH, OnModified)
    ON_BN_CLICKED(IDC_FTRACKER_FIRST_F0, OnModified)
    ON_EN_KILLFOCUS(IDC_FTRACKER_WINDOW_BANDWIDTH, OnModified)
    ON_CBN_SELCHANGE(IDC_RESEARCH_WINDOW_TYPE, OnModified)
    ON_BN_CLICKED(IDC_FTRACKER_ORIGINAL, OnModified)
    ON_BN_CLICKED(IDC_FTRACKER_LOWPASS, OnModified)
    ON_EN_KILLFOCUS(IDC_FTRACKER_UPDATE_RATE, OnModified)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgParametersFormantTracker message handlers

void CDlgParametersFormantTracker::OnModified() {

    UpdateData(TRUE); // Get Data from dialog
    SetModified(FormantTrackerOptions != m_workingSettings);
}

void CDlgParametersFormantTracker::Apply() {

    if ((FormantTrackerOptions != m_workingSettings) && m_hWnd) {
        UpdateData(TRUE); // Get Data from dialog

        CMainFrame * pMDIFrameWnd = (CMainFrame *)AfxGetMainWnd();
        ASSERT(pMDIFrameWnd->IsKindOf(RUNTIME_CLASS(CMainFrame)));
        CSaView * pView = (CSaView *)pMDIFrameWnd->GetCurrSaView();
        CSaDoc * pDoc = (CSaDoc *)pView->GetDocument();


        CFormantTrackerOptions testSettings = m_workingSettings;

        testSettings.m_bShowOriginalFormantTracks = FormantTrackerOptions.m_bShowOriginalFormantTracks;
        if (testSettings != FormantTrackerOptions) {
            pDoc->GetFormantTracker()->SetDataInvalid();
        }

        FormantTrackerOptions = m_workingSettings;
        pView->RefreshGraphs(TRUE, TRUE);
    }
    SetModified(FALSE);
}

//###########################################################################
// CDlgGraphsParameters property sheet
// Displays all controls to change calculation parameters for the wave data
// calculations.

IMPLEMENT_DYNAMIC(CDlgGraphsParameters, CPropertySheet)

/////////////////////////////////////////////////////////////////////////////
// CDlgGraphsParameters message map

BEGIN_MESSAGE_MAP(CDlgGraphsParameters, CPropertySheet)
    //{{AFX_MSG_MAP(CDlgGraphsParameters)
    ON_WM_CREATE()
    ON_COMMAND(ID_APPLY_NOW, OnApply)
    ON_COMMAND(IDOK, OnOK)
    ON_COMMAND(IDHELP, OnHelpParameters)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgGraphsParameters construction/destruction/creation

/***************************************************************************/
// CDlgGraphsParameters::CDlgGraphsParameters Constructor
/***************************************************************************/
CDlgGraphsParameters::CDlgGraphsParameters(LPCTSTR pszCaption, CWnd * pParent)
    : CPropertySheet(pszCaption, pParent) {

    //{{AFX_DATA_INIT(CDlgGraphsParameters)
    // NOTE: the ClassWizard will add member initialization here
    //}}AFX_DATA_INIT
    // check if spectrogram graphs available

    m_pDlgSpectrogramPage = NULL;
    m_pDlgSnapshotPage = NULL;
    m_pDlgResearchPage = NULL;
    m_pDlgFTrackerPage = NULL;

    // get pointer to view
    CMainFrame * pMainWnd = (CMainFrame *)AfxGetMainWnd();
    CMainFrame * pMDIFrameWnd = (CMainFrame *)AfxGetMainWnd();
    ASSERT(pMDIFrameWnd->IsKindOf(RUNTIME_CLASS(CMainFrame)));
    CSaView * pView = (CSaView *)pMDIFrameWnd->GetCurrSaView();

    int nPage = 0;

    int nFocusedID = pView->GetFocusedGraphID();

    if (pView->GetGraphIndexForIDD(IDD_RAWDATA) >= 0 || pView->GetGraphIndexForIDD(IDD_MAGNITUDE) >= 0 || pView->GetGraphIndexForIDD(IDD_RECORDING) >= 0) {
        AddPage(&m_dlgRawdataPage);

        if (nFocusedID == IDD_RAWDATA || nFocusedID == IDD_MAGNITUDE || nFocusedID == IDD_RECORDING) {
            nPage = GetPageCount() - 1;
        }
    }

    if (pView->GetGraphIndexForIDD(IDD_CEPPITCH) >= 0 || pView->GetGraphIndexForIDD(IDD_GRAPITCH) >= 0
            || pView->GetGraphIndexForIDD(IDD_SMPITCH) >= 0 || pView->GetGraphIndexForIDD(IDD_PITCH) >= 0
            || pView->GetGraphIndexForIDD(IDD_CHPITCH) >= 0) {
        AddPage(&m_dlgPitchPage);

        if (nFocusedID == IDD_CEPPITCH || nFocusedID == IDD_GRAPITCH || nFocusedID == IDD_SMPITCH
                || nFocusedID == IDD_CHPITCH || nFocusedID == IDD_PITCH) {
            nPage = GetPageCount() - 1;
        }
    }

    if (pView->GetGraphIndexForIDD(IDD_SPECTROGRAM) >= 0) {
        m_pDlgSpectrogramPage = new CDlgParametersSpectroPage(IDS_TABTITLE_SPECTROGRAM,
                IDD_SPECTROGRAM, pMainWnd->GetSpectrogramParmDefaults());
        AddPage(m_pDlgSpectrogramPage);
        if (nFocusedID == IDD_SPECTROGRAM) {
            nPage = GetPageCount() - 1;
        }
    }


    if (pView->GetGraphIndexForIDD(IDD_SNAPSHOT) >= 0) {
        m_pDlgSnapshotPage = new CDlgParametersSpectroPage(IDS_TABTITLE_SNAPSHOT,
                IDD_SNAPSHOT, pMainWnd->GetSnapshotParmDefaults());
        AddPage(m_pDlgSnapshotPage);
        if (nFocusedID == IDD_SNAPSHOT) {
            nPage = GetPageCount() - 1;
        }
    }

    // check if spectrum graph available
    if (pView->GetGraphIndexForIDD(IDD_SPECTRUM) >= 0) {
        AddPage(&m_dlgSpectrumPage);
        if (nFocusedID == IDD_SPECTRUM) {
            nPage = GetPageCount() - 1;
        }
    }

    if (pView->GetGraphIndexForIDD(IDD_F1F2) >= 0
            || pView->GetGraphIndexForIDD(IDD_F2F1) >= 0
            || pView->GetGraphIndexForIDD(IDD_F2F1F1) >= 0
            || pView->GetGraphIndexForIDD(IDD_3D) >= 0) {
        AddPage(&m_dlgFormantsPage);
        m_dlgFormantsPage.m_bAccess = EXPERIMENTAL_ACCESS;

        if (nFocusedID == IDD_F1F2 || nFocusedID == IDD_F2F1 || nFocusedID == IDD_F2F1F1 || nFocusedID == IDD_3D) {
            nPage = GetPageCount() - 1;
        }
    }

    // check if SDP graphs available
    if (pView->GetGraphIndexForIDD(IDD_SDP_A) >= 0 || pView->GetGraphIndexForIDD(IDD_SDP_B) >= 0 || pView->GetGraphIndexForIDD(IDD_INVSDP) >= 0) {
        AddPage(&m_dlgSDPPage);

        if (nFocusedID == IDD_SDP_A || nFocusedID == IDD_SDP_B || nFocusedID == IDD_INVSDP) {
            nPage = GetPageCount() - 1;
        }
    }

    // check if melogram or TWC graph is available - 09/25/2000, DDO
    if (pView->GetGraphIndexForIDD(IDD_MELOGRAM) >= 0 || pView->GetGraphIndexForIDD(IDD_TWC) >= 0) {
        AddPage(&m_dlgMusicPage);

        if (nFocusedID == IDD_MELOGRAM || nFocusedID == IDD_TWC) {
            nPage = GetPageCount() - 1;
        }
    }

    // check if melogram or TWC graph is available - 09/25/2000, DDO
    if (pView->GetGraphIndexForIDD(IDD_LOUDNESS) >= 0) {
        AddPage(&m_dlgIntensityPage);

        if (nFocusedID == IDD_LOUDNESS) {
            nPage = GetPageCount() - 1;
        }
    }

    if (PROGRAMMER_ACCESS) {
        m_pDlgResearchPage = new CDlgParametersResearchPage;

        AddPage(m_pDlgResearchPage);
    }

    if (PROGRAMMER_ACCESS) {
        m_pDlgFTrackerPage = new CDlgParametersFormantTracker;

        AddPage(m_pDlgFTrackerPage);
    }

    if (!GetPageCount()) {
        AddPage(&m_dlgRawdataPage);    // add a page (must have at least one page
    }

    EnableStackedTabs(FALSE); // set for scrolling tabs style

    SetActivePage(nPage);
}

// SDM 1.5Test10.3
/***************************************************************************/
// CDlgGraphsParameters::~CDlgGraphsParameters Destructor
/***************************************************************************/
CDlgGraphsParameters::~CDlgGraphsParameters() {

    if (m_pDlgSpectrogramPage) {
        delete m_pDlgSpectrogramPage;
    }
    if (m_pDlgSnapshotPage) {
        delete m_pDlgSnapshotPage;
    }
    if (m_pDlgResearchPage) {
        delete m_pDlgResearchPage;
    }
    if (m_pDlgFTrackerPage) {
        delete m_pDlgFTrackerPage;
    }
}

/***************************************************************************/
// CDlgGraphsParameters::OnInitDialog Dialog initialization
/***************************************************************************/
BOOL CDlgGraphsParameters::OnInitDialog() {

    CPropertySheet::OnInitDialog();
    ChangeButtons();
    return TRUE;
}

/***************************************************************************/
// CDlgGraphsParameters::ChangeButtons Rearrange buttons on the dialog
/***************************************************************************/
void CDlgGraphsParameters::ChangeButtons() {

    CWnd * pWndOK = GetDlgItem(IDOK); // get pointers to the button objects
    CWnd * pWndCancel = GetDlgItem(IDCANCEL);
    CWnd * pWndApply = GetDlgItem(ID_APPLY_NOW);
    CRect rBtnOK, rBtnCancel, rBtnApply, rBtnHelp;

    // get button coordinates
    pWndOK->GetWindowRect(rBtnOK);
    ScreenToClient(rBtnOK);
    pWndCancel->GetWindowRect(rBtnCancel);
    ScreenToClient(rBtnCancel);
    pWndApply->GetWindowRect(rBtnApply);
    ScreenToClient(rBtnApply);

    // move buttons
    rBtnHelp = rBtnApply;
    int nOffset = rBtnOK.left - rBtnCancel.left;
    rBtnOK.OffsetRect(nOffset, 0);
    pWndOK->MoveWindow(rBtnOK);
    rBtnCancel.OffsetRect(nOffset, 0);
    pWndCancel->MoveWindow(rBtnCancel);
    rBtnApply.OffsetRect(nOffset, 0);
    pWndApply->MoveWindow(rBtnApply);

    // create Help button
    m_cHelp.Create(_T("&Help"), WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON, rBtnHelp, this, IDHELP);
    m_cHelp.SetFont(pWndOK->GetFont());
}

/////////////////////////////////////////////////////////////////////////////
// CDlgGraphsParameters helper functions

/***************************************************************************/
// CDlgGraphsParameters::DoDataExchange Data exchange
/***************************************************************************/
void CDlgGraphsParameters::DoDataExchange(CDataExchange * pDX) {

    CPropertySheet::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CDlgGraphsParameters)
    // NOTE: the ClassWizard will add DDX and DDV calls here
    //}}AFX_DATA_MAP
}

/////////////////////////////////////////////////////////////////////////////
// CDlgGraphsParameters message handlers

/***************************************************************************/
// CDlgGraphsParameters::OnCreate Dialog creation
/***************************************************************************/
int CDlgGraphsParameters::OnCreate(LPCREATESTRUCT lpCreateStruct) {

    if (CPropertySheet::OnCreate(lpCreateStruct) == -1) {
        return -1;
    }

    return 0;
}


/***************************************************************************/
// CDlgGraphsParameters::OnApply Apply button hit
/***************************************************************************/
void CDlgGraphsParameters::OnApply() {

    m_dlgPitchPage.Apply();
    m_dlgFormantsPage.Apply();
    // get pointer to view
    CMainFrame * pMainFrame = (CMainFrame *)AfxGetMainWnd();
    ASSERT(pMainFrame->IsKindOf(RUNTIME_CLASS(CMainFrame)));
    CSaView * pView = (CSaView *)pMainFrame->GetCurrSaView();

    //***********************************************************
    // If either spectrogram is visible then apply settings.
    // If there is a spectrum visible then apply its settings.
    //***********************************************************
    if (pView->GetGraphIndexForIDD(IDD_SPECTROGRAM) >= 0) {
        m_pDlgSpectrogramPage->Apply();
    }
    if (pView->GetGraphIndexForIDD(IDD_SNAPSHOT) >= 0) {
        m_pDlgSnapshotPage->Apply();
    }
    if (pView->GetGraphIndexForIDD(IDD_SPECTRUM)      >= 0) {
        m_dlgSpectrumPage.Apply();
    }

    //***********************************************************
    // If any of the SDP graphs are visible apply their settings.
    //***********************************************************
    if (pView->GetGraphIndexForIDD(IDD_SDP_A)  >= 0 ||
            pView->GetGraphIndexForIDD(IDD_SDP_B)  >= 0 ||
            pView->GetGraphIndexForIDD(IDD_INVSDP) >= 0) {
        m_dlgSDPPage.Apply();
    }

    //***********************************************************
    // If music graphs are visible then apply their settings.
    //***********************************************************
    if (pView->GetGraphIndexForIDD(IDD_MELOGRAM) >= 0 ||
            pView->GetGraphIndexForIDD(IDD_TWC)      >= 0) {
        m_dlgMusicPage.Apply();
    }

    //***********************************************************
    // If intensity graph is visible then apply settings.
    //***********************************************************
    if (pView->GetGraphIndexForIDD(IDD_LOUDNESS) >= 0) {
        m_dlgIntensityPage.Apply();
    }

    if (m_pDlgResearchPage) {
        m_pDlgResearchPage->Apply();
    }

    if (m_pDlgFTrackerPage) {
        m_pDlgFTrackerPage->Apply();
    }

    pMainFrame->OnSetDefaultParameters();
    SendMessage(PSM_CANCELTOCLOSE, 0, 0L);
}

/***************************************************************************/
// CDlgGraphsParameters::OnOK OK button hit
/***************************************************************************/
void CDlgGraphsParameters::OnOK() {

    SetWindowPos(NULL, 0,0,0,0,SWP_NOMOVE | SWP_NOZORDER);

    MSG msg;
    while (::PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) {
        AfxGetApp()->PumpMessage();
    }

    OnApply();
    EndDialog(IDOK);
}

/***************************************************************************/
// CDlgGraphsParameters::OnHelpParameters Call Parameters help
/***************************************************************************/
void CDlgGraphsParameters::OnHelpParameters() {

    // create the pathname
    CString szPath = AfxGetApp()->m_pszHelpFilePath;
    szPath += "::/User_Interface/Menus/Graphs/Graph_Parameters/";

    // get the name of the active page
    CString szPageName;
    GetActivePage()->GetWindowText(szPageName);

    // find the correct topic for the active page
    if (szPageName == "Waveform") {
        szPath += "Waveform_tab_Graph_Parameters.htm";
    } else if (szPageName == "Pitch") {
        szPath += "Pitch_tab_Graph_Parameters.htm";
    } else if ((szPageName == "Spectrogram") || (szPageName == "Snapshot")) {
        szPath += "Spectrogram_tab_Graph_Parameters.htm";
    } else if (szPageName == "Spectrum") {
        szPath += "Spectrum_tab_Graph_Parameters.htm";
    } else if (szPageName == "Formants") {
        szPath += "Formants_tab_Graph_Parameters.htm";
    } else if (szPageName == "Music") {
        szPath += "Music_tab_Graph_Parameters.htm";
    } else if (szPageName == "Intensity") {
        szPath += "Intensity_tab_Graph_Parameters.htm";
    } else {
        szPath += "Graph_Parameters_overview.htm";
    }

    ::HtmlHelp(NULL, szPath, HH_DISPLAY_TOPIC, NULL);
}
