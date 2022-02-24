#pragma once
#include "sa_process.h"

class CAbstractPitchProcess : public CProcess {

public:
    CAbstractPitchProcess(Context context) : CProcess(context) {};
    CAbstractPitchProcess() = delete;

    double GetUncertainty(double fPitch);

protected:
    SGrapplParms m_CalcParm;
};

