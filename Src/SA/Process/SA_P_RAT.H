/////////////////////////////////////////////////////////////////////////////
// sa_p_rat.h:
// Interface of the CProcessRatio class
// Author: Urs Ruchti
// copyright 1997 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////
#ifndef _SA_P_RAT_H
#define _SA_P_RAT_H

//###########################################################################
// CProcessRatio data processing

class CProcessRatio : public CProcess {
// Construction/destruction/creation
public:
    CProcessRatio();
    virtual ~CProcessRatio();

// Attributes
private:

// Operations
public:
    long Process(void * pCaller, ISaDoc * pDoc, int nProgress = 0, int nLevel = 1);
};

#endif //_SA_P_RAT_H
