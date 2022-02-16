/////////////////////////////////////////////////////////////////////////////
// sa_p_raw.h:
// Interface of the CProcessRaw class
// Author: Urs Ruchti
// copyright 1996 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////
#ifndef _SA_P_RAW_H
#define _SA_P_RAW_H

//###########################################################################
// CProcessRaw data processing

class CProcessRaw : public CProcess {
public:
    long Process(void * pCaller, Model * pModel, int nProgress = 0, int nLevel = 1);
};

#endif //_SA_P_RAW_H
