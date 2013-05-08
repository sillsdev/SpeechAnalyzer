/////////////////////////////////////////////////////////////////////////////
// sa_p_gra.h:
// Interface of the CProcessGrappl class
// Author: Urs Ruchti
// copyright 1996 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////
#ifndef _SA_P_GRA_H
#define _SA_P_GRA_H
#include "dsp\grappl.h"

class CProcessGrappl : public CProcess
{
public:
    CProcessGrappl();
    virtual ~CProcessGrappl();
    SGrapplParms & GetCalcParms();
    long Process(void * pCaller, ISaDoc * pDoc, int nProgress = 0, int nLevel = 1);
    BOOL IsVoiced(ISaDoc * pDoc, DWORD dwWaveOffset);
    double GetAveragePitch();
    double GetUncertainty(double fPitch);
    virtual void SetDataInvalid();
	void Dump( LPCSTR ofilename);

private:
    SGrapplParms m_CalcParm;
    double m_dAvgPitch;
    DWORD m_dwDataPos;  // byte offset into waveform data

};

#endif
