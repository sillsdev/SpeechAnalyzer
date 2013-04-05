/////////////////////////////////////////////////////////////////////////////

// sa_wave.h:
// Interface of the CWaveNotifyObj
//                  CWave          classes.
// Author: Urs Ruchti
// copyright 1996 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////
#ifndef _SA_WAVE_H
#define _SA_WAVE_H

#include "AppDefs.h"
#include <mmsystem.h>
#include "FmtParm.h"
#include "RecMixer.h"
#include "dsp\wavewarp.h"
#include "dsp\hpfltr70.h"

class CRecMixer;
class CPlayMixer;
class CWaveOutDevice;
class CWaveInDevice;

struct SWaveHdrInfo
{
    WAVEHDR * pHdr;
    int iBuffer;
};

//###########################################################################
// CWaveNotifyObj this class provides a notification object used by CWave objects

class CWaveNotifyObj : public CObject
{
public:
    virtual void BlockFinished(UINT nLevel, DWORD dwPosition, UINT nSpeed = 100) = 0;
    virtual void BlockStored(UINT nLevel, DWORD dwPosition, BOOL * bSaveOverride = NULL) = 0;
    virtual void StoreFailed() = 0;
    virtual void EndPlayback() = 0;
    virtual HPSTR GetWaveData(CView * pView, DWORD dwPlayPosition, DWORD dwDataSize) = 0;
};

class CWave : public CObject
{

public:
    CWave();
    virtual ~CWave();
    BOOL Play(DWORD dwStart, DWORD dwSize, UINT nVolume, UINT nSpeed, CView * pView, CWaveNotifyObj * pNotify = NULL);
    BOOL Monitor(CView * pView, CWaveNotifyObj * pNotify = NULL);
    BOOL Record(HMMIO hmmioFile, CView * pView, DWORD dwOffset, CWaveNotifyObj * pNotify, bool bRecord = true);
    void Stop();
    CFmtParm GetFormat();
    int GetActiveBlock();                   // returns the last block ID
    int GetNextBlock();                     // returns the last block ID
    DWORD CalculateBufferSize();            // adjust buffer size to parameters
    char * GetBufferPointer(int nBuffer);   // returns a pointer to the current wave data buffer
    char * GetPlaybackPointer(int nBuffer); // returns a pointer to the current playback buffer
    DWORD GetBufferSize(int nBuffer);       // returns the size of the current wave data buffer
    DWORD GetPlaybackSize(int nBuffer);     // returns the size of the current playback data buffer
    DWORD GetProcessedSize(int nBuffer);    // returns the size of the processed data for the current wave data buffer
    void SetBufferSize(int nBuffer, DWORD dwProcessedSize, DWORD dwBufferSize); // sets the size of the current wave data buffer
    void NextBlock();                       // prepare next wave data block
    void StoreBlock();                      // store a recorded wave data block
    UINT GetVolume(BOOL & bResult);         // get playback volume
    void SetVolume(UINT nVolume);           // set playback volume
    void SetSpeed(UINT nSpeed);             // set playback speed
    void SetMaxLevel(UINT nMaxLevel);       // set peak magnitude
    CWaveOutDevice * GetOutDevice();
    CWaveInDevice * GetInDevice();

private:
    BOOL AllocateBuffers();                 // allocate wave buffers
    BOOL ProcessData(int nBuffer);          // process sample data
    void CopyBuffer(HPSTR pSource, HPSTR pTarget, DWORD dwLength, BOOL bIs8Bit, int * piMax, int * piMin);
    void FindMinMax(HPSTR pSource, DWORD dwLength, BOOL bIs8Bit, int * piMax, int * piMin);

    CWaveOutDevice * m_pOutDev;
    enum { m_kBuffers = 16, m_kPlayBuffers = 4, m_kRecordBuffers = m_kBuffers };
    HPSTR m_lpWaveData[m_kBuffers];             // pointer to wave data
    DWORD m_dwBufferSizes[m_kBuffers];          // size of data buffer
    DWORD m_dwProcessedSize[m_kBuffers];        // size of processed data for buffer
    DWORD m_dwPosition[m_kBuffers];
    UINT m_nProcessedSpeed[m_kBuffers];
    UINT m_nProcessedMax[m_kBuffers];
    CView * m_pView;                // pointer to view
    CFmtParm m_FmtParm;
    CWaveWarp * m_pWaveWarp;        // pointer to waveform warping object
    CWaveInDevice * m_pInDev;       // input device
    CWaveNotifyObj * m_pNotifyObj;  // notify object
    HMMIO m_hmmioFile;              // temporary file handle
    DWORD m_dwPlayPosition;         // index of current playing sample block
    DWORD m_dwRecordPointer;        // index of current recording sample block
    DWORD m_dwStart;                // index of first sample to play
    DWORD m_dwEnd;                  // index of last sample to play
    int m_nActiveBlock;             // current block recording or playing
    int m_nNextBlock;               // next block to be queued for playback
    FRAG_PARMS m_CallbackData;      // callback waveform data
    UINT m_nMaxLevel;               // maximal volume level of sound data
    bool m_bRecording;              // TRUE, if recording, FALSE if monitoring
    UINT m_nSpeed;                  // playback speed
    BOOL m_bPlayDone;               // playback finished flag
    BOOL m_bProcessDone;            // waveform processing finished flag
    BOOL m_bBackgroundEnabled;      // background processing enabled flag
};

extern void MMERR(MMRESULT mmr);

#endif
