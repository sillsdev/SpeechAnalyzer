/////////////////////////////////////////////////////////////////////////////
// playerRecorder.h:
// Interface of the CDlgWaveNotifyObj (notify object)
//                  CDlgPlayer (dialog)
//                  CDlgRecorder (dialog)
//                  CDlgRecorderOptions (dialog)
//                  CDlgFnKeys (dialog)
//
// Author: Urs Ruchti
// copyright 2000 JAARS Inc. SIL
//
// Revision History
//
// 01/12/2000
//    SDM   Extract from sa_dlg.h
//
/////////////////////////////////////////////////////////////////////////////
#ifndef _PLAYER_RECORDER_H

#include "sa_doc.h"
#include "sa_dlwnd.h"

#define _PLAYER_RECORDER_H

#include "sa_wave.h"
#include "dlgaudio.h"
#include "fnkeys.h"

//###########################################################################
// CDlgWaveNotifyObj notify object

class CDlgWaveNotifyObj : public CWaveNotifyObj
{

  // Construction/destruction/creation
public:
  CDlgWaveNotifyObj();
  ~CDlgWaveNotifyObj();

  // Attributes
private:
  CDlgAudio * m_pDlg;

  // Operations
public:
  void Attach(CDlgAudio * pDlg) {m_pDlg = pDlg;}
  virtual void BlockFinished(UINT nLevel, DWORD dwPosition, UINT = 100);
  virtual void BlockStored(UINT nLevel, DWORD dwPosition, BOOL *bSaveOverride = NULL);
  virtual void StoreFailed();
  virtual void EndPlayback();
  virtual HPSTR GetWaveData(CView* pView, DWORD dwPlayPosition, DWORD dwDataSize);
};

//###########################################################################
// CDlgPlayer dialog

struct SSpecific
{
  double begin;
  double end;
};

class CDlgPlayer : public CDlgAudio
{

  // Construction/destruction/creation
public:
  CDlgPlayer(CWnd* pParent = NULL); // standard constructor
  ~CDlgPlayer();
  BOOL Create();

  // Attributes
public:
  static BOOL bPlayer;
private:
  CLEDDisplay     m_LEDTotalTime;  // embedded control objects
  CLEDDisplay     m_LEDPosTime;
  CSliderVertical m_SliderSpeed;
  CSliderVertical m_SliderVolume;
  CSliderVertical m_SliderDelay;
  CVUBar          m_VUBar;
  CSpinControl    m_SpinVolume;
  CSpinControl    m_SpinSpeed;
  CSpinControl    m_SpinDelay;
  CToggleButton   m_play;          // toggle button objects
  CToggleButton   m_stop;
  CToggleButton   m_pause;
  UINT            m_nMode;         // player mode (play, rewind, stop...)
  UINT            m_nOldMode;      // previous recorder mode
  UINT            m_nSubMode;      // player submode (play between cursors, window, file...)
  BOOL            m_bFullSize;     // if TRUE, player shows up in full size
  CRect           m_rSize;         // dialog width and height expanded and small
  CWave*          m_pWave;
  CDlgWaveNotifyObj m_NotifyObj;   // player notification object
  CDocument*      m_pDoc;          // pointer to document
  CSaView*        m_pView;         // pointer to view
  DWORD           m_dwPlayPosition; // pointer in already played data
  BOOL            m_bTestRunning;  // TRUE if function key dialog open
  UINT            m_nOldVolume;
  UINT            m_nOldSpeed;
  UINT            m_nOldDelay;
  BOOL            m_bOldRepeat;
  BOOL            m_bFnKeySetting; // TRUE, if function key setting
  UINT            m_nFnKey;

  // Dialog Data
  //{{AFX_DATA(CDlgPlayer)
  enum { IDD = IDD_PLAYER };
  int     m_nComboPlayMode;
  UINT    m_nVolume;
  UINT    m_nSpeed;
  UINT    m_nDelay;
  BOOL    m_bRepeat;
  //}}AFX_DATA

  static const UINT SubModeUndefined;

