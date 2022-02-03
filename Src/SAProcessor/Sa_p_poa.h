/////////////////////////////////////////////////////////////////////////////
// sa_p_poa.h:
// Interface of the CProcessPOA class
// Author: Alec Epting
// copyright 1999 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////
#ifndef _SA_P_POA_H
#define _SA_P_POA_H

#define MODEL_SECTION_COUNT  18

class CProcessPOA : public CProcess {

// Attributes
private:

// Operations
protected:
    virtual long Exit(int nError, HANDLE hDataFrame); // exit processing on error
public:
    virtual long Process(void * pCaller, Model * pModel, DWORD dwStart, DWORD dwStop, int nProgress = 0, int nLevel = 1);
    virtual DWORD GetDataSize();
    virtual DWORD GetDataSize(size_t nElements);
    virtual void * GetProcessedData(DWORD dwOffset = 0, BOOL bBlockBegin = FALSE); // return POA data pointer on given position (offset)
    virtual int GetProcessedData(DWORD dwOffset, BOOL *); // return POA data from given position (offset)
    long Process(void * pCaller, Model * pModel, int nWidth, int nHeight, int nProgress = 0, int nLevel = 1);
};

#endif //_SA_P_POA_H

