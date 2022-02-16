/////////////////////////////////////////////////////////////////////////////
// sa_p_InstantaneousPower.h:
// Interface of the CProcessInstantaneousPower class.
// Author: Steve MacLean
// copyright 2002 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////
#ifndef _SA_P_InstantaneousPower_H
#define _SA_P_InstantaneousPower_H

//###########################################################################
// CProcessInstantaneousPower data processing

class CProcessInstantaneousPower : public CProcess {
public:
    long Process(void * pCaller, Model * pModel, int nProgress = 0, int nLevel = 1);
};

#endif //_SA_P_InstantaneousPower_H
