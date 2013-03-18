/////////////////////////////////////////////////////////////////////////////
// mixer.h:
// Interface of the CMixer
//                  CRecMixer
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

#include "CSaString.h"

int GetWindowsVersion();

class CMixer {
public:
    CMixer(DWORD dwComponentType);
    virtual ~CMixer();

protected:
    HMIXER m_hMixerCallback;
    const DWORD m_dwComponentType;
    CSaString m_szPlayMixerCmd;
    CSaString m_szRecMixerCmd;

public:
    MMRESULT Connect(UINT uMixID, DWORD dwMixIdFlags, HWND hCallback);
    MMRESULT Disconnect();
    MMRESULT SetVolume(UINT uMixID, DWORD dwMixIdFlags, DWORD dwVolume);
    MMRESULT GetVolume(UINT uMixID, DWORD dwMixIdFlags, DWORD * dwVolume);
    BOOL IsSndVolInstalled();

private:
    MMRESULT GetMixerControlID(HMIXER & hmx, DWORD * dwControl, MIXERCONTROL * control = NULL , DWORD dwControlType = MIXERCONTROL_CONTROLTYPE_VOLUME);
};

class CPlayMixer : public CMixer {
public:
    CPlayMixer();;
    ~CPlayMixer();

public:
    BOOL Connect(HWAVEOUT hPlayer, HWND hCallback);
    MMRESULT SetVolume(HWAVEOUT hPlayer, DWORD dwVolume);
    MMRESULT GetVolume(HWAVEOUT hPlayer, DWORD * dwVolume);
    BOOL ShowMixerControls(HWAVEOUT hPlayer);
    BOOL CanShowMixerControls(HWAVEOUT hPlayer);
};

class CRecMixer : public CMixer {
public:
    CRecMixer();
    ~CRecMixer();

public:
    BOOL Connect(HWAVEIN hRecorder, HWND hCallback);
    MMRESULT SetVolume(HWAVEIN hRecorder, DWORD dwVolume);
    MMRESULT GetVolume(HWAVEIN hRecorder, DWORD * dwVolume);
    BOOL ShowMixerControls(HWAVEIN hRecorder);
    BOOL CanShowMixerControls(HWAVEIN hRecorder);

};

