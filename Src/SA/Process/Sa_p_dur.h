/////////////////////////////////////////////////////////////////////////////
// sa_p_dur.h:
// Interface of the CDurationProcess
//
// Author: Alec Epting
// copyright 1998 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////
#ifndef _SA_P_DUR_H
#define _SA_P_DUR_H

class CProcessDurations : public CProcess {
public:
    CProcessDurations();
    virtual ~CProcessDurations();

    long Process(void * pCaller, ISaDoc * pDoc, int nProgress = 0, int nLevel = 1);
    DWORD GetProcessedData(int nSegmentIndex);
    virtual DWORD GetDataSize();
    void SetDataInvalid();
    DWORD GetMaxDuration();
    DWORD GetMinDuration();

private:
    CDWordArray m_Durations;    // pointer to segment durations
    DWORD m_dwMaxValue;         // maximum duration
    DWORD m_dwMinValue;         // minimum duration
};

#endif

