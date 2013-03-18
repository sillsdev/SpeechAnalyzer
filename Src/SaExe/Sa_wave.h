/////////////////////////////////////////////////////////////////////////////

// sa_wave.h:
// Interface of the CWaveInDevice
//                  CWaveOutDevice
//                  CWaveNotifyObj
//                  CWave          classes.
// Author: Urs Ruchti
// copyright 1996 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////
#ifndef _SA_WAVE_H
#define _SA_WAVE_H

#include <mmsystem.h>
#include "dsp\wavewarp.h"
#include "dsp\hpfltr70.h"


//###########################################################################
// CWaveInDevice

class CWave;
class CWaveNotifyObj;
class CRecMixer;
class CPlayMixer;

struct SWaveHdrInfo {
    WAVEHDR * pHdr;
    int iBuffer;
};

class CWaveInDevice : public CWnd {

    // Construction/destruction/creation
public:
    CWaveInDevice();
    virtual ~CWaveInDevice();
private:
    BOOL Create();

    // Attributes
private:
    HWAVEIN m_hInDev;                      // input device handle
    CHighPassFilter70 * m_pHighPassFilter;
    CRecMixer * m_pMixer;
    int m_nBuffers;

    // Operations
public:
    BOOL Open(WAVEFORMATEX * pFormat); // device open
    BOOL CanDoFormat(WAVEFORMATEX * pFormat);
    BOOL Close();                            // device close
    void WaveInDone(CWave * pWave, WAVEHDR * pHdr);
    void Reset();
    BOOL Record(int nBuffer, CWave * pWave, BOOL bStart=FALSE);
    BOOL IsOpen() {
        return m_hInDev ? TRUE : FALSE;
    }
    BOOL AttachHighPassFilter(USHORT wSmpRate);
    CHighPassFilter70 * GetHighPassFilter() {
        return m_pHighPassFilter;
    }
    void DetachHighPassFilter() {
        delete m_pHighPassFilter;
        m_pHighPassFilter = NULL;
    }
    UINT GetVolume(BOOL * bResult = NULL);
    void SetVolume(UINT nVolume, BOOL * bResult = NULL);
    BOOL ShowMixer(BOOL bShow = TRUE);
    BOOL ConnectMixer(CWnd * pCallback);
    int GetPreparedBuffers() const {
        return m_nBuffers;
    }

    // Generated message map functions
protected:
    //{{AFX_MSG(CWaveInDevice)
    afx_msg LRESULT OnWimData(WPARAM w, LPARAM l);
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

//###########################################################################
// CWaveOutDevice

class CWaveOutDevice : public CWnd {

    // Construction/destruction/creation
public:
    CWaveOutDevice();
    virtual ~CWaveOutDevice();

private:
    BOOL Create();

    // Attributes
private:
    HWAVEOUT m_hOutDev;           // output device handle
private:
    CPlayMixer * m_pMixer;

    // Operations
public:
    BOOL Open(WAVEFORMATEX * pFormat);
    BOOL CanDoFormat(WAVEFORMATEX * pFormat);
    BOOL Close();
    BOOL Play(int nBuffer, UINT nVolume, CWave * pWave, BOOL bPause = FALSE);
    UINT GetVolume(BOOL * bResult = NULL);
    void SetVolume(UINT nVolume, BOOL * bResult = NULL);
    void WaveOutDone(CWave * pWave, WAVEHDR * pHdr);
    void Reset();
    BOOL IsOpen() {
        return m_hOutDev ? TRUE : FALSE;
    }
    BOOL ShowMixer(BOOL bShow = TRUE);
    BOOL ConnectMixer(CWnd * pCallback);

    // Generated message map functions
protected:
    //{{AFX_MSG(CWaveOutDevice)
    afx_msg LRESULT OnWomDone(WPARAM w, LPARAM l);
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

// some global items
extern CWaveOutDevice theDefaultWaveOutDevice;

//###########################################################################
// CWaveNotifyObj this class provides a notification object used by CWave objects

class CWaveNotifyObj : public CObject {
public:
    virtual void BlockFinished(UINT nLevel, DWORD dwPosition, UINT nSpeed = 100) = 0;
    virtual void BlockStored(UINT nLevel, DWORD dwPosition, BOOL * bSaveOverride = NULL) = 0;
    virtual void StoreFailed() = 0;
    virtual void EndPlayback() = 0;
    virtual HPSTR GetWaveData(CView * pView, DWORD dwPlayPosition, DWORD dwDataSize) = 0;
};

//###########################################################################
// CWave

struct FmtParm;

class CWave : public CObject {

