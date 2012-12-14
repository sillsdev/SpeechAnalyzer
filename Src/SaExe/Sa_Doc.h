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
//          SDM moved ParseParm & SegmentParm to CMainFrame
// 1.5Test11.0
//          SDM added AutoSnapUpdate
// 1.5Test11.1A
//          RLJ Added ParseParm back in (leaving ParseParmDefaults in MainFrame)
//          RLJ Added SegmentParm back in (leaving ParseSegmentDefaults in MainFrame)
//
// 09/25/00 DDO Got rid of the m_bCheckedTWC member var. I rewrote some stuff
//              in sa_dlg so it's no longer needed.
/////////////////////////////////////////////////////////////////////////////
#ifndef _SA_DOC_H
#define _SA_DOC_H

#include "undoredo.h"
#include "CSaString.h"
#include "Appdefs.h"
#include "saParm.h"
#include "sourceParm.h"
#include "ISa_Doc.h"
#include "TranscriptionData.h"
#include "DlgAutoReferenceData.h"

#import "speechtoolsutils.tlb" no_namespace named_guids
#import "st_audio.tlb" no_namespace named_guids

//###########################################################################
// CSaDoc document

class CSaView;
class CFontTable;
class CProcessDoc;
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
class CHilbert;
class CProcessSpectrum;
class CProcessSpectrogram;
class CProcessWavelet;				// ARH 8/2/01 Added for wavelet graph
class CProcessFormants;
class CFormantTracker;
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

class CAlignInfo 
{

public:
	CAlignInfo() { bValid = FALSE; }
	bool bValid;
	double dTotalLength;			// in seconds
	double dStart;				    // in seconds
};

class CSaDoc : public CUndoRedoDoc, public ISaDoc
{   

	DECLARE_DYNCREATE(CSaDoc)

	// Construction/destruction/creation
protected:
	CSaDoc();

	// Attributes
private:
	bool				m_bAudioModified;						// dirty flag for audio data
	bool				m_bTransModified;						// dirty flag for transcription data
	bool				m_bTempOverlay;
	bool				m_bMultiChannel;
	int					m_ID;               // document ID
	int					m_nWbProcess;       // workbench process number
	CFileStatus			m_fileStat;         // file status information
	CSaString			m_szRawDataWrk[10]; // wave working temporary files
	DWORD				m_dwDataSize;       // size of the data subchunk
	FmtParm				m_fmtParm;          // contains format parameters
	SaParm				m_saParm;           // contains sa parameters
	SourceParm			m_sourceParm;       // contains source parameters
	UttParm				m_uttParm;          // contains utterance (pitch) parameters
	UttParm				m_uttOriginal;      // contains utterance (pitch) parameters as read from file
	PitchParm			m_pitchParm;        // contains pitch parameters
	MusicParm			m_musicParm;        // contains music parameters
	IntensityParm		m_intensityParm;    // contains intensity parameters
	SDPParm				m_SDPParm;          // contains SDP parameters
	CSaString			m_szMusicScore;     // musical score string
	BOOL				m_bProcessBackground;   // TRUE = enable background processing when idle
	CProcessDoc*		m_pProcessUnprocessed;
	CProcessAdjust*		m_pProcessAdjust;
	CProcessFragments*	m_pProcessFragments; // data processing object
	CProcessLoudness*	m_pProcessLoudness; // data processing object
	CProcessSmoothLoudness* m_pProcessSmoothLoudness; // data processing object
	CProcessZCross*		m_pProcessZCross;   // data processing object
	CProcessPitch*		m_pProcessPitch;    // data processing object
	CProcessCustomPitch* m_pProcessCustomPitch; // data processing object
	CProcessSmoothedPitch* m_pProcessSmoothedPitch; // data processing object
	CProcessGrappl*		m_pProcessGrappl;   // data processing object
	CProcessMelogram*   m_pProcessMelogram;   // data processing object
	CProcessChange*		m_pProcessChange;   // data processing object
	CProcessRaw*		m_pProcessRaw;   // data processing object
	CHilbert*			m_pProcessHilbert;   // data processing object
	CProcessSpectrum*	m_pProcessSpectrum; // data processing object
	CProcessSpectrogram* m_pProcessSpectrogram; // data processing object
	CProcessSpectrogram* m_pProcessSnapshot; // data processing object
	CProcessWavelet*	m_pProcessWavelet; // data processing object  ARH 8/2/01 Added for wavelet graph
	CProcessFormants*	m_pProcessFormants; // data processing object
	CFormantTracker*	m_pProcessFormantTracker; // data processing object
	CProcessDurations*	m_pProcessDurations; // data processing object
	CProcessSDP*		m_pProcessSDP[3];      // data processing object
	CProcessRatio*		m_pProcessRatio;    // data processing object
	CProcessPOA*		m_pProcessPOA;      // data processing object
	CProcessGlottis*	m_pProcessGlottis;  // data processing object
	CProcessTonalWeightChart* m_pProcessTonalWeightChart; // data processing object CLW 11/5/99
	CSegment*			m_apSegments[ANNOT_WND_NUMBER]; // array of pointers to the segment objects
	CStringArray*		m_pFontFaces;  // array of graph font face strings
	CUIntArray*			m_pFontSizes;// array of graph font sizes
	CObArray*			m_pCreatedFonts;
	HPSTR				m_lpData;           // pointer to wave data
	DWORD				m_dwRdBufferOffset; // actual read buffer offset
	BOOL				m_bBlockBegin;      // actual block begin flag
	CSaString			m_szTempWave;       // file name and path of temp file for wave data chunk
	CSaString			m_szTempConvertedWave;	// path of temp wave file converted from non-wave audio
	BOOL				m_bWaveUndoNow;     // TRUE, if wave file change is to undo
	int					m_nCheckPointCount; // counter for checkpoints (wave undo)
	_bstr_t				m_szMD5HashCode;    // assigned from SA wave doc reader COM object
	bool				m_bUsingTempFile;	// FALSE, if audio file is non-wave (mp3, wma, etc.) or standard wave
	bool				m_bAllowEdit;		// TRUE if the file is editable

