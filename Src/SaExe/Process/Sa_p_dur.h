/////////////////////////////////////////////////////////////////////////////
// sa_p_dur.h:
// Interface of the CDurationProcess
//
// Author: Alec Epting
// copyright 1998 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////
#ifndef _SA_P_DUR_H
#define _SA_P_DUR_H


//###########################################################################
// CProcessDuration data processing

class CProcessDurations : public CDataProcess
{
// Construction/destruction/creation
public:
    CProcessDurations();
    virtual ~CProcessDurations();

// Attributes
private:
    CDWordArray* m_pDurations;  // pointer to segment durations
    DWORD m_dwMaxValue;         // maximum duration
    DWORD m_dwMinValue;         // minimum duration

// Operations
public:
    long Process(void* pCaller, ISaDoc* pDoc, int nProgress = 0, int nLevel = 1);
    DWORD GetProcessedData(int nSegmentIndex)
                            { return m_pDurations->GetAt(nSegmentIndex); }
    virtual DWORD GetDataSize() {return (DWORD)m_pDurations->GetSize();} //return number of durations                        
    void SetDataInvalid() {CDataProcess::SetDataInvalid(); }
    DWORD GetMaxDuration() {return m_dwMaxValue;} // return maximum value
    DWORD GetMinDuration() {return m_dwMinValue;} // return minimum value                                  
};

#endif //_SA_P_DUR_H

