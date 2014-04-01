#ifndef WAVE_OUT_DEVICE_H
#define WAVE_OUT_DEVICE_H

#include "RecMixer.h"
#include <mmsystem.h>
#include <wavewarp.h>
#include <hpfltr70.h>
#include "Sa_wave.h"

class CWave;
class CWaveNotifyObj;
class CPlayMixer;

//###########################################################################
// CWaveOutDevice
class CWaveOutDevice : public CWnd
{

public:
    CWaveOutDevice();
    virtual ~CWaveOutDevice();
    BOOL Open(WAVEFORMATEX & format);
    BOOL CanDoFormat(WAVEFORMATEX & format);
    BOOL Close();
    BOOL Play(int nBuffer, UINT nVolume, CWave * pWave, BOOL bPause = FALSE);
    UINT GetVolume(BOOL & bResult);
    void SetVolume(UINT nVolume, BOOL & bResult);
    void WaveOutDone(CWave * pWave, WAVEHDR * pHdr);
    void Reset();
    BOOL ShowMixer(BOOL bShow = TRUE);
    BOOL ConnectMixer(CWnd * pCallback);

protected:
    afx_msg LRESULT OnWomDone(WPARAM w, LPARAM l);

    DECLARE_MESSAGE_MAP()

private:
    BOOL Create();

    HWAVEOUT m_hOutDev;           // output device handle
    CPlayMixer * m_pMixer;

};

// some global items
extern CWaveOutDevice theDefaultWaveOutDevice;

#endif