	CDlgAdvancedSegment * m_pDlgAdvancedSegment;
	CDlgAdvancedParseWords * m_pDlgAdvancedParseWords;
	CDlgAdvancedParsePhrases * m_pDlgAdvancedParsePhrases;

	// Operations
public:
	void				SetAudioModifiedFlag(bool bMod = true) {m_bAudioModified = bMod;}
	bool				IsAudioModified() const {return m_bAudioModified;}
	void				SetTransModifiedFlag(bool bMod = true) {m_bTransModified = bMod;}
	bool				IsTransModified() const {return m_bTransModified;}
	bool				IsMultiChannel() const { return m_bMultiChannel;}
	void				SetID(int nID) {m_ID = nID;}				// set document ID
	int					GetID() {return m_ID;}
	void				SetWbProcess(int nProcess) {m_nWbProcess = nProcess;} // set workbench process number
	int					GetWbProcess() {return m_nWbProcess;}		// return workbench process number
	CFileStatus*		GetFileStatus() {return &m_fileStat;}		// pointer to file status structure
	FmtParm*			GetFmtParm() {return &m_fmtParm;}			// pointer to format parameters structure
	SaParm*				GetSaParm() {return &m_saParm;}				// pointer to sa parameters structure
	SourceParm*			GetSourceParm() {return &m_sourceParm;}		// pointer to source parameters structure
	int					GetGender();                               // returns gender: 0 = male, 1 = female, 2 = child
	// guesses if undefined in source parameters
	const UttParm*		GetUttParm()      {return &m_uttParm;}     // pointer to utterance parameters structure
	void				GetFmtParm(FmtParm*);                      // get a copy of the format parameters structure
	void				GetSaParm(SaParm*);                        // get a copy of the sa parameters structure
	void				GetUttParm(UttParm*, BOOL bOriginal=FALSE); // get a copy of the utterance parameters structure
	void				SetFmtParm(FmtParm*, BOOL bAdjustSpectro = TRUE); // set format parameters structure
	void				SetSaParm(SaParm*);                        // set sa parameters structure
	void				SetUttParm(const UttParm*, BOOL bOriginal = FALSE);      // set utterance parameters structure
	const PitchParm*    GetPitchParm() const {return &m_pitchParm;}   // pointer to pitch parameters structure
	void SetPitchParm(const PitchParm &parm) { m_pitchParm = parm;}
	const MusicParm*    GetMusicParm() const {return &m_musicParm;}   // pointer to music parameters structure
	void SetMusicParm(const MusicParm &parm) { m_musicParm = parm;}
	const IntensityParm&    GetIntensityParm() const {return m_intensityParm;}   // pointer to music parameters structure
	void SetIntensityParm(const IntensityParm &parm) { m_intensityParm = parm;}
	SDPParm*			GetSDPParm()      {return &m_SDPParm;}     // pointer to SDP parameters structure
	DWORD				GetDataSize();                             // return wave source data size
	DWORD				GetUnprocessedDataSize() {
		// return sampled data size from wave file
		return m_dwDataSize;
	} 
	double				GetTimeFromBytes(DWORD); // return the length of the sampled data in seconds
	DWORD				GetBytesFromTime(double); // return the length of the sampled data in bytes
	HPSTR				GetWaveData(DWORD dwOffset, BOOL bBlockBegin = FALSE); // return wave source data pointer on given position (offset)
	int					GetWaveData(DWORD dwOffset, BOOL*); // return wave source (one sample) on given position (offset)
	HPSTR				GetUnprocessedWaveData(DWORD dwOffset, BOOL bBlockBegin = FALSE, BOOL bAdjusted = TRUE); // return wave data pointer on given position (offset)
	DWORD				GetWaveBufferIndex();    // return index for wave source data buffer
	CSaString			GetMusicScore() {return m_szMusicScore;}
	BOOL				IsBackgroundProcessing() {return m_bProcessBackground;}
	BOOL				EnableBackgroundProcessing(BOOL bState = TRUE) {BOOL result = m_bProcessBackground;m_bProcessBackground = bState;return result;}  // background processing: TRUE = enabled
	CProcessDoc*		GetUnprocessed() {return m_pProcessUnprocessed;}  // process pointer to Unprocessed
	CProcessAdjust*		GetAdjust() {return m_pProcessAdjust;}  // process pointer to adjust
	CProcessFragments*	GetFragments() {return m_pProcessFragments;}  // process pointer to fragment object
	CProcessLoudness*	GetLoudness() {return m_pProcessLoudness;}  // process pointer to loudness object
	CProcessSmoothLoudness* GetSmoothLoudness() {return m_pProcessSmoothLoudness;} // process pointer to smooth loudness object
	CProcessZCross*		GetZCross() {return m_pProcessZCross;}    // process pointer to zero crossings object
	CProcessPitch*		GetPitch() {return m_pProcessPitch;}     // process pointer to pitch object
	CProcessCustomPitch* GetCustomPitch() {return m_pProcessCustomPitch;} // process pointer to custom pitch object
	CProcessSmoothedPitch* GetSmoothedPitch() {return m_pProcessSmoothedPitch;} // process pointer to smoothed pitch object
	CProcessChange*		GetChange() {return m_pProcessChange;}		// process pointer to change object
	CProcessRaw*		GetRaw() {return m_pProcessRaw;}			// process pointer to change object
	CHilbert*			GetHilbert() {return m_pProcessHilbert;}    // process pointer to change object
	CProcessSpectrogram* GetSpectrogram(bool bRealTime);			// returns either the spectrogram or snapshot process dependent on flag
	CProcessWavelet*	GetWavelet() {return m_pProcessWavelet;}	// process pointer to wavelet object  ARH 8/2/01 added for wavelet graph
	CProcessSpectrum*	GetSpectrum() {return m_pProcessSpectrum;}  // process pointer to spectrum object
	CProcessGrappl*		GetGrappl() {return m_pProcessGrappl;}		// process pointer to grappl object
	CProcessMelogram*   GetMelogram() {return m_pProcessMelogram;}  // process pointer to melogram object
	CProcessFormants*	GetFormants() {return m_pProcessFormants;}  // process pointer to spectrogram object
	CFormantTracker*	GetFormantTracker() {return m_pProcessFormantTracker;}  // process pointer to spectrogram object
	CProcessDurations*	GetDurations() {return m_pProcessDurations; } // process pointer to phonetic segment durations
	CProcessSDP*		GetSDP(int nIndex);							// process pointer to SDP object
	CProcessRatio*		GetRatio() {return m_pProcessRatio;}        // process pointer to ratio object
	CProcessPOA*		GetPOA() {return m_pProcessPOA;}            // process pointer to vocal tract model for finding place of articulation
	CProcessGlottis*	GetGlottalWave(){return m_pProcessGlottis;} // process pointer to glottal waveform object
	CProcessTonalWeightChart* GetTonalWeightChart() {return m_pProcessTonalWeightChart;} // process pointer to tonal weighting chart CLW 11/8/99
	CSegment*			GetSegment(int nIndex);						// get the pointers to a segment object
	CGlossSegment *		GetGlossSegment();
	CFontTable*			GetFont(int nIndex) {return (CFontTable*)m_pCreatedFonts->GetAt(nIndex);} // return font size
	CSaString			GetMeasurementsString(DWORD dwOffset, DWORD dwLength, BOOL* pbRes);

