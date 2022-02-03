/////////////////////////////////////////////////////////////////////////////
// sa_w_rev.h:
// Interface of the CProcessWbEcho
//          CProcessWbReverb classes.
// Author: Urs Ruchti
// copyright 1997 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////
#ifndef _SA_W_REV_H
#define _SA_W_REV_H

class CProcessWbEcho : public CProcess {
public:
    CProcessWbEcho(Context & context);

    long Process(void * pCaller, Model * pModel, int nProgress = 0, int nLevel = 1);
    virtual int PropertiesDialog(); // calls the properties dialog for this process
    virtual void WriteProperties(ObjectOStream & obs);
    virtual BOOL ReadProperties(ObjectIStream & obs);

private:
    UINT m_nDelay; // delay in ms
    int  m_nGain;  // gain in dB
};

//###########################################################################
// CProcessWbReverb data processing

class CProcessWbReverb : public CProcess {
public:
    CProcessWbReverb();
    virtual ~CProcessWbReverb();

    long Process(void * pCaller, Model * pModel, int nProgress = 0, int nLevel = 1);
    virtual int PropertiesDialog(); // calls the properties dialog for this process
    virtual void WriteProperties(ObjectOStream & obs);
    virtual BOOL ReadProperties(ObjectIStream & obs);

private:
    UINT m_nDelay; // delay in ms
    int  m_nGain;  // gain in dB
};

#endif
