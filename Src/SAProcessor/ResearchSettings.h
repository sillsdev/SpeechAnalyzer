#ifndef _RESEARCHSETTINGS_H
#define _RESEARCHSETTINGS_H

#include "SA_DSP.h"

// Graph Parameter.cpp for implementation
class CResearchSettings : public ResearchSettings {
public:
    CResearchSettings() {
        Init();
    }
    
    void Init();
    
    int getLpcCepstralSmooth() { return m_nLpcCepstralSmooth; }
    int getLpcCepstralSharp() { return m_nLpcCepstralSharp; }
    int getSpectrumLpcOrderFsMult() { return m_nSpectrumLpcOrderFsMult; }
    int getSpectrumLpcOrderAuxMax() { return m_nSpectrumLpcOrderAuxMax; }
    int getSpectrumLpcMethod() { return m_nSpectrumLpcMethod; }
    int getSpectrumLpcOrderExtra() { return m_nSpectrumLpcOrderExtra; }
    CWindowSettings getWindow() { return m_cWindow;}
    void setWindow(CWindowSettings window) { m_cWindow = window; }

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
