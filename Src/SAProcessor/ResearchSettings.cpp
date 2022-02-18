#include "pch.h"
#include "ResearchSettings.h"
#include "Lpc.h"

static LPCSTR psz_parse            = "parse";
static LPCSTR psz_breakWidth       = "break_ms";
static LPCSTR psz_phraseBreakWidth = "phrase_break_ms";
static LPCSTR psz_minThreshold     = "minThreshold";
static LPCSTR psz_maxThreshold     = "maxThreshold";

void CResearchSettings::Init() {
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
}