	void				DestroyAdvancedSegment();
	void				DestroyAdvancedParse();
	void				CreateFonts(); // set and create fonts
	CDocTemplate *		GetTemplate() { return m_pDocTemplate; }
	void				AutoSnapUpdate(void);
	// methods for saving the window settings and program options.
	void				WriteProperties(Object_ostream& obs);
	static BOOL			ReadProperties(Object_istream& obs);
	static BOOL			ReadPropertiesOfViews(Object_istream& obs, const CSaString & str);
	void				DeleteSegmentContents(Annotations type);
	CSaString			GetLastTranscriptionImport();

protected:
	virtual void		DeleteContents();
	virtual BOOL		OnNewDocument();
	virtual BOOL		OnOpenDocument(const TCHAR* pszPathName);
	virtual BOOL		OnSaveDocument(const TCHAR* pszPathName);
	virtual BOOL		OnSaveDocument(const TCHAR* pszPathName, BOOL bSaveAudio);
	virtual BOOL		CopyWaveToTemp(const TCHAR* pszSourcePathName, CAlignInfo info);
	virtual BOOL		SaveModified(); // return TRUE if ok to continue

private:
	const CSaString&	GetRawDataWrk(int nIndex) const {return m_szRawDataWrk[nIndex];}
	virtual BOOL		CopyWaveToTemp(const TCHAR* pszSourcePathName, const TCHAR* pszTempPathName = NULL, BOOL bInsert=FALSE, DWORD dwPos=0);
	void				SetMultiChannelFlag(bool bMultiChannel) { m_bMultiChannel = bMultiChannel;}
	bool				SplitMultiChannelTempFile(int channels, int selectedChannel);
	CSaString			SetFileExtension(CSaString fileName, CSaString fileExtension);

