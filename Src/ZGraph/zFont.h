// zFONT.H -- Handles Font Manipulations in OS/2 PM

///////////////////////////////////////////////////////////////////////////
//
// ZGRAF C++ Multi-Platform Graph Toolkit v. 1.41, (c) 1996, ZGRAF Software
//
///////////////////////////////////////////////////////////////////////////

#ifndef _zFONT_H_
#define _zFONT_H_

#include "toolkit.h"
#include "zrect.h"
#include "TCHAR.H"

///////////////////////////////////////////////////////////////////////////
//
//                       FONT HANDLING FOR WINDOWS
//
//////////////////////////////////////////////////////////////////////////

#ifdef WINDOWS_PLATFORM

#define zBLACK    RGB(0  ,   0,   0)
#define zWHITE    RGB(255, 255, 255)

class zFONT {
protected:
    WCHAR   szTypeFaceName[80];// Name of Typeface to Use
    HDC      hDC;               // Pointer to a DC
    LOGFONTW  logfont;           // A LOGFONT Container
    HFONT    pNewFont;          // For The New Font
    HFONT    pOldFont;          // For The Old Font
    zRECT    RBounds;           // zRECT. Boundary for String
    BOOL     UseBoldType;       // TRUE If Using Bold Type
    INT      MaxPointSize;      // Set This Variable to Limit Size of Font
    BOOL     FontAllocated;     // TRUE if a New Font's Been Allocated
    INT      FontHeight;        // Height of Font
    INT      FontWidth;         // Width of Font
    COLORREF TextColor;         // Color of Text
    COLORREF BkgColor;          // Color of Background

public:
    zFONT(void) {
        zInitDefaults();
    }

    zFONT(HDC hDC1) {
        zInitDefaults();

        // Set the HDC
        zSetDC(hDC1);
    }

    ~zFONT() {
        // Kill Old Font, If Necessary
        zKillFont();
    }

    void zKillFont(void) {
        // Kills the Font and Restores the Original Font

        if (zIsValidDC()) {
            if (FontAllocated) {
                // Re-Select Old Font
                SelectObject(hDC, pOldFont);

                // Delete Any New Font
                DeleteObject(pNewFont);

                // New Font No Longer Allocated
                FontAllocated = FALSE;
            }

            // Restore Regular Text Justification
            SetTextAlign(hDC, TA_LEFT | TA_TOP);
        }
    }

    void zInitDefaults(void) {
        // Initializes Some Defaults
        pNewFont = 0;                         // NULL The New Font
        pOldFont = 0;                         // NULL The Old Font

        UseBoldType   = FALSE;                // TRUE If Using Bold Type
        MaxPointSize  = 100;                  // TRUE If We Need to Limit Height
        FontAllocated = FALSE;                // Haven't Allocated a New Font Yet
        FontHeight = -1;                      // Height of Font
        FontWidth  = -1;                      // Width of Font

        TextColor = zBLACK;                   // Default Color of Text
        BkgColor  = zWHITE;                   // Default Color of Background

        wcscpy_s(szTypeFaceName, _countof(szTypeFaceName), _T("Arial"));      // Use Arial Font for Default
    }


    // Set the Maximum Point Size to Set an Upper Limit on the Font's Height
    void zSetMaxPointSize(INT mps) {
        MaxPointSize = mps;
    }

    INT      zGetFontWidth(void) {
        return zGetStringWidth(_T("A"));
    }
    INT      zGetFontHeight(void) {
        return zGetStringHeight(_T("A"));
    }

    COLORREF zGetTextColor(void) {
        return TextColor;
    }
    COLORREF zGetBkgColor(void) {
        return BkgColor;
    }
    LPCTSTR   zGetTypeFaceName(void) {
        return (LPCTSTR) szTypeFaceName;
    }
    HDC      zGetDC(void) {
        return hDC;
    }


    // "Set" Member Functions

    void zSetTextColor(COLORREF TextColor1) {
        // Sets the Color Of Text
        if (zIsValidDC()) {
            SetTextColor(hDC, (TextColor = TextColor1));
        }
    }

    void zSetBkgColor(COLORREF BkgColor1) {
        // Sets the Color Of Text
        if (zIsValidDC()) {
            // Select Opaque Mode to Show Bkg. Color Behind Text
            SetBkMode(hDC, OPAQUE);

            // Set the Bkg. Color
            SetBkColor(hDC, (BkgColor  = BkgColor1));
        }
    }

