/////////////////////////////////////////////////////////////////////////////
// mixer.h:
// Implementation of the CMixer
//                       CPlayMixer
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

#include "stdafx.h"
#include <mmsystem.h>
#include "mixer.h"

CMixer::CMixer(DWORD dwComponentType) : m_dwComponentType(dwComponentType) {
    //
    //  WARNING DANGER WARNING DANGER WARNING DANGER WARNING DANGER WARNING
    //
    //      BEFORE calling any other mixer API's, we must call the
    //      mixerGetNumDevs() function to let the mixer thunk library
    //      dynamically link to all the mixer API's!
    //
    //  WARNING DANGER WARNING DANGER WARNING DANGER WARNING DANGER WARNING
    //
    mixerGetNumDevs();

    m_hMixerCallback = 0;
}

CMixer::~CMixer() {
    Disconnect();
}

MMRESULT CMixer::Disconnect() {
    MMRESULT result = MMSYSERR_NOERROR;
    if (m_hMixerCallback) {
        result = mixerClose(m_hMixerCallback);
        m_hMixerCallback = 0;
    }
    return result;
}

MMRESULT CMixer::Connect(UINT uMixId, DWORD dwMixIdFlags, HWND hCallback) {
    Disconnect();
    if (hCallback) {
        dwMixIdFlags |= CALLBACK_WINDOW;
    }
    return mixerOpen(&m_hMixerCallback, uMixId, (DWORD) hCallback, 0, dwMixIdFlags);
}

