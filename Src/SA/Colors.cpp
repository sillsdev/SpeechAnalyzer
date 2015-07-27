#include "stdafx.h"
#include "objectistream.h"
#include "objectostream.h"
#include "colors.h"


static LPCSTR psz_colors = "colors";
static LPCSTR psz_rgb = "rgb";
static LPCSTR psz_annotationbkg = "annotationbkg";
static LPCSTR psz_annotationfont = "annotationfont";
static LPCSTR psz_plotcolors = "plotcolors";
static LPCSTR psz_scalecolors = "scalecolors";
static LPCSTR psz_overlay = "overlay";


void Colors::WriteProperties(CObjectOStream & obs) {
    obs.WriteBeginMarker(psz_colors);

    obs.WriteBeginMarker(psz_plotcolors);
    obs.WriteCOLORREF(psz_rgb, cPlotBkg);
    obs.WriteCOLORREF(psz_rgb, cPlotHiBkg);
    obs.WriteCOLORREF(psz_rgb, cPlotAxes);
    obs.WriteCOLORREF(psz_rgb, cPlotData[0]);
    obs.WriteCOLORREF(psz_rgb, cPlotData[1]);
    obs.WriteCOLORREF(psz_rgb, cPlotData[2]);
    obs.WriteCOLORREF(psz_rgb, cPlotData[3]);
    obs.WriteCOLORREF(psz_rgb, cPlotData[4]);
    obs.WriteCOLORREF(psz_rgb, cPlotData[5]);
    obs.WriteCOLORREF(psz_rgb, cPlotHiData);
    obs.WriteCOLORREF(psz_rgb, cPlotBoundaries);
    obs.WriteCOLORREF(psz_rgb, cPlotGrid);
    obs.WriteCOLORREF(psz_rgb, cPlotStartCursor);
    obs.WriteCOLORREF(psz_rgb, cPlotStopCursor);
    obs.WriteCOLORREF(psz_rgb, cPlotPrivateCursor);
    obs.WriteEndMarker(psz_plotcolors);

    obs.WriteBeginMarker(psz_scalecolors);
    obs.WriteCOLORREF(psz_rgb, cScaleBkg);
    obs.WriteCOLORREF(psz_rgb, cScaleLines);
    obs.WriteCOLORREF(psz_rgb, cScaleFont);
    obs.WriteEndMarker(psz_scalecolors);

    int i=0;

    obs.WriteBeginMarker(psz_annotationbkg);
    for (; i<ANNOT_WND_NUMBER; i++) {
        obs.WriteCOLORREF(psz_rgb, cAnnotationBkg[i]);
    }
    obs.WriteEndMarker(psz_annotationbkg);

    obs.WriteBeginMarker(psz_annotationfont);
    for (i=0; i<ANNOT_WND_NUMBER; i++) {
        obs.WriteCOLORREF(psz_rgb, cAnnotationFont[i]);
    }
    obs.WriteEndMarker(psz_annotationfont);

    obs.WriteEndMarker(psz_colors);
}

BOOL Colors::ReadProperties(CObjectIStream & obs) {
    int i = 0;

    if (!obs.bAtBackslash() || !obs.bReadBeginMarker(psz_colors)) {
        return FALSE;
    }

    while (!obs.bAtEnd()) {
        if (obs.bReadBeginMarker(psz_plotcolors)) {
            obs.bReadCOLORREF(psz_rgb, cPlotBkg);
#define RC(rgb) obs.bReadCOLORREF(psz_rgb, rgb)
            RC(cPlotHiBkg);
            RC(cPlotAxes);
            RC(cPlotData[0]);
            RC(cPlotData[1]);
            RC(cPlotData[2]);
            RC(cPlotData[3]);
            RC(cPlotData[4]);
            RC(cPlotData[5]);
            RC(cPlotHiData);
            RC(cPlotBoundaries);
            RC(cPlotGrid);
            RC(cPlotStartCursor);
            RC(cPlotStopCursor);
            RC(cPlotPrivateCursor);
            obs.SkipToEndMarker(psz_plotcolors);
        } else if (obs.bReadBeginMarker(psz_scalecolors)) {
            RC(cScaleBkg);
            RC(cScaleLines);
            RC(cScaleFont);
            obs.SkipToEndMarker(psz_scalecolors);
        } else if (obs.bReadBeginMarker(psz_annotationbkg)) {
            for (; i<ANNOT_WND_NUMBER; i++) {
                obs.bReadCOLORREF(psz_rgb, cAnnotationBkg[i]);
            }
            obs.SkipToEndMarker(psz_annotationbkg);
        } else if (obs.bReadBeginMarker(psz_annotationfont)) {
            for (i=0; i<ANNOT_WND_NUMBER; i++) {
                obs.bReadCOLORREF(psz_rgb, cAnnotationFont[i]);
            }
            obs.SkipToEndMarker(psz_annotationfont);
        } else if (obs.bEnd(psz_colors)) {
            break;
        }
    }

    return TRUE;
}



