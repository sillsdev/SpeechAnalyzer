#ifndef DLGFNKEYS_H
#define DLGFNKEYS_H

#include "sa_dlwnd.h"
#include "sa_wave.h"
#include "fnkeys.h"
#include "DlgWaveNotifyObj.h"
#include "IWaveNotifiable.h"

class CSaDoc;
class CSaView;

class CDlgFnKeys : public CDialog, public IWaveNotifiable {
public:
    CDlgFnKeys(CWnd * pParent = NULL); // standard constructor

    virtual void BlockFinished(UINT /*nLevel*/, DWORD /*dwPosition*/, UINT = 100);
    virtual void BlockStored(UINT /*nLevel*/, DWORD /*dwPosition*/, BOOL * /*bSaveOverride*/ = NULL);
    virtual void StoreFailed();
    virtual void EndPlayback();
    virtual HPSTR GetWaveData(DWORD /*dwPlayPosition*/, DWORD /*dwDataSize*/);

    CSliderVertical m_SliderSpeed;
    CSliderVertical m_SliderVolume;
    CSliderVertical m_SliderDelay;
    CSpinControl m_SpinVolume;
    CSpinControl m_SpinSpeed;
    CSpinControl m_SpinDelay;
    CString m_szTest;           // caption of test button
    BOOL m_bNoTest;             // TRUE, it test run not allowed
    BOOL m_bTestRunning;        // TRUE if test run
    CFnKeys m_fnKeys;           // function keys setup
    CWave * m_pWave;
    CDlgWaveNotifyObj m_NotifyObj;   // player notification object
    CDocument * m_pDoc;         // pointer to document
    CSaView * m_pView;          // pointer to view
    int m_nSelection;           // function key selection

    // Dialog Data
    enum { IDD = IDD_FNKEYS };
    UINT m_nDelay;
    UINT m_nSpeed;
    UINT m_nVolume;
    BOOL m_bRepeat;
    int m_nPlayMode;

    // Operations
protected:
    virtual void DoDataExchange(CDataExchange * pDX); // DDX/DDV support

public:
    void NoTest();
    void OnHelpFnKeys();

    // Generated message map functions
protected:
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
    DECLARE_MESSAGE_MAP()
};

#endif
