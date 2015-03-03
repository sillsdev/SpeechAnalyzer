/////////////////////////////////////////////////////////////////////////////
// waveformGenerator.cpp:
// Implementation of the CDlgWaveformGenerator
//                       CDlgWaveformGeneratorSettings
//
// Author: Steve MacLean
// copyright 2000-2001 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DlgWaveformGenerator.h"
#include "Process\Process.h"
#include "math.h"
#include "mainfrm.h"
#include "sa_doc.h"
#include "sa.h"
#include "sa_view.h"
#include "objectostream.h"
#include "riff.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

//###########################################################################
// CDlgWaveformGenerator dialog
// Lets the user configure echo/reverb filters.

/////////////////////////////////////////////////////////////////////////////
// CDlgWaveformGenerator message map

BEGIN_MESSAGE_MAP(CDlgWaveformGenerator, CDialog)
    ON_BN_CLICKED(IDC_GENERATOR_BANDWITH, OnGeneratorBandwith)
    ON_BN_CLICKED(IDC_GENERATOR_NYQUIST, OnGeneratorBandwith)
    ON_BN_CLICKED(IDC_GENERATOR_N_HARMONICS, OnGeneratorBandwith)
    ON_COMMAND(IDHELP, OnHelpWaveformGenerator)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgWaveformGenerator construction/destruction/creation

/***************************************************************************/
// CDlgWaveformGenerator::CDlgWaveformGenerator Constructor
/***************************************************************************/
CDlgWaveformGenerator::CDlgWaveformGenerator(CWnd * pParent) :
    CDialog(CDlgWaveformGenerator::IDD, pParent) {

    m_szSamplingRate = _T("");
    m_szBits = _T("");
}

/////////////////////////////////////////////////////////////////////////////
// CDlgWaveformGenerator helper functions