	// Methods for loading a wave file and all it's transcription data.
	BOOL				ReadRiff(const TCHAR* pszPathName);
	BOOL				LoadTranscriptionData(const TCHAR* pszPathName, BOOL bTemp);
	void				ReadNonSegmentData(ISaAudioDocumentReaderPtr saAudioDocRdr);
	void				ReadTranscription(int transType, ISaAudioDocumentReaderPtr saAudioDocRdr);
	void				ReadGlossPosAndRefSegments(ISaAudioDocumentReaderPtr saAudioDocRdr);
	void				ReadScoreData(ISaAudioDocumentReaderPtr saAudioDocRdr);

	// Methods for saving a wave file and all it's transcription data.
	DWORD				WriteRiff(const TCHAR* pszPathName);
	void				WriteNonSegmentData(DWORD dwDataSize, ISaAudioDocumentWriterPtr saAudioDocWriter);
	void				WriteTranscription(int transType, ISaAudioDocumentWriterPtr saAudioDocWriter);
	void				WriteGlossPosAndRefSegments(ISaAudioDocumentWriterPtr saAudioDocWriter);
	void				WriteScoreData(ISaAudioDocumentWriterPtr saAudioDocWriter);
	
	CString				GenerateSplitName( CSaView* pView, EFilenameConvention convention, int index);
	CString				GeneratePhraseSplitName( CSaView* pView, Annotations type, int index);
	CString				FilterName( CString text);
	bool				CreateFolder( CString folder);
	bool				ExportWord( int & count, Annotations type, EFilenameConvention convention, CString path);
	bool				ExportPhrase(Annotations type, int & count, CString path);

public:
	void				GetAlignInfo( CAlignInfo & alignInfo);
	void				SetTempOverlay();
	bool				IsTempOverlay();
	virtual void		OnCloseDocument();
	virtual BOOL		DoFileSave();

