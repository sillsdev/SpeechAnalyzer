#include "stdafx.h"
#include "fnkeys.h"
#include "objectostream.h"
#include "objectistream.h"

static LPCSTR psz_fnkeys = "fnkeys";

static LPCSTR psz_repeat = "repeat";
static LPCSTR psz_delay  = "delay";
static LPCSTR psz_volume = "volume";
static LPCSTR psz_speed  = "speed";
static LPCSTR psz_mode   = "mode";


void CFnKeys::WriteProperties(CObjectOStream & obs) {

    obs.WriteBeginMarker(psz_fnkeys);
    for (int i=0; i<24; i++) {
        obs.WriteBool(psz_repeat, bRepeat[i]);
        obs.WriteUInt(psz_delay,  nDelay[i]);
        obs.WriteUInt(psz_volume, nVolume[i]);
        obs.WriteUInt(psz_speed,  nSpeed[i]);
        obs.WriteUInt(psz_mode,   nMode[i]);
    }

    obs.WriteEndMarker(psz_fnkeys);
}

BOOL CFnKeys::ReadProperties(CObjectIStream & obs) {
    if (!obs.bAtBackslash() || !obs.bReadBeginMarker(psz_fnkeys)) {
        return FALSE;
    }

    int nIndex = 0;

    while (!obs.bAtEnd()) {
        if (obs.bReadBool(psz_repeat, bRepeat[nIndex]));
        else if (obs.bReadUInt(psz_delay, nDelay[nIndex]));
        else if (obs.bReadUInt(psz_volume, nVolume[nIndex]));
        else if (obs.bReadUInt(psz_speed, nSpeed[nIndex]));
        else if (obs.bReadUInt(psz_mode, nMode[nIndex++]));
        else if (obs.bEnd(psz_fnkeys)) {
            break;
        }
    }

    return TRUE;
}



void CFnKeys::SetupDefault() {
    for (int nLoop = 0; nLoop < 24; nLoop++) {
        bRepeat[nLoop]   = FALSE; // repeat disabled
        nDelay[nLoop]   = 1000; // default repeat delay time
        nVolume[nLoop]   = 50; // play volume
        nSpeed[nLoop]   = 100; // replay speed
        nMode[nLoop]   = ID_PLAYBACK_FILE; // replay mode
    }
}
