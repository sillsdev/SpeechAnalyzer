/////////////////////////////////////////////////////////////////////////////
// sa_p_raw.h:
// Interface of the CProcessRaw class
// Author: Urs Ruchti
// copyright 1996 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////
#ifndef _SA_P_RAW_H
#define _SA_P_RAW_H

class CProcessRaw : public CProcess {
public:
    CProcessRaw(Context& context) : CProcess(context) {}
    CProcessRaw() = delete;
    long Process(void * pCaller, int nProgress = 0, int nLevel = 1);
};

#endif
