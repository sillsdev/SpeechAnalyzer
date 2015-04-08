/////////////////////////////////////////////////////////////////////////////
// sa_doc.h:
// Interface of the CSaDoc class
// Author: Urs Ruchti
// copyright 2000 JAARS Inc. SIL
//
// Revision History
// 1.06.1.2 Added Function CSaDoc::UpdateSegmentBoundaries() SDM
// 1.06.4   SDM class wizard added menu messages for Tools/Import & Advanced/Align
// 1.06.5   SDM removed unused handlers, added handlers for update boundaries
// 1.06.6U2 SDM changed clipboard to WAV format added functions to support changes
// 1.06.6U4 SDM changed time to double to fix scrolling problems
// 1.06.6U5 SDM changed InsertSegment to non-const
// 1.06.8   SDM added GetId function
// 1.5Test8.1
//          SDM added bOverlap to UpdateBoundaries
// 1.5Test8.2
//          SDM added AdvancedSegment, AdvancedParse
// 1.5Test8.5
//          SDM added support for mode dependent menu and popups(GetDefaultMenu & GetDefaultAccel)
// 1.5Test10.0
//          SDM added support for WAV file database registration
//          SDM moved CParseParm & CSegmentParm to CMainFrame
// 1.5Test11.0
//          SDM added AutoSnapUpdate
// 1.5Test11.1A
//          RLJ Added CParseParm back in (leaving CParseParmDefaults in MainFrame)
//          RLJ Added CSegmentParm back in (leaving ParseSegmentDefaults in MainFrame)
//
// 09/25/00 DDO Got rid of the m_bCheckedTWC member var. I rewrote some stuff
//              in sa_dlg so it's no longer needed.
/////////////////////////////////////////////////////////////////////////////
#ifndef _SA_DOC_H
#define _SA_DOC_H

#include "undoredo.h"
#include "SaString.h"
#include "Appdefs.h"
#include "sourceParm.h"
#include "ISa_Doc.h"
#include "TranscriptionData.h"
#include "DlgAutoReferenceData.h"
#include "AlignInfo.h"
#include "ExportFWSettings.h"
#include "ExportLiftSettings.h"
#include "FmtParm.h"
#include "AutoSave.h"
#include "Process\ProcessDoc.h"
#include "SaParam.h"
#include <ElanUtils.h>
#include <LiftUtils.h>
#include "SegmentOps.h"

#import "SAUtils.tlb" no_namespace named_guids

//###########################################################################
// CSaDoc document

class CSaView;
class CFontTable;
class CProcessAdjust;
class CProcessLoudness;
class CProcessSmoothLoudness;
class CProcessZCross;
class CProcessPitch;
class CProcessCustomPitch;
class CProcessSmoothedPitch;
class CProcessGrappl;
class CProcessMelogram;
class CProcessChange;
class CProcessRaw;
class CProcessHilbert;
class CProcessSpectrum;
class CProcessSpectrogram;
class CProcessWavelet;              // ARH 8/2/01 Added for wavelet graph
class CProcessFormants;
class CProcessFormantTracker;
class CProcessDurations;
class CProcessFragments;
class CProcessSDP;
class CProcessRatio;
class CProcessPOA;
class CProcessGlottis;
class CProcessTonalWeightChart;
class CSegment;
class CGlossSegment;
class CGlossNatSegment;
class CDlgAdvancedParsePhrases;
class CDlgAutoReferenceData;
class CTranscriptionDataSettings;
class CMusicPhraseSegment;
class CPhoneticSegment;

