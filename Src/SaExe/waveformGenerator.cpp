/////////////////////////////////////////////////////////////////////////////
// waveformGenerator.cpp:
// Implementation of the CDlgWaveformGenerator
//                       CDlgWaveformGeneratorSettings
//
// Author: Steve MacLean
// copyright 2000-2001 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Process\sa_proc.h"
#include "waveformGenerator.h"

#include "math.h"
#include "mainfrm.h"
#include "sa_doc.h"
#include "sa.h"
#include "sa_view.h"
#include "settings\obstream.h"
#include "riff.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

CDlgWaveformGenerator dlgWaveformGenerator;

class CDlgWaveformGenerator::settings::process : public CDataProcess
{
public:
  process() 
  {
  };
  virtual ~process() 
  {
  };
  
  long Process(settings &parms, void* pCaller = NULL, int nProgress = 0, int nLevel = 1);
};

/***************************************************************************/
// CDlgWaveformGenerator::process::Process Generate wav file
/***************************************************************************/
long CDlgWaveformGenerator::settings::process::Process(settings &parms, void* pCaller, int nProgress, int nLevel)
{
  TRACE(_T("Process: CProcessWbGenerator\n"));
  if (IsCanceled()) return MAKELONG(PROCESS_CANCELED, nProgress); // process canceled
  // check if nested workbench processes
  if (IsDataReady()) return MAKELONG(--nLevel, nProgress); // data is already ready
  else
  {
    SetDataInvalid();
  }

  if (nLevel < 0) // previous processing error
  {
    if ((nLevel == PROCESS_CANCELED)) 
      CancelProcess(); // set your own cancel flag
    return MAKELONG(nLevel, nProgress);
  }

  // start process
  BeginWaitCursor(); // wait cursor
  if(!StartProcess(pCaller, IDS_STATTXT_PROCESSWBGENERATOR)) // memory allocation failed or previous processing error
  {
    EndProcess(); // end data processing
    EndWaitCursor();
    return MAKELONG(PROCESS_ERROR, nProgress);
  }
  // create the temporary file
  if (!CreateTempFile(_T("GEN"))) // creating error
  {
    EndProcess(); // end data processing
    EndWaitCursor();
    SetDataInvalid();
    return MAKELONG(PROCESS_ERROR, nProgress);
  }
  DWORD dwBufferSize = GetBufferSize();
  // get source data size
  DWORD dwDataSize = DWORD(parms.m_fFileLength*parms.pcm.wf.nSamplesPerSec*parms.pcm.wf.nBlockAlign);
  DWORD dwDataPos = 0; // data position pointer
  // alocate the temporary global buffer for the fifo
  // start processing loop
  while (dwDataPos < dwDataSize)
  {
    DWORD dwBlockEnd = dwDataPos + dwBufferSize;
    if (dwBlockEnd > dwDataSize)
    {
      dwBlockEnd = dwDataSize;
      dwBufferSize = dwDataSize - dwDataPos;
    }

    // Synthesize
    parms.SynthesizeSamples(m_lpData, dwDataPos, dwBufferSize);

    dwDataPos += dwBufferSize;
    
    // result block is ready, store it
    if (dwDataPos >= dwBlockEnd)
    {
      // write the processed data block
      try
      {
        Write((HPSTR)m_lpData, dwBufferSize);
      }
      catch (CFileException e)
      {
        // error writing file
        ErrorMessage(IDS_ERROR_WRITETEMPFILE, GetProcessFileName());
        return Exit(PROCESS_ERROR); // error, writing failed
      }
    }
    // set progress bar
    SetProgress(nProgress + (int)(100 * dwDataPos / dwDataSize / (DWORD)nLevel));
    if (IsCanceled())
    {
      return Exit(PROCESS_CANCELED); // process canceled
    }
  }
  nProgress = nProgress + (int)(100 / nLevel); // calculate the actual progress
  // close the temporary file and read the status
  CloseTempFile(); // close the file
  EndProcess((nProgress >= 95)); // end data processing
  EndWaitCursor();
  SetDataReady();
  return MAKELONG(nLevel, nProgress);
}

/////////////////////////////////////////////////////////////////////////////
// CDlgWaveformGeneratorSettings helper functions

