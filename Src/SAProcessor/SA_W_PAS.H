/////////////////////////////////////////////////////////////////////////////
// sa_w_pas.h:
// Interface of the CProcessWbHighpass
//          CProcessWbLowpass
//          CProcessWbBandpass classes.
// Author: Urs Ruchti
// copyright 1997 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////
#ifndef _SA_W_PAS_H
#define _SA_W_PAS_H

#include "Butterworth.h"
#include "WbProcess.h"

class CProcessWbHighpass : public CProcessButterworth {
public:
    CProcessWbHighpass(Context& context);
    long Process(void* pCaller, int nProgress = 0, int nLevel = 1);
    virtual void WriteProperties(CObjectOStream& obs);
    virtual BOOL ReadProperties(CObjectIStream& obs);
    UINT GetCutoff() {return m_nCutoff; };
    UINT GetOrder() { return m_nOrder; };
    BOOL GetFilter() { return m_bFilterFilter; };
    void SetCutoff(UINT val) { m_nCutoff = val; };
    void SetOrder(UINT val) { m_nOrder = val; };
    void SetFilter(BOOL val) { m_bFilterFilter = val; };
private:
    UINT m_nCutoff; // cutoff frequency
    UINT m_nOrder;
    BOOL m_bFilterFilter;

};

//###########################################################################
// CProcessWbLowpass data processing

class CProcessWbLowpass : public CProcessButterworth {
public:
    CProcessWbLowpass(Context& context);
    long Process(void* pCaller, int nProgress = 0, int nLevel = 1);
    virtual void WriteProperties(CObjectOStream& obs);
    virtual BOOL ReadProperties(CObjectIStream& obs);
    UINT GetCutoff() { return m_nCutoff; };
    UINT GetOrder() { return m_nOrder; };
    BOOL GetFilter() { return m_bFilterFilter; };
    void SetCutoff(UINT val) { m_nCutoff = val; };
    void SetOrder(UINT val) { m_nOrder = val; };
    void SetFilter(BOOL val) { m_bFilterFilter = val; };
private:
    UINT m_nCutoff; // cutoff frequency
    UINT m_nOrder;
    BOOL m_bFilterFilter;
};

class CProcessWbBandpass : public CProcessButterworth {
public:
    CProcessWbBandpass(Context& context);
    long Process(void* pCaller, int nProgress = 0, int nLevel = 1);
    virtual void WriteProperties(CObjectOStream& obs);
    virtual BOOL ReadProperties(CObjectIStream& obs);
    UINT GetUpperCutoff() { return m_nUpperCutoff; };
    UINT GetLowerCutoff() { return m_nLowerCutoff; };
    UINT GetOrder() { return m_nOrder; };
    BOOL GetFilter() { return m_bFilterFilter; };
    void SetUpperCutoff(UINT val) { m_nUpperCutoff = val; };
    void SetLowerCutoff(UINT val) { m_nLowerCutoff = val; };
    void SetOrder(UINT val) { m_nOrder = val; };
    void SetFilter(BOOL val) { m_bFilterFilter = val; };
private:
    UINT m_nUpperCutoff; // upper cutoff frequency
    UINT m_nLowerCutoff; // lower cutoff frequency
    UINT m_nOrder;
    BOOL m_bFilterFilter;

};

#endif //_SA_W_PAS_H