  // Operations
protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  void SetPlayerFullSize(); // set the full size of the player
public:
  void SetTotalTime();    // set total time display
  void SetPositionTime (); // set position time display
  void EnableSpeedSlider(BOOL bState = TRUE);  // activate speed slider
  
	virtual void BlockStored(UINT /*nLevel*/, DWORD /*dwPosition*/, BOOL * /* bSaveOverride*/ = NULL) {	ASSERT(FALSE);};	// not supported on player
  virtual void BlockFinished(UINT nLevel, DWORD dwPosition, UINT nSpeed = 100);
  virtual void StoreFailed() { ASSERT(FALSE);};
  virtual void EndPlayback();
  virtual HPSTR GetWaveData(DWORD /*dwPlayPosition*/, DWORD /*dwDataSize*/) { return NULL;}; // The player does not have any data (recorder does)
  BOOL SetPlayerMode(UINT nMode, UINT nSubMode, BOOL bFullSize, BOOL bFnKey = FALSE, SSpecific *pSpecific = NULL);

  void ChangeView(CSaView* pView); // set new view
  BOOL IsPlaying() {return ((m_nMode == IDC_PLAY)||m_bFnKeySetting);} // return TRUE if player is playing
  BOOL IsFullSize() {return m_bFullSize;} // return TRUE if player has full size
  BOOL IsTestRun() {return m_bTestRunning;} // return TRUE if function key dialog open
	void OnHelpPlayer();

  // Generated message map functions
protected:
  //{{AFX_MSG(CDlgPlayer)
  virtual BOOL OnInitDialog();
  afx_msg void OnPlay();
  afx_msg void OnStop();
  afx_msg void OnPause();
  afx_msg void OnVolumeSlide();
  afx_msg void OnVolumeScroll();
  afx_msg void OnKillfocusVolumeEdit();
  virtual void OnOK();
  virtual void OnCancel();
  afx_msg void OnSpeedSlide();
  afx_msg void OnSpeedScroll();
  afx_msg void OnKillfocusSpeedEdit();
  afx_msg void OnDelaySlide();
  afx_msg void OnDelayScroll();
  afx_msg void OnKillfocusDelayEdit();
  afx_msg void OnSelchangePlaymode();
  afx_msg void OnClose();
  afx_msg void OnRepeat();
  afx_msg void OnTimer(UINT nIDEvent);
  afx_msg void OnSetup();
  afx_msg void OnPlayControls();
  afx_msg LRESULT OnMixerControlChange(WPARAM, LPARAM);
  //}}AFX_MSG
  afx_msg LRESULT OnSetupFnKeys(WPARAM, LPARAM);
  DECLARE_MESSAGE_MAP()
};

//###########################################################################
// CDlgRecorder dialog

class CDlgRecorder : public CDlgAudio
{

  // Construction/destruction/creation
public:
  CDlgRecorder(CWnd* pParent = NULL); // standard constructor
	virtual ~CDlgRecorder()
	{
		if (m_pWave)
			delete m_pWave;
	}

  // Attributes
private:
	void EnableRecVolume(BOOL bEnable);
  HANDLE            m_hData;          // needed to get m_lpData
  HPSTR             m_lpData;         // pointer to wave data
  HMMIO             m_hmmioFile;      // mmio file handle
  TCHAR             m_szFileName[_MAX_PATH]; // file name of the temporary wave file
  MMCKINFO          m_mmckinfoSubchunk; // 'data' subchunk information
  MMCKINFO          m_mmckinfoParent; // 'RIFF' parent chunk information
  BOOL              m_bFileReady;     // TRUE, if temporary file OK
  BOOL              m_bFileApplied;   // TRUE, if temporary file ready to copy
  CLEDDisplay       m_LEDTotalTime;   // embedded control objects
  CLEDDisplay       m_LEDPosTime;
  CSliderVertical   m_SliderVolume;
  CSpinControl      m_SpinVolume;
  CSliderVertical   m_SliderRecVolume;
  CSpinControl      m_SpinRecVolume;
  CVUBar            m_VUBar;
  CToggleButton     m_record;         // bitmap buttons
  CToggleButton     m_stop;
  CToggleButton     m_pause;
  CToggleButton     m_play;
  UINT              m_nMode;          // recorder mode (record, play, stop...)
  UINT              m_nOldMode;       // previous recorder mode
  CFont             m_Font;           // special font for dialog controls
  CWave*            m_pWave;
  CDlgWaveNotifyObj m_NotifyObj;      // recorder notification object
  CSaDoc*           m_pDoc;           // pointer to document
  CSaView*          m_pView;          // pointer to view
  DWORD             m_dwRecordSize;   // size of recorded data
  DWORD             m_dwPlayPosition;  // pointer in already played data