class CSaDoc : public CUndoRedoDoc, public ISaDoc {
    DECLARE_DYNCREATE(CSaDoc)

protected:
    CSaDoc();
    virtual ~ CSaDoc();

public:
    void SetAudioModifiedFlag(bool bMod = true);
    bool IsAudioModified() const;
    void SetTransModifiedFlag(bool bMod = true);
    bool IsTransModified() const;
	// set document ID
    void SetID(int nID);
    int GetID();
	// set workbench process number
    void SetWbProcess(int nProcess);
	// return workbench process number
    int GetWbProcess();
	// pointer to file status structure
    CFileStatus * GetFileStatus();
	// pointer to source parameters structure
    SourceParm * GetSourceParm();
	// returns gender: 0 = male, 1 = female, 2 = child
    EGender GetGender();
    // guesses if undefined in source parameters
    const CUttParm * GetUttParm();
	// get a copy of the utterance parameters structure
    void GetUttParm(CUttParm *, BOOL bOriginal=FALSE);
	// set format parameters structure
    void SetFmtParm(CFmtParm *, BOOL bAdjustSpectro = TRUE);
	// set utterance parameters structure
    void SetUttParm(const CUttParm *, BOOL bOriginal = FALSE);
	// pointer to pitch parameters structure
    const CPitchParm * GetPitchParm() const;
    void SetPitchParm(const CPitchParm & parm);
	// pointer to music parameters structure
    const CMusicParm * GetMusicParm() const;
    void SetMusicParm(const CMusicParm & parm);
	// pointer to music parameters structure
    const CIntensityParm & GetCIntensityParm() const;
    void SetCIntensityParm(const CIntensityParm & parm);
	// pointer to SDP parameters structure
    SDPParm * GetSDPParm();
	// return wave source data size in bytes for one channel
    DWORD GetDataSize() const;
	// return wave source data size in bytes for all channels
    DWORD GetRawDataSize() const;
	// return wave source data pointer on given position (offset)
    HPSTR GetWaveData(DWORD dwOffset, BOOL bBlockBegin = FALSE);
	// return wave source (one sample) on given position (offset)
    int GetWaveData(DWORD dwOffset, BOOL *);
	// return index for wave source data buffer
    DWORD GetWaveBufferIndex();
    CSaString GetMusicScore();
    BOOL IsBackgroundProcessing();
	// background processing: TRUE = enabled
    BOOL EnableBackgroundProcessing(BOOL bState = TRUE);