    void zSetDC(HDC hDC1) {
        // Sets the HDC
        hDC = hDC1;
    }

    void SwitchFont(BOOL ApplyNow) {
        // Switches In a New Font
        if (ApplyNow) {
            zCreateNewFont();
        }
    }

    void zSetTypeFaceName(PWCHAR sz1, BOOL ApplyNow = TRUE) {
        // Set Type Face
        wcscpy_s(szTypeFaceName, _countof(szTypeFaceName), sz1);

        SwitchFont(ApplyNow);
    }

    void zSetBold(BOOL state, BOOL ApplyNow = TRUE) {
        // Sets Bold Type On or Off
        UseBoldType = state;

        // Set New Font
        SwitchFont(ApplyNow);
    }

    BOOL zIsValidDC() {
        // Checks If the DC Is Valid

        if (hDC) {
            return TRUE;
        } else {
            zDisplayError(_T("zFONT--DC Not Set!"));
        }

        return (FALSE);
    }


    void zSelectColors(void) {
        // Selects the Text Color and the Background Color

        if (zIsValidDC()) {
            SetTextColor(hDC, TextColor);
            SetBkColor(hDC, BkgColor);
            SetBkMode(hDC, TRANSPARENT);
        }
    }

    void zBorderTextArea(zRECT * pzRECT) {
        // Draws a Border Around the Text Font Area

        // Select Pen
        HPEN pNewPen = CreatePen(PS_SOLID, 1, TextColor);
        HPEN pOldPen  = (HPEN) SelectObject(hDC, pNewPen);

        // Draw the Rectangle
        POINT ptDummy;
        MoveToEx(hDC, pzRECT->left,  pzRECT->top, &ptDummy);
        LineTo(hDC, pzRECT->right, pzRECT->top);
        LineTo(hDC, pzRECT->right, pzRECT->bottom);
        LineTo(hDC, pzRECT->left,  pzRECT->bottom);
        LineTo(hDC, pzRECT->left,  pzRECT->top);

        // Select Old Pen
        SelectObject(hDC, pOldPen);
    }

    void zCreateNewFont(void) {
        // Creates a Font Of Appropriate Size Using the Class Variables for
        //   [FontHeight] and [FontWidth]

        // Kill the Current Font, If Any
        zKillFont();

        // Set Font Height and Width
        logfont.lfHeight = FontHeight;
        logfont.lfWidth  = (FontWidth < 0) ? 0 : FontWidth;
        logfont.lfEscapement     = 0;
        logfont.lfOrientation    = 0;
        logfont.lfWeight         = (UseBoldType) ? FW_BOLD : FW_NORMAL;
        logfont.lfItalic         = FALSE;
        logfont.lfUnderline      = FALSE;
        logfont.lfStrikeOut      = FALSE;
        logfont.lfCharSet        = ANSI_CHARSET;
        logfont.lfOutPrecision   = OUT_TT_PRECIS;
        logfont.lfClipPrecision  = CLIP_DEFAULT_PRECIS;
        logfont.lfQuality        = PROOF_QUALITY;
        logfont.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE | 0x4;

        // Set Font TypeFace
        wcscpy_s((PWCHAR)(logfont.lfFaceName), _countof(logfont.lfFaceName), szTypeFaceName);

        // Create the Font and Install It in the current display context.
        pNewFont = CreateFontIndirect((LPLOGFONT) &logfont);

        // Select the New Font and Save the Old Font
        if (zIsValidDC()) {
            pOldFont = (HFONT) SelectObject(hDC, pNewFont);
        }

        // Now We Have a Font
        FontAllocated = TRUE;

    }

    INT zGetFontHtFromPtSize(int PtSize) {
        // Gets a Font Height from a Font Point Size

        // Create a Font for Drawing the Defect Numbers and Select It Into DC
        long LogY = GetDeviceCaps(hDC, LOGPIXELSY);

        INT TheFontHeight = -PtSize * LogY / 72;  // (See Microsoft Note Q74299)

        // Return the Font Height
        return TheFontHeight;
    }

