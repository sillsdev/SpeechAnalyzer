/////////////////////////////////////////////////////////////////////////////
// sa_p_gra.h:
// Interface of the CProcessGrappl class
// Author: Urs Ruchti
// copyright 1996 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////
#ifndef _SA_P_GRAPPL_H
#define _SA_P_GRAPPL_H

#include "AbstractPitchProcess.h"

class CProcessGrappl : public CAbstractPitchProcess {
public:
    CProcessGrappl();
    SGrapplParms & GetCalcParms();
    long Process(void * pCaller, ISaDoc * pDoc, int nProgress = 0, int nLevel = 1);
    BOOL IsVoiced(ISaDoc * pDoc, DWORD dwWaveOffset);
    double GetAveragePitch();
    virtual void SetDataInvalid();
    void Dump(LPCSTR ofilename);

private:
    double m_dAvgPitch;
    DWORD m_dwDataPos;  // byte offset into waveform data

};

#endif
