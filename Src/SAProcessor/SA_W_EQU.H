/////////////////////////////////////////////////////////////////////////////
// sa_w_equ.h:
// Interface of the CProcessWbEquation class
// Author: Urs Ruchti
// copyright 1997 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////
#ifndef _SA_W_EQU_H
#define _SA_W_EQU_H

#include "WbProcess.h"
#include "ObjectIStream.h"
#include "ObjectOStream.h"

class CProcessWbEquation : public CWbProcess {
public:
    CProcessWbEquation(Context& context);

    BOOL CheckFunction(LPCSTR); // check the function string
    long Process(void* pCaller, int nProgress = 0, int nLevel = 1);
    virtual void WriteProperties(CObjectOStream& obs);
    virtual BOOL ReadProperties(CObjectIStream& obs);
    const char* GetEquation() {
        return m_szEquation.c_str();
    }
    void SetEquation(const char* value) {
        m_szEquation = value;
    }
    bool ReadStreamString(CObjectIStream& stream, LPCSTR pszMarker, string& szResult);

private:
    string m_szEquation; // equation string
    double ParseUtf8(LPCSTR, INT* att, double X, double Y = 0.0);
};

#endif //_SA_W_EQU_H