    INT zGetStringWidth(PCWCH string) {
        // Returns a String's Width
        INT width;

        SIZE s;
        GetTextExtentPoint32W(hDC, string, _tcslen(string), &s);
        width = s.cx;

        return (width);
    }

    INT zGetStringHeight(PCWCH string) {
        // Returns a String's Height
        INT ht;

        SIZE s;
        GetTextExtentPoint32W(hDC, string, _tcslen(string), &s);
        ht = s.cy;

        return (ht);
    }

    void zPickFontAndCenterText(PWCHAR string, zRECT RString) {
        // Picks a Font of Appropriate Size to
        //   Center a String [string] in Rectangle [RString]
        zBestFitText(string, RString, TRUE);
    }

    void zPickFont(PWCHAR string, zRECT RString) {
        // Picks a Font That Will Fit [string] Inside of [RString]
        zBestFitText(string, RString, FALSE);
    }

    void zPickFontForPtSize(int PointSize) {
        // Picks a Font Based Upon Point Size
        FontHeight = zGetFontHtFromPtSize(PointSize);

        zCreateNewFont();

    }

    void zBestFitText(PWCHAR string, zRECT RString, BOOL ShowText = TRUE) {
        // Calculates a Text Font to Best Fit String Within Rectangle [RString].
        //    Displays Text If [ShowText] Flag Is Set to TRUE

        // These are the Legal Point Sizes We Support
        INT LegalPtSizes[] = { 32, 28, 24, 20, 18, 16, 14, 12, 11, 10, 9, 8, 7, -1 };

        // Try a Bunch of Different Point Sizes Until We Find a Font
        //   that Fits Reasonably Well...

        int index = 0;

        // Jump Past Any Font Point Sizes In the Array That Are Larger Than
        //   The Maximum Allowable Point Size
        while ((LegalPtSizes[index] > 0)  && (LegalPtSizes[index] >= MaxPointSize)) {
            index++ ;
        }

        BOOL done = FALSE;
        while (! done  &&  LegalPtSizes[index] > 0) {
            // Pick the Next Point Size
            FontHeight = zGetFontHtFromPtSize(LegalPtSizes[index]);


            // Create a New Font Using This Size
            zCreateNewFont();


            // Get the Actual Size So We Can Check How String Will Fit...
            SIZE s;
            GetTextExtentPoint32W(hDC, string, _tcslen(string), &s);
            INT width  = s.cx;
            INT height = s.cy;

            // Does the String Fit?
            if (width <= RString.Width()  &&  height <= RString.Height()) {
                done = TRUE;
            }

            // Go to Next Point Size in the Array
            index++ ;
        }

        // Select Colors For Showing Text
        zSelectColors();


        // Set Text Alignment
        SetTextAlign(hDC, TA_CENTER | TA_BASELINE);

        // Locate the Point About Which We'll Display the String.
        //    Here We Adjust the Vertical Positioning a Bit, To Account for
        //    the Fact that [TA_BASELINE] Alignment Doesn't Really Align to
        //    the Center Point of the Text Rectangle Area

        POINT pt = RString.Center();
        pt.y += (int)(0.333 * abs(FontHeight));

        // Save Bounding Rectangle
        RBounds = RString;

        // Display the String
        if (ShowText) {
            zDrawTextString(pt.x, pt.y, string);
        }

        // Set Text Alignment Back to Default
        SetTextAlign(hDC, TA_LEFT | TA_TOP);
    }

    void zDrawTextString(INT x, INT y, PWCHAR  string) {
        // Displays a String [string] at [x, y]

        if (zIsValidDC()) {
            zSelectColors();

            // Draw the Text
            TextOutW(hDC, x, y, string, _tcslen(string));
        }
    }

    void zCenterText(PWCHAR string, zRECT RString) {
        // Centers a String [string] in Rectangle.  Does Not
        //  Pick a New Font--Uses the Current Font

        if (zIsValidDC()) {
            zSelectColors();

            // Find Position to Start Text
            POINT ptCenter = RString.Center();
            ptCenter.x -= zGetStringWidth(string) / 2;

            // Draw the Text
            TextOutW(hDC, ptCenter.x, RString.top, string, _tcslen(string));
        }
    }