	BOOL				LoadDataFiles(const TCHAR* pszPathName, bool bTemp = false);
	BOOL				WriteDataFiles(const TCHAR* pszPathName, BOOL bSaveAudio = TRUE, BOOL bIsClipboardFile = FALSE);
	bool				GetWaveFormatParams(const TCHAR* pszPathName, FmtParm &fmtParm, DWORD & dwDataSize);
	bool				IsStandardWaveFormat(const TCHAR* pszPathName);
	bool				IsMultiChannelWave(const TCHAR* pszPathName, int & channels);
	DWORD				CheckWaveFormatForPaste(const TCHAR* pszPathName);
	DWORD				CheckWaveFormatForOpen(const TCHAR* pszPathName);
	bool				ConvertToWave(const TCHAR* pszPathName);
	BOOL				InsertTranscriptions(const TCHAR* pszPathName, DWORD dwPos);
	BOOL				InsertTranscription(int transType, ISaAudioDocumentReaderPtr saAudioDocRdr, DWORD dwPos);
	void				InsertGlossPosAndRefTranscription(ISaAudioDocumentReaderPtr saAudioDocRdr, DWORD dwPos);
	virtual BOOL		CopyFile(const TCHAR* pszSourceName, const TCHAR* pszTargetName, DWORD dwStart=0, DWORD dwMax=0xFFFFFFFF, BOOL bTruncate=TRUE);
	BOOL				CopySectionToNewWavFile(DWORD dwSectionStart, DWORD dwSectionLength, LPCTSTR szNewWave);
	void				ApplyWaveFile(const TCHAR * pszFileName, DWORD dwDataSize, BOOL bInialUpdate=TRUE);		// apply a new recorded wave file
	void				ApplyWaveFile(const TCHAR * pszFileName, DWORD dwDataSize, CAlignInfo alignInfo);		// Update for rt auto-pitch
	DWORD				SnapCursor(CURSOR_SELECT nCursorSelect, DWORD dwCursorOffset, DWORD dwLowerLimit, DWORD dwUpperLimit,
									SNAP_DIRECTION nSnapDirection = SNAP_BOTH,
									CURSOR_ALIGNMENT nCursorAlignment = ALIGN_USER_SETTING); // align cursors
	DWORD				SnapCursor(CURSOR_SELECT nCursorSelect, DWORD dwCursorOffset, SNAP_DIRECTION nSnapDirection = SNAP_BOTH);
	void				InvalidateAllProcesses(); // invalidates all the graph processes
	void				RestartAllProcesses(); // invalidates all the processes
	BOOL				AnyProcessCanceled(); // invalidates all the processes
	BOOL				WorkbenchProcess(BOOL bInvalidate = FALSE, BOOL bRestart = FALSE); // process the actually selected workbench process
	BOOL				PutWaveToClipboard(DWORD dwSectionStart, DWORD dwSectionLength, BOOL bDelete = FALSE); // copies wave data out of the wave file
	BOOL				PasteClipboardToWave(DWORD dwPastePos); // pastes wave data into the wave file
	void				DeleteWaveFromUndo(); // deletes a wave undo entry from the undo list
	void				UndoWaveFile(); // undo a wave file change
	BOOL				IsWaveToUndo() {return m_bWaveUndoNow;} // return TRUE, if wave file change is to undo
	void				CopyProcessTempFile();
	void				AdjustSegments(DWORD dwSectionStart, DWORD dwSectionLength, BOOL bShrink); // adjust segments to new file size
	BOOL				UpdateSegmentBoundaries(BOOL bOverlap); // update segment boundaries
	BOOL				UpdateSegmentBoundaries(BOOL bOverlap, int nAnnotation, int nSelection, DWORD start, DWORD stop);
	BOOL				AutoSnapUpdateNeeded(void);
	virtual				~CSaDoc();
	virtual void		SerializeForUndoRedo(CArchive& ar);   // overridden for document i/o
	BOOL				AdvancedParsePhrase();
	BOOL				AdvancedParseWord();
	BOOL				AdvancedParseAuto();
	BOOL				AdvancedSegment();
	virtual HMENU		GetDefaultMenu(); // get menu depending on state
	virtual HACCEL		GetDefaultAccel();
	virtual void		NotifyAutoPitchDone(void *pCaller);
	virtual void		NotifyFragmentDone(void *pCaller);

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSaDoc)
public:
	virtual void		Serialize(CArchive& ar);
	//}}AFX_VIRTUAL

