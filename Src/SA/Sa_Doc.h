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
#include "FmtParm.h"
#include "AutoSave.h"
#include "Process\ProcessDoc.h"
#include "SaParam.h"

#include "ElanUtils.h"

#import "speechtoolsutils.tlb" no_namespace named_guids
#import "st_audio.tlb" no_namespace named_guids

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
class CDlgAdvancedSegment;
class CDlgAdvancedParseWords;
class CDlgAdvancedParsePhrases;
class CTranscriptionDataSettings;
class CMusicPhraseSegment;

class CSaDoc : public CUndoRedoDoc, public ISaDoc
{
    DECLARE_DYNCREATE(CSaDoc)

protected:
    CSaDoc();
    virtual ~ CSaDoc();

public:
    void SetAudioModifiedFlag(bool bMod = true);
    bool IsAudioModified() const;
    void SetTransModifiedFlag(bool bMod = true);
    bool IsTransModified() const;
    void SetID(int nID);                                            // set document ID
    int GetID();
    void SetWbProcess(int nProcess);                                // set workbench process number
    int GetWbProcess();                                             // return workbench process number
    CFileStatus * GetFileStatus();                                  // pointer to file status structure
    SourceParm * GetSourceParm();                                   // pointer to source parameters structure
    EGender GetGender();											// returns gender: 0 = male, 1 = female, 2 = child
    // guesses if undefined in source parameters
    const CUttParm * GetUttParm();
    void GetUttParm(CUttParm *, BOOL bOriginal=FALSE);              // get a copy of the utterance parameters structure
    void SetFmtParm(CFmtParm *, BOOL bAdjustSpectro = TRUE);        // set format parameters structure
    void SetUttParm(const CUttParm *, BOOL bOriginal = FALSE);      // set utterance parameters structure
    const CPitchParm * GetPitchParm() const;                        // pointer to pitch parameters structure
    void SetPitchParm(const CPitchParm & parm);
    const CMusicParm * GetMusicParm() const;                        // pointer to music parameters structure
    void SetMusicParm(const CMusicParm & parm);
    const CIntensityParm & GetCIntensityParm() const;               // pointer to music parameters structure
    void SetCIntensityParm(const CIntensityParm & parm);
    SDPParm * GetSDPParm();                                         // pointer to SDP parameters structure
    DWORD GetDataSize() const;                                      // return wave source data size in bytes for one channel
    DWORD GetRawDataSize() const;                                   // return wave source data size in bytes for all channels
    HPSTR GetWaveData(DWORD dwOffset, BOOL bBlockBegin = FALSE);    // return wave source data pointer on given position (offset)
    int GetWaveData(DWORD dwOffset, BOOL *);                        // return wave source (one sample) on given position (offset)
    DWORD GetWaveBufferIndex();                                     // return index for wave source data buffer
    CSaString GetMusicScore();
    BOOL IsBackgroundProcessing();
    BOOL EnableBackgroundProcessing(BOOL bState = TRUE);            // background processing: TRUE = enabled
	
	CProcessDoc * GetUnprocessed();                                 // process pointer to Unprocessed
    CProcessAdjust * GetAdjust();                                   // process pointer to adjust
    CProcessFragments * GetFragments();                             // process pointer to fragment object
    CProcessLoudness * GetLoudness();                               // process pointer to loudness object
    CProcessSmoothLoudness * GetSmoothLoudness();                   // process pointer to smooth loudness object
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
    CSegment * GetSegment(int nIndex);								// get the pointers to a segment object
    CSegment * GetSegment(EAnnotation nIndex);                      // get the pointers to a segment object
    CGlossSegment * GetGlossSegment();
    CFontTable * GetFont(int nIndex);                               // return font size
    CSaString GetMeasurementsString(DWORD dwOffset, DWORD dwLength, BOOL * pbRes);
    void DestroyAdvancedSegment();
    void DestroyAdvancedParse();
    void CreateFonts(); // set and create fonts
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
    BOOL CopySectionToNewWavFile( WAVETIME dwSectionStart, WAVETIME dwSectionLength, LPCTSTR szNewWave, BOOL usingClipboard);
	bool ConvertToMono( bool extractLeft, LPCTSTR filename);
    BOOL LoadDataFiles(LPCTSTR pszPathName, bool bTemp = false);
    BOOL WriteDataFiles(LPCTSTR pszPathName, BOOL bSaveAudio = TRUE, BOOL bIsClipboardFile = FALSE);
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
    void ApplyWaveFile(LPCTSTR pszFileName, DWORD dwDataSize, BOOL bInialUpdate=TRUE);     // apply a new recorded wave file
    void ApplyWaveFile(LPCTSTR pszFileName, DWORD dwDataSize, CAlignInfo alignInfo);       // Update for rt auto-pitch
    DWORD SnapCursor(ECursorSelect nCursorSelect,
                     DWORD dwCursorOffset,
                     DWORD dwLowerLimit,
                     DWORD dwUpperLimit,
                     ESnapDirection nSnapDirection = SNAP_BOTH,
                     ECursorAlignment nCursorAlignment = ALIGN_USER_SETTING); // align cursors
    DWORD SnapCursor(ECursorSelect nCursorSelect,
                     DWORD dwCursorOffset,
                     ESnapDirection nSnapDirection = SNAP_BOTH);
    void InvalidateAllProcesses();	// invalidates all the graph processes
    void RestartAllProcesses();		// invalidates all the processes
    BOOL AnyProcessCanceled();		// invalidates all the processes
	// process the actually selected workbench process
    BOOL WorkbenchProcess(BOOL bInvalidate = FALSE, BOOL bRestart = FALSE); 
	// copies wave data out of the wave file
    BOOL PutWaveToClipboard( WAVETIME sectionStart, WAVETIME sectionLength, BOOL bDelete = FALSE); 
	// pastes wave data into the wave file
    BOOL PasteClipboardToWave(HGLOBAL hGlobal, WAVETIME start);
	BOOL InsertSilenceIntoWave( WAVETIME silence, WAVETIME start);

