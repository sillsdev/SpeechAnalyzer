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
#ifndef MIXER_H
#define MIXER_H

#include "SaString.h"

int GetWindowsVersion();

class CMixer {
public:
    CMixer(DWORD dwComponentType);
    virtual ~CMixer();
    MMRESULT Connect(UINT uMixID, DWORD dwMixIdFlags, HWND hCallback);
    MMRESULT Disconnect();
    MMRESULT SetVolume(UINT uMixID, DWORD dwMixIdFlags, DWORD dwVolume);
    MMRESULT GetVolume(UINT uMixID, DWORD dwMixIdFlags, DWORD * dwVolume);
    BOOL IsSndVolInstalled();


protected:
    HMIXER m_hMixerCallback;
    const DWORD m_dwComponentType;
    CSaString m_szPlayMixerCmd;
    CSaString m_szRecMixerCmd;

private:
    MMRESULT GetMixerControlID(HMIXER & hmx, DWORD * dwControl, MIXERCONTROL * control = NULL , DWORD dwControlType = MIXERCONTROL_CONTROLTYPE_VOLUME);
};

#endif
