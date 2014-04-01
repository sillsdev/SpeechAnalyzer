#ifndef DLGRECORDER_H
#define DLGRECORDER_H

#include "sa_doc.h"
#include "sa_dlwnd.h"
#include "sa_wave.h"
#include "fnkeys.h"
#include "DlgWaveNotifyObj.h"
#include "IWaveNotifiable.h"

class CDlgRecorder : public CDialog, public IWaveNotifiable
{
public:
    CDlgRecorder(CWnd * pParent = NULL); // standard constructor
    virtual ~CDlgRecorder();

	enum EMode { IDLE=0, STOPPED=1, PAUSED=2, PLAYING=3, RECORDING=4};

    struct sourceInfo
    {
        BOOL bEnable;
        SourceParm source;

        sourceInfo()
        {
            source.nGender = -1;
        };

        void WriteProperties(CObjectOStream & obs);
        BOOL ReadProperties(CObjectIStream & obs);
    };

    static sourceInfo & GetStaticSourceInfo();
    UINT GetRecVolume();
    void SetRecVolume(int nVolume);
    BOOL CreateTempFile();		// create the temporary wave file for writing
    void DeleteTempFile();		// delete the temporary wave file
    void CleanUp();				// clean up memory and delete the temporary wave file
    void OnHelpRecorder();
    virtual void BlockStored(UINT nLevel, DWORD dwPosition, BOOL * bSaveOverride = NULL);
    virtual void BlockFinished(UINT nLevel, DWORD dwPosition, UINT = 100);
    virtual void StoreFailed();
    virtual void EndPlayback();
    virtual HPSTR GetWaveData(DWORD dwPlayPosition, DWORD dwDataSize);
    void SetRecorderMode(EMode mode); // set recorder mode (record, play, stop...)
    HMMIO GetFileHandle();
    BOOL Apply(CDocument *);	// apply wave file to document
    void ClearFileName();
    CSaDoc * GetDocument();

protected:
    virtual void DoDataExchange(CDataExchange * pDX); // DDX/DDV support
    void SetTotalTime();    // set total time display
    void SetPositionTime(); // set position time display
    void SetSettingsText(); // set the settings text
    BOOL CloseRecorder();   // close the recorder
    void HighPassFilter();  // filter the waveform

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
    DECLARE_MESSAGE_MAP()

private:
    void EnableRecVolume(BOOL bEnable);
    HANDLE m_hData;					// needed to get m_lpPlayData
    HPSTR m_lpPlayData;				// pointer to wave data
    HMMIO m_hmmioFile;				// mmio file handle
    TCHAR m_szFileName[_MAX_PATH];	// file name of the temporary wave file
    MMCKINFO m_mmckinfoSubchunk;	// 'data' subchunk information
    MMCKINFO m_mmckinfoParent;		// 'RIFF' parent chunk information
    BOOL m_bFileReady;				// TRUE, if temporary file OK
    BOOL m_bFileApplied;			// TRUE, if temporary file ready to copy
    CLEDDisplay m_LEDTotalTime;     // embedded control objects
    CLEDDisplay m_LEDPosTime;
    CSliderVertical m_SliderVolume;
    CSpinControl m_SpinVolume;
    CSliderVertical m_SliderRecVolume;
    CSpinControl m_SpinRecVolume;
    CVUBar m_VUBar;
    CToggleButton m_record;         // bitmap buttons
    CToggleButton m_stop;
    CToggleButton m_pause;
    CToggleButton m_play;
    EMode m_nMode;					// recorder mode (record, play, stop...)
    EMode m_nOldMode;				// previous recorder mode
    CFont m_Font;					// special font for dialog controls
    CWave * m_pWave;
    CDlgWaveNotifyObj m_NotifyObj;  // recorder notification object
    CSaDoc * m_pDoc;				// pointer to document
    CSaView * m_pView;              // pointer to view
    DWORD m_dwRecordSize;			// size of recorded data
    DWORD m_dwPlayPosition;			// pointer in already played data
    UINT m_nVolume;
    UINT m_nRecVolume;
    enum { IDD = IDD_RECORDER };
};

#endif