	// process pointer to Unprocessed
    CProcessDoc * GetUnprocessed();
	// process pointer to adjust
    CProcessAdjust * GetAdjust();
	// process pointer to fragment object
    CProcessFragments * GetFragments();
	// process pointer to loudness object
    CProcessLoudness * GetLoudness();
	// process pointer to smooth loudness object
    CProcessSmoothLoudness * GetSmoothLoudness();
    CProcessZCross * GetZCross();                                   // process pointer to zero crossings object
    CProcessPitch * GetPitch();                                     // process pointer to pitch object
    CProcessCustomPitch * GetCustomPitch();                         // process pointer to custom pitch object
    CProcessSmoothedPitch * GetSmoothedPitch();                     // process pointer to smoothed pitch object
    CProcessChange * GetChange();                                   // process pointer to change object
    CProcessRaw * GetRaw();                                         // process pointer to change object
    CProcessHilbert * GetHilbert();                                 // process pointer to change object
    CProcessSpectrogram * GetSpectrogram(bool bRealTime);           // returns either the spectrogram or snapshot process dependent on flag
    CProcessWavelet * GetWavelet();                                 // process pointer to wavelet object  ARH 8/2/01 added for wavelet graph
    CProcessSpectrum * GetSpectrum();                               // process pointer to spectrum object
    CProcessGrappl * GetGrappl();                                   // process pointer to grappl object
    CProcessMelogram * GetMelogram();                               // process pointer to melogram object
    CProcessFormants * GetFormants();                               // process pointer to spectrogram object
    CProcessFormantTracker * GetFormantTracker();                   // process pointer to spectrogram object
    CProcessDurations * GetDurations();                             // process pointer to phonetic segment durations
    CProcessSDP * GetSDP(int nIndex);                               // process pointer to SDP object
    CProcessRatio * GetRatio();                                     // process pointer to ratio object
    CProcessPOA * GetPOA();                                         // process pointer to vocal tract model for finding place of articulation
    CProcessGlottis * GetGlottalWave();                             // process pointer to glottal waveform object
    CProcessTonalWeightChart * GetTonalWeightChart();               // process pointer to tonal weighting chart CLW 11/8/99
    int GetSegmentSize(EAnnotation nIndex);                         // get the pointers to a segment object
	// get the pointers to a segment object
    CSegment * GetSegment(int nIndex);
	// get the pointers to a segment object
    CSegment * GetSegment(EAnnotation nIndex);
    CGlossSegment * GetGlossSegment();
    CGlossNatSegment * GetGlossNatSegment();
	// return font size
    CFontTable * GetFont(int nIndex);
    CSaString GetMeasurementsString(DWORD dwOffset, DWORD dwLength, BOOL * pbRes);
	// set and create fonts
    void CreateFonts(); 
    CDocTemplate * GetTemplate();
    void AutoSnapUpdate(void);
    // methods for saving the window settings and program options.
    void WriteProperties(CObjectOStream & obs);
    static BOOL ReadProperties(CObjectIStream & obs);
    static BOOL ReadPropertiesOfViews(CObjectIStream & obs, const CSaString & str);
    void DeleteSegmentContents(EAnnotation type);
    bool HasSegmentData(EAnnotation val);
    void GetAlignInfo(CAlignInfo & alignInfo);
    void SetTempOverlay();
    bool IsTempOverlay();
    virtual void OnCloseDocument();
    virtual BOOL DoFileSave();
    bool CopySectionToNewWavFile(WAVETIME dwSectionStart, WAVETIME dwSectionLength, LPCTSTR szNewWave, bool usingClipboard);
    bool ConvertToMono(bool extractLeft, LPCTSTR filename);
    BOOL LoadDataFiles(LPCTSTR pszPathName, bool bTemp = false);
    BOOL WriteDataFiles(LPCTSTR pszPathName, bool bSaveAudio, bool bIsClipboardFile);
    bool GetWaveFormatParams(LPCTSTR pszPathName, CFmtParm & fmtParm, DWORD & dwDataSize);
    bool IsStandardWaveFormat(LPCTSTR pszPathName);
    bool IsMultiChannelWave(LPCTSTR pszPathName, int & channels);
    DWORD CheckWaveFormatForPaste(LPCTSTR pszPathName);
    DWORD CheckWaveFormatForOpen(LPCTSTR pszPathName);
    bool ConvertToWave(LPCTSTR pszPathName);
    BOOL InsertTranscriptions(LPCTSTR pszPathName, DWORD dwPos);
    BOOL InsertTranscription(int transType, ISaAudioDocumentReaderPtr saAudioDocRdr, DWORD dwPos);
    void InsertGlossPosRefTranscription(ISaAudioDocumentReaderPtr saAudioDocRdr, DWORD dwPos);
    BOOL CopyWave(LPCTSTR pszSourceName, LPCTSTR pszTargetName);
    virtual BOOL CopyWave(LPCTSTR pszSourceName, LPCTSTR pszTargetName, WAVETIME start, WAVETIME length, BOOL bTruncate);
	// apply a new recorded wave file
    void ApplyWaveFile(LPCTSTR pszFileName, DWORD dwDataSize, BOOL bInialUpdate=TRUE);
	// Update for rt auto-pitch
    void ApplyWaveFile(LPCTSTR pszFileName, DWORD dwDataSize, CAlignInfo alignInfo);
    DWORD SnapCursor(ECursorSelect nCursorSelect,
                     DWORD dwCursorOffset,
                     DWORD dwLowerLimit,
                     DWORD dwUpperLimit,
                     ESnapDirection nSnapDirection = SNAP_BOTH,
                     ECursorAlignment nCursorAlignment = ALIGN_USER_SETTING); // align cursors
    DWORD SnapCursor(ECursorSelect nCursorSelect, DWORD dwCursorOffset, ESnapDirection nSnapDirection = SNAP_BOTH);
	// invalidates all the graph processes
    void InvalidateAllProcesses();
	// invalidates all the processes
    void RestartAllProcesses();
	// invalidates all the processes
    BOOL AnyProcessCanceled();
    // process the actually selected workbench process
    BOOL WorkbenchProcess(BOOL bInvalidate = FALSE, BOOL bRestart = FALSE);
    // copies wave data out of the wave file
    BOOL PutWaveToClipboard(WAVETIME sectionStart, WAVETIME sectionLength, BOOL bDelete = FALSE);
    // pastes wave data into the wave file
    BOOL PasteClipboardToWave(HGLOBAL hGlobal, WAVETIME start);
    BOOL InsertSilenceIntoWave(WAVETIME silence, WAVETIME start, int repetitions);

