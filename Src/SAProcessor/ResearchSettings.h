#ifndef _RESEARCHSETTINGS_H
#define _RESEARCHSETTINGS_H

#include "SA_DSP.h"
#include "lpc.h"

class CResearchSettings {
public:
    CResearchSettings() {
        m_bSpectrogramConnectFormants = FALSE;
        m_bSpectrogramContrastEnhance = FALSE;
        m_bShowHilbertTransform = FALSE;
        m_bShowInstantaneousPower = FALSE;
        m_nSpectrumLpcMethod = LPC_COVAR_LATTICE;
        m_nSpectrumLpcOrderFsMult = 1;
        m_nSpectrumLpcOrderExtra = 1;
        m_nSpectrumLpcOrderAuxMax = 4;
        m_nLpcCepstralSmooth = -1;
        m_nLpcCepstralSharp = 0;
    };

    int GetLpcCepstralSmooth() { return m_nLpcCepstralSmooth; }
    int GetLpcCepstralSharp() { return m_nLpcCepstralSharp; }
    int GetSpectrumLpcOrderFsMult() { return m_nSpectrumLpcOrderFsMult; }
    int GetSpectrumLpcOrderAuxMax() { return m_nSpectrumLpcOrderAuxMax; }
    int GetSpectrumLpcMethod() { return m_nSpectrumLpcMethod; }
    int GetSpectrumLpcOrderExtra() { return m_nSpectrumLpcOrderExtra; }
    CWindowSettings GetWindow() { return m_cWindow;}
    void SetWindow(CWindowSettings window) { m_cWindow = window; }

    BOOL m_bSpectrogramConnectFormants;
    BOOL m_bSpectrogramContrastEnhance;
    BOOL m_bShowHilbertTransform;
    BOOL m_bShowInstantaneousPower;

private:
    int m_nLpcCepstralSmooth;
    int m_nLpcCepstralSharp;
    int m_nSpectrumLpcOrderFsMult;
    int m_nSpectrumLpcOrderAuxMax;
    int m_nSpectrumLpcMethod;
    int m_nSpectrumLpcOrderExtra;
    CWindowSettings m_cWindow;
};

#endif
