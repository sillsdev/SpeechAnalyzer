#include "pch.h"
#include "AbstractPitchProcess.h"
#include "Process.h"

/**
* Returns uncertainty at a specific pitch
*/
double CAbstractPitchProcess::GetUncertainty(double fPitch) {
    double fFs = m_CalcParm.sampfreq; // sampling frequency
    double fUncertainty = fPitch * fPitch / (2 * fFs - fPitch);
    if (fUncertainty < 0.1) {
        fUncertainty = 0.1;
    }
    return fUncertainty;
}
