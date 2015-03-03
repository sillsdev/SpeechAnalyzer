#include "stdafx.h"
#include <mmsystem.h>
#include "RecMixer.h"

CRecMixer::CRecMixer() : CMixer(MIXERLINE_COMPONENTTYPE_DST_WAVEIN) {
}

CRecMixer::~CRecMixer() {
}

MMRESULT CRecMixer::Connect(HWAVEIN hRecorder, HWND hCallback) {
    return CMixer::Connect((UINT) hRecorder, MIXER_OBJECTF_HWAVEIN, hCallback);
}

MMRESULT CRecMixer::SetVolume(HWAVEIN hRecorder, DWORD dwVolume) {
    return CMixer::SetVolume((UINT) hRecorder, MIXER_OBJECTF_HWAVEIN, dwVolume);
}

MMRESULT CRecMixer::GetVolume(HWAVEIN hRecorder, DWORD * dwVolume) {
    return CMixer::GetVolume((UINT) hRecorder, MIXER_OBJECTF_HWAVEIN, dwVolume);
}

BOOL CRecMixer::CanShowMixerControls(HWAVEIN hRecorder) {
    BOOL result = TRUE;

    if (!m_hMixerCallback) {
        // try to open mixer
        result = Connect(hRecorder, NULL);
        Disconnect();
    }

    return IsSndVolInstalled() && result;
}

BOOL CRecMixer::ShowMixerControls(HWAVEIN hRecorder) {
    if (CanShowMixerControls(hRecorder)) {
        UINT uDevID;
        MMRESULT result = mixerGetID((HMIXEROBJ)hRecorder, &uDevID, MIXER_OBJECTF_HWAVEIN);
        if (result == MMSYSERR_NOERROR) {
            STARTUPINFO si;
            PROCESS_INFORMATION pi;

            ZeroMemory(&si, sizeof(si));
            si.cb = sizeof(si);
            ZeroMemory(&pi, sizeof(pi));

            CSaString command;
            CSaString args;
            if (GetWindowsVersion() < 6) {
                args.Format(_T("%d"), uDevID);
            }
            command = m_szRecMixerCmd + args;

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
            command.ReleaseBuffer();
            return result;
        }
    }
    return FALSE;
}
