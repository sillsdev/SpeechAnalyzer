/////////////////////////////////////////////////////////////////////////////
// sa_doc.cpp:
// Implementation of the CSaDoc class.
// Author: Urs Ruchti
// copyright 2000 JAARS Inc. SIL
//
// Revision History
// 1.06.1.2
//      SDM Added Function CSaDoc::UpdateSegmentBoundaries()
//      SDM Added delete m_pProcessGlottis to CSaDoc::~CSaDoc() memory leak #1
// 1.06.4
//      SDM Added tools/import and advanced/auto-align menu support
//      SDM Moved UserInterface from CPhoneticSegment::Process()
// 1.06.5
//      SDM Removed unused handler for CDlgInput
//      SDM Added update boundaries command handler
//      SDM Added auto snap update
//      SDM Fixed SnapCursor() SNAP_RIGHT to find zero crossing at initial position
// 1.06.6U2
//      SDM initialized m_dwDataSize to 0 in constructor to identify an empty document to editor
//      SDM changed clipboard format for wav to include annotation
//      SDM changed AdjustSegment to adjust overlapping segments on cut
//      SDM removed SaParm.nWordCount
// 1.06.6U4
//      SDM changed undo to include m_bModified
// 1.06.6U5
//      SDM applied default settings to in ApplyWaveFile()
//      SDM changed LoadWave to use InsertSegment()
//      SDM changed InsertSegment to Apply InputFilter to segment string
// 1.5Test8
//      SDM added changes by CLW in 1.07a
// 1.5Test8.1
//      SDM changed WAV format to RIFF_VERSION 7.1
//      SDM added support for Reference annotation
//      SDM added file SaveAs...
//      SDM added validation of WAV data referenced by SA annotations
//      SDM changed UpdateBoundaries to handle bOverlap parameter
// 1.5Test8.2
//      SDM moved save of Workbench processed data to SaveAs
//      SDM fixed WriteWave to handle empty Reference segments
//      SDM cleared database registration in SaveAs
//      SDM Set SaveAs files to Read-Only
//      SDM Modified AdvancedSegment to preserve GLOSS, POS, & REFERENCE
// 1.5Test8.5
//      SDM added support for mode dependent menu and popups
// 1.5Test10.0
//      SDM added support for WAV file database registration
//      SDM moved CParseParm & CSegmentParm to CMainFrame
// 1.5Test10.2
//      SDM fixed LoadWave to correctly set SaParm.szDescription length
//      SDM fixed WriteWave to trim file to correct length
//      SDM disable saving unchanged files
//      SDM fixed bug in SaveAs for new files
// 1.5Test10.7
//      SDM fixed OnAdvancedParse to allow reparsing file
// 1.5Test11.0
//      SDM replaced GetOffsets()->GetSize() with GetSize()
//      SDM replaced GetOffsets()->GetAt(n) with GetOffset(n)
//      SDM replaced GetDurations()->GetAt(n) with GetDuration(n)
//      SDM removed changes to durations for gloss and children
//      SDM split OnAutoSnapUpdate
// 1.5Test11.3
//      SDM replaced changes to Gloss and children (see 11.0)
// 1.5Test11.1A
//      RLJ Set parsing and segmenting paramters to default values.
// 06/07/2000
//      RLJ  Changed OnOpenDocument(const char* pszPathName) so that
//              "File-->Open As" can selectively bring up graphs for
//              Phonetic Analysis or Music Analysis (rather than
//              using waveform default or *.PSA settings).
//      RLJ Added OnCloseDocument
// 1.5Test11.4
//      SDM added support for editing PHONEMIC/TONE/ORTHO to span multiple segments
// 06/17/2000
//        RLJ Extend FileOpenAs to support not only Phonetic/Music Analysis,
//              but also OpenScreenF, OpenScreenG, OpenScreenI, OpenScreenK,
//              OpenScreenM, etc.
//
// 09/25/00 DDO Got rid of the m_bCheckedTWC member var. I rewrote some stuff
//              in sa_dlg so it's no longer needed.
//
/////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "sa_doc.h"
#include "sa_plot.h"
#include "sa_graph.h"
#include "Segment.h"
#include "dlgadvancedsegment.h"
#include "dlgadvancedparsewords.h"
#include "dlgadvancedparsephrases.h"
#include "dlgautoreferencedata.h"
#include "sa.h"
#include "sa_view.h"
#include "sa_wbch.h"
#include "mainfrm.h"
#include "sa_g_stf.h"
#include "DlgExportFW.h"
#include "DlgSaveAsOptions.h"
#include "DlgAutoRecorder.h"
#include "DlgSplitFile.h"
#include "waveresampler.h"
#include "dlgmultichannel.h"
#include "dlgaligntranscriptiondatasheet.h"
#include "TranscriptionDataSettings.h"
#include "dlgimportsfmref.h"
#include "SFMHelper.h"
#include "TextHelper.h"
#include "TranscriptionHelper.h"
#include "Shlobj.h"
#include "ArchiveTransfer.h"
#include "ReferenceSegment.h"
#include "GlossSegment.h"
#include "OrthoSegment.h"
#include "PhonemicSegment.h"
#include "PhoneticSegment.h"
#include "ToneSegment.h"
#include "MusicPhraseSegment.h"
#include "IndependentSegment.h"
#include "DependentSegment.h"
#include "FileUtils.h"
#include "SplitFileUtils.h"
#include "DlgExportFWResult.h"
#include "ClipboardHelper.h"
#include "DlgPlayer.h"
#include "Process\Process.h"
#include "Process\ProcessDoc.h"
#include "Process\sa_w_adj.h"
#include "Process\sa_p_fra.h"
#include "Process\sa_p_lou.h"
#include "Process\sa_p_fra.h"
#include "Process\sa_p_pit.h"
#include "Process\sa_p_cpi.h"
#include "Process\sa_p_spi.h"
#include "Process\sa_p_gra.h"
#include "Process\sa_p_mel.h"
#include "Process\sa_p_cha.h"
#include "Process\sa_p_raw.h"
#include "Process\sa_p_spg.h"
#include "Process\sa_p_sfmt.h"
#include "Process\sa_p_spu.h"
#include "Process\sa_p_fmt.h"
#include "Process\sa_p_zcr.h"
#include "Process\sa_p_dur.h"
#include "Process\sa_p_glo.h"
#include "Process\sa_p_poa.h"
#include "Process\sa_p_rat.h"
#include "Process\sa_p_twc.h"
#include "Process\FormantTracker.h"
#include "Process\Hilbert.h"
#include "settings\obstream.h"
#include "settings\tools.h"
#include "dsp\fragment.h"
#include "DlgInsertSilence.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

//###########################################################################
// CSaDoc
// Document class. Uses processing classes, to process wave data.

static BOOL s_bDocumentWasAlreadyOpen = FALSE;  // 1996-09-05 MRP

IMPLEMENT_DYNCREATE(CSaDoc, CUndoRedoDoc)

/////////////////////////////////////////////////////////////////////////////
// CSaDoc message map
// SDM 1.06.5 removed unused messages
BEGIN_MESSAGE_MAP(CSaDoc, CDocument)
    ON_UPDATE_COMMAND_UI(ID_FILE_SAVE, OnUpdateFileSave)
    ON_COMMAND(ID_FILE_SAVE_AS, OnFileSaveAs)
    ON_UPDATE_COMMAND_UI(ID_FILE_SAVE_AS, OnUpdateFileSaveAs)
    ON_COMMAND(ID_FILE_SPLIT, OnFileSplitFile)
    ON_UPDATE_COMMAND_UI(ID_FILE_SPLIT, OnUpdateFileSplit)
    ON_COMMAND(ID_ADVANCED_PARSE_WORD, OnAdvancedParseWords)
    ON_UPDATE_COMMAND_UI(ID_ADVANCED_PARSE_WORD, OnUpdateAdvancedParseWords)
    ON_COMMAND(ID_ADVANCED_PARSE_PHRASE, OnAdvancedParsePhrases)
    ON_UPDATE_COMMAND_UI(ID_ADVANCED_PARSE_PHRASE, OnUpdateAdvancedParsePhrases)
    ON_COMMAND(ID_ADVANCED_SEGMENT, OnAdvancedSegment)
    ON_UPDATE_COMMAND_UI(ID_ADVANCED_SEGMENT, OnUpdateAdvancedSegment)
    ON_COMMAND(ID_AUTO_ALIGN, OnAutoAlign)
    ON_UPDATE_COMMAND_UI(ID_AUTO_ALIGN, OnUpdateAutoAlign)
    ON_COMMAND(ID_EDIT_UPDATE_BOUNDARIES, OnUpdateBoundaries)
    ON_UPDATE_COMMAND_UI(ID_EDIT_UPDATE_BOUNDARIES, OnUpdateUpdateBoundaries)
    ON_COMMAND(ID_EDIT_AUTO_SNAP_UPDATE, OnAutoSnapUpdate)
    ON_UPDATE_COMMAND_UI(ID_EDIT_AUTO_SNAP_UPDATE, OnUpdateAutoSnapUpdate)
    ON_COMMAND(ID_TOOLS_ADJUST_INVERT, OnToolsAdjustInvert)
    ON_UPDATE_COMMAND_UI(ID_TOOLS_ADJUST_INVERT, OnUpdateToolsAdjustInvert)
    ON_COMMAND(ID_TOOLS_ADJUST_NORMALIZE, OnToolsAdjustNormalize)
    ON_UPDATE_COMMAND_UI(ID_TOOLS_ADJUST_NORMALIZE, OnUpdateToolsAdjustNormalize)
    ON_COMMAND(ID_TOOLS_ADJUST_ZERO, OnToolsAdjustZero)
    ON_UPDATE_COMMAND_UI(ID_TOOLS_ADJUST_ZERO, OnUpdateToolsAdjustZero)
    ON_COMMAND(ID_AUTOMATICMARKUP_REFERENCEDATA, OnAutoReferenceData)
    ON_UPDATE_COMMAND_UI(ID_AUTOMATICMARKUP_REFERENCEDATA, OnUpdateAutoReferenceData)
    ON_COMMAND(ID_TOOLS_ADJUST_SILENCE, OnToolsAdjustSilence)
    ON_UPDATE_COMMAND_UI(ID_TOOLS_ADJUST_SILENCE, OnUpdateToolsAdjustSilence)
END_MESSAGE_MAP()

static LPCSTR IMPORT_END = "import";
static const wchar_t * EMPTY = L"";

// default tags for text importing
static LPCSTR psz_Phonemic = "pm";
static LPCSTR psz_Gloss = "gl";
static LPCSTR psz_Phonetic = "ph";
static LPCSTR psz_Orthographic = "or";
static LPCSTR psz_Reference = "ref";

static LPCSTR psz_sadoc  = "sadoc";
static LPCSTR psz_wndlst = "wndlst";
static LPCSTR psz_saview = "saview";

/////////////////////////////////////////////////////////////////////////////
// CSaDoc construction/destruction/creation

/***************************************************************************/
// CSaDoc::CSaDoc Constructor
/***************************************************************************/
CSaDoc::CSaDoc()
{
    m_bAudioModified = false;
    m_bTransModified = false;
    m_bTempOverlay = false;
    m_ID = -1;
    m_apSegments[PHONETIC] = new CPhoneticSegment(PHONETIC);    // create phonetic segment object
    m_apSegments[TONE] = new CToneSegment(TONE,PHONETIC);       // create tone segment object
    m_apSegments[PHONEMIC] = new CPhonemicSegment(PHONEMIC,PHONETIC); // create phonemic segment object
    m_apSegments[ORTHO] = new COrthoSegment(ORTHO,PHONETIC);    // create orthographic segment object
    m_apSegments[GLOSS] = new CGlossSegment(GLOSS,PHONETIC);    // create gloss segment object
    m_apSegments[REFERENCE] = new CReferenceSegment(REFERENCE,GLOSS);
    m_apSegments[MUSIC_PL1] = new CMusicPhraseSegment(MUSIC_PL1);
    m_apSegments[MUSIC_PL2] = new CMusicPhraseSegment(MUSIC_PL2);
    m_apSegments[MUSIC_PL3] = new CMusicPhraseSegment(MUSIC_PL3);
    m_apSegments[MUSIC_PL4] = new CMusicPhraseSegment(MUSIC_PL4);
    m_bProcessBackground = TRUE;                        // enable background processing
    m_pProcessAdjust = new CProcessAdjust(this);        // create data processing object
    m_pProcessFragments = new CProcessFragments();      //create data processing object
    m_pProcessLoudness = new CProcessLoudness();        // create data processing object
    m_pProcessSmoothLoudness = new CProcessSmoothLoudness(); // create data processing object
    m_pProcessPitch = new CProcessPitch();              // create data processing object
    m_pProcessCustomPitch = new CProcessCustomPitch();  // create data processing object
    m_pProcessSmoothedPitch = new CProcessSmoothedPitch(); // create data processing object
    m_pProcessGrappl = new CProcessGrappl();            // create data processing object
    m_pProcessMelogram = new CProcessMelogram();        // create data processing object CLW 4/5/00
    m_pProcessChange = new CProcessChange();            // create data processing object
    m_pProcessRaw = new CProcessRaw();                  // create data processing object
    m_pProcessHilbert = new CProcessHilbert(m_pProcessRaw);// create data processing object
    m_pProcessSpectrogram = NULL;                       // create data processing object
    m_pProcessSnapshot = NULL;                          // create data processing object
    m_pProcessFormants = new CProcessFormants();        // create data processing object
    m_pProcessFormantTracker = new CProcessFormantTracker(*m_pProcessRaw, *m_pProcessHilbert, *m_pProcessGrappl);  // create data processing object
    m_pProcessZCross   = new CProcessZCross();          // create data processing object
    m_pProcessSpectrum = new CProcessSpectrum();        // create data processing object
    m_pProcessDurations = new CProcessDurations();      // create data processing object
    m_pProcessGlottis  = new CProcessGlottis();         // create data processing object
    m_pProcessPOA = new CProcessPOA();                  // create data processing object
    m_pProcessSDP[0] = NULL;
    m_pProcessSDP[1] = NULL;
    m_pProcessSDP[2] = NULL;
    m_pProcessRatio    = new CProcessRatio();           // create data processing object
    m_pProcessTonalWeightChart = new CProcessTonalWeightChart(); // create data processing object CLW 11/4/99
    m_pCreatedFonts = new CObArray;                     // create graph font array object

    //SDM 1.06.6U2
    m_dwDataSize = 0;
    m_nWbProcess = 0;
    m_bBlockBegin = FALSE;
    m_bWaveUndoNow = FALSE;
    m_nCheckPointCount = 0;
    m_szTempConvertedWave.Empty();
    m_bUsingTempFile = false;
    m_bAllowEdit = true;

    m_pDlgAdvancedSegment = NULL;
    m_pDlgAdvancedParseWords = NULL;
    m_pDlgAdvancedParsePhrases = NULL;

    m_nSelectedChannel = 0;
    m_bMultiChannel = false;
}

/***************************************************************************/
// CSaDoc::~CSaDoc Destructor
/***************************************************************************/
CSaDoc::~CSaDoc()
{

    for (int nLoop = 0; nLoop < ANNOT_WND_NUMBER; nLoop++)
    {
        if (m_apSegments[nLoop])
        {
            delete m_apSegments[nLoop];
        }
    }
    if (m_pProcessAdjust)
    {
        delete m_pProcessAdjust;
        m_pProcessAdjust = NULL;
    }
    if (m_pProcessFragments)
    {
        delete m_pProcessFragments;
        m_pProcessFragments = NULL;
    }
    if (m_pProcessLoudness)
    {
        delete m_pProcessLoudness;
        m_pProcessLoudness = NULL;
    }
    if (m_pProcessSmoothLoudness)
    {
        delete m_pProcessSmoothLoudness;
    }
    if (m_pProcessPitch)
    {
        delete m_pProcessPitch;
    }
    if (m_pProcessCustomPitch)
    {
        delete m_pProcessCustomPitch;
    }
    if (m_pProcessSmoothedPitch)
    {
        delete m_pProcessSmoothedPitch;
    }
    if (m_pProcessGrappl)
    {
        delete m_pProcessGrappl;
    }
    if (m_pProcessMelogram)
    {
        delete m_pProcessMelogram;    // CLW 4/5/00
    }
    if (m_pProcessChange)
    {
        delete m_pProcessChange;
    }
    if (m_pProcessRaw)
    {
        delete m_pProcessRaw;
    }
    if (m_pProcessHilbert)
    {
        delete m_pProcessHilbert;
    }
    if (m_pProcessSpectrogram)
    {
        delete m_pProcessSpectrogram;
    }
    if (m_pProcessSnapshot)
    {
        delete m_pProcessSnapshot;
    }
    if (m_pProcessFormants)
    {
        delete m_pProcessFormants;
    }
    if (m_pProcessFormantTracker)
    {
        delete m_pProcessFormantTracker;
    }
    if (m_pProcessZCross)
    {
        delete m_pProcessZCross;
    }
    if (m_pProcessSpectrum)
    {
        delete m_pProcessSpectrum;
    }
    if (m_pProcessDurations)
    {
        delete m_pProcessDurations;
    }
    if (m_pProcessGlottis)
    {
        delete m_pProcessGlottis;
    }
    if (m_pProcessPOA)
    {
        delete m_pProcessPOA;
    }
    if (m_pProcessSDP[0])
    {
        delete m_pProcessSDP[0];
    }
    if (m_pProcessSDP[1])
    {
        delete m_pProcessSDP[1];
    }
    if (m_pProcessSDP[2])
    {
        delete m_pProcessSDP[2];
    }
    if (m_pProcessRatio)
    {
        delete m_pProcessRatio;
    }
    if (m_pProcessTonalWeightChart)
    {
        delete m_pProcessTonalWeightChart;    // CLW 11/5/99
    }
    if (m_pCreatedFonts)
    {
        for (int i=0; i<m_pCreatedFonts->GetSize(); i++)
        {
            CFontTable * pFont = (CFontTable *) m_pCreatedFonts->GetAt(i);
            if (pFont)
            {
                delete pFont;
            }
        }
        delete m_pCreatedFonts;
    }
    try
    {
        // delete the temporary file
        if (!m_szRawDataWrk.empty())
        {
            RemoveFile(m_szRawDataWrk.c_str());
        }
    }
    catch (CFileException e)
    {
        // error removing file
        CSaApp * pApp = (CSaApp *)AfxGetApp();
        pApp->ErrorMessage(IDS_ERROR_DELTEMPFILE, m_szRawDataWrk.c_str());
    }

    RemoveFile(m_szTempConvertedWave); // delete converted wave temp file

    // unlock and free global data buffer
    if (m_pDlgAdvancedSegment)
    {
        delete m_pDlgAdvancedSegment;
        m_pDlgAdvancedSegment = NULL;
    }
    if (m_pDlgAdvancedParseWords)
    {
        delete m_pDlgAdvancedParseWords;
        m_pDlgAdvancedParseWords = NULL;
    }
    if (m_pDlgAdvancedParsePhrases)
    {
        delete m_pDlgAdvancedParsePhrases;
        m_pDlgAdvancedParsePhrases = NULL;
    }
}

/***************************************************************************/
// CSaDoc::GetFmtParm Get format parameters
// if processed is false, return the parameters for the document
// if processed is true, return the parameters for a single channel
/***************************************************************************/
void CSaDoc::GetFmtParm(CFmtParm & format, bool processed)
{
    format = m_FmtParm;
    if (processed)
    {
        if (format.wChannels==1)
        {
            return;
        }
        // resize format for a single channel
        format.wBlockAlign/=format.wChannels;
        format.dwAvgBytesPerSec/=format.wChannels;
        format.wChannels = 1;
    }
}

/***************************************************************************/
// CSaDoc::GetSaParm Get SA parameters
/***************************************************************************/
void CSaDoc::GetSaParm(SaParm * pSaParm)
{
    pSaParm->fVersion = m_saParm.fVersion;
    pSaParm->szDescription = m_saParm.szDescription;
    pSaParm->wFlags = m_saParm.wFlags;
    pSaParm->byRecordFileFormat = m_saParm.byRecordFileFormat;
    pSaParm->RecordTimeStamp = m_saParm.RecordTimeStamp;
    pSaParm->dwRecordBandWidth = m_saParm.dwRecordBandWidth;
    pSaParm->byRecordSmpSize = m_saParm.byRecordSmpSize;
    pSaParm->dwNumberOfSamples = m_saParm.dwNumberOfSamples;
    pSaParm->lSignalMax = m_saParm.lSignalMax;
    pSaParm->lSignalMin = m_saParm.lSignalMin;
    pSaParm->dwSignalBandWidth = m_saParm.dwSignalBandWidth;
    pSaParm->byQuantization = m_saParm.byQuantization;
    // SDM 1.06.6U2 remove nWordCount
}

/***************************************************************************/
// CSaDoc::GetGender  Retrieve gender info
/***************************************************************************/
int CSaDoc::GetGender()
{
    SourceParm * pSourceParm = GetSourceParm();
    int nGender = pSourceParm->nGender;
    if (nGender == UNDEFINED_DATA)
    {
        // guess
        m_pProcessGrappl->Process(this, this); // process pitch
        if (m_pProcessGrappl->IsDataReady())
        {
            double dAvgPitch = m_pProcessGrappl->GetAveragePitch();
            if (dAvgPitch < 200.0)
            {
                nGender = 0;    // adult male
            }
            else if (dAvgPitch < 350.0)
            {
                nGender = 1;    // adult female
            }
            else
            {
                nGender = 2;    // child
            }
        }
    }
    return nGender;
}

/***************************************************************************/
// CSaDoc::GetUttParm Get utterance parameters
/***************************************************************************/
void CSaDoc::GetUttParm(CUttParm * pUttParm, BOOL bOriginal)
{
    CUttParm & uttParm = bOriginal ? m_uttOriginal : m_uttParm;

    pUttParm->nMinFreq = uttParm.nMinFreq;
    pUttParm->nMaxFreq = uttParm.nMaxFreq;
    pUttParm->nCritLoud = uttParm.nCritLoud;
    pUttParm->nMaxChange = uttParm.nMaxChange;
    pUttParm->nMinGroup = uttParm.nMinGroup;
    pUttParm->nMaxInterp = uttParm.nMaxInterp;
}

/***************************************************************************/
// CSaDoc::SetFmtParm Set format parameters
/***************************************************************************/
void CSaDoc::SetFmtParm(CFmtParm * pFmtParm, BOOL bAdjustSpectro)
{

    m_FmtParm.wTag = pFmtParm->wTag;
    m_FmtParm.wChannels = pFmtParm->wChannels;
    m_FmtParm.dwSamplesPerSec = pFmtParm->dwSamplesPerSec;
    m_FmtParm.dwAvgBytesPerSec = pFmtParm->dwAvgBytesPerSec;
    m_FmtParm.wBlockAlign = pFmtParm->wBlockAlign;
    m_FmtParm.wBitsPerSample = pFmtParm->wBitsPerSample;

    if (bAdjustSpectro)
    {
        // adjust the spectrum parameters
        CSpectrumParm * pSpectrum = m_pProcessSpectrum->GetSpectrumParms();
        pSpectrum->nFreqUpperBound = (int)(m_FmtParm.dwSamplesPerSec / 2);
    }
}

/***************************************************************************/
// CSaDoc::SetSaParm Set SA parameters
/***************************************************************************/
void CSaDoc::SetSaParm(SaParm * pSaParm)
{
    m_saParm.fVersion = pSaParm->fVersion;
    m_saParm.szDescription = pSaParm->szDescription;
    m_saParm.wFlags = pSaParm->wFlags;
    m_saParm.byRecordFileFormat = pSaParm->byRecordFileFormat;
    m_saParm.RecordTimeStamp = pSaParm->RecordTimeStamp;
    m_saParm.dwRecordBandWidth = pSaParm->dwRecordBandWidth;
    m_saParm.byRecordSmpSize = pSaParm->byRecordSmpSize;
    m_saParm.dwNumberOfSamples = pSaParm->dwNumberOfSamples;
    m_saParm.lSignalMax = pSaParm->lSignalMax;
    m_saParm.lSignalMin = pSaParm->lSignalMin;
    m_saParm.dwSignalBandWidth = pSaParm->dwSignalBandWidth;
    m_saParm.byQuantization = pSaParm->byQuantization;
    // SDM 1.06.6U2 remove nWordCount
}

/***************************************************************************/
// CSaDoc::SetUttParm Set utterance parameters
/***************************************************************************/
void CSaDoc::SetUttParm(const CUttParm * pUttParm, BOOL bOriginal)
{
    CUttParm & uttParm = bOriginal ? m_uttOriginal : m_uttParm;

    uttParm.nMinFreq = pUttParm->nMinFreq;
    uttParm.nMaxFreq = pUttParm->nMaxFreq;
    uttParm.nCritLoud = pUttParm->nCritLoud;
    uttParm.nMaxChange = pUttParm->nMaxChange;
    uttParm.nMinGroup = pUttParm->nMinGroup;
    uttParm.nMaxInterp = pUttParm->nMaxInterp;
}

/***************************************************************************/
// CSaDoc::DeleteContents
// Called by the framework to clear contents of member data before opening
// a new document.
/***************************************************************************/
void CSaDoc::DeleteContents()
{
    CMainFrame * pMain = (CMainFrame *)AfxGetMainWnd();
    // reset and clear all data members
    m_nWbProcess = 0; // plain raw data
    m_bBlockBegin = FALSE;
    m_bWaveUndoNow = FALSE;
    m_nCheckPointCount = 0;
    m_fileStat.m_ctime = 0;
    m_fileStat.m_mtime = 0;
    m_fileStat.m_atime = 0;
    m_fileStat.m_size = 0;
    m_fileStat.m_attribute = 0;
    m_fileStat.m_szFullName[0] = 0;
    m_dwDataSize = 0;
    m_FmtParm.wTag = 1;
    m_FmtParm.wChannels = 1;
    m_FmtParm.dwSamplesPerSec = 22050;
    m_FmtParm.dwAvgBytesPerSec = 44100;
    m_FmtParm.wBlockAlign = 2;
    m_FmtParm.wBitsPerSample = 16;
    m_saParm.fVersion = (float) RIFF_VERSION_UNKNOWN;
    m_saParm.szDescription.Empty();
    m_saParm.wFlags = SA_FLAG_HIGHPASS;
    m_saParm.byRecordFileFormat = FILE_FORMAT_WAV;
    m_saParm.RecordTimeStamp = 0;
    m_saParm.dwRecordBandWidth = 0;
    m_saParm.byRecordSmpSize = 0;
    m_saParm.dwNumberOfSamples = 0;
    m_saParm.lSignalMax = 0;
    m_saParm.lSignalMin = 0;
    m_saParm.dwSignalBandWidth = 0;
    m_saParm.byQuantization = 0;
    // SDM 1.06.6U2 remove nWordCount
    m_sourceParm.szLanguage.Empty();
    m_sourceParm.szEthnoID = "???"; // must be three characters
    m_sourceParm.szDialect.Empty();
    m_sourceParm.szCountry.Empty();
    m_sourceParm.szFamily.Empty();
    m_sourceParm.szSpeaker.Empty();
    m_sourceParm.szReference.Empty();
    m_sourceParm.szFreeTranslation.Empty();// SDM 1.5Test8.1
    m_sourceParm.szTranscriber.Empty();// SDM 1.5Test8.1
    m_sourceParm.nGender = UNDEFINED_DATA;
    m_uttParm.Init(8); // assume 8-bit data

    // Set pitch parameters to default values. // RLJ 1.5Test11.1A
    const CPitchParm * pPitchParm   = pMain->GetPitchParmDefaults();
    m_pitchParm.nRangeMode  = pPitchParm->nRangeMode;
    m_pitchParm.nScaleMode  = pPitchParm->nScaleMode;
    m_pitchParm.nUpperBound = pPitchParm->nUpperBound;
    m_pitchParm.nLowerBound = pPitchParm->nLowerBound;
    m_pitchParm.bUseCepMedianFilter  = pPitchParm->bUseCepMedianFilter;
    m_pitchParm.nCepMedianFilterSize = pPitchParm->nCepMedianFilterSize;

    // RLJ, 09/27/2000: Bug GPI-01
    m_pitchParm.nManualPitchUpper = pPitchParm->nManualPitchUpper;
    m_pitchParm.nManualPitchLower = pPitchParm->nManualPitchLower;

    // Set music parameters to default values.
    const CMusicParm * pMusicParm   = pMain->GetMusicParmDefaults();
    m_musicParm.nRangeMode  = pMusicParm->nRangeMode;
    m_musicParm.nUpperBound = pMusicParm->nUpperBound;
    m_musicParm.nLowerBound = pMusicParm->nLowerBound;

    m_musicParm.nManualUpper = pMusicParm->nManualUpper;
    m_musicParm.nManualLower = pMusicParm->nManualLower;

    m_musicParm.nCalcRangeMode  = pMusicParm->nCalcRangeMode;
    m_musicParm.nCalcUpperBound = pMusicParm->nCalcUpperBound;
    m_musicParm.nCalcLowerBound = pMusicParm->nCalcLowerBound;

    m_musicParm.nManualCalcUpper = pMusicParm->nManualCalcUpper;
    m_musicParm.nManualCalcLower = pMusicParm->nManualCalcLower;

    m_intensityParm = pMain->GetCIntensityParmDefaults();

    m_SDPParm.nPanes = 6;
    m_SDPParm.nStepMode = 0;
    m_SDPParm.nSteps = 1;
    m_SDPParm.bAverage = TRUE;
    m_SDPParm.nUpperBound = 50;

    for (int nLoop = 0; nLoop < ANNOT_WND_NUMBER; nLoop++)
    {
        if (m_apSegments[nLoop])
        {
            m_apSegments[nLoop]->DeleteContents();
        }
    }

    // Set spectrum parameters to default values. // RLJ 1.5Test11.1A
    CSpectrumParm * pSpectrumParmDefaults    = pMain->GetSpectrumParmDefaults();
    CSpectrumParm Spectrum;
    Spectrum.nScaleMode        = pSpectrumParmDefaults->nScaleMode;
    Spectrum.nPwrUpperBound    = pSpectrumParmDefaults->nPwrUpperBound;
    Spectrum.nPwrLowerBound    = pSpectrumParmDefaults->nPwrLowerBound;
    Spectrum.nFreqUpperBound   = pSpectrumParmDefaults->nFreqUpperBound;
    Spectrum.nFreqLowerBound   = pSpectrumParmDefaults->nFreqLowerBound;
    Spectrum.nFreqScaleRange   = pSpectrumParmDefaults->nFreqScaleRange;
    Spectrum.nSmoothLevel      = pSpectrumParmDefaults->nSmoothLevel;
    Spectrum.nPeakSharpFac     = pSpectrumParmDefaults->nPeakSharpFac;
    Spectrum.cWindow        = pSpectrumParmDefaults->cWindow;
    Spectrum.bShowLpcSpectrum  = pSpectrumParmDefaults->bShowLpcSpectrum;
    Spectrum.bShowCepSpectrum  = pSpectrumParmDefaults->bShowCepSpectrum;
    Spectrum.bShowFormantFreq  = pSpectrumParmDefaults->bShowFormantFreq;
    Spectrum.bShowFormantBandwidth  = pSpectrumParmDefaults->bShowFormantBandwidth;
    Spectrum.bShowFormantPower = pSpectrumParmDefaults->bShowFormantPower;

    m_pProcessSpectrum->SetSpectrumParms(&Spectrum);

    // Set formant chart parameters to default values. // AKE 1.5Test13.1
    CFormantParm * pFormantParmDefaults    = pMain->GetFormantParmDefaults();
    CFormantParm Formant;
    Formant.bFromLpcSpectrum             = pFormantParmDefaults->bFromLpcSpectrum;
    Formant.bFromCepstralSpectrum        = pFormantParmDefaults->bFromCepstralSpectrum;
    Formant.bTrackFormants               = pFormantParmDefaults->bTrackFormants;
    Formant.bSmoothFormants              = pFormantParmDefaults->bSmoothFormants;
    Formant.bMelScale                    = pFormantParmDefaults->bMelScale;

    m_pProcessFormants->SetFormantParms(&Formant);


    // delete temporary file
    if (!m_szTempWave.IsEmpty())
    {
        // SDM 1.06.6U2
        try
        {
            RemoveFile(m_szTempWave);
            m_szTempWave.Empty();
        }
        catch (CFileException e)
        {
            m_szTempWave.Empty();
        }
    }
    CreateFonts();
}

/***************************************************************************/
// CSaDoc::CreateFonts
// Create the fonts for this document
/***************************************************************************/
void CSaDoc::CreateFonts()
{

    CMainFrame * pMain = (CMainFrame *)AfxGetMainWnd();

    // init the graph fonts with default if requested
    try
    {
        for (int i=0; i < pMain->m_GraphFontFaces.GetSize(); i++)
        {
            CString face = pMain->m_GraphFontFaces.GetAt(i);
            UINT size = pMain->m_GraphFontSizes.GetAt(i);
            pMain->SetFontFace(i, face);
            pMain->SetFontSize(i, size);

            if (m_pCreatedFonts->GetSize() > i && GetFont(i))
            {
                delete GetFont(i);
            }
            m_pCreatedFonts->SetAtGrow(i, GetSegment(i)->NewFontTable());
            CAnnotationWnd::CreateAnnotationFont(GetFont(i), pMain->GetFontSize(i), pMain->GetFontFace(i));
        }
    }
    catch (CMemoryException e)
    {
        // handle memory fail exception
        CSaApp * pApp = (CSaApp *)AfxGetApp();
        pApp->ErrorMessage(IDS_ERROR_MEMALLOC);
        return;
    }
}


/***************************************************************************/
// CSaDoc::OnCloseDocument document closing
/***************************************************************************/
void CSaDoc::OnCloseDocument()
{
    CMainFrame * pFrame = (CMainFrame *) AfxGetMainWnd();
    ASSERT(pFrame);
    ASSERT(pFrame->IsKindOf(RUNTIME_CLASS(CMainFrame)));

    // NOTE - OnCloseDocument calls delete this,
    // don't access the document after this point.
    CDocument::OnCloseDocument();

    if (pFrame)
    {
        pFrame->AppMessage(HINT_APP_DOC_LIST_CHANGED,NULL);
    }
}

/***************************************************************************/
// CSaDoc::OnNewDocument Creating a new document
// This function allocates the pcm read buffer for the new document.
/***************************************************************************/
BOOL CSaDoc::OnNewDocument()
{
    if (!CDocument::OnNewDocument())
    {
        return FALSE;
    }
    return TRUE;
}

//SDM 1.06.6U2 Split function
/***************************************************************************/
// CSaDoc::OnOpenDocument Opening a new document
// Reads all available information from the RIFF header of a wave file and
// stores it into data members. From the wave data itself only the first
// block of data is read and stored in the data buffer.
/***************************************************************************/
BOOL CSaDoc::OnOpenDocument(LPCTSTR pszPathName)
{
    CSaApp * pApp = (CSaApp *)AfxGetApp();
    CSaString szWavePath = pszPathName;

    // first check the file size
    HANDLE hFile = CreateFile(pszPathName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    DWORD dwSize = GetFileSize(hFile, NULL);
    CloseHandle(hFile);

    if ((dwSize==0) || (dwSize == INVALID_FILE_SIZE))
    {
        pApp->ErrorMessage(IDS_ERROR_FILEOPEN, pszPathName);
        EndWaitCursor();
        return FALSE;
    }

    m_bAllowEdit = true;

    // if the return value is false, the file is not in an acceptable format
    if (!IsStandardWaveFormat(pszPathName))
    {
        // tell the user what's going on!
        AfxMessageBox(IDS_SUPPORT_WAVE_COPY, MB_OK|MB_ICONWARNING,0);
        // convert to wave will select or merge the channels.
        // the output will be a single channel file
        m_bUsingTempFile = true;
        if (!ConvertToWave(pszPathName))
        {
            // couldn't convert to wave
            return FALSE;
        }
        szWavePath = m_szTempConvertedWave;
    }

    if (!CheckWaveFormatForOpen(szWavePath))
    {
        return FALSE; // can't open the wave file
    }

    if (!CDocument::OnOpenDocument(pszPathName))
    {
        return FALSE;
    }

    return LoadDataFiles(pszPathName);
}

/*
* helper class for splitting channels
*/
struct SChannel
{
public:
    HPSTR pData;
    CFile * pFile;
};

//SDM 1.06.6U2 Split function from OnOpenDocument
/***************************************************************************/
// CSaDoc::LoadDataFiles Load WAV file into document
// Reads all available information from the RIFF header of a wave file and
// stores it into data members. From the wave data itself only the first
// block of data is read and stored in the data buffer.
/***************************************************************************/
BOOL CSaDoc::LoadDataFiles(LPCTSTR pszPathName, bool bTemp/*=FALSE*/)
{
    CMainFrame * pMainWnd = (CMainFrame *)AfxGetMainWnd();

    BeginWaitCursor(); // wait cursor
    CSaString szWavePath = pszPathName;
    if (m_bUsingTempFile)
    {
        szWavePath = m_szTempConvertedWave;
    }

    // this will reload the m_FmtParm structure
    if (!ReadRiff(szWavePath))
    {
        EndWaitCursor();
        return FALSE;
    }

    LoadTranscriptionData(pszPathName, bTemp);

    //SDM 1.06.6U2
    //handle temporary file load.
    if (bTemp)
    {
        ApplyWaveFile( pszPathName, GetRawDataSize());
    }
    else
    {
        // create the temporary wave chunk copy
        CopyWaveToTemp( szWavePath);
    }

    // This flag will be the only signature of a "Stereo file"
    m_bMultiChannel = (m_FmtParm.wChannels > 1);
    if (m_bMultiChannel)
    {
        // stereo SA is expecting a mono file (so we will make this a mono file)
        m_nSelectedChannel = 0;
        if (pMainWnd->GetShowAdvancedAudio())
        {
            CDlgMultiChannel dlg(m_FmtParm.wChannels);
            if (dlg.DoModal()!=IDOK)
            {
                return FALSE;
            }
            m_nSelectedChannel = dlg.m_nChannel;
        }
    }

    s_bDocumentWasAlreadyOpen = FALSE;

    ((CGlossSegment *)m_apSegments[GLOSS])->CorrectGlossDurations(this);; // SDM 1.5Test11.3

    // if player is visible, disable the speed slider until required processing is completed
    CMainFrame * pMain = (CMainFrame *)AfxGetMainWnd();
    CDlgPlayer * pPlayer = pMain->GetPlayer();
    if ((pPlayer!=NULL) && (pPlayer->IsWindowVisible()))
    {
        pPlayer->EnableSpeedSlider(FALSE);
    }

    EndWaitCursor();

    return TRUE;
}

/***************************************************************************/
// CSaDoc::LoadTranscriptionData Load WAV file into document
// Reads all available information from the RIFF header of a wave file and
// stores it into data members. From the wave data itself only the first
// block of data is read and stored in the data buffer.
/***************************************************************************/
BOOL CSaDoc::LoadTranscriptionData(LPCTSTR pszWavePath, BOOL bTemp)
{

    CSaApp * pApp = (CSaApp *)AfxGetApp();

    CoInitialize(NULL);
    ISaAudioDocumentReaderPtr saAudioDocRdr;
    HRESULT createResult = saAudioDocRdr.CreateInstance(__uuidof(SaAudioDocumentReader));
    if (createResult)
    {
        CSaString szCreateResult;
        szCreateResult.Format(_T("%x"), createResult);
        pApp->ErrorMessage(IDS_ERROR_CREATE_INSTANCE, _T("SaAudioDocumentReader.CreateInstance()"), szCreateResult);
        return FALSE;
    }

    TRACE(L"reading %s\n",pszWavePath);
    short initSucceeded = TRUE;
    try
    {
        initSucceeded = saAudioDocRdr->Initialize(pszWavePath, (short)bTemp);
    }
    catch (...)
    {
        CSaString szError;
        szError.Format(_T("Unexpected exception thrown from 'SaAudioDocumentReader::Initialize()'"));
        pApp->ErrorMessage(szError);
        initSucceeded = FALSE;
    }

    // re-initialize the utterance parameters
    m_uttParm.Init(m_FmtParm.wBitsPerSample);

    if (initSucceeded && saAudioDocRdr->DocumentExistsInDB)
    {
        ReadNonSegmentData(saAudioDocRdr);
        ReadTranscription(PHONETIC, saAudioDocRdr);
        ReadTranscription(PHONEMIC, saAudioDocRdr);
        ReadTranscription(TONE, saAudioDocRdr);
        ReadTranscription(ORTHO, saAudioDocRdr);
        ReadGlossPosAndRefSegments(saAudioDocRdr);
        ReadScoreData(saAudioDocRdr);
        ReadTranscription(MUSIC_PL1, saAudioDocRdr);
        ReadTranscription(MUSIC_PL2, saAudioDocRdr);
        ReadTranscription(MUSIC_PL3, saAudioDocRdr);
        ReadTranscription(MUSIC_PL4, saAudioDocRdr);
    }
    else
    {
        m_saParm.fVersion = (float)RIFF_VERSION_UNKNOWN;
        m_saParm.szDescription.Empty();
        m_saParm.wFlags = 0;
        m_saParm.byRecordFileFormat = FILE_FORMAT_WAV;
        m_saParm.lSignalMax = 0;
        m_saParm.lSignalMin = 0;

        if (m_saParm.dwRecordBandWidth == 0 && m_saParm.byRecordSmpSize == 0 &&
                m_saParm.dwSignalBandWidth == 0 && m_saParm.byQuantization == 0)
        {
            // These parameters used to be initialized improperly to 0 -- Change them
            m_saParm.RecordTimeStamp = m_fileStat.m_ctime; // Creation time of file
            m_saParm.dwRecordBandWidth = m_FmtParm.dwSamplesPerSec / 2; // Assume raw untouched file
            m_saParm.byRecordSmpSize = (BYTE)m_FmtParm.wBitsPerSample;
            m_saParm.dwNumberOfSamples = GetDataSize() / m_FmtParm.wBlockAlign;
            m_saParm.dwSignalBandWidth = m_FmtParm.dwSamplesPerSec / 2;
            m_saParm.byQuantization = (BYTE)m_FmtParm.wBitsPerSample;
        }
    }

    saAudioDocRdr->Close();
    saAudioDocRdr->Release();
    saAudioDocRdr = NULL;
    CoUninitialize();

    return TRUE;
}

/***************************************************************************/
// CSaDoc::ReadRiff  Read fmt and (wave) data chunks from wave file
/***************************************************************************/
BOOL CSaDoc::ReadRiff(LPCTSTR pszPathName)
{

    CSaApp * pApp = ((CSaApp *)AfxGetApp());

    // get file information
    CFile::GetStatus(pszPathName, m_fileStat);

    // open file
    HMMIO hmmioFile; // file handle
    hmmioFile = mmioOpen(const_cast<TCHAR *>(pszPathName), NULL, MMIO_READ | MMIO_DENYWRITE);
    if (!hmmioFile)
    {
        // error opening file
        pApp->ErrorMessage(IDS_ERROR_FILEOPEN, pszPathName);
        return FALSE;
    }

    // locate a 'RIFF' chunk with a 'WAVE' form type to make sure it's a WAVE file.
    MMCKINFO mmckinfoParent;
    mmckinfoParent.fccType = mmioFOURCC('W', 'A', 'V', 'E'); // prepare search code
    if (mmioDescend(hmmioFile, (LPMMCKINFO)&mmckinfoParent, NULL, MMIO_FINDRIFF))
    {
        // error descending into wave chunk
        pApp->ErrorMessage(IDS_ERROR_WAVECHUNK, pszPathName);
        mmioClose(hmmioFile, 0);
        return FALSE;
    }

    // find the format chunk. It should be a subchunk of the 'RIFF' parent chunk
    MMCKINFO mmckinfoSubchunk;
    mmckinfoSubchunk.ckid = mmioFOURCC('f', 'm', 't', ' '); // prepare search code
    LONG lError;
    if (!mmioDescend(hmmioFile, &mmckinfoSubchunk, &mmckinfoParent, MMIO_FINDCHUNK))
    {
        // fmt chunk found
        lError = mmioRead(hmmioFile, (HPSTR)&m_FmtParm.wTag, sizeof(WORD)); // read format tag
        if (m_FmtParm.wTag != WAVE_FORMAT_PCM)   // check if PCM format
        {
            // error testing pcm format
            pApp->ErrorMessage(IDS_ERROR_FORMATPCM, pszPathName);
            mmioClose(hmmioFile, 0);
            return FALSE;
        }
        if (lError != -1)
        {
            // read channel number
            lError = mmioRead(hmmioFile, (HPSTR)&m_FmtParm.wChannels, sizeof(WORD));
        }
        if (lError != -1)
        {
            // read sampling rate
            lError = mmioRead(hmmioFile, (HPSTR)&m_FmtParm.dwSamplesPerSec, sizeof(DWORD));
        }
        if (lError != -1)
        {
            // read throughput
            lError = mmioRead(hmmioFile, (HPSTR)&m_FmtParm.dwAvgBytesPerSec, sizeof(DWORD));
        }
        if (lError != -1)
        {
            // read sampling rate for all channels
            lError = mmioRead(hmmioFile, (HPSTR)&m_FmtParm.wBlockAlign, sizeof(WORD));
        }
        if (lError != -1)
        {
            // read sample word size
            lError = mmioRead(hmmioFile, (HPSTR)&m_FmtParm.wBitsPerSample, sizeof(WORD));
        }

        // get out of 'fmt ' chunk
        mmioAscend(hmmioFile, &mmckinfoSubchunk, 0);
        if (lError == -1)
        {
            // error reading format chunk
            pApp->ErrorMessage(IDS_ERROR_READFORMAT, pszPathName);
            mmioClose(hmmioFile, 0);
            return FALSE;
        }

        // adjust the spectrum parameters
        CSpectrumParm * pSpectrum = m_pProcessSpectrum->GetSpectrumParms();
        pSpectrum->nFreqUpperBound = (int)(m_FmtParm.dwSamplesPerSec / 2);
    }
    else
    {
        // error finding format chunk
        pApp->ErrorMessage(IDS_ERROR_FORMATCHUNK, pszPathName);
        mmioClose(hmmioFile, 0);
        return FALSE;
    }

    // determine how much sound data is in the file. Find the data subchunk
    mmckinfoSubchunk.ckid = mmioFOURCC('d', 'a', 't', 'a');
    if (mmioDescend(hmmioFile, &mmckinfoSubchunk, &mmckinfoParent, MMIO_FINDCHUNK))
    {
        // error finding data chunk
        pApp->ErrorMessage(IDS_ERROR_DATACHUNK, pszPathName);
        mmioClose(hmmioFile, 0);
        return FALSE;
    }

    // get the size of the data subchunk
    m_dwDataSize = mmckinfoSubchunk.cksize;
    if (m_dwDataSize == 0L)
    {
        // error 0 size data chunk
        pApp->ErrorMessage(IDS_ERROR_NODATA, pszPathName);
        mmioClose(hmmioFile, 0);
        return FALSE;
    }

    mmioClose(hmmioFile, 0);
    return TRUE;
}

/***************************************************************************/
// CSaDoc::ReadNonSegmentData reads from the database the non segment
// oriented data. (i.e. what used to be stored in the wave file's sa, utt
// speaker, language, and reference chunks.
/***************************************************************************/
void CSaDoc::ReadNonSegmentData(ISaAudioDocumentReaderPtr saAudioDocRdr)
{
    m_szMD5HashCode = saAudioDocRdr->MD5HashCode;

    // Get SA data
    m_saParm.szDescription = (wchar_t *)saAudioDocRdr->SADescription;
    m_saParm.wFlags = (USHORT)saAudioDocRdr->SAFlags;
    m_saParm.byRecordFileFormat = (BYTE)saAudioDocRdr->RecordFileFormat;
    m_saParm.RecordTimeStamp = (CTime)saAudioDocRdr->RecordTimeStamp;
    m_saParm.dwRecordBandWidth = saAudioDocRdr->RecordBandWidth;
    m_saParm.byRecordSmpSize = (BYTE)saAudioDocRdr->RecordSampleSize;
    m_saParm.lSignalMax = saAudioDocRdr->SignalMax; // read maximum signal
    m_saParm.lSignalMin = saAudioDocRdr->SignalMin; // read minimum signal
    m_saParm.dwSignalBandWidth = saAudioDocRdr->SignalBandWidth; // read signal bandwidth
    m_saParm.byQuantization = (BYTE)saAudioDocRdr->SignalEffSampSize; // read signal effective sample size
    m_saParm.dwNumberOfSamples = saAudioDocRdr->NumberOfSamples; // number of samples

    // Get UTT data
    m_uttParm.nMinFreq = (USHORT)saAudioDocRdr->CalcFreqLow;
    m_uttParm.nMaxFreq = (USHORT)saAudioDocRdr->CalcFreqHigh;
    m_uttParm.nCritLoud = (USHORT)saAudioDocRdr->CalcVoicingThd;
    m_uttParm.nMaxChange = (USHORT)saAudioDocRdr->CalcPercntChng;
    m_uttParm.nMinGroup = (USHORT)saAudioDocRdr->CalcGrpSize;
    m_uttParm.nMaxInterp = (USHORT)saAudioDocRdr->CalcIntrpGap;

    // check the value of nCritLoud
    if ((m_FmtParm.wBitsPerSample == 16) && (m_uttParm.nCritLoud > 0) && (m_uttParm.nCritLoud < 256))
    {
        m_uttParm.nCritLoud <<= 8;
    }

    SetUttParm(&m_uttParm, TRUE);  // Copy uttParms to original for safe keeping

    // Get Speaker data
    int nGender = UNDEFINED_DATA;
    switch ((wchar_t)saAudioDocRdr->SpeakerGender)
    {
    case 'M':
        nGender = 0;
        break;
    case 'F':
        nGender = 1;
        break;
    case 'C':
        nGender = 2;
        break;
    }

    m_sourceParm.nGender = nGender;
    m_sourceParm.szSpeaker = (wchar_t *)saAudioDocRdr->SpeakerName;

    // Get language data
    m_sourceParm.szEthnoID = (wchar_t *)saAudioDocRdr->EthnologueId;
    m_sourceParm.szRegion = (wchar_t *)saAudioDocRdr->Region;
    m_sourceParm.szCountry = (wchar_t *)saAudioDocRdr->Country;
    m_sourceParm.szFamily = (wchar_t *)saAudioDocRdr->Family;
    m_sourceParm.szLanguage = (wchar_t *)saAudioDocRdr->LanguageName;
    m_sourceParm.szDialect = (wchar_t *)saAudioDocRdr->Dialect;

    // Get reference data
    m_sourceParm.szReference = (wchar_t *)saAudioDocRdr->NoteBookReference;
    m_sourceParm.szFreeTranslation = (wchar_t *)saAudioDocRdr->FreeFormTranslation;
    m_sourceParm.szTranscriber = (wchar_t *)saAudioDocRdr->Transcriber;
}

/***************************************************************************/
// CSaDoc::ReadTranscription  Read a transcription from the transcription
// database and load it into the document.
/***************************************************************************/
void CSaDoc::ReadTranscription(int transType, ISaAudioDocumentReaderPtr saAudioDocRdr)
{

    CSegment * pSegment = ((CSaDoc *)this)->GetSegment(transType);

    DWORD offset = 0;
    DWORD nLength = 0;
    BSTR * annotation = (BSTR *)calloc(1, sizeof(long));
    CSaString szFullTrans = _T("");

    CDWordArray dwOffsets;
    CDWordArray dwDurations;

    while (saAudioDocRdr->ReadSegment((long)transType, &offset, &nLength, annotation))
    {
        CSaString sztmpAnnotation = *annotation;
        szFullTrans += sztmpAnnotation;

        // Loop through the code points in the annotation and save
        // offsets and durations for each.
        for (int i = 0; i < sztmpAnnotation.GetLength(); i++)
        {
            dwOffsets.Add(offset / (long)m_FmtParm.wChannels);
            dwDurations.Add(nLength / (long)m_FmtParm.wChannels);
        }
    }

    pSegment->SetString(szFullTrans);
    ASSERT(dwOffsets.GetSize()==dwDurations.GetSize());
    for (int i=0; i<dwOffsets.GetSize(); i++)
    {
        pSegment->InsertAt(i,dwOffsets[i],dwDurations[i]);
    }

    free(annotation);
}

/***************************************************************************/
// CSaDoc::ReadGlossPosAndRefSegments reads the gloss, part of speech and
// reference information from the database.
/***************************************************************************/
void CSaDoc::ReadGlossPosAndRefSegments(ISaAudioDocumentReaderPtr saAudioDocRdr)
{

    CGlossSegment * pGloss = (CGlossSegment *)m_apSegments[GLOSS];
    DWORD offset = 0;
    DWORD length = 0;
    BSTR * gloss = (BSTR *)calloc(1, sizeof(long));
    BSTR * pos = (BSTR *)calloc(1, sizeof(long));
    BSTR * ref = (BSTR *)calloc(1, sizeof(long));
    VARIANT_BOOL isBookmark;
    int i = 0;
    int nRef = 0;

    while (saAudioDocRdr->ReadMarkSegment(&offset, &length, gloss, pos, ref, &isBookmark))
    {
        offset /= m_FmtParm.wChannels;
        length /= m_FmtParm.wChannels;

        CSaString szGloss = *gloss;
        pGloss->Insert(i, szGloss, (isBookmark!=0), offset, length);
        pGloss->POSSetAtGrow( i++, (CSaString)*pos);

        CSaString szRef = *ref;
        if (szRef.GetLength())
        {
            m_apSegments[REFERENCE]->Insert(nRef++, szRef, 0, offset, length);
        }
    }

    free(gloss);
    free(pos);
    free(ref);
}

/***************************************************************************/
// CSaDoc::ReadScoreData Reads the score data from the database and loads
// it into the staff graph.
/***************************************************************************/
void CSaDoc::ReadScoreData(ISaAudioDocumentReaderPtr saAudioDocRdr)
{
    //disabling loading of MusicXML information
    //m_szMusicScore = (wchar_t *)saAudioDocRdr->ReadAsMusicXML();
}

/***************************************************************************/
// CSaDoc::InsertTranscriptions  Insert transcriptions from another WAV file
// into current document
/***************************************************************************/
BOOL CSaDoc::InsertTranscriptions(LPCTSTR pszPathName, DWORD dwPos)
{

    TRACE(L"inserting transcriptions for %s at %d\n",pszPathName,dwPos);

    CSaApp * pApp = (CSaApp *)AfxGetApp();

    BeginWaitCursor(); // wait cursor

    // initialize com object
    CoInitialize(NULL);
    ISaAudioDocumentReaderPtr saAudioDocRdr;
    HRESULT createResult = saAudioDocRdr.CreateInstance(__uuidof(SaAudioDocumentReader));
    if (createResult)
    {
        CSaString szCreateResult;
        szCreateResult.Format(_T("%x"), createResult);
        pApp->ErrorMessage(IDS_ERROR_CREATE_INSTANCE, _T("SaAudioDocumentReader.CreateInstance()"), szCreateResult);
        return FALSE;
    }

    TRACE(L"reading %s\n",pszPathName);
    if (!saAudioDocRdr->Initialize(pszPathName, VARIANT_TRUE))
    {
        // TODO: Display a more relevant error message.
        pApp->ErrorMessage(IDS_ERROR_NODATA, pszPathName);
        saAudioDocRdr->Close();
        saAudioDocRdr->Release();
        saAudioDocRdr = NULL;
        CoUninitialize();
        EndWaitCursor();
        return FALSE;
    }

    InsertTranscription(PHONETIC, saAudioDocRdr, dwPos);
    InsertTranscription(PHONEMIC, saAudioDocRdr, dwPos);
    InsertTranscription(TONE, saAudioDocRdr, dwPos);
    InsertTranscription(ORTHO, saAudioDocRdr, dwPos);
    InsertGlossPosRefTranscription(saAudioDocRdr, dwPos);
    InsertTranscription(MUSIC_PL1, saAudioDocRdr, dwPos);
    InsertTranscription(MUSIC_PL2, saAudioDocRdr, dwPos);
    InsertTranscription(MUSIC_PL3, saAudioDocRdr, dwPos);
    InsertTranscription(MUSIC_PL4, saAudioDocRdr, dwPos);

    // deinitialize com object
    saAudioDocRdr->Close();
    saAudioDocRdr->Release();
    saAudioDocRdr = NULL;
    CoUninitialize();

    EndWaitCursor();

    return TRUE;
}

/***************************************************************************/
// CSaDoc::InsertTranscription  Insert transcription from another WAV file
// into the current document (except Gloss, POS and Ref)
/***************************************************************************/
BOOL CSaDoc::InsertTranscription(int transType, ISaAudioDocumentReaderPtr saAudioDocRdr, DWORD dwPos)
{

    CSegment * pSegment = GetSegment(transType);

    DWORD offset = 0;
    DWORD length = 0;
    BSTR * annotation = (BSTR *)calloc(1, sizeof(long));

    // which segment includes the insertion position?
    int nIndex = 0;
    if ((dwPos > 0)&&(!pSegment->IsEmpty()))
    {
        while ((nIndex!=-1)&&(pSegment->GetOffset(nIndex) <= dwPos))
        {
            nIndex = pSegment->GetNext(nIndex);
        }
        if (nIndex==-1)
        {
            nIndex = pSegment->GetOffsetSize();
        }
    }

    TRACE("beginning insertion at %d\n",nIndex);
    while (saAudioDocRdr->ReadSegment((long)transType, &offset, &length, annotation))
    {
        CSaString szString = *annotation;
        offset /= m_FmtParm.wChannels;
        length /= m_FmtParm.wChannels;
        pSegment->Insert(nIndex++, szString, false, offset+dwPos, length);
    }

    free(annotation);

    // apply input filter to transcription data
    if (pSegment->GetInputFilter())
    {
        BOOL bChanged = (pSegment->GetInputFilter())(*pSegment->GetString());
        if (bChanged)
        {
            SetModifiedFlag(TRUE);
            SetTransModifiedFlag(TRUE); // transcription data has been modified
        }
    }

    return TRUE;
}

/***************************************************************************/
// CSaDoc::InsertPosTranscription  Insert gloss, POS and Ref
// transcriptions from another WAV file into current document
/***************************************************************************/
void CSaDoc::InsertGlossPosRefTranscription(ISaAudioDocumentReaderPtr saAudioDocRdr, DWORD dwPos)
{

    CGlossSegment * pGloss = (CGlossSegment *)m_apSegments[GLOSS];

    // which segment includes the insertion position?
    int nIndex = 0;
    if ((dwPos > 0)&&!pGloss->IsEmpty())
    {
        while ((nIndex!=-1)&&(pGloss->GetOffset(nIndex) <= dwPos))
        {
            nIndex = pGloss->GetNext(nIndex);
        }

        if (nIndex==-1)
        {
            nIndex = pGloss->GetOffsetSize();
        }
    }

    DWORD offset = 0;
    DWORD length = 0;
    BSTR * gloss = (BSTR *)calloc(1, sizeof(long));
    BSTR * pos = (BSTR *)calloc(1, sizeof(long));
    BSTR * ref = (BSTR *)calloc(1, sizeof(long));
    VARIANT_BOOL isBookmark;
    while (saAudioDocRdr->ReadMarkSegment(&offset, &length, gloss, pos, ref, &isBookmark))
    {
        offset /= m_FmtParm.wChannels;
        length /= m_FmtParm.wChannels;

        CSaString szGloss = *gloss;
        pGloss->Insert(nIndex, szGloss, (isBookmark!=0), offset + dwPos, length);
        pGloss->POSInsertAt( nIndex, (CSaString)*pos);
        CSaString szRef = *ref;
        if (szRef.GetLength())
        {
            m_apSegments[REFERENCE]->Insert(nIndex++, szRef, 0, offset + dwPos, length);
        }
    }

    free(gloss);
    free(pos);
    free(ref);
}

/***************************************************************************/
// CSaDoc::GetWaveFormatParams Checks basic format of a WAV file and populates
// CFmtParm
// silent if this is true, do not display an error on NON-PCM files as they
//        will be retried for conversion at a later point.
/***************************************************************************/
bool CSaDoc::GetWaveFormatParams(LPCTSTR pszPathName,
                                 CFmtParm & fmtParm,
                                 DWORD & dwDataSize)
{

    dwDataSize = 0;
    CSaApp * pApp = (CSaApp *)AfxGetApp();
    // open file
    HMMIO hmmioFile; // file handle
    hmmioFile = mmioOpen(const_cast<TCHAR *>(pszPathName), NULL, MMIO_READ | MMIO_DENYWRITE);
    if (!hmmioFile)
    {
        pApp->ErrorMessage(IDS_ERROR_FILEOPEN, pszPathName);
        EndWaitCursor();
        return false;
    }
    // locate a 'RIFF' chunk with a 'WAVE' form type to make sure it's a WAVE file.
    MMCKINFO mmckinfoParent;
    mmckinfoParent.fccType = mmioFOURCC('W', 'A', 'V', 'E'); // prepare search code
    if (mmioDescend(hmmioFile, (LPMMCKINFO)&mmckinfoParent, NULL, MMIO_FINDRIFF))
    {
        mmioClose(hmmioFile, 0);
        // error descending into wave chunk
        pApp->ErrorMessage(IDS_ERROR_WAVECHUNK, pszPathName);
        EndWaitCursor();
        return false;
    }
    // find the format chunk. It should be a subchunk of the 'RIFF' parent chunk
    MMCKINFO mmckinfoSubchunk;
    mmckinfoSubchunk.ckid = mmioFOURCC('f', 'm', 't', ' '); // prepare search code
    LONG lError;
    if (!mmioDescend(hmmioFile, &mmckinfoSubchunk, &mmckinfoParent, MMIO_FINDCHUNK))
    {
        // fmt chunk found
        lError = mmioRead(hmmioFile, (HPSTR)&fmtParm.wTag, sizeof(WORD)); // read format tag
        if (fmtParm.wTag != WAVE_FORMAT_PCM)   // check if PCM format
        {
            mmioClose(hmmioFile, 0);
            // error testing pcm format
            pApp->ErrorMessage(IDS_ERROR_FORMATPCM, pszPathName);
            EndWaitCursor();
            return false;
        }
        if (lError != -1)
        {
            // read channel number
            lError = mmioRead(hmmioFile, (HPSTR)&fmtParm.wChannels, sizeof(WORD));
        }
        if (lError != -1)
        {
            // read sampling rate
            lError = mmioRead(hmmioFile, (HPSTR)&fmtParm.dwSamplesPerSec, sizeof(DWORD));
        }
        if (lError != -1)
        {
            // read throughput
            lError = mmioRead(hmmioFile, (HPSTR)&fmtParm.dwAvgBytesPerSec, sizeof(DWORD));
        }
        if (lError != -1)
        {
            // read sampling rate for all channels
            lError = mmioRead(hmmioFile, (HPSTR)&fmtParm.wBlockAlign, sizeof(WORD));
        }
        if (lError != -1)
        {
            // read sample word size
            lError = mmioRead(hmmioFile, (HPSTR)&fmtParm.wBitsPerSample, sizeof(WORD));
        }

        // get out of 'fmt ' chunk
        mmioAscend(hmmioFile, &mmckinfoSubchunk, 0);
        if (lError == -1)
        {
            // error reading format chunk
            pApp->ErrorMessage(IDS_ERROR_READFORMAT, pszPathName);
            mmioClose(hmmioFile, 0);
            EndWaitCursor();
            return false;
        }
    }
    else
    {
        // error finding format chunk
        pApp->ErrorMessage(IDS_ERROR_FORMATCHUNK, pszPathName);
        mmioClose(hmmioFile, 0);
        EndWaitCursor();
        return false;
    }
    // determine how much sound data is in the file. Find the data subchunk
    mmckinfoSubchunk.ckid = mmioFOURCC('d', 'a', 't', 'a');
    if (mmioDescend(hmmioFile, &mmckinfoSubchunk, &mmckinfoParent, MMIO_FINDCHUNK))
    {
        // error finding data chunk
        pApp->ErrorMessage(IDS_ERROR_DATACHUNK, pszPathName);
        mmioClose(hmmioFile, 0);
        EndWaitCursor();
        return false;
    }
    // get the size of the data subchunk
    dwDataSize = mmckinfoSubchunk.cksize;
    if (dwDataSize == 0L)
    {
        // error 0 size data chunk
        pApp->ErrorMessage(IDS_ERROR_NODATA, pszPathName);
        mmioClose(hmmioFile, 0);
        EndWaitCursor();
        return false;
    }
    // ascend out of the data chunk
    mmioAscend(hmmioFile, &mmckinfoSubchunk, 0);

    // close the wave file
    mmioClose(hmmioFile, 0);

    return true;
}

/***************************************************************************
* CSaDoc::IsStandardWaveFormat
* Checks basic format of a WAV file and populates CFmtParm.
* Returns false if the file is not a wave file, a non-PCM file, or is not
* in the standard PCM 16/8 bit 22k format that SA uses.
* No errors will be displayed.  Other code will attempt to read the file
* and convert it.
***************************************************************************/
bool CSaDoc::IsStandardWaveFormat(LPCTSTR pszPathName)
{

    // open file
    HMMIO hmmioFile = mmioOpen(const_cast<TCHAR *>(pszPathName), NULL, MMIO_READ | MMIO_DENYWRITE);
    if (!hmmioFile)
    {
        return false;
    }

    // locate a 'RIFF' chunk with a 'WAVE' form type to make sure it's a WAVE file.
    MMCKINFO mmckinfoParent;
    mmckinfoParent.fccType = mmioFOURCC('W', 'A', 'V', 'E'); // prepare search code
    if (mmioDescend(hmmioFile, (LPMMCKINFO)&mmckinfoParent, NULL, MMIO_FINDRIFF))
    {
        mmioClose(hmmioFile, 0);
        return false;
    }

    CFmtParm fmtParm;
    // find the format chunk. It should be a subchunk of the 'RIFF' parent chunk
    MMCKINFO mmckinfoSubchunk;
    // prepare search code
    mmckinfoSubchunk.ckid = mmioFOURCC('f', 'm', 't', ' ');
    LONG lError = mmioDescend(hmmioFile, &mmckinfoSubchunk, &mmckinfoParent, MMIO_FINDCHUNK);
    if (lError!=MMSYSERR_NOERROR)
    {
        mmioClose(hmmioFile, 0);
        return false;
    }

    // fmt chunk found
    // read format tag
    lError = mmioRead(hmmioFile, (HPSTR)&fmtParm.wTag, sizeof(WORD));
    if (lError == -1)
    {
        mmioClose(hmmioFile, 0);
        return false;
    }

    if (fmtParm.wTag != WAVE_FORMAT_PCM)   // check if PCM format
    {
        mmioClose(hmmioFile, 0);
        return false;
    }

    // read channel number
    lError = mmioRead(hmmioFile, (HPSTR)&fmtParm.wChannels, sizeof(WORD));
    if (lError == -1)
    {
        mmioClose(hmmioFile, 0);
        return false;
    }

    // read sampling rate
    lError = mmioRead(hmmioFile, (HPSTR)&fmtParm.dwSamplesPerSec, sizeof(DWORD));
    if (lError == -1)
    {
        mmioClose(hmmioFile, 0);
        return false;
    }

    // read throughput
    lError = mmioRead(hmmioFile, (HPSTR)&fmtParm.dwAvgBytesPerSec, sizeof(DWORD));
    if (lError == -1)
    {
        mmioClose(hmmioFile, 0);
        return false;
    }

    // read sampling rate for all channels
    lError = mmioRead(hmmioFile, (HPSTR)&fmtParm.wBlockAlign, sizeof(WORD));
    if (lError == -1)
    {
        mmioClose(hmmioFile, 0);
        return false;
    }

    // read sample word size
    lError = mmioRead(hmmioFile, (HPSTR)&fmtParm.wBitsPerSample, sizeof(WORD));
    if (lError == -1)
    {
        mmioClose(hmmioFile, 0);
        return false;
    }

    // if it's not in the expected 8/16 bit format, we will use the conversion routine
    if ((fmtParm.wBitsPerSample != 16) && (fmtParm.wBitsPerSample != 8))
    {
        mmioClose(hmmioFile, 0);
        return false;
    }

    // get out of 'fmt ' chunk
    lError = mmioAscend(hmmioFile, &mmckinfoSubchunk, 0);
    if (lError != MMSYSERR_NOERROR)
    {
        mmioClose(hmmioFile, 0);
        return false;
    }

    // determine how much sound data is in the file. Find the data subchunk
    mmckinfoSubchunk.ckid = mmioFOURCC('d', 'a', 't', 'a');
    lError = mmioDescend(hmmioFile, &mmckinfoSubchunk, &mmckinfoParent, MMIO_FINDCHUNK);
    if (lError != MMSYSERR_NOERROR)
    {
        mmioClose(hmmioFile, 0);
        return false;
    }

    // get the size of the data subchunk
    DWORD dwDataSize = mmckinfoSubchunk.cksize;
    if (dwDataSize == 0L)
    {
        mmioClose(hmmioFile, 0);
        return false;
    }

    // ascend out of the data chunk
    mmioAscend(hmmioFile, &mmckinfoSubchunk, 0);

    // close the wave file
    mmioClose(hmmioFile, 0);

    return true;
}

/***************************************************************************
* CSaDoc::IsMultiChannelWave
* Returns true if this is a standard wave file and it contains multi channel
* data.
* This function assumes IsStandardWaveFormat was called first
***************************************************************************/
bool CSaDoc::IsMultiChannelWave(LPCTSTR pszPathName, int & channels)
{
    channels = 0;

    // open file
    HMMIO hmmioFile = mmioOpen(const_cast<TCHAR *>(pszPathName), NULL, MMIO_READ | MMIO_DENYWRITE);
    if (!hmmioFile)
    {
        return false;
    }

    // locate a 'RIFF' chunk with a 'WAVE' form type to make sure it's a WAVE file.
    MMCKINFO mmckinfoParent;
    mmckinfoParent.fccType = mmioFOURCC('W', 'A', 'V', 'E'); // prepare search code
    if (mmioDescend(hmmioFile, (LPMMCKINFO)&mmckinfoParent, NULL, MMIO_FINDRIFF))
    {
        mmioClose(hmmioFile, 0);
        return false;
    }

    CFmtParm fmtParm;
    // find the format chunk. It should be a subchunk of the 'RIFF' parent chunk
    MMCKINFO mmckinfoSubchunk;
    mmckinfoSubchunk.ckid = mmioFOURCC('f', 'm', 't', ' '); // prepare search code
    LONG lError = mmioDescend(hmmioFile, &mmckinfoSubchunk, &mmckinfoParent, MMIO_FINDCHUNK);
    if (lError!=MMSYSERR_NOERROR)
    {
        mmioClose(hmmioFile, 0);
        return false;
    }

    // fmt chunk found
    lError = mmioRead(hmmioFile, (HPSTR)&fmtParm.wTag, sizeof(WORD)); // read format tag
    if (lError == -1)
    {
        mmioClose(hmmioFile, 0);
        return false;
    }

    if (fmtParm.wTag != WAVE_FORMAT_PCM)   // check if PCM format
    {
        mmioClose(hmmioFile, 0);
        return false;
    }

    lError = mmioRead(hmmioFile, (HPSTR)&fmtParm.wChannels, sizeof(WORD)); // read channel number
    if (lError == -1)
    {
        mmioClose(hmmioFile, 0);
        return false;
    }

    // if it's multi channel, we will convert the file
    channels = fmtParm.wChannels;
    if (fmtParm.wChannels>1)
    {
        mmioClose(hmmioFile, 0);
        return true;
    }

    mmioClose(hmmioFile, 0);

    return false;
}

/***************************************************************************/
// CSaDoc::CheckWaveFormatForPaste Checks format of a WAV file for insertion into
// document.
/***************************************************************************/
DWORD CSaDoc::CheckWaveFormatForPaste(LPCTSTR pszPathName)
{

    CFmtParm fmtParm;
    DWORD dwDataSize;

    if (!GetWaveFormatParams(pszPathName, fmtParm, dwDataSize))
    {
        // there is an error in the file.
        // an error message will have already been displayed
        return 0;
    }

    CSaApp * pApp = (CSaApp *)AfxGetApp();

    if (dwDataSize)
    {
        CFmtParm * pFmtParm = &fmtParm;
        if ((m_FmtParm.wChannels != pFmtParm->wChannels) ||
                (m_FmtParm.dwSamplesPerSec != pFmtParm->dwSamplesPerSec) ||
                (m_FmtParm.dwAvgBytesPerSec != pFmtParm->dwAvgBytesPerSec) ||
                (m_FmtParm.wBlockAlign != pFmtParm->wBlockAlign) ||
                (m_FmtParm.wBitsPerSample != pFmtParm->wBitsPerSample))
        {
            // not the right format
            pApp->ErrorMessage(IDS_ERROR_PASTEFORMAT);
            return FALSE;
        }
    }

    return dwDataSize; // OK return data chunk length
}

/***************************************************************************/
// CSaDoc::CheckWaveFormatForOpen Checks format of a WAV file for opening
/***************************************************************************/
DWORD CSaDoc::CheckWaveFormatForOpen(LPCTSTR pszPathName)
{

    CFmtParm fmtParm;
    DWORD dwDataSize = 0;
    if (!GetWaveFormatParams(pszPathName, fmtParm, dwDataSize))
    {
        // there is an error in the file.
        // an error message will have already been displayed
        return 0;
    }

    CSaApp * pApp = (CSaApp *)AfxGetApp();

    if (dwDataSize)
    {
        if (fmtParm.dwSamplesPerSec < 1)
        {
            // not the right format
            pApp->ErrorMessage(IDS_ERROR_INVALID_SAMPLING_RATE, pszPathName);
            return 0;
        }
    }

    return dwDataSize; // OK return data chunk length
}

/***************************************************************************/
// CSaDoc::ConvertToWave Converts file to 22kHz, 16bit, Mono WAV format
/***************************************************************************/
bool CSaDoc::ConvertToWave(LPCTSTR pszPathName)
{

    bool result = true;
    CSaApp * pApp = (CSaApp *)AfxGetApp();

    // display status bar message
    CMainFrame * pMainFrame = (CMainFrame *)AfxGetMainWnd();
    CProgressStatusBar * pStatusBar = (CProgressStatusBar *)pMainFrame->GetProgressStatusBar();
    if (!pStatusBar->GetProcessOwner())
    {
        pMainFrame->ShowDataStatusBar(FALSE); // show the progress status bar
    }
    CString szText;
    szText.LoadString(IDS_STATTXT_PROCESSRAW);
    pStatusBar->SetPaneText(ID_PROGRESSPANE_1, szText);
    pStatusBar->SetPaneText(ID_PROGRESSPANE_3, _T(""));

    // create temp file
    TCHAR szTempFilePath[_MAX_PATH];
    GetTempFileName(_T("WAV"), szTempFilePath, _countof(szTempFilePath));
    m_szTempConvertedWave = szTempFilePath;

    // if it's a wave file, but in a different format then try and convert it
    // if this errors, we will just continue on trying with ST_Audio
    {
        CWaveResampler resampler;
        CWaveResampler::ECONVERT result = resampler.Run(pszPathName, szTempFilePath, pStatusBar);
        if (result==CWaveResampler::EC_SUCCESS)
        {
            pMainFrame->ShowDataStatusBar(TRUE); // restore data status bar
            return true;
        }
        else if (result==CWaveResampler::EC_USERABORT)
        {
            // the user chose to quite
            pMainFrame->ShowDataStatusBar(TRUE); // restore data status bar
            return false;
        }
    }

    // convert to wave
    CoInitialize(NULL);
    ISTAudioPtr stAudio;
    HRESULT createResult = stAudio.CreateInstance(__uuidof(STAudio));
    if (createResult)
    {
        CSaString szCreateResult;
        szCreateResult.Format(_T("%x"), createResult);
        pApp->ErrorMessage(IDS_ERROR_CREATE_INSTANCE, _T("STAudio.CreateInstance()"), szCreateResult);
        pMainFrame->ShowDataStatusBar(TRUE); // restore data status bar
        return false;
    }
    pStatusBar->SetProgress(30);
    try
    {
        result = (stAudio->ConvertToWAV(_bstr_t(pszPathName), _bstr_t(szTempFilePath), 22050, 16, 1)==VARIANT_TRUE);
    }
    catch (...)
    {
        pApp->ErrorMessage(IDS_ERROR_FORMATPCM, pszPathName);
        pMainFrame->ShowDataStatusBar(TRUE); // restore data status bar
        return false;
    }
    pStatusBar->SetProgress(90);
    stAudio->Release();
    stAudio = NULL;
    CoUninitialize();

    pMainFrame->ShowDataStatusBar(TRUE); // restore data status bar

    return result;
}

// SDM 1.06.6U2 split function
/***************************************************************************/
// CSaDoc::OnSaveDocument Saving a document
// Override for CDocument::OnSaveDocument()
/***************************************************************************/
BOOL CSaDoc::OnSaveDocument(LPCTSTR pszPathName)
{
    return OnSaveDocument(pszPathName, TRUE);
}

/***************************************************************************/
// CSaDoc::OnSaveDocument Saving a document
// Stores all available information from data members in the RIFF header of
// the document (wave file). The fmt and data chunks have to be there al-
// ready! The temporary wave file data will be copied into the wave chunk.
// If the string m_szTempWave is not empty, the wave data has first to be
// copied (renamed) from its current place given in the string. This temp
// file from the recorder contains the RIFF structure with the fmt and the
// data chunks. After the copying, the file has to be saved in the normal way.
/***************************************************************************/
BOOL CSaDoc::OnSaveDocument( LPCTSTR pszPathName, BOOL bSaveAudio)
{

    std::wstring target = pszPathName;

    CSaApp * pApp = (CSaApp *)AfxGetApp(); // get pointer to application
    BeginWaitCursor(); // wait cursor
    if (!m_szTempWave.IsEmpty())
    {
        // check if the file already opened
		if (pApp->IsFileOpened(target.c_str()))
        {
            // error file already opened by SA
            pApp->ErrorMessage(IDS_ERROR_FILEOPENED, target.c_str());
            EndWaitCursor();
            return FALSE;
        }
        // temporary wave file to rename
        CFileStatus status;
		// check if file exists already
        if (CFile::GetStatus(target.c_str(), status) != 0)   
        {
            // file does exist already, be sure to allow writing and delete it
            RemoveFile(target.c_str());
        }
        // check if a copy is needed
        if (m_szTempWave[0] != target.c_str()[0])
        {
            // different drives, copy the file
            if (!CopyWave( m_szTempWave, target.c_str()))
            {
                // error copying file
                pApp->ErrorMessage(IDS_ERROR_FILEWRITE, target.c_str());
                EndWaitCursor();
                return FALSE;
            }
            // now delete the old file (source)
            RemoveFile(m_szTempWave);
        }
        else     // rename the file
        {
            try
            {
                CFile::Rename(m_szTempWave, target.c_str());
            }
            catch (CFileException e)
            {
                // error renaming file
                pApp->ErrorMessage(IDS_ERROR_FILEWRITE, target.c_str());
                EndWaitCursor();
                return FALSE;
            }
        }
        m_szTempWave.Empty(); // empty the new file name string
    }
    else
    {
        // we are dealing with a normal file.
        // does the file still exist?
        if (!::FileExists(target.c_str()))
        {

            CSaString fileName = GetPathName();
            if (fileName.IsEmpty())
            {
                fileName = GetFilenameFromTitle().c_str(); // get the current view caption string
            }

            CSaString fileExt = _T(".wav");
            fileName = SetFileExtension(fileName, fileExt);

            CDlgSaveAsOptions dlg(_T("wav"), fileName, OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT, _T("WAV Files (*.wav)|*.wav||"));

            CSaString szDefault = static_cast<CSaApp *>(AfxGetApp())->DefaultDir(); // need to save copy (return value is destroyed)
            dlg.m_ofn.lpstrInitialDir = szDefault;

            if (dlg.DoModal()!=IDOK)
            {
                return FALSE;
            }

            fileName = dlg.GetPathName();

            BOOL bSameFileName = FALSE;
            if (fileName == GetPathName())
            {
                bSameFileName = TRUE;
                dlg.m_nShowFiles = CDlgSaveAsOptions::showNew; // There is only one file.
            }
            else if (static_cast<CSaApp *>(AfxGetApp())->IsFileOpened(fileName))
            {
                // error file already opened by SA
                static_cast<CSaApp *>(AfxGetApp())->ErrorMessage(IDS_ERROR_FILEOPENED, fileName);
                return FALSE;
            }

            CFileStatus status;
            if (CFile::GetStatus(fileName, status))
            {
                // File exists overwrite existing file
                try
                {
                    CFile::SetStatus(fileName, status);
                }
                catch (...)
                {
                    ((CSaApp *) AfxGetApp())->ErrorMessage(IDS_ERROR_FILEWRITE, fileName);
                    return FALSE;
                }
            }

            target = fileName;
        }
    }

    DeleteWaveFromUndo(); // delete wave undo entry
    if (!WriteDataFiles(target.c_str(), bSaveAudio))
    {
        return FALSE;
    }

    // get file information
    CFile::GetStatus(target.c_str(), m_fileStat);
    EndWaitCursor();
    SetModifiedFlag(FALSE);
    SetTransModifiedFlag(FALSE);
    SetAudioModifiedFlag(FALSE);

    // if batch mode, set file in changed state
    if (pApp->GetBatchMode() != 0)
    {
        pApp->SetBatchFileChanged(target.c_str(), m_ID, this); // set changed state
    }
    return TRUE;
}

/***************************************************************************/
// CSaDoc::WriteDataFiles Saving a document
// Stores all available information from data members in the RIFF header of
// the document (wave file). The fmt and data chunks have to be there al-
// ready! The temporary wave file data will be copied into the wave chunk.
/***************************************************************************/
BOOL CSaDoc::WriteDataFiles( LPCTSTR pszPathName, BOOL bSaveAudio/*=TRUE*/, BOOL bIsClipboardFile/*=FALSE*/)
{

    CSaApp * pApp = (CSaApp *)AfxGetApp();

    BeginWaitCursor(); // wait cursor

    DWORD dwDataSize = m_dwDataSize;
    if (bSaveAudio)
    {
        dwDataSize = WriteRiff(pszPathName);
        if (dwDataSize == 0)
        {
            return FALSE;
        }
    }

    CoInitialize(NULL);
    ISaAudioDocumentWriterPtr saAudioDocWriter;
    HRESULT createResult = saAudioDocWriter.CreateInstance(__uuidof(SaAudioDocumentWriter));
    if (createResult)
    {
        CSaString szCreateResult;
        szCreateResult.Format(_T("%x"), createResult);
        pApp->ErrorMessage(IDS_ERROR_CREATE_INSTANCE, _T("SaAudioDocumentWriter.CreateInstance()"), szCreateResult);
        return FALSE;
    }

    _bstr_t szMD5HashCode = (!m_szMD5HashCode ? (wchar_t *)0 : m_szMD5HashCode);

    if (bIsClipboardFile)
    {
        szMD5HashCode = (wchar_t *)0;
    }

    if (!saAudioDocWriter->Initialize(pszPathName, szMD5HashCode, (short)bIsClipboardFile))
    {
        CSaApp * pApp = (CSaApp *)AfxGetApp(); // get pointer to application
        pApp->ErrorMessage(IDS_ERROR_WRITEPHONETIC, pszPathName);
        EndWaitCursor();
        saAudioDocWriter->Close();
        saAudioDocWriter->Release();
        saAudioDocWriter = NULL;
        CoUninitialize();
        return FALSE;
    }

    WriteNonSegmentData(dwDataSize, saAudioDocWriter);
    saAudioDocWriter->DeleteSegments();
    WriteTranscription(PHONETIC, saAudioDocWriter);
    WriteTranscription(PHONEMIC, saAudioDocWriter);
    WriteTranscription(TONE, saAudioDocWriter);
    WriteTranscription(ORTHO, saAudioDocWriter);
    WriteGlossPosAndRefSegments(saAudioDocWriter);
    WriteScoreData(saAudioDocWriter);
    WriteTranscription(MUSIC_PL1, saAudioDocWriter);
    WriteTranscription(MUSIC_PL2, saAudioDocWriter);
    WriteTranscription(MUSIC_PL3, saAudioDocWriter);
    WriteTranscription(MUSIC_PL4, saAudioDocWriter);

    saAudioDocWriter->Commit();
    saAudioDocWriter->Close();
    saAudioDocWriter->Release();
    saAudioDocWriter = NULL;
    CoUninitialize();

    EndWaitCursor();
    return TRUE;
}

/***************************************************************************/
// CSaDoc::WriteRiff - The fmt and data chunks have to be there already!
// The temporary wave file data will be copied into the wave chunk.
/***************************************************************************/
DWORD CSaDoc::WriteRiff(LPCTSTR pszPathName)
{
    CSaApp * pApp = (CSaApp *)AfxGetApp(); // get pointer to application

    BeginWaitCursor(); // wait cursor

    // open file
    HMMIO hmmioFile; // file handle
    hmmioFile = mmioOpen(const_cast<TCHAR *>(pszPathName), NULL, MMIO_READWRITE | MMIO_EXCLUSIVE);
    if (!hmmioFile)
    {
        // error opening file
        pApp->ErrorMessage(IDS_ERROR_FILEOPEN, pszPathName);
        EndWaitCursor();
        return 0;
    }

    // locate a 'RIFF' chunk with a 'WAVE' form type to make sure it's a WAVE file.
    MMCKINFO mmckinfoParent;
    mmckinfoParent.fccType = mmioFOURCC('W', 'A', 'V', 'E'); // prepare search code
    if (mmioDescend(hmmioFile, (LPMMCKINFO)&mmckinfoParent, NULL, MMIO_FINDRIFF))
    {
        // error descending into wave chunk
        pApp->ErrorMessage(IDS_ERROR_WAVECHUNK, pszPathName);
        mmioClose(hmmioFile, 0);
        EndWaitCursor();
        return 0;
    }

    // find the data chunk. It should be a subchunk of the 'RIFF' parent chunk
    MMCKINFO mmckinfoSubchunk;
    LONG lError = 0;
    DWORD dwDataSize = 0;
    mmckinfoSubchunk.ckid = mmioFOURCC('d', 'a', 't', 'a');
    if (lError != -1)
    {
        lError = mmioDescend(hmmioFile, &mmckinfoSubchunk, &mmckinfoParent, MMIO_FINDCHUNK);
    }

    if (lError != -1)                           // copy the temporary wave file into the wave chunk
    {
        mmckinfoSubchunk.dwFlags = MMIO_DIRTY;  // force ascend to update the chunk length
        mmckinfoParent.dwFlags = MMIO_DIRTY;

        // open the temporary wave file
        CopyProcessTempFile(); // copy the Adjust or Workbench temp file if necessary

        CFile file;
        if (!file.Open(m_szRawDataWrk.c_str(), CFile::modeRead))
        {
            // error opening file
            pApp->ErrorMessage(IDS_ERROR_OPENTEMPFILE, m_szRawDataWrk.c_str());
            mmioClose(hmmioFile, 0);
            EndWaitCursor();
            return 0;
        }

        // use a local buffer
        char buffer[0x10000];

        // start copying wave data
        while (TRUE)
        {
            // read the waveform data block
            DWORD dwSizeRead = 0;
            try
            {
                dwSizeRead = file.Read(buffer, _countof(buffer));
            }
            catch (CFileException e)
            {
                // error reading file
                pApp->ErrorMessage(IDS_ERROR_READTEMPFILE, m_szRawDataWrk.c_str());
                mmioClose(hmmioFile, 0);
                EndWaitCursor();
                return 0;
            }

            if (dwSizeRead==0)
            {
                break;    // all read
            }

            if (mmioWrite(hmmioFile, buffer, dwSizeRead) == -1)
            {
                // error writing wave file
                pApp->ErrorMessage(IDS_ERROR_WRITEDATACHUNK, pszPathName);
                mmioClose(hmmioFile, 0);
                EndWaitCursor();
                return 0;
            }
        }

        // get out of 'data' chunk
        if (mmioAscend(hmmioFile, &mmckinfoSubchunk, 0))
        {
            // error writing data chunk
            pApp->ErrorMessage(IDS_ERROR_WRITEDATACHUNK, pszPathName);
            mmioClose(hmmioFile, 0);
            EndWaitCursor();
            return 0;
        }

        dwDataSize = mmckinfoSubchunk.cksize;
    }

    // get out of 'RIFF' chunk, to write RIFF size
    if (mmioAscend(hmmioFile, &mmckinfoParent, 0))
    {
        // error writing RIFF chunk
        pApp->ErrorMessage(IDS_ERROR_WRITERIFFCHUNK, pszPathName);
        mmioClose(hmmioFile, 0);
        EndWaitCursor();
        return 0;
    }

    if (!mmioClose(hmmioFile, 0))   // close file
    {
        // Set File Length ...
        // mmioAscend() does not set mmioinfo.lDiskOffset correctly under
        // certain conditions, so use the RIFF chunk size + 8 instead
        CFile WaveFile(pszPathName,CFile::modeReadWrite);
        DWORD dwPadBytes = mmckinfoParent.cksize - 2 * (mmckinfoParent.cksize / 2);
        DWORD dwFileSize = mmckinfoParent.cksize + 8 + dwPadBytes;
        WaveFile.SetLength(dwFileSize);
    }

    EndWaitCursor();

    return dwDataSize;
}

/***************************************************************************/
// CSaDoc::WriteTranscription  Write the transcription from the document
// to the transcription database.
/***************************************************************************/
void CSaDoc::WriteNonSegmentData(DWORD dwDataSize, ISaAudioDocumentWriterPtr saAudioDocWriter)
{
    saAudioDocWriter->DataChunkSize = dwDataSize;
    saAudioDocWriter->FormatTag = m_FmtParm.wTag;
    saAudioDocWriter->Channels =m_FmtParm.wChannels;
    saAudioDocWriter->SamplesPerSecond = m_FmtParm.dwSamplesPerSec;
    saAudioDocWriter->AverageBytesPerSecond = m_FmtParm.dwAvgBytesPerSec;
    saAudioDocWriter->BlockAlignment = m_FmtParm.wBlockAlign;
    saAudioDocWriter->BitsPerSample = m_FmtParm.wBitsPerSample;
    saAudioDocWriter->PutSADescription((_bstr_t)m_saParm.szDescription);
    saAudioDocWriter->SAFlags = m_saParm.wFlags;
    saAudioDocWriter->RecordFileFormat = m_saParm.byRecordFileFormat;
    saAudioDocWriter->RecordBandWidth = m_saParm.dwRecordBandWidth;
    saAudioDocWriter->RecordSampleSize = m_saParm.byRecordSmpSize;
    saAudioDocWriter->RecordTimeStamp = m_saParm.RecordTimeStamp.GetTime();
    DWORD dwSingleChannelDataSize = dwDataSize/m_FmtParm.wChannels;
    saAudioDocWriter->NumberOfSamples = dwSingleChannelDataSize / m_FmtParm.wBlockAlign;
    saAudioDocWriter->SignalMax = m_saParm.lSignalMax;
    saAudioDocWriter->SignalMin = m_saParm.lSignalMin;
    saAudioDocWriter->SignalBandWidth = m_saParm.dwSignalBandWidth;
    saAudioDocWriter->SignalEffSampSize = m_saParm.byQuantization;

    saAudioDocWriter->CalcFreqLow = m_uttParm.nMinFreq;
    saAudioDocWriter->CalcFreqHigh = m_uttParm.nMaxFreq;
    saAudioDocWriter->CalcIntrpGap = m_uttParm.nMaxInterp;
    saAudioDocWriter->CalcPercntChng = m_uttParm.nMaxChange;
    saAudioDocWriter->CalcGrpSize = m_uttParm.nMinGroup;
    saAudioDocWriter->CalcVoicingThd = m_uttParm.nCritLoud;

    // Write speaker information
    saAudioDocWriter->PutSpeakerName((_bstr_t)m_sourceParm.szSpeaker);
    switch (m_sourceParm.nGender)
    {
    case 0:
        saAudioDocWriter->SpeakerGender = 'M';
        break;
    case 1:
        saAudioDocWriter->SpeakerGender = 'F';
        break;
    case 2:
        saAudioDocWriter->SpeakerGender = 'C';
        break;
    default:
        saAudioDocWriter->SpeakerGender = ' ';
    }

    // Write language information
    saAudioDocWriter->PutEthnologueId((_bstr_t)m_sourceParm.szEthnoID);
    saAudioDocWriter->PutRegion((_bstr_t)m_sourceParm.szRegion);
    saAudioDocWriter->PutCountry((_bstr_t)m_sourceParm.szCountry);
    saAudioDocWriter->PutFamily((_bstr_t)m_sourceParm.szFamily);
    saAudioDocWriter->PutLanguageName((_bstr_t)m_sourceParm.szLanguage);
    saAudioDocWriter->PutDialect((_bstr_t)m_sourceParm.szDialect);

    // Write reference information
    saAudioDocWriter->PutNoteBookReference((_bstr_t)m_sourceParm.szReference);
    saAudioDocWriter->PutFreeFormTranslation((_bstr_t)m_sourceParm.szFreeTranslation);
    saAudioDocWriter->PutTranscriber((_bstr_t)m_sourceParm.szTranscriber);
}

/***************************************************************************/
// CSaDoc::WriteTranscription  Write the transcription from the document
// to the transcription database.
/***************************************************************************/
void CSaDoc::WriteTranscription(int transType, ISaAudioDocumentWriterPtr saAudioDocWriter)
{
    CSaString szFullTrans = *m_apSegments[transType]->GetString();
    WORD wTransLength = (WORD)szFullTrans.GetLength();
    if (wTransLength == 0)
    {
        return;
    }

	TRACE("trans length = %d\n",wTransLength);

    CSaString szAnnotation = _T("");
	DWORD dwOffset = m_apSegments[transType]->GetOffset(0) * m_FmtParm.wChannels;
    DWORD dwLength = m_apSegments[transType]->GetDuration(0) * m_FmtParm.wChannels;

    for (int i = 0; i < (int)wTransLength; i++)
    {
        if (dwOffset != m_apSegments[transType]->GetOffset(i) * m_FmtParm.wChannels)
        {
            // Write annotation
            saAudioDocWriter->AddSegment( transType, dwOffset, dwLength, (_bstr_t)szAnnotation);
            szAnnotation.Empty();
            dwOffset = m_apSegments[transType]->GetOffset(i) * m_FmtParm.wChannels;
            dwLength = m_apSegments[transType]->GetDuration(i) * m_FmtParm.wChannels;
        }

        szAnnotation += szFullTrans[i];
    }

    saAudioDocWriter->AddSegment(transType, dwOffset, dwLength, (_bstr_t)szAnnotation);
}

/***************************************************************************/
// CSaDoc::WriteGlossPosAndRefSegments Writes the gloss, part of speech and
// reference information to the database.
/***************************************************************************/
void CSaDoc::WriteGlossPosAndRefSegments(ISaAudioDocumentWriterPtr saAudioDocWriter)
{
    CGlossSegment * pGloss = (CGlossSegment *)m_apSegments[GLOSS];
    CSaString szPos;
    CSaString szRef;
    CSaString szGloss;

    DWORD offset;
    DWORD length;
    int nRef = 0;

    for (int i = 0; i < pGloss->GetTexts().GetSize(); i++)
    {
        offset = pGloss->GetOffset(i);
        length = pGloss->GetDuration(i);

        szGloss = pGloss->GetTexts().GetAt(i);
        szPos = pGloss->GetPOSAt(i);

        CReferenceSegment * pRef = (CReferenceSegment *)m_apSegments[REFERENCE];
        if ((nRef < pRef->GetTexts().GetSize()) && (pRef->GetOffset(nRef) == offset))
        {
            szRef = pRef->GetTexts().GetAt(nRef++);
        }

        VARIANT_BOOL isBookmark = FALSE;

        // Strip off the word boundary or bookmark character.
        if ((szGloss.GetLength() > 0) && 
			((szGloss[0] == WORD_DELIMITER) || (szGloss[0] == TEXT_DELIMITER)))
        {
            isBookmark = (szGloss[0] == TEXT_DELIMITER);
            szGloss = szGloss.Mid(1);
        }

		// the length and offset at this point is for a single channel
		// we need to scale up to match audio data
		offset *= m_FmtParm.wChannels;
		length *= m_FmtParm.wChannels;

        saAudioDocWriter->AddMarkSegment( offset, length,
                                         (szGloss.GetLength() == 0 ? (wchar_t *)0 : (_bstr_t)szGloss),
                                         (szPos.GetLength() == 0 ? (wchar_t *)0 : (_bstr_t)szPos),
                                         (szRef.GetLength() == 0 ? (wchar_t *)0 : (_bstr_t)szRef), 
										 isBookmark);

        szRef.Empty();
    }
}

/***************************************************************************/
// CSaDoc::WriteScoreData Writes the score data from the staff (SAMA)
// to the database.
/***************************************************************************/
void CSaDoc::WriteScoreData(ISaAudioDocumentWriterPtr saAudioDocWriter)
{

    POSITION pos = GetFirstViewPosition();
    CSaView * pView = (CSaView *)GetNextView(pos);
    int nMusicScoreSize = 0;
    int nGraphIndex = pView->GetGraphIndexForIDD(IDD_STAFF);
    CPlotStaff * pMusicStaff = NULL;
    if (nGraphIndex >= 0)
    {
        CGraphWnd * pMusicGraph = pView->GetGraph(nGraphIndex);
        pMusicStaff = (CPlotStaff *)pMusicGraph->GetPlot();
        nMusicScoreSize = pMusicStaff->GetWindowTextLength();
    }

    TCHAR * pMusicScore = new TCHAR[nMusicScoreSize + 1];
    if (nMusicScoreSize)
    {
        if (!pMusicStaff->GetWindowText(pMusicScore, nMusicScoreSize))
        {
            nMusicScoreSize = 0;
        }
    }
    pMusicScore[nMusicScoreSize] = 0;

    saAudioDocWriter->WriteAsMusicXML((_bstr_t)pMusicScore);
    if (pMusicStaff)
    {
        pMusicStaff->SetModifiedFlag(FALSE);
    }
    delete [] pMusicScore;
    SetTransModifiedFlag(FALSE); // transcription data has been modified
}

// SDM 1.06.6U2 added ability to insert wave dat from a file
/***************************************************************************
* CSaDoc::CopyWaveToTemp Copy the wave chunk into a temporary file
* The function creates a temporary file if pszTempPathName points to NULL
* (default), and it copies the wave chunk from the wave file with the
* pathname pszSourcePathName points to to the file (the tempfile or the one
* with the pathname pszPathName points to).
* NOTE This is only used by the auto recorder.
*
* dStart is the starting location where the first audible data portion should
* be located.
* dTotalLength is the expected total length, include dStart.
*
/***************************************************************************/
BOOL CSaDoc::CopyWaveToTemp(LPCTSTR pszSourcePathName, double dStart, double dTotalLength)
{
    TCHAR szTempPath[_MAX_PATH];
    GetTempFileName(_T("WAV"), szTempPath, _countof(szTempPath));
    LPCTSTR pszTempPathName = szTempPath;
	TRACE(L"using %s\n",pszSourcePathName);
	TRACE(L"generating %s\n",szTempPath);

    // get pointer to view and app
    POSITION pos = GetFirstViewPosition();
    CSaView * pView = (CSaView *)GetNextView(pos);
    CSaApp * pApp = (CSaApp *)AfxGetApp();

	ASSERT(dStart<dTotalLength);

    long lSizeWritten = 0;  // this is number of bytes written
    {
        CFile file;
        // create and open or just open the file
        if (!file.Open(pszTempPathName, CFile::modeCreate | CFile::modeReadWrite | CFile::shareExclusive))
        {
            // error opening file
            pApp->ErrorMessage(IDS_ERROR_OPENTEMPFILE, pszTempPathName);
            m_dwDataSize = 0;		// no data available
            SetModifiedFlag(FALSE); // will be unable to save
            pView->SendMessage(WM_COMMAND, ID_FILE_CLOSE, 0L); // close file
            return FALSE;
        }

        // now open wave file and go to wave chunk
        HMMIO hmmioFile = mmioOpen(const_cast<TCHAR *>(pszSourcePathName), NULL, MMIO_READ | MMIO_DENYWRITE);
        if (!hmmioFile)
        {
            // error opening file
            pApp->ErrorMessage(IDS_ERROR_FILEOPEN, pszSourcePathName);
            m_dwDataSize = 0;		// no data available
            SetModifiedFlag(FALSE); // will be unable to save
            pView->SendMessage(WM_COMMAND, ID_FILE_CLOSE, 0L); // close file
            return FALSE;
        }
        // locate a 'RIFF' chunk with a 'WAVE' form type to make sure it's a WAVE file.
        MMCKINFO mmckinfoParent;
        mmckinfoParent.fccType = mmioFOURCC('W', 'A', 'V', 'E'); // prepare search code
        if (mmioDescend(hmmioFile, (LPMMCKINFO)&mmckinfoParent, NULL, MMIO_FINDRIFF))
        {
            // error descending into wave chunk
            pApp->ErrorMessage(IDS_ERROR_WAVECHUNK, m_fileStat.m_szFullName);
            mmioClose(hmmioFile, 0);
            m_dwDataSize = 0;		// no data available
            SetModifiedFlag(FALSE); // will be unable to save
            pView->SendMessage(WM_COMMAND, ID_FILE_CLOSE, 0L); // close file
            return FALSE;
        }
        // find the data subchunk
        MMCKINFO mmckinfoSubchunk;
        mmckinfoSubchunk.ckid = mmioFOURCC('d', 'a', 't', 'a');
        if (mmioDescend(hmmioFile, &mmckinfoSubchunk, &mmckinfoParent, MMIO_FINDCHUNK))
        {
            // error descending into data chunk
            pApp->ErrorMessage(IDS_ERROR_DATACHUNK, m_fileStat.m_szFullName);
            mmioClose(hmmioFile, 0);
            m_dwDataSize = 0;		// no data available
            SetModifiedFlag(FALSE); // will be unable to save
            pView->SendMessage(WM_COMMAND, ID_FILE_CLOSE, 0L); // close file
            return FALSE;
        }

        DWORD dwSmpSize = DWORD(GetSampleSize());

		// first write the front pad
        // number of zero samples to add
        try
        {
            long lFrontPad = ((long)(dStart*m_FmtParm.dwSamplesPerSec)*dwSmpSize)/dwSmpSize;
	        TRACE(_T("front pad samples=%lu\n"), lFrontPad);
            if (dwSmpSize == 1)
            {
                char cData = 0;
                for (long j=0; j<lFrontPad; j++)
                {
                    file.Write(&cData, sizeof(cData));
                    lSizeWritten++;
                }
            }
            else
            {
                short int iData = 0;
                for (long j=0; j<lFrontPad; j++)
                {
                    file.Write(&iData, sizeof(iData));
                    lSizeWritten+=2;
                }
            }
	        TRACE(_T("front pad bytes written=%lu\n"), lSizeWritten);
        }
        catch (CFileException e)
        {
            // error writing file
            pApp->ErrorMessage(IDS_ERROR_WRITETEMPFILE, pszTempPathName);
            m_dwDataSize = 0;		// no data available
            SetModifiedFlag(FALSE); // will be unable to save
            pView->SendMessage(WM_COMMAND, ID_FILE_CLOSE, 0L); // close file
            return FALSE;
        }

        // why are we here?
        // the basic assumption is that the autorecord starts when a level
        // is reached, so there is blank data in the front.
        // We need to carefully trim it

		long lTotalLength = (long)((dTotalLength)*m_FmtParm.dwSamplesPerSec);
		if (dwSmpSize == 2)
		{
			lTotalLength *= 2;
		}
		TRACE("lTotalLength=%lu\n",lTotalLength);

		// get the size of the data subchunk
		DWORD dwSizeLeft = mmckinfoSubchunk.cksize;
		TRACE("dwSizeLeft=%lu\n",dwSizeLeft);

		char buffer[0x10000];
		while (dwSizeLeft>0)
		{
			// read the waveform data block
			long lSizeRead = mmioRead( hmmioFile, buffer, _countof(buffer));
			if (lSizeRead == -1)
			{
				// error reading from data chunk
				pApp->ErrorMessage(IDS_ERROR_READDATACHUNK, m_fileStat.m_szFullName);
				mmioClose(hmmioFile, 0);
				m_dwDataSize = 0;		// no data available
				SetModifiedFlag(FALSE); // will be unable to save
				pView->SendMessage(WM_COMMAND, ID_FILE_CLOSE, 0L); // close file
				return FALSE;
			}

			if ((DWORD)lSizeRead > dwSizeLeft)
			{
				lSizeRead = (long)dwSizeLeft;
			}
			dwSizeLeft -= (DWORD)lSizeRead;

			bool bDone = false;
			long dwDataPos = 0;
			unsigned char * pSource = (unsigned char *)buffer;
			while (dwDataPos < lSizeRead)
			{
				// read data
				LONG nData;
				if (dwSmpSize == 1)
				{
					// 8 bit per sample
					// data range is 0...255 (128 is center)
					unsigned char bData = *pSource++;
					nData = bData - 128;
					nData *= 256;
				}
				else
				{
					// 16 bit data
					nData = *((short int *)pSource++);
					pSource++;
					dwDataPos++;
				}

				// set peak level
				LONG nMaxLevel = abs(nData);
				nMaxLevel = ((LONG)100 * (LONG)nMaxLevel / 32768);
				dwDataPos++;

				// we are about to record our first valid sample
				// first write out the pad
				if (nMaxLevel > MIN_VOICE_LEVEL)
				{
					TRACE("voice found %d\n",dwDataPos);
					// write the data block from the buffer
					try
					{
						// always write one sample short when we are truncating so
						// that it will be zero-filled
						DWORD lWriteSize = lSizeRead - dwDataPos;
						if ((lSizeWritten + lWriteSize + dwSmpSize)>lTotalLength)
						{
							TRACE("truncating data\n");
							lWriteSize = lTotalLength-lSizeWritten-dwSmpSize;
							ASSERT(lWriteSize>=0);
						}
						file.Write(&buffer[dwDataPos], (DWORD)lWriteSize);
						lSizeWritten += lWriteSize;
					}
					catch (CFileException e)
					{
						// error writing file
						pApp->ErrorMessage(IDS_ERROR_WRITETEMPFILE, pszTempPathName);
						m_dwDataSize = 0;		// no data available
						SetModifiedFlag(FALSE); // will be unable to save
						pView->SendMessage(WM_COMMAND, ID_FILE_CLOSE, 0L); // close file
						return FALSE;
					}
					bDone = true;
					break;
				}
			}
			if (bDone)
			{
				break;
			}
		}

        TRACE(_T("first segment bytes written=%lu\n"), lSizeWritten);

        while (dwSizeLeft>0)
        {
            // read the waveform data block
            long lSizeRead = mmioRead(hmmioFile, buffer, _countof(buffer));
            if (lSizeRead == -1)
            {
                // error reading from data chunk
                pApp->ErrorMessage(IDS_ERROR_READDATACHUNK, m_fileStat.m_szFullName);
                mmioClose(hmmioFile, 0);
                m_dwDataSize = 0;		// no data available
                SetModifiedFlag(FALSE); // will be unable to save
                pView->SendMessage(WM_COMMAND, ID_FILE_CLOSE, 0L); // close file
                return FALSE;
            }

            if ((DWORD)lSizeRead > dwSizeLeft)
            {
                lSizeRead = (long)dwSizeLeft;
            }
            dwSizeLeft -= (DWORD)lSizeRead;

            // write the data block from the buffer
			// we will truncate the data if we need to.
            try
            {
                long lWriteSize = lSizeRead;
                if ((lSizeWritten + lSizeRead + dwSmpSize) > lTotalLength)
                {
					TRACE("truncating data\n");
                    lWriteSize = lTotalLength - lSizeWritten - dwSmpSize;
                }
				ASSERT(lWriteSize>=0);
                file.Write(buffer, (DWORD)lWriteSize);
                lSizeWritten += lWriteSize;
            }
            catch (CFileException e)
            {
                // error writing file
                pApp->ErrorMessage(IDS_ERROR_WRITETEMPFILE, pszTempPathName);
                m_dwDataSize = 0;		// no data available
                SetModifiedFlag(FALSE); // will be unable to save
                pView->SendMessage(WM_COMMAND, ID_FILE_CLOSE, 0L); // close file
                return FALSE;
            }
        }

        TRACE(_T("remainder bytes written=%lu\n"), lSizeWritten);

        // now, if this data is still to short, add the remainder
        try
        {
            if (lSizeWritten < lTotalLength)
            {
                long lRearPad = (lTotalLength-lSizeWritten)/dwSmpSize;
                TRACE(_T("rear pad samples=%lu\n"), lRearPad);
                if (dwSmpSize == 1)
                {
                    char cData = 0;
                    for (long j=0; j<lRearPad; j++)
                    {
                        file.Write(&cData, sizeof(cData));
                        lSizeWritten++;
                    }
                }
                else
                {
                    short int iData = 0;
                    for (long j=0; j<lRearPad; j++)
                    {
                        file.Write(&iData, sizeof(iData));
                        lSizeWritten+=2;
                    }
                }
            }
        }
        catch (CFileException e)
        {
            // error writing file
            pApp->ErrorMessage(IDS_ERROR_WRITETEMPFILE, pszTempPathName);
            m_dwDataSize = 0;		// no data available
            SetModifiedFlag(FALSE); // will be unable to save
            pView->SendMessage(WM_COMMAND, ID_FILE_CLOSE, 0L); // close file
            return FALSE;
        }

        TRACE(_T("total bytes written=%lu\n"), lSizeWritten);

        // ascend out of the data chunk
        mmioAscend(hmmioFile, &mmckinfoSubchunk, 0);
        // close the wave file
        mmioClose(hmmioFile, 0);
    }

    if (!m_szRawDataWrk.empty())
    {
        RemoveFile(m_szRawDataWrk.c_str());
    }

    m_szRawDataWrk = pszTempPathName;

    m_dwDataSize = lSizeWritten;

    // now use our modified temp file to recreate the wave file.
    WriteDataFiles(pszSourcePathName);

    // fragment the waveform
    // remove old fragmented data
    m_pProcessFragments->SetDataInvalid();

    return TRUE;
}

// SDM 1.06.6U2 added ability to insert wave data from a file
/***************************************************************************/
// CSaDoc::CopyWaveToTemp Copy the wave chunk into a temporary file
// The function creates a temporary file if pszTempPathName points to NULL
// (default), and it copies the wave chunk from the wave file with the
// pathname pszSourcePathName points to to the file (the tempfile or the one
// with the pathname pszPathName points to).
/***************************************************************************/
BOOL CSaDoc::CopyWaveToTemp(LPCTSTR pszSourcePathName)
{

    TCHAR pszTempPathName[_MAX_PATH];
    wmemset(pszTempPathName,0,_MAX_PATH);
    GetTempFileName(_T("WAV"), pszTempPathName, _countof(pszTempPathName));

    // get pointer to view and app
    POSITION pos = GetFirstViewPosition();
    CSaView * pView = (CSaView *)GetNextView(pos);
    CSaApp * pApp = (CSaApp *)AfxGetApp();

    CFile file;
    // create and open or just open the file
    if (!file.Open(pszTempPathName, CFile::modeCreate | CFile::modeReadWrite | CFile::shareExclusive))
    {
        // error opening file
        pApp->ErrorMessage(IDS_ERROR_OPENTEMPFILE, pszTempPathName);
        m_dwDataSize = 0;		// no data available
        SetModifiedFlag(FALSE); // will be unable to save
        pView->SendMessage(WM_COMMAND, ID_FILE_CLOSE, 0L); // close file
        return FALSE;
    }

    // now open wave file and go to wave chunk
    HMMIO hmmioFile = mmioOpen(const_cast<TCHAR *>(pszSourcePathName), NULL, MMIO_READ | MMIO_DENYWRITE);
    if (!hmmioFile)
    {
        // error opening file
        pApp->ErrorMessage(IDS_ERROR_FILEOPEN, pszSourcePathName);
        m_dwDataSize = 0;		// no data available
        SetModifiedFlag(FALSE); // will be unable to save
        pView->SendMessage(WM_COMMAND, ID_FILE_CLOSE, 0L); // close file
        return FALSE;
    }

    // locate a 'RIFF' chunk with a 'WAVE' form type to make sure it's a WAVE file.
    MMCKINFO mmckinfoParent;
    mmckinfoParent.fccType = mmioFOURCC('W', 'A', 'V', 'E'); // prepare search code
    if (mmioDescend(hmmioFile, (LPMMCKINFO)&mmckinfoParent, NULL, MMIO_FINDRIFF))
    {
        // error descending into wave chunk
        pApp->ErrorMessage(IDS_ERROR_WAVECHUNK, m_fileStat.m_szFullName);
        mmioClose(hmmioFile, 0);
        m_dwDataSize = 0;		// no data available
        SetModifiedFlag(FALSE); // will be unable to save
        pView->SendMessage(WM_COMMAND, ID_FILE_CLOSE, 0L); // close file
        return FALSE;
    }

    // find the data subchunk
    MMCKINFO mmckinfoSubchunk;
    mmckinfoSubchunk.ckid = mmioFOURCC('d', 'a', 't', 'a');
    if (mmioDescend(hmmioFile, &mmckinfoSubchunk, &mmckinfoParent, MMIO_FINDCHUNK))
    {
        // error descending into data chunk
        pApp->ErrorMessage(IDS_ERROR_DATACHUNK, m_fileStat.m_szFullName);
        mmioClose(hmmioFile, 0);
        m_dwDataSize = 0;		// no data available
        SetModifiedFlag(FALSE); // will be unable to save
        pView->SendMessage(WM_COMMAND, ID_FILE_CLOSE, 0L); // close file
        return FALSE;
    }

    // get the size of the data subchunk
    DWORD dwSizeLeft = mmckinfoSubchunk.cksize;

    char buffer[0x10000];

    VirtualLock(buffer, _countof(buffer));
    while (dwSizeLeft)
    {
        // read the waveform data block
        long lSizeRead = mmioRead(hmmioFile, buffer, _countof(buffer));
        if (lSizeRead == -1)
        {
            // error reading from data chunk
            pApp->ErrorMessage(IDS_ERROR_READDATACHUNK, m_fileStat.m_szFullName);
            mmioClose(hmmioFile, 0);
            m_dwDataSize = 0;		// no data available
            SetModifiedFlag(FALSE); // will be unable to save
            pView->SendMessage(WM_COMMAND, ID_FILE_CLOSE, 0L); // close file
            return FALSE;
        }

        if ((DWORD)lSizeRead > dwSizeLeft)
        {
            lSizeRead = (long)dwSizeLeft;
        }
        dwSizeLeft -= (DWORD)lSizeRead;

        // write the data block from the buffer
        try
        {
            file.Write(buffer, (DWORD)lSizeRead);
        }
        catch (CFileException e)
        {
            // error writing file
            pApp->ErrorMessage(IDS_ERROR_WRITETEMPFILE, pszTempPathName);
            m_dwDataSize = 0;		// no data available
            SetModifiedFlag(FALSE); // will be unable to save
            pView->SendMessage(WM_COMMAND, ID_FILE_CLOSE, 0L); // close file
            return FALSE;
        }
    }

    VirtualUnlock(buffer, _countof(buffer));

    // ascend out of the data chunk
    mmioAscend(hmmioFile, &mmckinfoSubchunk, 0);

    // close the wave file
    mmioClose(hmmioFile, 0);

    file.Close();

    // read temporary file status and set new data size
    m_szRawDataWrk = pszTempPathName;

    // fragment the waveform
    m_pProcessFragments->SetDataInvalid();  // remove old fragmented data

    return TRUE;
}

// SDM 1.06.6U2 added ability to insert wave data from a file
/***************************************************************************
* CSaDoc::InsertWaveToTemp Copy the wave chunk into a temporary file
* The function creates a temporary file if pszTempPathName points to NULL
* (default), and it copies the wave chunk from the wave file with the
* pathname pszSourcePathName points to to the file (the tempfile or the one
* with the pathname pszPathName points to).
***************************************************************************/
BOOL CSaDoc::InsertWaveToTemp(LPCTSTR pszSourcePathName, LPCTSTR pszTempPathName, DWORD dwPos)
{
    // get pointer to view and app
    POSITION pos = GetFirstViewPosition();
    CSaView * pView = (CSaView *)GetNextView(pos);
    CSaApp * pApp = (CSaApp *)AfxGetApp();

    CFile file;
    // open the file
    if (!file.Open(pszTempPathName, CFile::modeReadWrite | CFile::shareExclusive))
    {
        // error opening file
        pApp->ErrorMessage(IDS_ERROR_OPENTEMPFILE, pszTempPathName);
        return FALSE;
    }
    // seek insert position
    try
    {
        file.Seek(dwPos,CFile::begin);
    }
    catch (CFileException e)
    {
        // error opening file
        pApp->ErrorMessage(IDS_ERROR_OPENTEMPFILE, pszTempPathName);
        return FALSE;
    }

    // now open wave file and go to wave chunk
    HMMIO hmmioFile = mmioOpen(const_cast<TCHAR *>(pszSourcePathName), NULL, MMIO_READ | MMIO_DENYWRITE);
    if (!hmmioFile)
    {
        // error opening file
        pApp->ErrorMessage(IDS_ERROR_FILEOPEN, pszSourcePathName);
        m_dwDataSize = 0;		// no data available
        SetModifiedFlag(FALSE); // will be unable to save
        pView->SendMessage(WM_COMMAND, ID_FILE_CLOSE, 0L); // close file
        return FALSE;
    }

    // locate a 'RIFF' chunk with a 'WAVE' form type to make sure it's a WAVE file.
    MMCKINFO mmckinfoParent;
    mmckinfoParent.fccType = mmioFOURCC('W', 'A', 'V', 'E'); // prepare search code
    if (mmioDescend(hmmioFile, (LPMMCKINFO)&mmckinfoParent, NULL, MMIO_FINDRIFF))
    {
        // error descending into wave chunk
        pApp->ErrorMessage(IDS_ERROR_WAVECHUNK, m_fileStat.m_szFullName);
        mmioClose(hmmioFile, 0);
        m_dwDataSize = 0;		// no data available
        SetModifiedFlag(FALSE); // will be unable to save
        pView->SendMessage(WM_COMMAND, ID_FILE_CLOSE, 0L); // close file
        return FALSE;
    }

    // find the data subchunk
    MMCKINFO mmckinfoSubchunk;
    mmckinfoSubchunk.ckid = mmioFOURCC('d', 'a', 't', 'a');
    if (mmioDescend(hmmioFile, &mmckinfoSubchunk, &mmckinfoParent, MMIO_FINDCHUNK))
    {
        // error descending into data chunk
        pApp->ErrorMessage(IDS_ERROR_DATACHUNK, m_fileStat.m_szFullName);
        mmioClose(hmmioFile, 0);
        m_dwDataSize = 0;		// no data available
        SetModifiedFlag(FALSE); // will be unable to save
        pView->SendMessage(WM_COMMAND, ID_FILE_CLOSE, 0L); // close file
        return FALSE;
    }

    // get the size of the data subchunk
    DWORD dwSizeLeft = mmckinfoSubchunk.cksize;

    // use local buffer
    char buffer[0x10000];

    VirtualLock(buffer, _countof(buffer));
    while (dwSizeLeft)
    {
        // read the waveform data block
        long lSizeRead = mmioRead(hmmioFile, (HPSTR)buffer, _countof(buffer));
        if (lSizeRead == -1)
        {
            // error reading from data chunk
            pApp->ErrorMessage(IDS_ERROR_READDATACHUNK, m_fileStat.m_szFullName);
            mmioClose(hmmioFile, 0);
            m_dwDataSize = 0;		// no data available
            SetModifiedFlag(FALSE); // will be unable to save
            pView->SendMessage(WM_COMMAND, ID_FILE_CLOSE, 0L); // close file
            return FALSE;
        }

        if ((DWORD)lSizeRead > dwSizeLeft)
        {
            lSizeRead = (long)dwSizeLeft;
        }
        dwSizeLeft -= (DWORD)lSizeRead;

        // write the data block from the buffer
        try
        {
            file.Write(buffer, (DWORD)lSizeRead);
        }
        catch (CFileException e)
        {
            // error writing file
            pApp->ErrorMessage(IDS_ERROR_WRITETEMPFILE, pszTempPathName);
            m_dwDataSize = 0;		// no data available
            SetModifiedFlag(FALSE); // will be unable to save
            pView->SendMessage(WM_COMMAND, ID_FILE_CLOSE, 0L); // close file
            return FALSE;
        }
    }

    VirtualUnlock(buffer, _countof(buffer));

    // ascend out of the data chunk
    mmioAscend(hmmioFile, &mmckinfoSubchunk, 0);

    // close the wave file
    mmioClose(hmmioFile, 0);

    file.Close();

    // read temporary file status and set new data size
    m_dwDataSize = GetFileSize(m_szRawDataWrk.c_str());

    // fragment the waveform
    m_pProcessFragments->SetDataInvalid();  // remove old fragmented data

    return TRUE;
}

/***************************************************************************/
// CSaDoc::SaveModified Saving a modified document
// Before using the framework original save functions we have to change the
// read only file status to read and write and put it back after the saving.
// If the pathname is empty, CDocument will call save as routines and there-
// fore use the documents title to generate a default file name. So in this
// case we have to clean up the title and after the closing reset it with
// the graph title (because the user may cancel the closing).
/***************************************************************************/
BOOL CSaDoc::SaveModified()
{
    CSaString szCaption, szGraphTitle;
    if (!IsModified())
    {
        return TRUE;    // ok to continue
    }

    // we never save temporary overlay documents.
    if (m_bTempOverlay)
    {
        return TRUE;
    }

    // file is modified, change the file attribute to allow write operation
    CSaString szPathName = GetPathName();
    if (!szPathName.IsEmpty())
    {
        if (!SetFileAttributes(szPathName, (DWORD)m_fileStat.m_attribute))
        {
            CSaString prompt;
            AfxFormatString1(prompt, AFX_IDP_ASK_TO_SAVE, szPathName);
            switch (AfxMessageBox(prompt, MB_YESNOCANCEL, AFX_IDP_ASK_TO_SAVE))
            {
            case IDYES:
                // If so, either Save or Update, as appropriate
                if (!DoFileSave())
                {
                    return FALSE;    // don't continue
                }
                break;

            case IDNO:
                // If not saving changes, revert the document
                return TRUE;
                break;

            default:
                return FALSE;
            }
        }
    }
    else
    {
        // the pathname is empty, reset view title string
        szCaption = GetTitle();					// get the current view caption string
        int nFind = szCaption.Find(':');
        if (nFind != -1)
        {
            szGraphTitle = szCaption.Right(szCaption.GetLength() - nFind); // extract part right of and with :
            szCaption = szCaption.Left(nFind);	// extract part left of :
        }
        SetTitle(szCaption);					// write the new caption string
    }
    // call the base class saving
    BOOL bResult = CDocument::SaveModified();

    if (bResult == TRUE)   // file has been saved or changes should be abandoned
    {
        SetModifiedFlag(FALSE);
        SetTransModifiedFlag(FALSE);
    }
    // change the file attribute to read only
    szPathName = GetPathName();
    if (!szPathName.IsEmpty())
    {
        SetFileAttributes(szPathName, (DWORD)m_fileStat.m_attribute);
    }
    if (!szCaption.IsEmpty())
    {
        // set back the title string
        szCaption = GetTitle(); // get the current view caption string
        szCaption += szGraphTitle; // add the graph title
        SetTitle(szCaption); // write the new caption string
    }
    return bResult;
}

// SDM 1.06.6U2 Added start and max parameters to copy a file portion
/***************************************************************************/
// CSaDoc::CopyWave Copy a file
// The function returns TRUE, if the copying was successful and FALSE, if
// not. It uses the data read buffer to copy the files data. The buffer
// contents will be destroyed after the call.  
// dwStart indicates position in source file to copy from in bytes  
// dwLength indicates maximum length to copy in bytes
/***************************************************************************/
BOOL CSaDoc::CopyWave(LPCTSTR pszSourceName, LPCTSTR pszTargetName)
{
	TRACE("copying %s to %s\n",pszSourceName, pszTargetName);
		
    CFile sourceFile;
    CFile targetFile; // destructor will close the files

    CFileStatus status;
    // open the files
    if (!sourceFile.Open(pszSourceName, CFile::modeRead))
    {
        return FALSE;
    }

    CFileException ex;
    if (!targetFile.Open(pszTargetName, CFile::modeWrite | CFile::modeCreate, &ex))
    {
        TCHAR szError[1024];
        ex.GetErrorMessage(szError, 1024);
        sourceFile.Abort(); // close the source file
        return FALSE;
    }

    // get the source file status
    if (!sourceFile.GetStatus(status))
    {
        sourceFile.Abort(); // close the source file
        targetFile.Abort(); // close the target file
        return FALSE;
    }

    DWORD dwSize = status.m_size;

    try
    {
        targetFile.SeekToEnd();
        sourceFile.Seek( 0, CFile::begin);
    }
    catch (CFileException e)
    {
        sourceFile.Abort(); // close the source file
        targetFile.Abort(); // close the target file
        return FALSE;
    }

    // use local buffer
    char buffer[0x10000];
    DWORD dwCopied = 0;
    while (dwSize > 0)
    {
        try
        {
            DWORD dwCopy = (dwSize > _countof(buffer)) ? _countof(buffer) : dwSize;
            dwCopied = sourceFile.Read(buffer, dwCopy);
            targetFile.Write(buffer, dwCopied);
        }
        catch (CFileException e)
        {
            sourceFile.Abort(); // close the source file
            targetFile.Abort(); // close the target file
            return FALSE;
        }
        dwSize -= dwCopied;
    }

    // kg close the files, we are done!
    // GetWaveData is going to try to open the temp file.
    sourceFile.Abort();
    targetFile.Abort();

    // the CFile destructors will close the files
    return TRUE;
}

// SDM 1.06.6U2 Added start and max parameters to copy a file portion
/***************************************************************************/
// CSaDoc::CopyWave Copy a file
// The function returns TRUE, if the copying was successful and FALSE, if
// not. It uses the data read buffer to copy the files data. The buffer
// contents will be destroyed after the call.  
// dwStart indicates position in source file to copy from in seconds
// dwLength indicates maximum length to copy in seconds
/***************************************************************************/
BOOL CSaDoc::CopyWave(LPCTSTR pszSourceName, LPCTSTR pszTargetName, WAVETIME start, WAVETIME length, BOOL bTruncate)
{
	TRACE("copy %s to %s from %f to %f\n",pszSourceName, pszTargetName, start, start+length);

    CFile sourceFile;
    CFile targetFile; // destructor will close the files

    CFileStatus status;
    // open the files
    if (!sourceFile.Open(pszSourceName, CFile::modeRead))
    {
        return FALSE;
    }

    CFileException ex;
    if (!targetFile.Open(pszTargetName, CFile::modeWrite | CFile::modeCreate | (bTruncate ? 0 : CFile::modeNoTruncate), &ex))
    {
        TCHAR szError[1024];
        ex.GetErrorMessage(szError, 1024);
        sourceFile.Abort(); // close the source file
        return FALSE;
    }

    // get the source file status
    if (!sourceFile.GetStatus(status))
    {
        sourceFile.Abort(); // close the source file
        targetFile.Abort(); // close the target file
        return FALSE;
    }

    DWORD dwSize = status.m_size;
	if (dwSize < toBytes( start, false))
    {
        return TRUE; // Empty file
    }

    try
    {
        targetFile.SeekToEnd();
		sourceFile.Seek( toBytes( start, false), CFile::begin);
    }
    catch (CFileException e)
    {
        sourceFile.Abort(); // close the source file
        targetFile.Abort(); // close the target file
        return FALSE;
    }
    dwSize -= toBytes( start, false); // size to copy
	if (dwSize > toBytes( length, false))
    {
		dwSize = toBytes( length, false);
    }

    // use local buffer
    char buffer[0x10000];
    DWORD dwCopied = 0;
    while (dwSize > 0)
    {
        try
        {
            DWORD dwCopy = (dwSize > _countof(buffer)) ? _countof(buffer) : dwSize;
            dwCopied = sourceFile.Read(buffer, dwCopy);
            targetFile.Write(buffer, dwCopied);
        }
        catch (CFileException e)
        {
            sourceFile.Abort(); // close the source file
            targetFile.Abort(); // close the target file
            return FALSE;
        }
        dwSize -= dwCopied;
    }

    // kg close the files, we are done!
    // GetWaveData is going to try to open the temp file.
    sourceFile.Abort();
    targetFile.Abort();

    // the CFile destructors will close the files
    return TRUE;
}

/***************************************************************************/
// CSaDoc::ApplyWaveFile Apply a new recorded wave file
// This function sets the string m_szTempWave with the full path for the
// OnSaveDocument function, to tell it, that it first must rename (copy) the
// temporary wave file, before open and saving the parameters to it.
// Finally the scrolling parameters and the cursor positions have to be set
// and the graph(s) will be refreshed.
/***************************************************************************/
// SDM 1.06.6U2
void CSaDoc::ApplyWaveFile(LPCTSTR pszFileName, DWORD dwDataSize, BOOL bInitialUpdate)
{
    // save the temporary file
    m_szTempWave = pszFileName;
    // set the data size
    m_dwDataSize = dwDataSize;
    // get file information
    CFile::GetStatus(pszFileName, m_fileStat);

    // create the temporary wave copy
    CopyWaveToTemp(pszFileName);

    // get pointer to view
    POSITION pos = GetFirstViewPosition();
    CSaView * pView = (CSaView *)GetNextView(pos);

    if (bInitialUpdate)
    {
        // SDM 1.06.6U5 Load default graph settings
        // set scrolling parameters, cursors and refresh graph(s)
        pView->InitialUpdate(TRUE);     // Load default graph settings ...
    }
    InvalidateAllProcesses();
    pView->RefreshGraphs(TRUE, TRUE);   // repaint whole graphs
    m_uttParm.Init(m_FmtParm.wBitsPerSample); // re-initialize utterance parameters
    SetUttParm(&m_uttParm, TRUE);       // Copy uttParms to original for safe keeping
    SetModifiedFlag();                  // document is modified
    SetAudioModifiedFlag();
}

/***************************************************************************/
// CSaDoc::ApplyWaveFile Apply a new recorded wave file
// This function sets the string m_szTempWave with the full path for the
// OnSaveDocument function, to tell it, that it first must rename (copy) the
// temporary wave file, before open and saving the parameters to it.
// Finally the scrolling parameters and the cursor positions have to be set
// and the graph(s) will be refreshed.
/***************************************************************************/
// SDM 1.06.6U2
void CSaDoc::ApplyWaveFile(LPCTSTR pszFileName, DWORD dwDataSize, CAlignInfo info)
{
	TRACE(L"Applying wave file  %s of %d at %f for %f\n",pszFileName,dwDataSize,info.dStart,info.dTotalLength);
    // save the temporary file
    if (!m_szTempWave.IsEmpty())
    {
        RemoveFile(m_szTempWave);
    }

    m_szTempWave = pszFileName;
    // set the data size
    m_dwDataSize = dwDataSize;
    // get file information
    CFile::GetStatus(pszFileName, m_fileStat);

    // create the temporary wave copy
	ASSERT(info.bValid);
	CopyWaveToTemp( pszFileName, info.dStart, info.dTotalLength+info.dStart);

    // get pointer to view
    POSITION pos = GetFirstViewPosition();
    CSaView * pView = (CSaView *)GetNextView(pos);

    // SDM 1.06.6U5 Load default graph settings
    // set scrolling parameters, cursors and refresh graph(s)
    pView->InitialUpdate(TRUE); // Load default graph settings ...
    pView->RefreshGraphs(TRUE, TRUE); // repaint whole graphs
    SetModifiedFlag(); // document is modified
    SetAudioModifiedFlag();
}

/***************************************************************************/
// CSaDoc::SnapCursor Snap cursor position to closest zero crossing
// This function sets the given cursor position (second parameter) based on
// cursor alignment specified (sixth parameter).
//
// If the cursor is to be aligned to zero crossings, the function will search
// for them between the upper and lower limits (third and fourth parameters,
// respectively).  The limits have to be allowed positions. The new cursor
// position will be the sample just at or after the crossing if it is the start
// cursor, or to the sample before the zero crossing if it is the stop cursor.
// If the snap direction (fifth parameter) is set to SNAP_BOTH, the
// function looks for the next zero crossing on both sides of the current
// position and takes the closer one as result; otherwise, it looks to the left
// (SNAP_LEFT) or right (SNAP_RIGHT).
//
// If the cursor is to be aligned to waveform fragments, the first parameter is
// used to determine where the cursor will be set.  If START_CURSOR is specified,
// it is set to the beginning of the nearest fragment if SNAP_BOTH directions is
// requested, or to the current fragment if not. If STOP_CURSOR is specified,
// it is set to the last sample in the nearest fragment if SNAP_BOTH directions is
// requested, or to the end of the current fragment if not.
/***************************************************************************/
DWORD CSaDoc::SnapCursor(ECursorSelect nCursorSelect, DWORD dwCursorOffset, ESnapDirection nSnapDirection)
{
    return SnapCursor(nCursorSelect, dwCursorOffset, 0, GetDataSize() - m_FmtParm.wBlockAlign, nSnapDirection);
}

/***************************************************************************/
// CSaDoc::SnapCursor Snap cursor position to closest zero crossing
// This function sets the given cursor position (second parameter) based on
// cursor alignment specified (sixth parameter).
//
// If the cursor is to be aligned to zero crossings, the function will search
// for them between the upper and lower limits (third and fourth parameters,
// respectively).  The limits have to be allowed positions. The new cursor
// position will be the sample just at or after the crossing if it is the start
// cursor, or to the sample before the zero crossing if it is the stop cursor.
// If the snap direction (fifth parameter) is set to SNAP_BOTH, the
// function looks for the next zero crossing on both sides of the current
// position and takes the closer one as result; otherwise, it looks to the left
// (SNAP_LEFT) or right (SNAP_RIGHT).
//
// If the cursor is to be aligned to waveform fragments, the first parameter is
// used to determine where the cursor will be set.
// If START_CURSOR is specified,
// it is set to the beginning of the nearest fragment if SNAP_BOTH directions is
// requested, or to the current fragment if not.
// If STOP_CURSOR is specified,
// it is set to the last sample in the nearest fragment if SNAP_BOTH directions is
// requested, or to the end of the current fragment if not.
/***************************************************************************/
DWORD CSaDoc::SnapCursor(ECursorSelect nCursorSelect,
                         DWORD dwCursorOffset,
                         DWORD dwLowerLimit,
                         DWORD dwUpperLimit,
                         ESnapDirection nSnapDirection,
                         ECursorAlignment nCursorAlignment)
{
    if (GetDataSize() == 0)
    {
        return dwCursorOffset;
    }

    if (nCursorAlignment == ALIGN_USER_SETTING)
    {
        // get pointer to view
        POSITION pos = GetFirstViewPosition();
        CSaView * pView = (CSaView *)GetNextView(pos);
        nCursorAlignment = pView->GetCursorAlignment();
    }

    DWORD wSmpSize = GetSampleSize();
    switch (nCursorAlignment)
    {
    case ALIGN_AT_SAMPLE:
        return dwCursorOffset;

    case ALIGN_AT_ZERO_CROSSING:
    {
        BOOL bOk = TRUE;
        DWORD dwRight = dwUpperLimit + wSmpSize;
        DWORD dwLeft = 0;

        // search for left zero crossing first
        DWORD dwDataPos = dwCursorOffset;
        int nData = GetWaveData(dwDataPos, &bOk);	// get actual data block
        if (!bOk)
        {
            return dwCursorOffset;					// error, return current position
        }
        dwDataPos-=wSmpSize;

        while ((dwDataPos >= dwLowerLimit) && (dwDataPos <= dwUpperLimit))
        {
            int nOldData = nData;
            nData = GetWaveData(dwDataPos, &bOk);	// get actual data block
            if (!bOk)
            {
                return dwCursorOffset;				// error, return current position
            }

            // check if positive zero crossing
            if ((nData < 0) && (nOldData >= 0))
            {
                // crossing found, set back match position
                if (nCursorSelect == START_CURSOR)
                {
                    // position at zero or to the right of the zero crossing
                    dwLeft = dwDataPos + wSmpSize;
                }
                else
                {
                    dwLeft = dwDataPos;
                }
                break;
            }

            if (dwDataPos < wSmpSize)
            {
                break;
            }

            dwDataPos-=wSmpSize;
        }

        // search for right zero crossing
        dwDataPos = dwCursorOffset;
        nData = GetWaveData(dwDataPos, &bOk); // get actual data block
        if (!bOk)
        {
            return dwCursorOffset; // error, return current position
        }
        dwDataPos+=wSmpSize;

        while ((dwDataPos >= dwLowerLimit) && (dwDataPos <= dwUpperLimit))
        {
            int nOldData = nData;

            nData = GetWaveData(dwDataPos, &bOk); // get actual data block
            if (!bOk)
            {
                return dwCursorOffset; // error, return current position
            }

            // check if positive zero crossing
            if ((nData >= 0) && (nOldData < 0))
            {
                // crossing found, set back match position
                if (nCursorSelect == START_CURSOR)
                {
                    // position at zero or to the right of the zero crossing
                    dwRight = dwDataPos;
                }
                else
                {
                    dwRight = dwDataPos - wSmpSize;
                }
                break;
            }
            dwDataPos+=wSmpSize;
        }

        if (dwLeft == dwCursorOffset)
        {
            dwRight = dwCursorOffset;
        }

        if (dwRight == dwCursorOffset)
        {
            dwLeft = dwCursorOffset;
        }


        if ((nSnapDirection == SNAP_BOTH)&&((dwRight > dwUpperLimit)||(dwRight < dwCursorOffset)))
        {
            nSnapDirection = SNAP_LEFT;
        }

        if ((nSnapDirection == SNAP_BOTH)&&((dwLeft < dwLowerLimit)||(dwLeft > dwCursorOffset)))
        {
            nSnapDirection = SNAP_RIGHT;
        }

        if (nSnapDirection == SNAP_BOTH)
        {
            if (dwRight - dwCursorOffset < dwCursorOffset - dwLeft)
            {
                return dwRight;
            }
            else
            {
                return dwLeft;
            }
        }

        if (nSnapDirection == SNAP_RIGHT)
        {
            if ((dwRight > dwUpperLimit)||(dwRight < dwCursorOffset))
            {
                return dwCursorOffset;
            }
            else
            {
                return dwRight;
            }
        }

        if (nSnapDirection == SNAP_LEFT)
        {
            if ((dwLeft > dwCursorOffset)||(dwLeft < dwLowerLimit))
            {
                return dwCursorOffset;
            }
            else
            {
                return dwLeft;
            }
        }
    }

    case ALIGN_AT_FRAGMENT:
    {
        if (m_pProcessFragments->IsDataReady())
        {
            DWORD dwFragmentCount = m_pProcessFragments->GetFragmentCount();
            DWORD dwCursorIndex = dwCursorOffset / wSmpSize;
            DWORD dwFragmentIndex = m_pProcessFragments->GetFragmentIndex(dwCursorIndex);

            if (dwFragmentIndex == UNDEFINED_OFFSET)
            {
                return dwCursorOffset;
            }

            SFragParms stFragment = m_pProcessFragments->GetFragmentParms(dwFragmentIndex);
            DWORD dwFragmentStart = stFragment.dwOffset * wSmpSize;
            DWORD dwFragmentEnd = (stFragment.dwOffset + stFragment.wLength - 1) * wSmpSize;

            DWORD dwRight = 0;
            DWORD dwLeft = 0;

            if (nCursorSelect == START_CURSOR)
            {
                dwLeft = dwFragmentStart;
                if (dwFragmentIndex < dwFragmentCount - 1)
                {
                    // move to next if not last
                    stFragment = m_pProcessFragments->GetFragmentParms(++dwFragmentIndex);
                    dwRight = stFragment.dwOffset * wSmpSize;
                }
                else
                {
                    dwRight = dwUpperLimit + wSmpSize;
                }
            }
            else
            {
                // if we are at the end of the data, then simply pick the start
                // of the last fragment, which is the last valid data chunk.
                if (dwFragmentIndex==(dwFragmentCount-1))
                {
                    return dwFragmentStart;
                }

                dwRight = dwFragmentEnd;
                if (dwFragmentIndex > 0)
                {
                    // move to prev if not first
                    stFragment = m_pProcessFragments->GetFragmentParms(--dwFragmentIndex);
                    dwLeft = (stFragment.dwOffset + stFragment.wLength - 1) * wSmpSize;
                }
                else
                {
                    dwLeft = 0;
                }
            }

            if (dwLeft == dwCursorOffset)
            {
                dwRight = dwCursorOffset;
            }

            if (dwRight == dwCursorOffset)
            {
                dwLeft = dwCursorOffset;
            }

            if ((nSnapDirection == SNAP_BOTH) &&
                    ((dwRight > dwUpperLimit) || (dwRight < dwCursorOffset)))
            {
                nSnapDirection = SNAP_LEFT;
            }

            if ((nSnapDirection == SNAP_BOTH) &&
                    ((dwLeft < dwLowerLimit)||(dwLeft > dwCursorOffset)))
            {
                nSnapDirection = SNAP_RIGHT;
            }

            if (nSnapDirection == SNAP_BOTH)
            {
                if ((dwRight - dwCursorOffset) < (dwCursorOffset - dwLeft))
                {
                    return dwRight;
                }
                else
                {
                    return dwLeft;
                }
            }

            if (nSnapDirection == SNAP_RIGHT)
            {
                if ((dwRight > dwUpperLimit)||(dwRight < dwCursorOffset))
                {
                    return dwCursorOffset;
                }
                else
                {
                    return dwRight;
                }
            }

            if (nSnapDirection == SNAP_LEFT)
            {
                if ((dwLeft > dwCursorOffset) || (dwLeft < dwLowerLimit))
                {
                    return dwCursorOffset;
                }
                else
                {
                    return dwLeft;
                }
            }
        }

        return SnapCursor(nCursorSelect, dwCursorOffset, dwLowerLimit, dwUpperLimit, nSnapDirection, ALIGN_AT_ZERO_CROSSING);
    }

    default:
        return SnapCursor(nCursorSelect, dwCursorOffset, dwLowerLimit, dwUpperLimit, nSnapDirection, ALIGN_AT_FRAGMENT);
    }

}

/***************************************************************************/
// CSaDoc::InvalidateAllProcesses Invalidates all the graph processes
/***************************************************************************/
void CSaDoc::InvalidateAllProcesses()
{
    m_ProcessDoc.SetDataInvalid();
    if (m_pProcessAdjust)
    {
        m_pProcessAdjust->SetDataInvalid();
    }
    if (m_pProcessFragments)
    {
        m_pProcessFragments->SetDataInvalid();
    }
    if (m_pProcessLoudness)
    {
        m_pProcessLoudness->SetDataInvalid();
    }
    if (m_pProcessSmoothLoudness)
    {
        m_pProcessSmoothLoudness->SetDataInvalid();
    }
    if (m_pProcessPitch)
    {
        m_pProcessPitch->SetDataInvalid();
    }
    if (m_pProcessCustomPitch)
    {
        m_pProcessCustomPitch->SetDataInvalid();
    }
    if (m_pProcessSmoothedPitch)
    {
        m_pProcessSmoothedPitch->SetDataInvalid();
    }
    if (m_pProcessChange)
    {
        m_pProcessChange->SetDataInvalid();
    }
    if (m_pProcessRaw)
    {
        m_pProcessRaw->SetDataInvalid();
    }
    if (m_pProcessHilbert)
    {
        m_pProcessHilbert->SetDataInvalid();
    }
    if (m_pProcessSpectrogram)
    {
        m_pProcessSpectrogram->SetDataInvalid();
        m_pProcessSpectrogram->GetFormantProcess()->SetDataInvalid();
    }
    if (m_pProcessSnapshot)
    {
        m_pProcessSnapshot->SetDataInvalid();
        m_pProcessSnapshot->GetFormantProcess()->SetDataInvalid();
    }
    if (m_pProcessFormants)
    {
        m_pProcessFormants->SetDataInvalid();
    }
    if (m_pProcessFormantTracker)
    {
        m_pProcessFormantTracker->SetDataInvalid();
    }
    if (m_pProcessDurations)
    {
        m_pProcessDurations->SetDataInvalid();
    }
    if (m_pProcessZCross)
    {
        m_pProcessZCross->SetDataInvalid();
    }
    if (m_pProcessSpectrum)
    {
        m_pProcessSpectrum->SetDataInvalid();
    }
    if (m_pProcessGrappl)
    {
        m_pProcessGrappl->SetDataInvalid();
    }
    if (m_pProcessPOA)
    {
        m_pProcessPOA->SetDataInvalid();
    }
    if (m_pProcessGlottis)
    {
        m_pProcessGlottis->SetDataInvalid();
    }
    if (m_pProcessMelogram)
    {
        m_pProcessMelogram->SetDataInvalid();    // CLW 4/5/00
    }
    if (m_pProcessTonalWeightChart)
    {
        m_pProcessTonalWeightChart->SetDataInvalid();    // CLW 10/24/00
    }
    if (m_pProcessSDP[0])
    {
        m_pProcessSDP[0]->SetDataInvalid();
    }
    if (m_pProcessSDP[1])
    {
        m_pProcessSDP[1]->SetDataInvalid();
    }
    if (m_pProcessSDP[2])
    {
        m_pProcessSDP[2]->SetDataInvalid();
    }
    if (m_pProcessRatio)
    {
        m_pProcessRatio->SetDataInvalid();
    }
}

/***************************************************************************/
// CSaDoc::RestartAllProcesses Restart all the processes (except CAreaDataProcesses)
/***************************************************************************/
void CSaDoc::RestartAllProcesses()
{
    if (m_pProcessAdjust && m_pProcessAdjust->IsCanceled())
    {
        m_pProcessAdjust->SetDataInvalid();
        m_pProcessAdjust->RestartProcess();
    }
    if (m_pProcessFragments && m_pProcessFragments->IsCanceled())
    {
        m_pProcessFragments->SetDataInvalid();
        m_pProcessFragments->RestartProcess();
    }
    if (m_pProcessLoudness && m_pProcessLoudness->IsCanceled())
    {
        m_pProcessLoudness->SetDataInvalid();
        m_pProcessLoudness->RestartProcess();
    }
    if (m_pProcessSmoothLoudness && m_pProcessSmoothLoudness->IsCanceled())
    {
        m_pProcessSmoothLoudness->SetDataInvalid();
        m_pProcessSmoothLoudness->RestartProcess();
    }
    if (m_pProcessPitch && m_pProcessPitch->IsCanceled())
    {
        m_pProcessPitch->SetDataInvalid();
        m_pProcessPitch->RestartProcess();
    }
    if (m_pProcessCustomPitch && m_pProcessCustomPitch->IsCanceled())
    {
        m_pProcessCustomPitch->SetDataInvalid();
        m_pProcessCustomPitch->RestartProcess();
    }
    if (m_pProcessSmoothedPitch && m_pProcessSmoothedPitch->IsCanceled())
    {
        m_pProcessSmoothedPitch->SetDataInvalid();
        m_pProcessSmoothedPitch->RestartProcess();
    }
    if (m_pProcessChange && m_pProcessChange->IsCanceled())
    {
        m_pProcessChange->SetDataInvalid();
        m_pProcessChange->RestartProcess();
    }
    if (m_pProcessRaw && m_pProcessRaw->IsCanceled())
    {
        m_pProcessRaw->SetDataInvalid();
        m_pProcessRaw->RestartProcess();
    }
    if (m_pProcessHilbert && m_pProcessHilbert->IsCanceled())
    {
        m_pProcessHilbert->SetDataInvalid();
        m_pProcessHilbert->RestartProcess();
    }
    if (m_pProcessFormants && m_pProcessFormants->IsCanceled())
    {
        m_pProcessFormants->SetDataInvalid();
        m_pProcessFormants->RestartProcess();
    }
    if (m_pProcessFormantTracker && m_pProcessFormantTracker->IsCanceled())
    {
        m_pProcessFormantTracker->SetDataInvalid();
        m_pProcessFormantTracker->RestartProcess();
    }
    if (m_pProcessDurations && m_pProcessDurations->IsCanceled())
    {
        m_pProcessDurations->SetDataInvalid();
        m_pProcessDurations->RestartProcess();
    }
    if (m_pProcessZCross && m_pProcessZCross->IsCanceled())
    {
        m_pProcessZCross->SetDataInvalid();
        m_pProcessZCross->RestartProcess();
    }
    if (m_pProcessSpectrum && m_pProcessSpectrum->IsCanceled())
    {
        m_pProcessSpectrum->SetDataInvalid();
        m_pProcessSpectrum->RestartProcess();
    }
    if (m_pProcessGrappl && m_pProcessGrappl->IsCanceled())
    {
        m_pProcessGrappl->SetDataInvalid();
        m_pProcessGrappl->RestartProcess();
    }
    if (m_pProcessPOA && m_pProcessPOA->IsCanceled())
    {
        m_pProcessPOA->SetDataInvalid();
        m_pProcessPOA->RestartProcess();
    }
    if (m_pProcessGlottis && m_pProcessGlottis->IsCanceled())
    {
        m_pProcessGlottis->SetDataInvalid();
        m_pProcessGlottis->RestartProcess();
    }
    if (m_pProcessMelogram && m_pProcessMelogram->IsCanceled())
    {
        m_pProcessMelogram->SetDataInvalid(); // CLW 4/5/00
        m_pProcessMelogram->RestartProcess();
    }
    if (m_pProcessTonalWeightChart && m_pProcessTonalWeightChart->IsCanceled())
    {
        m_pProcessTonalWeightChart->SetDataInvalid(); // CLW 10/24/00
        m_pProcessTonalWeightChart->RestartProcess();
    }
    if (m_pProcessRatio && m_pProcessRatio->IsCanceled())
    {
        m_pProcessRatio->SetDataInvalid();
        m_pProcessRatio->RestartProcess();
    }
    if (m_pProcessSpectrogram)
    {
        if (m_pProcessSpectrogram->IsCanceled())
        {
            m_pProcessSpectrogram->SetDataInvalid();
            m_pProcessSpectrogram->RestartProcess();
        }
        if (m_pProcessSpectrogram->GetFormantProcess()->IsCanceled())
        {
            m_pProcessSpectrogram->GetFormantProcess()->SetDataInvalid();
            m_pProcessSpectrogram->GetFormantProcess()->RestartProcess();
        }
    }
    if (m_nWbProcess>0)
    {
        CMainFrame * pMain = (CMainFrame *)AfxGetMainWnd();
        for (int i=0; i < MAX_FILTER_NUMBER; i++)
        {
            if (!pMain->GetWbProcess(m_nWbProcess - 1, i))
            {
                break;
            }
            if (!pMain->GetWbProcess(m_nWbProcess - 1, i)->IsCanceled())
            {
                continue;
            }
            pMain->GetWbProcess(m_nWbProcess - 1, 0)->SetDataInvalid();
            pMain->GetWbProcess(m_nWbProcess - 1, i)->RestartProcess();
        }
    }
}

/***************************************************************************/
// CSaDoc::AnyProcessCanceled Check cancel status of all the processes (except CAreaDataProcesses)
/***************************************************************************/
BOOL CSaDoc::AnyProcessCanceled()
{

    if (m_pProcessAdjust && m_pProcessAdjust->IsCanceled())
    {
        return TRUE;
    }
    if (m_pProcessFragments && m_pProcessFragments->IsCanceled())
    {
        return TRUE;
    }
    if (m_pProcessLoudness && m_pProcessLoudness->IsCanceled())
    {
        return TRUE;
    }
    if (m_pProcessSmoothLoudness && m_pProcessSmoothLoudness->IsCanceled())
    {
        return TRUE;
    }
    if (m_pProcessPitch && m_pProcessPitch->IsCanceled())
    {
        return TRUE;
    }
    if (m_pProcessCustomPitch && m_pProcessCustomPitch->IsCanceled())
    {
        return TRUE;
    }
    if (m_pProcessSmoothedPitch && m_pProcessSmoothedPitch->IsCanceled())
    {
        return TRUE;
    }
    if (m_pProcessChange && m_pProcessChange->IsCanceled())
    {
        return TRUE;
    }
    if (m_pProcessRaw && m_pProcessRaw->IsCanceled())
    {
        return TRUE;
    }
    if (m_pProcessHilbert && m_pProcessHilbert->IsCanceled())
    {
        return TRUE;
    }
    if (m_pProcessFormants && m_pProcessFormants->IsCanceled())
    {
        return TRUE;
    }
    if (m_pProcessFormantTracker && m_pProcessFormantTracker->IsCanceled())
    {
        return TRUE;
    }
    if (m_pProcessDurations && m_pProcessDurations->IsCanceled())
    {
        return TRUE;
    }
    if (m_pProcessZCross && m_pProcessZCross->IsCanceled())
    {
        return TRUE;
    }
    if (m_pProcessSpectrum && m_pProcessSpectrum->IsCanceled())
    {
        return TRUE;
    }
    if (m_pProcessGrappl && m_pProcessGrappl->IsCanceled())
    {
        return TRUE;
    }
    if (m_pProcessPOA && m_pProcessPOA->IsCanceled())
    {
        return TRUE;
    }
    if (m_pProcessGlottis && m_pProcessGlottis->IsCanceled())
    {
        return TRUE;
    }
    if (m_pProcessMelogram && m_pProcessMelogram->IsCanceled())
    {
        return TRUE;
    }
    if (m_pProcessTonalWeightChart && m_pProcessTonalWeightChart->IsCanceled())
    {
        return TRUE;
    }
    if (m_pProcessRatio && m_pProcessRatio->IsCanceled())
    {
        return TRUE;
    }
    if (m_pProcessSpectrogram && m_pProcessSpectrogram->IsCanceled())
    {
        return TRUE;
    }
    if (m_pProcessSpectrogram && m_pProcessSpectrogram->GetFormantProcess() && m_pProcessSpectrogram->GetFormantProcess()->IsCanceled())
    {
        return TRUE;
    }
    if (m_pProcessSnapshot && m_pProcessSnapshot->IsCanceled())
    {
        return TRUE;
    }
    CMainFrame * pMain = (CMainFrame *)AfxGetMainWnd();
    if (m_nWbProcess>0)
    {
        for (int i=0; i < MAX_FILTER_NUMBER; i++)
        {
            if (!pMain->GetWbProcess(m_nWbProcess - 1, i))
            {
                break;
            }
            if ((pMain->GetWbProcess(m_nWbProcess - 1, i))->IsCanceled())
            {
                return TRUE;
            }
        }
    }
    return FALSE;
}

/***************************************************************************/
// CSaDoc::WorkbenchProcess Processes the actually selected workbench process
// This function processes new wave source data, if necessary. If an error
// occurs or the process has been canceled by the user, the it displays an
// appropriate message and switches the wave source back to the wave file.
// All the data of the processes for the graphs will be invalidated if there
// has something been processed in the workbench or if the flag bInvalidate
// is TRUE (default FALSE). The function returns TRUE, if it did processing
// or invalidating, FALSE, if it didn't. If the flag bRestart is TRUE
// (default FALSE), the process will reprocess.
/***************************************************************************/
BOOL CSaDoc::WorkbenchProcess(BOOL bInvalidate, BOOL bRestart)
{

    BOOL bProcess = FALSE;
    CMainFrame * pMain = (CMainFrame *)AfxGetMainWnd();
    m_pProcessAdjust->Process(this,this);

    if (m_nWbProcess>0)
    {
        if (pMain->GetWbProcess(m_nWbProcess - 1, 0))
        {
            // get pointer to view
            POSITION pos = GetFirstViewPosition();
            CSaView * pView = (CSaView *)GetNextView(pos);
            // in case of cancelled process, restart it
            pMain->GetWbProcess(m_nWbProcess - 1, 0)->RestartProcess();
            // force process data invalidation
            if (bRestart)
            {
                pMain->GetWbProcess(m_nWbProcess - 1, 0)->SetDataInvalid();
            }
            // now process
            short int nResult = LOWORD(pMain->GetWbProcess(m_nWbProcess - 1, 0)->Process(pView, this));
            bProcess = TRUE;
            // check process result
            CSaApp * pApp = (CSaApp *)AfxGetApp();
            if (nResult == PROCESS_CANCELED)
            {
                // process has been canceled, inform user and switch back to wave file
                pApp->ErrorMessage(IDS_ERROR_WBCANCEL);
                m_nWbProcess = 0;
                bInvalidate = TRUE;
            }
            else
            {
                if (nResult == PROCESS_ERROR)
                {
                    // processing error, inform user and switch back to wave file
                    pApp->ErrorMessage(IDS_ERROR_WBPROCESS);
                    m_nWbProcess = 0;
                    bInvalidate = TRUE;
                }
                else
                {
                    if (nResult)
                    {
                        bInvalidate = TRUE;    // new data processed, reprocess graph data
                    }
                }
            }
        }
    }
    if (bInvalidate)
    {
        // invalidate all the graph process data
        InvalidateAllProcesses();
        bProcess = TRUE;
    }
    return bProcess;
}

/***************************************************************************/
// CSaDoc::PutWaveToClipboard Copies wave data out of the wave file
// The function takes wave data out of the wave file copy at the position and
// with the length given as parameters (in bytes) and copies it to an
// globally allocated buffer (not locked). In case of error it returns a NULL
// handle. If the flag bDelete is TRUE (default is FALSE) it deletes the
// copied section out of the wavefile.
/***************************************************************************/
BOOL CSaDoc::PutWaveToClipboard( WAVETIME sectionStart, WAVETIME sectionLength, BOOL bDelete)
{
    // because we now use true CF_WAVE we support annotations embedded in the data
    // temporary target file has to be created
    // Make new wave file with selected data and segments
    // Find and Copy Original Wave File
    TCHAR szTempNewWave[_MAX_PATH];
    GetTempFileName(_T("WAV"), szTempNewWave, _countof(szTempNewWave));

    DWORD tempSize = GetFileSize( m_szRawDataWrk.c_str());

    if (!CopySectionToNewWavFile( sectionStart, sectionLength, szTempNewWave, TRUE))
    {
        return FALSE;
    }

    COleDataSource * pClipData = new COleDataSource();

    // We have a wave file
    CFile file;
    if (!file.Open(szTempNewWave,CFile::modeRead | CFile::shareExclusive))
    {
        TRACE("unable to open file on render\n");
        return FALSE;
    }

    DWORD dwLength = file.GetLength();

    CSaApp * pApp = (CSaApp *)AfxGetApp();

    // allocate clipboard buffer
    HGLOBAL hGlobal = ::GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, dwLength);
    if (hGlobal==NULL)
    {
        // memory allocation error
        pApp->ErrorMessage(IDS_ERROR_MEMALLOC);
        file.Abort();
        return FALSE;
    }

    // lock memory
    HPSTR lpData = (HPSTR)::GlobalLock(hGlobal);
    if (!lpData)
    {
        // memory lock error
        pApp->ErrorMessage(IDS_ERROR_MEMLOCK);
        ::GlobalFree(hGlobal);
        file.Abort();
        return FALSE;
    }

    // Copy temporary wave file to buffer then delete
    try
    {
        file.Read((HPSTR)lpData, dwLength);
        file.Abort();
        ::GlobalUnlock(hGlobal);
    }
    catch (const CException &)
    {
        ::GlobalFree(hGlobal);
        file.Abort();
        return FALSE;
    }

    // declare one format for transferring
    FORMATETC format;
    format.cfFormat = CF_WAVE;
    format.ptd = NULL;
    format.dwAspect = DVASPECT_CONTENT;
    format.lindex = -1;
    format.tymed = TYMED_HGLOBAL;
    pClipData->CacheGlobalData(CF_WAVE, hGlobal, &format);

    pClipData->SetClipboard();

    if (bDelete)
    {
        CSaApp * pApp = (CSaApp *)AfxGetApp();
        // this piece has to be deleted

        // open temporary wave file
        CFile file;
        if (!file.Open(m_szRawDataWrk.c_str(), CFile::modeReadWrite | CFile::shareExclusive))
        {
            // error opening file
            pApp->ErrorMessage(IDS_ERROR_OPENTEMPFILE, m_szRawDataWrk.c_str());
            return FALSE;
        }

		DWORD dwSectionStart1 = toBytes( sectionStart, true);
		DWORD dwSectionLength1 = toBytes( sectionLength, true);
		DWORD dwSectionStart = toBytes( sectionStart, false);
		DWORD dwSectionLength = toBytes( sectionLength, false);

        DWORD dwDataTail = tempSize - dwSectionStart - dwSectionLength;
        if (dwDataTail>0)
        {
			// do nothing
        }
        DWORD dwSectionPos = dwSectionStart;

		// use local buffer
        char buffer[0x10000];
        while (dwDataTail)
        {
            // find the source position in the data and read the wave data block
            DWORD dwReadSize;
            try
            {
                file.Seek((long)(dwSectionPos + dwSectionLength), CFile::begin);
                dwReadSize = file.Read(buffer, _countof(buffer));
                dwDataTail -= dwReadSize;
            }
            catch (CFileException e)
            {
                // error reading file
                pApp->ErrorMessage(IDS_ERROR_READTEMPFILE, m_szRawDataWrk.c_str());
                return FALSE;
            }
            // find the target position and write the data block from the buffer
            try
            {
                file.Seek((long)(dwSectionPos), CFile::begin);
                file.Write(buffer, dwReadSize);
            }
            catch (CFileException e)
            {
                // error writing file
                pApp->ErrorMessage(IDS_ERROR_WRITETEMPFILE, m_szRawDataWrk.c_str());
                return FALSE;
            }
            dwSectionPos += _countof(buffer);
        }
        try
        {
            file.SetLength(tempSize - dwSectionLength);
        }
        catch (CFileException e)
        {
            // error writing file
            pApp->ErrorMessage(IDS_ERROR_WRITETEMPFILE, m_szRawDataWrk.c_str());
            return FALSE;
        }

        // SDM 1.06.6U4 - Change Document after checkpoint (except wave)
        // register for undo function
        m_bWaveUndoNow = TRUE;
        CheckPoint();
        m_bWaveUndoNow = FALSE;
        m_nCheckPointCount++;

        // set new data size
		m_dwDataSize -= dwSectionLength;
        SetModifiedFlag(TRUE); // data has been modified
        SetAudioModifiedFlag(TRUE);
        POSITION pos = GetFirstViewPosition();

        ((CSaView *)GetNextView(pos))->AdjustCursors( dwSectionStart1, dwSectionLength1, TRUE); // adjust cursors to new file size
        AdjustSegments( sectionStart, sectionLength, true); // adjust segments to new file size
    }

    return TRUE;
}

/***************************************************************************/
// CSaDoc::PasteClipboardToWave Pastes wave data into the wave file
// This function gets a handle with a globally allocated block of memory full
// of wave data. 
// It pastes the data into the wave file at the position (in bytes) given as parameter. 
// In case of error it returns FALSE, else TRUE.
/***************************************************************************/
BOOL CSaDoc::PasteClipboardToWave( HGLOBAL hData, CURSORPOS dwPastePos)
{

    TRACE("dwPastPos=%d\n",dwPastePos);

    TCHAR szTempPath[_MAX_PATH];
    if (!CClipboardHelper::LoadFileFromData(hData,szTempPath,_MAX_PATH))
    {
        TRACE("Unable to retrieve clipboard data\n");
        return FALSE;
    }

    CFileStatus status;
    DWORD dwPasteSize = CheckWaveFormatForPaste(szTempPath);
    if (dwPasteSize == 0)
    {
        TRACE("paste size is zero\n");
        RemoveFile(szTempPath);
        return FALSE;
    }

    CSaApp * pApp = (CSaApp *)AfxGetApp();

    // open temporary wave file
    {
        TCHAR lpszRawTempPath[_MAX_PATH];
        GetTempFileName(_T("WAV"), lpszRawTempPath, _countof(lpszRawTempPath));

		WAVETIME start = toTimeFromSamples( 0);
		WAVETIME length = toTime( dwPastePos, true);
		CopyWave( m_szRawDataWrk.c_str(), lpszRawTempPath, start, length, TRUE);

        // increase the size of the file
        try
        {
            //Get new wave data
            if (!InsertWaveToTemp(szTempPath, lpszRawTempPath, dwPastePos))
            {
                RemoveFile(szTempPath);
                Undo(FALSE);
                return FALSE;  // Reason displayed in failed function
            }
        }
        catch (CFileException e)
        {
            // error writing file
            pApp->ErrorMessage(IDS_ERROR_WRITETEMPFILE, lpszRawTempPath);
            RemoveFile(szTempPath);
            return FALSE;
        }

		start = toTime( dwPastePos, true);
		length = toTime( 0xffffffff, true);
		CopyWave(m_szRawDataWrk.c_str(), lpszRawTempPath, start, length, FALSE);
        RemoveFile(m_szRawDataWrk.c_str());
        m_szRawDataWrk = lpszRawTempPath;
    }


    // SDM 1.06.6U4 - Change Document after checkpoint (except wave)
    // register for undo function
    m_bWaveUndoNow = TRUE;
    CheckPoint();
    m_bWaveUndoNow = FALSE;
    m_nCheckPointCount++;

    // set new data size
    m_dwDataSize += dwPasteSize;
    SetModifiedFlag(TRUE); // data has been modified
    SetAudioModifiedFlag();

    POSITION pos = GetFirstViewPosition();
    ((CSaView *)GetNextView(pos))->SetStartStopCursorPosition( dwPastePos, dwPastePos+dwPasteSize, SNAP_BOTH, ALIGN_AT_SAMPLE);

	WAVETIME start = toTime( dwPastePos, true);
	WAVETIME length = toTime( dwPasteSize, true);
    AdjustSegments( start, length, false);  // adjust segments to new file size

    //Get new segments
    InsertTranscriptions( pApp->GetLastClipboardPath(), dwPastePos);

    return TRUE;
}

/***************************************************************************
* CSaDoc::PasteClipboardToWave inserts a section of silence into the current
* wave.
* silence - the length to insert specified in seconds.
* insertAt - the location where to insert the new section.  this is for
* a single channel and may need to be converted
* In case of error it returns FALSE, else TRUE.
***************************************************************************/
BOOL CSaDoc::InsertSilenceIntoWave( WAVETIME silence, WAVETIME insertAt)
{
    TRACE("start=%d\n",insertAt);

    CSaApp * pApp = (CSaApp *)AfxGetApp();

	CURSORPOS dwSilenceSize = toCursor( silence); 

    // open temporary wave file
    {
        TCHAR lpszRawTempPath[_MAX_PATH];
        GetTempFileName(_T("WAV"), lpszRawTempPath, _countof(lpszRawTempPath));

		// copy the 'pre' section part of the wave
		WAVETIME start = toTimeFromSamples( 0);
		WAVETIME length = insertAt;
		CopyWave( m_szRawDataWrk.c_str(), lpszRawTempPath, start, length, TRUE);

        // increase the size of the file
        try
        {
			// get pointer to view and app
			POSITION pos = GetFirstViewPosition();
			CSaView * pView = (CSaView *)GetNextView(pos);
			CSaApp * pApp = (CSaApp *)AfxGetApp();

			CFile file;
			// open the file
			if (!file.Open(lpszRawTempPath, CFile::modeReadWrite | CFile::shareExclusive))
			{
				// error opening file
				pApp->ErrorMessage(IDS_ERROR_OPENTEMPFILE, lpszRawTempPath);
				Undo(FALSE);
				return FALSE;
			}
			// seek insert position
			try
			{
				DWORD dwTemp = toBytes( insertAt, false);
				file.Seek( dwTemp, CFile::begin);
			}
			catch (CFileException e)
			{
				// error opening file
				pApp->ErrorMessage(IDS_ERROR_OPENTEMPFILE, lpszRawTempPath);
				Undo(FALSE);
				return FALSE;
			}

			// calculate the data size
			DWORD dwSizeLeft = toBytes( silence, false);

			// use local buffer
			char buffer[0x10000];
			memset(buffer,0,_countof(buffer));
			DWORD lSizeRead = _countof(buffer);

			VirtualLock(buffer, _countof(buffer));
			while (dwSizeLeft)
			{
				// read the waveform data block
				if ((DWORD)lSizeRead > dwSizeLeft)
				{
					lSizeRead = (long)dwSizeLeft;
				}
				dwSizeLeft -= (DWORD)lSizeRead;

				// write the data block from the buffer
				try
				{
					file.Write(buffer, (DWORD)lSizeRead);
				}
				catch (CFileException e)
				{
					// error writing file
					pApp->ErrorMessage(IDS_ERROR_WRITETEMPFILE, lpszRawTempPath);
					m_dwDataSize = 0;		// no data available
					SetModifiedFlag(FALSE); // will be unable to save
					pView->SendMessage(WM_COMMAND, ID_FILE_CLOSE, 0L); // close file
					Undo(FALSE);
					return FALSE;
				}
			}

			VirtualUnlock(buffer, _countof(buffer));

			file.Close();

			// read temporary file status and set new data size
			m_dwDataSize = GetFileSize(m_szRawDataWrk.c_str());

			// fragment the waveform
			m_pProcessFragments->SetDataInvalid();  // remove old fragmented data
        }
        catch (CFileException e)
        {
            // error writing file
            pApp->ErrorMessage(IDS_ERROR_WRITETEMPFILE, lpszRawTempPath);
            return FALSE;
        }

		// copy the 'post' section part of the wave
		length =  (WAVETIME)GetFileSize(m_szRawDataWrk.c_str());
		length /= (WAVETIME)GetBytesPerSample(false);
		length /= (WAVETIME)GetSamplesPerSec();
		CopyWave( m_szRawDataWrk.c_str(), lpszRawTempPath, insertAt, length, FALSE);

		// all done!
        RemoveFile(m_szRawDataWrk.c_str());
        m_szRawDataWrk = lpszRawTempPath;
    }


    // SDM 1.06.6U4 - Change Document after checkpoint (except wave)
    // register for undo function
    m_bWaveUndoNow = TRUE;
    CheckPoint();
    m_bWaveUndoNow = FALSE;
    m_nCheckPointCount++;

    // set new data size
    m_dwDataSize += (dwSilenceSize*GetNumChannels());

	SetModifiedFlag(TRUE); // data has been modified
    SetAudioModifiedFlag();

    POSITION pos = GetFirstViewPosition();
	CURSORPOS startPos = toCursor( insertAt);
    ((CSaView *)GetNextView(pos))->SetStartStopCursorPosition( startPos, startPos+dwSilenceSize, SNAP_BOTH, ALIGN_AT_SAMPLE);

	AdjustSegments( insertAt, silence, false);  // adjust segments to new file size

    return TRUE;
}

/***************************************************************************/
// CSaDoc::DeleteWaveFromUndo Deletes all wave undo entries from the undo list
// All wave undo entries have to be deleted all the previous undos also have
// to, because they are invalid now (file size has changed).
/***************************************************************************/
void CSaDoc::DeleteWaveFromUndo()
{

    // first undo until no more wave entries or end of list found
    int nUndoCount = 0;
    while ((CanUndo()) && (m_nCheckPointCount>0))
    {
        Undo(TRUE, TRUE); // undo and add to redo list
        if (IsWaveToUndo())
        {
            // this is a wave entry
            Redo();
            while (CanUndo())
            {
                Undo(FALSE, FALSE);    // don't undo, simply remove undo from list with no redo
            }
            m_bWaveUndoNow = FALSE;
        }
        else
        {
            nUndoCount++;
        }
    }
    // now redo
    while (nUndoCount--)
    {
        Redo();
    }
    m_nCheckPointCount = 0;
    m_bWaveUndoNow = FALSE;
}

/***************************************************************************/
// CSaDoc::UndoWaveFile Undo a wave file change
// All wave undo entries have to be undone 
// all the previous undos also have to, they will not be able to be redone.
/***************************************************************************/
void CSaDoc::UndoWaveFile()
{
    while (CanUndo() && (m_nCheckPointCount))
    {
        Undo(FALSE, TRUE); // undo but no redo
    }

    RemoveFile(m_szRawDataWrk.c_str());
    CopyWaveToTemp(m_fileStat.m_szFullName);
    m_dwDataSize = GetFileSize(m_szRawDataWrk.c_str());

    InvalidateAllProcesses();
    m_bWaveUndoNow = FALSE;

    // Clear Redo List
    CheckPoint();
    Undo(FALSE, FALSE); // SDM 1.5Test10.5
}

/***************************************************************************/
// CSaDoc::SetFileExtension Sets the file extension of a filename.
/***************************************************************************/
CSaString CSaDoc::SetFileExtension(CSaString fileName, CSaString fileExtension)
{
    CSaString szReturn = fileName;

    // get rid of existing file extension
    int pos = fileName.ReverseFind(_T('.'));
    if (pos == -1)
    {
        pos = fileName.GetLength();
    }
    szReturn = fileName.Left(pos);

    // append desired file extension
    szReturn += fileExtension;

    return szReturn;
}

/***************************************************************************
* CSaDoc::AdjustSegments Adjust segments to new file size
* The function adjusts the segments to the new file size. The file size
* changed at the position dwSectionStart by dwSectionLength bytes and it
* shrunk, if the flag bShrink is TRUE, otherwise it grew. In case of growth
* all the segments offsets after dwSectionStart change. In the other way,
* the segments in the deleted section will be deleted. The ones that over-
* lap into the section will be adjusted if valid
***************************************************************************/
void CSaDoc::AdjustSegments( WAVETIME sectionStart, WAVETIME sectionLength, bool bShrink)
{

	DWORD dwSectionStart = toBytes( sectionStart, true);
	DWORD dwSectionLength = toBytes( sectionLength, true);

    if (bShrink)
    {
        // save new file size for later
        int nGlossIndex = m_apSegments[GLOSS]->GetOffsetSize() - 1;
        DWORD dwNewDataSize ;
        if (nGlossIndex != -1)   // we don't need the value if there is no gloss
        {
            dwNewDataSize = m_apSegments[GLOSS]->GetStop(nGlossIndex) - dwSectionLength;
        }

        for (int independent=0; independent<ANNOT_WND_NUMBER; independent++)
        {
            if (m_apSegments[independent]->GetMasterIndex() != -1)
            {
                continue;
            }

            // section in file deleted, find segments to delete
            int nIndex = m_apSegments[independent]->FindFromPosition(dwSectionStart);
            while (nIndex != -1)
            {
                DWORD dwOldOffset = m_apSegments[independent]->GetOffset(nIndex);
                if (dwOldOffset > (dwSectionStart + dwSectionLength))
                {
                    break; // no more to delete
                }
                if (m_apSegments[independent]->GetSelection() != nIndex)
                {
                    m_apSegments[independent]->SetSelection(nIndex);
                }
                //SDM 1.06.6U1 if segments overlap adjust if its valid
                BOOL bAdjusted;
                bAdjusted = FALSE;
                if (dwOldOffset < dwSectionStart)
                {
                    if ((m_apSegments[independent]->GetDuration(nIndex)+dwOldOffset) > (dwSectionStart+dwSectionLength))
                    {
                        //Segment overlaps entire cut region
                        DWORD dwNewStop = m_apSegments[independent]->GetDuration(nIndex)+dwOldOffset-dwSectionLength;
                        if (m_apSegments[independent]->CheckPosition(this, dwOldOffset, dwNewStop, CSegment::MODE_EDIT)!=-1)
                        {
                            bAdjusted = TRUE;
                            ((CIndependentSegment *)m_apSegments[independent])->Adjust(this, nIndex, dwOldOffset, dwNewStop-dwOldOffset);
                        }
                    }
                    else if ((m_apSegments[independent]->GetDuration(nIndex) + dwOldOffset) > dwSectionStart)
                    {
                        //Segment ends in cut region
                        if (m_apSegments[independent]->CheckPosition(this, dwOldOffset, dwSectionStart, CSegment::MODE_EDIT)!=-1)
                        {
                            bAdjusted = TRUE;
                            ((CIndependentSegment *)m_apSegments[independent])->Adjust(this, nIndex, dwOldOffset, dwSectionStart-dwOldOffset);
                        }
                    }
                    else
                    {
                        bAdjusted = TRUE;    // segment ends before the cut region
                    }
                }
                else if ((m_apSegments[independent]->GetDuration(nIndex)+dwOldOffset) > (dwSectionStart+dwSectionLength))
                {
                    //Segment starts in cut region
                    DWORD dwNewStop = m_apSegments[independent]->GetDuration(nIndex)+dwSectionStart-(dwSectionStart+dwSectionLength-dwOldOffset);
                    if (m_apSegments[independent]->CheckPosition(this, dwSectionStart, dwNewStop, CSegment::MODE_EDIT)!=-1)
                    {
                        bAdjusted = TRUE;
                        ((CIndependentSegment *)m_apSegments[independent])->Adjust(this, nIndex, dwSectionStart, dwNewStop-dwSectionStart);
                    }
                }
                if (!bAdjusted)
                {
                    // delete segment
                    // change the independent arrays
                    int nLength = m_apSegments[independent]->GetSegmentLength(nIndex);
                    CString * pAnnotation = m_apSegments[independent]->GetString();
                    *pAnnotation = pAnnotation->Left(nIndex) + pAnnotation->Right(pAnnotation->GetLength() - nLength - nIndex);
                    m_apSegments[independent]->RemoveAt(nIndex,nLength);
                    // delete aligned dependent segments and gloss
                    for (int nLoop = 1; nLoop < ANNOT_WND_NUMBER; nLoop++)
                    {
                        CSegment * pSegment = m_apSegments[nLoop];
                        if (pSegment)
                        {
                            int nInnerIndex = pSegment->FindOffset(dwOldOffset);
                            if (nInnerIndex != -1)
                            {
                                if (pSegment->GetSelection() != nInnerIndex)
                                {
                                    pSegment->SetSelection(nInnerIndex);
                                }
                                pSegment->RemoveNoRefresh(this);
                            }
                        }
                    }
                    nIndex--; // segment deleted decrement to point to last segment
                }
                if (nIndex==-1)
                {
                    // GetNext does not work correctly for (nIndex==-1)
                    nIndex = 0;
                    if (m_apSegments[independent]->IsEmpty())
                    {
                        return;    // nothing more to do
                    }
                }
                else
                {
                    nIndex = m_apSegments[independent]->GetNext(nIndex); // find next index
                }
            }
            // now change all the offsets of the following segment
            while (nIndex != -1)
            {
                DWORD dwOldOffset = m_apSegments[independent]->GetOffset(nIndex);
                DWORD dwOldDuration = m_apSegments[independent]->GetDuration(nIndex);
                ((CIndependentSegment *)m_apSegments[independent])->Adjust(this, nIndex, dwOldOffset - dwSectionLength, dwOldDuration);
                nIndex = m_apSegments[independent]->GetNext(nIndex); // find next index
            }
        }
    }
    else
    {
        // save new file size for later
        int nGlossIndex = m_apSegments[GLOSS]->GetOffsetSize() - 1;
        DWORD dwNewDataSize ;
        if (nGlossIndex != -1)   // we don't need the value if there is no gloss
        {
            dwNewDataSize = m_apSegments[GLOSS]->GetStop(nGlossIndex) + dwSectionLength;
        }

        for (int independent=0; independent<ANNOT_WND_NUMBER; independent++)
        {
            if (m_apSegments[independent]->GetMasterIndex() != -1)
            {
                continue;
            }

            m_apSegments[independent]->SetSelection(-1); // make sure nothing selected
            int nIndex = m_apSegments[independent]->GetPrevious(-1); // find last index (works if nothing selected)

            // now change all the offsets of the following segment
            while (nIndex != -1)
            {
                DWORD dwOldOffset = m_apSegments[independent]->GetOffset(nIndex);
                DWORD dwOldDuration = m_apSegments[independent]->GetDuration(nIndex);

                if (dwOldOffset < dwSectionStart)
                {
                    break;
                }

                ((CIndependentSegment *)m_apSegments[independent])->Adjust(this, nIndex, dwOldOffset + dwSectionLength, dwOldDuration);
                nIndex = m_apSegments[independent]->GetPrevious(nIndex); // find next index
            }

            // section in file added, adjust gloss duration and offsets
            nIndex = m_apSegments[independent]->FindFromPosition(dwSectionStart);
            if (nIndex != -1)
            {
                DWORD dwOldOffset = m_apSegments[independent]->GetOffset(nIndex);
                DWORD dwOldDuration = m_apSegments[independent]->GetDuration(nIndex);

                if (dwOldOffset <= dwSectionStart)
                {
                    // check if insertion into segment happened
                    DWORD dwOldCenter = dwOldOffset + dwOldDuration/2;

                    if (dwOldCenter <= dwSectionStart)
                    {
                        ((CIndependentSegment *)m_apSegments[independent])->Adjust(this, nIndex, dwOldOffset, dwSectionStart - dwOldOffset);
                    }
                    else
                    {
                        ((CIndependentSegment *)m_apSegments[independent])->Adjust(this, nIndex, dwSectionStart + dwSectionLength , dwOldOffset + dwOldDuration - dwSectionStart);
                    }
                }
            }
        }
    }
    // deselect everything
    for (int nLoop = 0; nLoop < ANNOT_WND_NUMBER; nLoop++)
    {
        if (m_apSegments[nLoop])
        {
            m_apSegments[nLoop]->SetSelection(-1);
        }
    }
}

/***************************************************************************/
// CSaDoc::OnAutoSnapUpdate Adjust all independent segments to snap boundaries
/***************************************************************************/
void CSaDoc::OnAutoSnapUpdate(void)
{
    CheckPoint();
    AutoSnapUpdate();
}

/***************************************************************************/
// CSaDoc::OnUpdateAutoSnapUpdate
/***************************************************************************/
void CSaDoc::OnUpdateAutoSnapUpdate(CCmdUI * pCmdUI)
{
    pCmdUI->Enable(AutoSnapUpdateNeeded());
}

/***************************************************************************/
// CSaDoc::OnAutoSnapUpdate Adjust all independent segments to snap boundaries
/***************************************************************************/
void CSaDoc::AutoSnapUpdate(void)
{
    POSITION pos = GetFirstViewPosition();
    CSaView * pView = (CSaView *)GetNextView(pos);

    for (int independent=0; independent<ANNOT_WND_NUMBER; independent++)
    {
        if (m_apSegments[independent]->GetMasterIndex() != -1)
        {
            continue;
        }

        CSegment * pIndependent = GetSegment(independent);


        if (pIndependent->IsEmpty())
        {
            continue;
        }

        int nLoop = 0;

        while (nLoop != -1)
        {
            UpdateSegmentBoundaries(TRUE, independent, nLoop, pIndependent->GetOffset(nLoop), pIndependent->GetStop(nLoop));
            nLoop = pIndependent->GetNext(nLoop);
        }
    }

    // refresh the annotation windows
    for (int nLoop = 0; nLoop < MAX_GRAPHS_NUMBER; nLoop++)
    {
        CGraphWnd * pGraph = pView->GetGraph(nLoop);
        if (pGraph)
        {
            for (int nInnerLoop = 0; nInnerLoop < ANNOT_WND_NUMBER; nInnerLoop++)
            {
                if (pGraph->HaveAnnotation(nInnerLoop))
                {
                    pGraph->GetAnnotationWnd(nInnerLoop)->Invalidate();    // redraw annotation window
                }
            }
            if (pGraph->HaveBoundaries())
            {
                pGraph->GetPlot()->Invalidate();
            }

        }
    }
}

/***************************************************************************/
// CSaDoc::AutoSnapUpdateNeeded Are all independent segments to snapped?
/***************************************************************************/
BOOL CSaDoc::AutoSnapUpdateNeeded(void)
{
    BOOL enable = FALSE;

    for (int independent=0; independent<ANNOT_WND_NUMBER; independent++)
    {
        if (m_apSegments[independent]->GetMasterIndex() != -1)
        {
            continue;
        }

        CSegment * pIndependent = GetSegment(independent);

        if (pIndependent->IsEmpty())
        {
            continue;
        }

        int nLoop = 0;

        while (nLoop != -1)
        {
            if (pIndependent->GetOffset(nLoop) != SnapCursor(START_CURSOR, pIndependent->GetOffset(nLoop)))
            {
                enable = TRUE;
                break;
            }
            nLoop = pIndependent->GetNext(nLoop);
        }
        if (enable)
        {
            break;
        }
    }

    return enable;
}

// SDM Added Function in Release 1.06.5
/***************************************************************************/
// CSaDoc::OnUpdateBoundaries
/***************************************************************************/
void CSaDoc::OnUpdateBoundaries(void)
{
    POSITION pos = GetFirstViewPosition();
    CSaView * pView = (CSaView *)GetNextView(pos);

    int nLoop = pView->FindSelectedAnnotationIndex();
    if (nLoop == -1)
    {
        return;
    }

    if (GetSegment(nLoop)->CheckCursors(this, pView->GetEditBoundaries(false)==BOUNDARIES_EDIT_SEGMENT_SIZE) != -1)
    {
        // save state for undo ability
        DWORD dwNewStart = pView->GetStartCursorPosition();
        DWORD dwNewStop = pView->GetStopCursorPosition();
        int nIndex = GetSegment(nLoop)->GetSelection();

        // first adjust cursors to old segment boundaries (undo to here)
        pView->SetStartCursorPosition( GetSegment(nLoop)->GetOffset(nIndex), SNAP_RIGHT);
        pView->SetStopCursorPosition( GetSegment(nLoop)->GetStop(nIndex), SNAP_LEFT);

        // If independent segment boundaries have moved they need to be updated for snapping
        if (((GetSegment(nLoop)->GetMasterIndex()==-1) &&
                ((pView->GetStartCursorPosition()!=GetSegment(nLoop)->GetOffset(nIndex)) ||
                 (pView->GetStopCursorPosition()!=(GetSegment(nLoop)->GetStop(nIndex))))))
        {
            // Phonetic Segment was not snapped - automatically update it
            pView->PostMessage(WM_COMMAND, ID_EDIT_AUTO_SNAP_UPDATE, 0L);
        }

        CheckPoint(); // Save state

        // Reload cursor locations to new segment boundaries
        pView->SetStartCursorPosition( dwNewStart, SNAP_LEFT);
        pView->SetStopCursorPosition( dwNewStop, SNAP_RIGHT);

        // Do update
        UpdateSegmentBoundaries(pView->GetEditBoundaries(false)==BOUNDARIES_EDIT_SEGMENT_SIZE);//SDM 1.5Test8.1
    }
}

// SDM Added Function in Release 1.06.5
/***************************************************************************/
// CSaDoc::OnUpdateUpdateBondaries
/***************************************************************************/
void CSaDoc::OnUpdateUpdateBoundaries(CCmdUI * pCmdUI)
{
    POSITION pos = GetFirstViewPosition();
    CSaView * pView = (CSaView *)GetNextView(pos);
    BOOL enable = FALSE;

    int nLoop = pView->FindSelectedAnnotationIndex();

    if ((nLoop != -1)&&(GetSegment(nLoop)->CheckCursors(this, pView->GetEditBoundaries(false)==BOUNDARIES_EDIT_SEGMENT_SIZE) != -1))
    {
        int nSelection = GetSegment(nLoop)->GetSelection();
        DWORD dwStart = pView->GetStartCursorPosition();
        DWORD dwStop = pView->GetStopCursorPosition();

        if ((GetSegment(nLoop)->GetMasterIndex()==-1)
                && ((dwStart != GetSegment(nLoop)->GetOffset(nSelection)) ||
                    (dwStop != GetSegment(nLoop)->GetStop(nSelection))))
        {
            enable = TRUE;
        }
        else
        {
            if (dwStart != SnapCursor(START_CURSOR, GetSegment(nLoop)->GetOffset(nSelection),0,GetDataSize(), SNAP_RIGHT))
            {
                enable = TRUE;
            }
            if (dwStop != SnapCursor(STOP_CURSOR, GetSegment(nLoop)->GetStop(nSelection),0,GetDataSize(),SNAP_LEFT))
            {
                enable = TRUE;
            }
        }
    }
    pCmdUI->Enable(enable);
}

// SDM Added Function in Release 1.06.1.2
// SDM 1.5Test8.1 added bOverlap
// SDM 1.5Test11.0 Modified to support change in gloss stop position
/***************************************************************************/
// CSaDoc::UpdateSegmentBondaries  Adjust selected segments to current cursor
// The function adjusts the current selected segment to the adjusted cursor
// positions.  The positions are snapped.  In the case of a gloss segment
// the cursors are aligned to the nearest phonetic segment
/***************************************************************************/
BOOL CSaDoc::UpdateSegmentBoundaries(BOOL bOverlap)
{
    POSITION pos = GetFirstViewPosition();
    CSaView * pView = (CSaView *)GetNextView(pos);
    CSegment * pSegment;
    int nSelection = -1;

    int nLoop = pView->FindSelectedAnnotationIndex();
    if (nLoop == -1) return FALSE;

    pSegment = m_apSegments[nLoop];
    nSelection = pSegment->GetSelection();

    DWORD dwStart = pView->GetStartCursorPosition();
    DWORD dwStop = pView->GetStopCursorPosition();

    BOOL result = UpdateSegmentBoundaries( bOverlap, nLoop, nSelection, dwStart, dwStop);

    // refresh the annotation windows
    for (nLoop = 0; nLoop < MAX_GRAPHS_NUMBER; nLoop++)
    {
        CGraphWnd * pGraph = pView->GetGraph(nLoop);
        if (pGraph)
        {
            for (int nInnerLoop = 0; nInnerLoop < ANNOT_WND_NUMBER; nInnerLoop++)
            {
                if (pGraph->HaveAnnotation(nInnerLoop))
                {
                    pGraph->GetAnnotationWnd(nInnerLoop)->Invalidate();    // redraw annotation window
                }
            }
            if (pGraph->HaveBoundaries())
            {
                pGraph->GetPlot()->Invalidate();
            }

        }
    }
    return result;
}


/***************************************************************************/
// CSaDoc::UpdateSegmentBondaries  Adjust selected segments to current cursor
// The function adjusts the current selected segment to the adjusted cursor
// positions.  The positions are snapped.  In the case of a gloss segment
// the cursors are aligned to the nearest phonetic segment
/***************************************************************************/
BOOL CSaDoc::UpdateSegmentBoundaries(BOOL bOverlap, int nAnnotation, int nSelection, DWORD dwNewOffset, DWORD dwNewStop)
{
    CSaDoc * pDoc = this;
    CSegment * pSegment;

    int nLoop = nAnnotation;
    if (nLoop == -1)
    {
        return FALSE;
    }

    pSegment = m_apSegments[nLoop];

    if (GetSegment(nLoop)->GetMasterIndex()==-1)
    {
        // Prepare for Update Boundaries
        CIndependentSegment * pSegment = (CIndependentSegment *) pDoc->GetSegment(nLoop); //SDM 1.5Test8.1

        DWORD dwOffset = pSegment->GetOffset(nSelection);
        DWORD dwStop = pSegment->GetStop(nSelection);


        dwNewOffset = SnapCursor(START_CURSOR, dwNewOffset);
        dwNewStop = SnapCursor(STOP_CURSOR, dwNewStop);

        if (dwNewOffset >= dwNewStop)
        {
            return FALSE;
        }

        //SDM 1.5Test8.1
        if ((dwNewOffset == dwOffset) && (dwNewStop == dwStop))
        {
            return FALSE;
        }

        if (!bOverlap)   // Move neighbors
        {
            int nPrevious = pSegment->GetPrevious(nSelection);
            int nNext = pSegment->GetNext(nSelection);
            if ((dwNewOffset != dwOffset) && (nPrevious != -1))
            {
                DWORD dwNewPreviousStop = SnapCursor(STOP_CURSOR, dwNewOffset);

                pSegment->Adjust(this, nPrevious,
                                 pSegment->GetOffset(nPrevious),
                                 dwNewPreviousStop - pSegment->GetOffset(nPrevious));
            }
            if (((dwNewStop) != (dwStop)) && (nNext != -1))
            {
                DWORD dwNewNextOffset = SnapCursor(START_CURSOR, dwNewStop);

                pSegment->Adjust(this, nNext,
                                 dwNewNextOffset,
                                 pSegment->GetStop(nNext) - (dwNewNextOffset));
            }
        }

        m_bModified = TRUE; // document has been modified

        pSegment->Adjust(this, nSelection, dwNewOffset, dwNewStop - dwNewOffset);
    }
    if (nLoop==GLOSS)
    {
        // gloss mode
        // Prepare for Update Boundaries
        DWORD dwOffset = pSegment->GetOffset(nSelection);
        DWORD dwStop = pSegment->GetStop(nSelection);

        int nNextSegment = pSegment->GetNext(nSelection);

        ((CDependentSegment *)pSegment)->AdjustPositionToMaster(pDoc, dwNewOffset, dwNewStop);

        if ((dwNewOffset == dwOffset) && (dwNewStop == dwStop))
        {
            return FALSE;
        }

        // Adjust Offset for current & Duration for previous segment & dependent segments
        ((CGlossSegment *)pSegment)->Adjust(pDoc, nSelection, dwNewOffset, dwNewStop - dwNewOffset);

        if (dwOffset!=dwNewOffset)
        {
            if (nSelection > 0)   // Adjust previous segment
            {
                ((CGlossSegment *)pSegment)->Adjust(pDoc, nSelection-1, pSegment->GetOffset(nSelection-1), ((CGlossSegment *)pSegment)->CalculateDuration(pDoc, nSelection-1));
            }
        }

        if ((dwStop)!=(dwNewStop))
        {
            if (nNextSegment != -1)
            {
                CSegment * pPhonetic = GetSegment(PHONETIC);
                DWORD dwNextStop = pSegment->GetStop(nNextSegment);

                DWORD dwNextOffset = pPhonetic->GetOffset(pPhonetic->GetNext(pPhonetic->FindStop(dwNewStop)));
                // Adjust offset for next and duration for previous segment & dependent segments
                ((CGlossSegment *)pSegment)->Adjust(pDoc, nNextSegment, dwNextOffset, dwNextStop - dwNextOffset);
            }
        }
    }
    if ((nLoop==TONE)||(nLoop==PHONEMIC)||(nLoop==ORTHO))
    {
        DWORD dwOffset = pSegment->GetOffset(nSelection);
        DWORD dwStop = pSegment->GetStop(nSelection);

        ((CDependentSegment *)pSegment)->AdjustPositionToMaster(pDoc, dwNewOffset, dwNewStop);

        if ((dwNewOffset == dwOffset) && (dwNewStop == dwStop))
        {
            return FALSE;
        }

        m_bModified = TRUE; // document has been modified

        pSegment->Adjust(this, nSelection, dwNewOffset, dwNewStop - dwNewOffset);
    }

    return TRUE;
}

/***************************************************************************/
// CSaDoc::GetDataSize
// Returns data size in bytes for a single channel
/***************************************************************************/
DWORD CSaDoc::GetDataSize() const
{
    return GetNumSamples() * m_FmtParm.GetSampleSize();
}

/***************************************************************************/
// CSaDoc::GetDataSize Return size of wave source data in bytes
// Returns the size of the wave file or of a workbench process temporary
// file according to the process selected.
// this is the length of the data in bytes for all channels.
/***************************************************************************/
DWORD CSaDoc::GetRawDataSize() const
{
    return m_dwDataSize;
}

/***************************************************************************/
// CSaDoc::GetWaveData Read wave data from wave source
// Reads a block of wave source data from the wave file or from a workbench
// process. If there is no process selected, it takes the wave data directly
// from the wave file, otherwise from the workbench process.
// dwOffset is the sample index
/***************************************************************************/
HPSTR CSaDoc::GetWaveData(DWORD dwOffset, BOOL bBlockBegin)
{
    //TRACE("GetWaveData %d %d\n",dwOffset,bBlockBegin);
    if (m_nWbProcess>0)
    {
        CProcess * pWbProcess = ((CMainFrame *)AfxGetMainWnd())->GetWbProcess(m_nWbProcess - 1, 0);
        if (pWbProcess!=NULL)
        {
            return pWbProcess->GetProcessedWaveData(dwOffset, bBlockBegin);
        }
    }
    return m_pProcessAdjust->GetProcessedWaveData(dwOffset, bBlockBegin);
}

/***************************************************************************/
// CSaDoc::GetWaveData Read wave data from wave source
// Reads a block of wave source data from the wave file or from a workbench
// process. If there is no process selected, it takes the wave data directly
// from the wave file, otherwise from the workbench process.
/***************************************************************************/
DWORD CSaDoc::GetWaveDataBufferSize()
{

    if (m_nWbProcess>0)
    {
        CProcess * pWbProcess = ((CMainFrame *)AfxGetMainWnd())->GetWbProcess(m_nWbProcess - 1, 0);
        if (pWbProcess!=NULL)
        {
            return pWbProcess->GetProcessedWaveDataBufferSize();
        }
    }
    return m_pProcessAdjust->GetProcessedWaveDataBufferSize();
}

/***************************************************************************/
// CSaDoc::GetWaveData Read wave data
// Returns just one sample out of the temporary wave file at the given
// offset. This function uses the block reading GetWaveData function to read
// the wave data into the read buffer and then returns one sample from the
// buffer. The caller gives a pointer to a BOOLEAN result variable. If this
// is FALSE after the operation, an error occured.
/***************************************************************************/
int CSaDoc::GetWaveData( DWORD dwOffset, BOOL * pbRes)
{
    HPSTR pData = GetWaveData(dwOffset); // read from file into buffer if necessary
    if (pData==NULL)
    {
        // error reading wave file
        *pbRes = FALSE; // set operation result
        return 0;
    }
    // read sample
    pData += (dwOffset - GetWaveBufferIndex());
    BYTE bData;
    int nData;
    if (m_FmtParm.wBlockAlign == 1)   // 8 bit data
    {
        bData = *pData; // data range is 0...255 (128 is center)
        nData = bData - 128;
    }
    else
    {
        nData = *((short int *)pData);    // 16 bit data
    }
    return nData;
}

/***************************************************************************/
// CSaDoc::GetUnprocessedWaveData Read wave data
// Reads a block of wave data from the temporary wave file into the data
// buffer and returns the pointer to the data. The user gives the data offset
// (byte number) in the file to tell what data he needs. If bBlockBegin is
// set, this given offset data will always be at the top of the data block
// (where the returned pointer points to). If bBlockBegin is not set, the
// data block is not reread, if the given offset data is already somewhere
// in the buffer, and only the actual pointer to the data block will be
// returned. In case of error NULL will be returned.
/***************************************************************************/
HPSTR CSaDoc::GetAdjustedUnprocessedWaveData(DWORD dwOffset)
{
    TRACE("GetunprocessedWaveData1 %d\n",dwOffset);
    return m_pProcessAdjust->GetProcessedWaveData(dwOffset, FALSE);
}

/***************************************************************************/
// CSaDoc::GetWaveBufferIndex Return index for wave source data buffer
// Returns the index of the wave file read buffer or of a workbench process
// buffer according to the process selected.
/***************************************************************************/
DWORD CSaDoc::GetWaveBufferIndex()
{
    if (m_nWbProcess>0)
    {
        CProcess * pWbProcess = ((CMainFrame *)AfxGetMainWnd())->GetWbProcess(m_nWbProcess - 1, 0);
        if (pWbProcess!=NULL)
        {
            return pWbProcess->GetProcessBufferIndex();
        }
    }
    return m_pProcessAdjust->GetProcessBufferIndex();
}

/***************************************************************************/
// CSaDoc::GetMeasurementsString Returns a tab-delimited string containing
// data at a given offset.
/***************************************************************************/
CSaString CSaDoc::GetMeasurementsString(DWORD dwOffset, DWORD dwLength, BOOL * pbRes)
{
    CSaString szMeasurement = _T("");
    CSaString szFilename;
    CSaString szSpeaker;
    CSaString szGender;
    CSaString szTimeStart;
    CSaString szDuration = _T("");
    CSaString szF0Hz;
    CSaString szF0st;
    CSaString szIntensity;
    CSaString szF1;
    CSaString szF2;
    CSaString szF3;
    CSaString szF4;
    *pbRes = TRUE;
    int nSmpSize = m_FmtParm.wBlockAlign;

    // process the necessary data
    short int nResult1 = LOWORD(m_pProcessGrappl->Process(this, this));
    short int nResult2 = LOWORD(m_pProcessLoudness->Process(this, this));
    short int nResult3 = LOWORD(m_pProcessFormantTracker->Process(this, this));
    if (nResult1 < 0 || nResult2 < 0 || nResult3 < 0)   // processing error
    {
        *pbRes = FALSE;
        return szMeasurement;
    }

    // get filename
    szFilename = GetFilenameFromTitle().c_str(); // get the current view caption string
    int nFind = szFilename.Find(_T(" (Read-Only)"));
    if (nFind != -1)
    {
        szFilename = szFilename.Left(nFind);    // extract part left of first space
    }

    // get speaker
    szSpeaker = GetSourceParm()->szSpeaker;

    // get gender
    CSaString szGenderArray[3] = {_T("Male"), _T("Female"), _T("Child")};
    szGender = szGenderArray[GetGender()];

    // get time
    szTimeStart.Format(_T("%.4f"), (float)GetTimeFromBytes(dwOffset));
    if (dwLength > 0)
    {
        szDuration.Format(_T("%.4f"), (float)GetTimeFromBytes(dwLength));
    }

    // get pitch
    DWORD dwStartPos = (dwOffset / nSmpSize) / Grappl_calc_intvl;
    DWORD dwEndPos = ((dwOffset + dwLength) / nSmpSize) / Grappl_calc_intvl;
    double fData;
    double fSum = 0;
    int count = 0;
    for (DWORD i = dwStartPos; i <= dwEndPos; i++)
    {
        fData = (float)m_pProcessGrappl->GetProcessedData(i, pbRes) / (float)PRECISION_MULTIPLIER;
        if (fData > 0.0)
        {
            fSum += fData;
            count++;
        }
    }

    if (pbRes && (count > 0))
    {
        szF0Hz.Format(_T("%.1f"), fSum / count);
        szF0st.Format(_T("%.1f"), CGraphWnd::GetSemitone(fSum / count));
    }
    else
    {
        szF0Hz = _T("-");
        szF0st = _T("-");
    }

    // get intensity
    double fSizeFactor = (double)nSmpSize * ceil((double)(GetDataSize() / nSmpSize) / (double)(m_pProcessLoudness->GetDataSize()));
    dwStartPos = (DWORD)((float)dwOffset / fSizeFactor);
    dwEndPos = (DWORD)((float)(dwOffset + dwLength) / fSizeFactor);
    int nRawData;
    fSum = 0;
    count = 0;
    for (DWORD i = dwStartPos; i <= dwEndPos; i++)
    {
        nRawData = m_pProcessLoudness->GetProcessedData(i, pbRes);
        if (nRawData > 0.)
        {
            fSum += (double)nRawData;
            count++;
        }
    }
    if (pbRes && (count > 0))
    {
        szIntensity.Format(_T("%.2f"), 20. * log10(fSum / count / 32767.) + 6.);
    }
    else
    {
        szIntensity = _T("-");
    }

    // get formants
    double dBytesPerSlice = GetDataSize() / double(m_pProcessFormantTracker->GetDataSize(sizeof(SFormantFreq)));
    dwStartPos = (DWORD) floor(dwOffset / dBytesPerSlice);
    dwEndPos = (DWORD) floor((dwOffset + dwLength) / dBytesPerSlice);
    double fF1Sum = 0;
    double fF2Sum = 0;
    double fF3Sum = 0;
    double fF4Sum = 0;
    count = 0;
    for (DWORD i = dwStartPos; i <= dwEndPos; i++)
    {
        CProcessIterator<SFormantFreq> iterFormants(*m_pProcessFormantTracker, i);
        if ((*iterFormants).F[1] != (double)NA)
        {
            fF1Sum += (*iterFormants).F[1];
            fF2Sum += (*iterFormants).F[2];
            fF3Sum += (*iterFormants).F[3];
            fF4Sum += (*iterFormants).F[4];
            count++;
        }
    }
    if (count > 0)
    {
        szF1.Format(_T("%.1f"), fF1Sum / count);
        szF2.Format(_T("%.1f"), fF2Sum / count);
        szF3.Format(_T("%.1f"), fF3Sum / count);
        szF4.Format(_T("%.1f"), fF4Sum / count);
    }
    else
    {
        szF1 = _T("-");
        szF2 = _T("-");
        szF3 = _T("-");
        szF4 = _T("-");
    }

    // format the data in a tab-delimited string
    CSaString szTab = _T("\t");
    szMeasurement = _T("Filename\tSpeaker\tGender\tStart (sec)\tDuration (sec)\tf0 (Hz)\tf0 (st)\tIntensity (dB)\t");
    szMeasurement += _T("F1 (Hz)\tF2 (Hz)\tF3 (Hz)\tF4 (Hz)\n");
    szMeasurement += szFilename + szTab + szSpeaker + szTab + szGender + szTab + szTimeStart + szTab + szDuration + szTab;
    szMeasurement += szF0Hz + szTab + szF0st + szTab + szIntensity + szTab + szF1 + szTab + szF2 + szTab + szF3 + szTab + szF4;

    return szMeasurement;
}

/////////////////////////////////////////////////////////////////////////////
// CSaDoc serialization
//
// This is not currently used.

void CSaDoc::Serialize(CArchive &)
{
}

/////////////////////////////////////////////////////////////////////////////
// CSaDoc::SerializeForUndoRedo
/////////////////////////////////////////////////////////////////////////////
CArchive & operator<< (CArchive & ar, const SourceParm & parm)
{
    ar << parm.szLanguage;         // language name
    ar << parm.szEthnoID;          // ethnologue ID
    ar << parm.szDialect;          // dialect
    ar << parm.szRegion;           // region
    ar << parm.szCountry;          // country
    ar << parm.szFamily;           // family
    ar << parm.szSpeaker;          // speaker name
    ar << parm.szReference;        // notebook reference
    ar << parm.szFreeTranslation;   // free translation
    ar << parm.szTranscriber;       // transcriber
    ar << long(parm.nGender);            // speaker gender
    return ar;
}

CArchive & operator>> (CArchive & ar, SourceParm & parm)
{
    ar >> parm.szLanguage;         // language name
    ar >> parm.szEthnoID;          // ethnologue ID
    ar >> parm.szDialect;          // dialect
    ar >> parm.szRegion;           // region
    ar >> parm.szCountry;          // country
    ar >> parm.szFamily;           // family
    ar >> parm.szSpeaker;          // speaker name
    ar >> parm.szReference;        // notebook reference
    ar >> parm.szFreeTranslation;   // free translation
    ar >> parm.szTranscriber;       // transcriber
    long gender;
    ar >> gender;            // speaker gender
    parm.nGender = (int) gender;
    return ar;
}

CArchive & operator<< (CArchive & ar, const SaParm & parm)
{
    ar << parm.szDescription;      // file description
    ar << parm.dwNumberOfSamples;  // number of samples in wave data
    ar << parm.lSignalMax;         // max signal value
    ar << parm.lSignalMin;         // min signal value

    return ar;
}

CArchive & operator>> (CArchive & ar, SaParm & parm)
{
    ar >> parm.szDescription;      // file description
    ar >> parm.dwNumberOfSamples;  // number of samples in wave data
    ar >> parm.lSignalMax;         // max signal value
    ar >> parm.lSignalMin;         // min signal value
    return ar;
}

void CSaDoc::SerializeForUndoRedo(CArchive & ar)
{
    CSegment * pDummySeg = (CSegment *)new CPhoneticSegment(PHONETIC);
    // get pointer to view
    POSITION pos = GetFirstViewPosition();
    CSaView * pView = (CSaView *)GetNextView(pos);

    if (pDummySeg && ar.IsStoring())
    {
        CArchiveTransfer::tDWORD(ar, pView->GetStartCursorPosition());
        CArchiveTransfer::tDWORD(ar, pView->GetStopCursorPosition());
        for (int nLoop = 0; nLoop < ANNOT_WND_NUMBER; nLoop++)
        {
            if (m_apSegments[nLoop] && m_apSegments[nLoop]->GetOffsetSize())
            {
                ar << CSaString("7");
                m_apSegments[nLoop]->Serialize(ar);
            }
            else if (m_apSegments[nLoop])
            {
                ar << CSaString("D");
            }
            else
            {
                ar << CSaString("3");
            }
        }
        ar << (BYTE)m_bWaveUndoNow;
        ar << (WORD)m_nCheckPointCount;
        // SDM 1.06.6U4 Undo modified status
        ar << (BYTE)m_bModified;
        ar << m_sourceParm;
        ar << m_saParm;
    }
    else if (pDummySeg)
    {
        CSaString isValid;

        pView->SetStartCursorPosition(CArchiveTransfer::tDWORD(ar));
        pView->SetStopCursorPosition(CArchiveTransfer::tDWORD(ar));
        for (int nLoop = 0; nLoop < ANNOT_WND_NUMBER; nLoop++)
        {
            ar >> isValid;
            if (isValid=="7")
            {
                if (m_apSegments[nLoop])
                {
                    m_apSegments[nLoop]->Serialize(ar);
                }
                else
                {
                    pDummySeg->Serialize(ar);
                }
            }
            else if (isValid=="D")
            {
                if (m_apSegments[nLoop])
                {
                    m_apSegments[nLoop]->DeleteContents();
                }
            }
        }
        ar >> (BYTE &)m_bWaveUndoNow;
        ar >> (WORD &)m_nCheckPointCount;
        // SDM 1.06.6U4 Undo modified status
        ar >> (BYTE &)m_bModified;
        ar >> m_sourceParm;
        ar >> m_saParm;
    }

    if (pDummySeg)
    {
        delete pDummySeg;
    }
}

/////////////////////////////////////////////////////////////////////////////
// CSaDoc diagnostics

#ifdef _DEBUG
void CSaDoc::AssertValid() const
{
    CDocument::AssertValid();
}

void CSaDoc::Dump(CDumpContext & dc) const
{
    CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CSaDoc message handlers

/***************************************************************************/
// CSaDoc::DoFileSave Save file
// Before using the framework original save functions we have to change the
// read only file status to read and write and put it back after the saving.
// If the file path is empty, CDocument will call save as routine to save
// the document. So we have to be modify the title string and reset to its
// original state (the view has added the graphs title), because this title
// is used to generate the new default filename. After that the view title
// has again to be modified with the graph title.
/***************************************************************************/
BOOL CSaDoc::DoFileSave()
{
    // SDM 1.5Test10.2
    if ((m_fileStat.m_szFullName[0] == 0) || !(IsModified() || m_nWbProcess))
    {
        // OnFileSaveDisabled
        // SDM 1.5Test10.2
        return FALSE;
    }

    BOOL bResult = FALSE;
    // SDM moved process WAV save to Save As 1.5Test8.2
    CSaString szCaption, szGraphTitle;
    // change the file attribute to allow write operation
    CSaString szPathName = GetPathName();

    // audio wasn't modified so just save the transcription
    if (!IsAudioModified())
    {
        return OnSaveDocument(szPathName, FALSE);
    }

    // this isn't a wave file, so let the user Save As...
    if (m_bUsingTempFile)
    {
        OnFileSaveAs();
        return TRUE;
    }

    BOOL bSaveAudio = TRUE;
    if (!szPathName.IsEmpty())
    {
        if (SetFileAttributes(szPathName, (DWORD)m_fileStat.m_attribute))
        {
			if (::IsReadOnly( szPathName))
            {
                switch (AfxMessageBox(IDS_QUESTION_READ_ONLY, MB_YESNOCANCEL))
                {
                case IDYES:
                    OnFileSaveAs();
                    break;
                case IDNO:
                    bSaveAudio = FALSE;
                    bResult = OnSaveDocument(szPathName, bSaveAudio);
                    break;
                case IDCANCEL:
                    return FALSE;
                }
            }
            else
            {
                bResult = OnSaveDocument(szPathName, bSaveAudio);
            }
        }
        else
        {
            szPathName.Empty();
        }
    }
    if (szPathName.IsEmpty())
    {
        // the pathname is empty, reset view title string
        CSaString fileName = GetTitle();        // get the current view caption string
        int nFind = fileName.Find(':');
        if (nFind != -1)
        {
            szGraphTitle = fileName.Mid(nFind);
            fileName = fileName.Left(nFind);    // extract part left of :
			fileName = fileName.Trim();
        }

        CFileDialog dlg(FALSE, _T("wav"), fileName, OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT, _T("WAV Files (*.wav)|*.wav||"));

        CSaString szDefault = static_cast<CSaApp *>(AfxGetApp())->DefaultDir(); // need to save copy (return value is destroyed)
        dlg.m_ofn.lpstrInitialDir = szDefault;

        if (dlg.DoModal()!=IDOK)
        {
            return FALSE;
        }
        else
        {
            szCaption = dlg.GetFileTitle();
            szPathName = dlg.GetPathName();
        }

        CFileStatus status;
        if (CFile::GetStatus(szPathName, status))
        {
            CFile::SetStatus(szPathName, status);
        }

        bResult = OnSaveDocument(szPathName, bSaveAudio);
        if (bResult)
        {
            SetPathName(szPathName);
        }

        if (!szGraphTitle.IsEmpty())
        {
            // set back the title string
            szCaption += szGraphTitle; // add the graph title
            SetTitle(szCaption); // write the new caption string
        }
    }

    // change the file attribute to read only
    szPathName = GetPathName();
    if (bSaveAudio && !szPathName.IsEmpty())
    {
        CFile::SetStatus(szPathName, m_fileStat);
    }

    return bResult;
}

/***************************************************************************/
// CSaDoc::OnUpdateFileSave Menu Update
/***************************************************************************/
void CSaDoc::OnUpdateFileSave(CCmdUI * pCmdUI)
{
    if (m_fileStat.m_szFullName[0] == 0)
    {
        pCmdUI->Enable(FALSE);    // no path name provided
    }
    else
    {
        pCmdUI->Enable((IsModified() || (m_nWbProcess)));    // path name ok, enable if dirty or processed
    }
}

// SDM 1.5Test8.1
/***************************************************************************/
// CSaDoc::OnFileSaveAs SaveAS file
// Saves file to new name. Sets file read only file status.
// If a workbench process has been selected, wave data comes
// from a data process. The wave temporary file will be deleted and the
// process temporary file will become the new wave temporary file. The
// process data will be invalidated and the process switched back to plain.
/***************************************************************************/
void CSaDoc::OnFileSaveAs()
{

    CSaString fileName = GetPathName();
    if (fileName.IsEmpty())
    {
        fileName = GetFilenameFromTitle().c_str(); // get the current view caption string
    }

    CSaString fileExt = _T(".wav");
    fileName = SetFileExtension(fileName, fileExt);

    CDlgSaveAsOptions dlg(_T("wav"), fileName, OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT, _T("WAV Files (*.wav)|*.wav||"));

    CSaString szDefault = static_cast<CSaApp *>(AfxGetApp())->DefaultDir(); // need to save copy (return value is destroyed)
    dlg.m_ofn.lpstrInitialDir = szDefault;

    if (dlg.DoModal()!=IDOK)
    {
        return;
    }

    fileName = dlg.GetPathName();

    BOOL bSameFileName = FALSE;
    if (fileName == GetPathName())
    {
        bSameFileName = TRUE;
        dlg.m_nShowFiles = CDlgSaveAsOptions::showNew; // There is only one file.
    }
    else if (static_cast<CSaApp *>(AfxGetApp())->IsFileOpened(fileName))
    {
        // error file already opened by SA
        static_cast<CSaApp *>(AfxGetApp())->ErrorMessage(IDS_ERROR_FILEOPENED, fileName);
        return;
    }

    CFileStatus status;
    if (CFile::GetStatus(fileName, status))
    {
        // File exists overwrite existing file
        try
        {
            CFile::SetStatus(fileName, status);
        }
        catch (...)
        {
            ((CSaApp *) AfxGetApp())->ErrorMessage(IDS_ERROR_FILEWRITE, fileName);
            return;
        }
    }

    BeginWaitCursor();

    BOOL bSuccess = FALSE;

    if (dlg.m_nSaveArea == CDlgSaveAsOptions::saveCursors)
    {
        CSaDoc * pDoc = this;
        POSITION pos = pDoc->GetFirstViewPosition();
        CSaView * pView = (CSaView *)pDoc->GetNextView(pos);
        CURSORPOS dwStart = pView->GetStartCursorPosition();
        CURSORPOS dwStop = pView->GetStopCursorPosition();
		WAVETIME start = pDoc->toTime( (CURSORPOS)dwStart);
		WAVETIME stop = pDoc->toTime( (CURSORPOS)dwStop);
        bSuccess = CopySectionToNewWavFile( start, stop-start,fileName, TRUE);
    }
    else if (dlg.m_nSaveArea == CDlgSaveAsOptions::saveView)
    {
        CSaDoc * pDoc = this;
        POSITION pos = pDoc->GetFirstViewPosition();
        CSaView * pView = (CSaView *)pDoc->GetNextView(pos);
        DWORD dwStart;
        DWORD dwFrame;
        pView->GetDataFrame(dwStart, dwFrame);

        if (pDoc->GetBitsPerSample() == 16)
        {
            dwFrame &= ~0x1; // force even if file is 16 bit.
        }
        bSuccess = CopySectionToNewWavFile(dwStart,dwFrame,fileName, TRUE);
    }
    else if (dlg.m_nShowFiles != CDlgSaveAsOptions::showNew)
    {
		WAVETIME start = 0;
		WAVETIME length = toTime( GetDataSize(), true);
        bSuccess = CopySectionToNewWavFile( start, length, fileName, TRUE);
    }
    else
    {
        if (bSameFileName)
        {
            SetModifiedFlag();			// Force SA to update file format
            SetAudioModifiedFlag();
            SetTransModifiedFlag(TRUE);
            OnFileSave();
            return;
        }
        // get graph title string
        CSaString szGraphTitle = GetTitle();      // get the current view caption string
        int nFind = szGraphTitle.Find(':');
        if (nFind != -1)
        {
            szGraphTitle = szGraphTitle.Right(szGraphTitle.GetLength() - nFind); // extract part right of and with :
        }
        else
        {
            szGraphTitle = _T("");
        }

        CSaString sourceFileName = ((!m_bUsingTempFile) ? GetPathName() : m_szTempConvertedWave);

        if (sourceFileName.GetLength()>0)
		{
			if (!CopyWave( sourceFileName, fileName))    // SDM 1.5Test10.2
			{
				// error copying file
				((CSaApp *) AfxGetApp())->ErrorMessage(IDS_ERROR_FILEWRITE, fileName);
				EndWaitCursor();
				return;
			}
        }

        BOOL bSaveAudio = TRUE;
        bSuccess = OnSaveDocument(fileName, bSaveAudio);

        if (bSuccess)
        {
            // Change the document name
            SetPathName(fileName);
            // set back the title string
            CSaString szCaption = GetTitle(); // get the current view caption string
            szCaption += szGraphTitle; // add the graph title
            SetTitle(szCaption); // write the new caption string
        }
    }

    if (!bSuccess)
    {
        // be sure to delete the file
        RemoveFile(fileName);
        EndWaitCursor();
        return;
    }

    // change the file attribute to read only
    if (_tcslen(fileName) > 0)
    {
        CFile::SetStatus(fileName, m_fileStat);
    }

    if (dlg.m_nShowFiles == CDlgSaveAsOptions::showBoth)
    {
        AfxGetApp()->OpenDocumentFile(fileName); // Open new document
    }
    if (dlg.m_nShowFiles == CDlgSaveAsOptions::showNew && fileName != GetPathName())
    {
        AfxGetApp()->OpenDocumentFile(fileName); // Open new document
        OnCloseDocument();  // Close Original
    }

    EndWaitCursor();

}

/***************************************************************************/
// CSaDoc::OnUpdateFileSaveAs Menu Update
/***************************************************************************/
void CSaDoc::OnUpdateFileSaveAs(CCmdUI * pCmdUI)
{
    pCmdUI->Enable(!((CSaApp *)AfxGetApp())->GetBatchMode() && (!m_bMultiChannel)); // SDM 1.5Test8.2
}

/***************************************************************************/
// CSaDoc::OnFileSplitFile
// Splits a file based on user defined keys such as reference or gloss
/***************************************************************************/
void CSaDoc::OnFileSplitFile()
{

    CString fileName = GetPathName();
    wchar_t buffer[MAX_PATH];
    swprintf_s(buffer,_countof(buffer),fileName);
    wchar_t drive[_MAX_DRIVE];
    wchar_t dir[_MAX_DIR];
    wchar_t fname[_MAX_FNAME];
    wchar_t ext[_MAX_EXT];
    _wsplitpath_s(buffer, drive, dir, fname, ext);

    CString homeDefault = AfxGetApp()->GetProfileString(_T(""), _T("DataLocation"));
    if (homeDefault.GetLength()==0)
    {
        homeDefault.Format(L"%s%s",drive,dir);
    }
    if (homeDefault.Right(1) != _T("\\"))
    {
        homeDefault += _T("\\");
    }

    CDlgSplitFile dlg;

    dlg.m_szFolderLocation = AfxGetApp()->GetProfileString(L"SplitFile",L"Home",(LPCTSTR)homeDefault);
    dlg.m_szFolderName.Format(L"Split-%s",fname);
    dlg.m_szFolderName = FilterName((LPCTSTR)dlg.m_szFolderName).c_str();
    dlg.m_szPhraseFolderName.Format(L"Split-%s-Phrase",fname);
    dlg.m_szPhraseFolderName = FilterName((LPCTSTR)dlg.m_szPhraseFolderName).c_str();
    dlg.m_szGlossFolderName.Format(L"Split-%s-Data",fname);
    dlg.m_szGlossFolderName = FilterName((LPCTSTR)dlg.m_szGlossFolderName).c_str();
    dlg.m_bOverwriteData = (AfxGetApp()->GetProfileInt(L"SplitFile",L"OverwriteData",0)!=0);
    dlg.m_bSkipGlossEmpty = (AfxGetApp()->GetProfileInt(L"SplitFile",L"SkipEmptyGloss",1)!=0);
    dlg.SetWordFilenameConvention(AfxGetApp()->GetProfileIntW(L"SplitFile",L"WordConvention",2));
    dlg.SetPhraseFilenameConvention(AfxGetApp()->GetProfileIntW(L"SplitFile",L"PhraseConvention",2));

    bool retry = false;

    wstring newPath;
    wstring glossPath;
    wstring phrasePath;
    int offsetSize;
    bool hasGloss;

    CSaApp * pApp = (CSaApp *)AfxGetApp();
    POSITION pos = 0;
    CSaView * pView = NULL;

    do
    {
        retry = false;

        if (dlg.DoModal()!=IDOK)
        {
            return;
        }

        AfxGetApp()->WriteProfileInt(L"SplitFile",L"WordConvention",(dlg.GetWordFilenameConvention()));
        AfxGetApp()->WriteProfileInt(L"SplitFile",L"PhraseConvention",(dlg.GetPhraseFilenameConvention()));
        AfxGetApp()->WriteProfileInt(L"SplitFile",L"OverwriteData",(dlg.m_bOverwriteData)?1:0);
        AfxGetApp()->WriteProfileInt(L"SplitFile",L"SkipEmptyGloss",(dlg.m_bSkipGlossEmpty)?1:0);
        AfxGetApp()->WriteProfileString(L"SplitFile",L"Home",dlg.m_szFolderLocation);

        pos = GetFirstViewPosition();
        pView = (CSaView *)GetNextView(pos);

        // we need a focused graph!
        if (pView->GetFocusedGraphWnd()==NULL)
        {
            pApp->ErrorMessage(IDS_SPLIT_NO_SELECTION);
            return;
        }

        // key off of gloss for now
        offsetSize = pView->GetAnnotation(GLOSS)->GetOffsetSize();
        hasGloss = (offsetSize!=0);

        newPath = L"";
        glossPath = L"";
        phrasePath = L"";

        newPath.append(dlg.m_szFolderLocation).append(dlg.m_szFolderName);
        glossPath.append(dlg.m_szFolderLocation).append(dlg.m_szFolderName).append(L"\\").append(dlg.m_szGlossFolderName);
        phrasePath.append(dlg.m_szFolderLocation).append(dlg.m_szFolderName).append(L"\\").append(dlg.m_szPhraseFolderName);

        // check the for preexistence of a file with the same name as the folder.
        if (FileExists(glossPath.c_str()))
        {
            pApp->ErrorMessage(IDS_SPLIT_FILE_EXISTS,glossPath.c_str());
            return;
        }

        if (FileExists(phrasePath.c_str()))
        {
            pApp->ErrorMessage(IDS_SPLIT_FILE_EXISTS,phrasePath.c_str());
            return;
        }

        if (!dlg.m_bOverwriteData)
        {
            // check the for preexistence of the folders.
            if (FolderExists(newPath.c_str()))
            {
                CString msg;
                msg.FormatMessage(IDS_SPLIT_FOLDER_EXISTS,newPath.c_str());
                int result = AfxMessageBox(msg, MB_ABORTRETRYIGNORE | MB_ICONQUESTION, 0);
                if (result == IDABORT)
                {
                    return;
                }
                if (result == IDRETRY)
                {
                    retry=true;
                }
            }
            else if (FolderExists(glossPath.c_str()))
            {
                CString msg;
                msg.FormatMessage(IDS_SPLIT_FOLDER_EXISTS,glossPath.c_str());
                int result = AfxMessageBox(msg, MB_ABORTRETRYIGNORE | MB_ICONQUESTION, 0);
                if (result == IDABORT)
                {
                    return;
                }
                if (result == IDRETRY)
                {
                    retry=true;
                }
            }
            else if (FolderExists(phrasePath.c_str()))
            {
                CString msg;
                msg.FormatMessage(IDS_SPLIT_FOLDER_EXISTS,phrasePath.c_str());
                int result = AfxMessageBox(msg, MB_ABORTRETRYIGNORE | MB_ICONQUESTION, 0);
                if (result == IDABORT)
                {
                    return;
                }
                if (result == IDRETRY)
                {
                    retry=true;
                }
            }
        }
    }
    while (retry);

    // create the folders.
    if (!CreateFolder(newPath.c_str()))
    {
        pApp->ErrorMessage(IDS_SPLIT_BAD_DIRECTORY,newPath.c_str());
        return;
    }
    if (!CreateFolder(glossPath.c_str()))
    {
        pApp->ErrorMessage(IDS_SPLIT_BAD_DIRECTORY,glossPath.c_str());
        return;
    }
    if (!CreateFolder(phrasePath.c_str()))
    {
        pApp->ErrorMessage(IDS_SPLIT_BAD_DIRECTORY,phrasePath.c_str());
        return;
    }

    int dataCount=0;
    int wavCount=0;
    BeginWaitCursor();

    EWordFilenameConvention wordConvention = dlg.GetWordFilenameConvention();
    EPhraseFilenameConvention phraseConvention = dlg.GetPhraseFilenameConvention();

    if ((hasGloss) || (!dlg.m_bSkipGlossEmpty))
    {
        if (!ExportWordSegments( wordConvention, glossPath.c_str(), dlg.m_bSkipGlossEmpty, dataCount, wavCount, dlg.m_szFilenamePrefix, dlg.m_szFilenameSuffix))
        {
            EndWaitCursor();
            return;
        }
    }

    if (!ExportPhraseSegments( MUSIC_PL1, phraseConvention, phrasePath, dataCount, wavCount, dlg.m_szFilenamePrefix, dlg.m_szFilenameSuffix))
    {
        EndWaitCursor();
        return;
    }

    if (!ExportPhraseSegments( MUSIC_PL2, phraseConvention, phrasePath, dataCount, wavCount, dlg.m_szFilenamePrefix, dlg.m_szFilenameSuffix))
    {
        EndWaitCursor();
        return;
    }

    EndWaitCursor();

    int count=dataCount+wavCount;

    if (count==0)
    {
        pApp->ErrorMessage(IDS_SPLIT_INCOMPLETE);
    }
    else
    {
        CString szText;
        wchar_t szNumber[128];
        swprintf_s(szNumber,_countof(szNumber),L"%d",count);
        pApp->Message(IDS_SPLIT_COMPLETE,szNumber);
    }
}

/***************************************************************************/
// CSaDoc::OnUpdateFileSplit Menu Update
/***************************************************************************/
void CSaDoc::OnUpdateFileSplit(CCmdUI * pCmdUI)
{
    pCmdUI->Enable(!((CSaApp *) AfxGetApp())->GetBatchMode() && (!m_bMultiChannel));
}

/***************************************************************************/
// CSaDoc::CopyProcessTempFile
// Copies either the Waveform Adjust process or Workbench process temp file
// over the raw waveform temp file to prepare for saving.
/***************************************************************************/
void CSaDoc::CopyProcessTempFile()
{
    // check for Waveform Adjust process
    LPCTSTR pszAdjustTempPath = m_pProcessAdjust->GetProcessFileName();
    BOOL bAdjust = (pszAdjustTempPath[0] != 0) && (pszAdjustTempPath != NULL);

    // check for Workbench process
    CProcess * pWbProcess = NULL;
    // fallback to adjust temp file
    LPCTSTR pszWBTempPath = pszAdjustTempPath;
    if (m_nWbProcess>0)
    {
        CMainFrame * pMain = (CMainFrame *)AfxGetMainWnd();
        pWbProcess = pMain->GetWbProcess(m_nWbProcess - 1, 0);
        if (pWbProcess!=NULL)
        {
            // get processed temporary file name
            pszWBTempPath = pWbProcess->GetProcessFileName();
        }
    }

    if (bAdjust || m_nWbProcess)
    {
        LPCTSTR pszProcTempPath = (m_nWbProcess ? pszWBTempPath : pszAdjustTempPath); // Workbench temp file takes precedence
        try
        {
            // copy the process temp file as the temporary wave file
            if (!m_szRawDataWrk.empty())
            {
                CopyWave( pszProcTempPath, m_szRawDataWrk.c_str());
            }
        }
        catch (CFileException e)
        {
            // error copying wave file
            CSaApp * pApp = (CSaApp *)AfxGetApp();
            pApp->ErrorMessage(IDS_ERROR_WRITETEMPFILE, m_szRawDataWrk.c_str());
        }

        // get file information
        m_dwDataSize = GetFileSize(m_szRawDataWrk.c_str());
        m_pProcessAdjust->SetInvert(FALSE);
        m_pProcessAdjust->SetNormalize(FALSE);
        m_pProcessAdjust->SetZero(FALSE);

        // delete the workbench process
        if (pWbProcess)
        {
            RemoveFile(pszProcTempPath);
            pWbProcess->DeleteProcessFileName();
            pWbProcess->SetDataInvalid();
        }
        m_nWbProcess = 0;
    }
}

// SDM 1.06.5 Removed unused command handlers

// Split function SDM 1.5Test8.2
/***************************************************************************/
// CSaDoc::OnAdvancedParseWords Parse wave data
/***************************************************************************/
void CSaDoc::OnAdvancedParseWords()
{
    CheckPoint();

    if (m_pDlgAdvancedParseWords==NULL)
    {
        m_pDlgAdvancedParseWords = new CDlgAdvancedParseWords(this);
        if (!m_pDlgAdvancedParseWords->Create())
        {
            CSaApp * pApp = (CSaApp *)AfxGetApp();
            pApp->ErrorMessage(IDS_ERROR_NO_DIALOG);
            delete m_pDlgAdvancedParseWords;
            m_pDlgAdvancedParseWords = NULL;
            return;
        }
    }
    m_pDlgAdvancedParseWords->Show((LPCTSTR)GetTitle());
}

/***************************************************************************/
// CSaDoc::OnUpdateAdvancedParseWords Menu update
/***************************************************************************/
void CSaDoc::OnUpdateAdvancedParseWords(CCmdUI * pCmdUI)
{
    pCmdUI->Enable(GetDataSize() != 0); // enable if data is available
}

// Split function SDM 1.5Test8.2
/***************************************************************************/
// CSaDoc::OnAdvancedParsePhrases Parse wave data
/***************************************************************************/
void CSaDoc::OnAdvancedParsePhrases()
{
    CheckPoint();

    if (m_pDlgAdvancedParsePhrases==NULL)
    {
        m_pDlgAdvancedParsePhrases = new CDlgAdvancedParsePhrases(this);
        if (!m_pDlgAdvancedParsePhrases->Create())
        {
            CSaApp * pApp = (CSaApp *)AfxGetApp();
            pApp->ErrorMessage(IDS_ERROR_NO_DIALOG);
            delete m_pDlgAdvancedParsePhrases;
            m_pDlgAdvancedParsePhrases = NULL;
            return;
        }
    }
    m_pDlgAdvancedParsePhrases->Show((LPCTSTR)GetTitle());
}

/***************************************************************************/
// CSaDoc::OnUpdateAdvancedParsePhrases Menu update
/***************************************************************************/
void CSaDoc::OnUpdateAdvancedParsePhrases(CCmdUI * pCmdUI)
{
    pCmdUI->Enable(GetDataSize() != 0); // enable if data is available
}

/**
* delete the contents of a segment type
*/
void CSaDoc::DeleteSegmentContents(EAnnotation type)
{

    CSegment * pSegment = m_apSegments[type];
    pSegment->DeleteContents();
}

// Split function SDM 1.5Test8.2
/***************************************************************************/
// CSaDoc::AdvancedParse Parse wave data
/***************************************************************************/
BOOL CSaDoc::AdvancedParseAuto()
{
    // get pointer to view
    POSITION pos = GetFirstViewPosition();
    CSaView * pView = (CSaView *)GetNextView(pos);

    // add the gloss transcription bar to all views
    pView->SendMessage(WM_COMMAND, ID_GLOSS_ALL);

    CSaApp * pApp = (CSaApp *)AfxGetApp();

    CSegment * pSegment = m_apSegments[GLOSS];

    RestartAllProcesses();

    pSegment->RestartProcess(); // for the case of a cancelled process
    pSegment->SetDataInvalid(); // SDM 1.5Test10.7
    short int nResult = LOWORD(pSegment->Process(NULL, this));   // process data
    if (nResult == PROCESS_ERROR)
    {
        // error parsing
        pApp->ErrorMessage(IDS_ERROR_PARSE);
        return FALSE;
    }
    if (nResult == PROCESS_CANCELED)
    {
        // error canceled parsing
        pApp->ErrorMessage(IDS_CANCELED);
        return FALSE;
    }

    // for importing this is as far as we need to go
    pView->RefreshGraphs(); // redraw graphs without legend window
    return TRUE;
}

// Split function SDM 1.5Test8.2
/***************************************************************************/
// CSaDoc::AdvancedParse Parse wave data
/***************************************************************************/
BOOL CSaDoc::AdvancedParseWord()
{
    // get pointer to view
    POSITION pos = GetFirstViewPosition();
    CSaView * pView = (CSaView *)GetNextView(pos);

    // add the gloss transcription bar to all views
    pView->SendMessage(WM_COMMAND, ID_GLOSS_ALL);

    CSaApp * pApp = (CSaApp *)AfxGetApp();

    RestartAllProcesses();

    DeleteSegmentContents(PHONETIC);
    DeleteSegmentContents(GLOSS);

    CSegment * pSegment = m_apSegments[GLOSS];
    pSegment->RestartProcess(); // for the case of a cancelled process
    pSegment->SetDataInvalid(); // SDM 1.5Test10.7

    short int nResult = LOWORD(pSegment->Process(NULL, this));   // process data
    if (nResult == PROCESS_ERROR)
    {
        // error parsing
        pApp->ErrorMessage(IDS_ERROR_PARSE);
        return FALSE;
    }
    if (nResult == PROCESS_CANCELED)
    {
        // error canceled parsing
        pApp->ErrorMessage(IDS_CANCELED);
        return FALSE;
    }

    pView->RefreshGraphs(); // redraw graphs without legend window
    return TRUE;
}

// Split function SDM 1.5Test8.2
/***************************************************************************/
// CSaDoc::AdvancedParse Parse wave data
/***************************************************************************/
BOOL CSaDoc::AdvancedParsePhrase()
{
    // get pointer to view
    POSITION pos = GetFirstViewPosition();
    CSaView * pView = (CSaView *)GetNextView(pos);

    // add the gloss transcription bar to all views
    pView->SendMessage(WM_COMMAND, ID_GLOSS_ALL);

    CSaApp * pApp = (CSaApp *)AfxGetApp();

    RestartAllProcesses();

    // set the L1 and L2 segments to match the GLOSS
    DeleteSegmentContents(PHONETIC);
    DeleteSegmentContents(GLOSS);
    DeleteSegmentContents(MUSIC_PL1);
    DeleteSegmentContents(MUSIC_PL2);

    CSegment * pSegment = m_apSegments[GLOSS];
    pSegment->RestartProcess(); // for the case of a cancelled process
    pSegment->SetDataInvalid(); // SDM 1.5Test10.7
    short int nResult = LOWORD(pSegment->Process(NULL, this));   // process data
    if (nResult == PROCESS_ERROR)
    {
        // error parsing
        pApp->ErrorMessage(IDS_ERROR_PARSE);
        return FALSE;
    }
    if (nResult == PROCESS_CANCELED)
    {
        // error canceled parsing
        pApp->ErrorMessage(IDS_CANCELED);
        return FALSE;
    }

    CSegment * pSegmentPho = m_apSegments[PHONETIC];
    CSegment * pSegmentL1 = m_apSegments[MUSIC_PL1];
    CSegment * pSegmentL2 = m_apSegments[MUSIC_PL2];

    // iterate through gloss segments and insert
    // segments into L1 and L2
    CSaString delimiter = SEGMENT_DEFAULT_CHAR;
    DWORD dwLast = -1;
    DWORD dwOrder = 0;
    for (int i=0; i<pSegmentPho->GetOffsetSize(); i++)
    {
        DWORD dwStart = pSegmentPho->GetOffset(i);
        DWORD dwDuration = pSegmentPho->GetDuration(i);
        if (dwStart!=dwLast)
        {
            pSegmentL1->Insert(dwOrder,delimiter,0,dwStart,dwDuration);
            pSegmentL2->Insert(dwOrder,delimiter,0,dwStart,dwDuration);
            dwLast=dwStart;
            dwOrder++;
        }
    }

    CGraphWnd * pGraph = pView->GraphIDtoPtr(IDD_RAWDATA);
    if (pGraph!=NULL)
    {
        pGraph->ShowAnnotation(PHONETIC, TRUE, TRUE);
        pGraph->ShowAnnotation(MUSIC_PL1, TRUE, TRUE);
        pGraph->ShowAnnotation(MUSIC_PL2, TRUE, TRUE);
    }

    pView->RefreshGraphs(); // redraw graphs without legend window
    return TRUE;
}

// Split function SDM 1.5Test8.2
/***************************************************************************/
// CSaDoc::OnAdvancedSegment Segment wave data
/***************************************************************************/
void CSaDoc::OnAdvancedSegment()
{
    CheckPoint();

    if (m_pDlgAdvancedSegment==NULL)
    {
        m_pDlgAdvancedSegment = new CDlgAdvancedSegment(this);
        if (!m_pDlgAdvancedSegment->Create())
        {
            CSaApp * pApp = (CSaApp *)AfxGetApp();
            pApp->ErrorMessage(IDS_ERROR_NO_DIALOG);
            delete m_pDlgAdvancedSegment;
            m_pDlgAdvancedSegment = NULL;
            return;
        }
    }
    m_pDlgAdvancedSegment->Show((LPCTSTR)GetTitle());
}

// Split function SDM 1.5Test8.2
/***************************************************************************/
// CSaDoc::AdvancedSegment Segment wave data
/***************************************************************************/
BOOL CSaDoc::AdvancedSegment()
{

    POSITION pos = GetFirstViewPosition();
    CSaView * pView = (CSaView *)GetNextView(pos);
    CSaDoc * pDoc = pView->GetDocument();

    pView->SendMessage(WM_COMMAND, ID_PHONETIC_ALL);

    // SDM1.5Test8.2
    CSegment * pPreserve[ANNOT_WND_NUMBER];
    for (int nLoop = GLOSS; nLoop < ANNOT_WND_NUMBER; nLoop++)
    {
        pPreserve[nLoop] = m_apSegments[nLoop];
        if (nLoop == GLOSS)
        {
            m_apSegments[nLoop] = new CGlossSegment(GLOSS,PHONETIC);
        }
        else
        {
            m_apSegments[nLoop] = new CReferenceSegment(REFERENCE,GLOSS);
        }
    }

    CPhoneticSegment * pSegment = (CPhoneticSegment *)m_apSegments[PHONETIC];
    RestartAllProcesses(); // for the case of cancelled dependent processes
    pSegment->RestartProcess(); // for the case of a cancelled process
    pSegment->SetDataInvalid(); // clear data from previous run SDM 1.06.4

    short int nResult = LOWORD(pSegment->Process(NULL, this)); // process data
    CSaApp * pApp = (CSaApp *)AfxGetApp();

    // restore preserved gloss etc. SDM 1.5Test8.2
    for (int nLoop = GLOSS; nLoop < ANNOT_WND_NUMBER; nLoop++)
    {
        if (m_apSegments[nLoop])
        {
            delete m_apSegments[nLoop];
        }
        m_apSegments[nLoop] = pPreserve[nLoop];
    }

    if (!m_apSegments[GLOSS]->IsEmpty())   // Gloss segments need to be aligned to phonetic SDM 1.5Test8.2
    {
        CGlossSegment * pGloss = (CGlossSegment *) m_apSegments[GLOSS];
        DWORD dwStart;
        DWORD dwStop;
        DWORD dwTemp;
        DWORD dwDistance;
        int nPhonetic;
        BOOL bInsert = FALSE;
        int nGloss;
        for (nGloss=0; nGloss < pGloss->GetOffsetSize(); nGloss++)
        {
            dwStart = pGloss->GetOffset(nGloss);
            dwStop = dwStart + pGloss->GetDuration(nGloss);
            dwTemp = dwStop;
            nPhonetic = pGloss->AdjustPositionToMaster(this, dwStart, dwTemp);

            if (dwStart > pGloss->GetOffset(nGloss))
            {
                dwDistance = dwStart - pGloss->GetOffset(nGloss);
            }
            else
            {
                dwDistance = pGloss->GetOffset(nGloss) - dwStart;
            }

            bInsert = FALSE;
            if (nPhonetic == -1)   // empty phonetic
            {
                bInsert = TRUE;
                nPhonetic = 0;
            }
            else if (dwDistance > GetBytesFromTime(MAX_AUTOSEGMENT_MOVE_GLOSS_TIME))     // too far away
            {
                bInsert = TRUE;
                if (dwStart < pGloss->GetOffset(nGloss))
                {
                    nPhonetic = pSegment->GetNext(nPhonetic);
                }
            }
            else if (nGloss && (dwStart == pGloss->GetOffset(nGloss-1)))     // last gloss attached to nearest phonetic
            {
                bInsert = TRUE;
                nPhonetic = pSegment->GetNext(nPhonetic);
            }
            else if ((pGloss->GetNext(nGloss) != -1) && ((dwStart+dwTemp)/2 <= pGloss->GetOffset(nGloss+1)))     // next gloss nearest to same phonetic
            {
                if ((pGloss->GetOffset(nGloss+1) < dwStart)
                        ||(dwDistance > (pGloss->GetOffset(nGloss+1) - dwStart)))   // next gloss closer
                {
                    bInsert = TRUE;
                    // nPhonetic is correct
                }
            }

            if (bInsert)
            {
                dwStart = pGloss->GetOffset(nGloss); // Insert in same location as old segment
                if (nPhonetic ==-1)
                {
                    nPhonetic = pSegment->GetOffsetSize();    // insert at end
                }
                int nPrevious = pSegment->GetPrevious(nPhonetic);
                if (nPrevious != -1)
                {
                    pSegment->Adjust(this, nPrevious, pSegment->GetOffset(nPrevious), dwStart - pSegment->GetOffset(nPrevious));
                }
                CSaString szEmpty = SEGMENT_DEFAULT_CHAR;
                pSegment->Insert(nPhonetic, szEmpty, FALSE, dwStart , pSegment->GetOffset(nPhonetic) - dwStart);
            }
            else
            {
                pGloss->Adjust(this, nGloss, dwStart, dwStop - dwStart);
            }
            if (nGloss > 0)   // Adjust previous gloss segment
            {
                pGloss->Adjust(pDoc, nGloss-1, pGloss->GetOffset(nGloss-1), pGloss->CalculateDuration(pDoc, nGloss-1));
            }
        }
        if (nGloss > 0)   // Adjust previous gloss segment (last)
        {
            pGloss->Adjust(pDoc, nGloss-1, pGloss->GetOffset(nGloss-1), pGloss->CalculateDuration(pDoc, nGloss-1));
        }
    }

    if (nResult == PROCESS_ERROR)
    {
        // error segmenting
        pApp->ErrorMessage(IDS_ERROR_SEGMENT);
        return FALSE;
    }
    if (nResult == PROCESS_CANCELED)
    {
        // error canceled segmenting
        pApp->ErrorMessage(IDS_CANCELED);
        return FALSE;
    }

    // get pointer to view
    pView->RefreshGraphs(); // redraw all graphs without legend window
    return TRUE;
}

/***************************************************************************/
// CSaDoc::OnUpdateAdvancedSegment Menu update
/***************************************************************************/
void CSaDoc::OnUpdateAdvancedSegment(CCmdUI * pCmdUI)
{
    pCmdUI->Enable(GetDataSize() != 0); // enable if data is available
}

void CSaDoc::WriteProperties(CObjectOStream & obs)
{

    const CSaString & sPath = GetPathName();
    TCHAR pszFullPath[_MAX_PATH];
    _tfullpath(pszFullPath, sPath, _MAX_PATH);
    CSaString szFullPath = pszFullPath;
    obs.WriteBeginMarker(psz_sadoc, szFullPath.utf8().c_str());

    obs.WriteBeginMarker(psz_wndlst);
    POSITION pos = GetFirstViewPosition();
    while (pos)
    {
        CSaView * pshv = (CSaView *)GetNextView(pos); // increments pos
        if (pshv && pshv->IsKindOf(RUNTIME_CLASS(CSaView)))
        {
            pshv->WriteProperties(obs);
        }
    }
    obs.WriteEndMarker(psz_wndlst);

    obs.WriteEndMarker(psz_sadoc);
    obs.WriteNewline();
}

BOOL CSaDoc::ReadProperties(CObjectIStream & obs)
{

    char buffer[1024];
    if (!obs.bReadBeginMarker(psz_sadoc, buffer, _countof(buffer)))
    {
        return FALSE;
    }
    CSaString sPath;
    sPath.setUtf8(buffer);

    if (_taccess(sPath, 04) != 0)
    {
        obs.SkipToEndMarker(psz_sadoc);
        return TRUE;
    }
    TRACE(_T("Autoload: %s\n"), sPath);
    CSaView::s_SetObjectStream(obs);  // 1996-09-05 MRP

    while (!obs.bAtEnd())
    {
        if (CSaDoc::ReadPropertiesOfViews(obs, sPath)) ;
        else if (obs.bReadEndMarker(psz_sadoc))
        {
            break;
        }
        else if (obs.bAtBeginOrEndMarker())
        {
            break;
        }
        else
        {
            obs.ReadMarkedString();    // Skip unexpected field
        }
    }
    CSaView::s_ClearObjectStream();  // 1996-09-05 MRP

    return TRUE;
}



BOOL CSaDoc::ReadPropertiesOfViews(CObjectIStream & obs, const CSaString & sPath)
{
    CSaDoc * pdoc = NULL;

    if (!obs.bReadBeginMarker(psz_wndlst))
    {
        return FALSE;
    }

    while (!obs.bAtEnd())
    {
        if (obs.bAtBeginMarker(psz_saview))
        {
            if (pdoc)
            {
                CDocTemplate * pTemplate = pdoc->GetDocTemplate();
                ASSERT_VALID(pTemplate);
                CFrameWnd * pwndFrame = pTemplate->CreateNewFrame(pdoc, NULL);
                ASSERT(pwndFrame);
                pTemplate->InitialUpdateFrame(pwndFrame, pdoc);
            }
            else
            {
                s_bDocumentWasAlreadyOpen = TRUE; // 1996-09-05 MRP: OnOpenDocument clears this flag
                pdoc = (CSaDoc *)AfxGetApp()->OpenDocumentFile(sPath);
                if (s_bDocumentWasAlreadyOpen)
                {
                    TRACE(_T("Not autoloading aliased file: %s\n"), sPath);
                    pdoc = NULL;
                }
                if (!pdoc)
                {
                    TRACE(_T("Database file <%s> does not exist.\n"), sPath);
                    obs.SkipToEndMarker(psz_wndlst);
                    break;
                }
            }
            CSaView * pview = ((CSaApp *)AfxGetApp())->GetViewActive();
            ASSERT(pview);
            ASSERT(pview->GetDocument() == pdoc);
            pview->ShowInitialStateAndZ();
            pview->ShowInitialTopState();
        }
        else if (obs.bReadEndMarker(psz_wndlst))
        {
            break;
        }
        else
        {
            obs.ReadMarkedString();    // Skip unexpected field
        }
    }

    return TRUE;
}

// SDM 1.06.4
/***************************************************************************/
// CSaDoc::OnAutoAlign Start Align Wizard
/***************************************************************************/
void CSaDoc::OnAutoAlign()
{
    m_nTranscriptionApplicationCount = 0;
    CDlgAlignTranscriptionDataSheet dlg(NULL,this);

    dlg.init.m_bGloss = (AfxGetApp()->GetProfileInt(L"TranscriptionAlignment",L"Gloss",0)!=0);
    dlg.init.m_bOrthographic = (AfxGetApp()->GetProfileInt(L"TranscriptionAlignment",L"Orthographic",0)!=0);
    dlg.init.m_bPhonemic = (AfxGetApp()->GetProfileInt(L"TranscriptionAlignment",L"Phonemic",0)!=0);
    dlg.init.m_bPhonetic = (AfxGetApp()->GetProfileInt(L"TranscriptionAlignment",L"Phonetic",1)!=0);
    dlg.init.m_bReference = (AfxGetApp()->GetProfileInt(L"TranscriptionAlignment",L"Reference",1)!=0);
    dlg.init.m_bUseReference = (AfxGetApp()->GetProfileInt(L"TranscriptionAlignment",L"UseReference",0)!=0);

    dlg.SetWizardMode();
    int result = dlg.DoModal();
    if (result==ID_WIZFINISH)
    {
        AfxGetApp()->WriteProfileInt(L"TranscriptionAlignment",L"Gloss",(dlg.init.m_bGloss)?1:0);
        AfxGetApp()->WriteProfileInt(L"TranscriptionAlignment",L"Orthographic",(dlg.init.m_bOrthographic)?1:0);
        AfxGetApp()->WriteProfileInt(L"TranscriptionAlignment",L"Phonemic",(dlg.init.m_bPhonemic)?1:0);
        AfxGetApp()->WriteProfileInt(L"TranscriptionAlignment",L"Phonetic",(dlg.init.m_bPhonetic)?1:0);
        AfxGetApp()->WriteProfileInt(L"TranscriptionAlignment",L"Reference",(dlg.init.m_bReference)?1:0);
        AfxGetApp()->WriteProfileInt(L"TranscriptionAlignment",L"UseReference",(dlg.init.m_bUseReference)?1:0);
    }
    else if (result==IDCANCEL)
    {
        while (m_nTranscriptionApplicationCount>0)
        {
            RevertTranscriptionChanges();
        }
    }
}

void CSaDoc::ApplyTranscriptionChanges(CTranscriptionDataSettings & settings)
{
    if (!settings.m_bUseReference)
    {
        AlignTranscriptionData(settings);
    }
    else
    {
        AlignTranscriptionDataByRef(settings.m_TranscriptionData);
    }
    m_nTranscriptionApplicationCount++;
}

void CSaDoc::RevertTranscriptionChanges()
{
    Undo(FALSE);
    POSITION pos = GetFirstViewPosition();
    CSaView * pView = (CSaView *)GetNextView(pos);
    //pView->SendMessage(WM_COMMAND,ID_EDIT_UNDO,0);
    pView->RefreshGraphs();
    if (m_nTranscriptionApplicationCount>0)
    {
        m_nTranscriptionApplicationCount--;
    }
}

void CSaDoc::OnUpdateAutoAlign(CCmdUI * pCmdUI)
{
    pCmdUI->Enable(GetDataSize() != 0); // enable if data is available
}

// SDM 1.5Test8.5
/***************************************************************************/
// CSaDoc::GetDefaultMenu Mode dependent Menu handle
/***************************************************************************/
HMENU CSaDoc::GetDefaultMenu()
{
    return ((CMainFrame *) AfxGetMainWnd())->GetNewMenu();   // just use original default
}

// SDM 1.5Test8.5
/***************************************************************************/
// CSaDoc::GetDefaultAccel Mode dependent Accelerator Table handle
/***************************************************************************/
HACCEL CSaDoc::GetDefaultAccel()
{
    return ((CMainFrame *) AfxGetMainWnd())->GetNewAccel();   // just use original default
}

/***************************************************************************/
// CSaDoc::OnAutoPitchDone  Pitch background processing complete handler
/***************************************************************************/
void CSaDoc::NotifyAutoPitchDone(void *)
{

}

/***************************************************************************/
// CSaDoc::OnFragmentDone  Fragmenter background processing complete handler
/***************************************************************************/
void CSaDoc::NotifyFragmentDone(void * pCaller)
{
    POSITION pos = GetFirstViewPosition();
    CSaView * pView = (CSaView *)GetNextView(pos);
    pView->NotifyFragmentDone(pCaller);

    CMainFrame * pMainWnd = (CMainFrame *)AfxGetMainWnd();
    pMainWnd->NotifyFragmentDone(pCaller);

    if ((pView->GetCursorAlignment() == ALIGN_AT_FRAGMENT) &&
            (AutoSnapUpdateNeeded()))
    {
        // Phonetic Segment was not snapped to Fragments
        CSaApp * pApp = static_cast<CSaApp *>(AfxGetApp());
        if (pApp->GetBatchMode() == 0)
        {
            pView->SendMessage(WM_COMMAND, ID_EDIT_AUTO_SNAP_UPDATE, 0L);
        }
    }
}

bool CSaDoc::IsTempOverlay()
{
    return m_bTempOverlay;
}

void CSaDoc::SetTempOverlay()
{
    m_bTempOverlay = true;
}

void CSaDoc::GetAlignInfo(CAlignInfo & info)
{
    // get source data size
    info.bValid = true;

    DWORD dwDataSize = GetDataSize();
    DWORD wSmpSize = GetSampleSize();

    info.dStart = 0;
    info.dTotalLength = GetTimeFromBytes(dwDataSize);

    // find the maximum level in the recorded data
    int nMaxValue = 0;
    int nData = 0;
	int nMinValue = 0;
    BYTE bData;
    unsigned int usMaxLevel = 0;

    DWORD dwDataPos = 0;
    DWORD dwBufferSize = m_pProcessAdjust->GetProcessedWaveDataBufferSize();
    DWORD dwBlockStart = 0;

    {
        // start processing loop
        while (dwDataPos < dwDataSize)
        {
            // load source data
            // get pointer to raw data block
            HPSTR pSource =  m_pProcessAdjust->GetProcessedWaveData(dwDataPos, FALSE);
            DWORD dwBlockEnd = dwDataPos + dwBufferSize;
            if (dwBlockEnd > dwDataSize)
            {
                dwBlockEnd = dwDataSize;
                dwBufferSize = dwDataSize - dwDataPos;
            }

            TRACE(_T("block %lu %lu %lu\n"),dwDataPos, dwBlockStart, dwDataSize);
            bool bDone = false;
            while (dwDataPos < dwBlockEnd)
            {
                // read data
                if (wSmpSize == 1)   // 8 bit per sample
                {
                    bData = *pSource++; // data range is 0...255 (128 is center)
                    nData = bData - 128;
                }
                else                  // 16 bit data
                {
                    nData = *((short int *)pSource++);
                    pSource++;
                    dwDataPos++;
                }
                // process data
                if (nData > nMaxValue)
                {
                    nMaxValue = nData;    // store higher value
                }
                else
                {
                    if (nData < nMinValue)
                    {
                        nMinValue = nData;    // store lower value
                    }
                }

                // set peak level
                if (nMinValue * -1 > nMaxValue)
                {
                    usMaxLevel = (UINT)(nMinValue * -1);
                }
                else
                {
                    usMaxLevel = (UINT)nMaxValue;
                }

                // norm the level to %
                if (wSmpSize == 1)   // 8 bit per sample
                {
                    usMaxLevel = 100 * usMaxLevel / 128;
                }
                else
                {
                    usMaxLevel = (UINT)((LONG)100 * (LONG)usMaxLevel / 32768);
                }

                dwDataPos++;
                if (usMaxLevel > MIN_VOICE_LEVEL)
                {
                    info.dStart = GetTimeFromBytes(dwDataPos - wSmpSize);
                    bDone = true;
                    break;
                }
            }
            dwBlockStart += dwBufferSize;
            if (bDone)
            {
                break;
            }
        }
    }

    TRACE(_T("found @ %f of %f\n"), info.dStart, info.dTotalLength);
    info.bValid = TRUE;
}

CProcessSpectrogram * CSaDoc::GetSpectrogram(bool bRealTime)
{
    CMainFrame * pMainWnd = (CMainFrame *)AfxGetMainWnd();
    if (bRealTime)
    {
        if (!m_pProcessSpectrogram)
        {
            m_pProcessSpectrogram = new CProcessSpectrogram(*pMainWnd->GetSpectrogramParmDefaults(),this,bRealTime);
        }
        return m_pProcessSpectrogram;
    }
    else
    {
        if (!m_pProcessSnapshot)
        {
            m_pProcessSnapshot = new CProcessSpectrogram(*pMainWnd->GetSnapshotParmDefaults(),this,bRealTime);
        }
        return m_pProcessSnapshot;
    }
}

CProcessSDP * CSaDoc::GetSDP(int nIndex)
{
    if (nIndex < 0 || nIndex >= 3)
    {
        return NULL;
    }

    if (!m_pProcessSDP[nIndex])
    {
        m_pProcessSDP[nIndex] = new CProcessSDP();
    }

    return m_pProcessSDP[nIndex];
}


void CSaDoc::OnToolsAdjustInvert()
{
    m_pProcessAdjust->SetInvert(!m_pProcessAdjust->GetInvert());
    InvalidateAllProcesses();
    WorkbenchProcess();
    SetModifiedFlag(TRUE);
    SetAudioModifiedFlag(TRUE);

    POSITION pos = GetFirstViewPosition();
    CSaView * pView = ((CSaView *)GetNextView(pos));
    pView->RefreshGraphs();
}

void CSaDoc::OnUpdateToolsAdjustInvert(CCmdUI * pCmdUI)
{
    pCmdUI->SetCheck(m_pProcessAdjust->GetInvert());
}

void CSaDoc::OnToolsAdjustNormalize()
{
    m_pProcessAdjust->SetNormalize(!m_pProcessAdjust->GetNormalize());
    InvalidateAllProcesses();
    WorkbenchProcess();
    SetModifiedFlag(TRUE);
    SetAudioModifiedFlag(TRUE);

    POSITION pos = GetFirstViewPosition();
    CSaView * pView = ((CSaView *)GetNextView(pos));
    pView->RefreshGraphs();
}

void CSaDoc::OnUpdateToolsAdjustNormalize(CCmdUI * pCmdUI)
{
    pCmdUI->SetCheck(m_pProcessAdjust->GetNormalize());
}

void CSaDoc::OnToolsAdjustZero()
{
    m_pProcessAdjust->SetZero(!m_pProcessAdjust->GetZero());

    InvalidateAllProcesses();
    WorkbenchProcess();
    SetModifiedFlag(TRUE);
    SetAudioModifiedFlag(TRUE);

    POSITION pos = GetFirstViewPosition();
    CSaView * pView = ((CSaView *)GetNextView(pos));

    pView->RefreshGraphs();
}

void CSaDoc::OnUpdateToolsAdjustZero(CCmdUI * pCmdUI)
{
    pCmdUI->SetCheck(m_pProcessAdjust->GetZero());
}

void CSaDoc::DestroyAdvancedSegment()
{
    if (m_pDlgAdvancedSegment!=NULL)
    {
        delete m_pDlgAdvancedSegment;
        m_pDlgAdvancedSegment=NULL;
    }
}

void CSaDoc::DestroyAdvancedParse()
{
    if (m_pDlgAdvancedSegment!=NULL)
    {
        delete m_pDlgAdvancedSegment;
        m_pDlgAdvancedSegment=NULL;
    }
}

/**
* return true if this temp file is editable
*/
bool CSaDoc::CanEdit()
{
    return m_bAllowEdit;
}

/**
* returns true if the file not a standard wave file
* and the document is working on a copy of the original
*/
bool CSaDoc::IsTempFile()
{
    // non-wave files have this set...
    if (m_bUsingTempFile)
    {
        return true;
    }
    if (m_szTempConvertedWave.IsEmpty())
    {
        return false;
    }
    if (m_szTempConvertedWave.GetLength()==0)
    {
        return false;
    }
    return true;
}

// SDM 1.06.4
/***************************************************************************/
// CSaDoc::OnAutoAlign Start Align Wizard
/***************************************************************************/
void CSaDoc::OnAutoReferenceData()
{
    CheckPoint();

    // determine how many words there are
    CGlossSegment * pGloss = (CGlossSegment *)m_apSegments[GLOSS];
    if (pGloss->IsEmpty())
    {
        CSaApp * pApp = (CSaApp *)AfxGetApp();
        pApp->ErrorMessage(IDS_ERROR_NO_WORDS_ON_AUTO_REFERENCE);
        return;
    }

    int begin = 1;
    int end = pGloss->GetOffsetSize();

    int selection = pGloss->GetSelection();
    bool glossSelected = (selection!=-1);

    CSaApp * pApp = (CSaApp *)AfxGetApp();

    int numWords = pGloss->GetOffsetSize();

    // query the user
    CDlgAutoReferenceData dlg( this, numWords);

    dlg.mLastImport = pApp->GetProfileString(L"AutoRef",L"LastImport",L"");
    dlg.mBeginRef = pApp->GetProfileString(L"AutoRef",L"BeginRef",L"");
    dlg.mEndRef = pApp->GetProfileString(L"AutoRef",L"EndRef",L"");
    dlg.mUsingNumbers = (pApp->GetProfileInt(L"AutoRef",L"UsingNumbers",1)!=0)?true:false;
    dlg.mUsingFirstGloss = (pApp->GetProfileInt(L"AutoRef",L"UsingFirstGloss",1)!=0)?true:false;
    dlg.mBegin = begin;
    dlg.mEnd = end;
    dlg.mGlossSelected = glossSelected;

    if (dlg.DoModal()!=IDOK)
    {
        // do nothing on cancel
        return;
    }

    pApp->WriteProfileString(L"AutoRef",L"LastImport",dlg.mLastImport);
    pApp->WriteProfileString(L"AutoRef",L"BeginRef",dlg.mBeginRef);
    pApp->WriteProfileString(L"AutoRef",L"EndRef",dlg.mEndRef);
    pApp->WriteProfileInt(L"AutoRef",L"UsingNumbers",((dlg.mUsingNumbers)?1:0));
    pApp->WriteProfileInt(L"AutoRef",L"UsingFirstGloss",((dlg.mUsingFirstGloss)?1:0));

    if (dlg.mUsingNumbers)
    {
        // apply the number
        int val = dlg.mBegin;
        // iterate through the gloss segments and add number to empty reference fields
        CReferenceSegment * pReference = (CReferenceSegment *)m_apSegments[REFERENCE];
        int start = (dlg.mUsingFirstGloss)?0:selection;
        // if we are inserting, we need to make sure
        // that any segments in between either have existing
        // reference segments, or we need to add a blank one
        for (int i = start; i < pGloss->GetOffsetSize(); i++)
        {
            DWORD offset = pGloss->GetOffset(i);
            DWORD duration = pGloss->GetDuration(i);
            CSaString text;
            text.Format(L"%d",val);
            DWORD roffset = pReference->GetOffset(i);
            if (roffset==0)
            {
                int j = pReference->GetOffsetSize();
                pReference->Insert(j,text,0,offset,duration);
            }
            else
            {
                pReference->SetText(i,text,0,offset,duration);
            }
            if (val==dlg.mEnd)
            {
                break;
            }
            val++;
        }
    }
    else
    {
        // data should be fully validated by dialog!
        CTranscriptionData td;
        CSaString temp = dlg.mLastImport;
        if (!ImportTranscription(temp,false,false,false,false,td,true,false))
        {
            CString msg;
            msg.LoadStringW(IDS_AUTO_REF_MAIN_1);
            CString msg2;
            msg2.LoadStringW(IDS_AUTO_REF_MAIN_2);
            msg.Append(msg2);
            msg2.LoadStringW(IDS_AUTO_REF_MAIN_3);
            msg.Append(msg2);
            AfxMessageBox(msg,MB_OK|MB_ICONEXCLAMATION);
            return;
        }

        CString ref = td.m_szPrimary;
        TranscriptionDataMap & map = td.m_TranscriptionData;
        MarkerList::iterator begin = find(map[ref].begin(),map[ref].end(),dlg.mBeginRef);
        MarkerList::iterator end = find(map[ref].begin(),map[ref].end(),dlg.mEndRef);

        // iterate through the gloss segments and add number to empty reference fields
        CReferenceSegment * pReference = (CReferenceSegment *)m_apSegments[REFERENCE];
        int start = (dlg.mUsingFirstGloss)?0:selection;
		// if preceeding reference fields don't exist, then install place holders
		for (int i = 0; i < start; i++)
		{
            DWORD offset = pGloss->GetOffset(i);
            DWORD duration = pGloss->GetDuration(i);
            DWORD roffset = pReference->GetOffset(i);
            if (roffset==0)
            {
                pReference->InsertBlank(i,offset,duration);
            }
		}
		// now fill in the real data
        for (int i = start; i < pGloss->GetOffsetSize(); i++)
        {
            DWORD offset = pGloss->GetOffset(i);
            DWORD duration = pGloss->GetDuration(i);
            CSaString text = *begin;
            DWORD roffset = pReference->GetOffset(i);
            if (roffset==0)
            {
                pReference->Insert(i,text,0,offset,duration);
            }
            else
            {
                pReference->SetText(i,text,0,offset,duration);
            }
            if (begin==end)
            {
                break;
            }
            begin++;
        }
    }

    SetModifiedFlag(TRUE); // data has been modified

    // refresh the tables
    POSITION pos = GetFirstViewPosition();
    CSaView * pView = ((CSaView *)GetNextView(pos));

    CGraphWnd * pGraph = pView->GraphIDtoPtr(IDD_RAWDATA);
    if (pGraph!=NULL)
    {
        pGraph->ShowAnnotation(REFERENCE, TRUE, TRUE);
    }

    pView->RefreshGraphs();

}

void CSaDoc::OnUpdateAutoReferenceData(CCmdUI * pCmdUI)
{
    // enable if data is available
    pCmdUI->Enable(GetDataSize() != 0);
}

void CSaDoc::AlignTranscriptionData(CTranscriptionDataSettings & settings)
{
    // save state for undo ability
    CheckPoint();
    SetModifiedFlag(TRUE); // document has been modified
    SetTransModifiedFlag(TRUE); // transcription has been modified

    POSITION pos = GetFirstViewPosition();
    CSaView * pView = (CSaView *)GetNextView(pos);

    enum { CHARACTER_OFFSETS = 0,CHARACTER_DURATIONS = 1,WORD_OFFSETS = 2};
    CDWordArray pArray[3];

    // clean gloss string
    // remove trailing and leading spaces
    settings.m_szGloss.Trim(SPACE_DELIMITER);

    if (GetSegment(GLOSS)->IsEmpty())
    {
        // auto parse
        if (!AdvancedParseAuto())
        {
            // process canceled by user
            Undo(FALSE);
            return;
        }
    }

    CSegment * pSegment = GetSegment(PHONETIC);

    //adjust character segments
    switch (settings.m_nSegmentBy)
    {
    case IDC_AUTOMATIC:
    {
        if (!AdvancedSegment())
        {
            // SDM 1.5Test8.2
            // process canceled by user
            Undo(FALSE);
            return;
        }
        for (int i=0; i<pSegment->GetOffsetSize(); i++)
        {
            pArray[CHARACTER_OFFSETS].InsertAt(i,pSegment->GetOffset(i)); // Copy Arrays
        }
        for (int i=0; i<pSegment->GetDurationSize(); i++)
        {
            pArray[CHARACTER_DURATIONS].InsertAt(i,pSegment->GetDuration(i));
        }
        // Copy gloss segments SDM 1.5Test8.2
        for (int i=0; i<GetSegment(GLOSS)->GetOffsetSize(); i++)
        {
            pArray[WORD_OFFSETS].InsertAt(i,GetSegment(GLOSS)->GetOffset(i));
        }
        // Create a gloss break at initial position SDM 1.5Test8.2
        if (pArray[WORD_OFFSETS][0] != pArray[CHARACTER_OFFSETS][0])
        {
            CSaString szEmpty = "";
            GetSegment(GLOSS)->Insert(0, szEmpty, FALSE, pArray[CHARACTER_OFFSETS][0], pArray[WORD_OFFSETS][0]-pArray[CHARACTER_OFFSETS][0]);
            pArray[WORD_OFFSETS].InsertAt(0,pArray[CHARACTER_OFFSETS][0]);
            settings.m_szGloss = CSaString(SPACE_DELIMITER) + settings.m_szGloss;
            settings.m_szPhonetic = CSaString(SPACE_DELIMITER) + settings.m_szPhonetic;
            settings.m_szPhonemic = CSaString(SPACE_DELIMITER) + settings.m_szPhonemic;
            settings.m_szOrthographic = CSaString(SPACE_DELIMITER) + settings.m_szOrthographic;
        }
        break;
    }

    case IDC_MANUAL:
    {
        // SDM 1.5Test8.2
        for (int i=0; i<GetSegment(GLOSS)->GetOffsetSize(); i++)
        {
            pArray[WORD_OFFSETS].InsertAt(i,GetSegment(GLOSS)->GetOffset(i)); // Copy gloss segments SDM 1.5Test8.2
        }
        switch (settings.m_nAlignBy)
        {
        case IDC_NONE:
        case IDC_WORD:
        {
            pArray[CHARACTER_OFFSETS].InsertAt(0,&pArray[WORD_OFFSETS]); // Copy gloss segments
            // build duration list
            int nIndex = 1;

            while (nIndex < pArray[CHARACTER_OFFSETS].GetSize())
            {
                pArray[CHARACTER_DURATIONS].Add(pArray[CHARACTER_OFFSETS][nIndex] - pArray[CHARACTER_OFFSETS][nIndex - 1]);
                nIndex++;
            }
            // Add final duration to fill remainder of waveform
            pArray[CHARACTER_DURATIONS].Add(GetDataSize() - pArray[CHARACTER_OFFSETS][nIndex - 1]);
            break;
        }
        case IDC_CHARACTER:
        {
            CFontTable * pTable = GetFont(PHONETIC);
            int nWord = 0;
            int nCharacters;
            CSaString szWord;
            for (int nGlossWordIndex = 0; nGlossWordIndex < pArray[WORD_OFFSETS].GetSize(); nGlossWordIndex++)
            {
                DWORD dwDuration;
                if ((nGlossWordIndex+1) < pArray[WORD_OFFSETS].GetSize())
                {
                    dwDuration = pArray[WORD_OFFSETS][nGlossWordIndex+1] - pArray[WORD_OFFSETS][nGlossWordIndex];
                    szWord = pTable->GetNext(CFontTable::DELIMITEDWORD, nWord, settings.m_szPhonetic);
                    nCharacters = pTable->GetLength(CFontTable::CHARACTER, szWord);
                }
                else
                {
                    dwDuration = GetDataSize() - pArray[WORD_OFFSETS][nGlossWordIndex];
                    szWord = pTable->GetNext(CFontTable::DELIMITEDWORD, nWord, settings.m_szPhonetic);
                    nCharacters = pTable->GetLength(CFontTable::CHARACTER, szWord);
                    szWord = pTable->GetRemainder(CFontTable::DELIMITEDWORD, nWord, settings.m_szPhonetic);
                    if (szWord.GetLength() != 0)
                    {
                        nCharacters++;    // remainder goes into one extra segment
                    }
                }
                if (nCharacters == 0)
                {
                    nCharacters++;
                }

                DWORD dwOffset = pArray[WORD_OFFSETS][nGlossWordIndex];
                DWORD dwSize = dwDuration/nCharacters;

                if (Is16Bit())
                {
                    dwSize &= ~1;
                }

                for (int nIndex = 0; nIndex < nCharacters; nIndex++)
                {
                    pArray[CHARACTER_OFFSETS].Add(dwOffset);
                    dwOffset = SnapCursor(STOP_CURSOR, dwOffset + dwSize, dwOffset + (dwSize/2) & ~1, dwOffset + dwSize, SNAP_LEFT);
                }
            }
            int nIndex = 1;
            while (nIndex < pArray[CHARACTER_OFFSETS].GetSize())
            {
                pArray[CHARACTER_DURATIONS].Add(pArray[CHARACTER_OFFSETS][nIndex] - pArray[CHARACTER_OFFSETS][nIndex - 1]);
                nIndex++;
            }
            // Add final duration to fill remainder of waveform
            pArray[CHARACTER_DURATIONS].Add(GetDataSize() - pArray[CHARACTER_OFFSETS][nIndex - 1]);
            break;
        }
        }
        break;
    }

    case IDC_KEEP:   // SDM 1.5Test8.2
    {
        // Copy gloss segments SDM 1.5Test8.2
        for (int i=0; i<GetSegment(GLOSS)->GetOffsetSize(); i++)
        {
            DWORD offset = GetSegment(GLOSS)->GetOffset(i);
            TRACE("word offset %d\n",offset);
            pArray[WORD_OFFSETS].InsertAt(i,offset);
        }

        // copy segment locations not character counts
        int nIndex = 0;
        while (nIndex != -1)
        {
            DWORD offset = pSegment->GetOffset(nIndex);
            pArray[CHARACTER_OFFSETS].Add(offset);
            TRACE("character offset %d\n",offset);
            DWORD duration = pSegment->GetDuration(nIndex);
            pArray[CHARACTER_DURATIONS].Add(duration);
            TRACE("character duration %d\n",offset);
            nIndex = pSegment->GetNext(nIndex);
        }
    }
    }

    CFontTable::tUnit nAlignMode = CFontTable::CHARACTER;
    switch (settings.m_nAlignBy)
    {
    case IDC_NONE:
        nAlignMode = CFontTable::NONE;
        break;
    case IDC_WORD:
        nAlignMode = CFontTable::DELIMITEDWORD;
        break;
    case IDC_CHARACTER:
    default:
        nAlignMode = CFontTable::CHARACTER;
    }

    // Insert Annotations
    {
        CSegment * pSegment = NULL;
        CFontTable * pTable = NULL;
        CSaString szNext;
        CSaString szNextWord;
        int nWordIndex = 0;
        int nStringIndex = 0;
        int nOffsetSize = 0;
        int nGlossIndex = 0;
        int nIndex = 0;

        // Process phonetic
        // SDM 1.06.8 only change if new segmentation or text changed
        if ((settings.m_bPhonetic) && ((settings.m_nSegmentBy != IDC_KEEP)||(settings.m_bPhoneticModified)))
        {

            nStringIndex = 0;
            nGlossIndex = 0;
            nWordIndex = 0;
            pSegment = GetSegment(PHONETIC);
            pTable = GetFont(PHONETIC);
            pSegment->DeleteContents(); // Delete contents and reinsert from scratch

            nOffsetSize = pArray[CHARACTER_OFFSETS].GetSize();
            for (nIndex = 0; nIndex < (nOffsetSize-1); nIndex++)
            {
                switch (settings.m_nAlignBy)
                {
                case IDC_NONE:
                    szNext = pTable->GetNext(nAlignMode, nStringIndex, settings.m_szPhonetic);
                    if (szNext.GetLength()==0)
                    {
                        szNext+=SEGMENT_DEFAULT_CHAR;
                    }
                    pSegment->Insert(pSegment->GetOffsetSize(),szNext, FALSE, pArray[CHARACTER_OFFSETS][nIndex], pArray[CHARACTER_DURATIONS][nIndex]);
                    break;
                case IDC_WORD:
                    if (nGlossIndex>=pArray[WORD_OFFSETS].GetSize())
                    {
                        // No more word breaks continue one character at a time
                        szNext = pTable->GetNext(nAlignMode, nStringIndex, settings.m_szPhonetic);
                    }
                    else if (pArray[CHARACTER_OFFSETS][nIndex]<pArray[WORD_OFFSETS][nGlossIndex])
                    {
                        // Insert default segment character if phonetic offset does not correspond to word boundary
                        szNext = SEGMENT_DEFAULT_CHAR;
                    }
                    else
                    {
                        // Insert Word on Gloss Boundary
                        szNext = pTable->GetNext(nAlignMode, nStringIndex, settings.m_szPhonetic);
                        nGlossIndex++;  // Increment word index
                    }
                    if (szNext.GetLength()==0)
                    {
                        szNext+=SEGMENT_DEFAULT_CHAR;
                    }
                    pSegment->Insert(pSegment->GetOffsetSize(),szNext, FALSE, pArray[CHARACTER_OFFSETS][nIndex], pArray[CHARACTER_DURATIONS][nIndex]);
                    break;
                case IDC_CHARACTER:
                    // the line is entered one character per segment
                    szNext.Empty();
                    while (true)
                    {
                        CSaString szTemp = pTable->GetNext(nAlignMode, nStringIndex, settings.m_szPhonetic);
                        if (szTemp.GetLength()==0)
                        {
                            // end of array
                            szTemp=SEGMENT_DEFAULT_CHAR;
                            break;
                        }
                        else if ((szTemp.GetLength()==1)&&(szTemp[0]==SPACE_DELIMITER))
                        {
                            // time to stop!
                            break;
                        }
                        else if (szTemp.GetLength()>1)
                        {
                            // in some situations if the trailing character is not a break
                            // it will be combined with the space.  we will break it here.
                            if (szTemp[0]==SPACE_DELIMITER)
                            {
                                if (szNext.GetLength()==0)
                                {
                                    // remove space and append
                                    szTemp.Delete(0,1);
                                }
                                else
                                {
                                    // backup and let the next character go into the next segment
                                    nStringIndex--;
                                    break;
                                }
                            }
                        }
                        szNext += szTemp;
                    }
                    pSegment->Insert(pSegment->GetOffsetSize(),szNext, FALSE, pArray[CHARACTER_OFFSETS][nIndex], pArray[CHARACTER_DURATIONS][nIndex]);
                    break;
                }
            }

            szNext = pTable->GetRemainder(nAlignMode, nStringIndex, settings.m_szPhonetic);
            if (szNext.GetLength()==0)
            {
                szNext+=SEGMENT_DEFAULT_CHAR;
            }
            pSegment->Insert(pSegment->GetOffsetSize(),szNext,FALSE,pArray[CHARACTER_OFFSETS][nOffsetSize-1], pArray[CHARACTER_DURATIONS][nOffsetSize-1]);

            // SDM 1.06.8 apply input filter to segment
            if (pSegment->GetInputFilter())
            {
                (pSegment->GetInputFilter())(*pSegment->GetString());
            }
        }

        // Process phonemic
        // SDM 1.06.8 only change  if new segmentation or text changed
        if ((settings.m_bPhonemic) && ((settings.m_nSegmentBy != IDC_KEEP)||(settings.m_bPhonemicModified)))
        {

            nStringIndex = 0;
            nGlossIndex = 0;
            nWordIndex = 0;
            pSegment = GetSegment(PHONEMIC);
            pTable = GetFont(PHONEMIC);
            pSegment->DeleteContents(); // Delete contents and reinsert from scratch

            nOffsetSize = pArray[CHARACTER_OFFSETS].GetSize();
            for (nIndex = 0; nIndex < (nOffsetSize-1); nIndex++)
            {
                switch (settings.m_nAlignBy)
                {
                case IDC_NONE:
                    szNext = pTable->GetNext(nAlignMode, nStringIndex, settings.m_szPhonemic);
                    if (szNext.GetLength()!=0)
                    {
                        // Skip Empty Segments
                        pSegment->Insert(pSegment->GetOffsetSize(),szNext, FALSE,pArray[CHARACTER_OFFSETS][nIndex], pArray[CHARACTER_DURATIONS][nIndex]);
                    }
                    break;
                case IDC_WORD:
                    if (nGlossIndex>=pArray[WORD_OFFSETS].GetSize())
                    {
                        // No more word breaks continue one character at a time
                        szNext = pTable->GetNext(nAlignMode, nStringIndex, settings.m_szPhonemic);
                    }
                    else if (pArray[CHARACTER_OFFSETS][nIndex]<pArray[WORD_OFFSETS][nGlossIndex])
                    {
                        // Skip if phonetic offset does not correspond to word boundary
                        continue;
                    }
                    else
                    {
                        // Insert Word on Gloss Boundary
                        szNext = pTable->GetNext(nAlignMode, nStringIndex, settings.m_szPhonemic);
                        nGlossIndex++;  // Increment word index
                    }
                    if (szNext.GetLength()==0)
                    {
                        // Skip NULL strings
                        continue;
                    }
                    pSegment->Insert(pSegment->GetOffsetSize(),szNext, FALSE, pArray[CHARACTER_OFFSETS][nIndex], pArray[CHARACTER_DURATIONS][nIndex]);
                    break;
                case IDC_CHARACTER:
                    // the line is entered one character per segment
                    szNext.Empty();
                    while (true)
                    {
                        CSaString szTemp = pTable->GetNext(nAlignMode, nStringIndex, settings.m_szPhonemic);
                        if (szTemp.GetLength()==0)
                        {
                            break;
                        }
                        else if ((szTemp.GetLength()==1)&&(szTemp[0]==SPACE_DELIMITER))
                        {
                            // time to stop!
                            break;
                        }
                        else if (szTemp.GetLength()>1)
                        {
                            // in some situations if the trailing character is not a break
                            // it will be combined with the space.  we will break it here.
                            if (szTemp[0]==SPACE_DELIMITER)
                            {
                                if (szNext.GetLength()==0)
                                {
                                    // remove space and append
                                    szTemp.Delete(0,1);
                                }
                                else
                                {
                                    // backup and let the next character go into the next segment
                                    nStringIndex--;
                                    break;
                                }
                            }
                        }
                        szNext += szTemp;
                    }
                    if (szNext.GetLength()>0)
                    {
                        pSegment->Insert(pSegment->GetOffsetSize(),szNext, FALSE,pArray[CHARACTER_OFFSETS][nIndex], pArray[CHARACTER_DURATIONS][nIndex]);
                    }
                    break;
                }
            }

            szNext = pTable->GetRemainder(nAlignMode, nStringIndex, settings.m_szPhonemic);
            // Skip empty segments
            if (szNext.GetLength()!=0)
            {
                pSegment->Insert(pSegment->GetOffsetSize(),szNext,FALSE, pArray[CHARACTER_OFFSETS][nOffsetSize-1], pArray[CHARACTER_DURATIONS][nOffsetSize-1]);
            }
            // SDM 1.06.8 apply input filter to segment
            if (pSegment->GetInputFilter())
            {
                (pSegment->GetInputFilter())(*pSegment->GetString());
            }
        }

        // Process tone
        if (settings.m_nSegmentBy != IDC_KEEP)
        {
            // SDM 1.06.8 only change  if new segmentation or text changed
            nStringIndex = 0;
            nGlossIndex = 0;
            nWordIndex = 0;
            pSegment = GetSegment(TONE);
            pSegment->DeleteContents();
        }

        // Process orthographic
        // SDM 1.06.8 only change  if new segmentation or text changed
        if ((settings.m_bOrthographic) && ((settings.m_nSegmentBy != IDC_KEEP)||(settings.m_bOrthographicModified)))
        {

            nStringIndex = 0;
            nGlossIndex = 0;
            nWordIndex = 0;
            pSegment = GetSegment(ORTHO);
            pTable = GetFont(ORTHO);
            pSegment->DeleteContents(); // Delete contents and reinsert from scratch

            nOffsetSize = pArray[CHARACTER_OFFSETS].GetSize();
            for (nIndex = 0; nIndex < (nOffsetSize-1); nIndex++)
            {
                switch (settings.m_nAlignBy)
                {
                case IDC_NONE:
                    szNext = pTable->GetNext(nAlignMode, nStringIndex, settings.m_szOrthographic);
                    if (szNext.GetLength()!=0)
                    {
                        // Skip Empty Segments
                        pSegment->Insert(pSegment->GetOffsetSize(),szNext, FALSE,pArray[CHARACTER_OFFSETS][nIndex], pArray[CHARACTER_DURATIONS][nIndex]);
                    }
                    break;
                case IDC_WORD:
                    if (nGlossIndex>=pArray[WORD_OFFSETS].GetSize())
                    {
                        // No more word breaks continue one character at a time
                        szNext = pTable->GetNext(nAlignMode, nStringIndex, settings.m_szOrthographic);
                    }
                    else if (pArray[CHARACTER_OFFSETS][nIndex]<pArray[WORD_OFFSETS][nGlossIndex])
                    {
                        // Skip if character offset does not correspond to word boundary
                        continue;
                    }
                    else
                    {
                        // Insert Word on Boundary
                        szNext = pTable->GetNext(nAlignMode, nStringIndex, settings.m_szOrthographic);
                        nGlossIndex++;  // Increment word index
                    }
                    if (szNext.GetLength()==0)
                    {
                        // Skip NULL words
                        continue;
                    }
                    pSegment->Insert(pSegment->GetOffsetSize(),szNext, FALSE, pArray[CHARACTER_OFFSETS][nIndex], pArray[CHARACTER_DURATIONS][nIndex]);
                    break;
                case IDC_CHARACTER:
                    // the line is entered one character per segment
                    szNext.Empty();
                    while (true)
                    {
                        CSaString szTemp = pTable->GetNext(nAlignMode, nStringIndex, settings.m_szOrthographic);
                        if (szTemp.GetLength()==0)
                        {
                            break;
                        }
                        else if ((szTemp.GetLength()==1)&&(szTemp[0]==SPACE_DELIMITER))
                        {
                            // time to stop!
                            break;
                        }
                        else if (szTemp.GetLength()>1)
                        {
                            // in some situations if the trailing character is not a break
                            // it will be combined with the space.  we will break it here.
                            if (szTemp[0]==SPACE_DELIMITER)
                            {
                                if (szNext.GetLength()==0)
                                {
                                    // remove space and append
                                    szTemp.Delete(0,1);
                                }
                                else
                                {
                                    // backup and let the next character go into the next segment
                                    nStringIndex--;
                                    break;
                                }
                            }
                        }
                        szNext += szTemp;
                    }
                    if (szNext.GetLength()>0)
                    {
                        pSegment->Insert(pSegment->GetOffsetSize(),szNext, FALSE,pArray[CHARACTER_OFFSETS][nIndex], pArray[CHARACTER_DURATIONS][nIndex]);
                    }
                    break;
                }
            }

            szNext = pTable->GetRemainder(nAlignMode, nStringIndex, settings.m_szOrthographic);
            if (szNext.GetLength()!=0)
            {
                // Skip empty segments
                pSegment->Insert(pSegment->GetOffsetSize(),szNext,FALSE,pArray[CHARACTER_OFFSETS][nOffsetSize-1], pArray[CHARACTER_DURATIONS][nOffsetSize-1]);
            }

            // SDM 1.06.8 apply input filter to segment
            if (pSegment->GetInputFilter())
            {
                (pSegment->GetInputFilter())(*pSegment->GetString());
            }
        }

        // Process gloss
        // SDM 1.5Test8.2 only change if text changed
        if (settings.m_bGlossModified)
        {
            nStringIndex = 0;
            pSegment = GetSegment(GLOSS);
            pTable = GetFont(GLOSS);

            bool poundDelimited = (settings.m_szGloss.FindOneOf(CSaString(WORD_DELIMITER))!=-1);

            // align gloss by word SDM 1.5Test8.2
            nAlignMode = CFontTable::DELIMITEDWORD;
            nOffsetSize = pArray[WORD_OFFSETS].GetSize();
            // Don't Select this segment SDM 1.5Test8.2
            pSegment->SelectSegment(*this,-1);
            // the gloss table uses a space as a delimiter,
            // the normally the text is delimited with a #.
            // if we see a # in the first position, we will continue
            // to scan the segments util we
            for (nIndex = 0; nIndex < (nOffsetSize-1); nIndex++)
            {

                szNext = pTable->GetNext(nAlignMode, nStringIndex, settings.m_szGloss);
                if (szNext.GetLength()==0)
                {
                    szNext = CSaString(WORD_DELIMITER);
                }
                else
                {
                    if (szNext[0]==WORD_DELIMITER)
                    {
                        // do nothing
                    }
                    else
                    {
                        szNext = CSaString(WORD_DELIMITER)+szNext;
                    }
                }

                // if the user used # delimiters, then if there are
                // embedded spaces, concatenate the lines
                if (poundDelimited)
                {
                    int nTempIndex = nStringIndex;
                    bool done = false;
                    while (!done)
                    {
                        int nLastIndex = nTempIndex;
                        CSaString szTemp = pTable->GetNext(nAlignMode, nTempIndex, settings.m_szGloss);
                        if (szTemp.GetLength()==0)
                        {
                            if (nTempIndex==nLastIndex)
                            {
                                // we are at the end of the data
                                done = true;
                            }
                            else
                            {
                                szNext += CSaString(SPACE_DELIMITER);
                            }
                        }
                        else
                        {
                            if (szTemp[0]==WORD_DELIMITER)
                            {
                                // we found the next line
                                nStringIndex = nLastIndex;
                                done = true;
                            }
                            else
                            {
                                szNext += CSaString(SPACE_DELIMITER) + szTemp;
                            }
                        }
                    }
                }

                szNext.Remove(0x0d);
                szNext.Remove(0x0a);
                pSegment->SelectSegment(*this,nIndex);
                ((CGlossSegment *)pSegment)->ReplaceSelectedSegment(this,szNext);
            };

            // take care of remainder
            szNext = pTable->GetRemainder(nAlignMode, nStringIndex, settings.m_szGloss);
            if (szNext.GetLength()==0)
            {
                szNext = CSaString(WORD_DELIMITER);
            }
            else
            {
                if (szNext[0]==WORD_DELIMITER)
                {
                    // do nothing
                }
                else
                {
                    szNext = CSaString(WORD_DELIMITER)+szNext;
                }
            }
            szNext.Remove(0x0d);
            szNext.Remove(0x0a);
            pSegment->SelectSegment(*this,nIndex);
            ((CGlossSegment *)pSegment)->ReplaceSelectedSegment(this,szNext);
        }
    }

    pView->ChangeAnnotationSelection(pSegment, -1);

    CGraphWnd * pGraph = pView->GraphIDtoPtr(IDD_RAWDATA);
    if (pGraph)
    {
        if (settings.m_bPhonetic)
        {
            pGraph->ShowAnnotation(PHONETIC, TRUE, TRUE);
        }
        if (settings.m_bPhonemic)
        {
            pGraph->ShowAnnotation(PHONEMIC, TRUE, TRUE);
        }
        if (settings.m_bOrthographic)
        {
            pGraph->ShowAnnotation(ORTHO, TRUE, TRUE);
        }
        if (settings.m_bGloss)
        {
            pGraph->ShowAnnotation(GLOSS, TRUE, TRUE);
        }
    }
    pView->RefreshGraphs(); // redraw all graphs without legend window
}

void CSaDoc::AlignTranscriptionDataByRef(CTranscriptionData & td)
{

    // save state for undo ability
    CheckPoint();
    SetModifiedFlag(TRUE); // document has been modified
    SetTransModifiedFlag(TRUE); // transcription has been modified

    POSITION pos = GetFirstViewPosition();
    CSaView * pView = (CSaView *)GetNextView(pos);

    // cycle through reference and assign the other fields
    CReferenceSegment * pReference = (CReferenceSegment *)GetSegment(REFERENCE);
    CGlossSegment * pGloss = GetGlossSegment();
    CPhoneticSegment * pPhonetic = (CPhoneticSegment *)GetSegment(PHONETIC);
    CPhonemicSegment * pPhonemic = (CPhonemicSegment *)GetSegment(PHONEMIC);
    COrthoSegment * pOrthographic = (COrthoSegment *)GetSegment(ORTHO);

    if (pReference->GetOffsetSize()==0)
    {
        AfxMessageBox(IDS_ERROR_TAT_NO_REFERENCE,MB_OK|MB_ICONEXCLAMATION,0);
        return;
    }

    const CStringArray & references = pReference->GetTexts();

    for (int i=0; i<pReference->GetOffsetSize(); i++)
    {
        CSaString thisRef = references.GetAt(i);
        DWORD start = pReference->GetOffset(i);
        DWORD duration = pReference->GetDuration(i);
        MarkerList::iterator git = td.m_TranscriptionData[td.m_MarkerDefs[GLOSS]].begin();
        MarkerList::iterator pmit = td.m_TranscriptionData[td.m_MarkerDefs[PHONEMIC]].begin();
        MarkerList::iterator pnit = td.m_TranscriptionData[td.m_MarkerDefs[PHONETIC]].begin();
        MarkerList::iterator oit = td.m_TranscriptionData[td.m_MarkerDefs[ORTHO]].begin();
        MarkerList refs = td.m_TranscriptionData[td.m_szPrimary];
        for (MarkerList::iterator it = refs.begin(); it!=refs.end(); it++)
        {
            CSaString thatRef = *it;
            if (thisRef.Compare(thatRef)==0)
            {
                if (td.m_bPhonetic)
                {
                    CSaString text = *pnit;
                    pPhonetic->SetAt(&text,false,start,duration);
                }
                if (td.m_bPhonemic)
                {
                    CSaString text = *pmit;
                    pPhonemic->SetAt(&text,false,start,duration);
                }
                if (td.m_bOrthographic)
                {
                    CSaString text = *oit;
                    pOrthographic->SetAt(&text,false,start,duration);
                }
                if (td.m_bGloss)
                {
                    CSaString text = *git;
                    if (text[0]==WORD_DELIMITER)
                    {
                        text = text.Mid(1);
                    }
                    pGloss->SetAt(&text,false,start,duration);
                }
            }
            if (td.m_bPhonetic)
            {
                pnit++;
            }
            if (td.m_bPhonemic)
            {
                pmit++;
            }
            if (td.m_bOrthographic)
            {
                oit++;
            }
            if (td.m_bGloss)
            {
                git++;
            }
        }
    }

    CGraphWnd * pGraph = pView->GraphIDtoPtr(IDD_RAWDATA);
    if (pGraph)
    {
        pGraph->ShowAnnotation(REFERENCE, TRUE, TRUE);
        if (td.m_bPhonetic)
        {
            pGraph->ShowAnnotation(PHONETIC, TRUE, TRUE);
        }
        if (td.m_bPhonemic)
        {
            pGraph->ShowAnnotation(PHONEMIC, TRUE, TRUE);
        }
        if (td.m_bOrthographic)
        {
            pGraph->ShowAnnotation(ORTHO, TRUE, TRUE);
        }
        if (td.m_bGloss)
        {
            pGraph->ShowAnnotation(GLOSS, TRUE, TRUE);
        }
    }
    pView->RefreshGraphs(); // redraw all graphs without legend window
}

/***************************************************************************/
// CSaDoc::BuildString builds an annotation string
/***************************************************************************/
const CSaString CSaDoc::BuildString(int nSegment)
{

    CSaString szBuild = "";
    CSaString szWorking = "";
    CSaString szSegmentDefault = SEGMENT_DEFAULT_CHAR;

    CSegment * pSegment = GetSegment(nSegment);
    if (pSegment == NULL)
    {
        return szBuild;
    }
    if (pSegment->IsEmpty())
    {
        return szBuild;
    }

    const char wordDelimiter = GetFont(nSegment)->m_wordDelimiter;

    int nIndex = 0;

    switch (nSegment)
    {
    case PHONETIC:
        while (nIndex != -1)
        {
            szWorking = pSegment->GetSegmentString(nIndex);
            if (szWorking != szSegmentDefault)
            {
                szBuild += szWorking;
                szBuild += wordDelimiter;
            }
            nIndex = pSegment->GetNext(nIndex);
        }
        break;

    case PHONEMIC:
    case ORTHO:
        while (nIndex != -1)
        {
            szWorking = pSegment->GetSegmentString(nIndex);
            szBuild += szWorking;
            szBuild += wordDelimiter;
            nIndex = pSegment->GetNext(nIndex);
        }
        break;

    case GLOSS:
        while (nIndex != -1)
        {
            szWorking = pSegment->GetSegmentString(nIndex);
            szBuild += szWorking;
            szBuild += wordDelimiter;
            nIndex = pSegment->GetNext(nIndex);
        }
        break;
    default:
        ;
    }
    return szBuild;
}

/***************************************************************************/
// CSaDoc::BuildString builds an annotation string
/***************************************************************************/
const CSaString CSaDoc::BuildImportString(BOOL /*gloss*/, BOOL /*phonetic*/, BOOL /*phonemic*/, BOOL /*orthographic*/)
{
    return CSaString("");
}

/**
* Read the incoming stream and return the transcription line
* This is used by the automatic transcription feature
* returns false on failure
*/
const bool CSaDoc::ImportTranscription(CSaString & filename, bool gloss, bool phonetic, bool phonemic, bool orthographic, CTranscriptionData & td, bool addTag, bool showDlg)
{

    td.m_MarkerDefs[REFERENCE] = psz_Reference;
    td.m_szPrimary = psz_Reference;

    // setup the default list
    td.m_Markers.push_back(psz_Reference);
    if (phonetic)
    {
        td.m_MarkerDefs[PHONETIC] = psz_Phonetic;
        td.m_Markers.push_back(psz_Phonetic);
        td.m_bPhonetic = true;
    }
    if (phonemic)
    {
        td.m_MarkerDefs[PHONEMIC] = psz_Phonemic;
        td.m_Markers.push_back(psz_Phonemic);
        td.m_bPhonemic = true;
    }
    if (orthographic)
    {
        td.m_MarkerDefs[ORTHO] = psz_Orthographic;
        td.m_Markers.push_back(psz_Orthographic);
        td.m_bOrthographic = true;
    }
    if (gloss)
    {
        td.m_MarkerDefs[GLOSS] = psz_Gloss;
        td.m_Markers.push_back(psz_Gloss);
        td.m_bGloss = true;
    }

    if (CSFMHelper::IsSFM(filename))
    {
		if (showDlg)
		{
			CDlgImportSFMRef dlg(phonetic, phonemic, orthographic, gloss);
			int result = dlg.DoModal();
			if (result==IDCANCEL)
			{
				return true;
			}

			if (result==IDC_IMPORT_PLAIN_TEXT)
			{
				return CTextHelper::ImportText(filename,td.m_szPrimary,td.m_Markers,td.m_TranscriptionData, addTag);
			}

			// proceeding as SFM
			// order is important here!
			td.m_Markers.clear();
			td.m_MarkerDefs[REFERENCE] = dlg.m_szReference;
			td.m_Markers.push_back(dlg.m_szReference);

			if (dlg.m_bPhonetic)
			{
				td.m_MarkerDefs[PHONETIC] = dlg.m_szPhonetic;
				td.m_Markers.push_back(dlg.m_szPhonetic);
				td.m_bPhonetic = true;
			}
			if (dlg.m_bPhonemic)
			{
				td.m_MarkerDefs[PHONEMIC] = dlg.m_szPhonemic;
				td.m_Markers.push_back(dlg.m_szPhonemic);
				td.m_bPhonemic = true;
			}
			if (dlg.m_bOrthographic)
			{
				td.m_MarkerDefs[ORTHO] = dlg.m_szOrthographic;
				td.m_Markers.push_back(dlg.m_szOrthographic);
				td.m_bOrthographic = true;
			}
			if (dlg.m_bGloss)
			{
				td.m_MarkerDefs[GLOSS] = dlg.m_szGloss;
				td.m_Markers.push_back(dlg.m_szGloss);
				td.m_bGloss = true;
			}
		}

        if (CSFMHelper::IsMultiRecordSFM(filename, td.m_MarkerDefs[REFERENCE]))
        {
            td.m_TranscriptionData = CSFMHelper::ImportMultiRecordSFM(filename, td.m_MarkerDefs[REFERENCE], td.m_Markers, addTag);
            return true;
        }
		if (CSFMHelper::IsColumnarSFM(filename))
		{
            td.m_TranscriptionData = CSFMHelper::ImportColumnarSFM(filename);
            return true;
		}

        //map = CSFMHelper::ImportSFM( stream);
        return false;
    }

    // proceeding as text
    return CTextHelper::ImportText(filename,td.m_szPrimary,td.m_Markers,td.m_TranscriptionData, addTag);
}

int CSaDoc::GetSegmentSize(EAnnotation nIndex)
{
    return m_apSegments[nIndex]->GetOffsetSize();
}

CSegment * CSaDoc::GetSegment(int nIndex)
{
    return m_apSegments[nIndex];
}

CGlossSegment * CSaDoc::GetGlossSegment()
{
    return (CGlossSegment *)m_apSegments[GLOSS];
}

CSaString CSaDoc::GetMusicScore()
{
    return m_szMusicScore;
}
BOOL CSaDoc::IsBackgroundProcessing()
{
    return m_bProcessBackground;
}
BOOL CSaDoc::EnableBackgroundProcessing(BOOL bState)
{
    BOOL result = m_bProcessBackground;
    m_bProcessBackground = bState;
    return result;
}
CProcessDoc * CSaDoc::GetUnprocessed()
{
    return &m_ProcessDoc;
}
CProcessAdjust * CSaDoc::GetAdjust()
{
    return m_pProcessAdjust;
}
CProcessFragments * CSaDoc::GetFragments()
{
    return m_pProcessFragments;
}
CProcessLoudness * CSaDoc::GetLoudness()
{
    return m_pProcessLoudness;
}
CProcessSmoothLoudness * CSaDoc::GetSmoothLoudness()
{
    return m_pProcessSmoothLoudness;
}
CProcessZCross * CSaDoc::GetZCross()
{
    return m_pProcessZCross;
}
CProcessPitch * CSaDoc::GetPitch()
{
    return m_pProcessPitch;
}
CProcessCustomPitch * CSaDoc::GetCustomPitch()
{
    return m_pProcessCustomPitch;
}
CProcessSmoothedPitch * CSaDoc::GetSmoothedPitch()
{
    return m_pProcessSmoothedPitch;
}
CProcessChange * CSaDoc::GetChange()
{
    return m_pProcessChange;
}
CProcessRaw * CSaDoc::GetRaw()
{
    return m_pProcessRaw;
}
CProcessHilbert * CSaDoc::GetHilbert()
{
    return m_pProcessHilbert;
}
CProcessWavelet * CSaDoc::GetWavelet()
{
    return m_pProcessWavelet;
}
CProcessSpectrum * CSaDoc::GetSpectrum()
{
    return m_pProcessSpectrum;
}
CProcessGrappl * CSaDoc::GetGrappl()
{
    return m_pProcessGrappl;
}
CProcessMelogram * CSaDoc::GetMelogram()
{
    return m_pProcessMelogram;
}
CProcessFormants * CSaDoc::GetFormants()
{
    return m_pProcessFormants;
}
CProcessFormantTracker * CSaDoc::GetFormantTracker()
{
    return m_pProcessFormantTracker;
}
CProcessDurations * CSaDoc::GetDurations()
{
    return m_pProcessDurations;
}
CProcessRatio * CSaDoc::GetRatio()
{
    return m_pProcessRatio;
}
CProcessPOA * CSaDoc::GetPOA()
{
    return m_pProcessPOA;
}
CProcessGlottis * CSaDoc::GetGlottalWave()
{
    return m_pProcessGlottis;
}
CProcessTonalWeightChart * CSaDoc::GetTonalWeightChart()
{
    return m_pProcessTonalWeightChart;
}
CFontTable  * CSaDoc::GetFont(int nIndex)
{
    return (CFontTable *)m_pCreatedFonts->GetAt(nIndex);
}

void CSaDoc::SetAudioModifiedFlag(bool bMod)
{
    m_bAudioModified = bMod;
}

bool CSaDoc::IsAudioModified() const
{
    return m_bAudioModified;
}

void CSaDoc::SetTransModifiedFlag(bool bMod)
{
	if (m_AutoSave.IsSaving())
    {
        return;
    }
    m_bTransModified = bMod;
}

bool CSaDoc::IsTransModified() const
{
    return m_bTransModified;
}

void CSaDoc::SetID(int nID)
{
    m_ID = nID;
}

int CSaDoc::GetID()
{
    return m_ID;
}

void CSaDoc::SetWbProcess(int nProcess)
{
    m_nWbProcess = nProcess;
}

int CSaDoc::GetWbProcess()
{
    return m_nWbProcess;
}

CFileStatus * CSaDoc::GetFileStatus()
{
    return &m_fileStat;
}

SaParm * CSaDoc::GetSaParm()
{
    return &m_saParm;
}

SourceParm * CSaDoc::GetSourceParm()
{
    return &m_sourceParm;
}

const CPitchParm * CSaDoc::GetPitchParm() const
{
    return &m_pitchParm;
}

void CSaDoc::SetPitchParm(const CPitchParm & parm)
{
    m_pitchParm = parm;
}

const CMusicParm * CSaDoc::GetMusicParm() const
{
    return &m_musicParm;
}

void CSaDoc::SetMusicParm(const CMusicParm & parm)
{
    m_musicParm = parm;
}

const CIntensityParm & CSaDoc::GetCIntensityParm() const
{
    return m_intensityParm;
}

void CSaDoc::SetCIntensityParm(const CIntensityParm & parm)
{
    m_intensityParm = parm;
}

SDPParm * CSaDoc::GetSDPParm()
{
    return &m_SDPParm;
}

/***************************************************************************/
// CSaDoc::CopySectionToNewWavFile Copies wave data out of the wave file
// and places in a new wav file
// dwSectionStart the start index in bytes (single channel)
// dwSectionLength the length in bytes (single channel)
/***************************************************************************/
BOOL CSaDoc::CopySectionToNewWavFile( WAVETIME sectionStart, WAVETIME sectionLength, LPCTSTR szNewWave, BOOL usingClipboard)
{

    CSaString szOriginalWave;
    if (m_bUsingTempFile)
    {
        szOriginalWave = m_szTempConvertedWave;
    }
    else if ((GetPathName().GetLength() !=0))
    {
        szOriginalWave = GetPathName();
    }
    else if (m_szTempWave.GetLength() !=0)
    {
        szOriginalWave = m_szTempWave;
    }

    if (szOriginalWave.GetLength() == 0)
    {
        //Original not found
        return FALSE;
    }

    BOOL bSameFileName = (szNewWave == szOriginalWave);
    if ((!bSameFileName) && (!CopyWave( szOriginalWave, szNewWave)))
    {
        RemoveFile(szNewWave);
        return FALSE;
    }

    // Copy wanted portion of the wave data file
    TCHAR szTempNewTemp[_MAX_PATH];
    GetTempFileName(_T("TMP"), szTempNewTemp, _countof(szTempNewTemp));

	if (!CopyWave( m_szRawDataWrk.c_str(), szTempNewTemp, sectionStart, sectionLength, TRUE))
    {
        if (!bSameFileName)
        {
            RemoveFile(szNewWave);
        }
        RemoveFile(szTempNewTemp);
        return FALSE;
    }

    if (!bSameFileName)
    {
        //Save segment data we will use this documents segments for calculations
        CheckPoint();  // save file state for Undo below
    }

    // Set segments to selected wave
    // adjust segments to new file size
	WAVETIME length = toTime( GetDataSize(), true);
    AdjustSegments( sectionStart+sectionLength, length-(sectionStart+sectionLength), true);
    AdjustSegments( 0, sectionStart, TRUE);

    if ( bSameFileName)
    {
        // Set document to use new wave data
		m_dwDataSize = toBytes( sectionLength, false);

        RemoveFile(m_szRawDataWrk.c_str());  // Done with this file

        m_szRawDataWrk = szTempNewTemp;
        OnFileSave();
        InvalidateAllProcesses();

        POSITION pos = GetFirstViewPosition();
        CSaView * pView = ((CSaView *)GetNextView(pos));

        pView->SetStartStopCursorPosition(0, GetDataSize());
        pView->RefreshGraphs();

        while (CanUndo())
        {
            // remove undo list
            Undo(FALSE, FALSE);
        }
        return TRUE;
    }

    //Copy key document parameters
    wstring szTempName = m_szRawDataWrk;
    DWORD dwDataSize = GetRawDataSize();
    WORD wFlags = m_saParm.wFlags;

    // Set document to use new wave data
	m_dwDataSize = toBytes( sectionLength, true);

    // Create Wave file
    try
    {
        m_szRawDataWrk = szTempNewTemp;
        if (!WriteDataFiles(szNewWave, TRUE, usingClipboard))
        {
            AfxThrowFileException(CFileException::genericException, -1);
        }
        //Restore Document
        m_saParm.wFlags = wFlags;
        m_dwDataSize = dwDataSize;
        m_szRawDataWrk = szTempName;
        RemoveFile(szTempNewTemp);  // Done with this file
        Undo(FALSE);  // return segments to original state
    }
    catch (const CException &)
    {
        m_szRawDataWrk = szTempName;
        m_dwDataSize = dwDataSize;
        RemoveFile(szNewWave);
        RemoveFile(szTempNewTemp);
        Undo(FALSE);
        return FALSE;
    }

    // save path for copying transcriptions
    ((CSaApp *)AfxGetApp())->SetLastClipboardPath(szNewWave);

    return TRUE;
}

void CSaDoc::DoExportFieldWorks(CExportFWSettings & settings)
{

    int dataCount = 0;
    int sfmCount = 0;
    int wavCount = 0;

    wstring filename;
    TCHAR szBuffer[MAX_PATH];
    wcscpy_s(szBuffer,MAX_PATH,settings.szPath);
    int result = GetSaveAsFilename(settings.szDocTitle, _T("Standard Format (*.sfm) |*.sfm||"), _T("sfm"), szBuffer, filename);
    if (result!=IDOK) return;

    if (filename.length()==0)
    {
        CSaApp * pApp = (CSaApp *)AfxGetApp(); // get pointer to application
        pApp->ErrorMessage(IDS_ERROR_NO_FW_FILE);
        return;
    }

    bool skipEmptyGloss = true;

    TCHAR szPath[MAX_PATH];
    memset(szPath, 0, MAX_PATH);
    wcscpy_s(szPath,MAX_PATH,settings.szPath);

    if (!FolderExists(szPath))
    {
        CreateFolder(szPath);
    }
    wcscat_s(szPath,MAX_PATH,L"LinkedFiles\\");
    if (!FolderExists(szPath))
    {
        CreateFolder(szPath);
    }
    wcscat_s(szPath,MAX_PATH,L"AudioVisual\\");
    
	if (!FolderExists(szPath))
    {
        CreateFolder(szPath);
    }

    if (!ValidateWordFilenames(WFC_REF_GLOSS,skipEmptyGloss,L"",L"")) return;
    if (!ValidatePhraseFilenames(MUSIC_PL1,PFC_REF_GLOSS,L"",L"")) return;
    if (!ValidatePhraseFilenames(MUSIC_PL2,PFC_REF_GLOSS,L"",L"")) return;

    CFile file(filename.c_str(), CFile::modeCreate | CFile::modeWrite);
    CSaString szString;

    if (!TryExportSegmentsBy(settings,REFERENCE, file, skipEmptyGloss, szPath, dataCount, wavCount))
    {
        if (!TryExportSegmentsBy(settings,GLOSS, file, skipEmptyGloss, szPath, dataCount, wavCount))
        {
            if (!TryExportSegmentsBy(settings,ORTHO, file, skipEmptyGloss, szPath, dataCount, wavCount))
            {
                if (!TryExportSegmentsBy(settings,PHONEMIC, file, skipEmptyGloss, szPath, dataCount, wavCount))
                {
                    if (!TryExportSegmentsBy(settings,TONE, file, skipEmptyGloss, szPath, dataCount, wavCount))
                    {
                        TryExportSegmentsBy(settings,PHONETIC, file, skipEmptyGloss, szPath, dataCount, wavCount);
                    }
                }
            }
        }
    }

    // \date write current time
    CTime time = CTime::GetCurrentTime();
    szString = "\\dt " + time.Format("%A, %B %d, %Y, %X") + "\r\n";
    WriteFileUtf8(&file, szString);

    file.Close();
    sfmCount++;

    CDlgExportFWResult dlg;
    dlg.m_WAVCount.Format(L"%d",wavCount);
    dlg.m_DataCount.Format(L"%d",dataCount);
    dlg.m_SFMCount.Format(L"%d",sfmCount);
    dlg.DoModal();

}

bool CSaDoc::TryExportSegmentsBy(CExportFWSettings & settings, EAnnotation master, CFile & file, bool skipEmptyGloss, LPCTSTR szPath, int & dataCount, int & wavCount)
{

    TRACE("EXPORTING>>>>%d\n",master);

    LPCTSTR szCrLf = L"\r\n";

    EWordFilenameConvention wordConvention = WFC_REF_GLOSS;
    EPhraseFilenameConvention phraseConvention = PFC_REF_GLOSS;

    if (!GetFlag(master,settings))
    {
        return false;
    }

    CSegment * pSeg = GetSegment(master);

    if (pSeg->GetOffsetSize() == 0)
    {
        return false;
    }

    WriteFileUtf8(&file, szCrLf);

    CSaString results[ANNOT_WND_NUMBER];
    for (int i = 0; i < ANNOT_WND_NUMBER; i++)
    {
        results[i] = L"";
    }
    DWORD last = pSeg->GetOffset(0) - 1;
    for (int i = 0; i < pSeg->GetOffsetSize(); i++)
    {
        DWORD dwStart = pSeg->GetOffset(i);
        DWORD dwStop = pSeg->GetStop(i);
        if (dwStart == last)
        {
            continue;
        }
        last = dwStart;
        for (int j = master; j >= 0; j--)
        {
            EAnnotation target = GetAnnotation(j);
            if (!GetFlag(target,settings))
            {
                continue;
            }
            results[target] = BuildRecord(target, dwStart, dwStop);
        }

        if (results[PHONETIC].GetLength() > 0)
        {
            WriteFileUtf8(&file, results[PHONETIC]);
        }
        if (results[TONE].GetLength() > 0)
        {
            WriteFileUtf8(&file, results[TONE]);
        }
        if (results[PHONEMIC].GetLength() > 0)
        {
            WriteFileUtf8(&file, results[PHONEMIC]);
        }
        if (results[ORTHO].GetLength() > 0)
        {
            WriteFileUtf8(&file, results[ORTHO]);
        }
        if (results[GLOSS].GetLength() > 0)
        {
            WriteFileUtf8(&file, results[GLOSS]);
        }
        if (results[REFERENCE].GetLength() > 0)
        {
            WriteFileUtf8(&file, results[REFERENCE]);
        }

        POSITION pos = GetFirstViewPosition();
        CSaView * pView = (CSaView *) GetNextView(pos);  // get pointer to view
        CGlossSegment * g = (CGlossSegment *)pView->GetAnnotation(GLOSS);
        CMusicPhraseSegment * pl1 = (CMusicPhraseSegment *)pView->GetAnnotation(MUSIC_PL1);
        CMusicPhraseSegment * pl2 = (CMusicPhraseSegment *)pView->GetAnnotation(MUSIC_PL2);
        DWORD offsetSize = g->GetOffsetSize();
        bool hasGloss = (offsetSize != 0);

        TRACE("gloss %d %d\n",dwStart,dwStop);

        if ((hasGloss) || (!skipEmptyGloss))
        {

            wstring filename;
            int index = FindNearestGlossIndex(g,dwStart,dwStop);
            if (index>=0)
            {
                int result = ComposeWordSegmentFilename(g, index, wordConvention, szPath, filename, L"", L"");
                if (result==0)
                {
                    int result = ExportWordSegment(g, index, filename.c_str(), skipEmptyGloss, dataCount, wavCount);
                    if (result<0)
                    {
                        return false;
                    }
                    TCHAR szBuffer[MAX_PATH];
                    wmemset(szBuffer,0,MAX_PATH);
                    wcscat_s(szBuffer,MAX_PATH,L"\\pf ");
                    wcscat_s(szBuffer,MAX_PATH,filename.c_str());
                    wcscat_s(szBuffer,MAX_PATH,szCrLf);
                    WriteFileUtf8(&file, szBuffer);
                }
            }

            if (settings.bPhrase)
            {
                TRACE("--searching for PL1\n");
                index = FindNearestPhraseIndex(pl1,dwStart,dwStop);
                if (index>=0)
                {
                    TRACE("--exporting PL1\n");
                    int result = ComposePhraseSegmentFilename(MUSIC_PL1, pl1, index, phraseConvention, szPath, filename, L"", L"");
                    if (result==0)
                    {
                        int result = ExportPhraseSegment(pl1, index, filename, dataCount, wavCount);
                        if (result<0)
                        {
                            return false;
                        }
                        TCHAR szBuffer[MAX_PATH];
                        wmemset(szBuffer,0,MAX_PATH);
                        wcscat_s(szBuffer,MAX_PATH,L"\\pf ");
                        wcscat_s(szBuffer,MAX_PATH,filename.c_str());
                        wcscat_s(szBuffer,MAX_PATH,szCrLf);
                        WriteFileUtf8(&file, szBuffer);
                    }
                }

                TRACE("--searching for PL2\n");
                index = FindNearestPhraseIndex(pl2,dwStart,dwStop);
                if (index>=0)
                {
                    TRACE("--exporting PL2\n");
                    int result = ComposePhraseSegmentFilename(MUSIC_PL2, pl2, index, phraseConvention, szPath, filename, L"", L"");
                    if (result==0)
                    {
                        int result = ExportPhraseSegment(pl2, index, filename, dataCount, wavCount);
                        if (result<0)
                        {
                            return false;
                        }
                        TCHAR szBuffer[MAX_PATH];
                        wmemset(szBuffer,0,MAX_PATH);
                        wcscat_s(szBuffer,MAX_PATH,L"\\pf ");
                        wcscat_s(szBuffer,MAX_PATH,filename.c_str());
                        wcscat_s(szBuffer,MAX_PATH,szCrLf);
                        WriteFileUtf8(&file, szBuffer);
                    }
                }
            }
        }

        WriteFileUtf8(&file, szCrLf);
    }

    return true;
}

CSaString CSaDoc::BuildRecord(EAnnotation target, DWORD dwStart, DWORD dwStop)
{

    LPCTSTR szCrLf = L"\r\n";
    CSaString szTag = GetTag(target);
    CSegment * pSegment = GetSegment(target);
    CSaString szText = pSegment->GetContainedText(dwStart, dwStop);
    szText = szText.Trim();
    if (szText.GetLength() == 0)
    {
        return L"";
    }
    if (target == GLOSS)
    {
        if (szText[0] == WORD_DELIMITER)
        {
            szText = szText.Right(szText.GetLength() - 1);
        }
    }
    return szTag + L" " + szText + szCrLf;
}

BOOL CSaDoc::GetFlag(EAnnotation val, CExportFWSettings & settings)
{
    switch (val)
    {
    case PHONETIC:
        return settings.bPhonetic;
    case PHONEMIC:
        return settings.bPhonemic;
    case ORTHO:
        return settings.bOrtho;
    case GLOSS:
        return settings.bGloss;
    case REFERENCE:
        return settings.bReference;
    case MUSIC_PL1:
        return settings.bPhrase;
    case MUSIC_PL2:
        return settings.bPhrase;
    case MUSIC_PL3:
        return settings.bPhrase;
    case MUSIC_PL4:
        return settings.bPhrase;
    }
    return false;
}

int CSaDoc::GetIndex(EAnnotation val)
{
    switch (val)
    {
    case PHONETIC:
        return 0;
    case TONE:
        return 1;
    case PHONEMIC:
        return 2;
    case ORTHO:
        return 3;
    case GLOSS:
        return 4;
    case REFERENCE:
        return 5;
    case MUSIC_PL1:
        return 6;
    case MUSIC_PL2:
        return 7;
    case MUSIC_PL3:
        return 8;
    case MUSIC_PL4:
        return 9;
    }
    return false;
}

LPCTSTR CSaDoc::GetTag(EAnnotation val)
{
    switch (val)
    {
    case PHONETIC:
        return L"\\lx-ph";
    case TONE:
        return L"\\tn";
    case PHONEMIC:
        return L"\\lx-pm";
    case ORTHO:
        return L"\\lx-or";
    case GLOSS:
        return L"\\ge";
    case REFERENCE:
        return L"\\rf";
    case MUSIC_PL1:
        return L"\\pf";
    case MUSIC_PL2:
        return L"\\tn";
    case MUSIC_PL3:
        return L"\\pf";
    case MUSIC_PL4:
        return L"\\tn";
    }
    return L"";
}

EAnnotation CSaDoc::GetAnnotation(int val)
{
    switch (val)
    {
    case 0:
        return PHONETIC;
    case 1:
        return TONE;
    case 2:
        return PHONEMIC;
    case 3:
        return ORTHO;
    case 4:
        return GLOSS;
    case 5:
        return REFERENCE;
    case 6:
        return MUSIC_PL1;
    case 7:
        return MUSIC_PL2;
    case 8:
        return MUSIC_PL3;
    case 9:
        return MUSIC_PL4;
    }
    return PHONETIC;
}

void CSaDoc::WriteFileUtf8(CFile * pFile, const CSaString szString)
{
    std::string szUtf8 = szString.utf8();
    pFile->Write(szUtf8.c_str(), szUtf8.size());
}

bool CSaDoc::HasSegmentData(EAnnotation val)
{
    return (m_apSegments[val]->GetOffsetSize()>0)?true:false;
}

LPCTSTR CSaDoc::GetProcessFilename()
{
    return m_szRawDataWrk.c_str();
}

CDocTemplate * CSaDoc::GetTemplate()
{
    return m_pDocTemplate;
}

const CUttParm * CSaDoc::GetUttParm()
{
    // pointer to utterance parameters structure
    return &m_uttParm;
}

BOOL CSaDoc::IsWaveToUndo()
{
    // return TRUE, if wave file change is to undo
    return m_bWaveUndoNow;
}

HPSTR CSaDoc::GetUnprocessedWaveData(DWORD dwOffset, BOOL bBlockBegin)
{
	//TRACE("GetUnprocessedWaveData %d %d\n", dwOffset, bBlockBegin);
    return m_ProcessDoc.GetProcessedWaveData( GetProcessFilename(), GetSelectedChannel(), GetNumChannels(), GetSampleSize(), dwOffset, bBlockBegin);
}

DWORD CSaDoc::GetUnprocessedWaveDataBufferSize()
{
    return m_ProcessDoc.GetBufferSize();
}

void * CSaDoc::GetUnprocessedDataBlock(DWORD dwByteOffset, size_t sObjectSize, BOOL bReverse)
{
    return m_ProcessDoc.GetProcessedDataBlock(GetProcessFilename(), GetSelectedChannel(), GetNumChannels(), GetSampleSize(), dwByteOffset, sObjectSize, bReverse);
}

DWORD CSaDoc::GetUnprocessedBufferIndex(size_t nSize)
{
    return m_ProcessDoc.GetProcessBufferIndex(nSize);
}

DWORD CSaDoc::GetSelectedChannel()
{
    return m_nSelectedChannel;
}

// get the sample size in bytes for a single channel
DWORD CSaDoc::GetSampleSize() const
{
    return m_FmtParm.GetSampleSize();
}

WAVETIME CSaDoc::GetTimeFromBytes( DWORD dwSize)
{
    // return sampled data size in seconds
	WAVETIME result = dwSize;
	result /= (((double)m_FmtParm.dwAvgBytesPerSec)/((double)m_FmtParm.wChannels));
	return result;
}

DWORD CSaDoc::GetBytesFromTime(WAVETIME fSize)
{
    // return sampled data size in bytes
    return (DWORD)(((double)fSize)*(((double)m_FmtParm.dwAvgBytesPerSec)/((double)m_FmtParm.wChannels)));
}

/**
* return the number of bytes used.
* @param singleChannel if true return bytes used for a single channel
*/
DWORD CSaDoc::GetBytesPerSample( bool singleChannel)
{
	if (singleChannel)
	{
		DWORD result = m_FmtParm.wBlockAlign;
		result /= m_FmtParm.wChannels;
		return result;
	}
	return m_FmtParm.wBlockAlign;
}

DWORD CSaDoc::GetSamplesPerSec()
{
    return m_FmtParm.dwSamplesPerSec;
}

WORD CSaDoc::GetBitsPerSample()
{
    return m_FmtParm.wBitsPerSample;
}

WORD CSaDoc::GetBlockAlign( bool singleChannel)
{
	if (singleChannel)
	{
		return m_FmtParm.wBlockAlign/m_FmtParm.wChannels;
	}
    return m_FmtParm.wBlockAlign;
}

bool CSaDoc::Is16Bit()
{
    return ((m_FmtParm.wBlockAlign/m_FmtParm.wChannels)==2);
}

bool CSaDoc::IsPCM()
{
    return (m_FmtParm.wTag==FILE_FORMAT_PCM);
}

DWORD CSaDoc::GetAvgBytesPerSec()
{
    return m_FmtParm.dwAvgBytesPerSec;
}

DWORD CSaDoc::GetNumChannels() const
{
    return m_FmtParm.wChannels;
}

DWORD CSaDoc::GetNumSamples() const
{
    return ( GetRawDataSize() / m_FmtParm.GetSampleSize())/ GetNumChannels();
}

CSaString CSaDoc::GetTempFilename()
{
	return m_szTempWave;
}

bool CSaDoc::IsUsingTempFile()
{
	return (!m_szTempWave.IsEmpty());
}

void CSaDoc::StoreAutoRecoveryInformation()
{
	m_AutoSave.StoreAutoRecoveryInformation(this);
}

wstring CSaDoc::GetFilenameFromTitle()
{
    wstring result = GetTitle();			// load file name
    size_t nFind = result.find(':');
    if (nFind != wstring::npos)
    {
        result = result.substr(0,nFind-1);  // extract part left of :
    }
    return result;
}

wstring CSaDoc::GetTranscriptionFilename()
{
    // a prerecorded file
    wstring result = GetPathName();
	result = result.substr( 0, result.length()-4);
	result.append(L".saxml");
	return result;
}

/*
* convert a position - in bytes into time.
*/
WAVETIME CSaDoc::toTime( CURSORPOS bytes, bool singleChannel)
{
	WAVETIME result = (WAVETIME)bytes;
	result /= (WAVETIME)GetBytesPerSample(singleChannel);
	result /= (WAVETIME)GetSamplesPerSec();
	return result;
}

WAVETIME CSaDoc::toTimeFromSamples( WAVESAMP samples)
{
	WAVETIME result = (WAVETIME)samples;
	result /= (WAVETIME)GetSamplesPerSec();
	return result;
}

/**
* convert cursor position ( a single channel) into time
*/
WAVETIME CSaDoc::toTime( CURSORPOS val)
{
	WAVETIME result = (WAVETIME)val;
	result /= (WAVETIME)GetBytesPerSample(true);
	result /= (WAVETIME)GetSamplesPerSec();
	return result;
}

DWORD CSaDoc::toBytes( WAVETIME val, bool singleChannel)
{
	val *= GetSamplesPerSec();
	// round up to nearest sample
	DWORD dwResult = ::ceil(val);
	dwResult *= GetBytesPerSample(singleChannel);
	return dwResult;
}

WAVESAMP CSaDoc::toSamples( WAVETIME val)
{
	val *= GetSamplesPerSec();
	WAVESAMP dwResult = ::ceil(val);
	return (WAVESAMP)dwResult;
}

CURSORPOS CSaDoc::toCursor( WAVETIME val)
{
	val *= GetSamplesPerSec();
	// round up to nearest sample
	CURSORPOS dwResult = ::ceil(val);
	dwResult *= GetBytesPerSample(true);
	return (CURSORPOS)dwResult;
}

CURSORPOS CSaDoc::toCursor( WAVESAMP val)
{
	val *= GetBytesPerSample(true);
	return (CURSORPOS)val;
}

void CSaDoc::OnToolsAdjustSilence()
{
    CheckPoint();

    POSITION pos = GetFirstViewPosition();
    CSaView * pView = (CSaView *)GetNextView(pos);  // get pointer to view
	CURSORPOS oldStart = pView->GetStartCursorPosition();
	CURSORPOS oldStop = pView->GetStopCursorPosition();

	CDlgInsertSilence dlg(this);
	if (dlg.DoModal()!=IDOK)
	{
		UndoWaveFile();
		// get wave from the clipboard
		InvalidateAllProcesses();
		// restore the cursors
		pView->SetStartCursorPosition(oldStart);
		pView->SetStopCursorPosition(oldStop);
		pView->RefreshGraphs(TRUE);
	}
}

void CSaDoc::OnUpdateToolsAdjustSilence(CCmdUI * pCmdUI)
{
	pCmdUI->Enable(TRUE);
}

