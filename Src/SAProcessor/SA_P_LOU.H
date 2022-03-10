/////////////////////////////////////////////////////////////////////////////
// sa_p_lou.h:
// Interface of the CProcessLoudness classes.
// Author: Urs Ruchti
// copyright 1996 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////
#pragma once
#ifndef _SA_P_LOU_H
#define _SA_P_LOU_H

#include "sa_process.h"

class CProcessLoudness : public CProcess {
public:
    CProcessLoudness(Context& context) : CProcess(context) {};
    CProcessLoudness() = delete;

    long Process(void * pCaller, int nProgress = 0, int nLevel = 1);
    virtual void SetDataInvalid();
};

#endif
