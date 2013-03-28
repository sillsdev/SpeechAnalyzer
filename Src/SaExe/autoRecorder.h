/////////////////////////////////////////////////////////////////////////////
// autoRecorder.h:
// Interface of the CDlgAutoRecorder (dialog)
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
#ifndef _AUTORECORDER__
#define _AUTORECORDER__

#include "sa_doc.h"
#include "sa_dlwnd.h"
#include "sa_wave.h"
#include "dlgaudio.h"
#include "playerrecorder.h"
#include "AlignInfo.h"

//###########################################################################
// CDlgAutoRecorder dialog

class CDlgAutoRecorder : public CDlgAudio
{

    enum eRecordState { WaitForSilence, WaitingForVoice, Recording, Stopping, Playing, Idle};
    enum eRecordMode { Disabled, Record, Stop, Monitor, Play};

#define MAX_SILENCE_LEVEL 5
#define MIN_VOICE_LEVEL 12

    // Construction/destruction/creation
public:
    CDlgAutoRecorder(CSaDoc * pDoc, CSaView * pParent, CSaView * pTarget, CAlignInfo & alignInfo); // standard constructor
    virtual ~CDlgAutoRecorder();

    // Attributes
private:
    void EnableRecVolume(BOOL bEnable);
    bool m_bStopPending;
    void StopWave();
    CAlignInfo m_AlignInfo;
    void StartShutdown();
    BOOL OnAssignOverlay(CSaView * pView);

    CSaView * m_pTargetUntested;  // Be careful, stale.  Maybe deleted under your feet.
    eRecordMode       m_eMode;          // recorder mode (record, play, stop...)
    eRecordMode       m_eOldMode;       // previous recorder mode

    HANDLE            m_hData;          // needed to get m_lpData
    HPSTR             m_lpData;         // pointer to wave data
    HMMIO             m_hmmioFile;      // mmio file handle
    TCHAR              m_szFileName[_MAX_PATH]; // file name of the temporary wave file
    MMCKINFO          m_mmckinfoSubchunk; // 'data' subchunk information
    MMCKINFO          m_mmckinfoParent; // 'RIFF' parents chunk information
    BOOL              m_bFileReady;     // TRUE, if temporary file OK
    BOOL              m_bFileApplied;   // TRUE, if temporary file ready to copy
    CLEDDisplay       m_LEDTotalTime;   // embedded control objects
    CLEDDisplay       m_LEDPosTime;
    CSliderVertical   m_SliderVolume;
    CSpinControl      m_SpinVolume;
    CSliderVertical   m_SliderRecVolume;
    CSpinControl      m_SpinRecVolume;
    CVUBar            m_VUBar;
    CFont             m_Font;           // special font for dialog controls
    CWave      *      m_pWave;
    CDlgWaveNotifyObj m_NotifyObj;      // recorder notification object
    CSaDoc      *     m_pDoc;           // pointer to document
    CSaView     *     m_pView;          // pointer to view
    DWORD             m_dwRecordSize;   // size of recorded data
    DWORD             m_dwPlayPosition;  // pointer in already played data

    // Dialog Data
    //{{AFX_DATA(CDlgAutoRecorder)
    enum { IDD = IDD_AUTORECORDER };
    UINT  m_nVolume;
    UINT  m_nRecVolume;
    //}}AFX_DATA

public:
    // Operations
protected:
    virtual void DoDataExchange(CDataExchange * pDX); // DDX/DDV support
    void SetTotalTime();    // set total time display
    void SetPositionTime(); // set position time display
    void HighPassFilter();  // filter the waveform
    BOOL CreateTempFile();  // create the temporary wave file for writing
    void DeleteTempFile();  // delete the temporary wave file
    void SetRecorderMode(eRecordMode eMode);  // set recorder mode (record, play, stop...)

public:
    UINT GetRecVolume();
    void SetRecVolume(int nVolume);
    CSaView * GetTarget();
    CString m_szTitle;
    void CleanUp();         // clean up memory and delete the temporary wave file
    virtual void BlockStored(UINT nLevel, DWORD dwPosition, BOOL * bSaveOverride = 0);
    virtual void BlockFinished(UINT nLevel, DWORD dwPosition, UINT = 100);
    virtual void StoreFailed();
    virtual void EndPlayback();
    virtual HPSTR GetWaveData(DWORD dwPlayPosition, DWORD dwDataSize);
    HMMIO GetFileHandle()
    {
        return m_hmmioFile;   // return handle to wave file
    }
    void OnHelpAutoRecorder();

    // Generated message map functions
protected:
    void ChangeState(eRecordState eState);
    BOOL Apply(); // apply wave file to document
    eRecordState m_eState;
    DWORD m_dwTickCount;    // current reference time

    //{{AFX_MSG(CDlgAutoRecorder)
    virtual BOOL OnInitDialog();
    afx_msg void OnStop();
    afx_msg void OnClose();
    afx_msg void OnCancel();
    afx_msg void OnVolumeSlide();
    afx_msg void OnVolumeScroll();
    afx_msg void OnKillfocusVolumeEdit();
    afx_msg void OnRecVolumeSlide();
    afx_msg void OnRecVolumeScroll();
    afx_msg void OnKillfocusRecVolumeEdit();
    afx_msg LRESULT OnMixerControlChange(WPARAM, LPARAM);
    afx_msg void OnPlay();
    //}}AFX_MSG
    afx_msg LRESULT OnAutoRestart(WPARAM, LPARAM);
    DECLARE_MESSAGE_MAP()
};

#endif
