/////////////////////////////////////////////////////////////////////////////
// sa_p_zcr.h:
// Interface of the CProcessZCross class
// Author: Urs Ruchti
// copyright 1996 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////
#ifndef _SA_P_ZCR_H
#define _SA_P_ZCR_H

#include "sa_process.h"

class CProcessZCross : public CProcess {
public:
    CProcessZCross(Context& context) : CProcess(context) {}
    CProcessZCross() = delete;

public:
    long Process(void * pCaller, int nProgress = 0, int nLevel = 1);
};

#endif //_SA_P_ZCR_H