    void zRightAlignText(INT x, INT y, PWCHAR  string) {
        // Displays a Right-Aligned String [string] at [x, y]

        if (zIsValidDC()) {
            // Set Text Alignment
            SetTextAlign(hDC, TA_RIGHT | TA_BASELINE);

            // Switch In Colors
            zSelectColors();

            // Draw the Text
            TextOutW(hDC, x, y, string, _tcslen(string));

            // Set Text Alignment Back to Default
            SetTextAlign(hDC, TA_LEFT | TA_TOP);
        }
    }

    void zDrawTextString(zRECT RInput, double xPercent, double yPercent, PWCHAR  string) {
        // Displays a String [string] Within [RInput] Using X- and Y- Percentage
        //   Component Offsets

        if (zIsValidDC()) {
            long XPos = RInput.left + Round(RInput.Width()  * xPercent);
            long YPos = RInput.top  + Round(RInput.Height() * yPercent);

            // Switch In Colors
            zSelectColors();

            // Draw the Text
            TextOutW(hDC, XPos, YPos, string, _tcslen(string));

        }
    }

    void zPickFont(zRECT  RString,
                   double xLeftPercent,
                   double yTopPercent,
                   double xRightPercent,
                   double yBottomPercent,
                   PWCHAR  string) {
        // Picks a Font That Will Fit [string] Inside of The Rectangle
        //   [RString], With Rectangle Percentage Component Offsets

        // Calcualate the Target Rectangle From the Input Rectangle and the
        //   Rectangle Percentage Component Offsets
        zRECT ROutput = GetPercentOfRect(RString,
                                         xLeftPercent,
                                         yTopPercent,
                                         xRightPercent,
                                         yBottomPercent);

        // Find a Font That Fits
        zBestFitText(string, &ROutput, FALSE);
    }

    INT Round(double d) {
        // This Function Is Used For Rounding Purposes.  It Returns the closest
        // integer to [d], a double variable

        double LowerLimit = floor(d);

        if ((d - LowerLimit) >= 0.5) {
            return ((INT)LowerLimit + 1);
        } else {
            return ((INT)LowerLimit);
        }
    }

};


#endif

///////////////////////////////////////////////////////////////////////////
//                       FONT HANDLING FOR MS-DOS
//////////////////////////////////////////////////////////////////////////

#if defined(MS_DOS_PLATFORM)

#ifdef MS_DOS_PLATFORM
#include <graph.h>
#endif


class zFONT {
protected:
    zRECT    RBounds;           // zRECT. Boundary for String
    BOOL     UseBoldType;       // TRUE If Using Bold Type
    BOOL     FontAllocated;     // TRUE if a New Font's Been Allocated
    INT      FontHeight;        // Height of Font
    INT      FontWidth;         // Width of Font
    zRGB     TextColor;         // Color of Text
    zRGB     BkgColor;          // Color of Background

public:
    zFONT(void) {
        zInitDefaults();
    }

    ~zFONT() {
        // Kill Old Font, If Necessary
        zKillFont();
    }

    inline void zSelectDefaultFont() {
        // Selects the Default Font
        extern void zDOSSelectDefaultFont();

        zDOSSelectDefaultFont();
    }

    void zKillFont(void) {
        // Kills the Font and Restores the Original Font


        if (FontAllocated) {
            // New Font No Longer Allocated
            FontAllocated = FALSE;

            zSelectDefaultFont();
        }
    }

    void zInitDefaults(void) {
        // Initializes Some Defaults
        UseBoldType   = FALSE;                // TRUE If Using Bold Type
        FontAllocated = FALSE;                // Haven't Allocated a New Font Yet
        FontHeight = -1;                      // Height of Font
        FontWidth  = -1;                      // Width of Font

        TextColor = zDARK_BLACK;              // Default Color of Text
        BkgColor  = zBRT_WHITE;               // Default Color of Background

    }

    // "Get" Member Functions

#ifdef MS_DOS_PLATFORM
    _fontinfo zGetAvgFontSize(void) {
        // Returns the Average Font Height and Width Information
        _fontinfo fi;
        _getfontinfo(&fi);

        return (fi);
    }

    INT zGetStringWidth(PWCHAR string) {
        // Returns Length of a Graphics String Using Default Font
        return (_getgtextextent(string));
    }

    INT zGetFontWidth(void) {
        return zGetAvgFontSize().avgwidth;
    }
    INT zGetFontHeight(void) {
        return zGetAvgFontSize().pixheight;
    }
#endif

