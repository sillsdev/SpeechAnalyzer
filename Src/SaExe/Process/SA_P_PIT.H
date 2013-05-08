/////////////////////////////////////////////////////////////////////////////
// sa_p_pit.h:
// Interface of the CProcessPitch class
// Author: Urs Ruchti
// copyright 1996 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////
#ifndef _SA_P_PIT_H
#define _SA_P_PIT_H

#include "dsp\grappl.h"

//###########################################################################
// CProcessPitch data processing

class CProcessPitch : public CProcess
{
// Construction/destruction/creation
public:
    CProcessPitch();
    virtual ~CProcessPitch();

// Attributes
private:
    SGrapplParms m_CalcParm;
    DWORD m_dwDataPos;  // byte offset into waveform data

// Operations
protected:

private:

public:
    long Process(void * pCaller, ISaDoc * pDoc, int nProgress = 0, int nLevel = 1);
    double GetUncertainty(double fPitchData);
};

#endif //_SA_P_PIT_H