CDlgWaveformGenerator::settings::settings()
{
  m_dAmplitude[0] = 100;
  m_dFrequency[0] = 220;
  m_dPhase[0] = 0;
  m_dAmplitude[1] = 100;
  m_dFrequency[1] = 440;
  m_dPhase[1] = 0;
  m_dAmplitude[2] = 100;
  m_dFrequency[2] = 660;
  m_dPhase[2] = 0;
  m_dAmplitude[3] = 100;
  m_dFrequency[3] = 220;
  m_dPhase[3] = 0;
  m_dAmplitude[4] = 100;
  m_dFrequency[4] = 220;
  m_dPhase[4] = 0;
  m_dAmplitude[5] = 100;
  m_dFrequency[5] = 220;
  m_dPhase[5] = 0;
  m_dAmplitude[6] = 100;
  m_dFrequency[6] = 220;
  m_dPhase[6] = 0;
  m_bSinusoid1 = TRUE;
  m_bSinusoid2 = FALSE;
  m_bSinusoid3 = FALSE;
  m_bComb = FALSE;
  m_bSquareWave = FALSE;
  m_bTriangle = FALSE;
  m_bSawtooth = FALSE;
  m_nHandleDiscontinuities = 0;
  m_nHarmonics = 10;

  m_fFileLength = 2;
  pcm.wBitsPerSample = 16;
  pcm.wf.nChannels = 1;
  pcm.wf.nBlockAlign = 2;
  pcm.wf.nSamplesPerSec = 22050;  // Already set
  pcm.wf.wFormatTag = 1;  // PCM

  pcm.wf.nAvgBytesPerSec = pcm.wf.nSamplesPerSec*pcm.wf.nBlockAlign;
  pcm.wf.nBlockAlign = (unsigned short)(pcm.wf.nChannels*(pcm.wBitsPerSample/8));
}

/***************************************************************************/
// CDlgWaveformGenerator::settings::DoDataExchange Data exchange
/***************************************************************************/
void CDlgWaveformGenerator::settings::DoDataExchange(CDataExchange* pDX)
{
  DDX_Text(pDX, IDC_GENERATOR_AMPLITUDE1, m_dAmplitude[0]);
  DDV_MinMaxDouble(pDX, m_dAmplitude[0], -100., 100.);
  DDX_Text(pDX, IDC_GENERATOR_FREQUENCY1, m_dFrequency[0]);
  DDV_MinMaxDouble(pDX, m_dFrequency[0], 0., 100000.);
  DDX_Text(pDX, IDC_GENERATOR_PHASE1, m_dPhase[0]);
  DDV_MinMaxDouble(pDX, m_dPhase[0], -180., 360.);

  DDX_Text(pDX, IDC_GENERATOR_AMPLITUDE2, m_dAmplitude[1]);
  DDV_MinMaxDouble(pDX, m_dAmplitude[1], -100., 100.);
  DDX_Text(pDX, IDC_GENERATOR_FREQUENCY2, m_dFrequency[1]);
  DDV_MinMaxDouble(pDX, m_dFrequency[1], 0., 100000.);
  DDX_Text(pDX, IDC_GENERATOR_PHASE2, m_dPhase[1]);
  DDV_MinMaxDouble(pDX, m_dPhase[1], -180., 360.);

  DDX_Text(pDX, IDC_GENERATOR_AMPLITUDE3, m_dAmplitude[2]);
  DDV_MinMaxDouble(pDX, m_dAmplitude[2], -100., 100.);
  DDX_Text(pDX, IDC_GENERATOR_FREQUENCY3, m_dFrequency[2]);
  DDV_MinMaxDouble(pDX, m_dFrequency[2], 0., 100000.);
  DDX_Text(pDX, IDC_GENERATOR_PHASE3, m_dPhase[2]);
  DDV_MinMaxDouble(pDX, m_dPhase[2], -180., 360.);

  DDX_Text(pDX, IDC_GENERATOR_AMPLITUDE4, m_dAmplitude[3]);
  DDV_MinMaxDouble(pDX, m_dAmplitude[3], -100., 100.);
  DDX_Text(pDX, IDC_GENERATOR_FREQUENCY4, m_dFrequency[3]);
  DDV_MinMaxDouble(pDX, m_dFrequency[3], 0., 100000.);
  DDX_Text(pDX, IDC_GENERATOR_PHASE4, m_dPhase[3]);
  DDV_MinMaxDouble(pDX, m_dPhase[3], -180., 360.);

  DDX_Text(pDX, IDC_GENERATOR_AMPLITUDE5, m_dAmplitude[4]);
  DDV_MinMaxDouble(pDX, m_dAmplitude[4], -100., 100.);
  DDX_Text(pDX, IDC_GENERATOR_FREQUENCY5, m_dFrequency[4]);
  DDV_MinMaxDouble(pDX, m_dFrequency[4], 0., 100000.);
  DDX_Text(pDX, IDC_GENERATOR_PHASE5, m_dPhase[4]);
  DDV_MinMaxDouble(pDX, m_dPhase[4], -180., 360.);

  DDX_Text(pDX, IDC_GENERATOR_AMPLITUDE6, m_dAmplitude[5]);
  DDV_MinMaxDouble(pDX, m_dAmplitude[5], -100., 100.);
  DDX_Text(pDX, IDC_GENERATOR_FREQUENCY6, m_dFrequency[5]);
  DDV_MinMaxDouble(pDX, m_dFrequency[5], 0., 100000.);
  DDX_Text(pDX, IDC_GENERATOR_PHASE6, m_dPhase[5]);
  DDV_MinMaxDouble(pDX, m_dPhase[5], -180., 360.);

  DDX_Text(pDX, IDC_GENERATOR_AMPLITUDE7, m_dAmplitude[6]);
  DDV_MinMaxDouble(pDX, m_dAmplitude[6], -100., 100.);
  DDX_Text(pDX, IDC_GENERATOR_FREQUENCY7, m_dFrequency[6]);
  //DDV_MinMaxDouble(pDX, m_dFrequency[6], 0., 100000.);
  DDX_Text(pDX, IDC_GENERATOR_PHASE7, m_dPhase[6]);
  DDV_MinMaxDouble(pDX, m_dPhase[6], -180., 360.);

  DDX_Check(pDX, IDC_GENERATOR_SINUSOID1, m_bSinusoid1);
  DDX_Check(pDX, IDC_GENERATOR_SINUSOID2, m_bSinusoid2);
  DDX_Check(pDX, IDC_GENERATOR_SINUSOID3, m_bSinusoid3);
  DDX_Check(pDX, IDC_GENERATOR_COMB, m_bComb);
  DDX_Check(pDX, IDC_GENERATOR_SQUAREWAVE, m_bSquareWave);
  DDX_Check(pDX, IDC_GENERATOR_TRIANGLE, m_bTriangle);
  DDX_Check(pDX, IDC_GENERATOR_SAWTOOTH, m_bSawtooth);
  DDX_Radio(pDX, IDC_GENERATOR_BANDWITH, m_nHandleDiscontinuities);
  DDX_Text(pDX, IDC_GENERATOR_NUMBER_OF_HARMONICS, m_nHarmonics);
  DDV_MinMaxDWord(pDX, m_nHarmonics, 1, 1000000000);
  DDX_Text(pDX, IDC_GENERATOR_LENGTH, m_fFileLength);
  DDV_MinMaxDouble(pDX, m_fFileLength, 0.001, 3600.);
}