  // Dialog Data
  //{{AFX_DATA(CDlgRecorder)
  enum { IDD = IDD_RECORDER };
  UINT  m_nVolume;
  UINT  m_nRecVolume;
  //}}AFX_DATA

public:
  struct sourceInfo
  {
    BOOL bEnable;
    SourceParm source;

    sourceInfo() { source.nGender = -1;};

    void WriteProperties(Object_ostream& obs);
    BOOL bReadProperties(Object_istream& obs);
  };

  static sourceInfo & GetStaticSourceInfo();

  // Operations
protected:
  virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support
  void SetTotalTime();    // set total time display
  void SetPositionTime(); // set position time display
  void SetSettingsText(); // set the settings text
  BOOL CloseRecorder();   // close the recorder
  void HighPassFilter();  // filter the waveform
public:
	UINT GetRecVolume();
	void SetRecVolume(int nVolume);
  BOOL CreateTempFile();  // create the temporary wave file for writing
  void DeleteTempFile();  // delete the temporary wave file
  void CleanUp();         // clean up memory and delete the temporary wave file
	void OnHelpRecorder();
  
	virtual void BlockStored(UINT nLevel, DWORD dwPosition, BOOL *bSaveOverride = NULL);
  virtual void BlockFinished(UINT nLevel, DWORD dwPosition, UINT = 100);
  virtual void StoreFailed();
  virtual void EndPlayback();
  virtual HPSTR GetWaveData(DWORD dwPlayPosition, DWORD dwDataSize);
  
	void SetRecorderMode(UINT nMode); // set recorder mode (record, play, stop...)
  HMMIO GetFileHandle() {return m_hmmioFile;} // return handle to wave file
  BOOL Apply(CDocument*); // apply wave file to document
  void ClearFileName() {m_szFileName[0] = 0;};
  CSaDoc* GetDocument();
  
  // Generated message map functions
protected:
  //{{AFX_MSG(CDlgRecorder)
  virtual BOOL OnInitDialog();
  afx_msg void OnRecord();
  afx_msg void OnPlay();
  afx_msg void OnStop();
  afx_msg void OnPause();
  afx_msg void OnClose();
  afx_msg void OnButtonClose();
  afx_msg void OnCancel();
  afx_msg void OnSettings();
  afx_msg void OnApply();
  afx_msg void OnVolumeSlide();
  afx_msg void OnVolumeScroll();
  afx_msg void OnKillfocusVolumeEdit();
  afx_msg void OnRecVolumeSlide();
  afx_msg void OnRecVolumeScroll();
  afx_msg void OnKillfocusRecVolumeEdit();
  afx_msg LRESULT OnMixerControlChange(WPARAM, LPARAM);
	afx_msg void OnMixer();
	//}}AFX_MSG
  DECLARE_MESSAGE_MAP()
};

//###########################################################################
// CDlgRecorderOptions dialog

class CDlgRecorderOptions : public CDialog
{

  // Construction/destruction/creation
public:
  CDlgRecorderOptions(CDlgRecorder* pParent); // standard constructor

