#pragma once

namespace PitchProcess {
    long Process(void* pCaller, ISaDoc* pDoc, int nProgress = 0, int nLevel = 1);
    double GetUncertainty(double fPitchData);
}
