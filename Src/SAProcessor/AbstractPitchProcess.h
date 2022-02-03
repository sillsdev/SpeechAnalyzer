#pragma once
#include "Process.h"

class CAbstractPitchProcess : public CProcess {

protected:
    SGrapplParms m_CalcParm;

public:
    double GetUncertainty(double fPitch);
};