MMRESULT CMixer::GetMixerControlID(HMIXER & hmx, DWORD * dwControl, MIXERCONTROL * control, DWORD dwControlType) {
    MIXERLINE mixerLine;

    mixerLine.cbStruct = sizeof(MIXERLINE);
    if (m_dwComponentType == MIXERLINE_COMPONENTTYPE_SRC_WAVEOUT) {
        mixerLine.dwComponentType = MIXERLINE_COMPONENTTYPE_DST_SPEAKERS;
    } else {
        mixerLine.dwComponentType = m_dwComponentType;
    }

    MMRESULT result = mixerGetLineInfo((HMIXEROBJ) hmx, &mixerLine, MIXER_GETLINEINFOF_COMPONENTTYPE);

    if (result != MMSYSERR_NOERROR) { // Can't Find Destination Line
        return result;
    }

    if (m_dwComponentType == MIXERLINE_COMPONENTTYPE_SRC_WAVEOUT) {
        DWORD maxSource = mixerLine.cConnections;
        DWORD dst = mixerLine.dwDestination;

        for (DWORD source=0; source < maxSource; source++) {
            mixerLine.cbStruct = sizeof(MIXERLINE);
            mixerLine.dwDestination = dst;
            mixerLine.dwSource = source;

            MMRESULT result2 = mixerGetLineInfo((HMIXEROBJ)hmx, &mixerLine,  MIXER_GETLINEINFOF_SOURCE);
            if (result2 != MMSYSERR_NOERROR) { // Can't Find Destination Line
                mixerClose(hmx);
                return result2;
            }

            if (mixerLine.dwComponentType == m_dwComponentType) {
                break;
            }
        }
        if (mixerLine.dwComponentType != m_dwComponentType) {
            return MMSYSERR_ERROR;
        }
    }

    if (!mixerLine.cControls && (dwControlType != MIXERCONTROL_CONTROLTYPE_VOLUME)) {
        return MMSYSERR_ERROR;
    }

    MIXERLINECONTROLS mixerLineControls;
    MIXERCONTROL mixerControl;

    if (mixerLine.cControls > 0) {
        mixerLineControls.cbStruct = sizeof(MIXERLINECONTROLS);
        mixerLineControls.dwLineID = mixerLine.dwLineID;
        mixerLineControls.dwControlType = dwControlType;
        mixerLineControls.cControls = 1;
        mixerLineControls.cbmxctrl = sizeof(MIXERCONTROL);
        mixerLineControls.pamxctrl = &mixerControl;

        result = mixerGetLineControls((HMIXEROBJ) hmx, &mixerLineControls, MIXER_GETLINECONTROLSF_ONEBYTYPE);

        if (result == MMSYSERR_NOERROR) {
            *dwControl = mixerControl.dwControlID;

            if (control) {
                *control = mixerControl;
            }

            return MMSYSERR_NOERROR;
        }

        // couldn't find the dwControlType Here
        if ((dwControlType != MIXERCONTROL_CONTROLTYPE_VOLUME) || (mixerLine.cConnections == 0)) {
            return result;
        }

        mixerLineControls.cbStruct = sizeof(MIXERLINECONTROLS);
        mixerLineControls.dwLineID = mixerLine.dwLineID;
        mixerLineControls.dwControlType = MIXERCONTROL_CONTROLTYPE_MUX;
        mixerLineControls.cControls = 1;
        mixerLineControls.cbmxctrl = sizeof(MIXERCONTROL);
        mixerLineControls.pamxctrl = &mixerControl;

        result = mixerGetLineControls((HMIXEROBJ) hmx, &mixerLineControls, MIXER_GETLINECONTROLSF_ONEBYTYPE);

        if (result == MMSYSERR_NOERROR) {
            MIXERCONTROLDETAILS mixerControlDetails;
            MIXERCONTROLDETAILS_BOOLEAN * listBool = new MIXERCONTROLDETAILS_BOOLEAN[mixerControl.cMultipleItems];
            MIXERCONTROLDETAILS_LISTTEXT * listText = new MIXERCONTROLDETAILS_LISTTEXT[mixerControl.cMultipleItems];

            mixerControlDetails.cbStruct = sizeof(MIXERCONTROLDETAILS);
            mixerControlDetails.dwControlID = mixerControl.dwControlID;
            mixerControlDetails.cChannels = 1;
            mixerControlDetails.cMultipleItems = mixerControl.cMultipleItems;
            mixerControlDetails.cbDetails = sizeof(MIXERCONTROLDETAILS_BOOLEAN);
            mixerControlDetails.paDetails = listBool;

            result =
                mixerGetControlDetails((HMIXEROBJ) hmx, &mixerControlDetails, MIXER_GETCONTROLDETAILSF_VALUE);

            if (result == MMSYSERR_NOERROR) {
                mixerControlDetails.cbStruct = sizeof(MIXERCONTROLDETAILS);
                mixerControlDetails.dwControlID = mixerControl.dwControlID;
                mixerControlDetails.cChannels = 1;
                mixerControlDetails.cMultipleItems = mixerControl.cMultipleItems;
                mixerControlDetails.cbDetails = sizeof(MIXERCONTROLDETAILS_LISTTEXT);
                mixerControlDetails.paDetails = listText;

                result =
                    mixerGetControlDetails((HMIXEROBJ) hmx, &mixerControlDetails, MIXER_GETCONTROLDETAILSF_LISTTEXT);
            }

            if (result == MMSYSERR_NOERROR) {
                for (unsigned int i=0; i<mixerControl.cMultipleItems; i++) {
                    if (listBool[i].fValue) {
                        mixerLineControls.cbStruct = sizeof(MIXERLINECONTROLS);
                        mixerLineControls.dwLineID = listText[i].dwParam1;
                        mixerLineControls.dwControlType = dwControlType;
                        mixerLineControls.cControls = 1;
                        mixerLineControls.cbmxctrl = sizeof(MIXERCONTROL);
                        mixerLineControls.pamxctrl = &mixerControl;

                        result = mixerGetLineControls((HMIXEROBJ) hmx, &mixerLineControls, MIXER_GETLINECONTROLSF_ONEBYTYPE);

                        if (result == MMSYSERR_NOERROR) {
                            *dwControl = mixerControl.dwControlID;

                            if (control) {
                                *control = mixerControl;
                            }

                            delete [] listBool;
                            delete [] listText;

                            return MMSYSERR_NOERROR;
                        }
                    }
                }
            }
            delete [] listBool;
            delete [] listText;
        }
    }

    return MMSYSERR_ERROR;
}