/***************************************************************************/
// CDlgWaveformGenerator::process::Process Generate wav file
/***************************************************************************/
void CDlgWaveformGenerator::settings::SynthesizeSamples(HPSTR pTargetData, DWORD dwDataPos, DWORD dwBufferSize)
{
  WORD wSmpSize = WORD(pcm.wf.nBlockAlign / pcm.wf.nChannels);

  DWORD dwBlockEnd = dwDataPos + dwBufferSize;

  int nData;
  BYTE bData;
  DWORD nSample = dwDataPos/wSmpSize;
  const double pi = 3.14159265358979323846264338327950288419716939937511;
  const double radiansPerCycle = 2*pi;
  double samplesPerCycle[7];
  double radiansPerSample[7];
  double phaseRadians[7];
  DWORD nHarmonics[7];
  
  for(int i = 0; i < 7; i++)
  {
    samplesPerCycle[i] = pcm.wf.nSamplesPerSec/m_dFrequency[i];
    radiansPerSample[i] = radiansPerCycle/samplesPerCycle[i];
    phaseRadians[i] = m_dPhase[i]/360*radiansPerCycle;
    nHarmonics[i] = (m_nHandleDiscontinuities == 2) ? m_nHarmonics : (DWORD) (pcm.wf.nSamplesPerSec/m_dFrequency[i]/2.);
  }
  
  while (dwDataPos < dwBlockEnd)
  {
    double intermediate = 0;
    
    if(m_bSinusoid1)
      intermediate += m_dAmplitude[0]*sin(nSample*radiansPerSample[0]+phaseRadians[0]);
    if(m_bSinusoid2)
      intermediate += m_dAmplitude[1]*sin(nSample*radiansPerSample[1]+phaseRadians[1]);
    if(m_bSinusoid3)
      intermediate += m_dAmplitude[2]*sin(nSample*radiansPerSample[2]+phaseRadians[2]);
    
    if(m_bComb)
    {
      double dCycles = nSample/samplesPerCycle[3] + m_dPhase[3]/360;
      
      if(fmod(dCycles, 1.0)*samplesPerCycle[3] < 1.0)
        intermediate += m_dAmplitude[3];
    }
    
    if(m_nHandleDiscontinuities == 0) // ideal
    {
      if(m_bSquareWave)  // Positive for 180 degrees then negative for 180 degrees
      {
        double dCycles = nSample/samplesPerCycle[4] + m_dPhase[4]/360;
        intermediate += m_dAmplitude[4]*(fmod(dCycles , 1.0) >= 0.5 ? -1. : 1.);
      }
      if(m_bTriangle) // rising from -90 to 90 then falling from 90 to 270
      {
        double dCycles = nSample/samplesPerCycle[5] + m_dPhase[5]/360;
        intermediate += m_dAmplitude[5]*(1 - 4.* fabs(fmod((dCycles + 0.25) , 1.0)-0.5));
      }
      if(m_bSawtooth)  // falling linearly from -180 to 180 then jump discontinuity and cycle repeats
      {
        double dCycles = nSample/samplesPerCycle[6] + m_dPhase[6]/360;
        intermediate += m_dAmplitude[6]*(1 - 2.*fmod((dCycles + 0.5) , 1.0));
      }
    }
    else
    {
      if(m_bSquareWave)  // Positive for 180 degrees then negative for 180 degrees
      {
        double square = 0;
        
        for(DWORD nHarmonic = 1; nHarmonic <= nHarmonics[4]; nHarmonic +=2)
        {
          square += sin((nSample*radiansPerSample[4]+phaseRadians[4])*nHarmonic)/ nHarmonic;
        }
        
        intermediate += m_dAmplitude[4]*4/pi*square;
      }
      if(m_bTriangle) // rising from -90 to 90 then falling from 90 to 270
      {
        double triangle = 0;
        double sign = 1;
        
        for(DWORD nHarmonic = 1; nHarmonic <= nHarmonics[5]; nHarmonic +=2)
        {
          triangle += sign*sin((nSample*radiansPerSample[5]+phaseRadians[5])*nHarmonic)/ (nHarmonic*nHarmonic);
          sign = -sign;
        }
        
        intermediate += m_dAmplitude[5]*8/pi/pi*triangle;
      }
      if(m_bSawtooth)  // falling linearly from -180 to 180 then jump discontinuity and cycle repeats
      {
        double saw = 0;
        double sign = 1;
        
        for(DWORD nHarmonic = 1; nHarmonic <= nHarmonics[6]; nHarmonic +=1)
        {
          saw += sign*sin((nSample*radiansPerSample[6]+phaseRadians[6])*nHarmonic)/ nHarmonic;
          sign = -sign;
        }
        
        intermediate += m_dAmplitude[6]*2/pi*saw;
      }
    }
    
    intermediate /= 100.0;
    
    intermediate = intermediate > 1.0 ? 1.0 : (intermediate < -1.0 ? -1.0 : intermediate); // clip result to prevent overflow
    
    // save data
    if (wSmpSize == 1) // 8 bit data
    {
      intermediate *= 127.0;
      bData = BYTE(128 + (int)((intermediate > 0 )? (intermediate + 0.5):(intermediate - 0.5)));
      *pTargetData++ = bData;
    }
    else                  // 16 bit data
    {
      intermediate *= 32767.0;
      nData = (int)((intermediate > 0 )? (intermediate + 0.5):(intermediate - 0.5));
      *pTargetData++ = (BYTE)nData;
      *pTargetData++ = (BYTE)(nData >> 8);
    }
    dwDataPos+=wSmpSize;
    nSample++;
  }
}