  // Attributes
public:
  void SetSamplingRate(int nRate);
  unsigned int GetSamplingRate();
  void SetBitDepth(int nBits);
  short GetBitDepth();
  void SetHighpass(BOOL bHighpass);
  BOOL GetHighpass();
  void SetChannels(int nChannels);
  short GetChannels();
  CDlgRecorder& GetRecorder();
	void OnHelpRecorderOptions();

protected:
  // Dialog Data
  //{{AFX_DATA(CDlgRecorderOptions)
	enum { IDD = IDD_RECORDEROPTIONS };
  int     m_nRate;
  int     m_nBits;
  BOOL    m_bHighpass;
  int     m_nMode;
  BOOL	m_bSourceAuto;
  //}}AFX_DATA
  CDlgRecorder* const m_pRecorder;

  // Operations
protected:
  virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support

  // Generated message map functions
protected:
  //{{AFX_MSG(CDlgRecorderOptions)
  virtual BOOL OnInitDialog();
  afx_msg void OnDefault();
	afx_msg void OnSource();
	afx_msg void OnSourceAuto();
	//}}AFX_MSG
  DECLARE_MESSAGE_MAP()
};

//###########################################################################
// CDlgFnKeys dialog

class CDlgFnKeys : public CDlgAudio
{

  // Construction/destruction/creation
public:
  CDlgFnKeys(CWnd* pParent = NULL); // standard constructor

  // Attributes
public:
  // not supported
	virtual void BlockFinished(UINT /*nLevel*/, DWORD /*dwPosition*/, UINT = 100) { ASSERT(FALSE);};
  virtual void BlockStored(UINT /*nLevel*/, DWORD /*dwPosition*/, BOOL * /*bSaveOverride*/ = NULL) { ASSERT(FALSE);};
  virtual void StoreFailed() { ASSERT(FALSE);};
  virtual void EndPlayback();
  virtual HPSTR GetWaveData(DWORD /*dwPlayPosition*/, DWORD /*dwDataSize*/) { ASSERT(FALSE); return NULL;};
  
	CSliderVertical m_SliderSpeed;
  CSliderVertical m_SliderVolume;
  CSliderVertical m_SliderDelay;
  CSpinControl    m_SpinVolume;
  CSpinControl    m_SpinSpeed;
  CSpinControl    m_SpinDelay;
  CString         m_szTest;        // caption of test button
  BOOL            m_bNoTest;       // TRUE, it test run not allowed
  BOOL            m_bTestRunning;  // TRUE if test run
  FnKeys          m_fnKeys;        // function keys setup
  CWave*          m_pWave;
  CDlgWaveNotifyObj m_NotifyObj;   // player notification object
  CDocument*      m_pDoc;          // pointer to document
  CSaView*        m_pView;         // pointer to view
  int             m_nSelection;    // function key selection

  // Dialog Data
  //{{AFX_DATA(CDlgFnKeys)
  enum { IDD = IDD_FNKEYS };
  UINT    m_nDelay;
  UINT    m_nSpeed;
  UINT    m_nVolume;
  BOOL    m_bRepeat;
  int     m_nPlayMode;
  //}}AFX_DATA

  // Operations
protected:
  virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support

public:
  void NoTest() {m_bNoTest = TRUE;} // don't allow test run
	void OnHelpFnKeys();

  // Generated message map functions
protected:
  //{{AFX_MSG(CDlgFnKeys)
  virtual BOOL OnInitDialog();
  afx_msg void OnVolumeSlide();
  afx_msg void OnVolumeScroll();
  afx_msg void OnKillfocusVolumeEdit();
  afx_msg void OnSpeedSlide();
  afx_msg void OnSpeedScroll();
  afx_msg void OnKillfocusSpeedEdit();
  afx_msg void OnDelaySlide();
  afx_msg void OnDelayScroll();
  afx_msg void OnKillfocusDelayEdit();
  afx_msg void OnRepeat();
  afx_msg void OnSelchangeFnlist();
  virtual void OnOK();
  virtual void OnCancel();
  afx_msg void OnClose();
  afx_msg void OnTimer(UINT nIDEvent);
  afx_msg void OnTest();
  afx_msg LRESULT OnMixerControlChange(WPARAM, LPARAM);
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
};

#endif //_PLAYER_RECORDER_H