    void DeleteWaveFromUndo();		// deletes a wave undo entry from the undo list
    void UndoWaveFile();			// undo a wave file change
    BOOL IsWaveToUndo();
    void CopyProcessTempFile();
    void AdjustSegments( WAVETIME sectionStart, WAVETIME sectionLength, bool bShrink); // adjust segments to new file size
    BOOL UpdateSegmentBoundaries(BOOL bOverlap);		// update segment boundaries
    BOOL UpdateSegmentBoundaries(BOOL bOverlap, int nAnnotation, int nSelection, DWORD start, DWORD stop);
    BOOL AutoSnapUpdateNeeded(void);
    virtual void SerializeForUndoRedo(CArchive & ar);	// overridden for document i/o
    BOOL AdvancedParsePhrase();
    BOOL AdvancedParseWord();
    BOOL AdvancedParseAuto();
    BOOL AdvancedSegment();
    virtual HMENU GetDefaultMenu(); // get menu depending on state
    virtual HACCEL GetDefaultAccel();
    virtual void NotifyAutoPitchDone(void * pCaller);
    virtual void NotifyFragmentDone(void * pCaller);
    virtual void Serialize(CArchive & ar);
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext & dc) const;
#endif
    void DoExportFieldWorks(CExportFWSettings & settings);
    const CSaString BuildString(int nSegment);
    const CSaString BuildImportString(BOOL gloss, BOOL phonetic, BOOL phonemic, BOOL orthographic);
    const bool ImportTranscription( wistringstream & strm, BOOL gloss, BOOL phonetic, BOOL phonemic, BOOL orthographic, CTranscriptionData & td, bool addTag, bool showDlg);
    void ApplyTranscriptionChanges(CTranscriptionDataSettings & settings);
    void RevertTranscriptionChanges();
    bool IsTempFile();
    bool CanEdit();
	wstring GetFilenameFromTitle();

    // wave helper functions
    void GetFmtParm(CFmtParm & format, bool processed);     // get a copy of the format parameters structure

    HPSTR GetAdjustedUnprocessedWaveData(DWORD dwOffset);   // return wave data pointer on given position (offset)
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
	DWORD GetBytesPerSample( bool singleChannel);
    WORD GetBitsPerSample();
    WORD GetBlockAlign( bool singleChannel=false);
    bool Is16Bit();
    bool IsPCM();
    DWORD GetAvgBytesPerSec();
    DWORD GetNumChannels() const;
    DWORD GetNumSamples() const;

	WAVETIME toTime( CURSORPOS val);
	WAVETIME toTime( CURSORPOS val, bool singleChannel);
	WAVETIME toTimeFromSamples( WAVESAMP val);
	CURSORPOS toCursor( WAVETIME val);
	CURSORPOS toCursor( WAVESAMP val);
	DWORD toBytes( WAVETIME val, bool singleChannel);
	DWORD toSamples( WAVETIME val);

	CSaString GetTempFilename();
	bool IsUsingTempFile();
	void StoreAutoRecoveryInformation();

	wstring GetTranscriptionFilename();
	bool IsUsingHighPassFilter();
	void DisableHighPassFilter();
	CSaString GetDescription();
	void SetDescription( LPCTSTR val);
	bool MatchesDescription( LPCTSTR val);
	bool IsValidRecordFileFormat();
	int GetRecordFileFormat();
	DWORD GetRecordBandWidth();
	BYTE GetRecordSampleSize();
	BYTE GetQuantization();
	void SetQuantization( BYTE val);
	DWORD GetSignalBandWidth();
	void SetSignalBandWidth( DWORD val);
	void ClearHighPassFilter();
	void SetHighPassFilter();
	DWORD GetNumberOfSamples();
	void SetNumberOfSamples( DWORD val);
	void SetRecordSampleSize( BYTE val);
	void SetRecordBandWidth( DWORD val);
	void SetRecordTimeStamp( CTime & val);

	int GetLastSegmentBeforePosition( int annotSetID, DWORD cursorPos);

