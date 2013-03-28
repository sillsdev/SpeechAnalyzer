/////////////////////////////////////////////////////////////////////////////
// sa_p_lou.h:
// Interface of the CProcessLoudness
//                  CProcessSmoothLoudness classes.
// Author: Urs Ruchti
// copyright 1996 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////
#ifndef _SA_P_LOU_H
#define _SA_P_LOU_H

#include "Process/Process.h"

//###########################################################################
// CProcessLoudness data processing

class CProcessLoudness : public CProcess
{
// Construction/destruction/creation
public:
    CProcessLoudness();
    virtual ~CProcessLoudness();

// Attributes
private:

// Operations
public:
    long Process(void * pCaller, ISaDoc * pDoc, int nProgress = 0, int nLevel = 1);
    virtual void SetDataInvalid()
    {
        CProcess::SetDataInvalid();
    }
};

//###########################################################################
// CProcessSmoothLoudness data processing

class CProcessSmoothLoudness : public CProcess
{
// Construction/destruction/creation
public:
    CProcessSmoothLoudness();
    virtual ~CProcessSmoothLoudness();

// Attributes
private:
    CFile * m_pSRDfile;     // smooth raw data file object
    CFileStatus m_SRDfileStatus; // smooth raw data file status
    HANDLE  m_hSRDdata;       // needed to get m_lpSRDData
    HPSTR   m_lpSRDdata;      // pointer to processed smooth raw data
    DWORD   m_dwSRDBufferOffset; // actual smooth raw data buffer offset
    BOOL    m_bSRDBlockBegin; // actual smooth raw data block begin flag

// Operations
protected:
    virtual long Exit(int nError, HANDLE hSmoothBlock); // exit processing on error
    HPSTR SmoothRawData(ISaDoc * pDoc, HPSTR pTarget, UINT nBlockAlign, int nOldBlock,
                        int nBlock, int * pnLastDone, DWORD dwSmplPerSec, DWORD dwStart, UINT * nAverage, long * lAverage);
public:
    void SetDataInvalid();
    long Process(void * pCaller, ISaDoc * pDoc, int nProgress = 0, int nLevel = 1);
    HPSTR GetSmoothRawData(DWORD dwOffset, BOOL bBlockBegin = FALSE); // return smooth raw data pointer on given position (offset)
    DWORD GetSmoothRawDataSize();
};

#endif //_SA_P_LOU_H
