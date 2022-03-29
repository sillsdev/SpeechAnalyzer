/////////////////////////////////////////////////////////////////////////////
// sa_doc.cpp:
// Implementation of the CSaDoc class.
// Author: Urs Ruchti
// copyright 2000 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "sa_doc.h"
#include "AlignInfo.h"
#include "ArchiveTransfer.h"
#include "AutoSave.h"
#include "ClipboardHelper.h"
#include "DependentSegment.h"
#include "DlgAutoRecorder.h"
#include "DlgAutoReferenceData.h"
#include "DlgExportFW.h"
#include "DlgExportFWResult.h"
#include "DlgExportLiftResult.h"
#include "DlgImportElanSheet.h"
#include "DlgImportSFMRef.h"
#include "DlgInsertSilence.h"
#include "DlgMultichannel.h"
#include "DlgPlayer.h"
#include "DlgSaveAsOptions.h"
#include "ExportFWSettings.h"
#include "ExportLiftSettings.h"
#include "FileEncodingHelper.h"
#include "FileUtils.h"
#include "FmtParm.h"
#include "GlossNatSegment.h"
#include "GlossSegment.h"
#include "IndependentSegment.h"
#include "ISa_Doc.h"
#include "LiftUtils.h"
#include "mainfrm.h"
#include "MusicPhraseSegment.h"
#include "objectistream.h"
#include "objectostream.h"
#include "OrthoSegment.h"
#include "PhonemicSegment.h"
#include "PhoneticSegment.h"
#include "ReferenceSegment.h"
#include "sa.h"
#include "SaParam.h"
#include "SaString.h"
#include "SAXMLUtils.h"
#include "sa_graph.h"
#include "sa_g_stf.h"
#include "sa_plot.h"
#include "sa_view.h"
#include "sa_wbch.h"
#include "ScopedCursor.h"
#include "Segment.h"
#include "SegmentOps.h"
#include "SFMHelper.h"
#include "Shlobj.h"
#include "sourceParm.h"
#include "SplitFileUtils.h"
#include "TextHelper.h"
#include "ToneSegment.h"
#include "TranscriptionData.h"
#include "TranscriptionDataSettings.h"
#include "TranscriptionHelper.h"
#include "undoredo.h"
#include "WaveUtils.h"
#include <ElanUtils.h>
#include <io.h>
#include <LiftUtils.h>
#include <uriparser/uri.h>
#include "AutoSegmentation.h"
#include "ScopedStatusBar.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

#pragma comment( lib, "waveutils")
#pragma comment( lib, "xmlutils")
#pragma comment( lib, "liftutils")
#pragma comment( lib, "elanutils")
#pragma comment( lib, "fileutils")
#pragma comment( lib, "uriparser")
#pragma comment( lib, "saprocessor")

//#ifdef _DEBUG
//#pragma comment( lib, "xerces-c_3d")
//#else
//#pragma comment( lib, "xerces-c_3")
//#endif

//###########################################################################
// CSaDoc
// Document class. Uses processing classes, to process wave data.

static BOOL s_bDocumentWasAlreadyOpen = FALSE;  // 1996-09-05 MRP

IMPLEMENT_DYNCREATE(CSaDoc, CUndoRedoDoc)

/////////////////////////////////////////////////////////////////////////////
// CSaDoc message map
// SDM 1.06.5 removed unused messages
static LPCSTR IMPORT_END = "import";
static const wchar_t * EMPTY = L"";

// default tags for text importing
static LPCSTR psz_Phonemic = "pm";
static LPCSTR psz_Gloss = "gl";
static LPCSTR psz_GlossNat = "gn";
static LPCSTR psz_Phonetic = "ph";
static LPCSTR psz_Orthographic = "or";
static LPCSTR psz_Reference = "ref";

static LPCSTR psz_sadoc = "sadoc";
static LPCSTR psz_wndlst = "wndlst";
static LPCSTR psz_saview = "saview";

CSaDoc::CSaDoc() :
	segmentOps(*this),
	sabLoaded(false) {
	printf("constructor called\n");
	App * app = (CSaApp*)AfxGetApp();
	m_pProcessDoc = make_unique<CProcessDoc>( app, this);
	m_bAudioModified = false;
	m_bTransModified = false;
	m_bTempOverlay = false;
	m_ID = -1;

	Context & context = GetContext();
	// create segments
	m_apSegments[PHONETIC] = new CPhoneticSegment(context,PHONETIC);
	m_apSegments[TONE] = new CToneSegment(context, TONE, PHONETIC);
	m_apSegments[PHONEMIC] = new CPhonemicSegment(context, PHONEMIC, PHONETIC);
	m_apSegments[ORTHO] = new COrthographicSegment(context, ORTHO, PHONETIC);
	m_apSegments[GLOSS] = new CGlossSegment(context, GLOSS, PHONETIC);
	m_apSegments[GLOSS_NAT] = new CGlossNatSegment(context, GLOSS_NAT, GLOSS);
	m_apSegments[REFERENCE] = new CReferenceSegment(context, REFERENCE, GLOSS);
	m_apSegments[MUSIC_PL1] = new CMusicPhraseSegment(context, MUSIC_PL1);
	m_apSegments[MUSIC_PL2] = new CMusicPhraseSegment(context, MUSIC_PL2);
	m_apSegments[MUSIC_PL3] = new CMusicPhraseSegment(context, MUSIC_PL3);
	m_apSegments[MUSIC_PL4] = new CMusicPhraseSegment(context, MUSIC_PL4);

	// enable background processing
	m_bProcessBackground = TRUE;

	// create data processing object
	m_pProcessAdjust = new CProcessAdjust(context);
	m_pProcessFragments = new CProcessFragments(context);
	m_pProcessLoudness = new CProcessLoudness(context);
	m_pProcessSmoothLoudness = new CProcessSmoothLoudness(context);
	m_pProcessPitch = new CProcessPitch(context);
	m_pProcessCustomPitch = new CProcessCustomPitch(context);
	m_pProcessSmoothedPitch = new CProcessSmoothedPitch(context);
	m_pProcessGrappl = new CProcessGrappl(context);
	m_pProcessMelogram = new CProcessMelogram(context);
	m_pProcessChange = new CProcessChange(context);
	m_pProcessRaw = new CProcessRaw(context);
	m_pProcessHilbert = new CProcessHilbert(context,m_pProcessRaw);
	m_pProcessSpectrogram = NULL;
	m_pProcessSnapshot = NULL;
	m_pProcessFormants = new CProcessFormants(context);
	m_pProcessFormantTracker = new CProcessFormantTracker(context ,*m_pProcessRaw, *m_pProcessHilbert, *m_pProcessGrappl);
	m_pProcessZCross = new CProcessZCross(context);
	m_pProcessSpectrum = new CProcessSpectrum(context);
	m_pProcessDurations = new CProcessDurations(context);
	m_pProcessGlottis = new CProcessGlottis(context);
	m_pProcessPOA = new CProcessPOA(context);
	m_pProcessSDP[0] = NULL;
	m_pProcessSDP[1] = NULL;
	m_pProcessSDP[2] = NULL;
	m_pProcessRatio = new CProcessRatio(context);
	m_pProcessTonalWeightChart = new CProcessTonalWeightChart(context);
	m_pCreatedFonts = new CObArray();
	m_pProcessWavelet = new CProcessWavelet(context);

	//SDM 1.06.6U2
	m_dwDataSize = 0;
	m_nWbProcess = 0;
	m_bBlockBegin = FALSE;
	m_bWaveUndoNow = FALSE;
	m_nCheckPointCount = 0;
	m_szTempConvertedWave.clear();
	m_bUsingTempFile = false;
	m_bAllowEdit = true;

	m_nSelectedChannel = 0;
	m_bMultiChannel = false;
}

CSaDoc::~CSaDoc() {

	for (int nLoop = 0; nLoop < ANNOT_WND_NUMBER; nLoop++) {
		if (m_apSegments[nLoop]) {
			delete m_apSegments[nLoop];
		}
	}
	if (m_pProcessAdjust) {
		delete m_pProcessAdjust;
		m_pProcessAdjust = NULL;
	}
	if (m_pProcessFragments) {
		delete m_pProcessFragments;
		m_pProcessFragments = NULL;
	}
	if (m_pProcessLoudness) {
		delete m_pProcessLoudness;
		m_pProcessLoudness = NULL;
	}
	if (m_pProcessSmoothLoudness) {
		delete m_pProcessSmoothLoudness;
		m_pProcessSmoothLoudness = NULL;
	}
	if (m_pProcessPitch) {
		delete m_pProcessPitch;
		m_pProcessPitch = NULL;
	}
	if (m_pProcessCustomPitch) {
		delete m_pProcessCustomPitch;
		m_pProcessCustomPitch = NULL;
	}
	if (m_pProcessSmoothedPitch) {
		delete m_pProcessSmoothedPitch;
		m_pProcessSmoothedPitch = NULL;
	}
	if (m_pProcessGrappl) {
		delete m_pProcessGrappl;
		m_pProcessGrappl = NULL;
	}
	if (m_pProcessMelogram) {
		delete m_pProcessMelogram;
		m_pProcessMelogram = NULL;
	}
	if (m_pProcessChange) {
		delete m_pProcessChange;
		m_pProcessChange = NULL;
	}
	if (m_pProcessRaw) {
		delete m_pProcessRaw;
		m_pProcessRaw = NULL;
	}
	if (m_pProcessHilbert) {
		delete m_pProcessHilbert;
		m_pProcessHilbert = NULL;
	}
	if (m_pProcessSpectrogram) {
		delete m_pProcessSpectrogram;
		m_pProcessSpectrogram = NULL;
	}
	if (m_pProcessSnapshot) {
		delete m_pProcessSnapshot;
		m_pProcessSnapshot = NULL;
	}
	if (m_pProcessFormants) {
		delete m_pProcessFormants;
		m_pProcessFormants = NULL;
	}
	if (m_pProcessWavelet) {
		delete m_pProcessWavelet;
		m_pProcessWavelet = nullptr;
	}
	if (m_pProcessFormantTracker) {
		delete m_pProcessFormantTracker;
		m_pProcessFormantTracker = NULL;
	}
	if (m_pProcessZCross) {
		delete m_pProcessZCross;
		m_pProcessZCross = NULL;
	}
	if (m_pProcessSpectrum) {
		delete m_pProcessSpectrum;
		m_pProcessSpectrum = NULL;
	}
	if (m_pProcessDurations) {
		delete m_pProcessDurations;
		m_pProcessDurations = NULL;
	}
	if (m_pProcessGlottis) {
		delete m_pProcessGlottis;
		m_pProcessGlottis = NULL;
	}
	if (m_pProcessPOA) {
		delete m_pProcessPOA;
		m_pProcessPOA = NULL;
	}
	if (m_pProcessSDP[0]) {
		delete m_pProcessSDP[0];
		m_pProcessSDP[0] = NULL;
	}
	if (m_pProcessSDP[1]) {
		delete m_pProcessSDP[1];
		m_pProcessSDP[1] = NULL;
	}
	if (m_pProcessSDP[2]) {
		delete m_pProcessSDP[2];
		m_pProcessSDP[2] = NULL;
	}
	if (m_pProcessRatio) {
		delete m_pProcessRatio;
		m_pProcessRatio = NULL;
	}
	if (m_pProcessTonalWeightChart) {
		delete m_pProcessTonalWeightChart;
		m_pProcessTonalWeightChart = NULL;
	}
	if (m_pCreatedFonts != NULL) {
		for (int i = 0; i < m_pCreatedFonts->GetSize(); i++) {
			CFontTable * pFont = (CFontTable *)m_pCreatedFonts->GetAt(i);
			if (pFont != NULL) {
				delete pFont;
			}
		}
		delete m_pCreatedFonts;
	}
	try {
		// delete the temporary file
		if (!m_szRawDataWrk.empty()) {
			FileUtils::Remove(m_szRawDataWrk.c_str());
		}
	} catch (CFileException * e) {
		// error removing file
		ErrorMessage(IDS_ERROR_DELTEMPFILE, m_szRawDataWrk.c_str(), NULL);
		e->Delete();
	}

	// delete converted wave temp file
	FileUtils::Remove(m_szTempConvertedWave.c_str());
}

/***************************************************************************/
// CSaDoc::GetFmtParm Get format parameters
// if processed is false, return the parameters for the document
// if processed is true, return the parameters for a single channel
/***************************************************************************/
void CSaDoc::GetFmtParm(CFmtParm & format, bool processed) {
	format = m_FmtParm;
	if (processed) {
		if (format.wChannels == 1) {
			return;
		}
		// resize format for a single channel
		format.wBlockAlign /= format.wChannels;
		format.dwAvgBytesPerSec /= format.wChannels;
		format.wChannels = 1;
	}
}

/***************************************************************************/
// CSaDoc::GetGender  Retrieve gender info
/***************************************************************************/
EGender CSaDoc::GetGender() {
	SourceParm * pSourceParm = GetSourceParm();
	int nGender = pSourceParm->nGender;
	if (nGender == UNDEFINED_DATA) {
		// guess
		m_pProcessGrappl->Process(this); // process pitch
		if (m_pProcessGrappl->IsDataReady()) {
			double dAvgPitch = m_pProcessGrappl->GetAveragePitch();
			if (dAvgPitch < 200.0) {
				nGender = male;    // adult male
			} else if (dAvgPitch < 350.0) {
				nGender = female;    // adult female
			} else {
				nGender = child;    // child
			}
		}
	}
	return (EGender)nGender;
}

/***************************************************************************/
// CSaDoc::GetUttParm Get utterance parameters
/***************************************************************************/
void CSaDoc::GetUttParm(CUttParm * pUttParm, BOOL bOriginal) {
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
void CSaDoc::SetFmtParm(CFmtParm * pFmtParm, BOOL bAdjustSpectro) {

	m_FmtParm.wTag = pFmtParm->wTag;
	m_FmtParm.wChannels = pFmtParm->wChannels;
	m_FmtParm.dwSamplesPerSec = pFmtParm->dwSamplesPerSec;
	m_FmtParm.dwAvgBytesPerSec = pFmtParm->dwAvgBytesPerSec;
	m_FmtParm.wBlockAlign = pFmtParm->wBlockAlign;
	m_FmtParm.wBitsPerSample = pFmtParm->wBitsPerSample;

	if (bAdjustSpectro) {
		// adjust the spectrum parameters
		CSpectrumParm * pSpectrum = m_pProcessSpectrum->GetSpectrumParms();
		pSpectrum->nFreqUpperBound = (int)(m_FmtParm.dwSamplesPerSec / 2);
	}
}

/***************************************************************************/
// CSaDoc::SetUttParm Set utterance parameters
/***************************************************************************/
void CSaDoc::SetUttParm(const CUttParm * pUttParm, BOOL bOriginal) {
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
void CSaDoc::DeleteContents() {

	CMainFrame * pMain = (CMainFrame *)AfxGetMainWnd();
	// reset and clear all data members
	// plain raw data
	m_nWbProcess = 0;
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

	m_saParam.szDescription.clear();
	m_saParam.wFlags = SA_FLAG_HIGHPASS;
	m_saParam.byRecordFileFormat = FILE_FORMAT_WAV;
	m_saParam.RecordTimeStamp = 0;
	m_saParam.dwRecordBandWidth = 0;
	m_saParam.byRecordSmpSize = 0;
	m_saParam.dwNumberOfSamples = 0;
	m_saParam.lSignalMax = 0;
	m_saParam.lSignalMin = 0;
	m_saParam.dwSignalBandWidth = 0;
	m_saParam.byQuantization = 0;

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
	const CPitchParm * pPitchParm = pMain->GetPitchParmDefaults();
	m_pitchParm.nRangeMode = pPitchParm->nRangeMode;
	m_pitchParm.nScaleMode = pPitchParm->nScaleMode;
	m_pitchParm.nUpperBound = pPitchParm->nUpperBound;
	m_pitchParm.nLowerBound = pPitchParm->nLowerBound;
	m_pitchParm.bUseCepMedianFilter = pPitchParm->bUseCepMedianFilter;
	m_pitchParm.nCepMedianFilterSize = pPitchParm->nCepMedianFilterSize;

	// RLJ, 09/27/2000: Bug GPI-01
	m_pitchParm.nManualPitchUpper = pPitchParm->nManualPitchUpper;
	m_pitchParm.nManualPitchLower = pPitchParm->nManualPitchLower;

	// Set music parameters to default values.
	const CMusicParm * pMusicParm = pMain->GetMusicParmDefaults();
	m_musicParm.nRangeMode = pMusicParm->nRangeMode;
	m_musicParm.nUpperBound = pMusicParm->nUpperBound;
	m_musicParm.nLowerBound = pMusicParm->nLowerBound;

	m_musicParm.nManualUpper = pMusicParm->nManualUpper;
	m_musicParm.nManualLower = pMusicParm->nManualLower;

	m_musicParm.nCalcRangeMode = pMusicParm->nCalcRangeMode;
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

	for (int nLoop = 0; nLoop < ANNOT_WND_NUMBER; nLoop++) {
		if (m_apSegments[nLoop]) {
			m_apSegments[nLoop]->DeleteContents();
		}
	}

	// Set spectrum parameters to default values. // RLJ 1.5Test11.1A
	CSpectrumParm * pSpectrumParmDefaults = pMain->GetSpectrumParmDefaults();
	CSpectrumParm spectrum;
	spectrum.nScaleMode = pSpectrumParmDefaults->nScaleMode;
	spectrum.nPwrUpperBound = pSpectrumParmDefaults->nPwrUpperBound;
	spectrum.nPwrLowerBound = pSpectrumParmDefaults->nPwrLowerBound;
	spectrum.nFreqUpperBound = pSpectrumParmDefaults->nFreqUpperBound;
	spectrum.nFreqLowerBound = pSpectrumParmDefaults->nFreqLowerBound;
	spectrum.nFreqScaleRange = pSpectrumParmDefaults->nFreqScaleRange;
	spectrum.nSmoothLevel = pSpectrumParmDefaults->nSmoothLevel;
	spectrum.nPeakSharpFac = pSpectrumParmDefaults->nPeakSharpFac;
	spectrum.window = pSpectrumParmDefaults->window;
	spectrum.bShowLpcSpectrum = pSpectrumParmDefaults->bShowLpcSpectrum;
	spectrum.bShowCepSpectrum = pSpectrumParmDefaults->bShowCepSpectrum;
	spectrum.bShowFormantFreq = pSpectrumParmDefaults->bShowFormantFreq;
	spectrum.bShowFormantBandwidth = pSpectrumParmDefaults->bShowFormantBandwidth;
	spectrum.bShowFormantPower = pSpectrumParmDefaults->bShowFormantPower;

	m_pProcessSpectrum->SetSpectrumParms(&spectrum);

	// Set formant chart parameters to default values. // AKE 1.5Test13.1
	CFormantParm * pFormantParmDefaults = pMain->GetFormantParmDefaults();
	CFormantParm formant;
	formant.bFromLpcSpectrum = pFormantParmDefaults->bFromLpcSpectrum;
	formant.bFromCepstralSpectrum = pFormantParmDefaults->bFromCepstralSpectrum;
	formant.bTrackFormants = pFormantParmDefaults->bTrackFormants;
	formant.bSmoothFormants = pFormantParmDefaults->bSmoothFormants;
	formant.bMelScale = pFormantParmDefaults->bMelScale;
	m_pProcessFormants->SetFormantParms(&formant);

	// delete temporary file
	if (IsUsingTempFile()) {
		// SDM 1.06.6U2
		try {
			FileUtils::Remove(m_szTempWave);
			m_szTempWave.Empty();
		} catch (CFileException * e) {
			m_szTempWave.Empty();
			e->Delete();
		}
	}
	CreateFonts();
}

/***************************************************************************/
// CSaDoc::CreateFonts
// Create the fonts for this document
/***************************************************************************/
void CSaDoc::CreateFonts() {

	CMainFrame * pMain = (CMainFrame *)AfxGetMainWnd();

	// init the graph fonts with default if requested
	try {
		for (int i = 0; i < pMain->m_GraphFontFaces.GetSize(); i++) {
			CString face = pMain->m_GraphFontFaces.GetAt(i);
			UINT size = pMain->m_GraphFontSizes.GetAt(i);
			pMain->SetFontFace(i, face);
			pMain->SetFontSize(i, size);

			if (m_pCreatedFonts->GetSize() > i && GetFont(i)) {
				delete GetFont(i);
			}
			m_pCreatedFonts->SetAtGrow(i, GetSegment(i)->NewFontTable());
			CAnnotationWnd::CreateAnnotationFont(GetFont(i), pMain->GetFontSize(i), pMain->GetFontFace(i));
		}
	} catch (CMemoryException * e) {
		// handle memory fail exception
		ErrorMessage(IDS_ERROR_MEMALLOC);
		e->Delete();
		return;
	}
}


/***************************************************************************/
// CSaDoc::OnCloseDocument document closing
/***************************************************************************/
void CSaDoc::OnCloseDocument() {

	CMainFrame * pFrame = (CMainFrame *)AfxGetMainWnd();
	ASSERT(pFrame);
	ASSERT(pFrame->IsKindOf(RUNTIME_CLASS(CMainFrame)));

	if (IsUsingTempFile()) {
		m_AutoSave.Close(m_szTempWave);
	} else {
		m_AutoSave.Close(GetPathName());
	}

	// NOTE - OnCloseDocument calls delete this,
	// don't access the document after this point.
	CDocument::OnCloseDocument();

	if (pFrame) {
		pFrame->AppMessage(HINT_APP_DOC_LIST_CHANGED, NULL);
	}
}

/***************************************************************************/
// CSaDoc::OnNewDocument Creating a new document
// This function allocates the pcm read buffer for the new document.
/***************************************************************************/
BOOL CSaDoc::OnNewDocument() {
	if (!CDocument::OnNewDocument()) {
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
BOOL CSaDoc::OnOpenDocument(LPCTSTR pszPathName) {

	wstring wave_file_name = pszPathName;

	if (FileUtils::EndsWith(pszPathName, L".eaf")) {
		Elan::CAnnotationDocument document;
		{
			CScopedCursor waitCursor(*this);
			document = Elan::LoadDocument(pszPathName);
		}
		int found = 0;
		for (size_t i = 0; i < document.header.mediaDescriptors.size(); i++) {
			wstring filename = document.header.mediaDescriptors[i].mediaURL.c_str();

			/*
			wchar_t buffer[MAX_PATH];
			wmemset(buffer,0,_countof(buffer));
			DWORD length = MAX_PATH;
			HRESULT result = PathCreateFromUrl( filename.c_str(), buffer, &length, NULL);
			if (result!=S_OK) {
				continue;
			}
			Use the Uri.LocalPath property
			filename = buffer;
			*/

			if (FileUtils::EndsWith(filename.c_str(), L".wav")) {
				if (FileUtils::FileExists(filename.c_str())) {
					wave_file_name = filename;
					found++;
				} else {
					filename = document.header.mediaDescriptors[i].relativeMediaURL.c_str();
					if (FileUtils::FileExists(filename.c_str())) {
						wave_file_name = filename;
						found++;
					}
				}
			} else {
				wstring extracted = document.header.mediaDescriptors[i].extractedFrom.c_str();
				if (FileUtils::EndsWith(extracted.c_str(), L".wav")) {
					if (FileUtils::FileExists(extracted.c_str())) {
						wave_file_name = extracted;
						found++;
					} else {
						filename = document.header.mediaDescriptors[i].relativeMediaURL.c_str();
						if (FileUtils::FileExists(filename.c_str())) {
							wave_file_name = filename;
							found++;
						}
					}
				}
			}
		}
		if (found > 1) {
			ErrorMessage(IDS_ERROR_ELAN_DUPE);
			return FALSE;
		}
		if (found == 0) {
			ErrorMessage(IDS_ERROR_ELAN_NO_FILE);
			return false;
		}

		wave_file_name = FileUtils::NormalizePath(wave_file_name.c_str());

		CDlgImportElanSheet dlg(NULL, document);
		dlg.SetWizardMode();
		int result = dlg.DoModal();
		if (result == ID_WIZFINISH) {
			map<EAnnotation, wstring> assignments = dlg.getAssignments();
			// create a SAXML file
			wstring filename = wave_file_name.substr(0, wave_file_name.length() - wcslen(L".wav"));
			filename.append(L".saxml");
			try {
				CSAXMLUtils::WriteSAXML(filename.c_str(), document, assignments);
			} catch (int error) {
				ErrorMessage(error);
				return false;
			}

		} else if (result == IDCANCEL) {
			return FALSE;
		}
	} else if (FileUtils::EndsWith(pszPathName, L".saxml")) {
		wave_file_name = wave_file_name.substr(0, wave_file_name.length() - wcslen(L".saxml"));
		wave_file_name.append(L".wav");
	}

	// at this point we have a wave filename.

	// first check the file size
	DWORD dwSize = 0;
	HANDLE hFile = CreateFile(wave_file_name.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile != NULL) {
		dwSize = GetFileSize(hFile, NULL);
		CloseHandle(hFile);
	}

	if ((dwSize == 0) || (dwSize == INVALID_FILE_SIZE)) {
		ErrorMessage(IDS_ERROR_FILEOPEN, wave_file_name.c_str());
		return FALSE;
	}

	m_bAllowEdit = true;

	// assuming wave or audio file
	// if the return value is false, the file is not in an acceptable format
	if (!IsStandardWaveFormat(wave_file_name.c_str(), true)) {
		// convert to wave will select or merge the channels.
		// the output will be a single channel file
		m_bUsingTempFile = true;
		if (!ConvertToWave(wave_file_name.c_str())) {
			// couldn't convert to wave
			return FALSE;
		}
		if (!CheckWaveFormatForOpen(m_szTempConvertedWave.c_str())) {
			return FALSE; // can't open the wave file
		}
	} else {
		if (!CheckWaveFormatForOpen(wave_file_name.c_str())) {
			return FALSE; // can't open the wave file
		}
	}

	if (!CDocument::OnOpenDocument(wave_file_name.c_str())) {
		return FALSE;
	}

	return LoadDataFiles(wave_file_name.c_str());
}

/*
* helper class for splitting channels
*/
struct SChannel {
public:
	BPTR pData;
	CFile * pFile;
};

//SDM 1.06.6U2 Split function from OnOpenDocument
/***************************************************************************/
// CSaDoc::LoadDataFiles Load WAV file into document
// Reads all available information from the RIFF header of a wave file and
// stores it into data members. From the wave data itself only the first
// block of data is read and stored in the data buffer.
/***************************************************************************/
BOOL CSaDoc::LoadDataFiles(LPCTSTR pszPathName, bool bTemp/*=FALSE*/) {

	CMainFrame * pMainWnd = (CMainFrame *)AfxGetMainWnd();

	CScopedCursor cursor(*this);

	wstring szWavePath = pszPathName;
	if (m_bUsingTempFile) {
		szWavePath = m_szTempConvertedWave;
	}

	// this will reload the m_FmtParm structure
	if (!ReadRiff(szWavePath.c_str())) {
		return FALSE;
	}

	LoadTranscriptionData(pszPathName, bTemp);

	//SDM 1.06.6U2
	//handle temporary file load.
	if (bTemp) {
		ApplyWaveFile(pszPathName, m_dwDataSize);
	} else {
		// create the temporary wave chunk copy
		CopyWaveToTemp(szWavePath.c_str());
	}

	// This flag will be the only signature of a "Stereo file"
	m_bMultiChannel = (m_FmtParm.wChannels > 1);
	if (m_bMultiChannel) {
		// stereo SA is expecting a mono file (so we will make this a mono file)
		m_nSelectedChannel = 0;
		if (pMainWnd->GetShowAdvancedAudio()) {
			CDlgMultiChannel dlg(m_FmtParm.wChannels);
			if (dlg.DoModal() != IDOK) {
				return FALSE;
			}
			m_nSelectedChannel = dlg.m_nChannel;
		}
	}

	s_bDocumentWasAlreadyOpen = FALSE;

	((CGlossSegment *)m_apSegments[GLOSS])->CorrectGlossDurations(this); // SDM 1.5Test11.3

	// if player is visible, disable the speed slider until required processing is completed
	CMainFrame * pMain = (CMainFrame *)AfxGetMainWnd();
	CDlgPlayer * pPlayer = pMain->GetPlayer();
	if ((pPlayer != NULL) && (pPlayer->IsWindowVisible())) {
		pPlayer->EnableSpeedSlider(FALSE);
	}

	return TRUE;
}

/***************************************************************************/
// CSaDoc::LoadTranscriptionData
// Reads all available information from the RIFF header of a wave file and
// stores it into data members. From the wave data itself only the first
// block of data is read and stored in the data buffer.
/***************************************************************************/
BOOL CSaDoc::LoadTranscriptionData(LPCTSTR pszWavePath, BOOL bTemp) {

	CoInitialize(NULL);
	ISaAudioDocumentReaderPtr saAudioDocRdr;
	HRESULT createResult = saAudioDocRdr.CreateInstance(__uuidof(SaAudioDocumentReader));
	if (createResult) {
		CSaString szCreateResult;
		szCreateResult.Format(_T("%x"), createResult);
		ErrorMessage(IDS_ERROR_CREATE_INSTANCE, _T("SaAudioDocumentReader.CreateInstance()"), szCreateResult);
		return FALSE;
	}

	short initSucceeded = TRUE;
	try {
		initSucceeded = saAudioDocRdr->Initialize(pszWavePath, (short)bTemp);
	} catch (...) {
		CSaString szError;
		szError.Format(_T("Unexpected exception thrown from 'SaAudioDocumentReader::Initialize()'"));
		ErrorMessage(szError);
		initSucceeded = FALSE;
	}

	// get sampled data size in seconds
	// we need to filter the incoming data for segments that fall outside of the
	// length of the audio file.  They will be dropped, but we need to tell the user
	int exceeded = 0;
	int limited = 0;
	DWORD limit = GetDataSize();

	// re-initialize the utterance parameters
	m_uttParm.Init(m_FmtParm.wBitsPerSample);

	if ((initSucceeded) && (saAudioDocRdr->DocumentExistsInDB)) {
		ReadNonSegmentData(saAudioDocRdr);
		ReadTranscription(PHONETIC, saAudioDocRdr, limit, exceeded, limited);
		ReadTranscription(PHONEMIC, saAudioDocRdr, limit, exceeded, limited);
		ReadTranscription(TONE, saAudioDocRdr, limit, exceeded, limited);
		ReadTranscription(ORTHO, saAudioDocRdr, limit, exceeded, limited);
		ReadGlossPosAndRefSegments(saAudioDocRdr, limit, exceeded, limited);
		ReadScoreData(saAudioDocRdr);
		ReadTranscription(MUSIC_PL1, saAudioDocRdr, limit, exceeded, limited);
		ReadTranscription(MUSIC_PL2, saAudioDocRdr, limit, exceeded, limited);
		ReadTranscription(MUSIC_PL3, saAudioDocRdr, limit, exceeded, limited);
		ReadTranscription(MUSIC_PL4, saAudioDocRdr, limit, exceeded, limited);
	} else {
		m_saParam.szDescription.clear();
		m_saParam.wFlags = 0;
		m_saParam.byRecordFileFormat = FILE_FORMAT_WAV;
		m_saParam.lSignalMax = 0;
		m_saParam.lSignalMin = 0;

		if ((m_saParam.dwRecordBandWidth == 0) &&
			(m_saParam.byRecordSmpSize == 0) &&
			(m_saParam.dwSignalBandWidth == 0) &&
			(m_saParam.byQuantization == 0)) {
			// These parameters used to be initialized improperly to 0 -- Change them
			m_saParam.RecordTimeStamp = m_fileStat.m_ctime;                     // Creation time of file
			m_saParam.byRecordSmpSize = (BYTE)m_FmtParm.wBitsPerSample;
			m_saParam.dwNumberOfSamples = GetDataSize() / m_FmtParm.wBlockAlign;
			m_saParam.dwRecordBandWidth = m_FmtParm.dwSamplesPerSec / 2;        // Assume raw untouched file
			m_saParam.dwSignalBandWidth = m_FmtParm.dwSamplesPerSec / 2;
			m_saParam.byQuantization = (BYTE)m_FmtParm.wBitsPerSample;
		}
	}

	if (limited > 0) {
		CString param;
		param.Format(L"%d", limited);
		CString msg;
		AfxFormatString1(msg, IDS_LENGTH_LIMITED, param);
		AfxMessageBox(msg, MB_OK | MB_ICONWARNING);
	}
	if (exceeded > 0) {
		CString param;
		param.Format(L"%d", exceeded);
		CString msg;
		AfxFormatString1(msg, IDS_LENGTH_EXCEEDED, param);
		AfxMessageBox(msg, MB_OK | MB_ICONEXCLAMATION);
	}

	saAudioDocRdr->Close();
	saAudioDocRdr->Release();
	saAudioDocRdr = NULL;
	CoUninitialize();

	NormalizePhoneticDependencies();

	return TRUE;
}

/**
* CSaDoc::ReadRiff  Read fmt and (wave) data chunks from wave file
* @param pszPathName The name of the wave file we are loading.
* @return false if we are unable to read the .wave file.
*/
bool CSaDoc::ReadRiff(LPCTSTR pszPathName) {

	// get file information
	CFile::GetStatus(pszPathName, m_fileStat);

	// open file
	HMMIO hmmioFile; // file handle
	hmmioFile = mmioOpen(const_cast<TCHAR *>(pszPathName), NULL, MMIO_READ | MMIO_DENYWRITE);
	if (!hmmioFile) {
		// error opening file
		ErrorMessage(IDS_ERROR_FILEOPEN, pszPathName);
		return false;
	}

	// locate a 'RIFF' chunk with a 'WAVE' form type to make sure it's a WAVE file.
	MMCKINFO mmckinfoParent = {};
	mmckinfoParent.fccType = mmioFOURCC('W', 'A', 'V', 'E'); // prepare search code
	if (mmioDescend(hmmioFile, (LPMMCKINFO)&mmckinfoParent, NULL, MMIO_FINDRIFF)) {
		// error descending into wave chunk
		ErrorMessage(IDS_ERROR_WAVECHUNK, pszPathName);
		mmioClose(hmmioFile, 0);
		return false;
	}

	// find the format chunk. It should be a subchunk of the 'RIFF' parent chunk
	MMCKINFO mmckinfoSubchunk = {};
	mmckinfoSubchunk.ckid = mmioFOURCC('f', 'm', 't', ' '); // prepare search code
	MMRESULT result = mmioDescend(hmmioFile, &mmckinfoSubchunk, &mmckinfoParent, MMIO_FINDCHUNK);
	if (result != MMSYSERR_NOERROR) {
		// error finding format chunk
		ErrorMessage(IDS_ERROR_FORMATCHUNK, pszPathName);
		mmioClose(hmmioFile, 0);
		return false;
	}

	// fmt chunk found
	LONG lError = mmioRead(hmmioFile, (BPTR)&m_FmtParm.wTag, sizeof(WORD)); // read format tag
	if (m_FmtParm.wTag != WAVE_FORMAT_PCM) { // check if PCM format
		// error testing pcm format
		ErrorMessage(IDS_ERROR_FORMATPCM, pszPathName);
		mmioClose(hmmioFile, 0);
		return false;
	}
	if (lError != -1) {
		// read channel number
		lError = mmioRead(hmmioFile, (BPTR)&m_FmtParm.wChannels, sizeof(WORD));
	}
	if (lError != -1) {
		// read sampling rate
		lError = mmioRead(hmmioFile, (BPTR)&m_FmtParm.dwSamplesPerSec, sizeof(DWORD));
	}
	if (lError != -1) {
		// read throughput
		lError = mmioRead(hmmioFile, (BPTR)&m_FmtParm.dwAvgBytesPerSec, sizeof(DWORD));
	}
	if (lError != -1) {
		// read sampling rate for all channels
		lError = mmioRead(hmmioFile, (BPTR)&m_FmtParm.wBlockAlign, sizeof(WORD));
	}
	if (lError != -1) {
		// read sample word size
		lError = mmioRead(hmmioFile, (BPTR)&m_FmtParm.wBitsPerSample, sizeof(WORD));
	}

	// get out of 'fmt ' chunk
	mmioAscend(hmmioFile, &mmckinfoSubchunk, 0);
	if (lError == -1) {
		// error reading format chunk
		ErrorMessage(IDS_ERROR_READFORMAT, pszPathName);
		mmioClose(hmmioFile, 0);
		return false;
	}

	// adjust the spectrum parameters
	CSpectrumParm * pSpectrum = m_pProcessSpectrum->GetSpectrumParms();
	pSpectrum->nFreqUpperBound = (int)(m_FmtParm.dwSamplesPerSec / 2);

	// determine how much sound data is in the file. Find the data subchunk
	mmckinfoSubchunk.ckid = mmioFOURCC('d', 'a', 't', 'a');
	result = mmioDescend(hmmioFile, &mmckinfoSubchunk, &mmckinfoParent, MMIO_FINDCHUNK);
	if (result != MMSYSERR_NOERROR) {
		// error finding data chunk
		ErrorMessage(IDS_ERROR_DATACHUNK, pszPathName);
		mmioClose(hmmioFile, 0);
		return false;
	}

	// get the size of the data subchunk
	m_dwDataSize = mmckinfoSubchunk.cksize;
	if (m_dwDataSize == 0L) {
		// error 0 size data chunk
		ErrorMessage(IDS_ERROR_NODATA, pszPathName);
		mmioClose(hmmioFile, 0);
		return false;
	}

	mmioClose(hmmioFile, 0);
	return true;
}

/***************************************************************************/
// CSaDoc::ReadNonSegmentData reads from the database the non segment
// oriented data. (i.e. what used to be stored in the wave file's sa, utt
// speaker, language, and reference chunks.
/***************************************************************************/
void CSaDoc::ReadNonSegmentData(ISaAudioDocumentReaderPtr saAudioDocRdr) {

	m_szMD5HashCode = saAudioDocRdr->MD5HashCode;

	// Get SA data
	m_saParam.szDescription = _to_utf8((wchar_t *)saAudioDocRdr->SADescription);
	m_saParam.wFlags = (USHORT)saAudioDocRdr->SAFlags;
	m_saParam.byRecordFileFormat = (BYTE)saAudioDocRdr->RecordFileFormat;
	m_saParam.RecordTimeStamp = (CTime)saAudioDocRdr->RecordTimeStamp;
	m_saParam.dwRecordBandWidth = saAudioDocRdr->RecordBandWidth;
	m_saParam.byRecordSmpSize = (BYTE)saAudioDocRdr->RecordSampleSize;
	m_saParam.lSignalMax = saAudioDocRdr->SignalMax;                    // read maximum signal
	m_saParam.lSignalMin = saAudioDocRdr->SignalMin;                    // read minimum signal
	m_saParam.dwSignalBandWidth = saAudioDocRdr->SignalBandWidth;       // read signal bandwidth
	m_saParam.byQuantization = (BYTE)saAudioDocRdr->SignalEffSampSize;  // read signal effective sample size
	m_saParam.dwNumberOfSamples = saAudioDocRdr->NumberOfSamples;       // number of samples

	// Get UTT data
	m_uttParm.nMinFreq = (USHORT)saAudioDocRdr->CalcFreqLow;
	m_uttParm.nMaxFreq = (USHORT)saAudioDocRdr->CalcFreqHigh;
	m_uttParm.nCritLoud = (USHORT)saAudioDocRdr->CalcVoicingThd;
	m_uttParm.nMaxChange = (USHORT)saAudioDocRdr->CalcPercntChng;
	m_uttParm.nMinGroup = (USHORT)saAudioDocRdr->CalcGrpSize;
	m_uttParm.nMaxInterp = (USHORT)saAudioDocRdr->CalcIntrpGap;

	// check the value of nCritLoud
	if ((m_FmtParm.wBitsPerSample == 16) &&
		(m_uttParm.nCritLoud > 0) &&
		(m_uttParm.nCritLoud < 256)) {
		m_uttParm.nCritLoud <<= 8;
	}

	SetUttParm(&m_uttParm, TRUE);  // Copy uttParms to original for safe keeping

	// Get Speaker data
	int nGender = UNDEFINED_DATA;
	switch ((wchar_t)saAudioDocRdr->SpeakerGender) {
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
// @param limit the length of the audio data in seconds.
/***************************************************************************/
void CSaDoc::ReadTranscription(int transType, ISaAudioDocumentReaderPtr saAudioDocRdr, DWORD limit, int & exceeded, int & limited) {

	CSegment * pSegment = ((CSaDoc *)this)->GetSegment(transType);

	DWORD offset = 0;
	DWORD length = 0;
	BSTR * annotation = (BSTR *)calloc(1, sizeof(long));
	CSaString szFullTrans = _T("");

	CStringArray texts;
	CDWordArray dwOffsets;
	CDWordArray dwDurations;

	while (saAudioDocRdr->ReadSegment((long)transType, &offset, &length, annotation)) {

		offset = offset / (long)m_FmtParm.wChannels;
		length = length / (long)m_FmtParm.wChannels;
		if (offset > limit) {
			TRACE("dropping segment type:%d offset:%d duration:%d sum:%d limit:%d\n", transType, offset, length, (offset + length), limit);
			exceeded++;
			continue;
		}

		if ((offset + length) > limit) {
			TRACE("limited segment type:%d offset:%d duration:%d sum:%d limit:%d\n", transType, offset, length, (offset + length), limit);
			limited++;
			length = limit - offset;
		}

		// Loop through the code points in the annotation and save
		texts.Add(*annotation);
		dwOffsets.Add(offset);
		dwDurations.Add(length);
	}

	ASSERT(dwOffsets.GetSize() == dwDurations.GetSize());
	for (int i = 0; i < dwOffsets.GetSize(); i++) {
		pSegment->InsertAt(i, (LPCTSTR)texts[i], dwOffsets[i], dwDurations[i]);
	}

	free(annotation);
}

/***************************************************************************/
// CSaDoc::ReadGlossPosAndRefSegments reads the gloss, part of speech and
// reference information from the database.
// @param limit the length of the audio data in seconds.
/***************************************************************************/
void CSaDoc::ReadGlossPosAndRefSegments(ISaAudioDocumentReaderPtr saAudioDocRdr, DWORD limit, int & exceeded, int & limited) {

	CGlossSegment * pGloss = (CGlossSegment *)m_apSegments[GLOSS];
	CGlossNatSegment * pGlossNat = (CGlossNatSegment *)m_apSegments[GLOSS_NAT];
	CReferenceSegment * pReference = (CReferenceSegment *)m_apSegments[REFERENCE];

	DWORD offset = 0;
	DWORD length = 0;
	BSTR * gloss = (BSTR *)calloc(1, sizeof(long));
	BSTR * glossNat = (BSTR *)calloc(1, sizeof(long));
	BSTR * ref = (BSTR *)calloc(1, sizeof(long));
	VARIANT_BOOL isBookmark;
	int i = 0;
	int nRef = 0;
	int nGlossNat = 0;

	// length (which is mark duration) determines whether segment exists or not
	// string pointer may be NULL if no data exists - but length>0 indicates empty segment.
	while (saAudioDocRdr->ReadMarkSegment(&offset, &length, gloss, glossNat, ref, &isBookmark)) {

		offset /= m_FmtParm.wChannels;
		length /= m_FmtParm.wChannels;

		if (offset > limit) {
			TRACE("dropping gloss-glossNat-ref segment offset:%d duration:%d sum:%d limit:%d\n", offset, length, (offset + length), limit);
			exceeded++;
			continue;
		}

		if ((offset + length) > limit) {
			TRACE("dropping gloss-glossNat-ref segment offset:%d duration:%d sum:%d limit:%d\n", offset, length, (offset + length), limit);
			limited++;
			length = limit - offset;
		}

		CSaString szGloss = *gloss;
		pGloss->Insert(i++, szGloss, (isBookmark != 0), offset, length);

		CSaString szGlossNat = *glossNat;
		pGlossNat->Insert(nGlossNat++, szGlossNat, false, offset, length);

		CSaString szRef = *ref;
		pReference->Insert(nRef++, szRef, false, offset, length);
	}

	free(gloss);
	free(glossNat);
	free(ref);
}

/***************************************************************************/
// CSaDoc::ReadScoreData Reads the score data from the database and loads
// it into the staff graph.
/***************************************************************************/
void CSaDoc::ReadScoreData(ISaAudioDocumentReaderPtr saAudioDocRdr) {
	//disabling loading of MusicXML information
	//m_szMusicScore = (wchar_t *)saAudioDocRdr->ReadAsMusicXML();
}

/***************************************************************************/
// CSaDoc::InsertTranscriptions  Insert transcriptions from another WAV file
// into current document
/***************************************************************************/
BOOL CSaDoc::InsertTranscriptions(LPCTSTR pszPathName, DWORD dwPos) {

	TRACE(L"inserting transcriptions for %s at %d\n", pszPathName, dwPos);

	CScopedCursor waitCursor(*this);

	// initialize com object
	CoInitialize(NULL);
	ISaAudioDocumentReaderPtr saAudioDocRdr;
	HRESULT createResult = saAudioDocRdr.CreateInstance(__uuidof(SaAudioDocumentReader));
	if (createResult) {
		CSaString szCreateResult;
		szCreateResult.Format(_T("%x"), createResult);
		ErrorMessage(IDS_ERROR_CREATE_INSTANCE, _T("SaAudioDocumentReader.CreateInstance()"), szCreateResult);
		return FALSE;
	}

	if (!saAudioDocRdr->Initialize(pszPathName, VARIANT_TRUE)) {
		// TODO: Display a more relevant error message.
		ErrorMessage(IDS_ERROR_NODATA, pszPathName);
		saAudioDocRdr->Close();
		saAudioDocRdr->Release();
		saAudioDocRdr = NULL;
		CoUninitialize();
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

	return TRUE;
}

/***************************************************************************/
// CSaDoc::InsertTranscription  Insert transcription from another WAV file
// into the current document (except Gloss, POS and Ref)
/***************************************************************************/
BOOL CSaDoc::InsertTranscription(int transType, ISaAudioDocumentReaderPtr saAudioDocRdr, DWORD dwPos) {

	CSegment * pSegment = GetSegment(transType);

	DWORD offset = 0;
	DWORD length = 0;
	BSTR * annotation = (BSTR *)calloc(1, sizeof(long));

	// which segment includes the insertion position?
	int nIndex = 0;
	if ((dwPos > 0) && (!pSegment->IsEmpty())) {
		while ((nIndex != -1) && (pSegment->GetOffset(nIndex) <= dwPos)) {
			nIndex = pSegment->GetNext(nIndex);
		}
		if (nIndex == -1) {
			nIndex = pSegment->GetOffsetSize();
		}
	}

	TRACE("beginning insertion at %d\n", nIndex);
	while (saAudioDocRdr->ReadSegment((long)transType, &offset, &length, annotation)) {
		CSaString szString = *annotation;
		offset /= m_FmtParm.wChannels;
		length /= m_FmtParm.wChannels;
		TRACE("type=%d offset=%d length=%d\n", transType, offset, length);
		pSegment->Insert(nIndex, szString, false, offset + dwPos, length);
		nIndex++;
	}

	free(annotation);

	// apply input filter to transcription data
	if (pSegment->Filter()) {
		SetModifiedFlag(TRUE);
		// transcription data has been modified
		SetTransModifiedFlag(TRUE);
	}

	return TRUE;
}

/***************************************************************************/
// CSaDoc::InsertPosTranscription  Insert gloss, POS and Ref
// transcriptions from another WAV file into current document
/***************************************************************************/
void CSaDoc::InsertGlossPosRefTranscription(ISaAudioDocumentReaderPtr saAudioDocRdr, DWORD dwPos) {

	CGlossSegment * pGloss = (CGlossSegment *)m_apSegments[GLOSS];
	CGlossNatSegment * pGlossNat = (CGlossNatSegment *)m_apSegments[GLOSS_NAT];
	CReferenceSegment * pReference = (CReferenceSegment *)m_apSegments[REFERENCE];

	// which segment includes the insertion position?
	int nIndex = 0;
	if ((dwPos > 0) && !pGloss->IsEmpty()) {
		while ((nIndex != -1) && (pGloss->GetOffset(nIndex) <= dwPos)) {
			nIndex = pGloss->GetNext(nIndex);
		}

		if (nIndex == -1) {
			nIndex = pGloss->GetOffsetSize();
		}
	}

	DWORD offset = 0;
	DWORD length = 0;
	BSTR * gloss = (BSTR *)calloc(1, sizeof(long));
	BSTR * glossNat = (BSTR *)calloc(1, sizeof(long));
	BSTR * ref = (BSTR *)calloc(1, sizeof(long));
	VARIANT_BOOL isBookmark;
	while (saAudioDocRdr->ReadMarkSegment(&offset, &length, gloss, glossNat, ref, &isBookmark)) {
		offset /= m_FmtParm.wChannels;
		length /= m_FmtParm.wChannels;

		CSaString szGloss = *gloss;
		pGloss->Insert(nIndex, szGloss, (isBookmark != 0), offset + dwPos, length);

		CSaString szGlossNat = *glossNat;
		pGlossNat->Insert(nIndex, szGlossNat, false, offset + dwPos, length);

		CSaString szRef = *ref;
		pReference->Insert(nIndex++, szRef, false, offset + dwPos, length);
	}

	free(gloss);
	free(glossNat);
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
	DWORD & dwDataSize) {

	dwDataSize = 0;
	// open file
	HMMIO hmmioFile; // file handle
	hmmioFile = mmioOpen(const_cast<TCHAR *>(pszPathName), NULL, MMIO_READ | MMIO_DENYWRITE);
	if (!hmmioFile) {
		ErrorMessage(IDS_ERROR_FILEOPEN, pszPathName);
		return false;
	}
	// locate a 'RIFF' chunk with a 'WAVE' form type to make sure it's a WAVE file.
	MMCKINFO mmckinfoParent = {};
	mmckinfoParent.fccType = mmioFOURCC('W', 'A', 'V', 'E'); // prepare search code
	if (mmioDescend(hmmioFile, (LPMMCKINFO)&mmckinfoParent, NULL, MMIO_FINDRIFF)) {
		mmioClose(hmmioFile, 0);
		// error descending into wave chunk
		ErrorMessage(IDS_ERROR_WAVECHUNK, pszPathName);
		return false;
	}
	// find the format chunk. It should be a subchunk of the 'RIFF' parent chunk
	MMCKINFO mmckinfoSubchunk = {};
	mmckinfoSubchunk.ckid = mmioFOURCC('f', 'm', 't', ' '); // prepare search code
	LONG lError;
	if (!mmioDescend(hmmioFile, &mmckinfoSubchunk, &mmckinfoParent, MMIO_FINDCHUNK)) {
		// fmt chunk found
		lError = mmioRead(hmmioFile, (BPTR)&fmtParm.wTag, sizeof(WORD)); // read format tag
		if (fmtParm.wTag != WAVE_FORMAT_PCM) { // check if PCM format
			mmioClose(hmmioFile, 0);
			// error testing pcm format
			ErrorMessage(IDS_ERROR_FORMATPCM, pszPathName);
			return false;
		}
		if (lError != -1) {
			// read channel number
			lError = mmioRead(hmmioFile, (BPTR)&fmtParm.wChannels, sizeof(WORD));
		}
		if (lError != -1) {
			// read sampling rate
			lError = mmioRead(hmmioFile, (BPTR)&fmtParm.dwSamplesPerSec, sizeof(DWORD));
		}
		if (lError != -1) {
			// read throughput
			lError = mmioRead(hmmioFile, (BPTR)&fmtParm.dwAvgBytesPerSec, sizeof(DWORD));
		}
		if (lError != -1) {
			// read sampling rate for all channels
			lError = mmioRead(hmmioFile, (BPTR)&fmtParm.wBlockAlign, sizeof(WORD));
		}
		if (lError != -1) {
			// read sample word size
			lError = mmioRead(hmmioFile, (BPTR)&fmtParm.wBitsPerSample, sizeof(WORD));
		}

		// get out of 'fmt ' chunk
		mmioAscend(hmmioFile, &mmckinfoSubchunk, 0);
		if (lError == -1) {
			// error reading format chunk
			ErrorMessage(IDS_ERROR_READFORMAT, pszPathName);
			mmioClose(hmmioFile, 0);
			return false;
		}
	} else {
		// error finding format chunk
		ErrorMessage(IDS_ERROR_FORMATCHUNK, pszPathName);
		mmioClose(hmmioFile, 0);
		return false;
	}
	// determine how much sound data is in the file. Find the data subchunk
	mmckinfoSubchunk.ckid = mmioFOURCC('d', 'a', 't', 'a');
	if (mmioDescend(hmmioFile, &mmckinfoSubchunk, &mmckinfoParent, MMIO_FINDCHUNK)) {
		// error finding data chunk
		ErrorMessage(IDS_ERROR_DATACHUNK, pszPathName);
		mmioClose(hmmioFile, 0);
		return false;
	}
	// get the size of the data subchunk
	dwDataSize = mmckinfoSubchunk.cksize;
	if (dwDataSize == 0L) {
		// error 0 size data chunk
		ErrorMessage(IDS_ERROR_NODATA, pszPathName);
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
bool CSaDoc::IsMultiChannelWave(LPCTSTR pszPathName, int & channels) {
	channels = 0;

	// open file
	HMMIO hmmioFile = mmioOpen(const_cast<TCHAR *>(pszPathName), NULL, MMIO_READ | MMIO_DENYWRITE);
	if (!hmmioFile) {
		return false;
	}

	// locate a 'RIFF' chunk with a 'WAVE' form type to make sure it's a WAVE file.
	MMCKINFO mmckinfoParent = {};
	mmckinfoParent.fccType = mmioFOURCC('W', 'A', 'V', 'E'); // prepare search code
	if (mmioDescend(hmmioFile, (LPMMCKINFO)&mmckinfoParent, NULL, MMIO_FINDRIFF)) {
		mmioClose(hmmioFile, 0);
		return false;
	}

	CFmtParm fmtParm;
	// find the format chunk. It should be a subchunk of the 'RIFF' parent chunk
	MMCKINFO mmckinfoSubchunk = {};
	mmckinfoSubchunk.ckid = mmioFOURCC('f', 'm', 't', ' '); // prepare search code
	LONG lError = mmioDescend(hmmioFile, &mmckinfoSubchunk, &mmckinfoParent, MMIO_FINDCHUNK);
	if (lError != MMSYSERR_NOERROR) {
		mmioClose(hmmioFile, 0);
		return false;
	}

	// fmt chunk found
	lError = mmioRead(hmmioFile, (BPTR)&fmtParm.wTag, sizeof(WORD)); // read format tag
	if (lError == -1) {
		mmioClose(hmmioFile, 0);
		return false;
	}

	if (fmtParm.wTag != WAVE_FORMAT_PCM) { // check if PCM format
		mmioClose(hmmioFile, 0);
		return false;
	}

	lError = mmioRead(hmmioFile, (BPTR)&fmtParm.wChannels, sizeof(WORD)); // read channel number
	if (lError == -1) {
		mmioClose(hmmioFile, 0);
		return false;
	}

	// if it's multi channel, we will convert the file
	channels = fmtParm.wChannels;
	if (fmtParm.wChannels > 1) {
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
DWORD CSaDoc::CheckWaveFormatForPaste(LPCTSTR pszPathName) {
	TRACE(L"CheckWaveFormatForPaste %s\n", pszPathName);
	CFmtParm fmtParm;
	DWORD dwDataSize;

	if (!GetWaveFormatParams(pszPathName, fmtParm, dwDataSize)) {
		// there is an error in the file.
		// an error message will have already been displayed
		return 0;
	}

	TRACE("WAVE FORMAT --------\n");
	TRACE("wFormatTag=%d\n", fmtParm.wTag);
	TRACE("channels=%d\n", fmtParm.wChannels);
	TRACE("samples per sec=%d\n", fmtParm.dwSamplesPerSec);
	TRACE("bytes per sec=%d\n", fmtParm.dwAvgBytesPerSec);
	TRACE("block align=%d\n", fmtParm.wBlockAlign);
	TRACE("bits per sample=%d\n", fmtParm.wBitsPerSample);
	TRACE("--------------------\n");

	if (dwDataSize) {
		CFmtParm * pFmtParm = &fmtParm;
		if ((m_FmtParm.wChannels != pFmtParm->wChannels) ||
			(m_FmtParm.dwSamplesPerSec != pFmtParm->dwSamplesPerSec) ||
			(m_FmtParm.dwAvgBytesPerSec != pFmtParm->dwAvgBytesPerSec) ||
			(m_FmtParm.wBlockAlign != pFmtParm->wBlockAlign) ||
			(m_FmtParm.wBitsPerSample != pFmtParm->wBitsPerSample)) {
			// not the right format
			ErrorMessage(IDS_ERROR_PASTEFORMAT);
			return FALSE;
		}
	}

	return dwDataSize; // OK return data chunk length
}

/***************************************************************************/
// CSaDoc::CheckWaveFormatForOpen Checks format of a WAV file for opening
/***************************************************************************/
DWORD CSaDoc::CheckWaveFormatForOpen(LPCTSTR pszPathName) {

	CFmtParm fmtParm;
	DWORD dwDataSize = 0;
	if (!GetWaveFormatParams(pszPathName, fmtParm, dwDataSize)) {
		// there is an error in the file.
		// an error message will have already been displayed
		return 0;
	}

	if (dwDataSize) {
		if (fmtParm.dwSamplesPerSec < 1) {
			// not the right format
			ErrorMessage(IDS_ERROR_INVALID_SAMPLING_RATE, pszPathName);
			return 0;
		}
	}

	return dwDataSize; // OK return data chunk length
}

/***************************************************************************/
// CSaDoc::ConvertToWave Converts file to 22kHz, 16bit, Mono WAV format
/***************************************************************************/
bool CSaDoc::ConvertToWave(LPCTSTR pszPathName) {

	CScopedStatusBar scopedStatusBar(IDS_CONVERT_WAVE);

	// create temp file
	m_szTempConvertedWave = FileUtils::GetTempFileName(_T("WAV"));

	// if it's a wave file, but in a different format then try and convert it
	// if this errors, we will just continue on trying with ST_Audio
	{
		CWaveResampler resampler;
		CWaveResampler::ECONVERT result = resampler.Normalize(pszPathName, m_szTempConvertedWave.c_str(), scopedStatusBar);
		if (result == CWaveResampler::EC_SUCCESS) {
			return true;
		} else if (result == CWaveResampler::EC_USERABORT) {
			// the user chose to quite
			return false;
		}
	}

	// convert to wave
	CoInitialize(NULL);
	ISTAudioPtr stAudio;
	HRESULT createResult = stAudio.CreateInstance(__uuidof(STAudio));
	if (createResult) {
		CSaString szCreateResult;
		szCreateResult.Format(_T("%x"), createResult);
		ErrorMessage(IDS_ERROR_CREATE_INSTANCE, _T("STAudio.CreateInstance()"), szCreateResult);
		return false;
	}

	scopedStatusBar.SetProgress(30);

	bool result = false;
	try {
		result = (stAudio->ConvertToWAV(_bstr_t(pszPathName), _bstr_t(m_szTempConvertedWave.c_str()), 22050, 16, 1) == VARIANT_TRUE);
	} catch (...) {
		ErrorMessage(IDS_ERROR_FORMATPCM, pszPathName);
		return false;
	}

	scopedStatusBar.SetProgress(90);

	stAudio->Release();
	stAudio = NULL;
	CoUninitialize();

	return result;
}

// SDM 1.06.6U2 split function
/***************************************************************************/
// CSaDoc::OnSaveDocument Saving a document
// Override for CDocument::OnSaveDocument()
/***************************************************************************/
BOOL CSaDoc::OnSaveDocument(LPCTSTR pszPathName) {
	return SaveDocument(pszPathName, true);
}

/***************************************************************************/
// CSaDoc::OnSaveDocument Saving a document
// Stores all available information from data members in the RIFF header of
// the document (wave file). 
// The fmt and data chunks have to be there already! 
// The temporary wave file data will be copied into the wave chunk.
// If the string m_szTempWave is not empty, the wave data has first to be
// copied (renamed) from its current place given in the string. 
// This temp file from the recorder contains the RIFF structure with the fmt and the
// data chunks. After the copying, the file has to be saved in the normal way.
/***************************************************************************/
BOOL CSaDoc::SaveDocument(LPCTSTR pszPathName, bool bSaveAudio) {

	// get pointer to application
	CSaApp & app = *(CSaApp *)AfxGetApp();
	CScopedCursor waitCursor(*this);

	if (IsUsingTempFile()) {
		// check if the file already opened
		if (app.IsFileOpened(pszPathName)) {
			// error file already opened by SA
			ErrorMessage(IDS_ERROR_FILEOPENED, pszPathName);
			return FALSE;
		}
		// temporary wave file to rename
		CFileStatus status;
		// check if file exists already
		if (CFile::GetStatus(pszPathName, status) != 0) {
			// file does exist already, be sure to allow writing and delete it
			FileUtils::Remove(pszPathName);
		}
		// check if a copy is needed
		if (m_szTempWave[0] != pszPathName[0]) {
			// different drives, copy the file
			if (!CopyWave(m_szTempWave, pszPathName)) {
				// error copying file
				ErrorMessage(IDS_ERROR_FILEWRITE, pszPathName);
				return FALSE;
			}
			// now delete the old file (source)
			FileUtils::Remove(m_szTempWave);
		} else { // rename the file
			try {
				CFile::Rename(m_szTempWave, pszPathName);
			} catch (CFileException * e) {
				// error renaming file
				ErrorMessage(IDS_ERROR_FILEWRITE, pszPathName);
				e->Delete();
				return FALSE;
			}
		}
		// empty the new file name string
		m_szTempWave.Empty();

		// delete wave undo entry
		DeleteWaveFromUndo();
		if (!WriteDataFiles(pszPathName, bSaveAudio, false)) {
			return FALSE;
		}

		ClearFileStatus(pszPathName);
		return TRUE;
	}

	// we are dealing with a normal file.
	// does the file still exist?
	if (FileUtils::FileExists(pszPathName)) {
		// delete wave undo entry
		DeleteWaveFromUndo();
		if (!WriteDataFiles(pszPathName, bSaveAudio, false)) {
			return FALSE;
		}
		ClearFileStatus(pszPathName);
		return TRUE;
	}

	// we are dealing with a normal file that exists
	CSaString oldFile = GetPathName();
	if (oldFile.IsEmpty()) {
		// get the current view caption string
		oldFile = GetFilenameFromTitle().c_str();
	}
	oldFile = FileUtils::ReplaceExtension((LPCTSTR)oldFile, L".wav").c_str();

	DWORD flags = MMIO_READ;
	WORD bitsPerSample = 0;
	WORD formatTag = 0;
	WORD channels = 0;
	DWORD samplesPerSec = 0;
	WORD blockAlign = 0;

	try {
		CWaveReader reader;
		reader.Read(oldFile, flags, bitsPerSample, formatTag, channels, samplesPerSec, blockAlign);
	} catch (wave_error e) {
		app.ErrorMessage(IDS_ERROR_CANT_READ_WAVE_FILE, (LPCTSTR)oldFile);
		return FALSE;
	}

	// need to save copy (return value is destroyed)
	CString defaultDir = app.GetDefaultDir();
	CString extension = _T("wav");
	CString filter = _T("WAV Files (*.wav)|*.wav||");
	CDlgSaveAsOptions dlg((LPCTSTR)extension, (LPCTSTR)oldFile, defaultDir, OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT, filter, NULL, false, (GetNumChannels() != 1), samplesPerSec);
	if (dlg.DoModal() != IDOK) {
		return FALSE;
	}

	// validate the file status
	if (dlg.IsSameFile()) {
		// There is only one file.
		dlg.mShowFiles = showNew;
	} else if (app.IsFileOpened(dlg.GetSelectedPath())) {
		// error file already opened by SA
		app.ErrorMessage(IDS_ERROR_FILEOPENED, dlg.GetSelectedPath());
		return FALSE;
	}

	CFileStatus status;
	if (CFile::GetStatus(dlg.GetSelectedPath(), status)) {
		// File exists overwrite existing file
		try {
			status.m_attribute |= CFile::readOnly;
			CFile::SetStatus(dlg.GetSelectedPath(), status);
		} catch (...) {
			app.ErrorMessage(IDS_ERROR_FILEWRITE, dlg.GetSelectedPath());
			return false;
		}
	}

	// delete wave undo entry
	DeleteWaveFromUndo();
	if (!WriteDataFiles(dlg.GetSelectedPath(), bSaveAudio, false)) {
		return false;
	}

	// only convert if they changed the sampling rate.
	if (samplesPerSec != dlg.mSamplingRate) {
		CScopedStatusBar scopedStatusBar(IDS_RESAMPLE_WAVE);
		CWaveResampler resampler;
		CWaveResampler::ECONVERT result = resampler.Resample(dlg.GetSelectedPath(), dlg.GetSelectedPath(), dlg.mSamplingRate, scopedStatusBar);
		if (result != CWaveResampler::EC_SUCCESS) {
			return FALSE;
		}
	}

	ClearFileStatus(dlg.GetSelectedPath());
	return TRUE;
}

void CSaDoc::ClearFileStatus(LPCTSTR filename) {

	CSaApp & app = *((CSaApp *)AfxGetApp());

	// get file information
	CFile::GetStatus(filename, m_fileStat);
	SetModifiedFlag(FALSE);
	SetTransModifiedFlag(FALSE);
	SetAudioModifiedFlag(FALSE);

	// if batch mode, set file in changed state
	if (app.GetBatchMode() != 0) {
		// set changed state
		app.SetBatchFileChanged(filename, m_ID, this);
	}

}

/***************************************************************************/
// CSaDoc::WriteDataFiles Saving a document
// Stores all available information from data members in the RIFF header of
// the document (wave file). 
// The fmt and data chunks have to be there already! 
// The temporary wave file data will be copied into the wave chunk.
/***************************************************************************/
BOOL CSaDoc::WriteDataFiles(LPCTSTR pszPathName, bool bSaveAudio, bool bIsClipboardFile) {

	CScopedCursor waitCursor(*this);

	DWORD dwDataSize = m_dwDataSize;
	if (bSaveAudio) {
		dwDataSize = WriteRiff(pszPathName);
		if (dwDataSize == 0) {
			return FALSE;
		}
	}

	CoInitialize(NULL);
	ISaAudioDocumentWriterPtr saAudioDocWriter;
	HRESULT createResult = saAudioDocWriter.CreateInstance(__uuidof(SaAudioDocumentWriter));
	if (createResult) {
		CSaString szCreateResult;
		szCreateResult.Format(_T("%x"), createResult);
		ErrorMessage(IDS_ERROR_CREATE_INSTANCE, _T("SaAudioDocumentWriter.CreateInstance()"), szCreateResult);
		return FALSE;
	}

	_bstr_t szMD5HashCode = (!m_szMD5HashCode ? (wchar_t *)0 : m_szMD5HashCode);

	if (bIsClipboardFile) {
		szMD5HashCode = (wchar_t *)0;
	}

	if (!saAudioDocWriter->Initialize(pszPathName, szMD5HashCode, (short)bIsClipboardFile)) {
		ErrorMessage(IDS_ERROR_WRITEPHONETIC, pszPathName);
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

	return TRUE;
}

/***************************************************************************/
// CSaDoc::WriteRiff - The fmt and data chunks have to be there already!
// The temporary wave file data will be copied into the wave chunk.
/***************************************************************************/
DWORD CSaDoc::WriteRiff(LPCTSTR pszPathName) {

	CScopedCursor waitCursor(*this);

	// open file
	// file handle
	HMMIO hmmioFile;
	hmmioFile = mmioOpen(const_cast<TCHAR *>(pszPathName), NULL, MMIO_READWRITE | MMIO_EXCLUSIVE);
	if (!hmmioFile) {
		// error opening file
		ErrorMessage(IDS_ERROR_FILEOPEN, pszPathName);
		return 0;
	}

	// locate a 'RIFF' chunk with a 'WAVE' form type to make sure it's a WAVE file.
	MMCKINFO mmckinfoParent = {};
	mmckinfoParent.fccType = mmioFOURCC('W', 'A', 'V', 'E'); // prepare search code
	if (mmioDescend(hmmioFile, (LPMMCKINFO)&mmckinfoParent, NULL, MMIO_FINDRIFF)) {
		// error descending into wave chunk
		ErrorMessage(IDS_ERROR_WAVECHUNK, pszPathName);
		mmioClose(hmmioFile, 0);
		return 0;
	}

	// find the data chunk. It should be a subchunk of the 'RIFF' parent chunk
	MMCKINFO mmckinfoSubchunk = {};
	LONG lError = 0;
	DWORD dwDataSize = 0;
	mmckinfoSubchunk.ckid = mmioFOURCC('d', 'a', 't', 'a');
	if (lError != -1) {
		lError = mmioDescend(hmmioFile, &mmckinfoSubchunk, &mmckinfoParent, MMIO_FINDCHUNK);
	}

	if (lError != -1) {                         // copy the temporary wave file into the wave chunk
		mmckinfoSubchunk.dwFlags = MMIO_DIRTY;  // force ascend to update the chunk length
		mmckinfoParent.dwFlags = MMIO_DIRTY;

		// open the temporary wave file
		// copy the Adjust or Workbench temp file if necessary
		CopyProcessTempFile();

		CFile file;
		if (!file.Open(m_szRawDataWrk.c_str(), CFile::modeRead)) {
			// error opening file
			ErrorMessage(IDS_ERROR_OPENTEMPFILE, m_szRawDataWrk.c_str());
			mmioClose(hmmioFile, 0);
			return 0;
		}

		// use a local buffer
		char buffer[0x10000];

		// start copying wave data
		while (TRUE) {
			// read the waveform data block
			DWORD dwSizeRead = 0;
			try {
				dwSizeRead = file.Read(buffer, _countof(buffer));
			} catch (CFileException * e) {
				// error reading file
				ErrorMessage(IDS_ERROR_READTEMPFILE, m_szRawDataWrk.c_str());
				mmioClose(hmmioFile, 0);
				e->Delete();
				return 0;
			}

			if (dwSizeRead == 0) {
				break;    // all read
			}

			if (mmioWrite(hmmioFile, buffer, dwSizeRead) == -1) {
				// error writing wave file
				ErrorMessage(IDS_ERROR_WRITEDATACHUNK, pszPathName);
				mmioClose(hmmioFile, 0);
				return 0;
			}
		}

		// get out of 'data' chunk
		if (mmioAscend(hmmioFile, &mmckinfoSubchunk, 0)) {
			// error writing data chunk
			ErrorMessage(IDS_ERROR_WRITEDATACHUNK, pszPathName);
			mmioClose(hmmioFile, 0);
			return 0;
		}

		dwDataSize = mmckinfoSubchunk.cksize;
	}

	// get out of 'RIFF' chunk, to write RIFF size
	if (mmioAscend(hmmioFile, &mmckinfoParent, 0)) {
		// error writing RIFF chunk
		ErrorMessage(IDS_ERROR_WRITERIFFCHUNK, pszPathName);
		mmioClose(hmmioFile, 0);
		return 0;
	}

	// close file
	if (!mmioClose(hmmioFile, 0)) {
		// Set File Length ...
		// mmioAscend() does not set mmioinfo.lDiskOffset correctly under
		// certain conditions, so use the RIFF chunk size + 8 instead
		CFile WaveFile(pszPathName, CFile::modeReadWrite);
		DWORD dwPadBytes = mmckinfoParent.cksize - 2 * (mmckinfoParent.cksize / 2);
		DWORD dwFileSize = mmckinfoParent.cksize + 8 + dwPadBytes;
		WaveFile.SetLength(dwFileSize);
	}

	return dwDataSize;
}

/***************************************************************************/
// CSaDoc::WriteTranscription  Write the transcription from the document
// to the transcription database.
/***************************************************************************/
void CSaDoc::WriteNonSegmentData(DWORD dwDataSize, ISaAudioDocumentWriterPtr saAudioDocWriter) {

	saAudioDocWriter->DataChunkSize = dwDataSize;
	saAudioDocWriter->FormatTag = m_FmtParm.wTag;
	saAudioDocWriter->Channels = m_FmtParm.wChannels;
	saAudioDocWriter->SamplesPerSecond = m_FmtParm.dwSamplesPerSec;
	saAudioDocWriter->AverageBytesPerSecond = m_FmtParm.dwAvgBytesPerSec;
	saAudioDocWriter->BlockAlignment = m_FmtParm.wBlockAlign;
	saAudioDocWriter->BitsPerSample = m_FmtParm.wBitsPerSample;

	saAudioDocWriter->PutSADescription((_bstr_t)_to_wstring(m_saParam.szDescription).c_str());
	saAudioDocWriter->SAFlags = m_saParam.wFlags;
	saAudioDocWriter->RecordFileFormat = m_saParam.byRecordFileFormat;
	saAudioDocWriter->RecordBandWidth = m_saParam.dwRecordBandWidth;
	saAudioDocWriter->RecordSampleSize = m_saParam.byRecordSmpSize;
	saAudioDocWriter->RecordTimeStamp = m_saParam.RecordTimeStamp.GetTime();
	DWORD dwSingleChannelDataSize = dwDataSize / m_FmtParm.wChannels;
	saAudioDocWriter->NumberOfSamples = dwSingleChannelDataSize / m_FmtParm.wBlockAlign;
	saAudioDocWriter->SignalMax = m_saParam.lSignalMax;
	saAudioDocWriter->SignalMin = m_saParam.lSignalMin;
	saAudioDocWriter->SignalBandWidth = m_saParam.dwSignalBandWidth;
	saAudioDocWriter->SignalEffSampSize = m_saParam.byQuantization;

	saAudioDocWriter->CalcFreqLow = m_uttParm.nMinFreq;
	saAudioDocWriter->CalcFreqHigh = m_uttParm.nMaxFreq;
	saAudioDocWriter->CalcIntrpGap = m_uttParm.nMaxInterp;
	saAudioDocWriter->CalcPercntChng = m_uttParm.nMaxChange;
	saAudioDocWriter->CalcGrpSize = m_uttParm.nMinGroup;
	saAudioDocWriter->CalcVoicingThd = m_uttParm.nCritLoud;

	// Write speaker information
	saAudioDocWriter->PutSpeakerName((_bstr_t)m_sourceParm.szSpeaker);
	switch (m_sourceParm.nGender) {
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
void CSaDoc::WriteTranscription(int transType, ISaAudioDocumentWriterPtr saAudioDocWriter) {
	CSegment * pSegment = m_apSegments[transType];
	size_t count = pSegment->GetOffsetSize();
	if (count == 0) {
		return;
	}

	//TRACE("entries to write = %d\n",count);
	for (size_t i = 0; i < count; i++) {
		CSaString text = pSegment->GetText(i);
		DWORD dwOffset = pSegment->GetOffset(i) * m_FmtParm.wChannels;
		DWORD dwLength = pSegment->GetDuration(i) * m_FmtParm.wChannels;
		saAudioDocWriter->AddSegment(transType, dwOffset, dwLength, (_bstr_t)text);
	}
}

/***************************************************************************/
// CSaDoc::WriteGlossPosAndRefSegments Writes the gloss, part of speech and
// reference information to the database.
/***************************************************************************/
void CSaDoc::WriteGlossPosAndRefSegments(ISaAudioDocumentWriterPtr saAudioDocWriter) {
	CGlossSegment * pGloss = (CGlossSegment *)m_apSegments[GLOSS];
	CSaString szRef;
	CSaString szGloss;
	CSaString szGlossNat;

	DWORD offset;
	DWORD length;
	int nGlossNat = 0;
	int nRef = 0;

	for (int i = 0; i < pGloss->GetOffsetSize(); i++) {
		offset = pGloss->GetOffset(i);
		length = pGloss->GetDuration(i);

		szGloss = pGloss->GetText(i);
		CGlossNatSegment * pGlossNat = (CGlossNatSegment *)m_apSegments[GLOSS_NAT];
		if ((nGlossNat < pGlossNat->GetOffsetSize()) && (pGlossNat->GetOffset(nGlossNat) == offset)) {
			szGlossNat = pGlossNat->GetText(nGlossNat++);
		}

		CReferenceSegment * pRef = (CReferenceSegment *)m_apSegments[REFERENCE];
		if ((nRef < pRef->GetOffsetSize()) && (pRef->GetOffset(nRef) == offset)) {
			szRef = pRef->GetText(nRef++);
		}

		VARIANT_BOOL isBookmark = FALSE;

		// Strip off the word boundary or bookmark character.
		if ((szGloss.GetLength() > 0) &&
			((szGloss[0] == WORD_DELIMITER) || (szGloss[0] == TEXT_DELIMITER))) {
			isBookmark = (szGloss[0] == TEXT_DELIMITER);
			szGloss = szGloss.Mid(1);
		}

		// the length and offset at this point is for a single channel
		// we need to scale up to match audio data
		offset *= m_FmtParm.wChannels;
		length *= m_FmtParm.wChannels;

		saAudioDocWriter->AddMarkSegment(offset, length,
			(szGloss.GetLength() == 0 ? (wchar_t *)0 : (_bstr_t)szGloss),
			(szGlossNat.GetLength() == 0 ? (wchar_t *)0 : (_bstr_t)szGlossNat),
			(szRef.GetLength() == 0 ? (wchar_t *)0 : (_bstr_t)szRef),
			isBookmark);

		szRef.Empty();
		szGlossNat.Empty();
	}
}

/***************************************************************************/
// CSaDoc::WriteScoreData Writes the score data from the staff (SAMA)
// to the database.
/***************************************************************************/
void CSaDoc::WriteScoreData(ISaAudioDocumentWriterPtr saAudioDocWriter) {

	POSITION pos = GetFirstViewPosition();
	CSaView * pView = (CSaView *)GetNextView(pos);
	int nMusicScoreSize = 0;
	int nGraphIndex = pView->GetGraphIndexForIDD(IDD_STAFF);
	CPlotStaff * pMusicStaff = NULL;
	if (nGraphIndex >= 0) {
		CGraphWnd * pMusicGraph = pView->GetGraph(nGraphIndex);
		pMusicStaff = (CPlotStaff *)pMusicGraph->GetPlot();
		nMusicScoreSize = pMusicStaff->GetWindowTextLength();
	}

	TCHAR * pMusicScore = new TCHAR[nMusicScoreSize + 1];
	if (nMusicScoreSize) {
		if (!pMusicStaff->GetWindowText(pMusicScore, nMusicScoreSize)) {
			nMusicScoreSize = 0;
		}
	}
	pMusicScore[nMusicScoreSize] = 0;

	saAudioDocWriter->WriteAsMusicXML((_bstr_t)pMusicScore);
	if (pMusicStaff != NULL) {
		pMusicStaff->SetModifiedFlag(FALSE);
	}
	delete[] pMusicScore;
	// transcription data has been modified
	SetTransModifiedFlag(FALSE);
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
BOOL CSaDoc::CopyWaveToTemp(LPCTSTR pszSourcePathName, double dStart, double dTotalLength) {

	wstring szTempPath = FileUtils::GetTempFileName(_T("WAV"));
	TRACE(L"using %s\n", pszSourcePathName);
	TRACE(L"generating %s\n", szTempPath.c_str());

	// get pointer to view and app
	POSITION pos = GetFirstViewPosition();
	CSaView * pView = (CSaView *)GetNextView(pos);

	ASSERT(dStart < dTotalLength);

	long lSizeWritten = 0;  // this is number of bytes written
	{
		CFile file;
		// create and open or just open the file
		if (!file.Open(szTempPath.c_str(), CFile::modeCreate | CFile::modeReadWrite | CFile::shareExclusive)) {
			// error opening file
			ErrorMessage(IDS_ERROR_OPENTEMPFILE, szTempPath.c_str());
			m_dwDataSize = 0;       // no data available
			SetModifiedFlag(FALSE); // will be unable to save
			pView->SendMessage(WM_COMMAND, ID_FILE_CLOSE, 0L); // close file
			return FALSE;
		}

		// now open wave file and go to wave chunk
		HMMIO hmmioFile = mmioOpen(const_cast<TCHAR *>(pszSourcePathName), NULL, MMIO_READ | MMIO_DENYWRITE);
		if (!hmmioFile) {
			// error opening file
			ErrorMessage(IDS_ERROR_FILEOPEN, pszSourcePathName);
			m_dwDataSize = 0;       // no data available
			SetModifiedFlag(FALSE); // will be unable to save
			pView->SendMessage(WM_COMMAND, ID_FILE_CLOSE, 0L); // close file
			return FALSE;
		}
		// locate a 'RIFF' chunk with a 'WAVE' form type to make sure it's a WAVE file.
		MMCKINFO mmckinfoParent = {};
		mmckinfoParent.fccType = mmioFOURCC('W', 'A', 'V', 'E'); // prepare search code
		if (mmioDescend(hmmioFile, (LPMMCKINFO)&mmckinfoParent, NULL, MMIO_FINDRIFF)) {
			// error descending into wave chunk
			ErrorMessage(IDS_ERROR_WAVECHUNK, m_fileStat.m_szFullName);
			mmioClose(hmmioFile, 0);
			m_dwDataSize = 0;       // no data available
			SetModifiedFlag(FALSE); // will be unable to save
			pView->SendMessage(WM_COMMAND, ID_FILE_CLOSE, 0L); // close file
			return FALSE;
		}
		// find the data subchunk
		MMCKINFO mmckinfoSubchunk = {};
		mmckinfoSubchunk.ckid = mmioFOURCC('d', 'a', 't', 'a');
		if (mmioDescend(hmmioFile, &mmckinfoSubchunk, &mmckinfoParent, MMIO_FINDCHUNK)) {
			// error descending into data chunk
			ErrorMessage(IDS_ERROR_DATACHUNK, m_fileStat.m_szFullName);
			mmioClose(hmmioFile, 0);
			m_dwDataSize = 0;       // no data available
			SetModifiedFlag(FALSE); // will be unable to save
			pView->SendMessage(WM_COMMAND, ID_FILE_CLOSE, 0L); // close file
			return FALSE;
		}

		DWORD dwSmpSize = DWORD(GetSampleSize());

		// first write the front pad
		// number of zero samples to add
		try {
			long lFrontPad = ((long)(dStart*m_FmtParm.dwSamplesPerSec)*dwSmpSize) / dwSmpSize;
			TRACE(_T("front pad samples=%lu\n"), lFrontPad);
			if (dwSmpSize == 1) {
				char cData = 0;
				for (long j = 0; j < lFrontPad; j++) {
					file.Write(&cData, sizeof(cData));
					lSizeWritten++;
				}
			} else {
				short int iData = 0;
				for (long j = 0; j < lFrontPad; j++) {
					file.Write(&iData, sizeof(iData));
					lSizeWritten += 2;
				}
			}
			TRACE(_T("front pad bytes written=%lu\n"), lSizeWritten);
		} catch (CFileException * e) {
			// error writing file
			ErrorMessage(IDS_ERROR_WRITETEMPFILE, szTempPath.c_str());
			// no data available
			m_dwDataSize = 0;
			// will be unable to save
			SetModifiedFlag(FALSE);
			// close file
			pView->SendMessage(WM_COMMAND, ID_FILE_CLOSE, 0L);
			e->Delete();
			return FALSE;
		}

		// why are we here?
		// the basic assumption is that the autorecord starts when a level
		// is reached, so there is blank data in the front.
		// We need to carefully trim it

		long lTotalLength = (long)((dTotalLength)*m_FmtParm.dwSamplesPerSec);
		if (dwSmpSize == 2) {
			lTotalLength *= 2;
		}
		TRACE("lTotalLength=%lu\n", lTotalLength);

		// get the size of the data subchunk
		DWORD dwSizeLeft = mmckinfoSubchunk.cksize;
		TRACE("dwSizeLeft=%lu\n", dwSizeLeft);

		char buffer[0x10000];
		while (dwSizeLeft > 0) {
			// read the waveform data block
			long lSizeRead = mmioRead(hmmioFile, buffer, _countof(buffer));
			if (lSizeRead == -1) {
				// error reading from data chunk
				ErrorMessage(IDS_ERROR_READDATACHUNK, m_fileStat.m_szFullName);
				mmioClose(hmmioFile, 0);
				m_dwDataSize = 0;       // no data available
				SetModifiedFlag(FALSE); // will be unable to save
				pView->SendMessage(WM_COMMAND, ID_FILE_CLOSE, 0L); // close file
				return FALSE;
			}

			if ((DWORD)lSizeRead > dwSizeLeft) {
				lSizeRead = (long)dwSizeLeft;
			}
			dwSizeLeft -= (DWORD)lSizeRead;

			bool bDone = false;
			long dwDataPos = 0;
			unsigned char * pSource = (unsigned char *)buffer;
			while (dwDataPos < lSizeRead) {
				// read data
				LONG nData;
				if (dwSmpSize == 1) {
					// 8 bit per sample
					// data range is 0...255 (128 is center)
					unsigned char bData = *pSource++;
					nData = bData - 128;
					nData *= 256;
				} else {
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
				if (nMaxLevel > MIN_VOICE_LEVEL) {
					TRACE("voice found %d\n", dwDataPos);
					// write the data block from the buffer
					try {
						// always write one sample short when we are truncating so
						// that it will be zero-filled
						DWORD lWriteSize = lSizeRead - dwDataPos;
						if ((lSizeWritten + lWriteSize + dwSmpSize) > lTotalLength) {
							TRACE("truncating data\n");
							lWriteSize = lTotalLength - lSizeWritten - dwSmpSize;
							ASSERT(lWriteSize >= 0);
						}
						file.Write(&buffer[dwDataPos], (DWORD)lWriteSize);
						lSizeWritten += lWriteSize;
					} catch (CFileException * e) {
						// error writing file
						ErrorMessage(IDS_ERROR_WRITETEMPFILE, szTempPath.c_str());
						// no data available
						m_dwDataSize = 0;
						// will be unable to save
						SetModifiedFlag(FALSE);
						pView->SendMessage(WM_COMMAND, ID_FILE_CLOSE, 0L); // close file
						e->Delete();
						return FALSE;
					}
					bDone = true;
					break;
				}
			}
			if (bDone) {
				break;
			}
		}

		TRACE(_T("first segment bytes written=%lu\n"), lSizeWritten);

		while (dwSizeLeft > 0) {
			// read the waveform data block
			long lSizeRead = mmioRead(hmmioFile, buffer, _countof(buffer));
			if (lSizeRead == -1) {
				// error reading from data chunk
				ErrorMessage(IDS_ERROR_READDATACHUNK, m_fileStat.m_szFullName);
				mmioClose(hmmioFile, 0);
				m_dwDataSize = 0;       // no data available
				SetModifiedFlag(FALSE); // will be unable to save
				pView->SendMessage(WM_COMMAND, ID_FILE_CLOSE, 0L); // close file
				return FALSE;
			}

			if ((DWORD)lSizeRead > dwSizeLeft) {
				lSizeRead = (long)dwSizeLeft;
			}
			dwSizeLeft -= (DWORD)lSizeRead;

			// write the data block from the buffer
			// we will truncate the data if we need to.
			try {
				long lWriteSize = lSizeRead;
				if ((lSizeWritten + lSizeRead + dwSmpSize) > lTotalLength) {
					TRACE("truncating data\n");
					lWriteSize = lTotalLength - lSizeWritten - dwSmpSize;
				}
				ASSERT(lWriteSize >= 0);
				file.Write(buffer, (DWORD)lWriteSize);
				lSizeWritten += lWriteSize;
			} catch (CFileException * e) {
				// error writing file
				ErrorMessage(IDS_ERROR_WRITETEMPFILE, szTempPath.c_str());
				// no data available
				m_dwDataSize = 0;
				// will be unable to save
				SetModifiedFlag(FALSE);
				// close file
				pView->SendMessage(WM_COMMAND, ID_FILE_CLOSE, 0L);
				e->Delete();
				return FALSE;
			}
		}

		TRACE(_T("remainder bytes written=%lu\n"), lSizeWritten);

		// now, if this data is still to short, add the remainder
		try {
			if (lSizeWritten < lTotalLength) {
				long lRearPad = (lTotalLength - lSizeWritten) / dwSmpSize;
				TRACE(_T("rear pad samples=%lu\n"), lRearPad);
				if (dwSmpSize == 1) {
					char cData = 0;
					for (long j = 0; j < lRearPad; j++) {
						file.Write(&cData, sizeof(cData));
						lSizeWritten++;
					}
				} else {
					short int iData = 0;
					for (long j = 0; j < lRearPad; j++) {
						file.Write(&iData, sizeof(iData));
						lSizeWritten += 2;
					}
				}
			}
		} catch (CFileException * e) {
			// error writing file
			ErrorMessage(IDS_ERROR_WRITETEMPFILE, szTempPath.c_str());
			// no data available
			m_dwDataSize = 0;
			// will be unable to save
			SetModifiedFlag(FALSE);
			// close file
			pView->SendMessage(WM_COMMAND, ID_FILE_CLOSE, 0L);
			e->Delete();
			return FALSE;
		}

		TRACE(_T("total bytes written=%lu\n"), lSizeWritten);

		// ascend out of the data chunk
		mmioAscend(hmmioFile, &mmckinfoSubchunk, 0);
		// close the wave file
		mmioClose(hmmioFile, 0);
	}

	if (!m_szRawDataWrk.empty()) {
		FileUtils::Remove(m_szRawDataWrk.c_str());
	}

	m_szRawDataWrk = szTempPath.c_str();
	m_dwDataSize = lSizeWritten;

	// now use our modified temp file to recreate the wave file.
	WriteDataFiles(pszSourcePathName, true, false);

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
BOOL CSaDoc::CopyWaveToTemp(LPCTSTR pszSourcePathName) {

	TCHAR pszTempPathName[_MAX_PATH];
	wmemset(pszTempPathName, 0, _MAX_PATH);
	FileUtils::GetTempFileName(_T("WAV"), pszTempPathName, _countof(pszTempPathName));

	// get pointer to view and app
	POSITION pos = GetFirstViewPosition();
	CSaView * pView = (CSaView *)GetNextView(pos);

	CFile file;
	// create and open or just open the file
	if (!file.Open(pszTempPathName, CFile::modeCreate | CFile::modeReadWrite | CFile::shareExclusive)) {
		// error opening file
		ErrorMessage(IDS_ERROR_OPENTEMPFILE, pszTempPathName);
		m_dwDataSize = 0;       // no data available
		SetModifiedFlag(FALSE); // will be unable to save
		pView->SendMessage(WM_COMMAND, ID_FILE_CLOSE, 0L); // close file
		return FALSE;
	}

	// now open wave file and go to wave chunk
	HMMIO hmmioFile = mmioOpen(const_cast<TCHAR *>(pszSourcePathName), NULL, MMIO_READ | MMIO_DENYWRITE);
	if (!hmmioFile) {
		// error opening file
		ErrorMessage(IDS_ERROR_FILEOPEN, pszSourcePathName);
		m_dwDataSize = 0;       // no data available
		SetModifiedFlag(FALSE); // will be unable to save
		pView->SendMessage(WM_COMMAND, ID_FILE_CLOSE, 0L); // close file
		return FALSE;
	}

	// locate a 'RIFF' chunk with a 'WAVE' form type to make sure it's a WAVE file.
	MMCKINFO mmckinfoParent = {};
	mmckinfoParent.fccType = mmioFOURCC('W', 'A', 'V', 'E'); // prepare search code
	if (mmioDescend(hmmioFile, (LPMMCKINFO)&mmckinfoParent, NULL, MMIO_FINDRIFF)) {
		// error descending into wave chunk
		ErrorMessage(IDS_ERROR_WAVECHUNK, m_fileStat.m_szFullName);
		mmioClose(hmmioFile, 0);
		m_dwDataSize = 0;       // no data available
		SetModifiedFlag(FALSE); // will be unable to save
		pView->SendMessage(WM_COMMAND, ID_FILE_CLOSE, 0L); // close file
		return FALSE;
	}

	// find the data subchunk
	MMCKINFO mmckinfoSubchunk = {};
	mmckinfoSubchunk.ckid = mmioFOURCC('d', 'a', 't', 'a');
	if (mmioDescend(hmmioFile, &mmckinfoSubchunk, &mmckinfoParent, MMIO_FINDCHUNK)) {
		// error descending into data chunk
		ErrorMessage(IDS_ERROR_DATACHUNK, m_fileStat.m_szFullName);
		mmioClose(hmmioFile, 0);
		m_dwDataSize = 0;       // no data available
		SetModifiedFlag(FALSE); // will be unable to save
		pView->SendMessage(WM_COMMAND, ID_FILE_CLOSE, 0L); // close file
		return FALSE;
	}

	// get the size of the data subchunk
	DWORD dwSizeLeft = mmckinfoSubchunk.cksize;

	char buffer[0x10000] = {};

	VirtualLock(buffer, _countof(buffer));
	while (dwSizeLeft) {
		// read the waveform data block
		long lSizeRead = mmioRead(hmmioFile, buffer, _countof(buffer));
		if (lSizeRead == -1) {
			// error reading from data chunk
			ErrorMessage(IDS_ERROR_READDATACHUNK, m_fileStat.m_szFullName);
			mmioClose(hmmioFile, 0);
			m_dwDataSize = 0;       // no data available
			SetModifiedFlag(FALSE); // will be unable to save
			pView->SendMessage(WM_COMMAND, ID_FILE_CLOSE, 0L); // close file
			return FALSE;
		}

		if ((DWORD)lSizeRead > dwSizeLeft) {
			lSizeRead = (long)dwSizeLeft;
		}
		dwSizeLeft -= (DWORD)lSizeRead;

		// write the data block from the buffer
		try {
			file.Write(buffer, (DWORD)lSizeRead);
		} catch (CFileException * e) {
			// error writing file
			ErrorMessage(IDS_ERROR_WRITETEMPFILE, pszTempPathName);
			// no data available
			m_dwDataSize = 0;
			// will be unable to save
			SetModifiedFlag(FALSE);
			// close file
			pView->SendMessage(WM_COMMAND, ID_FILE_CLOSE, 0L);
			e->Delete();
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
BOOL CSaDoc::InsertWaveToTemp(LPCTSTR pszSourcePathName, LPCTSTR pszTempPathName, DWORD insertPos) {
	// get pointer to view and app
	POSITION pos = GetFirstViewPosition();
	CSaView * pView = (CSaView *)GetNextView(pos);

	CFile file;
	// open the file
	if (!file.Open(pszTempPathName, CFile::modeReadWrite | CFile::shareExclusive)) {
		// error opening file
		ErrorMessage(IDS_ERROR_OPENTEMPFILE, pszTempPathName);
		return FALSE;
	}
	// seek insert position
	try {
		file.Seek(insertPos, CFile::begin);
	} catch (CFileException * e) {
		// error opening file
		ErrorMessage(IDS_ERROR_OPENTEMPFILE, pszTempPathName);
		e->Delete();
		return FALSE;
	}

	// now open wave file and go to wave chunk
	HMMIO hmmioFile = mmioOpen(const_cast<TCHAR *>(pszSourcePathName), NULL, MMIO_READ | MMIO_DENYWRITE);
	if (!hmmioFile) {
		// error opening file
		ErrorMessage(IDS_ERROR_FILEOPEN, pszSourcePathName);
		m_dwDataSize = 0;       // no data available
		SetModifiedFlag(FALSE); // will be unable to save
		pView->SendMessage(WM_COMMAND, ID_FILE_CLOSE, 0L); // close file
		return FALSE;
	}

	// locate a 'RIFF' chunk with a 'WAVE' form type to make sure it's a WAVE file.
	MMCKINFO mmckinfoParent = {};
	mmckinfoParent.fccType = mmioFOURCC('W', 'A', 'V', 'E'); // prepare search code
	if (mmioDescend(hmmioFile, (LPMMCKINFO)&mmckinfoParent, NULL, MMIO_FINDRIFF)) {
		// error descending into wave chunk
		ErrorMessage(IDS_ERROR_WAVECHUNK, m_fileStat.m_szFullName);
		mmioClose(hmmioFile, 0);
		m_dwDataSize = 0;       // no data available
		SetModifiedFlag(FALSE); // will be unable to save
		pView->SendMessage(WM_COMMAND, ID_FILE_CLOSE, 0L); // close file
		return FALSE;
	}

	// find the data subchunk
	MMCKINFO mmckinfoSubchunk = {};
	mmckinfoSubchunk.ckid = mmioFOURCC('d', 'a', 't', 'a');
	if (mmioDescend(hmmioFile, &mmckinfoSubchunk, &mmckinfoParent, MMIO_FINDCHUNK)) {
		// error descending into data chunk
		ErrorMessage(IDS_ERROR_DATACHUNK, m_fileStat.m_szFullName);
		mmioClose(hmmioFile, 0);
		m_dwDataSize = 0;       // no data available
		SetModifiedFlag(FALSE); // will be unable to save
		pView->SendMessage(WM_COMMAND, ID_FILE_CLOSE, 0L); // close file
		return FALSE;
	}

	// get the size of the data subchunk
	DWORD dwSizeLeft = mmckinfoSubchunk.cksize;

	// use local buffer
	char buffer[0x10000] = {};

	VirtualLock(buffer, _countof(buffer));
	while (dwSizeLeft) {
		// read the waveform data block
		long lSizeRead = mmioRead(hmmioFile, (BPTR)buffer, _countof(buffer));
		if (lSizeRead == -1) {
			// error reading from data chunk
			ErrorMessage(IDS_ERROR_READDATACHUNK, m_fileStat.m_szFullName);
			mmioClose(hmmioFile, 0);
			m_dwDataSize = 0;       // no data available
			SetModifiedFlag(FALSE); // will be unable to save
			pView->SendMessage(WM_COMMAND, ID_FILE_CLOSE, 0L); // close file
			return FALSE;
		}

		if ((DWORD)lSizeRead > dwSizeLeft) {
			lSizeRead = (long)dwSizeLeft;
		}
		dwSizeLeft -= (DWORD)lSizeRead;

		// write the data block from the buffer
		try {
			file.Write(buffer, (DWORD)lSizeRead);
		} catch (CFileException * e) {
			// error writing file
			ErrorMessage(IDS_ERROR_WRITETEMPFILE, pszTempPathName);
			// no data available
			m_dwDataSize = 0;
			// will be unable to save
			SetModifiedFlag(FALSE);
			// close file
			pView->SendMessage(WM_COMMAND, ID_FILE_CLOSE, 0L);
			e->Delete();
			return FALSE;
		}
	}

	VirtualUnlock(buffer, _countof(buffer));

	// ascend out of the data chunk
	mmioAscend(hmmioFile, &mmckinfoSubchunk, 0);

	// close the wave file
	mmioClose(hmmioFile, 0);

	file.Close();

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
BOOL CSaDoc::SaveModified() {

	if (!IsModified()) {
		// ok to continue
		return TRUE;
	}

	// we never save temporary overlay documents.
	if (m_bTempOverlay) {
		return TRUE;
	}

	// file is modified, change the file attribute to allow write operation
	CString szCaption = GetTitle();
	CSaString szPathName = GetPathName();
	CString nameNoExtension;

	if (!szPathName.IsEmpty()) {
		if (!SetFileAttributes(szPathName, (DWORD)m_fileStat.m_attribute)) {
			CSaString prompt;
			AfxFormatString1(prompt, IDP_ASK_TO_SAVE, szPathName);
			switch (AfxMessageBox(prompt, MB_YESNOCANCEL, AFX_IDP_ASK_TO_SAVE)) {
			case IDCANCEL:
				// don't continue
				return FALSE;
			case IDYES:
				// If so, either Save or Update, as appropriate
				if (!DoFileSave()) {
					// don't continue
					return FALSE;
				}
				break;
			case IDNO:
				// If not saving changes, revert the document
				SetModifiedFlag(FALSE);
				SetTransModifiedFlag(FALSE);
				return TRUE;
			default:
				ASSERT(FALSE);
				return FALSE;
			}
		}
		// get name based on file title of path name
		nameNoExtension = FileUtils::GetFilename(szPathName).c_str();
		nameNoExtension = FileUtils::RemoveExtension(nameNoExtension).c_str();
	} else {
		// the pathname is empty, reset view title string
		// get the current view caption string
		int nFind = szCaption.Find(':');
		if (nFind != -1) {
			// extract part left of :
			szCaption = szCaption.Left(nFind);
		}
		nameNoExtension = szCaption;
		nameNoExtension = FileUtils::GetFilename(nameNoExtension).c_str();
		nameNoExtension = FileUtils::RemoveExtension(nameNoExtension).c_str();
	}

	CString prompt;
	AfxFormatString1(prompt, IDP_ASK_TO_SAVE, nameNoExtension);
	switch (AfxMessageBox(prompt, MB_YESNOCANCEL, AFX_IDP_ASK_TO_SAVE)) {
	case IDCANCEL:
		// don't continue
		return FALSE;
	case IDYES:
		// If so, either Save or Update, as appropriate
		if (!DoFileSave())
			// don't continue
			return FALSE;
		break;
	case IDNO:
		SetModifiedFlag(FALSE);
		SetTransModifiedFlag(FALSE);
		return TRUE;
	default:
		ASSERT(FALSE);
		break;
	}

	// the above code pretty much duplicates this call, except
	// for the recovery manager code.
	// we override the messages to be more explicit.
	BOOL bResult = CDocument::SaveModified();
	// file has been saved or changes should be abandoned
	if (bResult == TRUE) {
		SetModifiedFlag(FALSE);
		SetTransModifiedFlag(FALSE);
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
BOOL CSaDoc::CopyWave(LPCTSTR pszSourceName, LPCTSTR pszTargetName) {
	TRACE(L"copying %s to %s\n", pszSourceName, pszTargetName);

	CFile sourceFile;
	CFile targetFile; // destructor will close the files

	CFileStatus status;
	// open the files
	if (!sourceFile.Open(pszSourceName, CFile::modeRead)) {
		return FALSE;
	}

	CFileException ex;
	if (!targetFile.Open(pszTargetName, CFile::modeWrite | CFile::modeCreate, &ex)) {
		TCHAR szError[1024];
		ex.GetErrorMessage(szError, 1024);
		sourceFile.Abort(); // close the source file
		return FALSE;
	}

	// get the source file status
	if (!sourceFile.GetStatus(status)) {
		sourceFile.Abort(); // close the source file
		targetFile.Abort(); // close the target file
		return FALSE;
	}

	DWORD dwSize = status.m_size;

	try {
		targetFile.SeekToEnd();
		sourceFile.Seek(0, CFile::begin);
	} catch (CFileException * e) {
		// close the source and target files
		sourceFile.Abort();
		targetFile.Abort();
		e->Delete();
		return FALSE;
	}

	// use local buffer
	char buffer[0x10000];
	DWORD dwCopied = 0;
	while (dwSize > 0) {
		try {
			DWORD dwCopy = (dwSize > _countof(buffer)) ? _countof(buffer) : dwSize;
			dwCopied = sourceFile.Read(buffer, dwCopy);
			targetFile.Write(buffer, dwCopied);
		} catch (CFileException * e) {
			// close the source and target files
			sourceFile.Abort();
			targetFile.Abort();
			e->Delete();
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
BOOL CSaDoc::CopyWave(LPCTSTR pszSourceName, LPCTSTR pszTargetName, WAVETIME start, WAVETIME length, BOOL bTruncate) {
	TRACE(L"CopyWave %s to %s from %f to %f\n", pszSourceName, pszTargetName, start, start + length);

	CFile sourceFile;
	CFile targetFile; // destructor will close the files

	CFileStatus status;
	// open the files
	if (!sourceFile.Open(pszSourceName, CFile::modeRead)) {
		return FALSE;
	}

	CFileException ex;
	if (!targetFile.Open(pszTargetName, CFile::modeWrite | CFile::modeCreate | (bTruncate ? 0 : CFile::modeNoTruncate), &ex)) {
		TCHAR szError[1024];
		ex.GetErrorMessage(szError, 1024);
		sourceFile.Abort(); // close the source file
		return FALSE;
	}

	// get the source file status
	if (!sourceFile.GetStatus(status)) {
		sourceFile.Abort(); // close the source file
		targetFile.Abort(); // close the target file
		return FALSE;
	}

	DWORD dwSize = status.m_size;
	TRACE("source file size=%d\n", dwSize);
	DWORD startBytes = ToBytes(start, false);
	if (dwSize < startBytes) {
		return TRUE; // Empty file
	}
	TRACE("startBytes=%d\n", startBytes);

	try {
		targetFile.SeekToEnd();
		sourceFile.Seek(startBytes, CFile::begin);
	} catch (CFileException * e) {
		// close the source and target files
		sourceFile.Abort();
		targetFile.Abort();
		e->Delete();
		return FALSE;
	}
	dwSize -= startBytes; // size to copy
	DWORD lengthBytes = ToBytes(length, false);
	TRACE("lengthBytes=%d\n", lengthBytes);
	if (dwSize > lengthBytes) {
		dwSize = lengthBytes;
	}
	TRACE("size to copy=%d\n", dwSize);

	// use local buffer
	char buffer[0x10000];
	DWORD dwCopied = 0;
	while (dwSize > 0) {
		try {
			DWORD dwCopy = (dwSize > _countof(buffer)) ? _countof(buffer) : dwSize;
			dwCopied = sourceFile.Read(buffer, dwCopy);
			targetFile.Write(buffer, dwCopied);
		} catch (CFileException * e) {
			sourceFile.Abort(); // close the source file
			targetFile.Abort(); // close the target file
			e->Delete();
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
void CSaDoc::ApplyWaveFile(LPCTSTR pszFileName, DWORD dwDataSize, BOOL bInitialUpdate) {

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

	if (bInitialUpdate) {
		// SDM 1.06.6U5 Load default graph settings
		// set scrolling parameters, cursors and refresh graph(s)
		pView->InitialUpdate(TRUE);     // Load default graph settings ...
	}
	InvalidateAllProcesses();
	pView->RedrawGraphs(TRUE, TRUE);	// repaint whole graphs
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
void CSaDoc::ApplyWaveFile(LPCTSTR pszFileName, DWORD dwDataSize, CAlignInfo info) {
	TRACE(L"Applying wave file  %s of %d at %f for %f\n", pszFileName, dwDataSize, info.dStart, info.dTotalLength);
	// save the temporary file
	if (IsUsingTempFile()) {
		FileUtils::Remove(m_szTempWave);
	}

	m_szTempWave = pszFileName;
	// set the data size
	m_dwDataSize = dwDataSize;
	// get file information
	CFile::GetStatus(pszFileName, m_fileStat);

	// create the temporary wave copy
	ASSERT(info.bValid);
	CopyWaveToTemp(pszFileName, info.dStart, info.dTotalLength + info.dStart);

	// get pointer to view
	POSITION pos = GetFirstViewPosition();
	CSaView * pView = (CSaView *)GetNextView(pos);

	// SDM 1.06.6U5 Load default graph settings
	// set scrolling parameters, cursors and refresh graph(s)
	pView->InitialUpdate(TRUE);			// Load default graph settings ...
	pView->RedrawGraphs(TRUE, TRUE);	// repaint whole graphs
	SetModifiedFlag();					// document is modified
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
DWORD CSaDoc::SnapCursor(ECursorSelect nCursorSelect, DWORD dwCursorOffset, ESnapDirection nSnapDirection) {
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
	ECursorAlignment nCursorAlignment) {
	if (GetDataSize() == 0) {
		return dwCursorOffset;
	}

	if (nCursorAlignment == ALIGN_USER_SETTING) {
		// get pointer to view
		CSaView * pView = GetFirstView();
		nCursorAlignment = pView->GetCursorAlignment();
	}

	DWORD wSmpSize = GetSampleSize();
	switch (nCursorAlignment) {
	case ALIGN_AT_SAMPLE:
		return dwCursorOffset;

	case ALIGN_AT_ZERO_CROSSING:
	{
		BOOL bOk = TRUE;
		DWORD dwRight = dwUpperLimit + wSmpSize;
		DWORD dwLeft = 0;

		// search for left zero crossing first
		DWORD dwDataPos = dwCursorOffset;
		int nData = GetWaveData(dwDataPos, &bOk);   // get actual data block
		if (!bOk) {
			return dwCursorOffset;                  // error, return current position
		}
		dwDataPos -= wSmpSize;

		while ((dwDataPos >= dwLowerLimit) && (dwDataPos <= dwUpperLimit)) {
			int nOldData = nData;
			nData = GetWaveData(dwDataPos, &bOk);   // get actual data block
			if (!bOk) {
				return dwCursorOffset;              // error, return current position
			}

			// check if positive zero crossing
			if ((nData < 0) && (nOldData >= 0)) {
				// crossing found, set back match position
				if (nCursorSelect == START_CURSOR) {
					// position at zero or to the right of the zero crossing
					dwLeft = dwDataPos + wSmpSize;
				} else {
					dwLeft = dwDataPos;
				}
				break;
			}

			if (dwDataPos < wSmpSize) {
				break;
			}

			dwDataPos -= wSmpSize;
		}

		// search for right zero crossing
		dwDataPos = dwCursorOffset;
		nData = GetWaveData(dwDataPos, &bOk); // get actual data block
		if (!bOk) {
			return dwCursorOffset; // error, return current position
		}
		dwDataPos += wSmpSize;

		while ((dwDataPos >= dwLowerLimit) && (dwDataPos <= dwUpperLimit)) {
			int nOldData = nData;

			nData = GetWaveData(dwDataPos, &bOk); // get actual data block
			if (!bOk) {
				return dwCursorOffset; // error, return current position
			}

			// check if positive zero crossing
			if ((nData >= 0) && (nOldData < 0)) {
				// crossing found, set back match position
				if (nCursorSelect == START_CURSOR) {
					// position at zero or to the right of the zero crossing
					dwRight = dwDataPos;
				} else {
					dwRight = dwDataPos - wSmpSize;
				}
				break;
			}
			dwDataPos += wSmpSize;
		}

		if (dwLeft == dwCursorOffset) {
			dwRight = dwCursorOffset;
		}

		if (dwRight == dwCursorOffset) {
			dwLeft = dwCursorOffset;
		}


		if ((nSnapDirection == SNAP_BOTH) && ((dwRight > dwUpperLimit) || (dwRight < dwCursorOffset))) {
			nSnapDirection = SNAP_LEFT;
		}

		if ((nSnapDirection == SNAP_BOTH) && ((dwLeft < dwLowerLimit) || (dwLeft > dwCursorOffset))) {
			nSnapDirection = SNAP_RIGHT;
		}

		if (nSnapDirection == SNAP_BOTH) {
			if (dwRight - dwCursorOffset < dwCursorOffset - dwLeft) {
				return dwRight;
			} else {
				return dwLeft;
			}
		}

		if (nSnapDirection == SNAP_RIGHT) {
			if ((dwRight > dwUpperLimit) || (dwRight < dwCursorOffset)) {
				return dwCursorOffset;
			} else {
				return dwRight;
			}
		}

		if (nSnapDirection == SNAP_LEFT) {
			if ((dwLeft > dwCursorOffset) || (dwLeft < dwLowerLimit)) {
				return dwCursorOffset;
			} else {
				return dwLeft;
			}
		}
	}

	case ALIGN_AT_FRAGMENT:
	{
		if (m_pProcessFragments->IsDataReady()) {
			DWORD dwFragmentCount = m_pProcessFragments->GetFragmentCount();
			DWORD dwCursorIndex = dwCursorOffset / wSmpSize;
			DWORD dwFragmentIndex = m_pProcessFragments->GetFragmentIndex(dwCursorIndex);

			if (dwFragmentIndex == UNDEFINED_OFFSET) {
				return dwCursorOffset;
			}

			SFragParms stFragment = m_pProcessFragments->GetFragmentParms(dwFragmentIndex);
			DWORD dwFragmentStart = stFragment.dwOffset * wSmpSize;
			DWORD dwFragmentEnd = (stFragment.dwOffset + stFragment.wLength - 1) * wSmpSize;

			DWORD dwRight = 0;
			DWORD dwLeft = 0;

			if (nCursorSelect == START_CURSOR) {
				dwLeft = dwFragmentStart;
				if (dwFragmentIndex < dwFragmentCount - 1) {
					// move to next if not last
					stFragment = m_pProcessFragments->GetFragmentParms(++dwFragmentIndex);
					dwRight = stFragment.dwOffset * wSmpSize;
				} else {
					dwRight = dwUpperLimit + wSmpSize;
				}
			} else {
				// if we are at the end of the data, then simply pick the start
				// of the last fragment, which is the last valid data chunk.
				if (dwFragmentIndex == (dwFragmentCount - 1)) {
					return dwFragmentStart;
				}

				dwRight = dwFragmentEnd;
				if (dwFragmentIndex > 0) {
					// move to prev if not first
					stFragment = m_pProcessFragments->GetFragmentParms(--dwFragmentIndex);
					dwLeft = (stFragment.dwOffset + stFragment.wLength - 1) * wSmpSize;
				} else {
					dwLeft = 0;
				}
			}

			if (dwLeft == dwCursorOffset) {
				dwRight = dwCursorOffset;
			}

			if (dwRight == dwCursorOffset) {
				dwLeft = dwCursorOffset;
			}

			if ((nSnapDirection == SNAP_BOTH) &&
				((dwRight > dwUpperLimit) || (dwRight < dwCursorOffset))) {
				nSnapDirection = SNAP_LEFT;
			}

			if ((nSnapDirection == SNAP_BOTH) &&
				((dwLeft < dwLowerLimit) || (dwLeft > dwCursorOffset))) {
				nSnapDirection = SNAP_RIGHT;
			}

			if (nSnapDirection == SNAP_BOTH) {
				if ((dwRight - dwCursorOffset) < (dwCursorOffset - dwLeft)) {
					return dwRight;
				} else {
					return dwLeft;
				}
			}

			if (nSnapDirection == SNAP_RIGHT) {
				if ((dwRight > dwUpperLimit) || (dwRight < dwCursorOffset)) {
					return dwCursorOffset;
				} else {
					return dwRight;
				}
			}

			if (nSnapDirection == SNAP_LEFT) {
				if ((dwLeft > dwCursorOffset) || (dwLeft < dwLowerLimit)) {
					return dwCursorOffset;
				} else {
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
void CSaDoc::InvalidateAllProcesses() {
	m_pProcessDoc->SetDataInvalid();
	if (m_pProcessAdjust) {
		m_pProcessAdjust->SetDataInvalid();
	}
	if (m_pProcessFragments) {
		m_pProcessFragments->SetDataInvalid();
	}
	if (m_pProcessLoudness) {
		m_pProcessLoudness->SetDataInvalid();
	}
	if (m_pProcessSmoothLoudness) {
		m_pProcessSmoothLoudness->SetDataInvalid();
	}
	if (m_pProcessPitch) {
		m_pProcessPitch->SetDataInvalid();
	}
	if (m_pProcessCustomPitch) {
		m_pProcessCustomPitch->SetDataInvalid();
	}
	if (m_pProcessSmoothedPitch) {
		m_pProcessSmoothedPitch->SetDataInvalid();
	}
	if (m_pProcessChange) {
		m_pProcessChange->SetDataInvalid();
	}
	if (m_pProcessRaw) {
		m_pProcessRaw->SetDataInvalid();
	}
	if (m_pProcessHilbert) {
		m_pProcessHilbert->SetDataInvalid();
	}
	if (m_pProcessSpectrogram) {
		m_pProcessSpectrogram->SetDataInvalid();
		m_pProcessSpectrogram->SetProcessDataInvalid();
	}
	if (m_pProcessSnapshot) {
		m_pProcessSnapshot->SetDataInvalid();
		m_pProcessSnapshot->SetProcessDataInvalid();
	}
	if (m_pProcessFormants) {
		m_pProcessFormants->SetDataInvalid();
	}
	if (m_pProcessFormantTracker) {
		m_pProcessFormantTracker->SetDataInvalid();
	}
	if (m_pProcessDurations) {
		m_pProcessDurations->SetDataInvalid();
	}
	if (m_pProcessZCross) {
		m_pProcessZCross->SetDataInvalid();
	}
	if (m_pProcessSpectrum) {
		m_pProcessSpectrum->SetDataInvalid();
	}
	if (m_pProcessGrappl) {
		m_pProcessGrappl->SetDataInvalid();
	}
	if (m_pProcessPOA) {
		m_pProcessPOA->SetDataInvalid();
	}
	if (m_pProcessGlottis) {
		m_pProcessGlottis->SetDataInvalid();
	}
	if (m_pProcessMelogram) {
		m_pProcessMelogram->SetDataInvalid();
	}
	if (m_pProcessTonalWeightChart) {
		m_pProcessTonalWeightChart->SetDataInvalid();
	}
	if (m_pProcessSDP[0]) {
		m_pProcessSDP[0]->SetDataInvalid();
	}
	if (m_pProcessSDP[1]) {
		m_pProcessSDP[1]->SetDataInvalid();
	}
	if (m_pProcessSDP[2]) {
		m_pProcessSDP[2]->SetDataInvalid();
	}
	if (m_pProcessRatio) {
		m_pProcessRatio->SetDataInvalid();
	}
}

/***************************************************************************/
// CSaDoc::RestartAllProcesses Restart all the processes (except CAreaDataProcesses)
/***************************************************************************/
void CSaDoc::RestartAllProcesses() {
	if (m_pProcessAdjust && m_pProcessAdjust->IsCanceled()) {
		m_pProcessAdjust->SetDataInvalid();
		m_pProcessAdjust->RestartProcess();
	}
	if (m_pProcessFragments && m_pProcessFragments->IsCanceled()) {
		m_pProcessFragments->SetDataInvalid();
		m_pProcessFragments->RestartProcess();
	}
	if (m_pProcessLoudness && m_pProcessLoudness->IsCanceled()) {
		m_pProcessLoudness->SetDataInvalid();
		m_pProcessLoudness->RestartProcess();
	}
	if (m_pProcessSmoothLoudness && m_pProcessSmoothLoudness->IsCanceled()) {
		m_pProcessSmoothLoudness->SetDataInvalid();
		m_pProcessSmoothLoudness->RestartProcess();
	}
	if (m_pProcessPitch && m_pProcessPitch->IsCanceled()) {
		m_pProcessPitch->SetDataInvalid();
		m_pProcessPitch->RestartProcess();
	}
	if (m_pProcessCustomPitch && m_pProcessCustomPitch->IsCanceled()) {
		m_pProcessCustomPitch->SetDataInvalid();
		m_pProcessCustomPitch->RestartProcess();
	}
	if (m_pProcessSmoothedPitch && m_pProcessSmoothedPitch->IsCanceled()) {
		m_pProcessSmoothedPitch->SetDataInvalid();
		m_pProcessSmoothedPitch->RestartProcess();
	}
	if (m_pProcessChange && m_pProcessChange->IsCanceled()) {
		m_pProcessChange->SetDataInvalid();
		m_pProcessChange->RestartProcess();
	}
	if (m_pProcessRaw && m_pProcessRaw->IsCanceled()) {
		m_pProcessRaw->SetDataInvalid();
		m_pProcessRaw->RestartProcess();
	}
	if (m_pProcessHilbert && m_pProcessHilbert->IsCanceled()) {
		m_pProcessHilbert->SetDataInvalid();
		m_pProcessHilbert->RestartProcess();
	}
	if (m_pProcessFormants && m_pProcessFormants->IsCanceled()) {
		m_pProcessFormants->SetDataInvalid();
		m_pProcessFormants->RestartProcess();
	}
	if (m_pProcessFormantTracker && m_pProcessFormantTracker->IsCanceled()) {
		m_pProcessFormantTracker->SetDataInvalid();
		m_pProcessFormantTracker->RestartProcess();
	}
	if (m_pProcessDurations && m_pProcessDurations->IsCanceled()) {
		m_pProcessDurations->SetDataInvalid();
		m_pProcessDurations->RestartProcess();
	}
	if (m_pProcessZCross && m_pProcessZCross->IsCanceled()) {
		m_pProcessZCross->SetDataInvalid();
		m_pProcessZCross->RestartProcess();
	}
	if (m_pProcessSpectrum && m_pProcessSpectrum->IsCanceled()) {
		m_pProcessSpectrum->SetDataInvalid();
		m_pProcessSpectrum->RestartProcess();
	}
	if (m_pProcessGrappl && m_pProcessGrappl->IsCanceled()) {
		m_pProcessGrappl->SetDataInvalid();
		m_pProcessGrappl->RestartProcess();
	}
	if (m_pProcessPOA && m_pProcessPOA->IsCanceled()) {
		m_pProcessPOA->SetDataInvalid();
		m_pProcessPOA->RestartProcess();
	}
	if (m_pProcessGlottis && m_pProcessGlottis->IsCanceled()) {
		m_pProcessGlottis->SetDataInvalid();
		m_pProcessGlottis->RestartProcess();
	}
	if (m_pProcessMelogram && m_pProcessMelogram->IsCanceled()) {
		m_pProcessMelogram->SetDataInvalid();
		m_pProcessMelogram->RestartProcess();
	}
	if (m_pProcessTonalWeightChart && m_pProcessTonalWeightChart->IsCanceled()) {
		m_pProcessTonalWeightChart->SetDataInvalid();
		m_pProcessTonalWeightChart->RestartProcess();
	}
	if (m_pProcessRatio && m_pProcessRatio->IsCanceled()) {
		m_pProcessRatio->SetDataInvalid();
		m_pProcessRatio->RestartProcess();
	}
	if (m_pProcessSpectrogram) {
		m_pProcessSpectrogram->InvalidateAndRestart();
	}
	if (m_nWbProcess > 0) {
		CMainFrame * pMain = (CMainFrame *)AfxGetMainWnd();
		for (int i = 0; i < MAX_FILTER_NUMBER; i++) {
			if (!pMain->GetWbProcess(m_nWbProcess - 1, i)) {
				break;
			}
			if (!pMain->GetWbProcess(m_nWbProcess - 1, i)->IsCanceled()) {
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
BOOL CSaDoc::AnyProcessCanceled() {

	if (m_pProcessAdjust && m_pProcessAdjust->IsCanceled()) {
		return TRUE;
	}
	if (m_pProcessFragments && m_pProcessFragments->IsCanceled()) {
		return TRUE;
	}
	if (m_pProcessLoudness && m_pProcessLoudness->IsCanceled()) {
		return TRUE;
	}
	if (m_pProcessSmoothLoudness && m_pProcessSmoothLoudness->IsCanceled()) {
		return TRUE;
	}
	if (m_pProcessPitch && m_pProcessPitch->IsCanceled()) {
		return TRUE;
	}
	if (m_pProcessCustomPitch && m_pProcessCustomPitch->IsCanceled()) {
		return TRUE;
	}
	if (m_pProcessSmoothedPitch && m_pProcessSmoothedPitch->IsCanceled()) {
		return TRUE;
	}
	if (m_pProcessChange && m_pProcessChange->IsCanceled()) {
		return TRUE;
	}
	if (m_pProcessRaw && m_pProcessRaw->IsCanceled()) {
		return TRUE;
	}
	if (m_pProcessHilbert && m_pProcessHilbert->IsCanceled()) {
		return TRUE;
	}
	if (m_pProcessFormants && m_pProcessFormants->IsCanceled()) {
		return TRUE;
	}
	if (m_pProcessFormantTracker && m_pProcessFormantTracker->IsCanceled()) {
		return TRUE;
	}
	if (m_pProcessDurations && m_pProcessDurations->IsCanceled()) {
		return TRUE;
	}
	if (m_pProcessZCross && m_pProcessZCross->IsCanceled()) {
		return TRUE;
	}
	if (m_pProcessSpectrum && m_pProcessSpectrum->IsCanceled()) {
		return TRUE;
	}
	if (m_pProcessGrappl && m_pProcessGrappl->IsCanceled()) {
		return TRUE;
	}
	if (m_pProcessPOA && m_pProcessPOA->IsCanceled()) {
		return TRUE;
	}
	if (m_pProcessGlottis && m_pProcessGlottis->IsCanceled()) {
		return TRUE;
	}
	if (m_pProcessMelogram && m_pProcessMelogram->IsCanceled()) {
		return TRUE;
	}
	if (m_pProcessTonalWeightChart && m_pProcessTonalWeightChart->IsCanceled()) {
		return TRUE;
	}
	if (m_pProcessRatio && m_pProcessRatio->IsCanceled()) {
		return TRUE;
	}
	if (m_pProcessSpectrogram && m_pProcessSpectrogram->IsCanceled()) {
		return TRUE;
	}
	if (m_pProcessSpectrogram && m_pProcessSpectrogram->IsProcessCanceled()) {
		return TRUE;
	}
	if (m_pProcessSnapshot && m_pProcessSnapshot->IsCanceled()) {
		return TRUE;
	}
	CMainFrame * pMain = (CMainFrame *)AfxGetMainWnd();
	if (m_nWbProcess > 0) {
		for (int i = 0; i < MAX_FILTER_NUMBER; i++) {
			if (!pMain->GetWbProcess(m_nWbProcess - 1, i)) {
				break;
			}
			if ((pMain->GetWbProcess(m_nWbProcess - 1, i))->IsCanceled()) {
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
BOOL CSaDoc::WorkbenchProcess(BOOL bInvalidate, BOOL bRestart) {

	BOOL bProcess = FALSE;
	CMainFrame * pMain = (CMainFrame *)AfxGetMainWnd();
	m_pProcessAdjust->Process(this);

	if (m_nWbProcess > 0) {
		// get pointer to view
		POSITION pos = GetFirstViewPosition();
		CSaView * pView = (CSaView *)GetNextView(pos);
		CWbProcess * pProcess = pMain->GetWbProcess(m_nWbProcess - 1, 0);
		if (pProcess != NULL) {
			// in case of cancelled process, restart it
			pProcess->RestartProcess();
			// force process data invalidation
			if (bRestart) {
				pProcess->SetDataInvalid();
			}
			// now process
			short int nResult = LOWORD(pProcess->Process(pView));
			bProcess = TRUE;
			// check process result
			if (nResult == PROCESS_CANCELED) {
				// process has been canceled, inform user and switch back to wave file
				ErrorMessage(IDS_ERROR_WBCANCEL);
				m_nWbProcess = 0;
				bInvalidate = TRUE;
			} else {
				if (nResult == PROCESS_ERROR) {
					// processing error, inform user and switch back to wave file
					ErrorMessage(IDS_ERROR_WBPROCESS);
					m_nWbProcess = 0;
					bInvalidate = TRUE;
				} else {
					if (nResult) {
						// new data processed, reprocess graph data
						bInvalidate = TRUE;
					}
				}
			}
		}
	}

	if (bInvalidate) {
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
// handle. 
// If the flag bDelete is TRUE (default is FALSE) it deletes the copied 
// section out of the wavefile.
/***************************************************************************/
BOOL CSaDoc::PutWaveToClipboard(WAVETIME sectionStart, WAVETIME sectionLength, BOOL bDelete) {

	// because we now use true CF_WAVE we support annotations embedded in the data
	// temporary target file has to be created
	// Make new wave file with selected data and segments
	// Find and Copy Original Wave File
	DWORD tempSize = FileUtils::GetFileSize(m_szRawDataWrk.c_str());

	wstring tempNewWave = FileUtils::GetTempFileName(_T("WAV"));
	if (!CopySectionToNewWavFile(sectionStart, sectionLength, tempNewWave.c_str(), true)) {
		return FALSE;
	}

	COleDataSource * pClipData = new COleDataSource();

	// We have a wave file
	CFile file;
	if (!file.Open(tempNewWave.c_str(), CFile::modeRead | CFile::shareExclusive)) {
		TRACE("unable to open file on render\n");
		return FALSE;
	}

	DWORD dwLength = file.GetLength();

	// allocate clipboard buffer
	HGLOBAL hGlobal = ::GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, dwLength);
	if (hGlobal == NULL) {
		// memory allocation error
		ErrorMessage(IDS_ERROR_MEMALLOC);
		file.Abort();
		return FALSE;
	}

	// lock memory
	BPTR lpData = (BPTR)::GlobalLock(hGlobal);
	if (!lpData) {
		// memory lock error
		ErrorMessage(IDS_ERROR_MEMLOCK);
		::GlobalFree(hGlobal);
		file.Abort();
		return FALSE;
	}

	// Copy temporary wave file to buffer then delete
	try {
		file.Read((BPTR)lpData, dwLength);
		file.Abort();
		::GlobalUnlock(hGlobal);
	} catch (const CException &) {
		::GlobalFree(hGlobal);
		file.Abort();
		return FALSE;
	}

	// declare one format for transferring
	FORMATETC format = {};
	format.cfFormat = CF_WAVE;
	format.ptd = NULL;
	format.dwAspect = DVASPECT_CONTENT;
	format.lindex = -1;
	format.tymed = TYMED_HGLOBAL;
	pClipData->CacheGlobalData(CF_WAVE, hGlobal, &format);

	pClipData->SetClipboard();

	if (bDelete) {
		// this piece has to be deleted
		// open temporary wave file
		CFile file;
		if (!file.Open(m_szRawDataWrk.c_str(), CFile::modeReadWrite | CFile::shareExclusive)) {
			// error opening file
			ErrorMessage(IDS_ERROR_OPENTEMPFILE, m_szRawDataWrk.c_str());
			return FALSE;
		}

		DWORD dwSectionStart1 = ToBytes(sectionStart, true);
		DWORD dwSectionLength1 = ToBytes(sectionLength, true);
		DWORD dwSectionStart = ToBytes(sectionStart, false);
		DWORD dwSectionLength = ToBytes(sectionLength, false);

		DWORD dwDataTail = tempSize - dwSectionStart - dwSectionLength;
		if (dwDataTail > 0) {
			// do nothing
		}
		DWORD dwSectionPos = dwSectionStart;

		// use local buffer
		char buffer[0x10000];
		while (dwDataTail) {
			// find the source position in the data and read the wave data block
			DWORD dwReadSize;
			try {
				file.Seek((long)(dwSectionPos + dwSectionLength), CFile::begin);
				dwReadSize = file.Read(buffer, _countof(buffer));
				dwDataTail -= dwReadSize;
			} catch (CFileException * e) {
				// error reading file
				ErrorMessage(IDS_ERROR_READTEMPFILE, m_szRawDataWrk.c_str());
				e->Delete();
				return FALSE;
			}
			// find the target position and write the data block from the buffer
			try {
				file.Seek((long)(dwSectionPos), CFile::begin);
				file.Write(buffer, dwReadSize);
			} catch (CFileException * e) {
				// error writing file
				ErrorMessage(IDS_ERROR_WRITETEMPFILE, m_szRawDataWrk.c_str());
				e->Delete();
				return FALSE;
			}
			dwSectionPos += _countof(buffer);
		}
		try {
			file.SetLength(tempSize - dwSectionLength);
		} catch (CFileException * e) {
			// error writing file
			ErrorMessage(IDS_ERROR_WRITETEMPFILE, m_szRawDataWrk.c_str());
			e->Delete();
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

		((CSaView *)GetNextView(pos))->AdjustCursors(dwSectionStart1, dwSectionLength1, TRUE);  // adjust cursors to new file size
		// adjust segments to new file size
		segmentOps.ShrinkSegments(sectionStart, sectionLength);
	}

	return TRUE;
}

/***************************************************************************/
// CSaDoc::PasteClipboardToWave Pastes wave data into the wave file
// This function gets a handle with a globally allocated block of memory
// full of wave data.
// It pastes the data into the wave file at the position (in seconds) given as parameter.
// In case of error it returns FALSE, else TRUE.
/***************************************************************************/
BOOL CSaDoc::PasteClipboardToWave(HGLOBAL hData, WAVETIME insertTime) {

	TRACE("PasteClipboardToWave insertTime=%f\n", insertTime);

	TCHAR szTempPath[_MAX_PATH];
	if (!CClipboardHelper::LoadFileFromData(hData, szTempPath, _MAX_PATH)) {
		TRACE("Unable to retrieve clipboard data\n");
		return FALSE;
	}

	TRACE(L"clipping is located in %s\n", szTempPath);

	// check the file size
	DWORD originalSize = FileUtils::GetFileSize(m_szRawDataWrk.c_str());
	TRACE("originalSize=%d\n", originalSize);

	WAVETIME lengthInTime = toTime(originalSize, false);
	TRACE("length in seconds=%f\n", lengthInTime);

	CFileStatus status;
	DWORD dwPasteSize = CheckWaveFormatForPaste(szTempPath);
	if (dwPasteSize == 0) {
		TRACE("paste size is zero\n");
		FileUtils::Remove(szTempPath);
		return FALSE;
	}
	WAVETIME pasteLength = toTime(dwPasteSize, false);

	CSaApp * pApp = (CSaApp *)AfxGetApp();

	// fragment the waveform
	m_pProcessFragments->SetDataInvalid();  // remove old fragmented data

	// open temporary wave file to create new data in
	{
		TRACE("Copying portion before start cursor\n");
		TCHAR lpszRawTempPath[_MAX_PATH];
		FileUtils::GetTempFileName(_T("WAV"), lpszRawTempPath, _countof(lpszRawTempPath));

		WAVETIME start = toTimeFromSamples(0);
		WAVETIME length = insertTime;
		CopyWave(m_szRawDataWrk.c_str(), lpszRawTempPath, start, length, TRUE);

		TRACE("Inserting pasted portion\n");
		// increase the size of the file
		try {
			DWORD insertPos = ToBytes(insertTime, false);
			//Get new wave data
			if (!InsertWaveToTemp(szTempPath, lpszRawTempPath, insertPos)) {
				FileUtils::Remove(szTempPath);
				Undo(FALSE);
				return FALSE;  // Reason displayed in failed function
			}
		} catch (CFileException * e) {
			// error writing file
			ErrorMessage(IDS_ERROR_WRITETEMPFILE, lpszRawTempPath);
			FileUtils::Remove(szTempPath);
			e->Delete();
			return FALSE;
		}

		DWORD newSize = FileUtils::GetFileSize(lpszRawTempPath);
		TRACE("length after insertion=%d\n", newSize);

		TRACE("Copying portion after start cursor\n");
		start = insertTime;
		length = lengthInTime - insertTime;

		CopyWave(m_szRawDataWrk.c_str(), lpszRawTempPath, start, length, FALSE);
		FileUtils::Remove(m_szRawDataWrk.c_str());
		m_szRawDataWrk = lpszRawTempPath;
	}


	// SDM 1.06.6U4 - Change Document after checkpoint (except wave)
	// register for undo function
	m_bWaveUndoNow = TRUE;
	CheckPoint();
	m_bWaveUndoNow = FALSE;
	m_nCheckPointCount++;

	// set new data size
	m_dwDataSize = FileUtils::GetFileSize(m_szRawDataWrk.c_str());
	TRACE("new datasize=%d\n", m_dwDataSize);

	SetModifiedFlag(TRUE); // data has been modified
	SetAudioModifiedFlag();

	// check the file size
	TRACE("expected size=%d\n", (originalSize + dwPasteSize));
	ASSERT((originalSize + dwPasteSize) == m_dwDataSize);

	POSITION pos = GetFirstViewPosition();
	DWORD insertPos = ToBytes(insertTime, true);
	((CSaView *)GetNextView(pos))->SetStartStopCursorPosition(insertTime, insertTime + pasteLength, SNAP_BOTH, ALIGN_AT_SAMPLE);

	WAVETIME start = insertTime;
	WAVETIME length = pasteLength;
	// adjust segments to new file size
	segmentOps.GrowSegments(start, length);

	//Get new segments
	InsertTranscriptions(pApp->GetLastClipboardPath(), insertPos);

	return TRUE;
}

/***************************************************************************
* CSaDoc::InsertSilenceIntoWave inserts a section of silence into the current
* wave.
* silence - the length to insert specified in seconds.
* insertAt - the location where to insert the new section.  this is for
* a single channel and may need to be converted
* In case of error it returns FALSE, else TRUE.
***************************************************************************/
BOOL CSaDoc::InsertSilenceIntoWave(WAVETIME silence, WAVETIME insertAt, int repetitions) {

	//TRACE("start=%d %d\n",insertAt, repetitions);

	CURSORPOS dwSilenceSize = toCursor(silence);

	// open temporary wave file
	{
		TCHAR lpszRawTempPath[_MAX_PATH];
		FileUtils::GetTempFileName(_T("WAV"), lpszRawTempPath, _countof(lpszRawTempPath));

		// copy the 'pre' section part of the wave
		WAVETIME start = toTimeFromSamples(0);
		WAVETIME length = insertAt;
		CopyWave(m_szRawDataWrk.c_str(), lpszRawTempPath, start, length, TRUE);

		// increase the size of the file
		try {
			// get pointer to view and app
			POSITION pos = GetFirstViewPosition();
			CSaView * pView = (CSaView *)GetNextView(pos);

			CFile file;
			// open the file
			if (!file.Open(lpszRawTempPath, CFile::modeReadWrite | CFile::shareExclusive)) {
				// error opening file
				ErrorMessage(IDS_ERROR_OPENTEMPFILE, lpszRawTempPath);
				Undo(FALSE);
				return FALSE;
			}
			// seek insert position
			try {
				DWORD dwTemp = ToBytes(insertAt, false);
				file.Seek(dwTemp, CFile::begin);
			} catch (CFileException * e) {
				// error opening file
				ErrorMessage(IDS_ERROR_OPENTEMPFILE, lpszRawTempPath);
				Undo(FALSE);
				e->Delete();
				return FALSE;
			}

			// calculate the data size
			DWORD dwSizeLeft = ToBytes(silence, false);

			// use local buffer
			char buffer[0x10000];
			memset(buffer, 0, _countof(buffer));
			DWORD lSizeRead = _countof(buffer);

			VirtualLock(buffer, _countof(buffer));
			while (dwSizeLeft) {
				// read the waveform data block
				if ((DWORD)lSizeRead > dwSizeLeft) {
					lSizeRead = (long)dwSizeLeft;
				}
				dwSizeLeft -= (DWORD)lSizeRead;

				// write the data block from the buffer
				try {
					file.Write(buffer, (DWORD)lSizeRead);
				} catch (CFileException * e) {
					// error writing file
					ErrorMessage(IDS_ERROR_WRITETEMPFILE, lpszRawTempPath);
					// no data available
					m_dwDataSize = 0;
					// will be unable to save
					SetModifiedFlag(FALSE);
					// close file
					pView->SendMessage(WM_COMMAND, ID_FILE_CLOSE, 0L);
					Undo(FALSE);
					e->Delete();
					return FALSE;
				}
			}

			VirtualUnlock(buffer, _countof(buffer));

			file.Close();

			// read temporary file status and set new data size
			m_dwDataSize = FileUtils::GetFileSize(m_szRawDataWrk.c_str());

			// fragment the waveform
			m_pProcessFragments->SetDataInvalid();  // remove old fragmented data
		} catch (CFileException * e) {
			// error writing file
			ErrorMessage(IDS_ERROR_WRITETEMPFILE, lpszRawTempPath);
			e->Delete();
			return FALSE;
		}

		// copy the 'post' section part of the wave
		length = (WAVETIME)FileUtils::GetFileSize(m_szRawDataWrk.c_str());
		length /= (WAVETIME)GetBytesPerSample(false);
		length /= (WAVETIME)GetSamplesPerSec();
		CopyWave(m_szRawDataWrk.c_str(), lpszRawTempPath, insertAt, length, FALSE);

		// all done!
		FileUtils::Remove(m_szRawDataWrk.c_str());
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
	CURSORPOS startPos = toCursor(insertAt);
	((CSaView *)GetNextView(pos))->SetStartStopCursorPosition(startPos, startPos + (repetitions*dwSilenceSize), SNAP_BOTH, ALIGN_AT_SAMPLE);

	// adjust segments to new file size
	segmentOps.GrowSegments(insertAt, silence);

	return TRUE;
}

/***************************************************************************/
// CSaDoc::DeleteWaveFromUndo Deletes all wave undo entries from the undo list
// All wave undo entries have to be deleted all the previous undos also have
// to, because they are invalid now (file size has changed).
/***************************************************************************/
void CSaDoc::DeleteWaveFromUndo() {

	// first undo until no more wave entries or end of list found
	int nUndoCount = 0;
	while ((CanUndo()) && (m_nCheckPointCount > 0)) {
		Undo(TRUE, TRUE); // undo and add to redo list
		if (IsWaveToUndo()) {
			// this is a wave entry
			Redo();
			while (CanUndo()) {
				// don't undo, simply remove undo from list with no redo
				Undo(FALSE, FALSE);
			}
			m_bWaveUndoNow = FALSE;
		} else {
			nUndoCount++;
		}
	}
	// now redo
	while (nUndoCount--) {
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
void CSaDoc::UndoWaveFile() {
	while (CanUndo() && (m_nCheckPointCount)) {
		Undo(FALSE, TRUE); // undo but no redo
	}

	FileUtils::Remove(m_szRawDataWrk.c_str());
	CopyWaveToTemp(m_fileStat.m_szFullName);
	m_dwDataSize = FileUtils::GetFileSize(m_szRawDataWrk.c_str());

	InvalidateAllProcesses();
	m_bWaveUndoNow = FALSE;

	// Clear Redo List
	CheckPoint();
	Undo(FALSE, FALSE); // SDM 1.5Test10.5
}

/***************************************************************************/
// CSaDoc::OnAutoSnapUpdate Adjust all independent segments to snap boundaries
/***************************************************************************/
void CSaDoc::AutoSnapUpdate(void) {
	POSITION pos = GetFirstViewPosition();
	CSaView * pView = (CSaView *)GetNextView(pos);

	for (int independent = 0; independent < ANNOT_WND_NUMBER; independent++) {
		if (m_apSegments[independent]->GetMasterIndex() != -1) {
			continue;
		}

		CSegment * pIndependent = GetSegment(independent);

		if (pIndependent->IsEmpty()) {
			continue;
		}

		int nLoop = 0;

		while (nLoop != -1) {
			UpdateSegmentBoundaries(TRUE, independent, nLoop, pIndependent->GetOffset(nLoop), pIndependent->GetStop(nLoop));
			nLoop = pIndependent->GetNext(nLoop);
		}
	}

	// refresh the annotation windows
	for (int nLoop = 0; nLoop < MAX_GRAPHS_NUMBER; nLoop++) {
		CGraphWnd * pGraph = pView->GetGraph(nLoop);
		if (pGraph) {
			for (int nInnerLoop = 0; nInnerLoop < ANNOT_WND_NUMBER; nInnerLoop++) {
				if (pGraph->HaveAnnotation(nInnerLoop)) {
					// redraw annotation window
					pGraph->GetAnnotationWnd(nInnerLoop)->Invalidate();
				}
			}
			if (pGraph->HasBoundaries()) {
				pGraph->GetPlot()->Invalidate();
			}
		}
	}
}

/***************************************************************************/
// CSaDoc::AutoSnapUpdateNeeded Are all independent segments to snapped?
/***************************************************************************/
BOOL CSaDoc::AutoSnapUpdateNeeded(void) {
	BOOL enable = FALSE;

	for (int independent = 0; independent < ANNOT_WND_NUMBER; independent++) {
		if (m_apSegments[independent]->GetMasterIndex() != -1) {
			continue;
		}

		CSegment * pIndependent = GetSegment(independent);

		if (pIndependent->IsEmpty()) {
			continue;
		}

		int nLoop = 0;

		while (nLoop != -1) {
			if (pIndependent->GetOffset(nLoop) != SnapCursor(START_CURSOR, pIndependent->GetOffset(nLoop))) {
				enable = TRUE;
				break;
			}
			nLoop = pIndependent->GetNext(nLoop);
		}
		if (enable) {
			break;
		}
	}

	return enable;
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
BOOL CSaDoc::UpdateSegmentBoundaries(BOOL bOverlap) {

	POSITION pos = GetFirstViewPosition();
	CSaView * pView = (CSaView *)GetNextView(pos);
	CSegment * pSegment;
	int nSelection = -1;

	int nLoop = pView->FindSelectedAnnotationIndex();
	if (nLoop == -1) {
		return FALSE;
	}

	pSegment = m_apSegments[nLoop];
	nSelection = pSegment->GetSelection();

	DWORD dwStart = pView->GetStartCursorPosition();
	DWORD dwStop = pView->GetStopCursorPosition();

	BOOL result = UpdateSegmentBoundaries(bOverlap, nLoop, nSelection, dwStart, dwStop);

	// refresh the annotation windows
	for (nLoop = 0; nLoop < MAX_GRAPHS_NUMBER; nLoop++) {
		CGraphWnd * pGraph = pView->GetGraph(nLoop);
		if (pGraph) {
			for (int nInnerLoop = 0; nInnerLoop < ANNOT_WND_NUMBER; nInnerLoop++) {
				if (pGraph->HaveAnnotation(nInnerLoop)) {
					// redraw annotation window
					pGraph->GetAnnotationWnd(nInnerLoop)->Invalidate();
				}
			}
			if (pGraph->HasBoundaries()) {
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
BOOL CSaDoc::UpdateSegmentBoundaries(BOOL bOverlap, int nAnnotation, int nSelection, DWORD dwNewOffset, DWORD dwNewStop) {

	CSaDoc * pModel = this;

	int nLoop = nAnnotation;
	if (nLoop == -1) {
		return FALSE;
	}

	if (GetSegment(nLoop)->GetMasterIndex() == -1) {
		// Prepare for Update Boundaries
		CIndependentSegment * pSegment = (CIndependentSegment *)pModel->GetSegment(nLoop); //SDM 1.5Test8.1

		DWORD dwOffset = pSegment->GetOffset(nSelection);
		DWORD dwStop = pSegment->GetStop(nSelection);


		dwNewOffset = SnapCursor(START_CURSOR, dwNewOffset);
		dwNewStop = SnapCursor(STOP_CURSOR, dwNewStop);

		if (dwNewOffset >= dwNewStop) {
			return FALSE;
		}

		//SDM 1.5Test8.1
		if ((dwNewOffset == dwOffset) && (dwNewStop == dwStop)) {
			return FALSE;
		}

		if (!bOverlap) { // Move neighbors
			int nPrevious = pSegment->GetPrevious(nSelection);
			int nNext = pSegment->GetNext(nSelection);
			if ((dwNewOffset != dwOffset) && (nPrevious != -1)) {
				DWORD dwNewPreviousStop = SnapCursor(STOP_CURSOR, dwNewOffset);
				pSegment->Adjust(this, nPrevious, pSegment->GetOffset(nPrevious), dwNewPreviousStop - pSegment->GetOffset(nPrevious), false);
			}
			if (((dwNewStop) != (dwStop)) && (nNext != -1)) {
				DWORD dwNewNextOffset = SnapCursor(START_CURSOR, dwNewStop);
				pSegment->Adjust(this, nNext, dwNewNextOffset, pSegment->GetStop(nNext) - (dwNewNextOffset), false);
			}
		}

		m_bModified = TRUE; // document has been modified
		pSegment->Adjust(this, nSelection, dwNewOffset, dwNewStop - dwNewOffset, false);
	}

	CSegment* pSegment = m_apSegments[nLoop];
	if (nLoop == GLOSS) {
		// gloss mode
		// Prepare for Update Boundaries
		DWORD dwOffset = pSegment->GetOffset(nSelection);
		DWORD dwStop = pSegment->GetStop(nSelection);

		int nNextSegment = pSegment->GetNext(nSelection);

		((CDependentSegment *)pSegment)->AdjustPositionToMaster(pModel, dwNewOffset, dwNewStop);

		if ((dwNewOffset == dwOffset) && (dwNewStop == dwStop)) {
			return FALSE;
		}

		// Adjust Offset for current & Duration for previous segment & dependent segments
		((CGlossSegment *)pSegment)->Adjust(pModel, nSelection, dwNewOffset, dwNewStop - dwNewOffset, false);

		if (dwOffset != dwNewOffset) {
			if (nSelection > 0) { // Adjust previous segment
				((CGlossSegment *)pSegment)->Adjust(pModel, nSelection - 1, pSegment->GetOffset(nSelection - 1), ((CGlossSegment *)pSegment)->CalculateDuration(pModel, nSelection - 1), false);
			}
		}

		if ((dwStop) != (dwNewStop)) {
			if (nNextSegment != -1) {
				CSegment * pPhonetic = GetSegment(PHONETIC);
				DWORD dwNextStop = pSegment->GetStop(nNextSegment);

				DWORD dwNextOffset = pPhonetic->GetOffset(pPhonetic->GetNext(pPhonetic->FindStop(dwNewStop)));
				// Adjust offset for next and duration for previous segment & dependent segments
				((CGlossSegment *)pSegment)->Adjust(pModel, nNextSegment, dwNextOffset, dwNextStop - dwNextOffset, false);
			}
		}
	}
	if ((nLoop == TONE) || (nLoop == PHONEMIC) || (nLoop == ORTHO)) {
		DWORD dwOffset = pSegment->GetOffset(nSelection);
		DWORD dwStop = pSegment->GetStop(nSelection);

		((CDependentSegment *)pSegment)->AdjustPositionToMaster(pModel, dwNewOffset, dwNewStop);

		if ((dwNewOffset == dwOffset) && (dwNewStop == dwStop)) {
			return FALSE;
		}

		m_bModified = TRUE; // document has been modified

		pSegment->Adjust(this, nSelection, dwNewOffset, dwNewStop - dwNewOffset, false);
	}

	return TRUE;
}

/***************************************************************************/
// CSaDoc::GetDataSize
// Returns data size in bytes for a single channel
/***************************************************************************/
DWORD CSaDoc::GetDataSize() const {
	//TRACE("numsamples=%lu\n",GetNumSamples());
	//TRACE("samplesize=%lu\n",m_FmtParm.GetSampleSize());
	return GetNumSamples() * m_FmtParm.GetSampleSize();
}

/***************************************************************************/
// CSaDoc::GetDataSize Return size of wave source data in bytes
// Returns the size of the wave file or of a workbench process temporary
// file according to the process selected.
// this is the length of the data in bytes for all channels.
/***************************************************************************/
DWORD CSaDoc::GetRawDataSize() const {
	return m_dwDataSize;
}

/***************************************************************************/
// CSaDoc::GetWaveData Read wave data from wave source
// Reads a block of wave source data from the wave file or from a workbench
// process. If there is no process selected, it takes the wave data directly
// from the wave file, otherwise from the workbench process.
// dwOffset is the sample index
/***************************************************************************/
BPTR CSaDoc::GetWaveData(DWORD dwOffset, BOOL bBlockBegin) {
	if (m_nWbProcess > 0) {
		CWbProcess * pWbProcess = ((CMainFrame *)AfxGetMainWnd())->GetWbProcess(m_nWbProcess - 1, 0);
		if (pWbProcess != NULL) {
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
DWORD CSaDoc::GetWaveDataBufferSize() {

	if (m_nWbProcess > 0) {
		CWbProcess * pWbProcess = ((CMainFrame *)AfxGetMainWnd())->GetWbProcess(m_nWbProcess - 1, 0);
		if (pWbProcess != NULL) {
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
int CSaDoc::GetWaveData(DWORD dwOffset, BOOL * pbRes) {
	BPTR pData = GetWaveData(dwOffset); // read from file into buffer if necessary
	if (pData == NULL) {
		// error reading wave file
		*pbRes = FALSE; // set operation result
		return 0;
	}
	// read sample
	DWORD wbi = GetWaveBufferIndex();
	pData += (dwOffset - wbi);
	BYTE bData;
	int nData;
	if (m_FmtParm.wBlockAlign == 1) { // 8 bit data
		bData = *pData; // data range is 0...255 (128 is center)
		nData = bData - 128;
	} else {
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
BPTR CSaDoc::GetAdjustedUnprocessedWaveData(DWORD dwOffset) {
	TRACE("GetunprocessedWaveData1 %d\n", dwOffset);
	return m_pProcessAdjust->GetProcessedWaveData(dwOffset, FALSE);
}

/***************************************************************************/
// CSaDoc::GetWaveBufferIndex Return index for wave source data buffer
// Returns the index of the wave file read buffer or of a workbench process
// buffer according to the process selected.
/***************************************************************************/
DWORD CSaDoc::GetWaveBufferIndex() {
	if (m_nWbProcess > 0) {
		CWbProcess * pWbProcess = ((CMainFrame *)AfxGetMainWnd())->GetWbProcess(m_nWbProcess - 1, 0);
		if (pWbProcess != NULL) {
			return pWbProcess->GetProcessBufferIndex();
		}
	}
	return m_pProcessAdjust->GetProcessBufferIndex();
}

/***************************************************************************/
// CSaDoc::GetMeasurementsString Returns a tab-delimited string containing
// data at a given offset.
/***************************************************************************/
CSaString CSaDoc::GetMeasurementsString(DWORD dwOffset, DWORD dwLength, BOOL * pbRes) {

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
	short int nResult1 = LOWORD(m_pProcessGrappl->Process(this));
	short int nResult2 = LOWORD(m_pProcessLoudness->Process(this));
	short int nResult3 = LOWORD(m_pProcessFormantTracker->Process(this));
	if (nResult1 < 0 || nResult2 < 0 || nResult3 < 0) { // processing error
		*pbRes = FALSE;
		return szMeasurement;
	}

	// get filename
	szFilename = GetFilenameFromTitle().c_str(); // get the current view caption string
	int nFind = szFilename.Find(_T(" (Read-Only)"));
	if (nFind != -1) {
		szFilename = szFilename.Left(nFind);    // extract part left of first space
	}

	// get speaker
	szSpeaker = GetSourceParm()->szSpeaker;

	// get gender
	CSaString szGenderArray[3] = { _T("Male"), _T("Female"), _T("Child") };
	szGender = szGenderArray[GetGender()];

	// get time
	szTimeStart.Format(_T("%.4f"), (float)GetTimeFromBytes(dwOffset));
	if (dwLength > 0) {
		szDuration.Format(_T("%.4f"), (float)GetTimeFromBytes(dwLength));
	}

	// get pitch
	DWORD dwStartPos = (dwOffset / nSmpSize) / Grappl_calc_intvl;
	DWORD dwEndPos = ((dwOffset + dwLength) / nSmpSize) / Grappl_calc_intvl;
	double fData;
	double fSum = 0;
	int count = 0;
	for (DWORD i = dwStartPos; i <= dwEndPos; i++) {
		fData = (float)m_pProcessGrappl->GetProcessedData(i, pbRes) / (float)PRECISION_MULTIPLIER;
		if (fData > 0.0) {
			fSum += fData;
			count++;
		}
	}

	if (pbRes && (count > 0)) {
		szF0Hz.Format(_T("%.1f"), fSum / count);
		szF0st.Format(_T("%.1f"), CGraphWnd::GetSemitone(fSum / count));
	} else {
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
	for (DWORD i = dwStartPos; i <= dwEndPos; i++) {
		nRawData = m_pProcessLoudness->GetProcessedData(i, pbRes);
		if (nRawData > 0.) {
			fSum += (double)nRawData;
			count++;
		}
	}
	if (pbRes && (count > 0)) {
		szIntensity.Format(_T("%.2f"), 20. * log10(fSum / count / 32767.) + 6.);
	} else {
		szIntensity = _T("-");
	}

	// get formants
	double dBytesPerSlice = GetDataSize() / double(m_pProcessFormantTracker->GetDataSize(sizeof(SFormantFreq)));
	dwStartPos = (DWORD)floor(dwOffset / dBytesPerSlice);
	dwEndPos = (DWORD)floor((dwOffset + dwLength) / dBytesPerSlice);
	double fF1Sum = 0;
	double fF2Sum = 0;
	double fF3Sum = 0;
	double fF4Sum = 0;
	count = 0;
	for (DWORD i = dwStartPos; i <= dwEndPos; i++) {
		CProcessIterator<SFormantFreq> iterFormants(*m_pProcessFormantTracker, i);
		if ((*iterFormants).F[1] != (double)NA) {
			fF1Sum += (*iterFormants).F[1];
			fF2Sum += (*iterFormants).F[2];
			fF3Sum += (*iterFormants).F[3];
			fF4Sum += (*iterFormants).F[4];
			count++;
		}
	}
	if (count > 0) {
		szF1.Format(_T("%.1f"), fF1Sum / count);
		szF2.Format(_T("%.1f"), fF2Sum / count);
		szF3.Format(_T("%.1f"), fF3Sum / count);
		szF4.Format(_T("%.1f"), fF4Sum / count);
	} else {
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

void CSaDoc::Serialize(CArchive &) {
}

/////////////////////////////////////////////////////////////////////////////
// CSaDoc::SerializeForUndoRedo
/////////////////////////////////////////////////////////////////////////////
CArchive & operator<< (CArchive & ar, const SourceParm & parm) {
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

CArchive & operator>> (CArchive & ar, SourceParm & parm) {
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
	parm.nGender = (int)gender;
	return ar;
}

void CSaDoc::SerializeForUndoRedo(CArchive & ar) {
	
	Context & context = GetContext();
	CSegment * pDummySeg = (CSegment *)new CPhoneticSegment(context,PHONETIC);
	// get pointer to view
	POSITION pos = GetFirstViewPosition();
	CSaView * pView = (CSaView *)GetNextView(pos);

	if (pDummySeg && ar.IsStoring()) {
		CArchiveTransfer::tDWORD(ar, pView->GetStartCursorPosition());
		CArchiveTransfer::tDWORD(ar, pView->GetStopCursorPosition());
		for (int nLoop = 0; nLoop < ANNOT_WND_NUMBER; nLoop++) {
			if (m_apSegments[nLoop] && m_apSegments[nLoop]->GetOffsetSize()) {
				ar << CSaString("7");
				m_apSegments[nLoop]->Serialize(ar);
			} else if (m_apSegments[nLoop]) {
				ar << CSaString("D");
			} else {
				ar << CSaString("3");
			}
		}
		ar << (BYTE)m_bWaveUndoNow;
		ar << (WORD)m_nCheckPointCount;
		// SDM 1.06.6U4 Undo modified status
		ar << (BYTE)m_bModified;
		ar << m_sourceParm;

		CSaString temp;
		temp = m_saParam.szDescription.c_str();
		ar << temp;
		ar << m_saParam.dwNumberOfSamples;
		ar << m_saParam.lSignalMax;
		ar << m_saParam.lSignalMin;

	} else if (pDummySeg) {
		CSaString isValid;

		pView->SetStartCursorPosition(CArchiveTransfer::tDWORD(ar));
		pView->SetStopCursorPosition(CArchiveTransfer::tDWORD(ar));
		for (int nLoop = 0; nLoop < ANNOT_WND_NUMBER; nLoop++) {
			ar >> isValid;
			if (isValid == "7") {
				if (m_apSegments[nLoop]) {
					m_apSegments[nLoop]->Serialize(ar);
				} else {
					pDummySeg->Serialize(ar);
				}
			} else if (isValid == "D") {
				if (m_apSegments[nLoop]) {
					m_apSegments[nLoop]->DeleteContents();
				}
			}
		}
		ar >> (BYTE &)m_bWaveUndoNow;
		ar >> (WORD &)m_nCheckPointCount;
		// SDM 1.06.6U4 Undo modified status
		ar >> (BYTE &)m_bModified;
		ar >> m_sourceParm;

		CSaString temp;
		ar >> temp;
		m_saParam.szDescription = ::_to_utf8(temp.GetString());
		ar >> m_saParam.dwNumberOfSamples;
		ar >> m_saParam.lSignalMax;
		ar >> m_saParam.lSignalMin;
	}

	if (pDummySeg) {
		delete pDummySeg;
	}
}

/////////////////////////////////////////////////////////////////////////////
// CSaDoc diagnostics

#ifdef _DEBUG
void CSaDoc::AssertValid() const {
	CDocument::AssertValid();
}

void CSaDoc::Dump(CDumpContext & dc) const {
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
BOOL CSaDoc::DoFileSave() {

	// SDM 1.5Test10.2
	if ((m_fileStat.m_szFullName[0] == 0) ||
		(!(IsModified() || m_nWbProcess))) {
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
	if (!IsAudioModified()) {
		return SaveDocument(szPathName, false);
	}

	// this isn't a wave file, so let the user Save As...
	if (m_bUsingTempFile) {
		OnFileSaveAs();
		return TRUE;
	}

	bool bSaveAudio = true;
	if (!szPathName.IsEmpty()) {
		if (SetFileAttributes(szPathName, (DWORD)m_fileStat.m_attribute)) {
			if (FileUtils::IsReadOnly(szPathName)) {
				switch (AfxMessageBox(IDS_QUESTION_READ_ONLY, MB_YESNOCANCEL)) {
				case IDYES:
					OnFileSaveAs();
					break;
				case IDNO:
					bSaveAudio = false;
					bResult = SaveDocument(szPathName, bSaveAudio);
					break;
				case IDCANCEL:
					return FALSE;
				}
			} else {
				bResult = SaveDocument(szPathName, bSaveAudio);
			}
		} else {
			szPathName.Empty();
		}
	}
	if (szPathName.IsEmpty()) {
		// the pathname is empty, reset view title string
		CSaString fileName = GetTitle();        // get the current view caption string
		int nFind = fileName.Find(':');
		if (nFind != -1) {
			szGraphTitle = fileName.Mid(nFind);
			fileName = fileName.Left(nFind);    // extract part left of :
			fileName = fileName.Trim();
		}

		CString extension = _T("wav");
		CString filter = _T("WAV Files (*.wav)|*.wav||");
		CFileDialog dlg(FALSE, extension, fileName, OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT, filter);

		CSaApp * pApp = (CSaApp*)AfxGetApp();
		CString defaultDir = pApp->GetDefaultDir();
		dlg.m_ofn.lpstrInitialDir = defaultDir;

		if (dlg.DoModal() != IDOK) {
			return FALSE;
		} else {
			szCaption = dlg.GetFileTitle();
			szPathName = dlg.GetPathName();
		}

		CFileStatus status;
		if (CFile::GetStatus(szPathName, status)) {
			CFile::SetStatus(szPathName, status);
		}

		if (SaveDocument(szPathName, bSaveAudio)) {
			SetPathName(szPathName);
		}

		if (!szGraphTitle.IsEmpty()) {
			// set back the title string
			szCaption += szGraphTitle; // add the graph title
			SetTitle(szCaption); // write the new caption string
		}
	}

	// change the file attribute to read only
	szPathName = GetPathName();
	if ((bSaveAudio) && (!szPathName.IsEmpty())) {
		CFile::SetStatus(szPathName, m_fileStat);
	}

	return bResult;
}

/**
* Save a selected section, view or entire file to a new filename
* @parm sameFile - true if filename is same, but user is maybe changing format
*/
void CSaDoc::SaveSection(bool sameFile, LPCTSTR oldFile, LPCTSTR newFile, ESaveArea saveArea, EFileFormat fileFormat, DWORD samplingRate) {

	CScopedCursor waitCursor(*this);
	TRACE(L"old file = %s\n", oldFile);
	TRACE(L"new file = %s\n", newFile);
	TRACE(L"same? = %d\n", sameFile);

	// do all the work in a temporary file, and copy it when done
	wstring tempFile = FileUtils::GetTempFileName(L"OFSA");
	TRACE(L"temp file = %s\n", tempFile.c_str());

	switch (saveArea) {
	case saveCursors:
	{
		POSITION pos = GetFirstViewPosition();
		CSaView * pView = (CSaView *)GetNextView(pos);
		CURSORPOS dwStart = pView->GetStartCursorPosition();
		CURSORPOS dwStop = pView->GetStopCursorPosition();
		WAVETIME start = toTime((CURSORPOS)dwStart);
		WAVETIME stop = toTime((CURSORPOS)dwStop);
		if (!CopySectionToNewWavFile(start, stop - start, tempFile.c_str(), false)) {
			return;
		}
	}
	break;
	case saveView:
	{
		POSITION pos = GetFirstViewPosition();
		CSaView * pView = (CSaView *)GetNextView(pos);
		DWORD dwStart = 0;
		DWORD dwFrame = 0;
		pView->GetDataFrame(dwStart, dwFrame);
		if (GetBitsPerSample() == 16) {
			// force even if file is 16 bit.
			dwFrame &= ~0x1;
		}
		WAVETIME start = toTime((CURSORPOS)dwStart);
		WAVETIME stop = toTime((CURSORPOS)dwFrame);
		if (!CopySectionToNewWavFile(start, stop, tempFile.c_str(), false)) {
			return;
		}
	}
	break;
	case saveEntire:
	{
		// save entire file
		// show original or both?
		WAVETIME start = 0;
		WAVETIME length = toTime(GetDataSize(), true);
		if (!CopySectionToNewWavFile(start, length, tempFile.c_str(), false)) {
			return;
		}
	}
	break;
	}

	switch (fileFormat) {
	case formatStereo:
		// no conversion
		break;
	case formatMono:
		if (!ConvertToMono(true, tempFile.c_str())) {
			FileUtils::Remove(tempFile.c_str());
			return;
		}
		break;
	case formatRight:
		if (!ConvertToMono(false, tempFile.c_str())) {
			FileUtils::Remove(tempFile.c_str());
			return;
		}
		break;
	}

	if (sameFile) {
		// the same file. delete the original and rename
		DeleteFile(oldFile);
		FileUtils::Rename(tempFile.c_str(), newFile);
		// rename data as well
		wstring from = FileUtils::ReplaceExtension(tempFile.c_str(), L".saxml");
		wstring to = FileUtils::ReplaceExtension(newFile, L".saxml");
		FileUtils::Rename(from.c_str(), to.c_str());
	} else {
		// not the same, just simply rename it
		FileUtils::Rename(tempFile.c_str(), newFile);
		// rename data as well
		wstring from = FileUtils::ReplaceExtension(tempFile.c_str(), L".saxml");
		wstring to = FileUtils::ReplaceExtension(newFile, L".saxml");
		FileUtils::Rename(from.c_str(), to.c_str());
	}

	// this routine is only used during saveas.
	// IF we are making a copy, we will remove the read-only attribute.
	if (sameFile) {
		CFile::SetStatus(newFile, m_fileStat);
	} else {
		m_fileStat.m_attribute &= ~CFile::readOnly;
		CFile::SetStatus(newFile, m_fileStat);
	}
}

/***************************************************************************/
// CSaDoc::CopyProcessTempFile
// Copies either the Waveform Adjust process or Workbench process temp file
// over the raw waveform temp file to prepare for saving.
/***************************************************************************/
void CSaDoc::CopyProcessTempFile() {

	// kg this is a hack.  We currently to do not process the other channels
	// in multichannel files.  The processed data will contain data for a single channel
	// only.  if we copy it here over a multi-channel data file, the wave data will be corrupted.
	if (m_FmtParm.wChannels != 1) return;

	// check for Waveform Adjust process
	LPCTSTR pszAdjustTempPath = m_pProcessAdjust->GetProcessFileName();
	BOOL bAdjust = (pszAdjustTempPath[0] != 0) && (pszAdjustTempPath != NULL);

	// check for Workbench process
	CWbProcess * pWbProcess = NULL;
	// fallback to adjust temp file
	LPCTSTR pszWBTempPath = pszAdjustTempPath;
	if (m_nWbProcess > 0) {
		CMainFrame * pMain = (CMainFrame *)AfxGetMainWnd();
		pWbProcess = pMain->GetWbProcess(m_nWbProcess - 1, 0);
		if (pWbProcess != NULL) {
			// get processed temporary file name
			pszWBTempPath = pWbProcess->GetProcessFileName();
		}
	}

	if ((bAdjust) || (m_nWbProcess)) {
		// Workbench temp file takes precedence
		LPCTSTR pszProcTempPath = (m_nWbProcess ? pszWBTempPath : pszAdjustTempPath);
		try {
			// copy the process temp file as the temporary wave file
			if (!m_szRawDataWrk.empty()) {
				CopyWave(pszProcTempPath, m_szRawDataWrk.c_str());
			}
		} catch (CFileException * e) {
			// error copying wave file
			ErrorMessage(IDS_ERROR_WRITETEMPFILE, m_szRawDataWrk.c_str());
			e->Delete();
		}

		// get file information
		m_dwDataSize = FileUtils::GetFileSize(m_szRawDataWrk.c_str());
		m_pProcessAdjust->SetInvert(FALSE);
		m_pProcessAdjust->SetNormalize(FALSE);
		m_pProcessAdjust->SetZero(FALSE);

		// delete the workbench process
		if (pWbProcess) {
			FileUtils::Remove(pszProcTempPath);
			pWbProcess->DeleteProcessFileName();
			pWbProcess->SetDataInvalid();
		}
		m_nWbProcess = 0;
	}
}

/**
* delete the contents of a segment type
*/
void CSaDoc::DeleteSegmentContents(EAnnotation type) {
	CSegment * pSegment = m_apSegments[type];
	pSegment->DeleteContents();
}

// Split function SDM 1.5Test8.2
/***************************************************************************/
// CSaDoc::AdvancedParse Parse wave data
/***************************************************************************/
BOOL CSaDoc::AdvancedParseAuto() {
	// get pointer to view
	POSITION pos = GetFirstViewPosition();
	CSaView * pView = (CSaView *)GetNextView(pos);

	// add the gloss transcription bar to all views
	pView->SendMessage(WM_COMMAND, ID_GLOSS_ALL);

	CSegment * pSegment = m_apSegments[GLOSS];

	RestartAllProcesses();

	pSegment->RestartProcess(); // for the case of a cancelled process
	pSegment->SetDataInvalid(); // SDM 1.5Test10.7
	short int nResult = LOWORD(pSegment->Process(NULL, this));   // process data
	if (nResult == PROCESS_ERROR) {
		// error parsing
		ErrorMessage(IDS_ERROR_PARSE);
		return FALSE;
	}
	if (nResult == PROCESS_CANCELED) {
		// error canceled parsing
		ErrorMessage(IDS_CANCELED);
		return FALSE;
	}

	// for importing this is as far as we need to go
	// redraw graphs without legend window
	pView->RedrawGraphs();
	return TRUE;
}

// Split function SDM 1.5Test8.2
/***************************************************************************/
// CSaDoc::AdvancedParse Parse wave data
/***************************************************************************/
BOOL CSaDoc::AdvancedParseWord() {
	// get pointer to view
	POSITION pos = GetFirstViewPosition();
	CSaView * pView = (CSaView *)GetNextView(pos);

	RestartAllProcesses();

	DeleteSegmentContents(REFERENCE);
	DeleteSegmentContents(TONE);
	DeleteSegmentContents(PHONEMIC);
	DeleteSegmentContents(ORTHO);
	DeleteSegmentContents(PHONETIC);
	DeleteSegmentContents(GLOSS);
	DeleteSegmentContents(GLOSS_NAT);

	CSegment * pSegment = m_apSegments[GLOSS];
	pSegment->RestartProcess(); // for the case of a cancelled process
	pSegment->SetDataInvalid(); // SDM 1.5Test10.7

	short int nResult = LOWORD(pSegment->Process(NULL, this));   // process data
	if (nResult == PROCESS_ERROR) {
		// error parsing
		ErrorMessage(IDS_ERROR_PARSE);
		return FALSE;
	}
	if (nResult == PROCESS_CANCELED) {
		// error canceled parsing
		ErrorMessage(IDS_CANCELED);
		return FALSE;
	}

	// redraw graphs without legend window
	pView->RedrawGraphs();
	return TRUE;
}

// Split function SDM 1.5Test8.2
/***************************************************************************/
// CSaDoc::AdvancedParse Parse wave data
/***************************************************************************/
BOOL CSaDoc::AdvancedParsePhrase() {
	// get pointer to view
	POSITION pos = GetFirstViewPosition();
	CSaView * pView = (CSaView *)GetNextView(pos);

	// add the gloss transcription bar to all views
	pView->SendMessage(WM_COMMAND, ID_GLOSS_ALL);

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
	if (nResult == PROCESS_ERROR) {
		// error parsing
		ErrorMessage(IDS_ERROR_PARSE);
		return FALSE;
	}
	if (nResult == PROCESS_CANCELED) {
		// error canceled parsing
		ErrorMessage(IDS_CANCELED);
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
	for (int i = 0; i < pSegmentPho->GetOffsetSize(); i++) {
		DWORD dwStart = pSegmentPho->GetOffset(i);
		DWORD dwDuration = pSegmentPho->GetDuration(i);
		if (dwStart != dwLast) {
			pSegmentL1->Insert(dwOrder, delimiter, false, dwStart, dwDuration);
			pSegmentL2->Insert(dwOrder, delimiter, false, dwStart, dwDuration);
			dwLast = dwStart;
			dwOrder++;
		}
	}

	CGraphWnd * pGraph = pView->GraphIDtoPtr(IDD_RAWDATA);
	if (pGraph != NULL) {
		pGraph->ShowAnnotation(PHONETIC, TRUE, TRUE);
		pGraph->ShowAnnotation(MUSIC_PL1, TRUE, TRUE);
		pGraph->ShowAnnotation(MUSIC_PL2, TRUE, TRUE);
	}

	// redraw graphs without legend window
	pView->RedrawGraphs();
	return TRUE;
}

// Split function SDM 1.5Test8.2
/***************************************************************************/
// CSaDoc::AdvancedSegment Segment wave data
/***************************************************************************/
BOOL CSaDoc::AdvancedSegment() {

	POSITION pos = GetFirstViewPosition();
	CSaView * pView = (CSaView *)GetNextView(pos);

	pView->SendMessage(WM_COMMAND, ID_PHONETIC_ALL);

	// SDM1.5Test8.2
	CSegment * pPreserve[ANNOT_WND_NUMBER] = {};
	Context& context = GetContext();
	for (int nLoop = GLOSS; nLoop < ANNOT_WND_NUMBER; nLoop++) {
		pPreserve[nLoop] = m_apSegments[nLoop];
		if (nLoop == GLOSS) {
			m_apSegments[nLoop] = new CGlossSegment(context, GLOSS, PHONETIC);
		} else {
			m_apSegments[nLoop] = new CReferenceSegment(context, REFERENCE, GLOSS);
		}
	}

	CPhoneticSegment * pSegment = (CPhoneticSegment *)m_apSegments[PHONETIC];
	// for the case of cancelled dependent processes
	RestartAllProcesses();
	// for the case of a cancelled process
	pSegment->RestartProcess();
	// clear data from previous run SDM 1.06.4
	pSegment->SetDataInvalid();

	// process data
	short int nResult = LOWORD(pSegment->Process(NULL, this));

	// restore preserved gloss etc. SDM 1.5Test8.2
	for (int nLoop = GLOSS; nLoop < ANNOT_WND_NUMBER; nLoop++) {
		if (m_apSegments[nLoop]) {
			delete m_apSegments[nLoop];
		}
		m_apSegments[nLoop] = pPreserve[nLoop];
	}

	// Gloss segments need to be aligned to phonetic SDM 1.5Test8.2
	if (!m_apSegments[GLOSS]->IsEmpty()) {

		CGlossSegment * pGloss = (CGlossSegment *)m_apSegments[GLOSS];
		DWORD dwStart;
		DWORD dwStop;
		DWORD dwTemp;
		DWORD dwDistance;
		int nPhonetic;
		BOOL bInsert = FALSE;
		int nGloss;
		for (nGloss = 0; nGloss < pGloss->GetOffsetSize(); nGloss++) {
			dwStart = pGloss->GetOffset(nGloss);
			dwStop = dwStart + pGloss->GetDuration(nGloss);
			dwTemp = dwStop;
			nPhonetic = pGloss->AdjustPositionToMaster(this, dwStart, dwTemp);

			if (dwStart > pGloss->GetOffset(nGloss)) {
				dwDistance = dwStart - pGloss->GetOffset(nGloss);
			} else {
				dwDistance = pGloss->GetOffset(nGloss) - dwStart;
			}

			// empty phonetic
			bInsert = FALSE;
			if (nPhonetic == -1) {
				bInsert = TRUE;
				nPhonetic = 0;
			} else if (dwDistance > GetBytesFromTime(MAX_AUTOSEGMENT_MOVE_GLOSS_TIME)) {
				// too far away
				bInsert = TRUE;
				if (dwStart < pGloss->GetOffset(nGloss)) {
					nPhonetic = pSegment->GetNext(nPhonetic);
				}
			} else if (nGloss && (dwStart == pGloss->GetOffset(nGloss - 1))) {
				// last gloss attached to nearest phonetic
				bInsert = TRUE;
				nPhonetic = pSegment->GetNext(nPhonetic);
			} else if ((pGloss->GetNext(nGloss) != -1) && ((dwStart + dwTemp) / 2 <= pGloss->GetOffset(nGloss + 1))) {
				// next gloss nearest to same phonetic
				if ((pGloss->GetOffset(nGloss + 1) < dwStart) ||
					(dwDistance > (pGloss->GetOffset(nGloss + 1) - dwStart))) {
					// next gloss closer
					bInsert = TRUE;
					// nPhonetic is correct
				}
			}

			if (bInsert) {
				// Insert in same location as old segment
				dwStart = pGloss->GetOffset(nGloss);
				if (nPhonetic == -1) {
					// insert at end
					nPhonetic = pSegment->GetOffsetSize();
				}
				int nPrevious = pSegment->GetPrevious(nPhonetic);
				if (nPrevious != -1) {
					pSegment->Adjust(this, nPrevious, pSegment->GetOffset(nPrevious), dwStart - pSegment->GetOffset(nPrevious), false);
				}
				CSaString szEmpty = SEGMENT_DEFAULT_CHAR;
				pSegment->Insert(nPhonetic, szEmpty, false, dwStart, pSegment->GetOffset(nPhonetic) - dwStart);
			} else {
				pGloss->Adjust(this, nGloss, dwStart, dwStop - dwStart, false);
			}
			if (nGloss > 0) {
				// Adjust previous gloss segment
				pGloss->Adjust(this, nGloss - 1, pGloss->GetOffset(nGloss - 1), pGloss->CalculateDuration(this, nGloss - 1), false);
			}
		}
		if (nGloss > 0) {
			// Adjust previous gloss segment (last)
			pGloss->Adjust(this, nGloss - 1, pGloss->GetOffset(nGloss - 1), pGloss->CalculateDuration(this, nGloss - 1), false);
		}
	}

	if (nResult == PROCESS_ERROR) {
		// error segmenting
		ErrorMessage(IDS_ERROR_SEGMENT);
		return FALSE;
	}
	if (nResult == PROCESS_CANCELED) {
		// error canceled segmenting
		ErrorMessage(IDS_CANCELED);
		return FALSE;
	}

	// get pointer to view
	// redraw all graphs without legend window
	pView->RedrawGraphs();
	return TRUE;
}

void CSaDoc::WriteProperties(CObjectOStream & obs) {

	const CSaString & sPath = GetPathName();
	TCHAR pszFullPath[_MAX_PATH];
	_tfullpath(pszFullPath, sPath, _MAX_PATH);
	CSaString szFullPath = pszFullPath;
	obs.WriteBeginMarker(psz_sadoc, szFullPath.utf8().c_str());

	obs.WriteBeginMarker(psz_wndlst);
	POSITION pos = GetFirstViewPosition();
	while (pos) {
		// increments pos
		CSaView * pshv = (CSaView *)GetNextView(pos);
		if (pshv && pshv->IsKindOf(RUNTIME_CLASS(CSaView))) {
			pshv->WriteProperties(obs);
		}
	}
	obs.WriteEndMarker(psz_wndlst);

	obs.WriteEndMarker(psz_sadoc);
	obs.WriteNewline();
}

BOOL CSaDoc::ReadProperties(CObjectIStream & obs) {

	char buffer[1024];
	if (!obs.bReadBeginMarker(psz_sadoc, buffer, _countof(buffer))) {
		return FALSE;
	}
	CSaString sPath;
	sPath.setUtf8(buffer);
	if (_taccess(sPath, 04) != 0) {
		obs.SkipToEndMarker(psz_sadoc);
		return TRUE;
	}

	TRACE(_T("Autoload: %s\n"), sPath.GetString());
	CSaView::SetObjectStream(obs);

	while (!obs.bAtEnd()) {
		if (CSaDoc::ReadPropertiesOfViews(obs, sPath));
		else if (obs.bReadEndMarker(psz_sadoc)) {
			break;
		} else if (obs.bAtBeginOrEndMarker()) {
			break;
		} else {
			// Skip unexpected field
			obs.ReadMarkedString();
		}
	}
	CSaView::ClearObjectStream();

	return TRUE;
}

BOOL CSaDoc::ReadPropertiesOfViews(CObjectIStream & obs, const CSaString & sPath) {

	CSaDoc * pdoc = NULL;
	if (!obs.bReadBeginMarker(psz_wndlst)) {
		return FALSE;
	}
	while (!obs.bAtEnd()) {
		if (obs.bAtBeginMarker(psz_saview)) {
			if (pdoc) {
				CDocTemplate * pTemplate = pdoc->GetDocTemplate();
				ASSERT_VALID(pTemplate);
				CFrameWnd * pwndFrame = pTemplate->CreateNewFrame(pdoc, NULL);
				ASSERT(pwndFrame);
				pTemplate->InitialUpdateFrame(pwndFrame, pdoc);
			} else {
				s_bDocumentWasAlreadyOpen = TRUE; // 1996-09-05 MRP: OnOpenDocument clears this flag
				pdoc = (CSaDoc *)AfxGetApp()->OpenDocumentFile(sPath);
				if (s_bDocumentWasAlreadyOpen) {
					TRACE(_T("Not autoloading aliased file: %s\n"), sPath.GetString());
					pdoc = NULL;
				}
				if (!pdoc) {
					TRACE(_T("Database file <%s> does not exist.\n"), sPath.GetString());
					obs.SkipToEndMarker(psz_wndlst);
					break;
				}
			}
			CSaView * pview = ((CSaApp *)AfxGetApp())->GetViewActive();
			ASSERT(pview);
			ASSERT(pview->GetDocument() == pdoc);
			pview->ShowInitialStateAndZ();
			pview->ShowInitialTopState();
		} else if (obs.bReadEndMarker(psz_wndlst)) {
			break;
		} else {
			// Skip unexpected field
			obs.ReadMarkedString();
		}
	}

	return TRUE;
}

void CSaDoc::ApplyTranscriptionChanges(CTranscriptionDataSettings & settings) {
	if (!settings.m_bUseReference) {
		AlignTranscriptionData(settings);
	} else {
		AlignTranscriptionDataByRef(settings.m_TranscriptionData);
	}
	m_nTranscriptionApplicationCount++;
}

void CSaDoc::RevertTranscriptionChanges() {
	Undo(FALSE);
	POSITION pos = GetFirstViewPosition();
	CSaView * pView = (CSaView *)GetNextView(pos);
	//pView->SendMessage(WM_COMMAND,ID_EDIT_UNDO,0);
	pView->RedrawGraphs();
	if (m_nTranscriptionApplicationCount > 0) {
		m_nTranscriptionApplicationCount--;
	}
}

// SDM 1.5Test8.5
/***************************************************************************/
// CSaDoc::GetDefaultMenu Mode dependent Menu handle
/***************************************************************************/
HMENU CSaDoc::GetDefaultMenu() {
	return ((CMainFrame *)AfxGetMainWnd())->GetNewMenu();   // just use original default
}

// SDM 1.5Test8.5
/***************************************************************************/
// CSaDoc::GetDefaultAccel Mode dependent Accelerator Table handle
/***************************************************************************/
HACCEL CSaDoc::GetDefaultAccel() {
	return ((CMainFrame *)AfxGetMainWnd())->GetNewAccel();   // just use original default
}

/***************************************************************************/
// CSaDoc::OnAutoPitchDone  Pitch background processing complete handler
/***************************************************************************/
void CSaDoc::NotifyAutoPitchDone(void *) {

}

/***************************************************************************/
// CSaDoc::OnFragmentDone  Fragmenter background processing complete handler
/***************************************************************************/
void CSaDoc::NotifyFragmentDone(void * pCaller) {
	POSITION pos = GetFirstViewPosition();
	CSaView * pView = (CSaView *)GetNextView(pos);
	pView->NotifyFragmentDone(pCaller);

	CMainFrame * pMainWnd = (CMainFrame *)AfxGetMainWnd();
	pMainWnd->NotifyFragmentDone(pCaller);

	if ((pView->GetCursorAlignment() == ALIGN_AT_FRAGMENT) &&
		(AutoSnapUpdateNeeded())) {
		// Phonetic Segment was not snapped to Fragments
		CSaApp * pApp = static_cast<CSaApp *>(AfxGetApp());
		if (pApp->GetBatchMode() == 0) {
			pView->SendMessage(WM_COMMAND, ID_EDIT_AUTO_SNAP_UPDATE, 0L);
		}
	}
}

bool CSaDoc::IsTempOverlay() {
	return m_bTempOverlay;
}

void CSaDoc::SetTempOverlay() {
	m_bTempOverlay = true;
}

void CSaDoc::GetAlignInfo(CAlignInfo & info) {
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
		while (dwDataPos < dwDataSize) {
			// load source data
			// get pointer to raw data block
			BPTR pSource = m_pProcessAdjust->GetProcessedWaveData(dwDataPos, FALSE);
			DWORD dwBlockEnd = dwDataPos + dwBufferSize;
			if (dwBlockEnd > dwDataSize) {
				dwBlockEnd = dwDataSize;
				dwBufferSize = dwDataSize - dwDataPos;
			}

			TRACE(_T("block %lu %lu %lu\n"), dwDataPos, dwBlockStart, dwDataSize);
			bool bDone = false;
			while (dwDataPos < dwBlockEnd) {
				// read data
				if (wSmpSize == 1) { // 8 bit per sample
					bData = *pSource++; // data range is 0...255 (128 is center)
					nData = bData - 128;
				} else {              // 16 bit data
					nData = *((short int *)pSource++);
					pSource++;
					dwDataPos++;
				}
				// process data
				if (nData > nMaxValue) {
					nMaxValue = nData;    // store higher value
				} else {
					if (nData < nMinValue) {
						nMinValue = nData;    // store lower value
					}
				}

				// set peak level
				if (nMinValue * -1 > nMaxValue) {
					usMaxLevel = (UINT)(nMinValue * -1);
				} else {
					usMaxLevel = (UINT)nMaxValue;
				}

				// norm the level to %
				if (wSmpSize == 1) { // 8 bit per sample
					usMaxLevel = 100 * usMaxLevel / 128;
				} else {
					usMaxLevel = (UINT)((LONG)100 * (LONG)usMaxLevel / 32768);
				}

				dwDataPos++;
				if (usMaxLevel > MIN_VOICE_LEVEL) {
					info.dStart = GetTimeFromBytes(dwDataPos - wSmpSize);
					bDone = true;
					break;
				}
			}
			dwBlockStart += dwBufferSize;
			if (bDone) {
				break;
			}
		}
	}

	TRACE(_T("found @ %f of %f\n"), info.dStart, info.dTotalLength);
	info.bValid = TRUE;
}

CProcessSpectrogram * CSaDoc::GetSpectrogram() {
	if (!m_pProcessSpectrogram) {
		CMainFrame* pMainFrame = (CMainFrame*)::AfxGetMainWnd();
		m_pProcessSpectrogram = new CProcessSpectrogram(GetContext(),*pMainFrame->GetSpectrogramParmDefaults(), TRUE);
	}
	return m_pProcessSpectrogram;
}

CProcessSpectrogram* CSaDoc::GetSnapshot() {
	if (!m_pProcessSnapshot) {
		CMainFrame* pMainFrame = (CMainFrame*)::AfxGetMainWnd();
		m_pProcessSnapshot = new CProcessSpectrogram(GetContext(),*pMainFrame->GetSnapshotParmDefaults(), FALSE);
	}
	return m_pProcessSnapshot;
}

CProcessSDP * CSaDoc::GetSDP(int nIndex) {
	if (nIndex < 0 || nIndex >= 3) {
		return NULL;
	}

	if (!m_pProcessSDP[nIndex]) {
		Context& context = GetContext();
		m_pProcessSDP[nIndex] = new CProcessSDP(context);
	}

	return m_pProcessSDP[nIndex];
}

/**
* return true if this temp file is editable
*/
bool CSaDoc::CanEdit() {
	return m_bAllowEdit;
}

/**
* returns true if the file not a standard wave file
* and the document is working on a copy of the original
*/
bool CSaDoc::IsTempFile() {
	// non-wave files have this set...
	if (m_bUsingTempFile) {
		return true;
	}
	if (m_szTempConvertedWave.length() == 0) {
		return false;
	}
	return true;
}

bool CSaDoc::PreflightAddReferenceData(CDlgAutoReferenceData & dlg, int selection) {

	CGlossSegment * pGloss = (CGlossSegment *)m_apSegments[GLOSS];
	if (dlg.mUsingNumbers) {
		// apply the number
		int val = dlg.mBegin;
		// iterate through the gloss segments and add number to empty reference fields
		CReferenceSegment * pReference = (CReferenceSegment *)m_apSegments[REFERENCE];
		int start = (dlg.mUsingFirstGloss) ? 0 : selection;
		// there are references
		for (int i = start; i < pGloss->GetOffsetSize(); i++) {
			DWORD offset = pGloss->GetOffset(i);
			bool found = false;
			for (int j = 0; j < pReference->GetOffsetSize(); j++) {
				DWORD roffset = pReference->GetOffset(j);
				if (roffset == offset) {
					return true;
				} else if (roffset > offset) {
					// for this instance, we are placed before a segment
					found = true;
				}
				if (found) {
					break;
				}
			}
			if (!found) {
				// if the segment preceeding this one overlaps,
				// we need to adjust it's length
				int j = pReference->GetOffsetSize();
				if (j > 0) {
					int r = j - 1;
					DWORD poffset = pReference->GetOffset(r);
					DWORD pstop = pReference->GetStop(r);
					if ((poffset < offset) && (offset < pstop)) {
						return true;
					}
				}
			}
			if (val == dlg.mEnd) {
				break;
			}
			val++;
		}
	} else {
		// data should be fully validated by dialog!
		CTranscriptionData td;
		CSaString temp = dlg.mLastImport;

		CFileEncodingHelper feh(temp);
		if (!feh.CheckEncoding(true)) {
			return false;
		}
		wistringstream stream;
		if (!feh.ConvertFileToUTF16(stream)) {
			return false;
		}
		if (!ImportTranscription(stream, FALSE, FALSE, FALSE, FALSE, FALSE, td, true, false)) {
			// we will let the implementation code catch this...
		}

		CString ref = td.m_szPrimary;
		TranscriptionDataMap & map = td.m_TranscriptionData;
		MarkerList::iterator begin = find(map[ref].begin(), map[ref].end(), dlg.mBeginRef);
		MarkerList::iterator end = find(map[ref].begin(), map[ref].end(), dlg.mEndRef);

		// iterate through the gloss segments and add number to empty reference fields
		CReferenceSegment * pReference = (CReferenceSegment *)m_apSegments[REFERENCE];
		int start = (dlg.mUsingFirstGloss) ? 0 : selection;

		// there are references
		for (int i = start; i < pGloss->GetOffsetSize(); i++) {
			CSaString text = *begin;
			DWORD offset = pGloss->GetOffset(i);
			bool found = false;
			for (int j = 0; j < pReference->GetOffsetSize(); j++) {
				CString text2 = pReference->GetText(j).Trim();
				DWORD roffset = pReference->GetOffset(j);
				if (roffset == offset) {
					if (text2.GetLength() > 0) {
						return true;
					}
				} else if (roffset > offset) {
					// for this instance, we are placed before a segment
					found = true;
				}
				if (found) {
					break;
				}
			}
			if (!found) {
				// if the segment preceeding this one overlaps,
				// we need to adjust it's length
				int j = pReference->GetOffsetSize();
				if (j > 0) {
					int r = j - 1;
					DWORD poffset = pReference->GetOffset(r);
					DWORD pstop = pReference->GetStop(r);
					CString text2 = pReference->GetText(r).Trim();
					if ((poffset < offset) && (offset < pstop)) {
						if (text2.GetLength() > 0) {
							return true;
						}
					}
				}
			}
			if (begin == end) {
				break;
			}
			begin++;
		}
	}
	return false;
}

void CSaDoc::AddReferenceData(CDlgAutoReferenceData & dlg, int selection) {
	//NOTES
	//-refererences will always start on gloss boundaries.
	//-one or more glosses can be contained in a single reference
	//-a gloss may be empty, but the matching ref can be filled in
	//
	CGlossSegment * pGloss = (CGlossSegment *)m_apSegments[GLOSS];
	if (dlg.mUsingNumbers) {
		// apply the number
		int val = dlg.mBegin;
		// iterate through the gloss segments and add number to empty reference fields
		CReferenceSegment * pReference = (CReferenceSegment *)m_apSegments[REFERENCE];
		int start = (dlg.mUsingFirstGloss) ? 0 : selection;
		// there are references
		for (int i = start; i < pGloss->GetOffsetSize(); i++) {
			CSaString text;
			text.Format(L"%d", val);
			DWORD offset = pGloss->GetOffset(i);
			DWORD duration = pGloss->GetDuration(i);
			bool found = false;
			for (int j = 0; j < pReference->GetOffsetSize(); j++) {
				DWORD roffset = pReference->GetOffset(j);
				DWORD rduration = pReference->GetDuration(j);
				if (roffset == offset) {
					if (rduration == duration) {
						// we can just overwrite the text
						pReference->SetText(j, text);
						found = true;
					} else {
						// we can just overwrite the text
						pReference->SetAt(text, false, offset, duration, true);
						found = true;
					}
				} else if (roffset > offset) {
					// we need to insert before
					pReference->Insert(j, text, false, offset, duration);
					found = true;
				}
				if (found) {
					break;
				}
			}
			if (!found) {
				// if the segment preceeding this one overlaps,
				// we need to adjust it's length
				int j = pReference->GetOffsetSize();
				if (j > 0) {
					int r = j - 1;
					DWORD poffset = pReference->GetOffset(r);
					DWORD pstop = pReference->GetStop(r);
					if ((poffset < offset) && (offset < pstop)) {
						CSaString rtext = pReference->GetContainedText(poffset, pstop);
						pReference->SetAt(rtext, r, poffset, offset - poffset, true);
					}
				}
				// add at end
				pReference->Insert(j, text, false, offset, duration);
			}
			if (val == dlg.mEnd) {
				break;
			}
			val++;
		}
	} else {
		// data should be fully validated by dialog!
		CTranscriptionData td;
		CSaString temp = dlg.mLastImport;

		CFileEncodingHelper feh(temp);
		if (!feh.CheckEncoding(true)) {
			return;
		}
		wistringstream stream;
		if (!feh.ConvertFileToUTF16(stream)) {
			return;
		}
		if (!ImportTranscription(stream, FALSE, FALSE, FALSE, FALSE, FALSE, td, true, false)) {
			CString msg;
			msg.LoadStringW(IDS_AUTO_REF_MAIN_1);
			CString msg2;
			msg2.LoadStringW(IDS_AUTO_REF_MAIN_2);
			msg.Append(msg2);
			msg2.LoadStringW(IDS_AUTO_REF_MAIN_3);
			msg.Append(msg2);
			AfxMessageBox(msg, MB_OK | MB_ICONEXCLAMATION);
			return;
		}

		CString ref = td.m_szPrimary;
		TranscriptionDataMap & map = td.m_TranscriptionData;
		MarkerList::iterator begin = find(map[ref].begin(), map[ref].end(), dlg.mBeginRef);
		MarkerList::iterator end = find(map[ref].begin(), map[ref].end(), dlg.mEndRef);

		// iterate through the gloss segments and add number to empty reference fields
		CReferenceSegment * pReference = (CReferenceSegment *)m_apSegments[REFERENCE];
		int start = (dlg.mUsingFirstGloss) ? 0 : selection;

		// there are references
		for (int i = start; i < pGloss->GetOffsetSize(); i++) {
			CSaString text = *begin;
			DWORD offset = pGloss->GetOffset(i);
			DWORD duration = pGloss->GetDuration(i);
			bool found = false;
			for (int j = 0; j < pReference->GetOffsetSize(); j++) {
				DWORD roffset = pReference->GetOffset(j);
				DWORD rduration = pReference->GetDuration(j);
				if (roffset == offset) {
					if (rduration == duration) {
						// the segment sizes are the same
						// we can just overwrite the text
						pReference->SetText(j, text);
						found = true;
					} else {
						// we need to readjust the segment to it's new size
						// we can just overwrite the text
						pReference->SetAt(text, false, offset, duration, true);
						found = true;
					}
				} else if (roffset > offset) {
					// we need to insert before
					pReference->Insert(j, text, false, offset, duration);
					found = true;
				}
				if (found) {
					break;
				}
			}
			if (!found) {
				// if the segment preceeding this one overlaps,
				// we need to adjust it's length
				int j = pReference->GetOffsetSize();
				if (j > 0) {
					int r = j - 1;
					DWORD poffset = pReference->GetOffset(r);
					DWORD pstop = pReference->GetStop(r);
					if ((poffset < offset) && (offset < pstop)) {
						CSaString rtext = pReference->GetContainedText(poffset, pstop);
						pReference->SetAt(rtext, r, poffset, offset - poffset, true);
					}
				}
				// add at end
				pReference->Insert(j, text, false, offset, duration);
			}
			if (begin == end) {
				break;
			}
			begin++;
		}
	}
}

void CSaDoc::AlignTranscriptionData(CTranscriptionDataSettings & settings) {

	// save state for undo ability
	CheckPoint();
	SetModifiedFlag(TRUE); // document has been modified
	SetTransModifiedFlag(TRUE); // transcription has been modified

	POSITION pos = GetFirstViewPosition();
	CSaView * pView = (CSaView *)GetNextView(pos);

	enum { CHARACTER_OFFSETS = 0, CHARACTER_DURATIONS = 1, WORD_OFFSETS = 2 };
	CDWordArray pArray[3];

	// clean gloss string
	// remove trailing and leading spaces
	settings.m_szGloss.Trim(SPACE_DELIMITER);

	CPhoneticSegment * pPhonetic = (CPhoneticSegment *)GetSegment(PHONETIC);
	CGlossSegment * pGloss = (CGlossSegment *)GetSegment(GLOSS);

	if (pGloss->IsEmpty()) {
		// auto parse
		if (!AdvancedParseAuto()) {
			// process canceled by user
			Undo(FALSE);
			return;
		}
	}

	//adjust character segments
	switch (settings.m_nSegmentBy) {
	case IDC_AUTOMATIC:
	{

		if (!AdvancedSegment()) {
			// SDM 1.5Test8.2
			// process canceled by user
			Undo(FALSE);
			return;
		}
		for (int i = 0; i < pPhonetic->GetOffsetSize(); i++) {
			pArray[CHARACTER_OFFSETS].InsertAt(i, pPhonetic->GetOffset(i)); // Copy Arrays
		}
		for (int i = 0; i < pPhonetic->GetDurationSize(); i++) {
			pArray[CHARACTER_DURATIONS].InsertAt(i, pPhonetic->GetDuration(i));
		}

		// Copy gloss segments SDM 1.5Test8.2
		for (int i = 0; i < pGloss->GetOffsetSize(); i++) {
			pArray[WORD_OFFSETS].InsertAt(i, pGloss->GetOffset(i));
		}
		// Create a gloss break at initial position SDM 1.5Test8.2
		if (pArray[WORD_OFFSETS][0] != pArray[CHARACTER_OFFSETS][0]) {
			CSaString szEmpty = "";
			pGloss->Insert(0, szEmpty, false, pArray[CHARACTER_OFFSETS][0], pArray[WORD_OFFSETS][0] - pArray[CHARACTER_OFFSETS][0]);
			pArray[WORD_OFFSETS].InsertAt(0, pArray[CHARACTER_OFFSETS][0]);
			settings.m_szGloss = CSaString(SPACE_DELIMITER) + settings.m_szGloss;
			settings.m_szGlossNat = CSaString(SPACE_DELIMITER) + settings.m_szGlossNat;
			settings.m_szPhonetic = CSaString(SPACE_DELIMITER) + settings.m_szPhonetic;
			settings.m_szPhonemic = CSaString(SPACE_DELIMITER) + settings.m_szPhonemic;
			settings.m_szOrthographic = CSaString(SPACE_DELIMITER) + settings.m_szOrthographic;
		}
		break;
	}

	case IDC_MANUAL:
	{
		// SDM 1.5Test8.2
		for (int i = 0; i < pGloss->GetOffsetSize(); i++) {
			pArray[WORD_OFFSETS].InsertAt(i, pGloss->GetOffset(i)); // Copy gloss segments SDM 1.5Test8.2
		}
		switch (settings.m_nAlignBy) {
		case IDC_NONE:
		case IDC_WORD:
		{
			pArray[CHARACTER_OFFSETS].InsertAt(0, &pArray[WORD_OFFSETS]); // Copy gloss segments
			// build duration list
			int nIndex = 1;

			while (nIndex < pArray[CHARACTER_OFFSETS].GetSize()) {
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
			for (int nGlossWordIndex = 0; nGlossWordIndex < pArray[WORD_OFFSETS].GetSize(); nGlossWordIndex++) {
				DWORD dwDuration;
				if ((nGlossWordIndex + 1) < pArray[WORD_OFFSETS].GetSize()) {
					dwDuration = pArray[WORD_OFFSETS][nGlossWordIndex + 1] - pArray[WORD_OFFSETS][nGlossWordIndex];
					szWord = pTable->GetNext(CFontTable::DELIMITEDWORD, nWord, settings.m_szPhonetic);
					nCharacters = pTable->GetLength(CFontTable::CHARACTER, szWord);
				} else {
					dwDuration = GetDataSize() - pArray[WORD_OFFSETS][nGlossWordIndex];
					szWord = pTable->GetNext(CFontTable::DELIMITEDWORD, nWord, settings.m_szPhonetic);
					nCharacters = pTable->GetLength(CFontTable::CHARACTER, szWord);
					szWord = pTable->GetRemainder(CFontTable::DELIMITEDWORD, nWord, settings.m_szPhonetic);
					if (szWord.GetLength() != 0) {
						nCharacters++;    // remainder goes into one extra segment
					}
				}
				if (nCharacters == 0) {
					nCharacters++;
				}

				DWORD dwOffset = pArray[WORD_OFFSETS][nGlossWordIndex];
				DWORD dwSize = dwDuration / nCharacters;

				if (Is16Bit()) {
					dwSize &= ~1;
				}

				for (int nIndex = 0; nIndex < nCharacters; nIndex++) {
					pArray[CHARACTER_OFFSETS].Add(dwOffset);
					dwOffset = SnapCursor(STOP_CURSOR, dwOffset + dwSize, dwOffset + (dwSize / 2) & ~1, dwOffset + dwSize, SNAP_LEFT);
				}
			}
			int nIndex = 1;
			while (nIndex < pArray[CHARACTER_OFFSETS].GetSize()) {
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

	case IDC_KEEP:
	{
		// SDM 1.5Test8.2
		// Copy gloss segments SDM 1.5Test8.2
		for (int i = 0; i < pGloss->GetOffsetSize(); i++) {
			DWORD offset = pGloss->GetOffset(i);
			TRACE("word offset %d\n", offset);
			pArray[WORD_OFFSETS].InsertAt(i, offset);
		}

		// copy segment locations not character counts
		int nIndex = 0;
		while (nIndex != -1) {
			DWORD offset = pPhonetic->GetOffset(nIndex);
			pArray[CHARACTER_OFFSETS].Add(offset);
			TRACE("character offset %d\n", offset);
			DWORD duration = pPhonetic->GetDuration(nIndex);
			pArray[CHARACTER_DURATIONS].Add(duration);
			TRACE("character duration %d\n", offset);
			nIndex = pPhonetic->GetNext(nIndex);
		}
	}
	}

	CFontTable::tUnit nAlignMode = CFontTable::CHARACTER;
	switch (settings.m_nAlignBy) {
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
		if ((settings.m_bPhonetic) && ((settings.m_nSegmentBy != IDC_KEEP) || (settings.m_bPhoneticModified))) {

			nStringIndex = 0;
			nGlossIndex = 0;
			nWordIndex = 0;
			CSegment * pPhonetic = GetSegment(PHONETIC);
			pTable = GetFont(PHONETIC);
			pPhonetic->DeleteContents(); // Delete contents and reinsert from scratch

			nOffsetSize = pArray[CHARACTER_OFFSETS].GetSize();
			for (nIndex = 0; nIndex < (nOffsetSize - 1); nIndex++) {
				switch (settings.m_nAlignBy) {
				case IDC_NONE:
					szNext = pTable->GetNext(nAlignMode, nStringIndex, settings.m_szPhonetic);
					if (szNext.GetLength() == 0) {
						szNext += SEGMENT_DEFAULT_CHAR;
					}
					pPhonetic->Append(szNext, FALSE, pArray[CHARACTER_OFFSETS][nIndex], pArray[CHARACTER_DURATIONS][nIndex]);
					break;
				case IDC_WORD:
					if (nGlossIndex >= pArray[WORD_OFFSETS].GetSize()) {
						// No more word breaks continue one character at a time
						szNext = pTable->GetNext(nAlignMode, nStringIndex, settings.m_szPhonetic);
					} else if (pArray[CHARACTER_OFFSETS][nIndex] < pArray[WORD_OFFSETS][nGlossIndex]) {
						// Insert default segment character if phonetic offset does not correspond to word boundary
						szNext = SEGMENT_DEFAULT_CHAR;
					} else {
						// Insert Word on Gloss Boundary
						szNext = pTable->GetNext(nAlignMode, nStringIndex, settings.m_szPhonetic);
						nGlossIndex++;  // Increment word index
					}
					if (szNext.GetLength() == 0) {
						szNext += SEGMENT_DEFAULT_CHAR;
					}
					pPhonetic->Append(szNext, FALSE, pArray[CHARACTER_OFFSETS][nIndex], pArray[CHARACTER_DURATIONS][nIndex]);
					break;
				case IDC_CHARACTER:
					// the line is entered one character per segment
					szNext.Empty();
					while (true) {
						CSaString szTemp = pTable->GetNext(nAlignMode, nStringIndex, settings.m_szPhonetic);
						if (szTemp.GetLength() == 0) {
							// end of array
							szTemp = SEGMENT_DEFAULT_CHAR;
							break;
						} else if ((szTemp.GetLength() == 1) && (szTemp[0] == SPACE_DELIMITER)) {
							// time to stop!
							break;
						} else if (szTemp.GetLength() > 1) {
							// in some situations if the trailing character is not a break
							// it will be combined with the space.  we will break it here.
							if (szTemp[0] == SPACE_DELIMITER) {
								if (szNext.GetLength() == 0) {
									// remove space and append
									szTemp.Delete(0, 1);
								} else {
									// backup and let the next character go into the next segment
									nStringIndex--;
									break;
								}
							}
						}
						szNext += szTemp;
					}
					pPhonetic->Append(szNext, FALSE, pArray[CHARACTER_OFFSETS][nIndex], pArray[CHARACTER_DURATIONS][nIndex]);
					break;
				}
			}

			szNext = pTable->GetRemainder(nAlignMode, nStringIndex, settings.m_szPhonetic);
			if (szNext.GetLength() == 0) {
				szNext += SEGMENT_DEFAULT_CHAR;
			}
			pPhonetic->Append(szNext, FALSE, pArray[CHARACTER_OFFSETS][nOffsetSize - 1], pArray[CHARACTER_DURATIONS][nOffsetSize - 1]);

			// SDM 1.06.8 apply input filter to segment
			pPhonetic->Filter();
		}

		// Process phonemic
		// SDM 1.06.8 only change  if new segmentation or text changed
		if ((settings.m_bPhonemic) &&
			((settings.m_nSegmentBy != IDC_KEEP) || (settings.m_bPhonemicModified))) {

			nStringIndex = 0;
			nGlossIndex = 0;
			nWordIndex = 0;
			CPhonemicSegment * pPhonemic = (CPhonemicSegment *)GetSegment(PHONEMIC);
			pTable = GetFont(PHONEMIC);
			pPhonemic->DeleteContents(); // Delete contents and reinsert from scratch

			nOffsetSize = pArray[CHARACTER_OFFSETS].GetSize();
			for (nIndex = 0; nIndex < (nOffsetSize - 1); nIndex++) {
				switch (settings.m_nAlignBy) {
				case IDC_NONE:
					szNext = pTable->GetNext(nAlignMode, nStringIndex, settings.m_szPhonemic);
					if (szNext.GetLength() != 0) {
						// Skip Empty Segments
						pPhonemic->Append(szNext, FALSE, pArray[CHARACTER_OFFSETS][nIndex], pArray[CHARACTER_DURATIONS][nIndex]);
					}
					break;
				case IDC_WORD:
					if (nGlossIndex >= pArray[WORD_OFFSETS].GetSize()) {
						// No more word breaks continue one character at a time
						szNext = pTable->GetNext(nAlignMode, nStringIndex, settings.m_szPhonemic);
					} else if (pArray[CHARACTER_OFFSETS][nIndex] < pArray[WORD_OFFSETS][nGlossIndex]) {
						// Skip if phonetic offset does not correspond to word boundary
						continue;
					} else {
						// Insert Word on Gloss Boundary
						szNext = pTable->GetNext(nAlignMode, nStringIndex, settings.m_szPhonemic);
						nGlossIndex++;  // Increment word index
					}
					if (szNext.GetLength() == 0) {
						// Skip NULL strings
						continue;
					}
					pPhonemic->Append(szNext, FALSE, pArray[CHARACTER_OFFSETS][nIndex], pArray[CHARACTER_DURATIONS][nIndex]);
					break;
				case IDC_CHARACTER:
					// the line is entered one character per segment
					szNext.Empty();
					while (true) {
						CSaString szTemp = pTable->GetNext(nAlignMode, nStringIndex, settings.m_szPhonemic);
						if (szTemp.GetLength() == 0) {
							break;
						} else if ((szTemp.GetLength() == 1) && (szTemp[0] == SPACE_DELIMITER)) {
							// time to stop!
							break;
						} else if (szTemp.GetLength() > 1) {
							// in some situations if the trailing character is not a break
							// it will be combined with the space.  we will break it here.
							if (szTemp[0] == SPACE_DELIMITER) {
								if (szNext.GetLength() == 0) {
									// remove space and append
									szTemp.Delete(0, 1);
								} else {
									// backup and let the next character go into the next segment
									nStringIndex--;
									break;
								}
							}
						}
						szNext += szTemp;
					}
					if (szNext.GetLength() > 0) {
						pPhonemic->Append(szNext, FALSE, pArray[CHARACTER_OFFSETS][nIndex], pArray[CHARACTER_DURATIONS][nIndex]);
					}
					break;
				}
			}

			szNext = pTable->GetRemainder(nAlignMode, nStringIndex, settings.m_szPhonemic);
			// Skip empty segments
			if (szNext.GetLength() != 0) {
				pPhonemic->Append(szNext, FALSE, pArray[CHARACTER_OFFSETS][nOffsetSize - 1], pArray[CHARACTER_DURATIONS][nOffsetSize - 1]);
			}
			// SDM 1.06.8 apply input filter to segment
			pPhonemic->Filter();
		}

		// Process tone
		if (settings.m_nSegmentBy != IDC_KEEP) {
			// SDM 1.06.8 only change  if new segmentation or text changed
			nStringIndex = 0;
			nGlossIndex = 0;
			nWordIndex = 0;
			CToneSegment * pTone = (CToneSegment *)GetSegment(TONE);
			pTone->DeleteContents();
		}

		// Process orthographic
		// SDM 1.06.8 only change if new segmentation or text changed
		if ((settings.m_bOrthographic) && ((settings.m_nSegmentBy != IDC_KEEP) || (settings.m_bOrthographicModified))) {

			nStringIndex = 0;
			nGlossIndex = 0;
			nWordIndex = 0;
			COrthographicSegment * pOrtho = (COrthographicSegment *)GetSegment(ORTHO);
			pTable = GetFont(ORTHO);
			pOrtho->DeleteContents(); // Delete contents and reinsert from scratch

			nOffsetSize = pArray[CHARACTER_OFFSETS].GetSize();
			for (nIndex = 0; nIndex < (nOffsetSize - 1); nIndex++) {
				switch (settings.m_nAlignBy) {
				case IDC_NONE:
					szNext = pTable->GetNext(nAlignMode, nStringIndex, settings.m_szOrthographic);
					if (szNext.GetLength() != 0) {
						// Skip Empty Segments
						pOrtho->Append(szNext, FALSE, pArray[CHARACTER_OFFSETS][nIndex], pArray[CHARACTER_DURATIONS][nIndex]);
					}
					break;
				case IDC_WORD:
					if (nGlossIndex >= pArray[WORD_OFFSETS].GetSize()) {
						// No more word breaks continue one character at a time
						szNext = pTable->GetNext(nAlignMode, nStringIndex, settings.m_szOrthographic);
					} else if (pArray[CHARACTER_OFFSETS][nIndex] < pArray[WORD_OFFSETS][nGlossIndex]) {
						// Skip if character offset does not correspond to word boundary
						continue;
					} else {
						// Insert Word on Boundary
						szNext = pTable->GetNext(nAlignMode, nStringIndex, settings.m_szOrthographic);
						nGlossIndex++;  // Increment word index
					}
					if (szNext.GetLength() == 0) {
						// Skip NULL words
						continue;
					}
					pOrtho->Append(szNext, FALSE, pArray[CHARACTER_OFFSETS][nIndex], pArray[CHARACTER_DURATIONS][nIndex]);
					break;
				case IDC_CHARACTER:
					// the line is entered one character per segment
					szNext.Empty();
					while (true) {
						CSaString szTemp = pTable->GetNext(nAlignMode, nStringIndex, settings.m_szOrthographic);
						if (szTemp.GetLength() == 0) {
							break;
						} else if ((szTemp.GetLength() == 1) && (szTemp[0] == SPACE_DELIMITER)) {
							// time to stop!
							break;
						} else if (szTemp.GetLength() > 1) {
							// in some situations if the trailing character is not a break
							// it will be combined with the space.  we will break it here.
							if (szTemp[0] == SPACE_DELIMITER) {
								if (szNext.GetLength() == 0) {
									// remove space and append
									szTemp.Delete(0, 1);
								} else {
									// backup and let the next character go into the next segment
									nStringIndex--;
									break;
								}
							}
						}
						szNext += szTemp;
					}
					if (szNext.GetLength() > 0) {
						pOrtho->Append(szNext, FALSE, pArray[CHARACTER_OFFSETS][nIndex], pArray[CHARACTER_DURATIONS][nIndex]);
					}
					break;
				}
			}

			szNext = pTable->GetRemainder(nAlignMode, nStringIndex, settings.m_szOrthographic);
			if (szNext.GetLength() != 0) {
				// Skip empty segments
				pOrtho->Append(szNext, FALSE, pArray[CHARACTER_OFFSETS][nOffsetSize - 1], pArray[CHARACTER_DURATIONS][nOffsetSize - 1]);
			}

			// SDM 1.06.8 apply input filter to segment
			pOrtho->Filter();
		}

		// Process gloss
		// SDM 1.5Test8.2 only change if text changed
		if (settings.m_bGlossModified) {
			nStringIndex = 0;
			pTable = GetFont(GLOSS);

			bool poundDelimited = (settings.m_szGloss.FindOneOf(CSaString(WORD_DELIMITER)) != -1);

			// align gloss by word SDM 1.5Test8.2
			nAlignMode = CFontTable::DELIMITEDWORD;
			nOffsetSize = pArray[WORD_OFFSETS].GetSize();
			// Don't Select this segment SDM 1.5Test8.2
			SelectSegment(pGloss, -1);
			// the gloss table uses a space as a delimiter,
			// the normally the text is delimited with a #.
			// if we see a # in the first position, we will continue
			// to scan the segments util we
			for (nIndex = 0; nIndex < (nOffsetSize - 1); nIndex++) {

				szNext = pTable->GetNext(nAlignMode, nStringIndex, settings.m_szGloss);
				if (szNext.GetLength() == 0) {
					szNext = CSaString(WORD_DELIMITER);
				} else {
					if (szNext[0] == WORD_DELIMITER) {
						// do nothing
					} else {
						szNext = CSaString(WORD_DELIMITER) + szNext;
					}
				}

				// if the user used # delimiters, then if there are
				// embedded spaces, concatenate the lines
				if (poundDelimited) {
					int nTempIndex = nStringIndex;
					bool done = false;
					while (!done) {
						int nLastIndex = nTempIndex;
						CSaString szTemp = pTable->GetNext(nAlignMode, nTempIndex, settings.m_szGloss);
						if (szTemp.GetLength() == 0) {
							if (nTempIndex == nLastIndex) {
								// we are at the end of the data
								done = true;
							} else {
								szNext += CSaString(SPACE_DELIMITER);
							}
						} else {
							if (szTemp[0] == WORD_DELIMITER) {
								// we found the next line
								nStringIndex = nLastIndex;
								done = true;
							} else {
								szNext += CSaString(SPACE_DELIMITER) + szTemp;
							}
						}
					}
				}

				szNext.Remove(0x0d);
				szNext.Remove(0x0a);
				pView->ToggleSegmentSelection(pGloss, nIndex);
				pGloss->ReplaceSelectedSegment(this, szNext, false);
			};
		}
	}

	pView->ChangeAnnotationSelection(pPhonetic, -1);

	CGraphWnd * pGraph = pView->GraphIDtoPtr(IDD_RAWDATA);
	if (pGraph != NULL) {
		if (settings.m_bPhonetic) {
			pGraph->ShowAnnotation(PHONETIC, TRUE, TRUE);
		}
		if (settings.m_bPhonemic) {
			pGraph->ShowAnnotation(PHONEMIC, TRUE, TRUE);
		}
		if (settings.m_bOrthographic) {
			pGraph->ShowAnnotation(ORTHO, TRUE, TRUE);
		}
		if (settings.m_bGloss) {
			pGraph->ShowAnnotation(GLOSS, TRUE, TRUE);
		}
		if (settings.m_bGlossNat) {
			pGraph->ShowAnnotation(GLOSS_NAT, TRUE, TRUE);
		}
	}
	// redraw all graphs without legend window
	pView->RedrawGraphs();
}

void CSaDoc::AlignTranscriptionDataByRef(CTranscriptionData & td) {

	// save state for undo ability
	CheckPoint();
	SetModifiedFlag(TRUE); // document has been modified
	SetTransModifiedFlag(TRUE); // transcription has been modified

	POSITION pos = GetFirstViewPosition();
	CSaView * pView = (CSaView *)GetNextView(pos);

	// cycle through reference and assign the other fields
	CReferenceSegment * pReference = (CReferenceSegment *)GetSegment(REFERENCE);
	CGlossSegment * pGloss = GetGlossSegment();
	CGlossNatSegment * pGlossNat = GetGlossNatSegment();
	CPhoneticSegment * pPhonetic = (CPhoneticSegment *)GetSegment(PHONETIC);
	CPhonemicSegment * pPhonemic = (CPhonemicSegment *)GetSegment(PHONEMIC);
	COrthographicSegment * pOrthographic = (COrthographicSegment *)GetSegment(ORTHO);

	if (pReference->GetOffsetSize() == 0) {
		AfxMessageBox(IDS_ERROR_TAT_NO_REFERENCE, MB_OK | MB_ICONEXCLAMATION, 0);
		return;
	}

	for (int i = 0; i < pReference->GetOffsetSize(); i++) {
		CSaString thisRef = pReference->GetText(i);
		DWORD start = pReference->GetOffset(i);
		DWORD duration = pReference->GetDuration(i);
		MarkerList::iterator git = td.m_TranscriptionData[td.m_MarkerDefs[GLOSS]].begin();
		MarkerList::iterator gnit = td.m_TranscriptionData[td.m_MarkerDefs[GLOSS_NAT]].begin();
		MarkerList::iterator pmit = td.m_TranscriptionData[td.m_MarkerDefs[PHONEMIC]].begin();
		MarkerList::iterator pnit = td.m_TranscriptionData[td.m_MarkerDefs[PHONETIC]].begin();
		MarkerList::iterator oit = td.m_TranscriptionData[td.m_MarkerDefs[ORTHO]].begin();

		bool glossValid = git != td.m_TranscriptionData[td.m_MarkerDefs[GLOSS]].end();
		bool glossNatValid = gnit != td.m_TranscriptionData[td.m_MarkerDefs[GLOSS_NAT]].end();
		bool phonemicValid = pmit != td.m_TranscriptionData[td.m_MarkerDefs[PHONEMIC]].end();
		bool phoneticValid = pnit != td.m_TranscriptionData[td.m_MarkerDefs[PHONETIC]].end();
		bool orthoValid = oit != td.m_TranscriptionData[td.m_MarkerDefs[ORTHO]].end();

		MarkerList refs = td.m_TranscriptionData[td.m_szPrimary];
		for (MarkerList::iterator it = refs.begin(); it != refs.end(); it++) {
			CSaString thatRef = *it;
			if (thisRef.Compare(thatRef) == 0) {
				if ((td.m_bPhonetic) && (phoneticValid)) {
					CSaString text = *pnit;
					pPhonetic->SetAt(text, start, duration);
				}
				if ((td.m_bPhonemic) && (phonemicValid)) {
					CSaString text = *pmit;
					pPhonemic->SetAt(text, start, duration);
				}
				if ((td.m_bOrthographic) && (orthoValid)) {
					CSaString text = *oit;
					pOrthographic->SetAt(text, start, duration);
				}
				if ((td.m_bGloss) && (glossValid)) {
					if (git != td.m_TranscriptionData[td.m_MarkerDefs[GLOSS]].end()) {
						CSaString text = *git;
						if (text[0] == WORD_DELIMITER) {
							text = text.Mid(1);
						}
						pGloss->SetAt(text, false, start, duration, true);
					}
				}
				if ((td.m_bGlossNat) && (glossNatValid)) {
					CSaString text = *gnit;
					if (pGlossNat->FindOffset(start) >= 0) {
						pGlossNat->SetAt(text, start, duration);
					} else if (pGlossNat->IsEmpty()) {
						pGlossNat->Insert(0, text, false, start, duration);
					} else {
						int ro = pReference->FindOffset(start);
						pGlossNat->Insert(ro, text, false, start, duration);
					}
				}
			}
			if ((td.m_bPhonetic) && (phoneticValid)) {
				pnit++;
			}
			if ((td.m_bPhonemic) && (phonemicValid)) {
				pmit++;
			}
			if ((td.m_bOrthographic) && (orthoValid)) {
				oit++;
			}
			if ((td.m_bGloss) && (glossValid)) {
				git++;
			}
			if ((td.m_bGlossNat) && (glossNatValid)) {
				gnit++;
			}
		}
	}

	CGraphWnd * pGraph = pView->GraphIDtoPtr(IDD_RAWDATA);
	if (pGraph) {
		pGraph->ShowAnnotation(REFERENCE, TRUE, TRUE);
		if (td.m_bPhonetic) {
			pGraph->ShowAnnotation(PHONETIC, TRUE, TRUE);
		}
		if (td.m_bPhonemic) {
			pGraph->ShowAnnotation(PHONEMIC, TRUE, TRUE);
		}
		if (td.m_bOrthographic) {
			pGraph->ShowAnnotation(ORTHO, TRUE, TRUE);
		}
		if (td.m_bGloss) {
			pGraph->ShowAnnotation(GLOSS, TRUE, TRUE);
		}
		if (td.m_bGlossNat) {
			pGraph->ShowAnnotation(GLOSS_NAT, TRUE, TRUE);
		}
	}
	// redraw all graphs without legend window
	pView->RedrawGraphs();
}

/***************************************************************************/
// CSaDoc::BuildString builds an annotation string
/***************************************************************************/
const CSaString CSaDoc::BuildString(int nSegment) {

	CSaString szBuild = "";
	CSaString szWorking = "";
	CSaString szSegmentDefault = SEGMENT_DEFAULT_CHAR;

	CSegment * pSegment = GetSegment(nSegment);
	if (pSegment == NULL) {
		return szBuild;
	}
	if (pSegment->IsEmpty()) {
		return szBuild;
	}

	const char wordDelimiter = GetFont(nSegment)->m_wordDelimiter;

	int nIndex = 0;

	switch (nSegment) {
	case PHONETIC:
		while (nIndex != -1) {
			szWorking = pSegment->GetSegmentString(nIndex);
			if (szWorking != szSegmentDefault) {
				szBuild += szWorking;
				szBuild += wordDelimiter;
			}
			nIndex = pSegment->GetNext(nIndex);
		}
		break;

	case PHONEMIC:
	case ORTHO:
		while (nIndex != -1) {
			szWorking = pSegment->GetSegmentString(nIndex);
			szBuild += szWorking;
			szBuild += wordDelimiter;
			nIndex = pSegment->GetNext(nIndex);
		}
		break;

	case GLOSS:
		while (nIndex != -1) {
			szWorking = pSegment->GetSegmentString(nIndex);
			szBuild += szWorking;
			szBuild += wordDelimiter;
			nIndex = pSegment->GetNext(nIndex);
		}
		break;

	case GLOSS_NAT:
		while (nIndex != -1) {
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

/**
* Read the incoming stream and return the transcription line
* This is used by the automatic transcription feature returns false on failure
*/
const bool CSaDoc::ImportTranscription(wistringstream & stream, BOOL gloss, BOOL glossNat, BOOL phonetic, BOOL phonemic, BOOL orthographic, CTranscriptionData & td, bool addTag, bool showDlg) {

	// rewind the stream
	stream.clear();
	stream.seekg(0);
	stream.clear();

	td.m_MarkerDefs[REFERENCE] = psz_Reference;
	td.m_szPrimary = psz_Reference;

	// setup the default list
	td.m_Markers.push_back(psz_Reference);
	if (phonetic) {
		td.m_MarkerDefs[PHONETIC] = psz_Phonetic;
		td.m_Markers.push_back(psz_Phonetic);
		td.m_bPhonetic = true;
	}
	if (phonemic) {
		td.m_MarkerDefs[PHONEMIC] = psz_Phonemic;
		td.m_Markers.push_back(psz_Phonemic);
		td.m_bPhonemic = true;
	}
	if (orthographic) {
		td.m_MarkerDefs[ORTHO] = psz_Orthographic;
		td.m_Markers.push_back(psz_Orthographic);
		td.m_bOrthographic = true;
	}
	if (gloss) {
		td.m_MarkerDefs[GLOSS] = psz_Gloss;
		td.m_Markers.push_back(psz_Gloss);
		td.m_bGloss = true;
	}
	if (glossNat) {
		td.m_MarkerDefs[GLOSS_NAT] = psz_GlossNat;
		td.m_Markers.push_back(psz_GlossNat);
		td.m_bGlossNat = true;
	}

	if (CSFMHelper::IsSFM(stream)) {
		if (showDlg) {
			CDlgImportSFMRef dlg(phonetic, phonemic, orthographic, gloss, glossNat);
			int result = dlg.DoModal();
			if (result == IDCANCEL) {
				return true;
			}

			if (result == IDC_IMPORT_PLAIN_TEXT) {
				return CTextHelper::ImportText(stream, td.m_szPrimary, td.m_Markers, td.m_TranscriptionData, addTag);
			}

			// proceeding as SFM
			// order is important here!
			td.m_Markers.clear();
			td.m_MarkerDefs[REFERENCE] = dlg.m_szReference;
			td.m_Markers.push_back(dlg.m_szReference);

			if (dlg.m_bPhonetic) {
				td.m_MarkerDefs[PHONETIC] = dlg.m_szPhonetic;
				td.m_Markers.push_back(dlg.m_szPhonetic);
				td.m_bPhonetic = true;
			}
			if (dlg.m_bPhonemic) {
				td.m_MarkerDefs[PHONEMIC] = dlg.m_szPhonemic;
				td.m_Markers.push_back(dlg.m_szPhonemic);
				td.m_bPhonemic = true;
			}
			if (dlg.m_bOrthographic) {
				td.m_MarkerDefs[ORTHO] = dlg.m_szOrthographic;
				td.m_Markers.push_back(dlg.m_szOrthographic);
				td.m_bOrthographic = true;
			}
			if (dlg.m_bGloss) {
				td.m_MarkerDefs[GLOSS] = dlg.m_szGloss;
				td.m_Markers.push_back(dlg.m_szGloss);
				td.m_bGloss = true;
			}
			if (dlg.m_bGlossNat) {
				td.m_MarkerDefs[GLOSS_NAT] = dlg.m_szGlossNat;
				td.m_Markers.push_back(dlg.m_szGlossNat);
				td.m_bGlossNat = true;
			}
		}

		if (CSFMHelper::IsMultiRecordSFM(stream, td.m_MarkerDefs[REFERENCE])) {
			td.m_TranscriptionData = CSFMHelper::ImportMultiRecordSFM(stream, td.m_MarkerDefs[REFERENCE], td.m_Markers, addTag);
			return true;
		}
		if (CSFMHelper::IsColumnarSFM(stream)) {
			td.m_TranscriptionData = CSFMHelper::ImportColumnarSFM(stream);
			return true;
		}

		//map = CSFMHelper::ImportSFM( stream);
		return false;
	}

	// proceeding as text
	return CTextHelper::ImportText(stream, td.m_szPrimary, td.m_Markers, td.m_TranscriptionData, addTag);
}

int CSaDoc::GetSegmentSize(EAnnotation nIndex) {
	return m_apSegments[nIndex]->GetOffsetSize();
}

CSegment * CSaDoc::GetSegment(int nIndex) {
	return m_apSegments[nIndex];
}

CSegment * CSaDoc::GetSegment(EAnnotation nIndex) {
	return m_apSegments[nIndex];
}

CGlossSegment * CSaDoc::GetGlossSegment() {
	return (CGlossSegment *)m_apSegments[GLOSS];
}

CGlossNatSegment * CSaDoc::GetGlossNatSegment() {
	return (CGlossNatSegment *)m_apSegments[GLOSS_NAT];
}

CSaString CSaDoc::GetMusicScore() {
	return m_szMusicScore;
}
BOOL CSaDoc::IsBackgroundProcessing() {
	return m_bProcessBackground;
}
BOOL CSaDoc::EnableBackgroundProcessing(BOOL bState) {
	BOOL result = m_bProcessBackground;
	m_bProcessBackground = bState;
	return result;
}
CProcessDoc * CSaDoc::GetUnprocessed() {
	return m_pProcessDoc.get();
}
CProcessAdjust * CSaDoc::GetAdjust() {
	return m_pProcessAdjust;
}
CProcessFragments * CSaDoc::GetFragments() {
	return m_pProcessFragments;
}
CProcessLoudness * CSaDoc::GetLoudness() {
	return m_pProcessLoudness;
}
CProcessSmoothLoudness * CSaDoc::GetSmoothLoudness() {
	return m_pProcessSmoothLoudness;
}
CProcessZCross * CSaDoc::GetZCross() {
	return m_pProcessZCross;
}
CProcessPitch * CSaDoc::GetPitch() {
	return m_pProcessPitch;
}
CProcessCustomPitch * CSaDoc::GetCustomPitch() {
	return m_pProcessCustomPitch;
}
CProcessSmoothedPitch * CSaDoc::GetSmoothedPitch() {
	return m_pProcessSmoothedPitch;
}
CProcessChange * CSaDoc::GetChange() {
	return m_pProcessChange;
}
CProcessRaw * CSaDoc::GetRaw() {
	return m_pProcessRaw;
}
CProcessHilbert * CSaDoc::GetHilbert() {
	return m_pProcessHilbert;
}
CProcessWavelet * CSaDoc::GetWavelet() {
	return m_pProcessWavelet;
}
CProcessSpectrum * CSaDoc::GetSpectrum() {
	return m_pProcessSpectrum;
}
CProcessGrappl * CSaDoc::GetGrappl() {
	return m_pProcessGrappl;
}
CProcessMelogram * CSaDoc::GetMelogram() {
	return m_pProcessMelogram;
}
CProcessFormants * CSaDoc::GetFormants() {
	return m_pProcessFormants;
}
CProcessFormantTracker * CSaDoc::GetFormantTracker() {
	return m_pProcessFormantTracker;
}
CProcessDurations * CSaDoc::GetDurations() {
	return m_pProcessDurations;
}
CProcessRatio * CSaDoc::GetRatio() {
	return m_pProcessRatio;
}
CProcessPOA * CSaDoc::GetPOA() {
	return m_pProcessPOA;
}
CProcessGlottis * CSaDoc::GetGlottalWave() {
	return m_pProcessGlottis;
}
CProcessTonalWeightChart * CSaDoc::GetTonalWeightChart() {
	return m_pProcessTonalWeightChart;
}
CFontTable * CSaDoc::GetFont(int nIndex) {
	return (CFontTable *)m_pCreatedFonts->GetAt(nIndex);
}

void CSaDoc::SetAudioModifiedFlag(bool bMod) {
	m_bAudioModified = bMod;
}

bool CSaDoc::IsAudioModified() const {
	return m_bAudioModified;
}

void CSaDoc::SetTransModifiedFlag(bool bMod) {
	if (m_AutoSave.IsSaving()) {
		return;
	}
	m_bTransModified = bMod;
}

bool CSaDoc::IsTransModified() const {
	return m_bTransModified;
}

void CSaDoc::SetID(int nID) {
	m_ID = nID;
}

int CSaDoc::GetID() {
	return m_ID;
}

void CSaDoc::SetWbProcess(int nProcess) {
	m_nWbProcess = nProcess;
}

CFileStatus * CSaDoc::GetFileStatus() {
	return &m_fileStat;
}

CSaView * CSaDoc::GetFirstView() const {
	POSITION pos = GetFirstViewPosition();
	return (CSaView*)GetNextView(pos);
}

SourceParm * CSaDoc::GetSourceParm() {
	return &m_sourceParm;
}

const CPitchParm * CSaDoc::GetPitchParm() const {
	return &m_pitchParm;
}

void CSaDoc::SetPitchParm(const CPitchParm & parm) {
	m_pitchParm = parm;
}

const CMusicParm * CSaDoc::GetMusicParm() const {
	return &m_musicParm;
}

void CSaDoc::SetMusicParm(const CMusicParm & parm) {
	m_musicParm = parm;
}

const CIntensityParm & CSaDoc::GetCIntensityParm() const {
	return m_intensityParm;
}

void CSaDoc::SetCIntensityParm(const CIntensityParm & parm) {
	m_intensityParm = parm;
}

SDPParm * CSaDoc::GetSDPParm() {
	return &m_SDPParm;
}

/***************************************************************************/
// CSaDoc::CopySectionToNewWavFile Copies wave data out of the wave file
// and places in a new wav file
// dwSectionStart the start index in bytes (single channel)
// dwSectionLength the length in bytes (single channel)
// if the function fails, it will delete the target wave file
/***************************************************************************/
bool CSaDoc::CopySectionToNewWavFile(WAVETIME sectionStart, WAVETIME sectionLength, LPCTSTR szNewWave, bool usingClipboard) {

	CString originalWave;
	if (m_bUsingTempFile) {
		originalWave = m_szTempConvertedWave.c_str();
	} else if ((GetPathName().GetLength() != 0)) {
		originalWave = GetPathName();
	} else if (m_szTempWave.GetLength() != 0) {
		originalWave = m_szTempWave;
	}

	if (originalWave.GetLength() == 0) {
		//Original not found
		FileUtils::Remove(szNewWave);
		return false;
	}

	// Copy wanted portion of the wave data file
	bool bSameFileName = (originalWave.CompareNoCase(szNewWave) == 0);
	if (bSameFileName) {

		wstring tempNewTemp = FileUtils::GetTempFileName(_T("TMP"));
		if (!CopyWave(m_szRawDataWrk.c_str(), tempNewTemp.c_str(), sectionStart, sectionLength, TRUE)) {
			FileUtils::Remove(tempNewTemp.c_str());
			FileUtils::Remove(szNewWave);
			return false;
		}

		// Set segments to selected wave
		// adjust segments to new file size
		WAVETIME length = toTime(GetDataSize(), true);
		segmentOps.ShrinkSegments(sectionStart + sectionLength, length - (sectionStart + sectionLength));
		segmentOps.ShrinkSegments(0, sectionStart);

		// Set document to use new wave data
		m_dwDataSize = ToBytes(sectionLength, false);

		// Done with this file
		FileUtils::Remove(m_szRawDataWrk.c_str());

		m_szRawDataWrk = tempNewTemp.c_str();

		OnFileSave();
		InvalidateAllProcesses();

		POSITION pos = GetFirstViewPosition();
		CSaView * pView = ((CSaView *)GetNextView(pos));

		pView->SetStartStopCursorPosition(0, GetDataSize());
		pView->RedrawGraphs();

		while (CanUndo()) {
			// remove undo list
			Undo(FALSE, FALSE);
		}
		return true;
	}

	// NOT the same file
	if (!CopyWave(originalWave, szNewWave)) {
		FileUtils::Remove(szNewWave);
		return false;
	}

	// write the current data to a backup.
	// we will restore it later
	wstring backupTemp = FileUtils::GetTempFileName(_T("TMP"));
	if (!CopyWave(m_szRawDataWrk.c_str(), backupTemp.c_str(), sectionStart, sectionLength, TRUE)) {
		FileUtils::Remove(szNewWave);
		FileUtils::Remove(backupTemp.c_str());
		return false;
	}

	// Save segment data we will use this documents segments for calculations
	// save file state for Undo below
	CheckPoint();

	TRACE("GetDataSize()=%lu\n", GetDataSize());
	TRACE("sectionLength=%f\n", sectionLength);
	// Set segments to selected wave
	// adjust segments to new file size
	WAVETIME length = toTime(GetDataSize(), true);
	TRACE("length = %f\n", length);
	segmentOps.ShrinkSegments(sectionStart + sectionLength, length - (sectionStart + sectionLength));
	segmentOps.ShrinkSegments(0, sectionStart);

	// backup key document parameters
	wstring szTempName = m_szRawDataWrk;
	DWORD dwDataSize = m_dwDataSize;
	WORD wFlags = m_saParam.wFlags;

	// Set document to use new wave data
	m_dwDataSize = ToBytes(sectionLength, true);
	TRACE("m_dwDataSize = %lu\n", m_dwDataSize);

	// Create Wave file
	try {
		m_szRawDataWrk = backupTemp.c_str();
		if (!WriteDataFiles(szNewWave, true, usingClipboard)) {
			m_szRawDataWrk = szTempName;
			m_dwDataSize = dwDataSize;
			FileUtils::Remove(szNewWave);
			FileUtils::Remove(backupTemp.c_str());
			Undo(FALSE);
			return false;
		}
		//Restore Document
		m_saParam.wFlags = wFlags;
		m_dwDataSize = dwDataSize;
		m_szRawDataWrk = szTempName;
		// Done with this file
		FileUtils::Remove(backupTemp.c_str());
		// return segments to original state
		Undo(FALSE);
	} catch (const CException &) {
		m_szRawDataWrk = szTempName;
		m_dwDataSize = dwDataSize;
		FileUtils::Remove(szNewWave);
		FileUtils::Remove(backupTemp.c_str());
		Undo(FALSE);
		return false;
	}

	// save path for copying transcriptions
	((CSaApp *)AfxGetApp())->SetLastClipboardPath(szNewWave);
	return true;
}

/**
* Extracts the left or right channel from the audio data
* the left channel is stored at 0 in wave audio data
* the right channel begins at 0+(blockAlign/channels) in the wave audio data
*
* @param wasStereo true if the original data is stereo
* @param left true if we should extract the left channel, false to extract the right channel
* @param filename the source (and target) file to modify
* @return false on failure
*/
bool CSaDoc::ConvertToMono(bool extractLeft, LPCTSTR filename) {

	// do the work in a temporary file
	TCHAR tempfilename[_MAX_PATH];
	FileUtils::GetTempFileName(_T("MONO"), tempfilename, _countof(tempfilename));
	if (!CopyWave(filename, tempfilename)) {
		return false;
	}

	WORD bitsPerSample = 0;
	WORD formatTag = 0;
	WORD channels = 0;
	DWORD samplesPerSec = 0;
	WORD blockAlign = 0;

	try {
		//TODO handle memory during exceptions
		vector<char> buffer;

		try {
			CWaveReader reader;
			reader.Read(filename, MMIO_ALLOCBUF | MMIO_READ, bitsPerSample, formatTag, channels, samplesPerSec, blockAlign, buffer);
		} catch (wave_error e) {
			CSaApp & app = *(CSaApp *)AfxGetApp();
			app.ErrorMessage(IDS_ERROR_CANT_READ_WAVE_FILE, filename);
			return false;
		}

		// is there anything for us to do?
		if (channels == 1) {
			return true;
		}

		WORD channel = extractLeft ? 0 : 1;

		vector<char> newBuffer;
		ExtractChannel(channel, channels, blockAlign, buffer, newBuffer);

		WORD newChannels = 1;
		CWaveWriter writer;
		writer.Write(tempfilename, MMIO_CREATE | MMIO_WRITE, bitsPerSample, formatTag, newChannels, samplesPerSec, newBuffer);

		// rename the new file
		FileUtils::Rename(tempfilename, filename);

		return true;
	} catch (logic_error e) {
		AfxMessageBox(L"logic_error occurred!", MB_OK | MB_ICONEXCLAMATION, 0);
		return false;
	} catch (...) {
		AfxMessageBox(L"Unexpected exception!", MB_OK | MB_ICONEXCLAMATION, 0);
		return false;
	}
}

void CSaDoc::DoExportFieldWorks(CExportFWSettings & settings) {

	int dataCount = 0;
	int sfmCount = 0;
	int wavCount = 0;

	wstring filename;
	TCHAR szBuffer[MAX_PATH];
	wcscpy_s(szBuffer, MAX_PATH, settings.szPath);
	int result = GetSaveAsFilename(settings.szDocTitle, _T("Standard Format (*.sfm) |*.sfm||"), _T("sfm"), szBuffer, filename);
	if (result != IDOK) {
		return;
	}

	if (filename.length() == 0) {
		ErrorMessage(IDS_ERROR_NO_FW_FILE);
		return;
	}

	bool skipEmptyGloss = true;

	TCHAR szPath[MAX_PATH];
	memset(szPath, 0, MAX_PATH);
	wcscpy_s(szPath, MAX_PATH, settings.szPath);

	if (!FileUtils::FolderExists(szPath)) {
		FileUtils::CreateFolder(szPath);
	}
	wcscat_s(szPath, MAX_PATH, L"LinkedFiles\\");
	if (!FileUtils::FolderExists(szPath)) {
		FileUtils::CreateFolder(szPath);
	}
	wcscat_s(szPath, MAX_PATH, L"AudioVisual\\");

	if (!FileUtils::FolderExists(szPath)) {
		FileUtils::CreateFolder(szPath);
	}

	if (!ValidateWordFilenames(WFC_REF_GLOSS, skipEmptyGloss, L"", L"")) {
		return;
	}
	if (!ValidatePhraseFilenames(MUSIC_PL1, PFC_REF_GLOSS, L"", L"")) {
		return;
	}
	if (!ValidatePhraseFilenames(MUSIC_PL2, PFC_REF_GLOSS, L"", L"")) {
		return;
	}

	CFile file(filename.c_str(), CFile::modeCreate | CFile::modeWrite);
	CSaString szString;

	if (!TryExportSegmentsBy(settings, REFERENCE, file, skipEmptyGloss, szPath, dataCount, wavCount)) {
		if (!TryExportSegmentsBy(settings, GLOSS, file, skipEmptyGloss, szPath, dataCount, wavCount)) {
			if (!TryExportSegmentsBy(settings, GLOSS_NAT, file, skipEmptyGloss, szPath, dataCount, wavCount)) {
				if (!TryExportSegmentsBy(settings, ORTHO, file, skipEmptyGloss, szPath, dataCount, wavCount)) {
					if (!TryExportSegmentsBy(settings, PHONEMIC, file, skipEmptyGloss, szPath, dataCount, wavCount)) {
						if (!TryExportSegmentsBy(settings, TONE, file, skipEmptyGloss, szPath, dataCount, wavCount)) {
							TryExportSegmentsBy(settings, PHONETIC, file, skipEmptyGloss, szPath, dataCount, wavCount);
						}
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
	dlg.m_WAVCount.Format(L"%d", wavCount);
	dlg.m_DataCount.Format(L"%d", dataCount);
	dlg.m_SFMCount.Format(L"%d", sfmCount);
	dlg.DoModal();

}

void CSaDoc::DoExportLift(CExportLiftSettings & settings) {

	int dataCount = 0;
	int liftCount = 0;
	int wavCount = 0;

	wstring filename;
	TCHAR szBuffer[MAX_PATH];
	wcscpy_s(szBuffer, MAX_PATH, settings.szPath);
	if (!FileUtils::FolderExists(szBuffer)) {
		FileUtils::CreateFolder(szBuffer);
	}
	//wcscat_s(szBuffer,MAX_PATH,L"LinkedFiles\\");
	//if (!FileUtils::FolderExists(szBuffer))
	//{
	//FileUtils::CreateFolder(szBuffer);
	//}
	int result = GetSaveAsFilename(settings.szDocTitle, _T("Lift Format (*.lift) |*.lift||"), _T("lift"), szBuffer, filename);
	if (result != IDOK) {
		return;
	}

	if (filename.length() == 0) {
		ErrorMessage(IDS_ERROR_NO_FW_FILE);
		return;
	}

	bool skipEmptyGloss = true;

	// extract the path from the returned filename in the case that they relocated it.
	wchar_t buffer[MAX_PATH];
	swprintf_s(buffer, _countof(buffer), filename.c_str());
	wchar_t drive[_MAX_DRIVE];
	wchar_t dir[_MAX_DIR];
	wchar_t fname[_MAX_FNAME];
	wchar_t ext[_MAX_EXT];
	_wsplitpath_s(buffer, drive, dir, fname, ext);

	TCHAR szPath[MAX_PATH];
	memset(szPath, 0, MAX_PATH);
	wcscpy_s(szPath, MAX_PATH, dir);
	if (!FileUtils::FolderExists(szPath)) {
		FileUtils::CreateFolder(szPath);
	}
	wcscat_s(szPath, MAX_PATH, L"audio\\");
	if (!FileUtils::FolderExists(szPath)) {
		FileUtils::CreateFolder(szPath);
	}

	Lift13::field_defn field(L"field");
	field.tag = wstring(L"Reference");
	field.form.append(Lift13::form(L"form", L"en", Lift13::text(LTEXT, Lift13::span(SPAN, L""))));
	field.form.append(Lift13::form(L"form", L"qaa-x-spec", Lift13::text(LTEXT, Lift13::span(SPAN, L"Class=LexEntry; Type=String; WsSelector=kwsAnal"))));

	Lift13::field_defns fields(L"fields");
	fields.field.append(field);

	Lift13::header header(L"header");
	header.fields = fields;

	Lift13::lift document(L"Speech Analyzer 3.1.1.2");
	document.header = header;

	ExportSegments(settings, document, skipEmptyGloss, szPath, dataCount, wavCount);

	bool pass = false;
	try {
		Lift13::store(document, filename.c_str());
		pass = true;
	} catch (DOMException & e) {
		TRACE(L"store failed : %s", e.getMessage());
	} catch (exception & e) {
		TRACE("store failed : %s", e.what());
	} catch (...) {
		TRACE(L"store failed : unexpected exception");
	}

	if (!pass) {
		ErrorMessage(IDS_ERROR_FAILED_TO_STORE_LIFT);
		return;
	}

	liftCount++;

	CDlgExportLiftResult dlg;
	dlg.m_WAVCount.Format(L"%d", wavCount);
	dlg.m_DataCount.Format(L"%d", dataCount);
	dlg.m_SFMCount.Format(L"%d", liftCount);
	dlg.DoModal();

}

bool CSaDoc::ExportSegments(CExportLiftSettings & settings,
	Lift13::lift & document,
	bool skipEmptyGloss,
	LPCTSTR szPath,
	int & dataCount,
	int & wavCount) {

	EWordFilenameConvention wordConvention = WFC_REF_GLOSS;
	EPhraseFilenameConvention phraseConvention = PFC_REF_GLOSS;

	if (!GetFlag(REFERENCE, settings)) {
		return false;
	}

	CSegment * pSeg = GetSegment(REFERENCE);
	if (pSeg->GetOffsetSize() == 0) {
		return false;
	}

	CSaString results[ANNOT_WND_NUMBER];
	for (int i = 0; i < ANNOT_WND_NUMBER; i++) {
		results[i] = L"";
	}
	DWORD last = pSeg->GetOffset(0) - 1;
	for (int i = 0; i < pSeg->GetOffsetSize(); i++) {
		DWORD dwStart = pSeg->GetOffset(i);
		DWORD dwStop = pSeg->GetStop(i);
		if (dwStart == last) {
			continue;
		}
		last = dwStart;
		for (int j = MUSIC_PL2; j >= 0; j--) {
			EAnnotation target = ConvertToAnnotation(j);
			if (!GetFlag(target, settings)) {
				continue;
			}
			CSegment * pSegment = GetSegment(target);
			CSaString szText = pSegment->GetContainedText(dwStart, dwStop);
			szText = szText.Trim();
			if (szText.GetLength() == 0) {
				results[target] = L"";
				continue;
			}
			if (target == GLOSS) {
				if (szText[0] == WORD_DELIMITER) {
					szText = szText.Right(szText.GetLength() - 1);
				}
			}
			results[target] = szText;
		}

		Lift13::entry entry(L"entry");
		entry.date_created = Lift::createDate();
		entry.date_modified = Lift::createDate();

		if (results[REFERENCE].GetLength() > 0) {

			POSITION pos = GetFirstViewPosition();
			CSaView * pView = (CSaView *)GetNextView(pos);
			CGlossSegment * gloss = (CGlossSegment *)pView->GetAnnotation(GLOSS);
			CMusicPhraseSegment * pl1 = (CMusicPhraseSegment *)pView->GetAnnotation(MUSIC_PL1);
			CMusicPhraseSegment * pl2 = (CMusicPhraseSegment *)pView->GetAnnotation(MUSIC_PL2);

			entry.id = Lift::createUUID();
			entry.guid = entry.id;

			// build the pronunciation for phonetic
			entry.pronunciation = Lift13::phonetic(L"pronunciation");

			// add phonetic media file
			if (settings.bGloss) {
				wstring filename;
				int index = FindNearestGlossIndex(gloss, dwStart, dwStop);
				if (index >= 0) {
					int result = ComposeWordSegmentFilename(gloss, index, wordConvention, szPath, filename, L"", L"");
					if (result == 0) {
						result = ExportWordSegment(gloss, index, filename.c_str(), skipEmptyGloss, dataCount, wavCount);
						if (result == 0) {
							size_t index = filename.find_last_of('\\');
							wstring uri = filename.substr(index + 1);
							entry.pronunciation[0].media.append(Lift13::urlref(L"media"));
							entry.pronunciation[0].media[0].href = uri;
							entry.pronunciation[0].media[0].label = Lift13::multitext(L"label");
							Lift13::span s(SPAN, filename.c_str());
							s.href.set(filename.c_str());
							s.clazz.set(L"Hyperlink");
							Lift13::text t(L"text", s);
							Lift13::form f(L"form", settings.gloss.c_str(), t);
							entry.pronunciation[0].media[0].label.get().form.append(f);
						}
					}
				}
			}

			// add PL1 media file
			if ((results[MUSIC_PL1].GetLength() > 0) && settings.bPhrase1) {
				wstring filename;
				int index = FindNearestPhraseIndex(pl1, dwStart, dwStop);
				if (index >= 0) {
					int result = ComposePhraseSegmentFilename(MUSIC_PL1, pl1, index, phraseConvention, szPath, filename, L"", L"");
					if (result == 0) {
						result = ExportPhraseSegment(pl1, index, filename, dataCount, wavCount);
						if (result == 0) {
							size_t index = filename.find_last_of('\\');
							wstring uri = filename.substr(index + 1);
							entry.pronunciation[0].media.append(Lift13::urlref(L"media"));
							entry.pronunciation[0].media[1].href = uri;
							entry.pronunciation[0].media[0].label = Lift13::multitext(L"label");
							Lift13::span s(SPAN, filename.c_str());
							Lift13::text t(L"text", s);
							Lift13::form f(L"form", settings.phrase1.c_str(), t);
							entry.pronunciation[0].media[0].label.get().form.append(f);
						}
					}
				}
			}

			// add PL2 media file
			if ((results[MUSIC_PL2].GetLength() > 0) && settings.bPhrase2) {
				wstring filename;
				int index = FindNearestPhraseIndex(pl2, dwStart, dwStop);
				if (index >= 0) {
					int result = ComposePhraseSegmentFilename(MUSIC_PL2, pl2, index, phraseConvention, szPath, filename, L"", L"");
					if (result == 0) {
						int result2 = ExportPhraseSegment(pl2, index, filename, dataCount, wavCount);
						if (result2 == 0) {
							size_t index = filename.find_last_of('\\');
							wstring uri = filename.substr(index + 1);
							entry.pronunciation[0].media.append(Lift13::urlref(L"media"));
							entry.pronunciation[0].media[2].href = uri;
							entry.pronunciation[0].media[0].label = Lift13::multitext(L"label");
							Lift13::span s(SPAN, filename.c_str());
							Lift13::text t(L"text", s);
							Lift13::form f(L"form", settings.phrase2.c_str(), t);
							entry.pronunciation[0].media[0].label.get().form.append(f);
						}
					}
				}
			}

			// build the lexical unit
			entry.lexical_unit = Lift13::multitext(L"lexical-unit");
			if (settings.bOrtho) {
				entry.lexical_unit.get().form.append(Lift13::form(L"form", settings.ortho.c_str(), Lift13::text(LTEXT, Lift13::span(SPAN, results[ORTHO]))));
			}

			// Build the IPA language tags from Ortho if available, fall back to "und" (BCP-47 for undefined)

			// add the phonetic
			if (settings.bPhonetic) {
				wstring phonetic;
				if (settings.bOrtho) {
					phonetic.append(settings.ortho.c_str());
				} else {
					phonetic.append(L"und");
				}
				AppendFonipaTag(phonetic, L"etic");

				entry.lexical_unit.get().form.append(Lift13::form(L"form", phonetic.c_str(), Lift13::text(LTEXT, Lift13::span(SPAN, results[PHONETIC]))));
			}

			// add the phonemic
			if (settings.bPhonemic) {
				wstring phonemic;
				if (settings.bOrtho) {
					phonemic.append(settings.ortho.c_str());
				}	else {
					phonemic.append(L"und");
				}
				AppendFonipaTag(phonemic, L"emic");

				entry.lexical_unit.get().form.append(Lift13::form(L"form", phonemic.c_str(), Lift13::text(LTEXT, Lift13::span(SPAN, results[PHONEMIC]))));
			}

			// build the field
			if (settings.bReference) {
				entry.field = Lift13::field(L"field", L"Reference");
				entry.field[0].form = Lift13::form(L"form", settings.reference.c_str(), Lift13::text(LTEXT, Lift13::span(SPAN, results[REFERENCE])));
			}

			// build the sense field
			if (settings.bGloss) {
				entry.sense = Lift13::sense(L"sense", createUUID().c_str(), i);
				entry.sense[0].gloss = Lift13::gloss(L"gloss", settings.gloss.c_str(), Lift13::span(SPAN, results[GLOSS]));
			}
		}
		document.entry.append(entry);
	}
	return true;
}

/**
* Merges the "fonipa-x-" variant to the language tag.
* @param str - The language tag
* @param privateUse - private use variant of "etic" or "emic"
*/
void CSaDoc::AppendFonipaTag(wstring & str, wstring privateUse) {
	// Determine if str already contains private use variant
	int index = str.find(L"-x-");
	if (index > 0) {
		str.insert(index + 3, privateUse + L"-");
		str.insert(index, L"-fonipa");
	} else {
		str.append(L"-fonipa-x-" + privateUse);
	}
}

bool CSaDoc::TryExportSegmentsBy(CExportFWSettings & settings,
	EAnnotation master,
	CFile & file,
	bool skipEmptyGloss,
	LPCTSTR szPath,
	int & dataCount,
	int & wavCount) {

	TRACE("EXPORTING>>>>%d\n", master);

	LPCTSTR szCrLf = L"\r\n";

	EWordFilenameConvention wordConvention = WFC_REF_GLOSS;
	EPhraseFilenameConvention phraseConvention = PFC_REF_GLOSS;

	if (!GetFlag(master, settings)) {
		return false;
	}

	CSegment * pSeg = GetSegment(master);

	if (pSeg->GetOffsetSize() == 0) {
		return false;
	}

	WriteFileUtf8(&file, szCrLf);

	CSaString results[ANNOT_WND_NUMBER];
	for (int i = 0; i < ANNOT_WND_NUMBER; i++) {
		results[i] = L"";
	}
	DWORD last = pSeg->GetOffset(0) - 1;
	for (int i = 0; i < pSeg->GetOffsetSize(); i++) {
		DWORD dwStart = pSeg->GetOffset(i);
		DWORD dwStop = pSeg->GetStop(i);
		if (dwStart == last) {
			continue;
		}
		last = dwStart;
		for (int j = master; j >= 0; j--) {
			EAnnotation target = ConvertToAnnotation(j);
			if (!GetFlag(target, settings)) {
				continue;
			}
			results[target] = BuildRecord(target, dwStart, dwStop);
		}

		if (results[PHONETIC].GetLength() > 0) {
			WriteFileUtf8(&file, results[PHONETIC]);
		}
		if (results[PHONEMIC].GetLength() > 0) {
			WriteFileUtf8(&file, results[PHONEMIC]);
		}
		if (results[ORTHO].GetLength() > 0) {
			WriteFileUtf8(&file, results[ORTHO]);
		}
		if (results[TONE].GetLength() > 0) {
			WriteFileUtf8(&file, results[TONE]);
		}
		if (results[GLOSS].GetLength() > 0) {
			WriteFileUtf8(&file, results[GLOSS]);
		}
		if (results[GLOSS_NAT].GetLength() > 0) {
			WriteFileUtf8(&file, results[GLOSS_NAT]);
		}
		if (results[REFERENCE].GetLength() > 0) {
			WriteFileUtf8(&file, results[REFERENCE]);
		}

		POSITION pos = GetFirstViewPosition();
		CSaView * pView = (CSaView *)GetNextView(pos);  // get pointer to view
		CGlossSegment * g = (CGlossSegment *)pView->GetAnnotation(GLOSS);
		CMusicPhraseSegment * pl1 = (CMusicPhraseSegment *)pView->GetAnnotation(MUSIC_PL1);
		CMusicPhraseSegment * pl2 = (CMusicPhraseSegment *)pView->GetAnnotation(MUSIC_PL2);
		DWORD offsetSize = g->GetOffsetSize();
		bool hasGloss = (offsetSize != 0);

		TRACE("gloss %d %d\n", dwStart, dwStop);

		if ((hasGloss) || (!skipEmptyGloss)) {

			wstring filename;
			int index = FindNearestGlossIndex(g, dwStart, dwStop);
			if (index >= 0) {
				int result = ComposeWordSegmentFilename(g, index, wordConvention, szPath, filename, L"", L"");
				if (result == 0) {
					if (ExportWordSegment(g, index, filename.c_str(), skipEmptyGloss, dataCount, wavCount) < 0) {
						return false;
					}
					TCHAR szBuffer[MAX_PATH];
					wmemset(szBuffer, 0, MAX_PATH);
					wcscat_s(szBuffer, MAX_PATH, L"\\pf ");
					wcscat_s(szBuffer, MAX_PATH, filename.c_str());
					wcscat_s(szBuffer, MAX_PATH, szCrLf);
					WriteFileUtf8(&file, szBuffer);
				}
			}

			if (settings.bPhrase) {
				TRACE("--searching for PL1\n");
				index = FindNearestPhraseIndex(pl1, dwStart, dwStop);
				if (index >= 0) {
					TRACE("--exporting PL1\n");
					int result = ComposePhraseSegmentFilename(MUSIC_PL1, pl1, index, phraseConvention, szPath, filename, L"", L"");
					if (result == 0) {
						if (ExportPhraseSegment(pl1, index, filename, dataCount, wavCount) < 0) {
							return false;
						}
						TCHAR szBuffer[MAX_PATH];
						wmemset(szBuffer, 0, MAX_PATH);
						wcscat_s(szBuffer, MAX_PATH, L"\\pf ");
						wcscat_s(szBuffer, MAX_PATH, filename.c_str());
						wcscat_s(szBuffer, MAX_PATH, szCrLf);
						WriteFileUtf8(&file, szBuffer);
					}
				}

				TRACE("--searching for PL2\n");
				index = FindNearestPhraseIndex(pl2, dwStart, dwStop);
				if (index >= 0) {
					TRACE("--exporting PL2\n");
					int result = ComposePhraseSegmentFilename(MUSIC_PL2, pl2, index, phraseConvention, szPath, filename, L"", L"");
					if (result == 0) {
						if (ExportPhraseSegment(pl2, index, filename, dataCount, wavCount) < 0) {
							return false;
						}
						TCHAR szBuffer[MAX_PATH];
						wmemset(szBuffer, 0, MAX_PATH);
						wcscat_s(szBuffer, MAX_PATH, L"\\pf ");
						wcscat_s(szBuffer, MAX_PATH, filename.c_str());
						wcscat_s(szBuffer, MAX_PATH, szCrLf);
						WriteFileUtf8(&file, szBuffer);
					}
				}
			}
		}

		WriteFileUtf8(&file, szCrLf);
	}

	return true;
}

CSaString CSaDoc::BuildRecord(EAnnotation target, DWORD dwStart, DWORD dwStop) {

	LPCTSTR szCrLf = L"\r\n";
	CSaString szTag = GetTag(target);
	CSegment * pSegment = GetSegment(target);
	CSaString szText = pSegment->GetContainedText(dwStart, dwStop);
	szText = szText.Trim();
	if (szText.GetLength() == 0) {
		return L"";
	}
	if (target == GLOSS) {
		if (szText[0] == WORD_DELIMITER) {
			szText = szText.Right(szText.GetLength() - 1);
		}
	}
	return szTag + L" " + szText + szCrLf;
}

BOOL CSaDoc::GetFlag(EAnnotation val, CExportFWSettings & settings) {
	switch (val) {
	case PHONETIC:
		return settings.bPhonetic;
	case PHONEMIC:
		return settings.bPhonemic;
	case ORTHO:
		return settings.bOrtho;
	case TONE:
		return settings.bTone;
	case GLOSS:
		return settings.bGloss;
	case GLOSS_NAT:
		return settings.bGlossNat;
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

BOOL CSaDoc::GetFlag(EAnnotation val, CExportLiftSettings & settings) {
	switch (val) {
	case PHONETIC:
		return settings.bPhonetic;
	case PHONEMIC:
		return settings.bPhonemic;
	case ORTHO:
		return settings.bOrtho;
	case GLOSS:
		return settings.bGloss;
	case GLOSS_NAT:
		return settings.bGlossNat;
	case REFERENCE:
		return settings.bReference;
	case MUSIC_PL1:
		return settings.bPhrase1;
	case MUSIC_PL2:
		return settings.bPhrase2;
	case MUSIC_PL3:
		return settings.bPhrase3;
	case MUSIC_PL4:
		return settings.bPhrase4;
	}
	return false;
}

int CSaDoc::GetIndex(EAnnotation val) {
	switch (val) {
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
	case GLOSS_NAT:
		return 5;
	case REFERENCE:
		return 6;
	case MUSIC_PL1:
		return 7;
	case MUSIC_PL2:
		return 8;
	case MUSIC_PL3:
		return 9;
	case MUSIC_PL4:
		return 10;
	}
	return false;
}

LPCTSTR CSaDoc::GetTag(EAnnotation val) {
	switch (val) {
	case PHONETIC:
		return L"\\lx-ph";
	case PHONEMIC:
		return L"\\lx-pm";
	case ORTHO:
		return L"\\lx-or";
	case TONE:
		return L"\\tn";
	case GLOSS:
		return L"\\ge";
	case GLOSS_NAT:
		return L"\\gn";
	case REFERENCE:
		return L"\\rf";
	case MUSIC_PL1:
		return L"\\pf";
	case MUSIC_PL2:
		return L"\\pf";
	case MUSIC_PL3:
		return L"\\pf";
	case MUSIC_PL4:
		return L"\\pf";
	}
	return L"";
}

EAnnotation CSaDoc::ConvertToAnnotation(int val) {
	switch (val) {
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
		return GLOSS_NAT;
	case 6:
		return REFERENCE;
	case 7:
		return MUSIC_PL1;
	case 8:
		return MUSIC_PL2;
	case 9:
		return MUSIC_PL3;
	case 10:
		return MUSIC_PL4;
	}
	return PHONETIC;
}

void CSaDoc::WriteFileUtf8(CFile * pFile, const CSaString szString) {
	std::string szUtf8 = szString.utf8();
	pFile->Write(szUtf8.c_str(), szUtf8.size());
}

bool CSaDoc::HasSegmentData(EAnnotation val) {
	return (m_apSegments[val]->GetOffsetSize() > 0) ? true : false;
}

LPCTSTR CSaDoc::GetProcessFilename() {
	return m_szRawDataWrk.c_str();
}

CDocTemplate * CSaDoc::GetTemplate() {
	return m_pDocTemplate;
}

const CUttParm * CSaDoc::GetUttParm() {
	// pointer to utterance parameters structure
	return &m_uttParm;
}

BOOL CSaDoc::IsWaveToUndo() {
	// return TRUE, if wave file change is to undo
	return m_bWaveUndoNow;
}

BPTR CSaDoc::GetUnprocessedWaveData(DWORD dwOffset, BOOL bBlockBegin) {
	return m_pProcessDoc->GetProcessedWaveData(GetProcessFilename(), GetSelectedChannel(), GetNumChannels(), GetSampleSize(), dwOffset, bBlockBegin);
}

DWORD CSaDoc::GetUnprocessedWaveDataBufferSize() {
	return m_pProcessDoc->GetBufferSize();
}

void * CSaDoc::GetUnprocessedDataBlock(DWORD dwByteOffset, size_t sObjectSize, BOOL bReverse) {
	return m_pProcessDoc->GetProcessedDataBlock(GetProcessFilename(), GetSelectedChannel(), GetNumChannels(), GetSampleSize(), dwByteOffset, sObjectSize, bReverse);
}

DWORD CSaDoc::GetUnprocessedBufferIndex(size_t nSize) {
	return m_pProcessDoc->GetProcessBufferIndex(nSize);
}

DWORD CSaDoc::GetSelectedChannel() {
	return m_nSelectedChannel;
}

// get the sample size in bytes for a single channel
DWORD CSaDoc::GetSampleSize() const {
	return m_FmtParm.GetSampleSize();
}

WAVETIME CSaDoc::GetTimeFromBytes(DWORD dwSize) {
	// return sampled data size in seconds
	WAVETIME result = dwSize;
	result /= (((double)m_FmtParm.dwAvgBytesPerSec) / ((double)m_FmtParm.wChannels));
	return result;
}

DWORD CSaDoc::GetBytesFromTime(WAVETIME fSize) {
	// return sampled data size in bytes
	return (DWORD)(((double)fSize)*(((double)m_FmtParm.dwAvgBytesPerSec) / ((double)m_FmtParm.wChannels)));
}

/**
* return the number of bytes used.
* @param singleChannel if true return bytes used for a single channel
*/
DWORD CSaDoc::GetBytesPerSample(bool singleChannel) {
	if (singleChannel) {
		DWORD result = m_FmtParm.wBlockAlign;
		result /= m_FmtParm.wChannels;
		return result;
	}
	return m_FmtParm.wBlockAlign;
}

DWORD CSaDoc::GetSamplesPerSec() {
	return m_FmtParm.dwSamplesPerSec;
}

WORD CSaDoc::GetBitsPerSample() {
	return m_FmtParm.wBitsPerSample;
}

WORD CSaDoc::GetBlockAlign(bool singleChannel) {
	if (singleChannel) {
		return m_FmtParm.wBlockAlign / m_FmtParm.wChannels;
	}
	return m_FmtParm.wBlockAlign;
}

bool CSaDoc::Is16Bit() {
	return ((m_FmtParm.wBlockAlign / m_FmtParm.wChannels) == 2);
}

bool CSaDoc::IsPCM() {
	return (m_FmtParm.wTag == FILE_FORMAT_PCM);
}

DWORD CSaDoc::GetAvgBytesPerSec() {
	return m_FmtParm.dwAvgBytesPerSec;
}

DWORD CSaDoc::GetNumChannels() const {
	return m_FmtParm.wChannels;
}

DWORD CSaDoc::GetNumSamples() const {
	return (m_dwDataSize / m_FmtParm.GetSampleSize()) / m_FmtParm.wChannels;
}

CSaString CSaDoc::GetTempFilename() {
	return m_szTempWave;
}

bool CSaDoc::IsUsingTempFile() {
	return (!m_szTempWave.IsEmpty());
}

void CSaDoc::StoreAutoRecoveryInformation() {
	m_AutoSave.Save(*this);
}

wstring CSaDoc::GetFilenameFromTitle() {
	// load file name
	wstring result = GetTitle();
	size_t nFind = result.find(':');
	if (nFind != wstring::npos) {
		// extract part left of :
		result = result.substr(0, nFind - 1);
	}
	return result;
}

wstring CSaDoc::GetTranscriptionFilename() {
	// a prerecorded file
	wstring result = GetPathName();
	result = result.substr(0, result.length() - 4);
	result.append(L".saxml");
	return result;
}

/*
* convert a position - in bytes into time.
*/
WAVETIME CSaDoc::toTime(CURSORPOS bytes, bool singleChannel) {
	WAVETIME result = (WAVETIME)bytes;
	result /= (WAVETIME)GetBytesPerSample(singleChannel);
	result /= (WAVETIME)GetSamplesPerSec();
	return result;
}

WAVETIME CSaDoc::toTimeFromSamples(WAVESAMP samples) {
	WAVETIME result = (WAVETIME)samples;
	result /= (WAVETIME)GetSamplesPerSec();
	return result;
}

/**
* convert cursor position ( a single channel) into time
*/
WAVETIME CSaDoc::toTime(CURSORPOS val) {
	WAVETIME result = (WAVETIME)val;
	result /= (WAVETIME)GetBytesPerSample(true);
	result /= (WAVETIME)GetSamplesPerSec();
	return result;
}

DWORD CSaDoc::ToBytes(WAVETIME val, bool singleChannel) {
	val *= GetSamplesPerSec();
	// round up to nearest sample
	DWORD dwResult = ::ceil(val);
	dwResult *= GetBytesPerSample(singleChannel);
	return dwResult;
}

WAVESAMP CSaDoc::toSamples(WAVETIME val) {
	val *= GetSamplesPerSec();
	WAVESAMP dwResult = ::ceil(val);
	return (WAVESAMP)dwResult;
}

CURSORPOS CSaDoc::toCursor(WAVETIME val) {
	val *= GetSamplesPerSec();
	// round up to nearest sample
	CURSORPOS dwResult = ::ceil(val);
	dwResult *= GetBytesPerSample(true);
	return (CURSORPOS)dwResult;
}

CURSORPOS CSaDoc::toCursor(WAVESAMP val) {
	val *= GetBytesPerSample(true);
	return (CURSORPOS)val;
}

bool CSaDoc::IsUsingHighPassFilter() {
	return (m_saParam.wFlags & SA_FLAG_HIGHPASS);
}

void CSaDoc::DisableHighPassFilter() {
	m_saParam.wFlags &= ~SA_FLAG_HIGHPASS;
}

CString CSaDoc::GetDescription() {
	return m_saParam.szDescription.c_str();
}

void CSaDoc::SetDescription(LPCTSTR val) {
	m_saParam.szDescription = _to_utf8(val);
}

bool CSaDoc::MatchesDescription(LPCTSTR val) {
	return (compare_no_case(_to_wstring(m_saParam.szDescription).c_str(),val));
}

bool CSaDoc::IsValidRecordFileFormat() {
	return (m_saParam.byRecordFileFormat <= FILE_FORMAT_TIMIT);
}

int CSaDoc::GetRecordFileFormat() {
	return m_saParam.byRecordFileFormat;
}

DWORD CSaDoc::GetRecordBandWidth() {
	return m_saParam.dwRecordBandWidth;
}

BYTE CSaDoc::GetRecordSampleSize() {
	return m_saParam.byRecordSmpSize;
}

BYTE CSaDoc::GetQuantization() {
	return m_saParam.byQuantization;
}

void CSaDoc::SetQuantization(BYTE val) {
	m_saParam.byQuantization = val;
}

DWORD CSaDoc::GetSignalBandWidth() {
	return m_saParam.dwSignalBandWidth;
}

void CSaDoc::SetSignalBandWidth(DWORD val) {
	m_saParam.dwSignalBandWidth = val;
	if (IsUsingHighPassFilter()) {
		m_saParam.dwSignalBandWidth -= 70;
	}
}

void CSaDoc::ClearHighPassFilter() {
	m_saParam.wFlags &= ~SA_FLAG_HIGHPASS;
}

void CSaDoc::SetHighPassFilter() {
	m_saParam.wFlags |= SA_FLAG_HIGHPASS;
}

DWORD CSaDoc::GetNumberOfSamples() {
	return m_saParam.dwNumberOfSamples;
}

void CSaDoc::SetNumberOfSamples(DWORD val) {
	m_saParam.dwNumberOfSamples = val;
}

void CSaDoc::SetRecordSampleSize(BYTE val) {
	m_saParam.byRecordSmpSize = val;
}

void CSaDoc::SetRecordBandWidth(DWORD val) {
	m_saParam.dwRecordBandWidth = val;
	if (IsUsingHighPassFilter()) {
		m_saParam.dwRecordBandWidth -= 70;
	}
}

void CSaDoc::SetRecordTimeStamp(CTime & val) {
	m_saParam.RecordTimeStamp = val;
}

int CSaDoc::GetLastSegmentBeforePosition(int annotSetID, DWORD cursorPos) {
	CSegment * pSeg = GetSegment(annotSetID);
	int selection = -1;
	for (int i = 0; i < pSeg->GetOffsetSize(); i++) {
		DWORD offset = pSeg->GetOffset(i);
		DWORD duration = pSeg->GetDuration(i);
		if ((offset + duration) < cursorPos) {
			selection = i;
		} else {
			break;
		}
	}
	return selection;
}

int CSaDoc::GetSaveAsFilename(LPCTSTR title, LPCTSTR filter, LPCTSTR extension, LPTSTR path, wstring & filename) {

	filename = L"";
	wstring title2 = title;
	int nFind = title2.find(':');
	if (nFind != -1) {
		title2 = title2.substr(0, nFind);
		title2 = FileUtils::Trim(title2);
	}
	nFind = title2.rfind('.');

	// remove extension
	title2 = FileUtils::Trim(title2);
	if (nFind >= ((title2.size() > 3) ? (title2.size() - 4) : 0)) {
		title2 = title2.substr(0, nFind);
	}

	CFileDialog dlg(FALSE, extension, title2.c_str(), OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, filter, NULL);

	if ((title2.size() > 0) && (path != NULL)) {
		TCHAR temp[MAX_PATH];
		wmemset(temp, 0, MAX_PATH);
		wcscat_s(temp, MAX_PATH, path);
		FileUtils::AppendDirSep(temp, MAX_PATH);
		wcscat_s(temp, MAX_PATH, title2.c_str());
		wcscat_s(temp, MAX_PATH, L".");
		wcscat_s(temp, MAX_PATH, extension);
		dlg.m_ofn.lpstrFile = temp;
		dlg.m_ofn.lpstrInitialDir = path;
	}

	int result = dlg.DoModal();
	if (result == IDOK) {
		// return the dialog result
		filename = dlg.GetPathName();
	}
	return result;
}

void CSaDoc::MergeSegments(CPhoneticSegment * pPhonetic, int sel) {

	if (sel == -1) {
		return;
	}
	// find the end of the prev segment
	int prev = pPhonetic->GetPrevious(sel);
	if (prev == -1) {
		return;
	}
	bool segmental = IsSegmental(pPhonetic, sel);

	bool modified = false;

	DWORD thisOffset = pPhonetic->GetOffset(sel);
	DWORD prevOffset = pPhonetic->GetOffset(prev);
	DWORD thisStop = pPhonetic->GetStop(sel);

	// remove this segment
	for (int n = 0; n < ANNOT_WND_NUMBER; n++) {
		CSegment * pSeg = m_apSegments[n];
		if (n == PHONETIC) {
			modified |= pSeg->Merge(thisOffset, prevOffset, thisStop);
		} else if ((pSeg->GetMasterIndex() == PHONETIC) && (!pSeg->Is(GLOSS))) {
			modified |= pSeg->Merge(thisOffset, prevOffset, thisStop);
		} else if (!segmental) {
			modified |= pSeg->Merge(thisOffset, prevOffset, thisStop);
		}
	}

	SetModifiedFlag(((modified) ? TRUE : FALSE) | IsModified());
	SetTransModifiedFlag(modified || IsTransModified());
}

/**
* determine if the transcriptions are segmental -
* segmental means multiple phonetic segments per gloss
* AudioSync works on non-segmental data
*/
bool CSaDoc::IsSegmental(CPhoneticSegment * pPhonetic, int sel) {

	CGlossSegment * pGloss = (CGlossSegment *)GetSegment(GLOSS);
	if (pGloss->IsEmpty()) {
		return false;
	}

	int gsel = pGloss->FindFromPosition(pPhonetic->GetOffset(sel));
	if (gsel == -1) {
		return false;
	}

	DWORD start = pGloss->GetOffset(gsel);
	DWORD stop = pGloss->GetStop(gsel);
	//TRACE("gsel=%d start=%d stop=%d\n",gsel,start,stop);

	size_t count = 0;
	DWORD lastoffset = 0;
	for (size_t i = 0; i < pPhonetic->GetOffsetSize(); i++) {
		DWORD offset = pPhonetic->GetOffset(i);
		if (offset >= stop) {
			break;
		}
		if (lastoffset == offset) {
			continue;
		}
		lastoffset = offset;
		if (offset < start) {
			continue;
		}
		//TRACE("index=%d offset=%d\n",i,offset);
		count++;
	}
	return (count > 1);
}

// determine if we are at a segment boundary.
// The phonetic offset would match a gloss offset
bool CSaDoc::IsBoundary(CPhoneticSegment * pPhonetic, int sel) {

	CGlossSegment * pGloss = (CGlossSegment *)GetSegment(GLOSS);
	if (pGloss->IsEmpty()) {
		return false;
	}
	DWORD thisOffset = pPhonetic->GetOffset(sel);
	int gsel = pGloss->FindOffset(thisOffset);
	return (gsel != -1) ? true : false;
}

void CSaDoc::SelectSegment(CSegment * pSegment, int index) {
	POSITION pos = GetFirstViewPosition();
	CSaView * pView = (CSaView *)GetNextView(pos);
	pView->ToggleSegmentSelection(pSegment, index);
}

/**
* find and load in the .SAB file mapping
*/
void CSaDoc::UpdateReferenceBuffer() {
	return;
}

typedef map<EAnnotation, CString> SABMap;

void CSaDoc::MoveDataLeft(DWORD offset) {

	bool modified = false;

	// non-sab case
	for (int n = 0; n < ANNOT_WND_NUMBER; n++) {
		CSegment * pSeg = m_apSegments[n];
		modified |= pSeg->MoveDataLeft(offset);
	}

	SetModifiedFlag(((modified) ? TRUE : FALSE) | IsModified());
	SetTransModifiedFlag(modified || IsTransModified());
}

void CSaDoc::MoveDataRight(DWORD offset) {

	bool modified = false;
	for (int n = 0; n < ANNOT_WND_NUMBER; n++) {
		CSegment * pSeg = m_apSegments[n];
		modified |= pSeg->MoveDataRight(offset, false);
	}
	SetModifiedFlag(((modified) ? TRUE : FALSE) | IsModified());
	SetTransModifiedFlag(modified || IsTransModified());
}

/**
* calculate the index at which we should insert a default segment
* if the segment is empty, we can just append
* if the segment is populated, and has an entry, return -1
* if the segment is populated and does not have a matching entry
* if the segment is populated and the offset exceeds it, return
* the last index for appending
*/
int CSaDoc::GetInsertionIndex(CSegment * pSegment, DWORD offset) {
	if (pSegment->GetOffsetSize() == 0) {
		return 0;
	}
	for (int i = 0; i < pSegment->GetOffsetSize(); i++) {
		DWORD thisOffset = pSegment->GetOffset(i);
		if (offset == thisOffset) {
			return -1;
		}
		if (offset < thisOffset) {
			return i;
		}
	}
	return pSegment->GetOffsetSize();
}

void CSaDoc::NormalizePhoneticDependencies() {

	CPhoneticSegment * pPhonetic = (CPhoneticSegment *)GetSegment(PHONETIC);
	CPhonemicSegment * pPhonemic = (CPhonemicSegment *)GetSegment(PHONEMIC);
	COrthographicSegment * pOrtho = (COrthographicSegment *)GetSegment(ORTHO);
	CToneSegment * pTone = (CToneSegment *)GetSegment(TONE);

	for (int i = 0; i < pPhonetic->GetOffsetSize(); i++) {
		DWORD offset = pPhonetic->GetOffset(i);
		DWORD duration = pPhonetic->GetDuration(i);
		// add blank segments for the locations that don't have them
		int j = GetInsertionIndex(pPhonemic, offset);
		if (j != -1) {
			pPhonemic->InsertAt(j, L"", offset, duration);
		}
		j = GetInsertionIndex(pOrtho, offset);
		if (j != -1) {
			pOrtho->InsertAt(j, L"", offset, duration);
		}
		j = GetInsertionIndex(pTone, offset);
		if (j != -1) {
			pTone->InsertAt(j, L"", offset, duration);
		}
	}
}

void CSaDoc::DeselectAll() {
	// deselect everything
	for (int nLoop = 0; nLoop < ANNOT_WND_NUMBER; nLoop++) {
		if (m_apSegments[nLoop] != NULL) {
			m_apSegments[nLoop]->SetSelection(-1);
		}
	}
}

/**
* Normal method called during 'import SAB menu item
*/
void CSaDoc::ImportSAB(CSaView & /*view*/, LPCTSTR filename, int /*algorithm*/) {

	CheckPoint();

	// create the backup SAB file
	// look for and copy the .sab file
	// if it's the same, just leave the sab file alone
	wstring oldFile;
	if (!IsTempWaveEmpty()) {
		oldFile = GetTempWave();
	} else {
		oldFile = GetPathName();
	}
	wstring to = FileUtils::ReplaceExtension(oldFile.c_str(), L".sab");
	FileUtils::Copy(filename, to.c_str());

	// verify ref and gloss segments are empty
	CReferenceSegment * pRef = (CReferenceSegment*)GetSegment(REFERENCE);
	CGlossSegment * pGloss = (CGlossSegment*)GetSegment(GLOSS);
	CGlossNatSegment * pGlossNat = (CGlossNatSegment*)GetSegment(GLOSS_NAT);

	bool hasRef = pRef->GetOffsetSize() > 0;
	bool hasGloss = pGloss->GetOffsetSize() > 0;
	bool hasGlossNat = pGlossNat->GetOffsetSize() > 0;

	bool hasSegments = hasRef || hasGloss || hasGlossNat;

	bool hasData = false;
	for (int i = 0; i < pRef->GetOffsetSize(); i++) {
		CString text = pRef->GetText(i).Trim();
		if (text.GetLength() == 0) continue;
		hasData = true;
		break;
	}
	for (int i = 0; i < pGloss->GetOffsetSize(); i++) {
		CString text = pGloss->GetText(i).Trim();
		if (text.GetLength() == 0) continue;
		if ((text.GetLength() == 1) && (text[0] == '#')) continue;
		hasData = true;
		break;
	}
	for (int i = 0; i < pGlossNat->GetOffsetSize(); i++) {
		CString text = pGlossNat->GetText(i).Trim();
		if (text.GetLength() == 0) continue;
		if ((text.GetLength() == 1) && (text[0] == '#')) continue;
		hasData = true;
		break;
	}

	if ((hasSegments) && (hasData)) {
		int nResponse = AfxMessageBox(IDS_SAB_QUERY_EXISTING_DATA, MB_YESNO | MB_ICONEXCLAMATION, 0);
		if (nResponse != IDYES) {
			return;
		}
	}

	bool autoSegment = false;
	if (hasSegments) {
		int nResponse = AfxMessageBox(IDS_SAB_QUERY_AUTOSEGMENT_EXISTING, MB_YESNOCANCEL | MB_ICONQUESTION, 0);
		if (nResponse == IDCANCEL) {
			return;
		} else if (nResponse == IDNO) {
			autoSegment = true;
		}
	} else {
		int nResponse = AfxMessageBox(IDS_SAB_QUERY_AUTOSEGMENT, MB_YESNO | MB_ICONQUESTION, 0);
		if (nResponse != IDYES) {
			return;
		}
		autoSegment = true;
	}

	CMainFrame * pMainFrame = (CMainFrame*)AfxGetMainWnd();
	ImportSAB(filename, autoSegment, true, 0, pMainFrame->GetAudioSyncAlgorithm());
}

/**
* Called during AS startup
*/
void CSaDoc::ImportSAB(LPCTSTR filename, bool autoSegment, bool loadData, int skipCount, int algorithm) {

	if ((!autoSegment) && (!loadData)) return;

	CheckPoint();
	// create the backup SAB file
	// look for and copy the .sab file
	// if it's the same, just leave the sab file alone
	wstring oldFile;
	if (!IsTempWaveEmpty()) {
		oldFile = GetTempWave();
	} else {
		oldFile = GetPathName();
	}
	wstring to = FileUtils::ReplaceExtension(oldFile.c_str(), L".sab");
	FileUtils::Copy(filename, to.c_str());

	// auto add reference numbers
	CTranscriptionData td;

	CFileEncodingHelper feh(filename);
	if (!feh.CheckEncoding(true)) {
		return;
	}

	wistringstream stream;
	if (!feh.ConvertFileToUTF16(stream)) {
		return;
	}

	if (!ImportTranscription(stream, TRUE, TRUE, FALSE, FALSE, FALSE, td, true, false)) {
		ErrorMessage(IDS_ERROR_SAB_NO_IMPORT);
		return;
	}

	CString ref = td.m_szPrimary;
	MarkerList rf = td.GetMarkerList(REFERENCE);
	MarkerList gl = td.GetMarkerList(GLOSS);
	MarkerList gn = td.GetMarkerList(GLOSS_NAT);
	TranscriptionDataMap & map = td.m_TranscriptionData;
	MarkerList::iterator rit = map[ref].begin();
	MarkerList::iterator rend = map[ref].end();
	MarkerList::iterator glit = gl.begin();
	MarkerList::iterator glend = gl.end();
	MarkerList::iterator gnit = gn.begin();
	MarkerList::iterator gnend = gn.end();

	// is the incoming data gl or gn?
	bool usingGL = false;
	while (glit != glend) {
		CString text = (*glit).Trim();
		glit++;
		if (text.GetLength() == 0) continue;
		if ((text.GetLength() == 1) && ((text[0] == WORD_DELIMITER) || (text[0] == TEXT_DELIMITER))) continue;
		usingGL = true;
		break;
	}

	if (autoSegment) {
		// delete segments for everything
		DeleteSegmentContents(PHONETIC);
		DeleteSegmentContents(PHONEMIC);
		DeleteSegmentContents(ORTHO);
		DeleteSegmentContents(TONE);
		DeleteSegmentContents(GLOSS);
		DeleteSegmentContents(GLOSS_NAT);
		DeleteSegmentContents(REFERENCE);
	}

	// verify ref and gloss segments are empty
	CReferenceSegment * pRef = (CReferenceSegment*)GetSegment(REFERENCE);
	CGlossSegment * pGloss = (CGlossSegment*)GetSegment(GLOSS);
	CGlossNatSegment * pGlossNat = (CGlossNatSegment*)GetSegment(GLOSS_NAT);

	// get reference to view
	POSITION pos = GetFirstViewPosition();
	CSaView & view = (CSaView &)*GetNextView(pos);

	CGraphWnd * pGraph = view.GraphIDtoPtr(IDD_RAWDATA);
	if (pGraph != NULL) {
		pGraph->ShowAnnotation(PHONETIC, TRUE);
		pGraph->ShowAnnotation(REFERENCE, TRUE);
		if (usingGL) {
			pGraph->ShowAnnotation(GLOSS, TRUE);
			pGraph->ShowAnnotation(GLOSS_NAT, FALSE);
		} else {
			pGraph->ShowAnnotation(GLOSS, FALSE);
			pGraph->ShowAnnotation(GLOSS_NAT, TRUE);
		}
		view.ResizeGraphs(TRUE, TRUE);
	}

	if (autoSegment) {

		DWORD goal = (usingGL) ? gl.size() : gn.size();
		goal += skipCount;

		if (!AutoSegment(filename, td, view, goal, algorithm, skipCount, usingGL)) {
			view.ResizeGraphs(TRUE, TRUE);
			return;
		}
	}

	view.ResizeGraphs(TRUE, TRUE);

	if (loadData) {

		CheckPoint();

		// reset for resue
		glit = gl.begin();

		if (usingGL) {
			DWORD limit = pGloss->GetOffsetSize() - 1;
			int i = 0;
			while (glit != glend) {
				// insert gloss
				CString text = (*glit).Trim();
				if (text.GetLength() == 0) {
					text = WORD_DELIMITER;
				} else if ((text[0] != WORD_DELIMITER) && (text[0] != TEXT_DELIMITER)) {
					text.Insert(0, WORD_DELIMITER);
				}
				pGloss->SetText(i, text);
				glit++;
				// insert reference
				if (rit != rend) {
					pRef->SetText(i, *rit);
					rit++;
				}
				i++;
				if (i > limit) break;
			}
		} else {
			// using gloss national
			DWORD limit = pGlossNat->GetOffsetSize() - 1;
			int i = 0;
			while (gnit != gnend) {
				// insert gloss
				CString text = (*gnit).Trim();
				pGlossNat->SetText(i, text);
				gnit++;
				// insert reference
				if (rit != rend) {
					pRef->SetText(i, *rit);
					rit++;
				}
				i++;
				if (i > limit) break;
			}
		}
	}
}

void CSaDoc::AdjustZero() {

	m_pProcessAdjust->SetZero(!m_pProcessAdjust->GetZero());
	InvalidateAllProcesses();
	WorkbenchProcess();
	SetModifiedFlag(TRUE);
	SetAudioModifiedFlag(TRUE);
}

int CSaDoc::GetZero() {
	return m_pProcessAdjust->GetZero();
}

void CSaDoc::AdjustNormalize() {

	m_pProcessAdjust->SetNormalize(!m_pProcessAdjust->GetNormalize());
	InvalidateAllProcesses();
	WorkbenchProcess();
	SetModifiedFlag(TRUE);
	SetAudioModifiedFlag(TRUE);
}

int CSaDoc::GetNormalize() {
	return m_pProcessAdjust->GetNormalize();
}

void CSaDoc::AdjustInvert() {

	m_pProcessAdjust->SetInvert(!m_pProcessAdjust->GetInvert());
	InvalidateAllProcesses();
	WorkbenchProcess();
	SetModifiedFlag(TRUE);
	SetAudioModifiedFlag(TRUE);
}

int CSaDoc::GetInvert() {
	return m_pProcessAdjust->GetInvert();
}

void CSaDoc::ClearSABLoaded() {
	sabLoaded = false;
}

void CSaDoc::ClearTranscriptionCount() {
	m_nTranscriptionApplicationCount = 0;
}

void CSaDoc::RevertAllTranscriptionChanges() {
	while (m_nTranscriptionApplicationCount > 0) {
		RevertTranscriptionChanges();
	}
}

bool CSaDoc::IsTempWaveEmpty() {
	return m_szTempWave.IsEmpty();
}

CString CSaDoc::GetTempWave() {
	return m_szTempWave;
}

int CSaDoc::GetWbProcess() {
	return m_nWbProcess;
}

bool CSaDoc::HasFullName() {
	return IsTempWaveEmpty();
}

void CSaDoc::CloseAutoSave(LPCTSTR file) {
	m_AutoSave.Close(file);
}

void CSaDoc::ExportTimeTable(LPCTSTR filename,
	BOOL bF1,
	BOOL bF2,
	BOOL bF3,
	BOOL bF4,
	BOOL bGloss,
	BOOL bGlossNat,
	CString szIntervalTime,
	BOOL bSegmentLength,
	BOOL bMagnitude,
	BOOL bOrtho,
	BOOL bPhonemic,
	BOOL bPhonetic,
	BOOL bPitch,
	BOOL bReference,
	BOOL bSegmentStart,
	BOOL bSampleTime,
	BOOL bTone,
	BOOL bZeroCrossings,
	int nSampleRate,
	int nCalculationMethod,
	bool bEntire,
	BOOL bMelogram) {

	ASSERT(filename != NULL);
	ASSERT(wcslen(filename) > 0);
	if (wcslen(filename) == 0) {
		return;
	}

	// process all flags
	if (nSampleRate == 0) {
		bSampleTime = FALSE;
	} else {
		bReference = bTone = bPhonemic = bOrtho = bGloss = bGlossNat = FALSE;
		bSegmentStart = bSegmentLength = FALSE;
	}

	CSaString szString;
	CSaString szFTFormantString = "";
	CSaString szCrLf = "\r\n";

	POSITION pos = GetFirstViewPosition();
	// get pointer to view
	CSaView * pView = (CSaView *)GetNextView(pos);
	CSegment * pPhonetic = GetSegment(PHONETIC);

	// no annotations
	if (pPhonetic->IsEmpty()) {
		bReference = bPhonetic = bTone = bPhonemic = bOrtho = bGloss = bGlossNat = FALSE;
		// no segments
		bSegmentStart = bSegmentLength = FALSE;
		if (nSampleRate == 0) {
			nSampleRate = 1;
		}
	}

	DWORD dwOffset = 0;
	DWORD dwStopPosition = 0;
	// entire file
	if (bEntire) {
		dwOffset = 0;
		dwStopPosition = GetDataSize() - GetBlockAlign();
	} else {
		dwOffset = pView->GetStartCursorPosition();
		dwStopPosition = pView->GetStopCursorPosition();
	}

	int nIndex = 0;

	// phonetic sampling
	if (nSampleRate != 1) {
		nIndex = pPhonetic->FindFromPosition(dwStopPosition);
		if ((nIndex != -1) && dwStopPosition < pPhonetic->GetOffset(nIndex)) {
			nIndex = pPhonetic->GetPrevious(nIndex);
		}
		if ((nIndex != -1) && dwStopPosition < pPhonetic->GetStop(nIndex)) {
			dwStopPosition = pPhonetic->GetStop(nIndex);
		}
	}

	nIndex = pPhonetic->FindFromPosition(dwOffset);

	// phonetic sampling
	if (nSampleRate != 1) {
		if (nIndex != -1) {
			dwOffset = pPhonetic->GetOffset(nIndex);
		}
	}

	DWORD dwNext = 0;
	DWORD dwIncrement = 0;

	// interval sampling
	if (nSampleRate == 1) {
		int nInterval = 20;
		if (szIntervalTime.GetLength() != 0) {
			swscanf_s(szIntervalTime, _T("%d"), &nInterval);
		}
		if (nInterval < 1) {
			nInterval = 20;
		}
		dwIncrement = GetBytesFromTime(nInterval / 1000.0);
		if (dwIncrement < 1) {
			dwIncrement++;
		}
		if (Is16Bit()) {
			dwIncrement++;
			dwIncrement &= ~1;
		}
		swprintf_s(szIntervalTime.GetBuffer(20), 20, _T("%d"), nInterval);
		szIntervalTime.ReleaseBuffer();
	}

	short int nResult;
	enum { MAG, PITCH, MELOGRAM, ZCROSS, FMTTRACKER, CALCULATIONS };
	double fSizeFactor[CALCULATIONS] = {};

	if (bMagnitude) {
		// get pointer to loudness object
		CProcessLoudness * pLoudness = (CProcessLoudness *)GetLoudness();
		// process data
		nResult = LOWORD(pLoudness->Process(this));
		if (nResult == PROCESS_ERROR) {
			bMagnitude = FALSE;
		} else if (nResult == PROCESS_CANCELED) {
			return;
		} else {
			fSizeFactor[MAG] = (double)GetDataSize() / (double)(pLoudness->GetDataSize() - 1);
		}
	}
	// formants need pitch info
	if (bPitch || bF1 || bF2 || bF3 || bF4) {
		// SDM 1.5 Test 11.0
		CProcessGrappl * pPitch = GetGrappl();
		// We also want raw and smoothed
		CProcessPitch * pRawPitch = GetPitch();
		CProcessSmoothedPitch * pSmoothedPitch = GetSmoothedPitch();
		// process data
		nResult = LOWORD(pPitch->Process(this));
		if (nResult == PROCESS_ERROR) {
			bPitch = FALSE;
		} else if (nResult == PROCESS_CANCELED) {
			return;
		}
		nResult = LOWORD(pRawPitch->Process(this));
		if (nResult == PROCESS_ERROR) {
			bPitch = bPitch && FALSE;
		} else if (nResult == PROCESS_CANCELED) {
			return;
		}
		nResult = LOWORD(pSmoothedPitch->Process(this));
		if (nResult == PROCESS_ERROR) {
			bPitch = bPitch && FALSE;
		} else if (nResult == PROCESS_CANCELED) {
			return;
		} else {
			fSizeFactor[PITCH] = (double)GetDataSize() / (double)(pPitch->GetDataSize() - 1);
		}
	}
	if (bMelogram) {
		CProcessMelogram * pMelogram = GetMelogram(); // SDM 1.5 Test 11.0
		nResult = LOWORD(pMelogram->Process(this)); // process data
		if (nResult == PROCESS_ERROR) {
			bMelogram = FALSE;
		} else if (nResult == PROCESS_CANCELED) {
			return;
		} else {
			fSizeFactor[MELOGRAM] = (double)GetDataSize() / (double)(pMelogram->GetDataSize() - 1);
		}
	}
	if (bZeroCrossings || bF1 || bF2 || bF3 || bF4) {
		CProcessZCross  * pZCross = GetZCross();
		nResult = LOWORD(pZCross->Process(this)); // process data
		if (nResult == PROCESS_ERROR) {
			bZeroCrossings = FALSE;
		} else if (nResult == PROCESS_CANCELED) {
			return;
		} else {
			fSizeFactor[ZCROSS] = (double)GetDataSize() / (double)(pZCross->GetDataSize() - 1);
		}
	}
	if (bF1 || bF2 || bF3 || bF4) {
		CProcessFormantTracker * pSpectroFormants = GetFormantTracker();

		// If pitch processed successfully, generate formant data.
		if (GetGrappl()->IsDataReady()) {
			short int nResult2 = LOWORD(pSpectroFormants->Process(this));
			if (nResult2 == PROCESS_ERROR) {
				bF1 = bF2 = bF3 = bF4 = FALSE;
			} else if (nResult2 == PROCESS_CANCELED) {
				return;
			} else {
				fSizeFactor[FMTTRACKER] = (double)GetDataSize() / (double)(pSpectroFormants->GetDataSize() - 1);
			}
		} else {
			bF1 = bF2 = bF3 = bF4 = FALSE;
		}
	}

	// now write the file
	{
		CFile file(filename, CFile::modeCreate | CFile::modeWrite);

		// write header

		// \name write filename
		szString = "\\name ";
		szString.Append(filename);
		szString.Append(szCrLf);

		WriteFileUtf8(&file, szString);

		// \date write current time
		CTime time = CTime::GetCurrentTime();
		szString = "\\date " + time.Format("%A, %B %d, %Y, %X") + "\r\n";
		WriteFileUtf8(&file, szString);

		// \wav  Audio FileName
		szString = "\\wav " + GetPathName() + szCrLf;
		WriteFileUtf8(&file, szString);

		// \calc calculation method
		if (nCalculationMethod == 0) {
			szString = "\\calc "  "midpoint" + szCrLf;
		} else {
			szString = "\\calc "  "average" + szCrLf;
		}
		WriteFileUtf8(&file, szString);

		if (bSampleTime) {
			szString = "\\table time " "every " + szIntervalTime + "ms" "\r\nTime\t";
		} else {
			szString = "\\table Etic\r\n";
		}
		if (bSegmentStart) {
			szString += "Start\t";
		}
		if (bSegmentLength) {
			szString += "Length\t";
		}
		if (bReference) {
			szString += "Ref\t";
		}
		if (bPhonetic) {
			szString += "Etic\t";
		}
		if (bTone) {
			szString += "Tone\t";
		}
		if (bPhonemic) {
			szString += "Emic\t";
		}
		if (bOrtho) {
			szString += "Ortho\t";
		};
		if (bGloss) {
			szString += "Gloss\t";
		}
		if (bGlossNat) {
			szString += "Gloss Nat.\t";
		}
		if (bMagnitude) {
			szString += "Int(dB)\t";
		}
		if (bPitch) {
			szString += "Pitch(Hz)\tRawPitch\tSmPitch\t";
		}
		if (bMelogram) {
			szString += "Melogram(st)\t";
		}
		if (bZeroCrossings) {
			szString += "ZCross\t";
		}
		if (bF1) {
			szString += "F1(Hz)\t";
		}
		if (bF2) {
			szString += "F2(Hz)\t";
		}
		if (bF3) {
			szString += "F3(Hz)\t";
		}
		if (bF4) {
			szString += "F4(Hz)\t";
		}
		szString += "\r\n";
		WriteFileUtf8(&file, szString);

		// construct table entries
		while (dwOffset < dwStopPosition) {
			if (nSampleRate == 1) { // interval sampling
				dwNext = dwOffset + dwIncrement;
			} else { // phonetic segment samples
				if (nIndex != -1) {
					dwNext = pPhonetic->GetStop(nIndex);
				} else {
					dwNext = GetDataSize();
				}
			}

			if (bSampleTime) {
				swprintf_s(szString.GetBuffer(25), 25, _T("%.3f\t"), GetTimeFromBytes(dwOffset));
				szString.ReleaseBuffer();
				WriteFileUtf8(&file, szString);
			}

			if ((nSampleRate == 0) && (nIndex != -1) && (pPhonetic->GetOffset(nIndex) < dwNext)) {
				DWORD dwPhonetic = pPhonetic->GetOffset(nIndex);
				if (bSegmentStart) {
					swprintf_s(szString.GetBuffer(25), 25, _T("%.3f\t"), GetTimeFromBytes(dwPhonetic));
					szString.ReleaseBuffer();
					WriteFileUtf8(&file, szString);
				}
				if (bSegmentLength) {
					swprintf_s(szString.GetBuffer(25), 25, _T("%.3f\t"), GetTimeFromBytes(pPhonetic->GetDuration(nIndex)));
					szString.ReleaseBuffer();
					WriteFileUtf8(&file, szString);
				}

				if (bReference) {
					GetSegment(REFERENCE)->GetString(dwPhonetic, szString, false);
					WriteFileUtf8(&file, szString);
				}
				if (bPhonetic) {
					szString = pPhonetic->GetSegmentString(nIndex) + "\t";
					WriteFileUtf8(&file, szString);
				}
				if (bTone) {
					GetSegment(TONE)->GetString(dwPhonetic, szString, false);
					WriteFileUtf8(&file, szString);
				}
				if (bPhonemic) {
					GetSegment(PHONEMIC)->GetString(dwPhonetic, szString, false);
					WriteFileUtf8(&file, szString);
				}
				if (bOrtho) {
					GetSegment(ORTHO)->GetString(dwPhonetic, szString, false);
					WriteFileUtf8(&file, szString);
				}
				if (bGloss) {
					GetSegment(GLOSS)->GetString(dwPhonetic, szString, true);
					WriteFileUtf8(&file, szString);
				}
				if (bGlossNat) {
					GetSegment(GLOSS_NAT)->GetString(dwPhonetic, szString, true);
					WriteFileUtf8(&file, szString);
				}
				nIndex = pPhonetic->GetNext(nIndex);
			} else if ((bPhonetic) && (nSampleRate == 1) && !pPhonetic->IsEmpty()) {
				nIndex = 0;

				while ((nIndex != -1) && (pPhonetic->GetStop(nIndex) < dwOffset)) {
					nIndex = pPhonetic->GetNext(nIndex);
				}

				if ((nIndex != -1) && pPhonetic->GetOffset(nIndex) < dwNext) { // this one overlaps
					int nLast = pPhonetic->GetNext(nIndex);
					szString = pPhonetic->GetSegmentString(nIndex);

					while ((nLast != -1) && (pPhonetic->GetOffset(nLast) < dwNext)) {
						szString += " " + pPhonetic->GetSegmentString(nLast);
						nLast = pPhonetic->GetNext(nLast);
					}
					szString += "\t";
				} else {
					szString = "\t";
				}
				WriteFileUtf8(&file, szString);
			}

			DWORD dwBegin = 0;
			DWORD dwEnd = 0;
			DWORD dwCalcIncrement = 0;
			DWORD dwIndex = 0;
			if (nCalculationMethod == 0) {
				// midpoint value
				dwBegin = dwEnd = (dwOffset + dwNext) / 2;
				dwEnd++;
				dwCalcIncrement = 10;
			} else {
				// average value
				dwBegin = dwOffset;
				dwEnd = dwNext;
				dwCalcIncrement = (dwEnd - dwBegin) / 20;
				if (!dwCalcIncrement) {
					dwCalcIncrement = 1;
				}
			}

			if (bMagnitude) {
				int dwSamples = 0;
				BOOL bRes = TRUE;
				double fData = 0;
				for (dwIndex = dwBegin; dwIndex < dwEnd; dwIndex += dwCalcIncrement) {
					DWORD dwProcData = (DWORD)(dwIndex / fSizeFactor[MAG]);
					// get data for this pixel
					fData += GetLoudness()->GetProcessedData(dwProcData, &bRes);
					dwSamples++;
				}
				if (dwSamples && bRes) {
					fData = fData / dwSamples;
					double fLoudnessMax = GetLoudness()->GetMaxValue();
					if (fData*10000. < fLoudnessMax) {
						fData = fLoudnessMax / 10000.;
					}

					double db = 20.0 * log10(fData / 32767.) + 9.;  // loudness is rms full scale would be 9dB over recommended recording level
					swprintf_s(szString.GetBuffer(25), 25, _T("%0.1f\t"), db);
					szString.ReleaseBuffer();
				} else {
					szString = "\t";
				}
				WriteFileUtf8(&file, szString);
			}
			if (bPitch) {
				int dwSamples = 0;
				BOOL bRes = TRUE;
				long nData = 0;
				for (dwIndex = dwBegin; dwIndex < dwEnd; dwIndex += dwCalcIncrement) {
					DWORD dwProcData = (DWORD)(dwIndex / fSizeFactor[PITCH]);
					// get data for this pixel
					int nHere = GetGrappl()->GetProcessedData(dwProcData, &bRes); // SDM 1.5Test11.0
					if (nHere > 0) {
						nData += nHere;
						dwSamples++;
					}
				}
				if (dwSamples && bRes) {
					double fData = double(nData) / PRECISION_MULTIPLIER / dwSamples;
					swprintf_s(szString.GetBuffer(25), 25, _T("%.1f\t"), fData);
					szString.ReleaseBuffer();
				} else {
					szString = "\t";
				}
				WriteFileUtf8(&file, szString);

				// Raw Pitch
				dwSamples = 0;
				bRes = TRUE;
				nData = 0;
				for (dwIndex = dwBegin; dwIndex < dwEnd; dwIndex += dwCalcIncrement) {
					DWORD dwProcData = (DWORD)(dwIndex / fSizeFactor[PITCH]);
					// get data for this pixel
					int nHere = GetPitch()->GetProcessedData(dwProcData, &bRes);
					if (nHere > 0) {
						nData += nHere;
						dwSamples++;
					}
				}
				if (dwSamples && bRes) {
					double fData = double(nData) / PRECISION_MULTIPLIER / dwSamples;
					swprintf_s(szString.GetBuffer(25), 25, _T("%.1f\t"), fData);
					szString.ReleaseBuffer();
				} else {
					szString = "\t";
				}
				WriteFileUtf8(&file, szString);

				// Smoothed Pitch
				dwSamples = 0;
				bRes = TRUE;
				nData = 0;
				for (dwIndex = dwBegin; dwIndex < dwEnd; dwIndex += dwCalcIncrement) {
					DWORD dwProcData = (DWORD)(dwIndex / fSizeFactor[PITCH]);
					// get data for this pixel
					int nHere = GetSmoothedPitch()->GetProcessedData(dwProcData, &bRes);
					if (nHere > 0) {
						nData += nHere;
						dwSamples++;
					}
				}
				if (dwSamples && bRes) {
					double fData = double(nData) / PRECISION_MULTIPLIER / dwSamples;
					swprintf_s(szString.GetBuffer(25), 25, _T("%.1f\t"), fData);
					szString.ReleaseBuffer();
				} else {
					szString = "\t";
				}
				WriteFileUtf8(&file, szString);
			}
			if (bMelogram) {
				int dwSamples = 0;
				BOOL bRes = TRUE;
				long nData = 0;
				for (dwIndex = dwBegin; dwIndex < dwEnd; dwIndex += dwCalcIncrement) {
					DWORD dwProcData = (DWORD)(dwIndex / fSizeFactor[MELOGRAM]);
					// get data for this pixel
					int nHere = GetMelogram()->GetProcessedData(dwProcData, &bRes); // SDM 1.5Test11.0
					if (nHere > 0) {
						nData += nHere;
						dwSamples++;
					}
				}
				if (dwSamples && bRes) {
					double fData = double(nData) / 100.0 / dwSamples;
					swprintf_s(szString.GetBuffer(25), 25, _T("%.2f\t"), fData);
					szString.ReleaseBuffer();
				} else {
					szString = "\t";
				}
				WriteFileUtf8(&file, szString);
			}
			if (bZeroCrossings) {
				int dwSamples = 0;
				BOOL bRes = TRUE;
				long nData = 0;
				for (dwIndex = dwBegin; dwIndex < dwEnd; dwIndex += dwCalcIncrement) {
					DWORD dwProcData = (DWORD)(dwIndex / fSizeFactor[ZCROSS]);
					// get data for this pixel
					nData += GetZCross()->GetProcessedData(dwProcData, &bRes);
					dwSamples++;
				}
				if (dwSamples && bRes) {
					nData = nData / dwSamples;
					swprintf_s(szString.GetBuffer(25), 25, _T("%d\t"), (int)nData);
					szString.ReleaseBuffer();
				} else {
					szString = "\t";
				}
				WriteFileUtf8(&file, szString);
			}

			if (bF1 || bF2 || bF3 || bF4) {
				int dwSamples[5] = { 0,0,0,0,0 };
				double pFormFreq[5] = { 0,0,0,0,0 };
				int bFormOn[5] = { 0, bF1, bF2, bF3, bF4 };

				// get FormantTracker data
				for (dwIndex = dwBegin; dwIndex < dwEnd; dwIndex++) {
					DWORD dwProcData = (DWORD)((DWORD)(dwIndex / fSizeFactor[FMTTRACKER] / sizeof(SFormantFreq)))*sizeof(SFormantFreq);
					SFormantFreq * pFormFreqCurr = (SFormantFreq *)GetFormantTracker()->GetProcessedData(dwProcData, sizeof(SFormantFreq));
					for (int n = 1; n < 5; n++) {
						if (pFormFreqCurr->F[n] == (float)NA) {
							continue;
						}
						pFormFreq[n] += (double)pFormFreqCurr->F[n];
						dwSamples[n]++;
					}
				}
				for (int n = 1; n < 5; n++) {
					pFormFreq[n] /= dwSamples[n];
					if (bFormOn[n]) {
						if (dwSamples[n]) {
							swprintf_s(szString.GetBuffer(25), 25, _T("%.1f\t"), (double)pFormFreq[n]);
							szString.ReleaseBuffer();
						} else {
							szString = "\t";
						}
						WriteFileUtf8(&file, szString);
					}
				}
			}

			szString = "\r\n"; // next line of table
			WriteFileUtf8(&file, szString);

			if (nSampleRate == 1) { // interval sampling
				dwOffset = dwNext < dwStopPosition ? dwNext : dwStopPosition;
			} else { // phonetic segment samples
				if (nIndex >= 0) {
					dwOffset = pPhonetic->GetOffset(nIndex);
				} else {
					dwOffset = dwStopPosition;
				}
			}
		}
	}
}

void CSaDoc::ExportAudacityLabelFile(LPCTSTR filename) {

	ASSERT(filename != NULL);
	ASSERT(wcslen(filename)>0);
	if (wcslen(filename) == 0) {
		return;
	}

	CSaString szString;

	CSegment * pPhonetic = GetSegment(PHONETIC);
	CSegment * pRef = GetSegment(REFERENCE);

	// now write the file
	{
		CFile file(filename, CFile::modeCreate | CFile::modeWrite);

		for (int i = 0; i < pPhonetic->GetOffsetSize(); i++) {

			DWORD offset = pPhonetic->GetOffset(i);
			DWORD duration = pPhonetic->GetDuration(i);

			// determine if we have a refid
			CString refTag;
			int nIndex2 = pRef->FindOffset(offset);
			if (nIndex2 != -1) {
				refTag = pRef->GetSegmentString(nIndex2).Trim();
			}
			if (refTag.GetLength() != 0) {

				swprintf_s(szString.GetBuffer(25), 25, _T("%.3f\t"), GetTimeFromBytes(offset));
				szString.ReleaseBuffer();
				WriteFileUtf8(&file, szString);

				swprintf_s(szString.GetBuffer(25), 25, _T("%.3f\t"), GetTimeFromBytes(offset + duration));
				szString.ReleaseBuffer();
				WriteFileUtf8(&file, szString);

				WriteFileUtf8(&file, refTag);

				// next line of table
				WriteFileUtf8(&file, "\r\n");
			}
		}
	}
}

/**
* We are assuming phonetic segments are populated
*/
void CSaDoc::GenerateCVData(CSaView & view) {

	POSITION pos = GetFirstViewPosition();
	CSaView * pView = (CSaView *)GetNextView(pos);
	Context& context = GetContext();

	pView->SendMessage(WM_COMMAND, ID_PHONETIC_ALL);

	CPhoneticSegment * pPhonetic = (CPhoneticSegment *)GetSegment(PHONETIC);

	// SDM1.5Test8.2
	CSegment * pSaveGloss = m_apSegments[GLOSS];
	CSegment * pSaveGlossNat = m_apSegments[GLOSS_NAT];
	CSegment * pSaveRef = m_apSegments[REFERENCE];
	m_apSegments[GLOSS] = new CGlossSegment(context,GLOSS, PHONETIC);
	m_apSegments[GLOSS_NAT] = new CGlossNatSegment(context,GLOSS_NAT, GLOSS);
	m_apSegments[REFERENCE] = new CReferenceSegment(context,REFERENCE, GLOSS);

	// for the case of cancelled dependent processes
	RestartAllProcesses();
	// for the case of a cancelled process
	pPhonetic->RestartProcess();
	// clear data from previous run SDM 1.06.4
	pPhonetic->SetDataInvalid();

	// process data
	short int nResult = LOWORD(pPhonetic->Process(NULL, this));
	if (nResult == PROCESS_ERROR) {
		// error segmenting
		ErrorMessage(IDS_ERROR_SEGMENT);
		return;
	}
	if (nResult == PROCESS_CANCELED) {
		// error cancelled segmenting
		ErrorMessage(IDS_CANCELED);
		return;
	}

	// formants need pitch info
	// SDM 1.5 Test 11.0
	CProcessGrappl * pPitch = GetGrappl();
	// process data
	nResult = LOWORD(pPitch->Process(this));
	if (nResult == PROCESS_ERROR) {
		ErrorMessage(IDS_ERROR_GENERATE_CV_NOPITCH);
		return;
	} else if (nResult == PROCESS_CANCELED) {
		ErrorMessage(IDS_CANCELED);
		return;
	}

	// restore the segments
	delete m_apSegments[GLOSS];
	delete m_apSegments[GLOSS_NAT];
	delete m_apSegments[REFERENCE];
	m_apSegments[GLOSS] = pSaveGloss;
	m_apSegments[GLOSS_NAT] = pSaveGlossNat;
	m_apSegments[REFERENCE] = pSaveRef;

	CGlossSegment * pGloss = (CGlossSegment *)GetSegment(GLOSS);

	double sizeFactor = (double)GetDataSize() / (double)(pPitch->GetDataSize() - 1);

	// Gloss segments need to be aligned to phonetic SDM 1.5Test8.2
	if (!pGloss->IsEmpty()) {

		DWORD dwDistance = 0;
		BOOL bInsert = FALSE;
		int nGloss = 0;
		for (nGloss = 0; nGloss < pGloss->GetOffsetSize(); nGloss++) {
			DWORD dwStart = pGloss->GetOffset(nGloss);
			DWORD dwStop = dwStart + pGloss->GetDuration(nGloss);
			DWORD dwTemp = dwStop;
			int nPhonetic = pGloss->AdjustPositionToMaster(this, dwStart, dwTemp);

			if (dwStart > pGloss->GetOffset(nGloss)) {
				dwDistance = dwStart - pGloss->GetOffset(nGloss);
			} else {
				dwDistance = pGloss->GetOffset(nGloss) - dwStart;
			}

			// empty phonetic
			bInsert = FALSE;
			if (nPhonetic == -1) {
				bInsert = TRUE;
				nPhonetic = 0;
			} else if (dwDistance > GetBytesFromTime(MAX_AUTOSEGMENT_MOVE_GLOSS_TIME)) {
				// too far away
				bInsert = TRUE;
				if (dwStart < pGloss->GetOffset(nGloss)) {
					nPhonetic = pPhonetic->GetNext(nPhonetic);
				}
			} else if (nGloss && (dwStart == pGloss->GetOffset(nGloss - 1))) {
				// last gloss attached to nearest phonetic
				bInsert = TRUE;
				nPhonetic = pPhonetic->GetNext(nPhonetic);
			} else if ((pGloss->GetNext(nGloss) != -1) && ((dwStart + dwTemp) / 2 <= pGloss->GetOffset(nGloss + 1))) {
				// next gloss nearest to same phonetic
				if ((pGloss->GetOffset(nGloss + 1) < dwStart) ||
					(dwDistance > (pGloss->GetOffset(nGloss + 1) - dwStart))) {
					// next gloss closer
					bInsert = TRUE;
					// nPhonetic is correct
				}
			}

			if (bInsert) {
				// Insert in same location as old segment
				dwStart = pGloss->GetOffset(nGloss);
				if (nPhonetic == -1) {
					// insert at end
					nPhonetic = pPhonetic->GetOffsetSize();
				}
				int nPrevious = pPhonetic->GetPrevious(nPhonetic);
				if (nPrevious != -1) {
					pPhonetic->Adjust(this, nPrevious, pPhonetic->GetOffset(nPrevious), dwStart - pPhonetic->GetOffset(nPrevious), false);
				}
				CSaString szEmpty = SEGMENT_DEFAULT_CHAR;
				pPhonetic->Insert(nPhonetic, szEmpty, false, dwStart, pPhonetic->GetOffset(nPhonetic) - dwStart);
			} else {
				pGloss->Adjust(this, nGloss, dwStart, dwStop - dwStart, false);
			}
			if (nGloss > 0) {
				// Adjust previous gloss segment
				pGloss->Adjust(this, nGloss - 1, pGloss->GetOffset(nGloss - 1), pGloss->CalculateDuration(this, nGloss - 1), false);
			}
		}
		// early termination
		if (nGloss > 0) {
			// Adjust previous gloss segment (last)
			pGloss->Adjust(this, nGloss - 1, pGloss->GetOffset(nGloss - 1), pGloss->CalculateDuration(this, nGloss - 1), false);
		}
	}

	CToneSegment * pTone = (CToneSegment*)GetSegment(TONE);
	pTone->DeleteContents();

	/**
	* We index the pitch data using the midpoint of the current segment
	* We need to scale the index as well
	*/
	for (int i = 0; i < pPhonetic->GetOffsetSize(); i++) {
		DWORD offset = pPhonetic->GetOffset(i);
		DWORD length = pPhonetic->GetDuration(i);
		int dwSamples = 0;
		BOOL bRes = TRUE;
		long nData = 0;
		DWORD index = (offset + (length / 2));
		DWORD data = index / sizeFactor;
		// get data for this pixel
		// SDM 1.5Test11.0
		int nHere = pPitch->GetProcessedData(data, &bRes);
		if (nHere > 0) {
			nData += nHere;
			dwSamples++;
		}
		if ((dwSamples > 0) && (bRes)) {
			//double fData = double(nData) / PRECISION_MULTIPLIER / dwSamples;
			//TRACE("pitch value = %f\n",fData);
			CSaString temp(L"V");
			pTone->Add(this, &view, offset, temp, false, false);
		} else {
			//TRACE("pitch value = none\n");
			CSaString temp(L"C");
			pTone->Add(this, &view, offset, temp, false, false);
		}
	}
}

/**
* Attempt to parse the gloss segments until
* we find the 'goal'
* ..Or end in defeat...
*/
bool CSaDoc::AutoSegment(LPCTSTR filename, CTranscriptionData & td, CSaView & view, DWORD goal, int algorithm, int skipCount, bool usingGL) {

	TRACE("Attempting to locate %d segments\n", goal);
	CAutoSegmentation worker;
	if (algorithm == 1) {
		return worker.DoPhoneticMatching(filename, *this, td, skipCount, usingGL);
	}
	return worker.DoDivideAndConquer(*this, view, goal, skipCount);
}

void CSaDoc::ErrorMessage(UINT nTextID, LPCTSTR pszText1, LPCTSTR pszText2) {
	CSaApp * pApp = (CSaApp*)AfxGetApp();
	pApp->ErrorMessage(nTextID, pszText1, pszText2);
}

void CSaDoc::ErrorMessage(CSaString & msg) {
	CSaApp * pApp = (CSaApp*)AfxGetApp();
	pApp->ErrorMessage(msg);
}

/**
* Try to identify the phonetic segment from this segment.
* Returns NULL if it can't be done (usually because of segmental formats)
*/
int CSaDoc::FindPhoneticIndex(CSegment * pSeg, int sel) {
	if (sel == -1) return -1;
	if (pSeg == NULL) return -1;
	if (pSeg->Is(PHONETIC)) return sel;
	DWORD offset = pSeg->GetOffset(sel);
	DWORD duration = pSeg->GetDuration(sel);
	CPhoneticSegment * pPhonetic = (CPhoneticSegment *)GetSegment(PHONETIC);
	for (int i = 0; i < pPhonetic->GetOffsetSize(); i++) {
		DWORD poffset = pPhonetic->GetOffset(i);
		DWORD pduration = pPhonetic->GetDuration(i);
		if ((offset == poffset) && (duration == pduration)) {
			return i;
		}
	}
	return -1;
}

/**
* We do not support segmental segmentation in the 'split operation
*/
void CSaDoc::SplitSegment(CPhoneticSegment * pPhonetic, int sel, DWORD splitPosition) {

	bool segmental = IsSegmental(pPhonetic, sel);
	if (segmental) return;

	bool modified = pPhonetic->SplitSegment(*this, sel, splitPosition);

	SetModifiedFlag(((modified) ? TRUE : FALSE) | IsModified());
	SetTransModifiedFlag(modified || IsTransModified());
}

void CSaDoc::ToggleSpectrogram() {

	CSpectroParm parameters = GetSpectrogram()->GetSpectroParm();
	bool bFormantSelected = !parameters.GetShowFormants();
	GetSpectrogram()->SetShowFormants(bFormantSelected);
	GetSnapshot()->SetShowFormants(bFormantSelected);
	CMainFrame* pMainFrame = (CMainFrame*)AfxGetMainWnd();
	pMainFrame->SetShowFormants(bFormantSelected);
	if ((bFormantSelected) && ((GetSpectrogram()->IsProcessCanceled()) || (GetSnapshot()->IsProcessCanceled()))) {
		RestartAllProcesses();
	}
}

Context& CSaDoc::GetContext() {
	// lazy construction
	if (pContext.get() == nullptr) {
		App& app = *(App*)(CSaApp*)AfxGetApp();
		MainFrame& frame = *(MainFrame*)(CMainFrame*)AfxGetMainWnd();
		pContext = make_unique<Context>(app, *this, frame, *this);
	}
	return *pContext;
}

PhoneticSegment* CSaDoc::GetPhoneticSegment() { 
	return (PhoneticSegment*)GetSegment(PHONETIC); 
};

string CSaDoc::GetSegmentContent(int index) {
	CSegment* pSegment = GetSegment(index);
	return _to_utf8(pSegment->GetContent().GetString());
}

int CSaDoc::GetProcessorText(ProcessorType processorType) {
	switch (processorType) {
	case PROCESSDUR: return IDS_STATTXT_PROCESSDUR;
	case PROCESSFMT: return IDS_STATTXT_PROCESSFMT;
	case PROCESSWBLP: return  IDS_STATTXT_PROCESSWBLP;
	case PROCESSWBEQU: return IDS_STATTXT_PROCESSWBEQU;
	case PROCESSDFLT: return IDS_STATTXT_PROCESSING;
	case PROCESSZCR: return IDS_STATTXT_PROCESSZCR;
	case PROCESSWVL: return IDS_STATTXT_PROCESSWVL;
	case PROCESSWBECHO: return IDS_STATTXT_PROCESSWBECHO;
	case PROCESSWBREV: return IDS_STATTXT_PROCESSWBREV;
	case PROCESSPIT: return IDS_STATTXT_PROCESSPIT;
	case PROCESSLOU: return IDS_STATTXT_PROCESSLOU;
	case PROCESSGRA: return IDS_STATTXT_PROCESSGRA;
	case BACKGNDFRA: return IDS_STATTXT_BACKGNDFRA;
	case PROCESSFRA: return IDS_STATTXT_PROCESSFRA;
	case PROCESSTWC: return IDS_STATTXT_PROCESSTWC;
	case PROCESSCHA: return IDS_STATTXT_PROCESSCHA;
	case PROCESSCPI: return IDS_STATTXT_PROCESSCPI;
	case BACKGNDGRA: return IDS_STATTXT_BACKGNDGRA;
	case PROCESSSLO: return IDS_STATTXT_PROCESSSLO;
	case PROCESSPOA: return IDS_STATTXT_PROCESSPOA;
	case PROCESSRAT: return IDS_STATTXT_PROCESSRAT;
	case PROCESSSPI: return IDS_STATTXT_PROCESSSPI;
	case PROCESSSPG: return IDS_STATTXT_PROCESSSPG;
	case PROCESSSPU: return IDS_STATTXT_PROCESSSPU;
	case PROCESSMEL: return IDS_STATTXT_PROCESSMEL;
	case PROCESSRAW: return IDS_STATTXT_PROCESSRAW;
	case SEGMENTING: return IDS_STATTXT_SEGMENTING;
	case PARSING: return IDS_STATTXT_PARSING;
	case PROCESSWBGENERATOR: return IDS_STATTXT_PROCESSWBGENERATOR;
	}
	return IDS_STATTXT_PROCESSING;
}
