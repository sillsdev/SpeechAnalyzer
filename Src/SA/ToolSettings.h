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
	// TRUE, if colors changed by user
    bool m_bColorsChanged;
	// internal color structure
    Colors m_cColors;       
    int m_nGraphSelect;
    int m_nAnnotationSelect;
    int m_nScaleSelect;
    int m_nOverlaySelect;
    int m_nTaskbarSelect;

    //font page
	// TRUE, if fonts changed by user
    BOOL m_bFontChanged;
	// array of graph font face strings
    CStringArray m_GraphFonts;
	// array of graph font sizes
    CUIntArray m_GraphFontSizes;
	// Experimental....
    BOOL m_bUseUnicodeEncoding;

    // save page
    BOOL m_saveOpenFiles;
    BOOL m_showStartupDlg;
    CString m_szPermGraphs;
    CString m_szTempGraphs;
    CString m_szPermCurrLabel;
    CString m_szTempCurrLabel;

    // audio page
    BOOL m_bShowAdvancedAudio;

	// algorithm used for AudioSync import function
	int m_nAlgorithm;
};

#endif
