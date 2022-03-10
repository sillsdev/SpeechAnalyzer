#pragma once
#ifndef _SA_P_SMLOU_H
#define _SA_P_SMLOU_H

#include "sa_process.h"

class CProcessSmoothLoudness : public CProcess {
public:
    CProcessSmoothLoudness(Context& context);
    virtual ~CProcessSmoothLoudness();

    void SetDataInvalid();
    long Process(void* pCaller, int nProgress = 0, int nLevel = 1);
    BPTR GetSmoothRawData(DWORD dwOffset, BOOL bBlockBegin = FALSE); // return smooth raw data pointer on given position (offset)

protected:
    virtual long Exit(int nError, HANDLE hSmoothBlock); // exit processing on error
    BPTR SmoothRawData(ofstream& file, BPTR pTarget, UINT nBlockAlign, int nOldBlock,
                        int nBlock, int* pnLastDone, DWORD dwSmplPerSec, DWORD dwStart, UINT* nAverage, long* lAverage);

private:
    wstring srdFilename;            // the file we are working with

    HANDLE m_hSRDdata;              // needed to get m_lpSRDData
    BPTR m_lpSRDdata;               // pointer to processed smooth raw data
    DWORD m_dwSRDBufferOffset;      // actual smooth raw data buffer offset
    BOOL m_bSRDBlockBegin;          // actual smooth raw data block begin flag

};
#endif
