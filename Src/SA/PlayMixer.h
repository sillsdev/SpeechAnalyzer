/////////////////////////////////////////////////////////////////////////////
// mixer.h:
// Interface of the CMixer
//                  CPlayMixer
//
// Author: Steve MacLean
// copyright 2001 JAARS Inc. SIL
//
// Revision History
//
// 01/23/2000
//    SDM   Original version
//
/////////////////////////////////////////////////////////////////////////////
#ifndef PLAYMIXER_H
#define PLAYMIXER_H

#include "Mixer.h"
#include "SaString.h"

class CPlayMixer : public CMixer
{
public:
    CPlayMixer();;
    ~CPlayMixer();

public:
    MMRESULT Connect(HWAVEOUT hPlayer, HWND hCallback);
    MMRESULT SetVolume(HWAVEOUT hPlayer, DWORD dwVolume);
    MMRESULT GetVolume(HWAVEOUT hPlayer, DWORD * dwVolume);
    BOOL ShowMixerControls(HWAVEOUT hPlayer);
    BOOL CanShowMixerControls(HWAVEOUT hPlayer);
};

#endif
