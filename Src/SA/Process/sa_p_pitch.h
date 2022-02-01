/////////////////////////////////////////////////////////////////////////////
// sa_p_pitch.h:
// Interface of the CProcessPitch class
// Author: Urs Ruchti
// copyright 1996 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////
#ifndef _SA_P_PITCH_H
#define _SA_P_PITCH_H

#include "abstractpitchprocess.h"
#include <atomic>

class CProcessPitch : public CAbstractPitchProcess {

private:
    DWORD m_dwDataPos;  // byte offset into waveform data

public:
    long Process(void * pCaller, ISaDoc * pDoc, int nProgress = 0, int nLevel = 1);
};

#endif