protected:
    virtual void DeleteContents();
    virtual BOOL OnNewDocument();
    virtual BOOL OnOpenDocument(LPCTSTR pszPathName);
    virtual BOOL OnSaveDocument(LPCTSTR pszPathName);
    virtual BOOL OnSaveDocument(LPCTSTR pszPathName, BOOL bSaveAudio);
    BOOL CopyWaveToTemp(LPCTSTR pszSourcePathName, double dStart, double dTotalLength);
    virtual BOOL SaveModified(); // return TRUE if ok to continue
    void AlignTranscriptionData(CTranscriptionDataSettings & settings);
    void AlignTranscriptionDataByRef(CTranscriptionData & td);
    bool TryExportSegmentsBy(CExportFWSettings & settings, EAnnotation master, CFile & file, bool skipEmptyGloss, LPCTSTR szPath, int & dataCount, int & wavCount);
    CSaString BuildRecord(EAnnotation target, DWORD dwStart, DWORD dwStop);
    EAnnotation ConvertToAnnotation(int val);
    BOOL GetFlag(EAnnotation val, CExportFWSettings & settings);
    int GetIndex(EAnnotation val);
    LPCTSTR GetTag(EAnnotation val);
    void WriteFileUtf8(CFile * pFile, const CSaString szString);
    LPCTSTR GetProcessFilename();

    afx_msg void OnUpdateFileSave(CCmdUI * pCmdUI);
    afx_msg void OnFileSaveAs();
    afx_msg void OnUpdateFileSaveAs(CCmdUI * pCmdUI);
    afx_msg void OnFileSplitFile();
    afx_msg void OnUpdateFileSplit(CCmdUI * pCmdUI);
    afx_msg void OnAdvancedParseWords();
    afx_msg void OnUpdateAdvancedParseWords(CCmdUI * pCmdUI);
    afx_msg void OnAdvancedParsePhrases();
    afx_msg void OnUpdateAdvancedParsePhrases(CCmdUI * pCmdUI);
    afx_msg void OnAdvancedSegment();
    afx_msg void OnUpdateAdvancedSegment(CCmdUI * pCmdUI);
    afx_msg void OnAutoAlign();
    afx_msg void OnUpdateAutoAlign(CCmdUI * pCmdUI);
    afx_msg void OnAutoReferenceData();
    afx_msg void OnUpdateAutoReferenceData(CCmdUI * pCmdUI);
    afx_msg void OnUpdateBoundaries();
    afx_msg void OnUpdateUpdateBoundaries(CCmdUI * pCmdUI);
    afx_msg void OnAutoSnapUpdate();
    afx_msg void OnUpdateAutoSnapUpdate(CCmdUI * pCmdUI);
    afx_msg void OnToolsAdjustInvert();
    afx_msg void OnUpdateToolsAdjustInvert(CCmdUI * pCmdUI);
    afx_msg void OnToolsAdjustNormalize();
    afx_msg void OnUpdateToolsAdjustNormalize(CCmdUI * pCmdUI);
    afx_msg void OnToolsAdjustZero();
    afx_msg void OnUpdateToolsAdjustZero(CCmdUI * pCmdUI);
    afx_msg void OnToolsAdjustSilence();
    afx_msg void OnUpdateToolsAdjustSilence(CCmdUI * pCmdUI);

    int m_nTranscriptionApplicationCount;

    DECLARE_MESSAGE_MAP()

