/////////////////////////////////////////////////////////////////////////////
// sa_w_rev.h:
// Interface of the CProcessWbEcho
//          CProcessWbReverb classes.
// Author: Urs Ruchti
// copyright 1997 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////
#ifndef _SA_W_REV_H
#define _SA_W_REV_H
#include "WbProcess.h"

class CProcessWbEcho : public CWbProcess {
public:
    CProcessWbEcho(Context & context);
    long Process(void * pCaller, Model * pModel, int nProgress = 0, int nLevel = 1);
    virtual void WriteProperties(ObjectOStream & obs);
    virtual BOOL ReadProperties(ObjectIStream & obs);
    void SetDelay(UINT val) { m_nDelay = val; };
    void SetGain(int val) { m_nGain = val; };
    UINT GetDelay() { return m_nDelay; };
    int GetGain() { return m_nGain; };

private:
    UINT m_nDelay; // delay in ms
    int  m_nGain;  // gain in dB
};

//###########################################################################
// CProcessWbReverb data processing

class CProcessWbReverb : public CWbProcess {
public:
    CProcessWbReverb(Context & context);
    long Process(void * pCaller, Model * pModel, int nProgress = 0, int nLevel = 1);
    virtual void WriteProperties(ObjectOStream & obs);
    virtual BOOL ReadProperties(ObjectIStream & obs);
    void SetDelay(UINT val) { m_nDelay = val; };
    void SetGain(int val) { m_nGain = val; };
    UINT GetDelay() { return m_nDelay; };
    int GetGain() { return m_nGain; };

private:
    UINT m_nDelay; // delay in ms
    int  m_nGain;  // gain in dB
};

#endif