    // Construction/destruction/creation
public:
    CWave();
    virtual ~CWave();

    // Attributes
private:
    CWaveOutDevice * m_pOutDev;

private:
    enum { m_kBuffers = 16, m_kPlayBuffers = 4, m_kRecordBuffers = m_kBuffers };
    HPSTR m_lpData[m_kBuffers];              // pointer to wave data
    DWORD m_dwBufferSize[m_kBuffers];        // size of data buffer
    DWORD m_dwProcessedSize[m_kBuffers];     // size of processed data for buffer
    DWORD m_dwPosition[m_kBuffers];
    UINT m_nProcessedSpeed[m_kBuffers];
    UINT m_nProcessedMax[m_kBuffers];
    CView * m_pView;              // pointer to view
    FmtParm * m_pFmtParm;         // wave format parameters
    CWaveWarp * m_pWaveWarp;      // pointer to waveform warping object
    CWaveInDevice * m_pInDev;     // input device
    CWaveNotifyObj * m_pNotifyObj; // notify object
    HMMIO m_hmmioFile;            // temporary file handle
    DWORD m_dwPlayPosition;        // index of current playing sample block
    DWORD m_dwRecordPointer;      // index of current recording sample block
    DWORD m_dwStart;              // index of first sample to play
    DWORD m_dwEnd;                // index of last sample to play
    int m_nActiveBlock;             // current block recording or playing
    int m_nNextBlock;             // next block to be queued for playback
    FRAG_PARMS m_stCallData;      // callback waveform data
    UINT m_nMaxLevel;             // maximal volume level of sound data
    bool m_bRecording;            // TRUE, if recording, FALSE if monitoring
    UINT m_nSpeed;                // playback speed
    BOOL m_bPlayDone;             // playback finished flag
    BOOL m_bProcessDone;          // waveform processing finished flag
    BOOL m_bBackgroundEnabled;    // background processing enabled flag

    // Operations
private:
    BOOL AllocateBuffers(); // allocate wave buffers
    BOOL ProcessData(int nBuffer); // process sample data
    void CopyBuffer(HPSTR pSource, HPSTR pTarget, DWORD dwLength, BOOL bIs8Bit, int * piMax, int * piMin);
    void FindMinMax(HPSTR pSource, DWORD dwLength, BOOL bIs8Bit, int * piMax, int * piMin);
public:
    BOOL Play(DWORD dwStart, DWORD dwSize, UINT nVolume, UINT nSpeed, CView * pView, CWaveNotifyObj * pNotify = NULL);
    BOOL Monitor(CView * pView, CWaveNotifyObj * pNotify = NULL);
    BOOL Record(HMMIO hmmioFile, CView * pView, DWORD dwOffset, CWaveNotifyObj * pNotify, bool bRecord = true);
    void Stop();
    WAVEFORMATEX * GetFormat() {
        return (WAVEFORMATEX *)m_pFmtParm;
    }
    int GetActiveBlock() {
        return m_nActiveBlock;   // returns the last block ID
    }
    int GetNextBlock() {
        return m_nNextBlock;   // returns the last block ID
    }
    DWORD AdjustToParameters(); // adjust buffer size to parameters
    char * GetBufferPointer(int nBuffer); // returns a pointer to the current wave data buffer
    char * GetPlaybackPointer(int nBuffer); // returns a pointer to the current playback buffer
    DWORD GetBufferSize(int nBuffer); // returns the size of the current wave data buffer
    DWORD GetPlaybackSize(int nBuffer); // returns the size of the current playback data buffer
    DWORD GetProcessedSize(int nBuffer); // returns the size of the processed data for the current wave data buffer
    void SetBufferSize(int nBuffer, DWORD dwProcessedSize, DWORD dwBufferSize); // sets the size of the current wave data buffer
    void NextBlock(); // prepare next wave data block
    void StoreBlock(); // store a recorded wave data block
    UINT GetVolume(BOOL * bResult = NULL) {
        return m_pOutDev->GetVolume(bResult);   // get playback volume
    }
    void SetVolume(UINT nVolume) {
        m_pOutDev->SetVolume(nVolume);   // set playback volume
    }
    void SetSpeed(UINT nSpeed) {
        m_nSpeed = nSpeed;   // set playback speed
    }
    void SetMaxLevel(UINT nMaxLevel) {
        m_nMaxLevel = nMaxLevel;   // set peak magnitude
    }
    CWaveOutDevice * GetOutDevice() {
        return m_pOutDev;
    }
    CWaveInDevice * GetInDevice() {
        return m_pInDev;
    }
};

#endif //_SA_WAVE_H