static const char* psz_generator = "generator";
static const char* psz_amplitude = "amplitude";
static const char* psz_frequency = "frequency";
static const char* psz_phase = "phase";
static const char* psz_sinusoid1 = "sinusoid1";
static const char* psz_sinusoid2 = "sinusoid2";
static const char* psz_sinusoid3 = "sinusoid3";
static const char* psz_comb = "comb";
static const char* psz_square = "square";
static const char* psz_triangle = "triangle";
static const char* psz_saw = "saw";
static const char* psz_enable = "enable";
static const char* psz_oscillators[] = {psz_sinusoid1,psz_sinusoid2,psz_sinusoid3,psz_comb,psz_square,psz_triangle,psz_saw};
static const char* psz_discontinuities = "discontinuities";
static const char* psz_harmonics = "harmonics";
static const char* psz_samplingrate = "samplingrate";
static const char* psz_filelength = "filelength";
static const char* psz_bits = "bits";


/***************************************************************************/
// CDlgWaveformGeneratorSettings::WriteProperties Write echo properties
/***************************************************************************/
void CDlgWaveformGenerator::settings::WriteProperties(Object_ostream& obs)
{
  BOOL enables[] =
  {m_bSinusoid1,m_bSinusoid2,m_bSinusoid3,m_bComb,m_bSquareWave,m_bTriangle,m_bSawtooth};
  obs.WriteBeginMarker(psz_generator);
  for(int i = 0; i<7; i++)
  {
    obs.WriteBeginMarker(psz_oscillators[i]);
    obs.WriteBool(psz_enable,enables[i]);
    obs.WriteDouble(psz_amplitude, m_dAmplitude[i]);
    obs.WriteDouble(psz_frequency, m_dFrequency[i]);
    obs.WriteDouble(psz_phase, m_dPhase[i]);
    obs.WriteEndMarker(psz_oscillators[i]);
  }
  obs.WriteInteger(psz_discontinuities, m_nHandleDiscontinuities);
  obs.WriteInteger(psz_harmonics, m_nHarmonics);
  obs.WriteInteger(psz_samplingrate, pcm.wf.nSamplesPerSec);
  obs.WriteDouble(psz_filelength, m_fFileLength);
  obs.WriteInteger(psz_bits, pcm.wBitsPerSample);
  obs.WriteEndMarker(psz_generator);
}