void Colors::GreyScale(COLORREF & rgb) {
    BYTE r = GetRValue(rgb);
    BYTE g = GetGValue(rgb);
    BYTE b = GetBValue(rgb);
    BYTE grey = (BYTE)(((r*30) + (g*59) + (b*11))/100);

    rgb = RGB(grey,grey,grey);
}



/***************************************************************************/
// Colors::SetupDefaultColors Set the default colors
// If the flag bSystem is TRUE, it sets up the colors with system defaults,
// otherwise with a custom setup.
/***************************************************************************/
void Colors::SetupDefault(BOOL bSystem, BOOL bPrinting) {

    if (bPrinting) {
        // setup colors for b/w printing.

        GreyScale(cPlotBkg);
        GreyScale(cPlotHiBkg);
        GreyScale(cPlotAxes);
        GreyScale(cPlotData[0]);
        GreyScale(cPlotData[1]);
        GreyScale(cPlotData[2]);
        GreyScale(cPlotData[3]);
        GreyScale(cPlotData[4]);
        GreyScale(cPlotData[5]);
        GreyScale(cPlotHiData);
        GreyScale(cPlotBoundaries);
        GreyScale(cPlotGrid);
        GreyScale(cPlotStartCursor);
        GreyScale(cPlotStopCursor);
        GreyScale(cPlotPrivateCursor);

        for (int nLoop = 0; nLoop < ANNOT_WND_NUMBER; nLoop++) {
            GreyScale(cAnnotationBkg[nLoop]);
            GreyScale(cAnnotationFont[nLoop]);
        }

        GreyScale(cScaleBkg);
        GreyScale(cScaleLines);
        GreyScale(cScaleFont);
        GreyScale(cSysBtnHilite);
        GreyScale(cSysBtnShadow);
        GreyScale(cSysBtnFace);
        GreyScale(cSysBtnText);
        GreyScale(cSysColorHilite);
        GreyScale(cSysColorHiText);
        GreyScale(cSysActiveCap);
        GreyScale(cSysCapText);
        GreyScale(cSysInActiveCap);
        GreyScale(cSysInActiveCapText);

    } else if (bSystem) {
        // use system colors
        cPlotBkg            = RGB(225,255,255);
        cPlotHiBkg          = RGB(0,128,192);
        cPlotAxes           = GetSysColor(COLOR_WINDOWTEXT);
        cPlotData[0]        = GetSysColor(COLOR_WINDOWTEXT);
        cPlotData[1]        = RGB(255, 128, 0);
        cPlotData[2]        = RGB(0, 0, 255); // blue. Was GetSysColor(COLOR_ACTIVEBORDER);
        cPlotData[3]        = GetSysColor(COLOR_WINDOWFRAME);
        cPlotData[4]        = RGB(0, 0, 255); // blue. Was GetSysColor(COLOR_MENU);
        cPlotData[5]        = GetSysColor(COLOR_HIGHLIGHT);
        cPlotHiData         = GetSysColor(COLOR_HIGHLIGHTTEXT);
        cPlotBoundaries     = GetSysColor(COLOR_GRAYTEXT);
        cPlotGrid           = RGB(0, 0, 0);
        cPlotStartCursor    = RGB(0, 128, 0); // dark green
        cPlotStopCursor     = RGB(255, 0, 0); // red
        cPlotPrivateCursor  = RGB(0, 0, 255); // blue

        for (int nLoop = 0; nLoop < ANNOT_WND_NUMBER; nLoop++) {
            cAnnotationBkg[nLoop] = GetSysColor(COLOR_BTNFACE);
        }
		// overrides
        cAnnotationBkg[0] = RGB(255,255,200);
        cAnnotationBkg[1] = RGB(255,232,200);
        cAnnotationBkg[2] = RGB(202,255,202);
        cAnnotationBkg[3] = RGB(200,225,255);
        cAnnotationBkg[4] = RGB(244,213,255);
        cAnnotationBkg[5] = RGB(244,235,185);
        cAnnotationBkg[6] = RGB(220,220,175);

        for (int nLoop = 0; nLoop < ANNOT_WND_NUMBER; nLoop++) {
            cAnnotationFont[nLoop] = GetSysColor(COLOR_BTNTEXT);
        }

        cScaleBkg           = RGB(250,250,230);
        cScaleLines         = RGB(0,0,255);
        cScaleFont          = RGB(0,0,255);
        SetupSystemColors();
    } else {
        // custom set

        cPlotBkg            = RGB(0, 0, 0);			// black
        cPlotHiBkg          = RGB(0, 0, 128);		// dark blue
        cPlotAxes           = RGB(255, 255, 255);	// white
        cPlotData[0]        = RGB(0, 255, 0);		// green
        cPlotData[1]        = RGB(255, 128, 0);		// orange
        cPlotData[2]        = RGB(128,   0, 0);
        cPlotData[3]        = RGB(192 , 0, 0);
        cPlotData[4]        = RGB(0, 192, 0);
        cPlotData[5]        = RGB(128, 0, 128);
        cPlotHiData         = RGB(255, 255, 0);		// yellow
        cPlotBoundaries     = RGB(128, 0, 64);		// brown
        cPlotGrid           = RGB(192, 192, 192);	// light gray
        cPlotStartCursor    = RGB(0, 255, 255);		// light blue
        cPlotStopCursor     = RGB(255, 0, 0);		// red
        cPlotPrivateCursor  = RGB(255, 255, 0);		// yellow

        for (int nLoop = 0; nLoop < ANNOT_WND_NUMBER; nLoop++) {
            cAnnotationBkg[nLoop]  = RGB(192, 192, 192); // light gray
            cAnnotationFont[nLoop] = RGB(0, 0, 160);	// dark blue
        }
        cScaleBkg           = RGB(192, 192, 192);	// light gray
        cScaleLines         = RGB(64, 128, 128);	// dark green gray
        cScaleFont          = RGB(64, 128, 128);	// dark green gray
        SetupSystemColors();
    }
}

/***************************************************************************/
// Colors::SetupSystemColors Set the system colors
/***************************************************************************/
void Colors::SetupSystemColors() {
    cSysBtnHilite       = GetSysColor(COLOR_BTNHIGHLIGHT);
    cSysBtnShadow       = GetSysColor(COLOR_BTNSHADOW);
    cSysBtnFace         = GetSysColor(COLOR_BTNFACE);
    cSysBtnText         = GetSysColor(COLOR_BTNTEXT);
    cSysColorHilite     = GetSysColor(COLOR_HIGHLIGHT);
    cSysColorHiText     = GetSysColor(COLOR_HIGHLIGHTTEXT);
    cSysActiveCap       = GetSysColor(COLOR_ACTIVECAPTION);
    cSysCapText         = GetSysColor(COLOR_CAPTIONTEXT);
    cSysInActiveCap     = GetSysColor(COLOR_INACTIVECAPTION);
    cSysInActiveCapText = GetSysColor(COLOR_INACTIVECAPTIONTEXT);
}
