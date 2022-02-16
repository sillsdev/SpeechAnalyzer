#pragma once
#include "Process.h"

class CAbstractPitchProcess : public CProcess {

protected:
    CAbstractPitchProcess(Context* pContext);
    SGrapplParms m_CalcParm;

public:
    double GetUncertainty(double fPitch);
};

