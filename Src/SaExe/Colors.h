#ifndef _COLORS_H

#include "appdefs.h"

#define _COLORS_H

class Object_ostream;
class Object_istream;

class Colors
{
public:

    void WriteProperties(Object_ostream & obs);
    BOOL ReadProperties(Object_istream & obs);
    void SetupDefault(BOOL bSystem= TRUE, BOOL bPrinting= FALSE);
    void SetupSystemColors();

    COLORREF     cPlotBkg;          // plot background color
    COLORREF     cPlotHiBkg;        // plot highlighted background color
    COLORREF     cPlotAxes;         // plot titles and axes color
    COLORREF     cPlotData[6];      // plot data color
    COLORREF     cPlotHiData;       // plot highlighted data color
    COLORREF     cPlotBoundaries;   // plot boundaries color
    COLORREF     cPlotGrid;         // plot gridlines color
    COLORREF     cPlotStartCursor;  // plot start cursor color
    COLORREF     cPlotStopCursor;   // plot stop cursor color
    COLORREF     cPlotPrivateCursor;// plot private cursor color
    COLORREF     cScaleBkg;         // scale windows background color
    COLORREF     cScaleLines;       // scale window lines color
    COLORREF     cScaleFont;        // scale window font color
    COLORREF   cSysBtnHilite;    // system button hilite color
    COLORREF   cSysBtnShadow;    // system button shadow color
    COLORREF   cSysBtnFace;    // system button face color
    COLORREF   cSysBtnText;    // system button text color
    COLORREF   cSysColorHilite;  // system hilite color
    COLORREF   cSysColorHiText;  // system hilite text color
    COLORREF   cSysActiveCap;    // system active caption color
    COLORREF   cSysCapText;    // system caption text color
    COLORREF   cSysInActiveCap;  // system active caption color
    COLORREF   cSysInActiveCapText; // system caption text color
    COLORREF   cBackupColor;      // needed for overlay colors

    COLORREF     cAnnotationBkg[ANNOT_WND_NUMBER]; // annotation window background color
    COLORREF     cAnnotationFont[ANNOT_WND_NUMBER]; // annotation window font color

protected:
    static void GreyScale(COLORREF & rgb);

};


#endif  //_COLORS_H
