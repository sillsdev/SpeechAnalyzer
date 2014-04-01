#include "stdafx.h"
#include "PlayMixer.h"
#include <mmsystem.h>

CPlayMixer::CPlayMixer() : CMixer(MIXERLINE_COMPONENTTYPE_SRC_WAVEOUT)
{
}

CPlayMixer::~CPlayMixer()
{
}

MMRESULT CPlayMixer::Connect(HWAVEOUT hPlayer, HWND hCallback)
{
    return CMixer::Connect((UINT) hPlayer, MIXER_OBJECTF_HWAVEOUT, hCallback);
}

MMRESULT CPlayMixer::SetVolume(HWAVEOUT hPlayer, DWORD dwVolume)
{

    MMRESULT result = CMixer::SetVolume((UINT) hPlayer, MIXER_OBJECTF_HWAVEOUT, dwVolume);
    if (result != MMSYSERR_NOERROR)
    {
        result = waveOutSetVolume(hPlayer, dwVolume);
    }

    return result;
}

MMRESULT CPlayMixer::GetVolume(HWAVEOUT hPlayer, DWORD * dwVolume)
{

    MMRESULT result = CMixer::GetVolume((UINT) hPlayer, MIXER_OBJECTF_HWAVEOUT, dwVolume);
    if (result != MMSYSERR_NOERROR)
    {
        result = waveOutGetVolume(hPlayer, dwVolume);
    }

    return result;
}

BOOL CPlayMixer::CanShowMixerControls(HWAVEOUT hPlayer)
{

    BOOL result = TRUE;
    if (!m_hMixerCallback)
    {
        // try to open mixer
        result = Connect(hPlayer, NULL);
        Disconnect();
    }
    return IsSndVolInstalled() && result;
}

BOOL CPlayMixer::ShowMixerControls(HWAVEOUT hPlayer)
{
    if (CanShowMixerControls(hPlayer))
    {
        UINT uDevID;
        MMRESULT result = mixerGetID((HMIXEROBJ)hPlayer, &uDevID, MIXER_OBJECTF_HWAVEOUT);
        if (result == MMSYSERR_NOERROR)
        {
            STARTUPINFO si;
            PROCESS_INFORMATION pi;

            ZeroMemory(&si, sizeof(si));
            si.cb = sizeof(si);
            ZeroMemory(&pi, sizeof(pi));

            CSaString command;
            CSaString args;
            if (GetWindowsVersion()<6)
            {
                args.Format(_T("%d"), uDevID);
            }
            command = m_szPlayMixerCmd + args;

            result = CreateProcess(NULL,
                                   command.GetBuffer(256),
                                   NULL,
                                   NULL,
                                   FALSE,
                                   CREATE_NEW_PROCESS_GROUP | CREATE_DEFAULT_ERROR_MODE | NORMAL_PRIORITY_CLASS,
                                   NULL,
                                   NULL,
                                   &si,
                                   &pi
                                  );
            return result;
        }
    }
    return FALSE;
}
