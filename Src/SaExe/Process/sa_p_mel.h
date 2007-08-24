/////////////////////////////////////////////////////////////////////////////
// sa_p_mel.h:
// Interface of the CProcessMelogram class
// Author: Urs Ruchti and Corey Wenger
// copyright 2000 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////
#ifndef _SA_P_MEL_H
#define _SA_P_MEL_H
#include "dsp\grappl.h"

//###########################################################################
// CProcessMelogram data processing

class CProcessMelogram : public CDataProcess
{
// Construction/destruction/creation
public:
  CProcessMelogram();
  virtual ~CProcessMelogram();

// Attributes
private:
    DWORD m_dwDataPos;  // byte offset into waveform data
    int m_nMinValidSemitone100;
    int m_nMaxValidSemitone100;
    Grappl_parms m_CalcParm;

// Operations
public:
  long Process(void* pCaller, ISaDoc* pDoc, int nProgress = 0, int nLevel = 1);
  BOOL IsVoiced(ISaDoc *pDoc, DWORD dwWaveOffset);
  double GetUncertainty(double fPitch);
};

#endif //_SA_P_MEL_H
