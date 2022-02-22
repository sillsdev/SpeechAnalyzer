/////////////////////////////////////////////////////////////////////////////
// sa_p_lou.h:
// Interface of the CProcessLoudness
//                  CProcessSmoothLoudness classes.
// Author: Urs Ruchti
// copyright 1996 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////
#ifndef _SA_P_LOU_H
#define _SA_P_LOU_H

#include "sa_process.h"

//###########################################################################
// CProcessLoudness data processing

class CProcessLoudness : public CProcess {
public:
    long Process(void * pCaller, Model * pModel, int nProgress = 0, int nLevel = 1);
    virtual void SetDataInvalid() {
        CProcess::SetDataInvalid();
    }
};

//###########################################################################
// CProcessSmoothLoudness data processing

class CProcessSmoothLoudness : public CProcess {
// Construction/destruction/creation
public:
    CProcessSmoothLoudness(Context & context);
    virtual ~CProcessSmoothLoudness();

// Attributes
private:
    CFile * m_pSRDfile;             // smooth raw data file object
    CFileStatus m_SRDfileStatus;    // smooth raw data file status
    HANDLE m_hSRDdata;              // needed to get m_lpSRDData
    BPTR m_lpSRDdata;              // pointer to processed smooth raw data
    DWORD m_dwSRDBufferOffset;      // actual smooth raw data buffer offset
    BOOL m_bSRDBlockBegin;          // actual smooth raw data block begin flag

// Operations
protected:
    virtual long Exit(int nError, HANDLE hSmoothBlock); // exit processing on error
    BPTR SmoothRawData(Model * pModel, BPTR pTarget, UINT nBlockAlign, int nOldBlock,
                        int nBlock, int * pnLastDone, DWORD dwSmplPerSec, DWORD dwStart, UINT * nAverage, long * lAverage);
public:
    void SetDataInvalid();
    long Process(void * pCaller, Model * pModel, int nProgress = 0, int nLevel = 1);
    BPTR GetSmoothRawData(DWORD dwOffset, BOOL bBlockBegin = FALSE); // return smooth raw data pointer on given position (offset)
    DWORD GetSmoothRawDataSize();
};

#endif //_SA_P_LOU_H
