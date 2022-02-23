#ifndef _RESEARCHSETTINGS_H
#define _RESEARCHSETTINGS_H

#include "SA_DSP.h"
#include "lpc.h"

struct SResearchSettings {

    bool spectrogramConnectFormants;
    bool spectrogramContrastEnhance;
    bool showHilbertTransform;
    bool showInstantaneousPower;
    int lpcCepstralSmooth;
    int lpcCepstralSharp;
    int spectrumLpcOrderFsMult;
    int spectrumLpcOrderAuxMax;
    int spectrumLpcMethod;
    int spectrumLpcOrderExtra;
    SWindowSettings window;

    void init() {
        spectrogramConnectFormants = false;
        spectrogramContrastEnhance = false;
        showHilbertTransform = false;
        showInstantaneousPower = false;
        spectrumLpcMethod = LPC_COVAR_LATTICE;
        spectrumLpcOrderFsMult = 1;
        spectrumLpcOrderExtra = 1;
        spectrumLpcOrderAuxMax = 4;
        lpcCepstralSmooth = -1;
        lpcCepstralSharp = 0;
    };

    bool hasSpectrumSettingsChanged(SResearchSettings& right) {
        return spectrumLpcMethod != right.spectrumLpcMethod ||
            spectrumLpcOrderFsMult != right.spectrumLpcOrderFsMult ||
            spectrumLpcOrderExtra != right.spectrumLpcOrderExtra ||
            spectrumLpcOrderAuxMax != right.spectrumLpcOrderAuxMax ||
            window != right.window ||
            ((right.spectrumLpcMethod == LPC_CEPSTRAL) &&
             (lpcCepstralSharp != right.lpcCepstralSharp || lpcCepstralSmooth != right.lpcCepstralSmooth));
    }
};

#endif
