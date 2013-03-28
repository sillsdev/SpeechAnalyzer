/////////////////////////////////////////////////////////////////////////////
// sa_p_gra.h:
// Interface of the CProcessGrappl class
// Author: Urs Ruchti
// copyright 1996 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////
#ifndef _SA_P_GRA_H
#define _SA_P_GRA_H
#include "dsp\grappl.h"

//###########################################################################
// CProcessGrappl data processing

class CProcessGrappl : public CProcess
{
// Construction/destruction/creation
public:
    CProcessGrappl();
    virtual ~CProcessGrappl();

// Attributes
private:
    Grappl_parms m_CalcParm;
    double m_dAvgPitch;
    DWORD m_dwDataPos;  // byte offset into waveform data

// Operations
public:
    Grappl_parms & GetCalcParms()
    {
        return m_CalcParm;
    }
    long Process(void * pCaller, ISaDoc * pDoc, int nProgress = 0, int nLevel = 1);
    BOOL IsVoiced(ISaDoc * pDoc, DWORD dwWaveOffset);
    double GetAveragePitch()
    {
        return m_dAvgPitch;
    }
    double GetUncertainty(double fPitch);
    virtual void SetDataInvalid()
    {
        CProcess::SetDataInvalid();
    }
};

#endif //_SA_P_GRA_H