	// deletes a wave undo entry from the undo list
    void DeleteWaveFromUndo();
	// undo a wave file change
    void UndoWaveFile();
    BOOL IsWaveToUndo();
    void CopyProcessTempFile();
	// update segment boundaries
    BOOL UpdateSegmentBoundaries(BOOL bOverlap);
    BOOL UpdateSegmentBoundaries(BOOL bOverlap, int nAnnotation, int nSelection, DWORD start, DWORD stop);
    BOOL AutoSnapUpdateNeeded(void);
	// overridden for document i/o
    virtual void SerializeForUndoRedo(CArchive & ar);
    BOOL AdvancedParsePhrase();
    BOOL AdvancedParseWord();
    BOOL AdvancedParseAuto();
    BOOL AdvancedSegment();
	// get menu depending on state
    virtual HMENU GetDefaultMenu();
    virtual HACCEL GetDefaultAccel();
    virtual void NotifyAutoPitchDone(void * pCaller);
    virtual void NotifyFragmentDone(void * pCaller);
    virtual void Serialize(CArchive & ar);
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext & dc) const;
#endif
    void DoExportFieldWorks(CExportFWSettings & settings);
    void DoExportLift(CExportLiftSettings & settings);
    const CSaString BuildString(int nSegment);
    const CSaString BuildImportString(BOOL gloss, BOOL glossNat, BOOL phonetic, BOOL phonemic, BOOL orthographic);
    const bool ImportTranscription(wistringstream & strm, BOOL gloss, BOOL glossNat, BOOL phonetic, BOOL phonemic, BOOL orthographic, CTranscriptionData & td, bool addTag, bool showDlg);
    void ApplyTranscriptionChanges(CTranscriptionDataSettings & settings);
    void RevertTranscriptionChanges();
    bool IsTempFile();
    bool CanEdit();
    wstring GetFilenameFromTitle();

    // wave helper functions
	// get a copy of the format parameters structure
    void GetFmtParm(CFmtParm & format, bool processed);

	// return wave data pointer on given position (offset)
    HPSTR GetAdjustedUnprocessedWaveData(DWORD dwOffset);
    HPSTR GetUnprocessedWaveData(DWORD dwOffset, BOOL bBlockBegin);
    void * GetUnprocessedDataBlock(DWORD dwByteOffset, size_t sObjectSize, BOOL bReverse);
    DWORD GetUnprocessedBufferIndex(size_t nSize);
    DWORD GetUnprocessedWaveDataBufferSize();
    DWORD GetWaveDataBufferSize();
    DWORD GetSelectedChannel();
    DWORD GetSampleSize() const;
    WAVETIME GetTimeFromBytes(DWORD dwSize);
    DWORD GetBytesFromTime(WAVETIME fSize);
    DWORD GetSamplesPerSec();
    DWORD GetBytesPerSample(bool singleChannel);
    WORD GetBitsPerSample();
    WORD GetBlockAlign(bool singleChannel=false);
    bool Is16Bit();
    bool IsPCM();
    DWORD GetAvgBytesPerSec();
    DWORD GetNumChannels() const;
    DWORD GetNumSamples() const;

    WAVETIME toTime(CURSORPOS val);
    WAVETIME toTime(CURSORPOS val, bool singleChannel);
    WAVETIME toTimeFromSamples(WAVESAMP val);
    CURSORPOS toCursor(WAVETIME val);
    CURSORPOS toCursor(WAVESAMP val);
    DWORD ToBytes(WAVETIME val, bool singleChannel);
    DWORD toSamples(WAVETIME val);

    CSaString GetTempFilename();
    bool IsUsingTempFile();
    void StoreAutoRecoveryInformation();

    wstring GetTranscriptionFilename();
    bool IsUsingHighPassFilter();
    void DisableHighPassFilter();
    CSaString GetDescription();
    void SetDescription(LPCTSTR val);
    bool MatchesDescription(LPCTSTR val);
    bool IsValidRecordFileFormat();
    int GetRecordFileFormat();
    DWORD GetRecordBandWidth();
    BYTE GetRecordSampleSize();
    BYTE GetQuantization();
    void SetQuantization(BYTE val);
    DWORD GetSignalBandWidth();
    void SetSignalBandWidth(DWORD val);
    void ClearHighPassFilter();
    void SetHighPassFilter();
    DWORD GetNumberOfSamples();
    void SetNumberOfSamples(DWORD val);
    void SetRecordSampleSize(BYTE val);
    void SetRecordBandWidth(DWORD val);
    void SetRecordTimeStamp(CTime & val);

    int GetLastSegmentBeforePosition(int annotSetID, DWORD cursorPos);

    static int GetSaveAsFilename(LPCTSTR title, LPCTSTR filter, LPCTSTR extension, LPTSTR path, wstring & result);
    void SplitSegment( CPhoneticSegment * pSeg, int sel, bool segmental);
    bool CanSplit(CSegment * pSeg);
    bool CanMerge(CSegment * pSeg);
    void MergeSegments( CPhoneticSegment * pSeg);
    bool IsSegmental(CPhoneticSegment * pSeg, int sel);
    bool IsBoundary(CPhoneticSegment * pPhonetic, int sel);
    void SelectSegment(CSegment * pSegment, int index);
    bool CanMoveDataRight(CSegment * pSegment);
    void MoveDataLeft(DWORD offset);
    void MoveDataRight(DWORD offset);
	void DeselectAll();
	void ImportSAB( CSaView & view, LPCTSTR filename);

	void AdjustZero();
	int GetZero();

	void AdjustNormalize();
	int GetNormalize();

	void AdjustInvert();
	int GetInvert();

	void ClearSABLoaded();

    bool PreflightAddReferenceData(CDlgAutoReferenceData & dlg, int selection);
    void AddReferenceData(CDlgAutoReferenceData & dlg, int selection);

	void ClearTranscriptionCount();
	void RevertAllTranscriptionChanges();

	void SaveSection( bool sameFile, LPCTSTR oldFile, LPCTSTR newFile, ESaveArea saveArea, EFileFormat fileFormat);

	bool IsTempWaveEmpty();
	CString GetTempWave();

	bool HasFullName();

	void CloseAutoSave( LPCTSTR file);

	void ExportTimeTable( BOOL bF1,
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
						  int nRegion,
						  BOOL bMelogram);
	void GenerateCVData( CSaView & view);
	bool AutoSegment( CSaView & view, DWORD goal);
	int FindPhoneticIndex( CSegment * pSeg, int index);

