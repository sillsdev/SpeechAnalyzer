/////////////////////////////////////////////////////////////////////////////
// sa_p_mel.h:
// Interface of the CProcessMelogram class
// Author: Urs Ruchti and Corey Wenger
// copyright 2000 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////
#ifndef _SA_P_MEL_H
#define _SA_P_MEL_H

#include "Process\Process.h"

class CProcessMelogram : public CProcess
{
public:
    CProcessMelogram();
    virtual ~CProcessMelogram();

    long Process(void * pCaller, ISaDoc * pDoc, int nProgress = 0, int nLevel = 1);
    BOOL IsVoiced(ISaDoc * pDoc, DWORD dwWaveOffset);
    double GetUncertainty(double fPitch);

private:
    DWORD m_dwDataPos;  // byte offset into waveform data
    int m_nMinValidSemitone100;
    int m_nMaxValidSemitone100;
    SGrapplParms m_CalcParm;

};

#endif
