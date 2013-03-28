#include "Stdafx.h"
#include "ToolSettings.h"

CToolSettings::CToolSettings()
{
}

CToolSettings::CToolSettings(const CToolSettings & right)
{
    *this = right;
}

CToolSettings & CToolSettings::operator=(const CToolSettings & right)
{

    // view page
    m_bStatusbar = right.m_bStatusbar;
    m_bToolbar = right.m_bToolbar;
    m_bScrollZoom = right.m_bScrollZoom;
    m_nCaptionStyle = right.m_nCaptionStyle;
    m_bXGrid = right.m_bXGrid;
    m_bYGrid = right.m_bYGrid;
    m_nXStyleIndex = right.m_nXStyleIndex;
    m_nYStyleIndex = right.m_nYStyleIndex;
    m_nCursorAlignment = right.m_nCursorAlignment;
    m_nPitchMode = right.m_nPitchMode;
    m_nPosMode = right.m_nPosMode;
    m_bToneAbove = right.m_bToneAbove;
    m_nGraphUpdateMode = right.m_nGraphUpdateMode;
    m_bAnimate = right.m_bAnimate;
    m_nAnimationRate = right.m_nAnimationRate;
    m_bTaskbar = right.m_bTaskbar;
    m_nDlgXStyle = right.m_nDlgXStyle;
    m_nDlgYStyle = right.m_nDlgYStyle;

    // color page
    m_bColorsChanged = right.m_bColorsChanged;
    m_cColors = right.m_cColors;
    m_nGraphSelect = right.m_nGraphSelect;
    m_nAnnotationSelect = right.m_nAnnotationSelect;
    m_nScaleSelect = right.m_nScaleSelect;
    m_nOverlaySelect = right.m_nOverlaySelect;

    //font page
    m_bFontChanged = right.m_bFontChanged;
    m_GraphFonts.RemoveAll();
    for (int i=0; i<right.m_GraphFonts.GetCount(); i++)
    {
        m_GraphFonts.Add(right.m_GraphFonts.GetAt(i));
    }
    m_GraphFontSizes.RemoveAll();
    for (int i=0; i<right.m_GraphFontSizes.GetCount(); i++)
    {
        m_GraphFontSizes.Add(right.m_GraphFontSizes.GetAt(i));
    }
    m_bUseUnicodeEncoding = right.m_bUseUnicodeEncoding;

    // save page
    m_saveOpenFiles = right.m_saveOpenFiles;
    m_showStartupDlg = right.m_showStartupDlg;
    m_szPermGraphs = right.m_szPermGraphs;
    m_szTempGraphs = right.m_szTempGraphs;
    m_szPermCurrLabel = right.m_szPermCurrLabel;
    m_szTempCurrLabel = right.m_szTempCurrLabel;

    // audio page
    m_bShowAdvancedAudio = right.m_bShowAdvancedAudio;
    return *this;
}