    zRGB zGetTextColor(void) {
        return TextColor;
    }
    zRGB zGetBkgColor(void) {
        return BkgColor;
    }

    void zSetTextColor(zRGB TextColor1) {
        // Sets the Color Of Text
#ifdef MS_DOS_PLATFORM
        _setcolor((TextColor = TextColor1));
#endif

    }

    void zSetBkgColor(zRGB BkgColor1) {
        // Set the Bkg. Color
        BkgColor = BkgColor1;
    }

    void zSelectColors(void) {
        // Selects the Text Color and the Background Color

        zSetTextColor(TextColor);
        zSetBkgColor(BkgColor);
    }

    void zPickFont(PWCHAR string, zRECT RString) {
        // Picks a Font That Will Fit [string] Inside of [RString]
        zBestFitText(string, RString, FALSE);
    }

    void zBestFitText(PWCHAR string, zRECT RString, BOOL ShowText = TRUE) {
        // Calculates a Text Font to Best Fit String Within Rectangle [RString].
        //    Displays Text If [ShowText] Flag Is Set to TRUE

        extern void zDOSShowTextInRect(PWCHAR string,
                                       zRECT RString,
                                       BOOL ShowText);

        // Select Colors For Showing Text
        zSelectColors();

        zDOSShowTextInRect(string, RString, ShowText);

    }

    void zDrawTextString(INT x, INT y, PWCHAR string) {
        // Displays a String [string] at [x, y]

        // Select Colors
        zSelectColors();

        // Display the String at the Proper Position
#ifdef MS_DOS_PLATFORM
        _moveto(x, y);
        _outgtext(string);
#endif

    }

    void zCenterText(PWCHAR string, zRECT RString) {
        // Centers a String [string] in Rectangle.  Does Not
        //  Pick a New Font--Uses the Current Font

        zSelectColors();

        // Find Position to Start Text
        POINT ptCenter = RString.Center();
        ptCenter.x -= zGetStringWidth(string) / 2;

        // Draw the Text
#ifdef MS_DOS_PLATFORM
        _moveto(ptCenter.x, RString.top);
        _outgtext(string);
#endif

    }

    void zRightAlignText(INT x, INT y, PWCHAR  string) {
        // Displays a Right-Aligned String [string] at [x, y]

        // Switch In Colors
        zSelectColors();

        // Draw the Text Simulating Right-Alignment
#ifdef MS_DOS_PLATFORM
        _moveto(x - zGetStringWidth(string), y);
        _outgtext(string);
#endif

#ifdef DOS_BGI_PLATFORM
        outtextxy(x - zGetStringWidth(string), y, string);
#endif
    }

    void zDrawTextString(zRECT RInput, double xPercent, double yPercent, PWCHAR  string) {
        // Displays a String [string] Within [RInput] Using X- and Y- Percentage
        //   Component Offsets

        short XPos = RInput.left + Round(RInput.Width()  * xPercent);
        short YPos = RInput.top  + Round(RInput.Height() * yPercent);

        // Switch In Colors
        zSelectColors();

        // Draw the Text
        zDrawTextString(XPos, YPos, string);

    }

    void zPickFont(zRECT  RString,
                   double xLeftPercent,
                   double yTopPercent,
                   double xRightPercent,
                   double yBottomPercent,
                   PWCHAR  string) {
        // Picks a Font That Will Fit [string] Inside of The Rectangle
        //   [RString], With Rectangle Percentage Component Offsets

        // Calcualate the Target Rectangle From the Input Rectangle and the
        //   Rectangle Percentage Component Offsets
        zRECT ROutput = GetPercentOfRect(RString,
                                         xLeftPercent,
                                         yTopPercent,
                                         xRightPercent,
                                         yBottomPercent);

        // Find a Font That Fits
        zBestFitText(string, &ROutput, FALSE);
    }

    INT Round(double d) {
        // This Function Is Used For Rounding Purposes.  It Returns the closest
        // integer to [d], a double variable

        double LowerLimit = floor(d);

        if ((d - LowerLimit) >= 0.5) {
            return ((INT)LowerLimit + 1);
        } else {
            return ((INT)LowerLimit);
        }
    }

};


#endif // #if defined(MS_DOS_PLATFORM)


#endif

