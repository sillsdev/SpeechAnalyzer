/////////////////////////////////////////////////////////////////////////////
// playerRecorder.h:
// Interface of the CDlgPlayer (dialog)
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
#ifndef DLGPLAYER_H
#define DLGPLAYER_H

#include "sa_dlwnd.h"
#include "sa_wave.h"
#include "fnkeys.h"
#include "DlgWaveNotifyObj.h"
#include "IWaveNotifiable.h"

class CSaDoc;
class CSaView;

//###########################################################################
// CDlgPlayer dialog
struct SSpecific {
    double begin;
    double end;
};

class CDlgPlayer : public CDialog, public IWaveNotifiable {
public:
    CDlgPlayer(CWnd * pParent = NULL);
    ~CDlgPlayer();

    enum EMode { IDLE=0, STOPPED=1, PAUSED=2, PLAYING=3, RECORDING=4};

    BOOL Create();
    void SetTotalTime();                                // set total time display
    void SetPositionTime();                             // set position time display
    void EnableSpeedSlider(BOOL bState = TRUE);         // activate speed slider
    virtual void BlockStored(UINT nLevel, DWORD dwPosition, BOOL * bSaveOverride = NULL);
    virtual void BlockFinished(UINT nLevel, DWORD dwPosition, UINT nSpeed = 100);
    virtual void StoreFailed();
    virtual void EndPlayback();
    virtual HPSTR GetWaveData(DWORD dwPlayPosition, DWORD dwDataSize);
    bool SetPlayerMode(EMode mode, UINT nSubMode, BOOL bFullSize, BOOL bFnKey = FALSE, SSpecific * pSpecific = NULL);
    void ChangeView(CSaView * pView);                   // set new view
    bool IsPlaying();
    bool IsPaused();
    bool IsFullSize();
    bool IsTestRun();
    void OnHelpPlayer();
    static bool IsLaunched();
    static const char * GetMode(EMode mode);
    UINT GetSubmode();

protected:
    virtual void DoDataExchange(CDataExchange * pDX);   // DDX/DDV support
    void SetPlayerFullSize();                           // set the full size of the player
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
    afx_msg LRESULT OnSetupFnKeys(WPARAM, LPARAM);
    DECLARE_MESSAGE_MAP()

private:
    CLEDDisplay m_LEDTotalTime;         // embedded control objects
    CLEDDisplay m_LEDPosTime;
    CSliderVertical m_SliderSpeed;
    CSliderVertical m_SliderVolume;
    CSliderVertical m_SliderDelay;
    CVUBar m_VUBar;
    CSpinControl m_SpinVolume;
    CSpinControl m_SpinSpeed;
    CSpinControl m_SpinDelay;
    CToggleButton m_play;               // toggle button objects
    CToggleButton m_stop;
    CToggleButton m_pause;
    EMode m_nMode;                      // player mode (play, rewind, stop...)
    EMode m_nOldMode;                   // previous recorder mode
    UINT m_nSubMode;                    // player submode (play between cursors, window, file...)
    bool m_bFullSize;                   // if TRUE, player shows up in full size
    CRect m_rSize;                      // dialog width and height expanded and small
    CWave * m_pWave;
    CDlgWaveNotifyObj m_NotifyObj;      // player notification object
    CDocument * m_pDoc;                 // pointer to document
    CSaView * m_pView;                  // pointer to view
    DWORD m_dwPlayPosition;             // pointer in already played data
    bool m_bTestRunning;                // TRUE if function key dialog open
    UINT m_nOldVolume;
    UINT m_nOldSpeed;
    UINT m_nOldDelay;
    bool m_bOldRepeat;
    bool m_bFnKeySetting;               // TRUE, if function key setting
    UINT m_nFnKey;

    // Dialog Data
    enum { IDD = IDD_PLAYER };
	// the combo box selection control
    int m_nComboPlayMode;
    UINT m_nVolume;
    UINT m_nSpeed;
    UINT m_nDelay;
    BOOL m_bRepeat;

    static const UINT SubModeUndefined;
    static bool bLaunched;
};

#endif