/***************************************************************************/
// CDlgWaveformGenerator::settings::bReadProperties Read echo properties
/***************************************************************************/
BOOL CDlgWaveformGenerator::settings::bReadProperties(Object_istream& obs)
{
  BOOL* enables[] =
  {&m_bSinusoid1,&m_bSinusoid2,&m_bSinusoid3,&m_bComb,&m_bSquareWave,&m_bTriangle,&m_bSawtooth};

  int nValue = 0;

  if (!obs.bAtBackslash() || !obs.bReadBeginMarker(psz_generator)) return FALSE;
  while (!obs.bAtEnd())
  {
    BOOL found = TRUE;
    if (obs.bReadDouble(psz_amplitude, m_dAmplitude[0]));
    else if (obs.bReadDouble(psz_frequency, m_dFrequency[0]));
    else if (obs.bReadInteger(psz_discontinuities, m_nHandleDiscontinuities));
    else if (obs.bReadInteger(psz_harmonics, m_nHarmonics));
    else if (obs.bReadInteger(psz_samplingrate, nValue)) pcm.wf.nSamplesPerSec = nValue;
    else if (obs.bReadDouble(psz_filelength, m_fFileLength));
    else if (obs.bReadInteger(psz_bits, nValue)) pcm.wBitsPerSample = (unsigned short)(nValue);
    else
    {
      found = FALSE;

      for (int i = 0; i < 7 ; i++)
      {
        if (obs.bReadBeginMarker(psz_oscillators[i]))
        {
          found = TRUE;
          while (!obs.bAtEnd())
          {
            if (obs.bReadBool(psz_enable, *(enables[i])));
            else if (obs.bReadDouble(psz_amplitude, m_dAmplitude[i]));
            else if (obs.bReadDouble(psz_frequency, m_dFrequency[i]));
            else if (obs.bReadDouble(psz_phase, m_dPhase[i]));
            else if (obs.bEnd(psz_oscillators[i]))
              break;
          }
          break;
        }
      }
    }

    if (!found && obs.bEnd(psz_generator))
      break;
  }
  // assume properties changed
  return TRUE;
}

BOOL CDlgWaveformGenerator::settings::Synthesize(TCHAR *szFileName)
{
  pcm.wf.nBlockAlign = (unsigned short)(pcm.wf.nChannels*(pcm.wBitsPerSample/8));
  pcm.wf.nAvgBytesPerSec = pcm.wf.nSamplesPerSec*pcm.wf.nBlockAlign;

  process *pProcess = new process;

  short int nResult = LOWORD(pProcess->Process(*this));
  if (nResult == PROCESS_ERROR || nResult == PROCESS_NO_DATA || nResult == PROCESS_CANCELED)
  {
    delete pProcess;
    return FALSE;
  }
  
  CRiff::NewWav(szFileName, pcm, pProcess->GetProcessFileName());

  delete pProcess;

  return TRUE;
}

