#ifndef WAVEINDEVICE_H
#define WAVEINDEVICE_H

#include <mmsystem.h>
#include <wavewarp.h>
#include <hpfltr70.h>

class CWave;
class CWaveNotifyObj;
class CRecMixer;
class CPlayMixer;
class CWaveOutDevice;

class CWaveInDevice : public CWnd
{

public:
    CWaveInDevice();
    virtual ~CWaveInDevice();
    BOOL Open(WAVEFORMATEX & format);           // device open
    BOOL CanDoFormat(WAVEFORMATEX & format);
    BOOL Close();                               // device close
    void WaveInDone(CWave * pWave, WAVEHDR * pHdr);
    void Reset();
    BOOL Record(int nBuffer, CWave * pWave, BOOL bStart=FALSE);
    BOOL AttachHighPassFilter( uint32 wSmpRate);
    CHighPassFilter70 * GetHighPassFilter();
    void DetachHighPassFilter();
    MMRESULT GetVolume(UINT & volume);
    void SetVolume(UINT nVolume, BOOL * bResult = NULL);
    BOOL ShowMixer(BOOL bShow = TRUE);
    BOOL ConnectMixer(CWnd * pCallback);
    int GetPreparedBuffers() const;

protected:
    afx_msg LRESULT OnWimData(WPARAM w, LPARAM l);

    DECLARE_MESSAGE_MAP()

private:
    BOOL Create();

    HWAVEIN m_hInDev;                           // input device handle
    CHighPassFilter70 * m_pHighPassFilter;
    CRecMixer * m_pMixer;
    int m_nBuffers;

};

#endif
