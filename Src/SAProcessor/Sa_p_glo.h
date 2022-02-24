/////////////////////////////////////////////////////////////////////////////
// sa_p_glo.h:
// Interface of the CProcessGlottis class
// Author: Alec Epting
// copyright 1997 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////
#ifndef _SA_P_GLO_H
#define _SA_P_GLO_H

//###########################################################################
// CProcessGlottis data processing

class CProcessGlottis : public CProcess {
public:
    long Process(void * pCaller, int nProgress = 0, int nLevel = 1);
};

#endif //_SA_P_GLO_H