/***************************************************************************/
// CDlgWaveformGenerator::DoDataExchange Data exchange
/***************************************************************************/
void CDlgWaveformGenerator::DoDataExchange(CDataExchange * pDX) {

    CDialog::DoDataExchange(pDX);

    DDX_CBString(pDX, IDC_GENERATOR_SAMPLINGRATE, m_szSamplingRate);
    DDX_CBString(pDX, IDC_GENERATOR_BITS, m_szBits);

    DDX_Text(pDX, IDC_GENERATOR_AMPLITUDE1, working.m_dAmplitude[0]);
    DDV_MinMaxDouble(pDX, working.m_dAmplitude[0], -100., 100.);
    DDX_Text(pDX, IDC_GENERATOR_FREQUENCY1, working.m_dFrequency[0]);
    DDV_MinMaxDouble(pDX, working.m_dFrequency[0], 0., 100000.);
    DDX_Text(pDX, IDC_GENERATOR_PHASE1, working.m_dPhase[0]);
    DDV_MinMaxDouble(pDX, working.m_dPhase[0], -180., 360.);

    DDX_Text(pDX, IDC_GENERATOR_AMPLITUDE2, working.m_dAmplitude[1]);
    DDV_MinMaxDouble(pDX, working.m_dAmplitude[1], -100., 100.);
    DDX_Text(pDX, IDC_GENERATOR_FREQUENCY2, working.m_dFrequency[1]);
    DDV_MinMaxDouble(pDX, working.m_dFrequency[1], 0., 100000.);
    DDX_Text(pDX, IDC_GENERATOR_PHASE2, working.m_dPhase[1]);
    DDV_MinMaxDouble(pDX, working.m_dPhase[1], -180., 360.);

    DDX_Text(pDX, IDC_GENERATOR_AMPLITUDE3, working.m_dAmplitude[2]);
    DDV_MinMaxDouble(pDX, working.m_dAmplitude[2], -100., 100.);
    DDX_Text(pDX, IDC_GENERATOR_FREQUENCY3, working.m_dFrequency[2]);
    DDV_MinMaxDouble(pDX, working.m_dFrequency[2], 0., 100000.);
    DDX_Text(pDX, IDC_GENERATOR_PHASE3, working.m_dPhase[2]);
    DDV_MinMaxDouble(pDX, working.m_dPhase[2], -180., 360.);

    DDX_Text(pDX, IDC_GENERATOR_AMPLITUDE4, working.m_dAmplitude[3]);
    DDV_MinMaxDouble(pDX, working.m_dAmplitude[3], -100., 100.);
    DDX_Text(pDX, IDC_GENERATOR_FREQUENCY4, working.m_dFrequency[3]);
    DDV_MinMaxDouble(pDX, working.m_dFrequency[3], 0., 100000.);
    DDX_Text(pDX, IDC_GENERATOR_PHASE4, working.m_dPhase[3]);
    DDV_MinMaxDouble(pDX, working.m_dPhase[3], -180., 360.);

    DDX_Text(pDX, IDC_GENERATOR_AMPLITUDE5, working.m_dAmplitude[4]);
    DDV_MinMaxDouble(pDX, working.m_dAmplitude[4], -100., 100.);
    DDX_Text(pDX, IDC_GENERATOR_FREQUENCY5, working.m_dFrequency[4]);
    DDV_MinMaxDouble(pDX, working.m_dFrequency[4], 0., 100000.);
    DDX_Text(pDX, IDC_GENERATOR_PHASE5, working.m_dPhase[4]);
    DDV_MinMaxDouble(pDX, working.m_dPhase[4], -180., 360.);

    DDX_Text(pDX, IDC_GENERATOR_AMPLITUDE6, working.m_dAmplitude[5]);
    DDV_MinMaxDouble(pDX, working.m_dAmplitude[5], -100., 100.);
    DDX_Text(pDX, IDC_GENERATOR_FREQUENCY6, working.m_dFrequency[5]);
    DDV_MinMaxDouble(pDX, working.m_dFrequency[5], 0., 100000.);
    DDX_Text(pDX, IDC_GENERATOR_PHASE6, working.m_dPhase[5]);
    DDV_MinMaxDouble(pDX, working.m_dPhase[5], -180., 360.);

    DDX_Text(pDX, IDC_GENERATOR_AMPLITUDE7, working.m_dAmplitude[6]);
    DDV_MinMaxDouble(pDX, working.m_dAmplitude[6], -100., 100.);
    DDX_Text(pDX, IDC_GENERATOR_FREQUENCY7, working.m_dFrequency[6]);
    //DDV_MinMaxDouble(pDX, m_dFrequency[6], 0., 100000.);
    DDX_Text(pDX, IDC_GENERATOR_PHASE7, working.m_dPhase[6]);
    DDV_MinMaxDouble(pDX, working.m_dPhase[6], -180., 360.);

    DDX_Check(pDX, IDC_GENERATOR_SINUSOID1, working.m_bSinusoid1);
    DDX_Check(pDX, IDC_GENERATOR_SINUSOID2, working.m_bSinusoid2);
    DDX_Check(pDX, IDC_GENERATOR_SINUSOID3, working.m_bSinusoid3);
    DDX_Check(pDX, IDC_GENERATOR_COMB, working.m_bComb);
    DDX_Check(pDX, IDC_GENERATOR_SQUAREWAVE, working.m_bSquareWave);
    DDX_Check(pDX, IDC_GENERATOR_TRIANGLE, working.m_bTriangle);
    DDX_Check(pDX, IDC_GENERATOR_SAWTOOTH, working.m_bSawtooth);
    DDX_Radio(pDX, IDC_GENERATOR_BANDWITH, working.m_nHandleDiscontinuities);
    DDX_Text(pDX, IDC_GENERATOR_NUMBER_OF_HARMONICS, working.m_nHarmonics);
    DDV_MinMaxDWord(pDX, working.m_nHarmonics, 1, 1000000000);
    DDX_Text(pDX, IDC_GENERATOR_LENGTH, working.m_fFileLength);
    DDV_MinMaxDouble(pDX, working.m_fFileLength, 0.001, 3600.);
}

BOOL CDlgWaveformGenerator::OnInitDialog() {

    CDialog::OnInitDialog();

    CComboBox * pWnd;
    CString szSelection;

    pWnd = (CComboBox *) GetDlgItem(IDC_GENERATOR_SAMPLINGRATE);
    szSelection.Format(_T("%d Hz"), working.pcm.wf.nSamplesPerSec);
    pWnd->SetCurSel(pWnd->FindString(0, szSelection));

    pWnd = (CComboBox *) GetDlgItem(IDC_GENERATOR_BITS);
    szSelection.Format(_T("%d"), working.pcm.wBitsPerSample);
    pWnd->SetCurSel(pWnd->FindString(0, szSelection));

    OnGeneratorBandwith();

    return TRUE;
}

void CDlgWaveformGenerator::OnGeneratorBandwith() {

    UpdateData(TRUE);

    BOOL bEnable = working.m_nHandleDiscontinuities == 2;

    CWnd * pWnd;

    pWnd = GetDlgItem(IDC_GENERATOR_NUMBER_OF_HARMONICS);

    if (pWnd) {
        pWnd->EnableWindow(bEnable);
    }
}

/***************************************************************************/
// CDlgWaveformGenerator::OnHelpWaveformGenerator
// Call Waveform Generator help
/***************************************************************************/
void CDlgWaveformGenerator::OnHelpWaveformGenerator() {

    // create the pathname
    CString szPath = AfxGetApp()->m_pszHelpFilePath;
    szPath += "::/User_Interface/Menus/Tools/Waveform_Generator.htm";
    ::HtmlHelp(NULL, szPath, HH_DISPLAY_TOPIC, NULL);
}
