/////////////////////////////////////////////////////////////////////////////
// sa_p_spi.h:
// Interface of the CProcessSmoothedPitch class
// Author: Urs Ruchti
// copyright 1996 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////
#ifndef _SA_P_SPI_H
#define _SA_P_SPI_H

//###########################################################################
// CProcessSmoothedPitch data processing

// Defines
#define GROUP_BOUNDARY  0x4000  // bit to mark cluster edge
#define POINT_MISSING   -2    // point is missing but can be interpolated
#define POINT_UNSET     -1    // there is no data for this point

class CProcessSmoothedPitch : public CProcess {
// Construction/destruction/creation
public:
    CProcessSmoothedPitch();
    virtual ~CProcessSmoothedPitch();

// Attributes
private:
    SGrapplParms m_CalcParm;
    DWORD m_dwDataPos;  // byte offset into waveform data

// Operations
public:
    long Process(void * pCaller, ISaDoc * pDoc, int nProgress = 0, int nLevel = 1);
    double GetUncertainty(double fPitch);
};

#endif //_SA_P_SPI_H
