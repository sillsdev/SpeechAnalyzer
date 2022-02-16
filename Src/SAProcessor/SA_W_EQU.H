/////////////////////////////////////////////////////////////////////////////
// sa_w_equ.h:
// Interface of the CProcessWbEquation class
// Author: Urs Ruchti
// copyright 1997 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////
#ifndef _SA_W_EQU_H
#define _SA_W_EQU_H
#include "WbProcess.h"

class CProcessWbEquation : public CWbProcess {
public:
    CProcessWbEquation(Context * pContext);

    BOOL CheckFunction(string*); // check the function string
    long Process(void* pCaller, Model* pModel, int nProgress = 0, int nLevel = 1);
    virtual void WriteProperties(ObjectOStream& obs);
    virtual BOOL ReadProperties(ObjectIStream& obs);
    const char* GetEquation() {
        return m_szEquation.c_str();
    }
    void SetEquation(const char* value) {
        m_szEquation = value;
    }
private:
    string m_szEquation; // equation string
    double ParseUtf8(const string &, INT* att, double X, double Y = 0.0);
};

#endif //_SA_W_EQU_H