protected:
    virtual void DeleteContents();
    virtual BOOL OnNewDocument();
    virtual BOOL OnOpenDocument(LPCTSTR pszPathName);
    virtual BOOL OnSaveDocument(LPCTSTR pszPathName);
    BOOL SaveDocument(LPCTSTR pszPathName, bool bSaveAudio);
    BOOL CopyWaveToTemp(LPCTSTR pszSourcePathName, double dStart, double dTotalLength);
	// return TRUE if ok to continue
    virtual BOOL SaveModified(); 
    void AlignTranscriptionData(CTranscriptionDataSettings & settings);
    void AlignTranscriptionDataByRef(CTranscriptionData & td);
    bool ExportSegments(CExportLiftSettings & settings, Lift13::lift & document, bool skipEmptyGloss, LPCTSTR szPath, int & dataCount, int & wavCount);
    bool TryExportSegmentsBy(CExportFWSettings & settings, EAnnotation master, CFile & file, bool skipEmptyGloss, LPCTSTR szPath, int & dataCount, int & wavCount);
    CSaString BuildRecord(EAnnotation target, DWORD dwStart, DWORD dwStop);
    EAnnotation ConvertToAnnotation(int val);
    BOOL GetFlag(EAnnotation val, CExportFWSettings & settings);
    BOOL GetFlag(EAnnotation val, CExportLiftSettings & settings);
    int GetIndex(EAnnotation val);
    LPCTSTR GetTag(EAnnotation val);
    void WriteFileUtf8(CFile * pFile, const CSaString szString);
    LPCTSTR GetProcessFilename();

    int m_nTranscriptionApplicationCount;

