/////////////////////////////////////////////////////////////////////////////
// sa_p_InstantaneousPower.h:
// Interface of the CProcessInstantaneousPower class.
// Author: Steve MacLean
// copyright 2002 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////
#ifndef _SA_P_InstantaneousPower_H
#define _SA_P_InstantaneousPower_H

class CProcessInstantaneousPower : public CProcess {
public:
    CProcessInstantaneousPower(Context context) : CProcess(context) {}
    long Process(void * pCaller, int nProgress = 0, int nLevel = 1);
};

#endif
