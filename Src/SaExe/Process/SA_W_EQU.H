/////////////////////////////////////////////////////////////////////////////
// sa_w_equ.h:
// Interface of the CProcessWbEquation class
// Author: Urs Ruchti
// copyright 1997 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////
#ifndef _SA_W_EQU_H

#include "CSAString.h"

#define _SA_W_EQU_H

//###########################################################################
// CProcessWbEquation data processing

class CProcessWbEquation : public CProcess
{
// Construction/destruction/creation
public:
    CProcessWbEquation();
    virtual ~CProcessWbEquation();

    // Attributes
private:
    CSaString m_szEquation; // equation string

    // Operations
public:
    BOOL CheckFunction(CSaString *); // check the function string
    long Process(void * pCaller, ISaDoc * pDoc, int nProgress = 0, int nLevel = 1);
    virtual int PropertiesDialog(); // calls the properties dialog for this process
    virtual void WriteProperties(Object_ostream & obs);
    virtual BOOL ReadProperties(Object_istream & obs);
};

#endif //_SA_W_EQU_H