private:
    BOOL CopyWaveToTemp(LPCTSTR pszSourcePathName);
    BOOL InsertWaveToTemp(LPCTSTR pszSourcePathName, LPCTSTR pszTempPathName, DWORD insertPos);

    // Methods for loading a wave file and all it's transcription data.
    bool ReadRiff(LPCTSTR pszPathName);
    BOOL LoadTranscriptionData(LPCTSTR pszPathName, BOOL bTemp);
    void ReadNonSegmentData(ISaAudioDocumentReaderPtr saAudioDocRdr);
    void ReadTranscription(int transType, ISaAudioDocumentReaderPtr saAudioDocRdr, DWORD limit, int & exceeded, int & limited);
    void ReadGlossPosAndRefSegments(ISaAudioDocumentReaderPtr saAudioDocRdr, DWORD limit, int & exceeded, int & limited);
    void ReadScoreData(ISaAudioDocumentReaderPtr saAudioDocRdr);

    // Methods for saving a wave file and all it's transcription data.
    DWORD WriteRiff(LPCTSTR pszPathName);
    void WriteNonSegmentData(DWORD dwDataSize, ISaAudioDocumentWriterPtr saAudioDocWriter);
    void WriteTranscription(int transType, ISaAudioDocumentWriterPtr saAudioDocWriter);
    void WriteGlossPosAndRefSegments(ISaAudioDocumentWriterPtr saAudioDocWriter);
    void WriteScoreData(ISaAudioDocumentWriterPtr saAudioDocWriter);

    void CreateSAXML(LPCTSTR filename, Elan::CAnnotationDocument & document, map<EAnnotation,wstring> & assignments);

    void ErrorMessage(UINT nTextID, LPCTSTR pszText1=NULL, LPCTSTR pszText2=NULL);

    void NormalizePhoneticDependencies();
    int GetInsertionIndex(CSegment * pSegment, DWORD offset);

	void UpdateReferenceBuffer();

	void ErrorMessage( CSaString & msg);

	// SAB transcription data buffer
	bool sabLoaded;
	CTranscriptionData sabBuffer;

	// segment operations
	CSegmentOps segmentOps;

	// dirty flag for audio data
    bool m_bAudioModified;
	// dirty flag for transcription data
    bool m_bTransModified;                      
    bool m_bTempOverlay;
	// document ID
    int m_ID;
	// workbench process number
    int m_nWbProcess;
	// file status information
    CFileStatus m_fileStat;                     

    // the data is interleaved
    // so for stereo 8 bit, the data is ordered
    // left, right, left, right
    // for stereo 16 bit, the data is ordered
    // left(low), left(high), right(low), right(high)

	// wave working temporary represents all channels of data
    wstring m_szRawDataWrk;
	// size of the data subchunk for all channels
    DWORD m_dwDataSize;
	// contains format parameters
    CFmtParm m_FmtParm;
	// contains sa parameters
    CSaParam m_saParam;
	// contains source parameters
    SourceParm m_sourceParm;
	// contains utterance (pitch) parameters
    CUttParm m_uttParm;
	// contains utterance (pitch) parameters as read from file
    CUttParm m_uttOriginal;
	// contains pitch parameters
    CPitchParm m_pitchParm;
	// contains music parameters
    CMusicParm m_musicParm;
	// contains intensity parameters
    CIntensityParm m_intensityParm;
	// contains SDP parameters
    SDPParm m_SDPParm;
	// musical score string
    CSaString m_szMusicScore;
	// TRUE = enable background processing when idle
    BOOL m_bProcessBackground;
	// process the raw data
    CProcessDoc m_ProcessDoc;
    
	// data processing objects
	CProcessAdjust * m_pProcessAdjust;
    CProcessFragments * m_pProcessFragments;    
    CProcessLoudness * m_pProcessLoudness;
    CProcessSmoothLoudness * m_pProcessSmoothLoudness;
    CProcessZCross * m_pProcessZCross;
    CProcessPitch * m_pProcessPitch;
    CProcessCustomPitch * m_pProcessCustomPitch;
    CProcessSmoothedPitch * m_pProcessSmoothedPitch;
    CProcessGrappl * m_pProcessGrappl;
    CProcessMelogram * m_pProcessMelogram;
    CProcessChange * m_pProcessChange;
    CProcessRaw * m_pProcessRaw;
    CProcessHilbert * m_pProcessHilbert;
    CProcessSpectrum * m_pProcessSpectrum;
    CProcessSpectrogram * m_pProcessSpectrogram;
    CProcessSpectrogram * m_pProcessSnapshot;
	// data processing object  ARH 8/2/01 Added for wavelet graph
    CProcessWavelet * m_pProcessWavelet;
    CProcessFormants * m_pProcessFormants;
    CProcessFormantTracker * m_pProcessFormantTracker;
    CProcessDurations * m_pProcessDurations;
    CProcessSDP * m_pProcessSDP[3];
    CProcessRatio * m_pProcessRatio;
    CProcessPOA * m_pProcessPOA;
    CProcessGlottis  * m_pProcessGlottis;
	// data processing object CLW 11/5/99
    CProcessTonalWeightChart * m_pProcessTonalWeightChart; 
	
	
	// array of pointers to the segment objects
    CSegment * m_apSegments[ANNOT_WND_NUMBER];
    CObArray * m_pCreatedFonts;
	// actual block begin flag
    BOOL m_bBlockBegin;
	// file name and path of temp file for wave data chunk
    CSaString m_szTempWave;
	// path of temp wave file converted from non-wave audio
    wstring m_szTempConvertedWave;
	// TRUE, if wave file change is to undo
    BOOL m_bWaveUndoNow;
	// counter for checkpoints (wave undo)
    int m_nCheckPointCount;
	// assigned from SA wave doc reader COM object
    _bstr_t m_szMD5HashCode;
	// FALSE, if audio file is non-wave (mp3, wma, etc.) or standard wave
    bool m_bUsingTempFile;
	// TRUE if the file is editable
    bool m_bAllowEdit;                          

    bool m_bMultiChannel;
    int m_nSelectedChannel;
    CAutoSave m_AutoSave;
};

#endif