//###########################################################################
// CDlgWaveformGenerator dialog
// Lets the user configure echo/reverb filters.

/////////////////////////////////////////////////////////////////////////////
// CDlgWaveformGenerator message map

BEGIN_MESSAGE_MAP(CDlgWaveformGenerator, CDialog)
    //{{AFX_MSG_MAP(CDlgWaveformGenerator)
	//}}AFX_MSG_MAP
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
CDlgWaveformGenerator::CDlgWaveformGenerator(CWnd* pParent) : CDialog(CDlgWaveformGenerator::IDD, pParent)
{
  //{{AFX_DATA_INIT(CDlgWaveformGenerator)
	m_szSamplingRate = _T("");
	m_szBits = _T("");
	//}}AFX_DATA_INIT
}

/////////////////////////////////////////////////////////////////////////////
// CDlgWaveformGenerator helper functions

/***************************************************************************/
// CDlgWaveformGenerator::DoDataExchange Data exchange
/***************************************************************************/
void CDlgWaveformGenerator::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);

  //{{AFX_DATA_MAP(CDlgWaveformGenerator)
  DDX_CBString(pDX, IDC_GENERATOR_SAMPLINGRATE, m_szSamplingRate);
	DDX_CBString(pDX, IDC_GENERATOR_BITS, m_szBits);
	//}}AFX_DATA_MAP

  working.DoDataExchange(pDX);
}

/////////////////////////////////////////////////////////////////////////////
// CDlgWaveformGenerator message handlers

void CDlgWaveformGenerator::OnOK() 
{
  UpdateData(TRUE);
  
  working.pcm.wf.nSamplesPerSec = _tcstol(m_szSamplingRate,NULL,10);
  ASSERT(working.pcm.wf.nSamplesPerSec > 0);
  working.pcm.wBitsPerSample = (unsigned short) _tcstol(m_szBits,NULL,10);
  ASSERT(working.pcm.wBitsPerSample == 8 || working.pcm.wBitsPerSample == 16);
  current = working;

  
  TCHAR lpszTempPath[_MAX_PATH];
  GetTempPath(_MAX_PATH, lpszTempPath);
  TCHAR szFile[_MAX_PATH];
  // create temp filename for synthesized waveform
  GetTempFileName(lpszTempPath, _T("wav"), 0, szFile);
  if(current.Synthesize(szFile))
  {  
    CSaApp *pApp = (CSaApp*)(AfxGetApp());    
    pApp->OpenWavFileAsNew(szFile);
  }
  
  CDialog::OnOK();
}

BOOL CDlgWaveformGenerator::OnInitDialog() 
{
  working = current;
  
  CDialog::OnInitDialog();

  CComboBox *pWnd;
  CString szSelection;

  pWnd = (CComboBox*) GetDlgItem(IDC_GENERATOR_SAMPLINGRATE);
  szSelection.Format(_T("%d Hz"), working.pcm.wf.nSamplesPerSec);
  pWnd->SetCurSel(pWnd->FindString(0, szSelection));
  
  pWnd = (CComboBox*) GetDlgItem(IDC_GENERATOR_BITS);
  szSelection.Format(_T("%d"), working.pcm.wBitsPerSample);
  pWnd->SetCurSel(pWnd->FindString(0, szSelection));

  OnGeneratorBandwith();
  
  return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgWaveformGenerator::OnGeneratorBandwith() 
{

  UpdateData(TRUE);
  
  BOOL bEnable = working.m_nHandleDiscontinuities == 2;

  CWnd *pWnd;

  pWnd = GetDlgItem(IDC_GENERATOR_NUMBER_OF_HARMONICS);

  if(pWnd)
    pWnd->EnableWindow(bEnable);
}

/***************************************************************************/
// CDlgWaveformGenerator::OnHelpWaveformGenerator
// Call Waveform Generator help
/***************************************************************************/
void CDlgWaveformGenerator::OnHelpWaveformGenerator()
{
	// create the pathname
	CString szPath = AfxGetApp()->m_pszHelpFilePath;
	szPath += "::/User_Interface/Menus/Tools/Waveform_Generator.htm";
	::HtmlHelp(NULL, szPath, HH_DISPLAY_TOPIC, NULL);
}
