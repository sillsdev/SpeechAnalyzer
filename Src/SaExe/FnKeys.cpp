#include "stdafx.h"
#include "fnkeys.h"
#include "settings\obstream.h"
#include "resource.h"


static const char * psz_fnkeys = "fnkeys";

static const char * psz_repeat = "repeat";
static const char * psz_delay  = "delay";
static const char * psz_volume = "volume";
static const char * psz_speed  = "speed";
static const char * psz_mode   = "mode";


void FnKeys::WriteProperties(Object_ostream & obs)
{
    obs.WriteBeginMarker(psz_fnkeys);

    for (int i=0; i<24; i++)
    {
        obs.WriteBool(psz_repeat, bRepeat[i]);
        obs.WriteUInt(psz_delay,  nDelay[i]);
        obs.WriteUInt(psz_volume, nVolume[i]);
        obs.WriteUInt(psz_speed,  nSpeed[i]);
        obs.WriteUInt(psz_mode,   nMode[i]);
    }

    obs.WriteEndMarker(psz_fnkeys);
}




BOOL FnKeys::ReadProperties(Object_istream & obs)
{
    if (!obs.bAtBackslash() || !obs.bReadBeginMarker(psz_fnkeys))
    {
        return FALSE;
    }

    int nIndex = 0;

    while (!obs.bAtEnd())
    {
        if (obs.bReadBool(psz_repeat, bRepeat[nIndex]));
        else if (obs.bReadUInt(psz_delay, nDelay[nIndex]));
        else if (obs.bReadUInt(psz_volume, nVolume[nIndex]));
        else if (obs.bReadUInt(psz_speed, nSpeed[nIndex]));
        else if (obs.bReadUInt(psz_mode, nMode[nIndex++]));
        else if (obs.bEnd(psz_fnkeys))
        {
            break;
        }
    }

    return TRUE;
}



void FnKeys::SetupDefault()
{
    for (int nLoop = 0; nLoop < 24; nLoop++)
    {
        bRepeat[nLoop]   = FALSE; // repeat disabled
        nDelay[nLoop]   = 1000; // default repeat delay time
        nVolume[nLoop]   = 50; // play volume
        nSpeed[nLoop]   = 100; // replay speed
        nMode[nLoop]   = ID_PLAYBACK_FILE; // replay mode
    }
}
