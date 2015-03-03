#ifndef TOOLSETTINGS_H
#define TOOLSETTINGS_H

#include "Colors.h"

class CToolSettings {

public:
    CToolSettings();
    CToolSettings(const CToolSettings & right);
    CToolSettings & operator=(const CToolSettings & from);

    // view page
    BOOL m_bStatusbar;
    BOOL m_bToolbar;
    BOOL m_bScrollZoom;
    int m_nCaptionStyle;
    BOOL m_bXGrid;
    BOOL m_bYGrid;
    int m_nXStyleIndex;
    int m_nYStyleIndex;
    int m_nCursorAlignment;
    int m_nPitchMode;
    int m_nPosMode;
    BOOL m_bToneAbove;
    int m_nGraphUpdateMode;
    BOOL m_bAnimate;
    int m_nAnimationRate;
    BOOL m_bTaskbar;
    int m_nDlgXStyle;
    int m_nDlgYStyle;

    // color page
    BOOL m_bColorsChanged;  // TRUE, if colors changed by user
    Colors m_cColors;       // internal color structure
    int m_nGraphSelect;
    int m_nAnnotationSelect;
    int m_nScaleSelect;
    int m_nOverlaySelect;

    //font page
    BOOL m_bFontChanged;            // TRUE, if fonts changed by user
    CStringArray m_GraphFonts;      // array of graph font face strings
    CUIntArray m_GraphFontSizes;    // array of graph font sizes
    BOOL m_bUseUnicodeEncoding;     // Experimental....

    // save page
    BOOL m_saveOpenFiles;           // tdg - 09/03/97
    BOOL m_showStartupDlg;          // DDO - 08/03/00
    CString m_szPermGraphs;
    CString m_szTempGraphs;
    CString m_szPermCurrLabel;
    CString m_szTempCurrLabel;

    // audio page
    BOOL m_bShowAdvancedAudio;
};

#endif