MMRESULT CMixer::GetVolume(UINT uMixID, DWORD dwMixIdFlags, DWORD * dwVolume) {
    MIXERCONTROLDETAILS mixerControlDetails;
    MIXERCONTROLDETAILS_UNSIGNED volume;

    HMIXER hmx;

    MMRESULT result = mixerOpen(&hmx, uMixID, NULL, NULL, dwMixIdFlags);
    if (result != MMSYSERR_NOERROR) {
        return result;
    }

    result = GetMixerControlID(hmx, &mixerControlDetails.dwControlID);

    mixerClose(hmx);
    if (result != MMSYSERR_NOERROR) {
        return result;
    }

    mixerControlDetails.cbStruct = sizeof(MIXERCONTROLDETAILS);
    mixerControlDetails.cChannels = 1;
    mixerControlDetails.cMultipleItems = 0;
    mixerControlDetails.cbDetails = sizeof(MIXERCONTROLDETAILS_UNSIGNED);
    mixerControlDetails.paDetails = &volume;

    result =
        mixerGetControlDetails((HMIXEROBJ) uMixID, &mixerControlDetails, dwMixIdFlags | MIXER_GETCONTROLDETAILSF_VALUE);

    if (result != MMSYSERR_NOERROR) {
        return result;
    }

    if (dwVolume) {
        *dwVolume = volume.dwValue;
    }

    return result;
}

MMRESULT CMixer::SetVolume(UINT uMixID, DWORD dwMixIdFlags, DWORD dwSetting) {
    MIXERCONTROLDETAILS mixerControlDetails;
    MIXERCONTROLDETAILS_UNSIGNED volume;

    HMIXER hmx;

    MMRESULT result = mixerOpen(&hmx, uMixID, NULL, NULL, dwMixIdFlags);
    if (result != MMSYSERR_NOERROR) {
        return result;
    }

    result = GetMixerControlID(hmx, &mixerControlDetails.dwControlID);

    mixerClose(hmx);
    if (result != MMSYSERR_NOERROR) {
        return result;
    }

    mixerControlDetails.cbStruct = sizeof(MIXERCONTROLDETAILS);
    mixerControlDetails.cChannels = 1;
    mixerControlDetails.cMultipleItems = 0;
    mixerControlDetails.cbDetails = sizeof(MIXERCONTROLDETAILS_UNSIGNED);
    mixerControlDetails.paDetails = &volume;

    volume.dwValue = dwSetting;

    return  mixerSetControlDetails((HMIXEROBJ) uMixID, &mixerControlDetails, dwMixIdFlags | MIXER_SETCONTROLDETAILSF_VALUE);
}

BOOL CMixer::IsSndVolInstalled() {

    BOOL bReturn = false;
    m_szPlayMixerCmd = "";
    m_szRecMixerCmd = "";

    if (GetWindowsVersion()<6) {
        bReturn = SearchPath(NULL, _T("SNDVOL32.EXE"), NULL, 0, NULL, NULL);
        if (bReturn) {
            m_szPlayMixerCmd = _T("SNDVOL32.EXE -D ");
            m_szRecMixerCmd = _T("SNDVOL32.EXE -R -D ");
        }
    } else {
        bReturn = SearchPath(NULL, _T("SNDVOL.EXE"), NULL, 0, NULL, NULL);
        bReturn = (bReturn && SearchPath(NULL, _T("CONTROL.EXE"), NULL, 0, NULL, NULL));
        if (bReturn) {
            m_szPlayMixerCmd = _T("SNDVOL.EXE");
            m_szRecMixerCmd = _T("CONTROL.EXE mmsys.cpl,,1");
        }
    }

    return bReturn;
}

/**
* returns major revision for operation system.
* see GetVersionEx for details
*/
int GetWindowsVersion() {
    OSVERSIONINFO versionInfo;
    memset(&versionInfo,0,sizeof(OSVERSIONINFO));
    versionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    BOOL result = ::GetVersionEx(&versionInfo);
    ASSERT(result);
    return versionInfo.dwMajorVersion;
}
