#ifndef RECMIXER_H
#define RECMIXER_H

#include "SaString.h"
#include "mixer.h"

class CRecMixer : public CMixer {
public:
    CRecMixer();
    ~CRecMixer();

    MMRESULT Connect(HWAVEIN hRecorder, HWND hCallback);
    MMRESULT SetVolume(HWAVEIN hRecorder, DWORD dwVolume);
    MMRESULT GetVolume(HWAVEIN hRecorder, DWORD * dwVolume);
    BOOL ShowMixerControls(HWAVEIN hRecorder);
    BOOL CanShowMixerControls(HWAVEIN hRecorder);
};

#endif
