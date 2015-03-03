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
#ifndef DLGAUTORECORDER_H
#define DLGAUTORECORDER_H

#include "sa_doc.h"
#include "sa_dlwnd.h"
#include "sa_wave.h"
#include "AlignInfo.h"
#include "DlgWaveNotifyObj.h"
#include "IWaveNotifiable.h"

//###########################################################################
// CDlgAutoRecorder dialog

class CDlgAutoRecorder : public CDialog, public IWaveNotifiable {

    enum ERecordState { WaitForSilence, WaitingForVoice, Recording, Stopping, PlayingRecording, PlayingOriginal, Idle};
    enum ERecordMode { Disabled, Record, Stop, Monitor, PlayRecording, PlayOriginal};

#define MAX_SILENCE_LEVEL 5
#define MIN_VOICE_LEVEL 12

    // Construction/destruction/creation
public:
    CDlgAutoRecorder(CSaDoc * pDoc, CSaView * pParent, CSaView * pTarget, CAlignInfo & alignInfo, int wholeFile); // standard constructor
    virtual ~CDlgAutoRecorder();

    void Create(CWnd * pParent);

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
    HMMIO GetFileHandle();
    void OnHelpAutoRecorder();
    afx_msg void OnRadioBetweenCursors();
    afx_msg void OnClickedRadioWholeFile();
    int GetPlayWholeFile();
    static bool IsLaunched();

    enum { IDD = IDD_AUTORECORDER };

protected:
    virtual void DoDataExchange(CDataExchange * pDX); // DDX/DDV support
    void SetTotalTime(DWORD val);        // set total time display
    double SetPositionTime(DWORD val);  // set position time display
    void HighPassFilter();              // filter the waveform
    BOOL CreateTempFile();              // create the temporary wave file for writing
    void DeleteTempFile();              // delete the temporary wave file
    void SetRecorderMode(ERecordMode eMode);  // set recorder mode (record, play, stop...)

    void ChangeState(ERecordState eState);
    const char * GetState(ERecordState eState);

    BOOL Apply();                       // apply wave file to document

    virtual BOOL OnInitDialog();
    afx_msg void OnPlayRecording();
    afx_msg void OnStopRecording();
    afx_msg void OnStopPlaying();
    afx_msg void OnPlaybackOriginal();
    afx_msg void OnStopOriginal();
    afx_msg void OnClose();
    afx_msg void OnCancel();
    afx_msg void OnVolumeSlide();
    afx_msg void OnVolumeScroll();
    afx_msg void OnKillfocusVolumeEdit();
    afx_msg void OnRecVolumeSlide();
    afx_msg void OnRecVolumeScroll();
    afx_msg void OnKillfocusRecVolumeEdit();
    afx_msg LRESULT OnMixerControlChange(WPARAM, LPARAM);
    afx_msg LRESULT OnAutoRestart(WPARAM, LPARAM);
    DECLARE_MESSAGE_MAP()

    ERecordState m_eState;
    DWORD m_dwTickCount;                // current reference time

private:
    void EnableRecVolume(BOOL bEnable);
    void StopWave();
    void StartShutdown();
    BOOL OnAssignOverlay(CSaView * pView);

    CSaView * m_pTargetUntested;            // Be careful, stale.  Maybe deleted under your feet.
    ERecordMode m_eMode;                    // recorder mode (record, play, stop...)
    ERecordMode m_eOldMode;                 // previous recorder mode

    bool m_bClosePending;                   // the close button was pressed
    CAlignInfo m_AlignInfo;
    double m_dSourceLength;                 // length of original source wave file
    double m_dRecordLength;                 // the amount data we will be recording.

    HANDLE m_hData;                         // needed to get m_lpRecData
    HPSTR m_lpRecData;                      // pointer to wave data
    HMMIO m_hmmioFile;                      // mmio file handle
    TCHAR m_szFileName[_MAX_PATH];          // file name of the temporary wave file
    MMCKINFO m_mmckinfoSubchunk;            // 'data' subchunk information
    MMCKINFO m_mmckinfoParent;              // 'RIFF' parents chunk information
    BOOL m_bFileReady;                      // TRUE, if temporary file OK
    BOOL m_bFileApplied;                    // TRUE, if temporary file ready to copy
    bool m_bRecordingAvailable;
    CLEDDisplay m_LEDTotalTime;             // embedded control objects
    CLEDDisplay m_LEDPosTime;
    CSliderVertical m_SliderVolume;
    CSpinControl m_SpinVolume;
    CSliderVertical m_SliderRecVolume;
    CSpinControl m_SpinRecVolume;
    CVUBar m_VUBar;
    CFont m_Font;                           // special font for dialog controls
    CWave * m_pWave;
    CDlgWaveNotifyObj m_NotifyObj;          // recorder notification object
    CSaDoc * m_pDoc;                        // pointer to document
    CSaView * m_pView;                      // pointer to view
    DWORD m_dwRecordSize;                   // size of recorded data
    DWORD m_dwPlayPosition;                 // pointer in already played data

    UINT m_nVolume;
    UINT m_nRecVolume;
    int m_nPlayWholeFile;

    virtual void PostNcDestroy();

    static bool bLaunched;
};

#endif
