#ifndef SIGNAL_H
#define SIGNAL_H
#include "dspTypes.h"

enum EPCM_TYPE {PCM_UBYTE=-1, PCM_2SBYTE=1, PCM_2SSHORT=2};
enum ESIG_SOURCE {UNKNOWN_HUMAN=-1, MAN=0, WOMAN=1, CHILD=2, MUSICAL_INSTRUMENT=3};

struct SSigParms
{
    SSigParms()
    {
        Start = 0;
        Length = 0;
        SmpDataFmt = 0;
        SmpRate = 0;
        Source = UNKNOWN_HUMAN;
    }
    void * Start;
    uint32 Length;
    int8 SmpDataFmt;
    uint32 SmpRate;
    ESIG_SOURCE Source;
};

// SProcParms added by CLW 1/5/99
struct SProcParms
{
    void * pBufferStart;	// pointer to buffer containing data to be processed
    uint32 dwBufferSize;	// size of buffer in bytes
    uint32 dwBlockOffset;	// sample number at which processing begins
    uint32 dwBlockLength;	// number of samples in process data
    SSigParms stSigParms;
};

#endif