private:
    BOOL CopyWaveToTemp(LPCTSTR pszSourcePathName);
    BOOL InsertWaveToTemp(LPCTSTR pszSourcePathName, LPCTSTR pszTempPathName, DWORD insertPos);
    CSaString SetFileExtension( CSaString fileName, LPCTSTR szExt);

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

	void CreateSAXML( LPCTSTR filename, Elan::CAnnotationDocument & document, map<EAnnotation,wstring> & assignments);

	void ErrorMessage( UINT nTextID, LPCTSTR pszText1=NULL, LPCTSTR pszText2=NULL);
	void ErrorMessage( CSaString & text);
    bool m_bAudioModified;                      // dirty flag for audio data
    bool m_bTransModified;                      // dirty flag for transcription data
    bool m_bTempOverlay;
    int m_ID;                                   // document ID
    int m_nWbProcess;                           // workbench process number
    CFileStatus m_fileStat;                     // file status information

    // the data is interleaved
    // so for stereo 8 bit, the data is ordered
    // left, right, left, right
    // for stereo 16 bit, the data is ordered
    // left(low), left(high), right(low), right(high)

    wstring m_szRawDataWrk;                     // wave working temporary represents all channels of data
    DWORD m_dwDataSize;                         // size of the data subchunk for all channels
    CFmtParm m_FmtParm;                         // contains format parameters
    CSaParam m_saParam;                         // contains sa parameters
    SourceParm m_sourceParm;                    // contains source parameters
    CUttParm m_uttParm;                         // contains utterance (pitch) parameters
    CUttParm m_uttOriginal;                     // contains utterance (pitch) parameters as read from file
    CPitchParm m_pitchParm;                     // contains pitch parameters
    CMusicParm m_musicParm;                     // contains music parameters
    CIntensityParm m_intensityParm;             // contains intensity parameters
    SDPParm m_SDPParm;                          // contains SDP parameters
    CSaString m_szMusicScore;                   // musical score string
    BOOL m_bProcessBackground;                  // TRUE = enable background processing when idle
    CProcessDoc m_ProcessDoc;                   // process the raw data
    CProcessAdjust * m_pProcessAdjust;
    CProcessFragments * m_pProcessFragments;    // data processing object
    CProcessLoudness * m_pProcessLoudness;      // data processing object
    CProcessSmoothLoudness * m_pProcessSmoothLoudness; // data processing object
    CProcessZCross * m_pProcessZCross;          // data processing object
    CProcessPitch * m_pProcessPitch;            // data processing object
    CProcessCustomPitch * m_pProcessCustomPitch; // data processing object
    CProcessSmoothedPitch * m_pProcessSmoothedPitch; // data processing object
    CProcessGrappl * m_pProcessGrappl;          // data processing object
    CProcessMelogram * m_pProcessMelogram;      // data processing object
    CProcessChange * m_pProcessChange;          // data processing object
    CProcessRaw * m_pProcessRaw;                // data processing object
    CProcessHilbert * m_pProcessHilbert;        // data processing object
    CProcessSpectrum * m_pProcessSpectrum;      // data processing object
    CProcessSpectrogram * m_pProcessSpectrogram; // data processing object
    CProcessSpectrogram * m_pProcessSnapshot; // data processing object
    CProcessWavelet * m_pProcessWavelet;        // data processing object  ARH 8/2/01 Added for wavelet graph
    CProcessFormants * m_pProcessFormants;      // data processing object
    CProcessFormantTracker * m_pProcessFormantTracker; // data processing object
    CProcessDurations * m_pProcessDurations;    // data processing object
    CProcessSDP * m_pProcessSDP[3];             // data processing object
    CProcessRatio * m_pProcessRatio;            // data processing object
    CProcessPOA * m_pProcessPOA;                // data processing object
    CProcessGlottis  * m_pProcessGlottis;       // data processing object
    CProcessTonalWeightChart * m_pProcessTonalWeightChart; // data processing object CLW 11/5/99
    CSegment * m_apSegments[ANNOT_WND_NUMBER];  // array of pointers to the segment objects
    CObArray * m_pCreatedFonts;
    BOOL m_bBlockBegin;                         // actual block begin flag
    CSaString m_szTempWave;                     // file name and path of temp file for wave data chunk
    wstring m_szTempConvertedWave;				// path of temp wave file converted from non-wave audio
    BOOL m_bWaveUndoNow;                        // TRUE, if wave file change is to undo
    int m_nCheckPointCount;                     // counter for checkpoints (wave undo)
    _bstr_t m_szMD5HashCode;                    // assigned from SA wave doc reader COM object
    bool m_bUsingTempFile;                      // FALSE, if audio file is non-wave (mp3, wma, etc.) or standard wave
    bool m_bAllowEdit;                          // TRUE if the file is editable

    CDlgAdvancedSegment * m_pDlgAdvancedSegment;
    CDlgAdvancedParseWords * m_pDlgAdvancedParseWords;
    CDlgAdvancedParsePhrases * m_pDlgAdvancedParsePhrases;

    bool m_bMultiChannel;
    int m_nSelectedChannel;
	CAutoSave m_AutoSave;
};

#endif
