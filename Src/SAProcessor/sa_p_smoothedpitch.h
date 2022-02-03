/////////////////////////////////////////////////////////////////////////////
// sa_p_spi.h:
// Interface of the CProcessSmoothedPitch class
// Author: Urs Ruchti
// copyright 1996 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////
#ifndef _SA_P_SMOOTHEDPITCH_H
#define _SA_P_SMOOTHEDPITCH_H

//###########################################################################
// CProcessSmoothedPitch data processing

#include "dspTypes.h"
#include "abstractpitchprocess.h"

// Defines
#define GROUP_BOUNDARY  0x4000  // bit to mark cluster edge
#define POINT_MISSING   -2    // point is missing but can be interpolated
#define POINT_UNSET     -1    // there is no data for this point

class CProcessSmoothedPitch : public CAbstractPitchProcess {

private:
    DWORD m_dwDataPos;  // byte offset into waveform data

public:
    long Process(void * pCaller, Model * pModel, int nProgress = 0, int nLevel = 1);
};

#endif //_SA_P_SMOOTHEDPITCH_H
