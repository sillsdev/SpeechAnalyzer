/////////////////////////////////////////////////////////////////////////////
// sa_p_mel.h:
// Interface of the CProcessMelogram class
// Author: Urs Ruchti and Corey Wenger
// copyright 2000 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////
#ifndef _SA_P_MELOGRAM_H
#define _SA_P_MELOGRAM_H

#include "abstractpitchprocess.h"

class CProcessMelogram : public CAbstractPitchProcess {

public:
    CProcessMelogram(Context context);
    long Process(void * pCaller,  int nProgress = 0, int nLevel = 1);
    BOOL IsVoiced( DWORD dwWaveOffset);

private:
    DWORD m_dwDataPos;  // byte offset into waveform data
    int m_nMinValidSemitone100;
    int m_nMaxValidSemitone100;
};

#endif