#ifdef _DEBUG
	virtual void		AssertValid() const;
	virtual void		Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	// SDM 1.06.5 removed unused mesages
	//{{AFX_MSG(CSaDoc)
	afx_msg void OnUpdateFileSave(CCmdUI* pCmdUI);
	afx_msg void OnFileSaveAs();
	afx_msg void OnUpdateFileSaveAs(CCmdUI* pCmdUI);
	afx_msg void OnFileSplit();
	afx_msg void OnUpdateFileSplit(CCmdUI* pCmdUI);
	afx_msg void OnAdvancedParseWords();
	afx_msg void OnUpdateAdvancedParseWords(CCmdUI* pCmdUI);
	afx_msg void OnAdvancedParsePhrases();
	afx_msg void OnUpdateAdvancedParsePhrases(CCmdUI* pCmdUI);
	afx_msg void OnAdvancedSegment();
	afx_msg void OnUpdateAdvancedSegment(CCmdUI* pCmdUI);
	afx_msg void OnAutoAlign();
	afx_msg void OnUpdateAutoAlign(CCmdUI* pCmdUI);
	afx_msg void OnAutoReferenceData();
	afx_msg void OnUpdateAutoReferenceData(CCmdUI* pCmdUI);
	afx_msg void OnUpdateBoundaries();
	afx_msg void OnUpdateUpdateBoundaries(CCmdUI* pCmdUI);
	afx_msg void OnAutoSnapUpdate();
	afx_msg void OnUpdateAutoSnapUpdate(CCmdUI* pCmdUI);
	afx_msg void OnToolsAdjustInvert();
	afx_msg void OnUpdateToolsAdjustInvert(CCmdUI* pCmdUI);
	afx_msg void OnToolsAdjustNormalize();
	afx_msg void OnUpdateToolsAdjustNormalize(CCmdUI* pCmdUI);
	afx_msg void OnToolsAdjustZero();
	afx_msg void OnUpdateToolsAdjustZero(CCmdUI* pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	bool IsTempFile();
	bool CanEdit();
	const CSaString BuildString( int nSegment);
	const CSaString BuildImportString( BOOL gloss, BOOL phonetic, BOOL phonemic, BOOL orthographic);
	const CTranscriptionData ImportTranscription( CSaString & filename, BOOL gloss, BOOL phonetic, BOOL phonemic, BOOL orthographic);

protected:
	void AlignTranscriptionData( CTranscriptionDataSettings & settings);
	void AlignTranscriptionDataByRef( CTranscriptionData & td);

	// cached dialogs
	CDlgAutoReferenceData autoReferenceDlg;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif //_SA_DOC_H
