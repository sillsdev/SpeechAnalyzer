// LOWLEVEL.CPP
//
/////////////////////////////////////////////////////////////////////////////
//   This Module Consists of the Low-Level Graphics Functions.
//   The "zLowLevelGraphics" Class is Used as a Base Class, From Which
//   Other Classes (Like the "zGraph" Class) Are Derived.
//
// ZGRAF C++ Multi-Platform Graph Toolkit v. 1.41, (c) 1996, ZGRAF Software
//
/////////////////////////////////////////////////////////////////////////////
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>

//
// Platform-Dependent Includes...
//
#ifdef WINDOWS_PLATFORM
#include <windows.h>
#include "zfont.h"
#define MoveTo(a,b,c) MoveToEx(a,b,c,NULL);
#endif

#include "toolkit.h"                             // Toolkit Header File
#include "lowlevel.h"                            // Basic Graph Class

#ifdef MS_DOS_PLATFORM
#include "graph.h"
#include "scrnsave.h"
#endif

//
// Instantiate Static Variables Declared in "zLowLevelGraphics" Class
//
RECT zLowLevelGraphics::RScreen      ;   // The Screen Area Rectangle
INT  zLowLevelGraphics::ScreenWidth  ;   // The Width  of the Screen In Pixels
INT  zLowLevelGraphics::ScreenHeight ;   // The Height of the Screen In Pixels
INT  zLowLevelGraphics::NumFonts     ;   // The Number of Fonts We Support


/////////////////////////////////////////////////////////////////////////////
////////////////////// zLowLevelGraphics Class //////////////////////////////
/////////////////////////////////////////////////////////////////////////////

zLowLevelGraphics::zLowLevelGraphics() {
    //
    // Constructor for zLowLevelGraphics Class.
    //
}

////////////////////////////// ~zLowLevelGraphics() ////////////////////////////////

zLowLevelGraphics::~zLowLevelGraphics() {
    //
    // Destructor for zLowLevelGraphics Class
    //
}


///////////////////////////////// zRectWidth() /////////////////////////////

INT zLowLevelGraphics::zRectWidth(RECT * R) {
    //
    // Returns the Width of a Rectangle [R]
    //
    return (abs(R->left - R->right));
}

//////////////////////////////// zRectHeight() /////////////////////////////

INT zLowLevelGraphics::zRectHeight(RECT * R) {
    //
    // Returns the Height of a Rectangle [R]
    //
    return (abs(R->top - R->bottom));
}

/////////////////////////////// zRectCenter() //////////////////////////////

POINT zLowLevelGraphics::zRectCenter(RECT * R) {
    //
    // Returns the Center Point of a Rectangle
    //
    POINT ptCenter =
    { (R->left + R->right)/2, (R->top + R->bottom)/2 };
    return (ptCenter);
}

///////////////////////// zCreateShadingBrush() ///////////////////////////

zFILL zLowLevelGraphics::zCreateShadingBrush(zRGB color) {
    // Takes a Color [color] and Creates a Brush Used For Color Shading.
    // The Shading Brush Will Contain the Same Relative Proportion of
    // Red, Green, and Blue, But Will Be a Darker Hue.

    //  NOTE:  We Make Each R, G, B Component a Fraction as Bright as
    //  the Original Color's Component...
    //  For LIGHT  SHADING Use 50%   (Right Shift of 1 Bit)
    //  For MEDIUM SHADING Use 25%   (Right Shift of 2 Bits)
    //  For HEAVY  SHADING Use 12.5% (Right Shift of 3 Bits)

#ifdef WINDOWS_PLATFORM

    static HBRUSH hBr;

    // Create R, G, B Shaded Color Values

    BYTE Red   = BYTE(GetRValue(color) >> shading_style);
    BYTE Green = BYTE(GetGValue(color) >> shading_style);
    BYTE Blue  = BYTE(GetBValue(color) >> shading_style);

    // Create a Brush...

    hBr = CreateSolidBrush(RGB(Red, Green, Blue));

    return ((zFILL) hBr);

#endif

#if defined(MS_DOS_PLATFORM)
    UNUSED_PARAMETERS(&color);

    // Operation Not Defined for this Platform
    return (zFILL)0;
#endif

}


////////////////////////////// zDrawWedge() /////////////////////////////

void zLowLevelGraphics::zDrawWedge(POINT P1, POINT P2, POINT P3, POINT P4,
                                   POINT P5, POINT P6,
                                   zPATTERN   Pattern,
                                   zFILL      InteriorColor,
                                   zLINE      BoundaryColor,
                                   BOOL       DrawFullWedge /* = TRUE */) {
    //
    // Draws and Fills in A Wedge-Shaped Region Composed of
    //   Polygon Points P1 thru P6.
    //   [Pattern], [InteriorColor], and [BoundaryColor]
    //   Are Used to Specify the Colors of Various Entities In the Wedge.
    //   If the [DrawFullWedge] Flag is Set TRUE, a Full Wedge is Drawn.
    //   If FALSE, Only the Top Portion of the Wedge is Drawn.
    //   This is a Key Function Used in Drawing Graphs Containing
    //   Area Fill Regions
    //


    ///////////////////////////////////////////////
    // Draw the Top of the Wedge
    ///////////////////////////////////////////////

    POINT point_2[] = {
        {P3.x, P3.y},
        {P4.x, P4.y},
        {P5.x, P5.y},
        {P6.x, P6.y}
    };
    zDrawPolygon(point_2, 4, Pattern, InteriorColor, BoundaryColor);

    //
    // Stop Now if We're Only Drawing the Top of the Wedge...
    //
    if (! DrawFullWedge) {
        return;
    }

    ///////////////////////////////////////////////
    // Draw the Front of the Wedge
    ///////////////////////////////////////////////

    POINT point_1[] = {
        {P1.x, P1.y},
        {P2.x, P2.y},
        {P3.x, P3.y},
        {P4.x, P4.y}
    };
    zDrawPolygon(point_1, 4, Pattern, InteriorColor, BoundaryColor);


    ///////////////////////////////////////////////
    // Draw the Right Side of the Wedge
    ///////////////////////////////////////////////

#ifdef WINDOWS_PLATFORM
    //
    // If We're Doing Shading, We Need to Create and Switch In a
    //   Special Brush to Handle the Shading.
    //
    HBRUSH zOldBrush=NULL, zNewBrush=NULL;

    if (shading_style > 0) {
        // Get The R,G,B Color Values From the Current Brush

        LOGBRUSH logBrush;
        GetObject((HBRUSH)InteriorColor,sizeof(LOGBRUSH),(PWCHAR)&logBrush);

        COLORREF brushColor = logBrush.lbColor;

        // Now Create a Brush That's Shaded
        zOldBrush = (HBRUSH)SelectObject(hDC, (zNewBrush = zCreateShadingBrush(brushColor)));
    }
#endif

#if defined(MS_DOS_PLATFORM)
    zFILL zNewBrush = InteriorColor;
#endif

    INT y_ht = abs(P3.y - P2.y);
    POINT P7 =
    { P6.x, P6.y + y_ht } ;

    POINT point_3[] = {
        {P2.x, P2.y},
        {P3.x, P3.y},
        {P6.x, P6.y},
        {P7.x, P7.y}
    };
    zDrawPolygon(point_3, 4, Pattern,
                 (shading_style > 0) ? zNewBrush : InteriorColor,
                 BoundaryColor);


#ifdef WINDOWS_PLATFORM
    // Restore Old Brush and Destroy Newly-Created One, if Necessary

    if (shading_style > 0) {
        SelectObject(hDC, zOldBrush);
        DeleteObject(zNewBrush);
    }
#endif

}

//////////////////////////// zDraw3DBar() /////////////////////////////////

void zLowLevelGraphics::zDraw3DBar(INT left, INT top,
                                   INT right, INT bottom,
                                   INT        depth,
                                   zPATTERN   Pattern,
                                   zFILL      InteriorColor,
                                   zLINE      BoundaryColor) {
    //
    // Draws a 3-D bar with front face (left, top, right, bottom)
    // and depth [depth], using the Fill Colors/Parameters Given
    //

    if (depth > 0) {

        ///////////////////////////////////////////////
        // Draw And Fill in the Top of Bar
        ///////////////////////////////////////////////

        POINT point_1[] = {
            { left         , top       },
            { left  + depth, top-depth },
            { right + depth, top-depth },
            { right        , top       }
        };
        zDrawPolygon(point_1, 4, Pattern, InteriorColor, BoundaryColor);


        ///////////////////////////////////////////////
        // Draw and Fill in the Right Side of the Bar
        ///////////////////////////////////////////////

#ifdef WINDOWS_PLATFORM
        //
        // If We Are Using Shading, We Now Need to Create and Switch In a
        //   Special Brush to Handle the Shading.
        //
        HBRUSH zOldBrush=NULL, zNewBrush=NULL;

        if (shading_style > 0) {
            // Get The R,G,B Color Values From the Current Brush

            LOGBRUSH logBrush;
            GetObject((HBRUSH)InteriorColor, sizeof(LOGBRUSH), (PWCHAR)&logBrush);

            COLORREF brushColor = logBrush.lbColor;

            // Now Create a Brush That's Shaded
            zOldBrush = (HBRUSH)SelectObject(hDC, (zNewBrush = zCreateShadingBrush(brushColor)));
        }
#endif

#ifdef MS_DOS_PLATFORM
        zFILL zNewBrush = InteriorColor;
#endif

        POINT point_2[] = {
            { right + depth , top-depth      },
            { right         , top            },
            { right         , bottom         },
            { right + depth , bottom - depth }
        };
        zDrawPolygon(point_2, 4, Pattern,
                     (shading_style > 0) ? zNewBrush : InteriorColor,
                     BoundaryColor);


#ifdef WINDOWS_PLATFORM
        //
        // Restore Old Brush and Destroy Newly-Created One, if Necessary
        //
        if (shading_style > 0) {
            SelectObject(hDC, zOldBrush);
            DeleteObject(zNewBrush);
        }
#endif
    }

    ///////////////////////////////////////////////
    // Draw and Fill in the Front of the Bar
    ///////////////////////////////////////////////

    POINT point_3[] = {
        { left,  top     },
        { left,  bottom  },
        { right, bottom  },
        { right, top     }
    };
    zDrawPolygon(point_3, 4, Pattern, InteriorColor, BoundaryColor);

}

/////////////////////////////// zDrawRect() ///////////////////////////

void zLowLevelGraphics::zDrawRect(RECT * R) {
    //
    // This Function Draws an Unfilled Rectangle [R] in the Window
    //   Using the Axis Pen
    //

#ifdef WINDOWS_PLATFORM
    SelectObject(hDC, hAxisPen);

    POINT pDummy;
    MoveToEx(hDC, R->left, R->top, &pDummy);

    LineTo(hDC, R->right, R->top);
    LineTo(hDC, R->right, R->bottom);
    LineTo(hDC, R->left,  R->bottom);
    LineTo(hDC, R->left,  R->top);
#endif

#ifdef MS_DOS_PLATFORM
    _setcolor((short)AxisColor);
    _settextcolor((short)AxisColor);
    _rectangle(_GBORDER, R->left, R->top, R->right, R->bottom);
#endif

}


///////////////////////////// zMoveTo() /////////////////////////////////

void zLowLevelGraphics::zMoveTo(INT xpos, INT ypos) {
    //
    // This Function is a Just a Quick Wrapper Around a Function
    //   to Move the Pen to [xpos, ypos]
    //

#ifdef WINDOWS_PLATFORM
    POINT pDummy;
    MoveToEx(hDC, xpos, ypos, &pDummy);
#endif

#ifdef MS_DOS_PLATFORM
    _moveto(xpos, ypos);
#endif

}

///////////////////////////// zLineTo() /////////////////////////////////

void zLowLevelGraphics::zLineTo(INT xpos, INT ypos) {
    //
    // This Function is a Just a Quick Wrapper Around a Function
    //   to Draw a Line From the Current Pen Position to [xpos, ypos]
    //

#ifdef WINDOWS_PLATFORM
    LineTo(hDC, xpos, ypos);
#endif

#ifdef MS_DOS_PLATFORM
    _lineto(xpos, ypos);
#endif

}

//////////////////////////// zDrawLine() /////////////////////////////////

void zLowLevelGraphics::zDrawLine(INT x1, INT y1, INT x2, INT y2) {
    //
    // This Function is a Just a Quick Wrapper Around a Function
    //   to Draw a Line From (x1, y1) to (x2, y2)
    //

#ifdef WINDOWS_PLATFORM
    POINT pDummy;

    MoveToEx(hDC, x1, y1, &pDummy);
    LineTo(hDC, x2, y2);
#endif

#ifdef MS_DOS_PLATFORM
    _moveto(x1, y1);
    _lineto(x2, y2);
#endif

}

////////////////////////////// zDrawCircle() //////////////////////////////

void zLowLevelGraphics::zDrawCircle(INT x1, INT y1, INT r1, double AspectRatio) {
    //
    // Draws a circle on the screen, with center at (x1, y1),
    // and with radius [r1].  AspectRatio is the device aspect
    // ratio. [--much like Borland's BGI circle() function]
    //

#ifdef WINDOWS_PLATFORM
    // Code to Draw a Circle in Windows...
    RECT RBounds = {
        x1 - r1,
        y1 - zRound(AspectRatio * r1),
        x1 + r1,
        y1 + zRound(AspectRatio * r1)
    };

    Arc(hDC, RBounds.left, RBounds.top, RBounds.right, RBounds.bottom,
        x1, zRound(y1 - AspectRatio * r1),
        x1, zRound(y1 - AspectRatio * r1));
#endif

#ifdef MS_DOS_PLATFORM
    // Code to Draw a Circle with Microsoft DOS Graphics..

    UNUSED_PARAMETERS(&AspectRatio);

    RECT RBounds = {
        x1 - r1,
        y1 - zRound(AspectRatio * r1),
        x1 + r1,
        y1 + zRound(AspectRatio * r1)
    };

    // Draw an Ellipse that Is a Circle
    _ellipse(_GBORDER, RBounds.left, RBounds.top, RBounds.right, RBounds.bottom);
#endif

}


///////////////////////////// zDrawPolygon() //////////////////////////////

void zLowLevelGraphics::zDrawPolygon(POINT * PtArray,
                                     INT      NumPoints,
                                     zPATTERN /*Pattern*/,
                                     zFILL    InteriorColor,
                                     zLINE    BoundaryColor) {
    //
    // Draws a Polygon Consisting of [NumPoints] Points in the
    //   [PtArray] Array.  This is a KEY Function for Drawing
    //   Polygons, and Is Implemented Differently for Each Platform
    //   That We'll Be Running Under...
    //

#ifdef WINDOWS_PLATFORM
    ////// NOTE /////
    //   Might Want to Add Code Here to Validate Pen and Brush
    //   Before Using Them!
    /////////////////

    // Select Proper Pen for Drawing Border Line of Polygon
    HPEN hpenOld = (HPEN) SelectObject(hDC, (HPEN) BoundaryColor);

    // Select Proper Brush for Filling Polygon
    HBRUSH hbrOld = (HBRUSH) SelectObject(hDC, (HBRUSH) InteriorColor);

    SetPolyFillMode(hDC, ALTERNATE);
    SetBkMode(hDC, TRANSPARENT);

    // Draw the Polygon
    Polygon(hDC, PtArray, NumPoints);


    // Restore Old Pen and Brush
    SelectObject(hDC, hpenOld);
    SelectObject(hDC, hbrOld);
#endif

#ifdef MS_DOS_PLATFORM
    // Set Color of Fill
    short OldColor = _setcolor((short)InteriorColor);

    // Create a Dynamic Array to Hold the Points
    _xycoord * pPoly = new _xycoord [NumPoints + 2];

    // Set Up the Polygon Points Array
    for (int i = 0; i < NumPoints; i++) {
        pPoly[i].xcoord = PtArray[i].x;
        pPoly[i].ycoord = PtArray[i].y;
    }

    // Select Type of Fill Mask
    zDOSSelectFill(fill_style, (int)Pattern);

    // Draw In the Polygon Interior Fill
    _polygon(_GFILLINTERIOR, pPoly, NumPoints);

    // Restore Original (Solid) Fill
    _setfillmask(NULL);

    // Draw the Border to the Polygon
    _setcolor((short)BoundaryColor);
    _polygon(_GBORDER, pPoly, NumPoints);

    // Clean Up
    delete [] pPoly;

    // Reselect Old Color
    _setcolor((short)OldColor);
#endif

}

/////////////////////////// zDrawTextString() ///////////////////////////////

void zLowLevelGraphics::zDrawTextString(INT x1, INT y1, PWCHAR Text) {
    //
    // This Function is a Just a Quick Wrapper Around
    //  a Function to Draw a Text String [Text] at [x1, y1]...
    //

#ifdef WINDOWS_PLATFORM           // Windows
    TextOutW(hDC, x1, y1, Text, _tcslen(Text));
#endif

// Microsoft DOS
#ifdef MS_DOS_PLATFORM
    // Microsoft Graphics Does Not Support Text Alignment,
    //   So We Have to Account for the Alignment Manually Here

    // Get Width of String In Pixels
    INT TextWidth = _getgtextextent(Text);

    switch (zAlignFormat) {
    case zALIGN_RIGHT_TOP: // Right-Alignment
        _moveto(x1 - TextWidth, y1);
        break;

    case zALIGN_CENTER:    // Center Alignment
        _moveto(x1 - TextWidth/2, y1);
        break;

    case zALIGN_LEFT_TOP:  // Left-Alignment
    default:
        _moveto(x1, y1);
        break;
    }

    // Display the String at the Proper Position
    _outgtext(Text);
#endif


}

///////////////////////// zDrawFilledRect() ///////////////////////////

void zLowLevelGraphics::zDrawFilledRect(INT left, INT top,
                                        INT right, INT bottom, BYTE Red, BYTE Green, BYTE Blue) {
    //
    // This Function Draws a Filled Rectangle in the Window, Using
    //   a Fill Color With the Given Red, Green, and Blue Components
    //

#ifdef WINDOWS_PLATFORM
    // Create a brush with desired shade; We Use
    //   Palette-Relative Index To Get Smoother Colors on
    //   Palette Devices.

    HBRUSH hMyNewBrush = CreateSolidBrush(PALETTERGB(Red, Green, Blue));
    HBRUSH hMyOldBrush = (HBRUSH)SelectObject(hDC, hMyNewBrush);

    // We Use the "Rectangle()" API

    Rectangle(hDC, left, top, right, bottom);

    SelectObject(hDC, hMyOldBrush);
    DeleteObject(hMyNewBrush);

#endif

#if defined(MS_DOS_PLATFORM)

    // Function Is Not Defined Yet!
    UNUSED_PARAMETERS(&left, &top, &right, &bottom,
                      &Red, &Green, &Blue);

#endif
}

////////////////////////// zGetDefaultTextSize() //////////////////////////

void zLowLevelGraphics::zGetDefaultTextSize() {
    //
    // This Function Computes the Average Character Width, Height for
    //   the Default Font
    //

#ifdef WINDOWS_PLATFORM   // Windows
    TEXTMETRIC tm;

    GetTextMetrics(hDC, &tm);
    charsize   = tm.tmAveCharWidth;
    charheight = tm.tmHeight;
#endif

#ifdef MS_DOS_PLATFORM    // Microsoft DOS
    _fontinfo fi;
    _getfontinfo(&fi);

    charsize   = fi.avgwidth;
    charheight = fi.pixheight;
#endif

}

///////////////////////////// zActivatePens() /////////////////////////////

void zLowLevelGraphics::zActivatePens() {
    //
    // This Function Sets Up Several Pen Objects for Line Styles, etc.
    //

#ifdef WINDOWS_PLATFORM
    //
    //   [hAxisPen] is Used for Drawing the Graph Axis and/or the
    //     Graph Borders.  [ hLinePen[i] ] is Used to Draw in the
    //     Graph Lines
    //
    if (! pens_created) {        // Skip if We've Already Created the Pens
        //   and They're Already Available

        // Set Up Pen for Grid

        INT GridType;
        switch (GridPenStyle) {
        case zDASHED:
            GridType = PS_DASH;
            break;

        case zDOTTED:
            GridType = PS_DOT;
            break;

        case zDASH_DOT:
            GridType = PS_DASHDOT;
            break;

        case zDASH_DOT_DOT:
            GridType = PS_DASHDOTDOT;
            break;

        case zSOLID:
        default:
            GridType = PS_SOLID;
            break;
        }
        hGridPen = CreatePen(GridType, 0, AxisColor);


        // Set Up Pen for Graph Axes
        hAxisPen = CreatePen(PS_SOLID, 0, AxisColor);

        // Set Up Line Pens
        for (INT i = 0; i < zMAX_GRAPH_COLORS; i++) {
            hLinePen[i] = CreatePen(PS_SOLID, 0, GraphColor[i]);
        }
    }

    //
    // Switch In the Axis Pen, and Use the Appropriate Axis Color
    //
    SetTextColor(hDC, AxisColor);       // Color of Numbers, Letters
    SelectObject(hDC, hAxisPen);        // Color of X-Y axis lines

#endif

#ifdef MS_DOS_PLATFORM
    //
    //     Set Up A Mask to Be Used with "_setlinestyle()"
    //     to Draw in the Graph Lines With the Appropriate Look
    //
    if (! pens_created) {        // Skip if We've Already Created the Pens
        //   and They're Already Available

        // Set Up Mask to Be Used In Drawing Grid

        switch (GridPenStyle) {
        case zDASHED:
            GridMask = 0xFCFC;
            break;

        case zDOTTED:
            GridMask = 0xCCCC;
            break;

        case zDASH_DOT:
            GridMask = 0xFF18;
            break;

        case zDASH_DOT_DOT:
            GridMask = 0xFF66;
            break;

        case zSOLID:
        default:
            GridMask = 0xFFFF;
            break;
        }
    }

    //
    // Switch In the Axis Color
    //
    _setcolor((short)AxisColor);       // Color of Numbers, Letters
#endif

    pens_created = TRUE;
}

///////////////////////////// zDeletePens() /////////////////////////////

void zLowLevelGraphics::zDeletePens() {
    //
    // Delete Any Pen Objects That We've Created
    //

#ifdef WINDOWS_PLATFORM
    //
    // Deletes Any Windows Pen GDI Objects
    //
    if (pens_created) {
        // Kill Axis Pen
        DeleteObject(hAxisPen);

        // Kill Grid Pen
        DeleteObject(hGridPen);

        // Kill Line Pens
        for (INT i = 0; i < zMAX_GRAPH_COLORS; i++) {
            DeleteObject(hLinePen[i]);
        }
    }
#endif


#ifdef MS_DOS_PLATFORM
    //  MS DOS Graphics Doesn't Require Anything Special Here!
#endif

}

////////////////////////// zActivateBrushes() ////////////////////////////

void zLowLevelGraphics::zActivateBrushes() {
    //
    // This function is used to set up a variety of Solid or Hatch-Filled
    // Styled Brushes, for Filling in Sections of the Pie, Bar, and
    // Area Graphs.  It is Assumed that the GraphColor[] Array is Loaded with
    // zFILL values used for the brushes.  Class Data variable
    // [fill_style] should be one of the following values (We default to
    // [zSOLID_FILL]):
    //
    //   zSOLID_FILL       -- Creates a SOLID COLOR FILL, Using a
    //                          Different Color for Each Fill Region
    //
    //   zCOLOR_HATCH_FILL -- Creates a COLOR HATCHED FILL, Using a
    //                          Different Color for Each Fill Region
    //                          NOTE:
    //                          WINDOWS--There are Only 6 Unique Fills of this
    //                                   type (Windows GDI Limitation)!
    //
    //   zMONO_HATCH_FILL  -- Creates a SINGLE-COLORED HATCH FILL for
    //                          Each Fill Region


    if (brushes_created) {       // Skip if We've Already Created the Brushes
        return;    //   and They're Now Available
    }


#ifdef WINDOWS_PLATFORM
    //
    // Array of Bit Maps for Brushes
    //
    static WORD zBrushBits[ zMAX_DATA_SETS ][ 8 ] = {
        { 0xF7, 0xFB, 0xFD, 0xFE,  0x7F, 0xBF, 0xDF, 0xEF },  // Left Diag. Lines
        { 0xFF, 0x99, 0x99, 0xFF,  0xFF, 0x99, 0x99, 0xFF },  // Dot Patt. #1
        { 0xFF, 0xFF, 0xFF, 0xE7,  0xE7, 0xFF, 0xFF, 0xFF },  // Small Dot
        { 0xFF, 0xBD, 0xE7, 0xFF,  0xBD, 0xE7, 0xBD, 0xFF },  // Pattern A
        { 0xFF, 0xFF, 0xC7, 0xC7,  0xC7, 0xFF, 0xFF, 0xFF },  // Medium Dot
        { 0xEF, 0xDF, 0xBF, 0x7F,  0xFE, 0xFD, 0xFB, 0xF7 },  // Rt. Diag Lines
        { 0xCC, 0xCC, 0x33, 0x33,  0xCC, 0xCC, 0x33, 0x33 },  // Dot Pattern  #2
        { 0xFF, 0xFF, 0xC3, 0xDB,  0xDB, 0xC3, 0xFF, 0xFF },  // A Small Square
        { 0xE7, 0xDB, 0xBD, 0x7E,  0x7E, 0xBD, 0xDB, 0xE7 },  // A Diamond
        { 0xDB, 0xDB, 0x00, 0xDB,  0xDB, 0x00, 0xDB, 0xDB }   // '#' Sign
    } ;

    for (INT i = 0; i < zMAX_GRAPH_COLORS; i++) { // Use 1 Brush for each Item
        switch (fill_style) {
        case zCOLOR_HATCH_FILL:
            hFillBrush[ i ] = CreateHatchBrush(i % 6, GraphColor[i]) ;
            break;

        case zMONO_HATCH_FILL :
            hBitmap[ i ] = CreateBitmap(8, 8, 1, 1,
                                        (PWCHAR) &zBrushBits[ i ][0]);
            hFillBrush[ i ]  = CreatePatternBrush(hBitmap[ i ]);
            DeleteObject(hBitmap[ i ]);
            break;

        case zSOLID_FILL :
        default          :
            hFillBrush[ i ] = CreateSolidBrush(GraphColor[i]);
            break;
        }
    }
#endif

#if defined(MS_DOS_PLATFORM)

    for (INT i = 0; i < zMAX_GRAPH_COLORS; i++) { // Use 1 Pattern for each Item
        switch (fill_style) {
        case zMONO_HATCH_FILL :
            // We Want Mono Fills--Ensure All Colors are the Same
            //   By Making Them all the Same as the Axis Color.
            //   This Emulates The Windows Behaviour
            GraphColor[ i ] = AxisColor;

            // NOTE:  No "break" Here, Fall Through

        case zCOLOR_HATCH_FILL:
            // Set to Emulate "Windows"-Like Patterns
            ColorPattern[ i ] = i + 1;
            break;

        case zSOLID_FILL :
        default          :
            ColorPattern[ i ] = zSOLID_FILL;
            break;
        }
    }
#endif


    brushes_created = TRUE;
}


////////////////////////// zDeleteBrushes() ////////////////////////////

void zLowLevelGraphics::zDeleteBrushes() {
    //
    // This function is used to delete any brushes created by the
    //   ActivateBrushes() function.  Typically Called When We're Done
    //   Making a Graph During Cleanup...
    //

#ifdef WINDOWS_PLATFORM
    if (brushes_created) {
        for (INT i = 0; i < zMAX_GRAPH_COLORS; i++) {
            DeleteObject(hFillBrush[ i ]);
        }
    }
#endif

#ifdef MS_DOS_PLATFORM
    // Nothing Special Needed Here...
#endif

}

//////////////////////////// zSaveGraphics() //////////////////////////////

void zLowLevelGraphics::zSaveGraphics() {
    //
    // Initialize Graphics as Necessary, and
    //   Save Any Old Graphics Objects That Will Need to Be Restored Later
    //

#ifdef WINDOWS_PLATFORM
    //
    // Save GDI Pen, Brush, and Font Objects
    //
    HBRUSH hBrush1 = (HBRUSH) GetStockObject(WHITE_BRUSH);  // Select a Brush
    hOldBrush      = (HBRUSH) SelectObject(hDC, hBrush1);

    HPEN hPen1   = (HPEN) GetStockObject(BLACK_PEN);        // Select a Pen
    hOldPen      = (HPEN) SelectObject(hDC, hPen1);

    HFONT hFont1 = (HFONT) GetStockObject(SYSTEM_FONT);  // Select a Font
    hOldFont     = (HFONT) SelectObject(hDC, hFont1);    // NEEDED TWICE
    hOldFont     = (HFONT) SelectObject(hDC, hOldFont);  //  TO USE ORIG FONT!
#endif

#ifdef MS_DOS_PLATFORM
    // Nothing Special Needed Here...
#endif

}

////////////////////////// zRestoreGraphics() ////////////////////////////

void zLowLevelGraphics::zRestoreGraphics() {
    //
    // Restores the Old Graphics Objects
    //

#ifdef WINDOWS_PLATFORM
    // Restore Old Pen, Brush, and Font GDI Objects
    SelectObject(hDC, hOldBrush);
    SelectObject(hDC, hOldPen);
    SelectObject(hDC, hOldFont);
#endif


#ifdef MS_DOS_PLATFORM
    // Nothing Special Needed Here...
#endif

}

/////////////////////////// zSetUpFonts() ////////////////////////////

void zLowLevelGraphics::zSetUpFonts() {
    // Sets Up Fonts

    // Set Up a Font for the Axis Titles, Axis Labels, Legend Titles

#ifdef WINDOWS_PLATFORM
    // Set the HDC
    AxisLabelFont  .zSetDC(hDC);
    AxisTitleFont  .zSetDC(hDC);
    LegendTitleFont.zSetDC(hDC);

    // Limit the Height of Fonts to 14-Point or Less
    AxisLabelFont  .zSetMaxPointSize(14);
    AxisTitleFont  .zSetMaxPointSize(14);
    LegendTitleFont.zSetMaxPointSize(14);
#endif

    // Set the Text Color for these Fonts
    AxisLabelFont  .zSetTextColor(AxisColor);
    AxisTitleFont  .zSetTextColor(AxisColor);
    LegendTitleFont.zSetTextColor(AxisColor);


    // Set the Size of these Fonts Based Upon the Relative Window Size
    zRECT RText;
    RText.ScaleViaPercentageOffsets(RWindow, 0.0, 0.0, 0.04, 0.04);

    AxisLabelFont  .zPickFont(_T("A"), RText);
    AxisTitleFont  .zPickFont(_T("A"), RText);
    LegendTitleFont.zPickFont(_T("A"), RText);
}

/////////////////////////// zSelectAxisColor() ////////////////////////////

void zLowLevelGraphics::zSelectAxisColor() {
    // Selects the Axis Color

    // Activate Pens, In Case They Haven't Been Activated Already
    zActivatePens();


#ifdef WINDOWS_PLATFORM
    SelectObject(hDC, hAxisPen);        // Select the Axis Pen
    SetTextColor(hDC, AxisColor);       // Set the Text Color
#endif

#ifdef MS_DOS_PLATFORM
    _setcolor((short)AxisColor);
#endif

    //
    // Save the Current Color
    //
    CurrentColor = AxisColor;

}

/////////////////////////// zSelectGraphColor() ////////////////////////////

void zLowLevelGraphics::zSelectGraphColor(INT index) {
    // Selects the Graph Color, Given an Index [index] in the Range
    //  [0, zMAX_GRAPH_COLORS - 1]

    // Make Sure Index Is Valid
    if (index < 0) {
        index = 0;
    } else if (index >= zMAX_GRAPH_COLORS) {
        index = index % zMAX_GRAPH_COLORS;
    }

    // Activate Pens and Brushes, In Case They Haven't Been Activated Already
    zActivatePens();
    zActivateBrushes();

    //
    // Switch In the Proper Color...
    //
#ifdef WINDOWS_PLATFORM
    // Select Line Pen
    if (pens_created) {
        SelectObject(hDC, hLinePen[index]);
    }

    // Select Brush for Fills
    if (brushes_created) {
        SelectObject(hDC, hFillBrush[index]);
    }

    // Set the Color of Text
    SetTextColor(hDC, GraphColor[index]);
#endif

#ifdef MS_DOS_PLATFORM
    _setcolor((short)GraphColor[index]);
#endif

    //
    // Save the Current Graph Color
    //
    CurrentColor = GraphColor[index];
}

/////////////////////////// zSelectGraphColor() ////////////////////////////

void zLowLevelGraphics::zSetPixel(INT xPos, INT yPos, zRGB color) {
    // Sets the Color of a Pixel at [xPos, yPos] to a Given Color [color]

    // Set the Current Color
    zSetCurrentColor(color);

#ifdef WINDOWS_PLATFORM
    SetPixel(hDC, xPos, yPos, color);
#endif

#ifdef MS_DOS_PLATFORM
    _setpixel(xPos, yPos);
#endif

}

////////////////////////// zShowStringInRect() ////////////////////////////

void zLowLevelGraphics::zShowStringInRect(
    PWCHAR      string,
    RECT    *   RString) {
    //
    // Builds a Font Of Appropriate Size for Displaying A String
    //   [string] Within a Rectangle and Displays the String
    //   Centered in [RString].

#ifdef WINDOWS_PLATFORM

    zFONT zf(hDC);

    // Set Color of the Text
    zf.zSetTextColor(AxisColor);

    // Draw the String
    zf.zPickFontAndCenterText(string, RString);

#endif


#ifdef MS_DOS_PLATFORM
    // Set Color of the Text
    zSelectAxisColor();

    // Draw Text In Rectangle
    zDOSShowTextInRect(string, *RString);
#endif

}

////////////////////////////// zAlignText() ///////////////////////////////

void zLowLevelGraphics::zAlignText(INT AlignValue) {
    // This Function Is Used to Set the Alignment of Text


    switch (AlignValue) {
    case zALIGN_RIGHT_TOP:    // Makes Text Right Justified

#ifdef WINDOWS_PLATFORM                                // Windows
        SetTextAlign(hDC, TA_RIGHT | TA_TOP);
#endif

        break;


    case zALIGN_LEFT_TOP:    // Makes Text Left Justified

#ifdef WINDOWS_PLATFORM                                // Windows
        SetTextAlign(hDC, TA_LEFT | TA_TOP);
#endif

        // [See MS_DOS_PLATFORM Code Below...]
        break;


    case zALIGN_CENTER:      // Makes Text Center Justified

#ifdef WINDOWS_PLATFORM
        SetTextAlign(hDC, TA_CENTER | TA_BASELINE);
#endif

        // [See MS_DOS_PLATFORM Code Below...]
        break;
    }

#ifdef MS_DOS_PLATFORM
    // Microsoft MS-DOS Graphics Can't Do Alignments, So We'll
    //   Have to Perform It Manually Ourselves When Drawing In the
    //   Text.  Here We Just Save the Desired Alignment Mode for
    //   Use Later
    zAlignFormat = AlignValue;
#endif
}

////////////////////////////// zTextWidth() /////////////////////////////

INT zLowLevelGraphics::zTextWidth(PWCHAR string) {
    // Returns the Horizontal Width of a Text String [string]
    //   In Pixels, Using the Current Font

#ifdef WINDOWS_PLATFORM
    SIZE dSize;
    GetTextExtentPoint32W(hDC, string,  _tcslen(string), &dSize);
    return (dSize.cx);
#endif

#ifdef MS_DOS_PLATFORM
    // Returns Length of a Graphics String Using Default Font
    return (_getgtextextent(string));
#endif

}

//////////////////////////// zTextHeight() /////////////////////////////

int zLowLevelGraphics::zTextHeight(PWCHAR string) {
    // Returns Height of a Graphics String Using the Current Font

#ifdef WINDOWS_PLATFORM
    SIZE dSize;
    GetTextExtentPoint32W(hDC, string, _tcslen(string), &dSize);
    return (dSize.cy);
#endif

#ifdef MS_DOS_PLATFORM
    _fontinfo fi;
    _getfontinfo(&fi);

    return (fi.pixheight);
#endif

}

//////////////////////////// zDraw_Symbol() /////////////////////////////

void zLowLevelGraphics::zDraw_Symbol(unsigned _x_, unsigned _y_, INT symbol,
                                     INT ColorIndex) {
    //
    // Displays or Prints a symbol [symbol] at (_x_, _y_) on the DC.
    //   Using Graph Color [ColorIndex]


    // Make Sure Index Is Valid
    if (ColorIndex < 0) {
        ColorIndex = 0;
    } else if (ColorIndex >= zMAX_GRAPH_COLORS) {
        ColorIndex = ColorIndex % zMAX_GRAPH_COLORS;
    }


    //
    // NOTE:    Several Custom Symbols are Currently Available:
    //          Diamond-Shape, Triangle, Upside-Down Triangle, Small Box.
    //          Cross, and Circle.  Users May Want to Edit/Extend
    //          the Symbols Here.  Have at it!
    //

#ifdef WINDOWS_PLATFORM
    SetPolyFillMode(hDC, ALTERNATE);
#endif


    switch (symbol) {                    // List Custom-Drawn Symbols Here
    case zCROSS:
        //
        // A Cross
        //
    {
        POINT Cross[] = {
            {_x_ -     charsize/4,  _y_                    },
            {_x_ +     charsize/4,  _y_                    },
            {_x_ +     charsize/4,  _y_ +     charsize/3   },
            {_x_ + 3 * charsize/5,  _y_ +     charsize/3   },
            {_x_ + 3 * charsize/5,  _y_ + 2 * charsize/3   },
            {_x_ +     charsize/4,  _y_ + 2 * charsize/3   },
            {_x_ +     charsize/4,  _y_ +     charsize     },
            {_x_ -     charsize/4,  _y_ +     charsize     },
            {_x_ -     charsize/4,  _y_ + 2 * charsize/3   },
            {_x_ - 3 * charsize/5,  _y_ + 2 * charsize/3   },
            {_x_ - 3 * charsize/5,  _y_ +     charsize/3   },
            {_x_ -     charsize/4,  _y_ +     charsize/3   },
        };

        zDrawPolygon(Cross, 12, (zPATTERN)0,
#ifdef WINDOWS_PLATFORM
                     hFillBrush[ ColorIndex ],
                     hAxisPen
#else
                     GraphColor[ ColorIndex ],
                     AxisColor
#endif
                    );

    }
    break;


    case zCIRCLE:
        //
        // A Circle (Actually a Shape Like a "Stop-Sign")
        //
    {
        POINT Circle[] = {
            {_x_ -     charsize/4,  _y_                  },
            {_x_ +     charsize/4,  _y_                  },
            {_x_ + 3 * charsize/5,  _y_ + charsize/3     },
            {_x_ + 3 * charsize/5,  _y_ + 2 * charsize/3 },
            {_x_ +     charsize/4,  _y_ + charsize       },
            {_x_ -     charsize/4,  _y_ + charsize       },
            {_x_ - 3 * charsize/5,  _y_ + 2 * charsize/3 },
            {_x_ - 3 * charsize/5,  _y_ + charsize/3     },
        };

        zDrawPolygon(Circle, 8, (zPATTERN)0,
#ifdef WINDOWS_PLATFORM
                     hFillBrush[ ColorIndex ],
                     hAxisPen
#else
                     GraphColor[ ColorIndex ],
                     AxisColor
#endif
                    );


    }
    break;


    case zDIAMOND:
        //
        // A Diamond
        //
    {
        POINT Diamond[] = {
            {_x_                         , _y_                          },
            {_x_ - zRound(0.6 * charsize), _y_ + zRound(0.6 * charsize) },
            {_x_                         , _y_ + zRound(1.2 * charsize) },
            {_x_ + zRound(0.6 * charsize), _y_ + zRound(0.6 * charsize) }
        };

        zDrawPolygon(Diamond, 4, (zPATTERN)0,
#ifdef WINDOWS_PLATFORM
                     hFillBrush[ ColorIndex ],
                     hAxisPen
#else
                     GraphColor[ ColorIndex ],
                     AxisColor
#endif
                    );


    }
    break ;


    case zTRIANGLE:
        //
        // A Small Triangle
        //
    {
        POINT Triangle[] = {
            {_x_             ,  _y_            },
            {_x_ - charsize/2,  _y_ + charsize },
            {_x_ + charsize/2,  _y_ + charsize }
        };

        zDrawPolygon(Triangle, 3, (zPATTERN)0,
#ifdef WINDOWS_PLATFORM
                     hFillBrush[ ColorIndex ],
                     hAxisPen
#else
                     GraphColor[ ColorIndex ],
                     AxisColor
#endif
                    );


    }
    break ;


    case zINVERTED_TRIANGLE:
        //
        // Small Upside-Down Triangle
        //
    {
        POINT InvertedTriangle[] = {
            {_x_ - charsize/2, _y_             },
            {_x_ + charsize/2, _y_             },
            {_x_             , _y_ + charsize  }
        };

        zDrawPolygon(InvertedTriangle, 3, (zPATTERN)0,
#ifdef WINDOWS_PLATFORM
                     hFillBrush[ ColorIndex ],
                     hAxisPen
#else
                     GraphColor[ ColorIndex ],
                     AxisColor
#endif
                    );

    }
    break ;

    // ## created for SA
    case zHUGEDOT:
        //
        // Dot
        //
    {
        MoveTo(hDC, _x_ - 2, _y_ - 2);
        LineTo(hDC, _x_ + 2, _y_ - 2);
        LineTo(hDC, _x_ + 2, _y_ + 2);
        LineTo(hDC, _x_ - 2, _y_ + 2);
        LineTo(hDC, _x_ - 2, _y_ - 2);
        MoveTo(hDC, _x_ - 1, _y_ - 1);
        LineTo(hDC, _x_ + 2, _y_ - 1);
        MoveTo(hDC, _x_ - 1, _y_);
        LineTo(hDC, _x_ + 2, _y_);
        MoveTo(hDC, _x_ - 1, _y_ + 1);
        LineTo(hDC, _x_ + 2, _y_ + 1);
    }
    break ;

    // ## created for SA
    case zBIGDOT:
        //
        // Big Dot
        //
    {
        MoveTo(hDC, _x_ - 1, _y_ - 1);
        LineTo(hDC, _x_ + 2, _y_ - 1);
        MoveTo(hDC, _x_ - 1, _y_);
        LineTo(hDC, _x_ + 2, _y_);
        MoveTo(hDC, _x_ - 1, _y_ + 1);
        LineTo(hDC, _x_ + 2, _y_ + 1);
    }
    break ;

    // ## created for SA
    case zDOT:
        //
        // Dot
        //
    {
        MoveTo(hDC, _x_, _y_);
        LineTo(hDC, _x_ + 1, _y_);
    }
    break ;

    case zBOX:
        //
        // Small Box
        //
    {
        POINT Box[] = {
            {_x_ - charsize/2 + 1, _y_ + 1            },
            {_x_ + charsize/2 - 1, _y_ + 1            },
            {_x_ + charsize/2 - 1, _y_ + charsize - 1 },
            {_x_ - charsize/2 + 1, _y_ + charsize - 1 }
        };

        zDrawPolygon(Box, 4, (zPATTERN)0,
#ifdef WINDOWS_PLATFORM
                     hFillBrush[ ColorIndex ],
                     hAxisPen
#else
                     GraphColor[ ColorIndex ],
                     AxisColor
#endif
                    );


    }
    break ;


    default: {
#ifdef WINDOWS_PLATFORM
        // Switch to Font for Drawing Symbols & Save Old Font
        HFONT save = (HFONT) SelectObject(hDC, GetStockObject(DEVICE_DEFAULT_FONT));
#endif

        //
        // Draw In This Character
        //
        WCHAR buf[ 2 ] =
        { (WCHAR)symbol, '\0' } ;

        INT yOffset = - charsize;

        zDrawTextString(_x_ - charsize/2, _y_ + yOffset, buf);

#ifdef WINDOWS_PLATFORM
        // Switch Back to Old Font
        SelectObject(hDC, save);
#endif
    }
    break ;
    }
}


/////////////////////////////// zSetRect() /////////////////////////////////

void zLowLevelGraphics::zSetRect(RECT * R1, INT x1, INT y1, INT x2, INT y2) {
    //
    // This Function Is Used To Set a Rectangle From Its Components
    //

#ifdef WINDOWS_PLATFORM
    SetRect(R1, x1, y1, x2, y2);
#else
    R1->left   = x1;
    R1->top    = y1;
    R1->bottom = x2;
    R1->right  = y2;
#endif

}

//////////////////////////////// zRound() /////////////////////////////////

INT zLowLevelGraphics::zRound(double d) {
    //
    // This Function Is Used For Rounding Purposes.  It Returns the closest
    // integer to [d], a double variable
    //
    double LowerLimit = floor(d);

    if ((d - LowerLimit) >= 0.5) {
        return ((INT)LowerLimit + 1);
    } else {
        return ((INT)LowerLimit);
    }
}

/////////////////////////////////////////////////////////////////////////
//                  MEMBER-FUNCTIONS FOR MS-DOS ONLY
/////////////////////////////////////////////////////////////////////////
#if defined(MS_DOS_PLATFORM)

//////////////////////////// zDOSOpenGraphics() ////////////////////////////

void zLowLevelGraphics::zDOSOpenGraphics() {
    // Initializes the Graphics Subsystem (Needed for MS-DOS Only)

#ifdef MS_DOS_PLATFORM

    // Set the Video Mode (Use VGA 640 x 480, 16 Color)
    if (!_setvideomode(/* _MAXRESMODE */ _VRES16COLOR)) {
        exit(1);
    }

    // Load the Helvetica Font Set.  User Must Have "HELVB.FON" File!!!
    WCHAR fondir[_MAX_PATH];
    if ((NumFonts = _registerfonts("HELVB.FON")) <= 0) {
        _outtext("Enter full path where HELVB.FON files are located: ");
        gets(fondir);
        strcat(fondir, "\\*.FON");
        if (_registerfonts(fondir) <= 0) {
            _outtext("Error: can't register fonts");
            exit(1);
        }
    }

    // Select Default Font
    zDOSSelectFont(zDEFAULT_FONT);


    // Set X- and Y- Values for Screen Rectangle
    struct _videoconfig vc;
    _getvideoconfig(&vc);

    RScreen.left   = RScreen.top = 0;
    RScreen.right  = (ScreenWidth  = vc.numxpixels) - 1;
    RScreen.bottom = (ScreenHeight = vc.numypixels) - 1;

    zDOSClearScreen();
#endif

}

//////////////////////////// zDOSCloseGraphics() /////////////////////////////

void zLowLevelGraphics::zDOSCloseGraphics() {
    // Shuts Down the Graphics Subsystem (Needed for MS-DOS Only)

#ifdef MS_DOS_PLATFORM
    _setvideomode(_DEFAULTMODE);
#endif

}

//////////////////////////// zDOSClearScreen() ///////////////////////////////

void zLowLevelGraphics::zDOSClearScreen() {
    // Clears the Screen (MS-DOS Only Function)

#ifdef MS_DOS_PLATFORM
    _clearscreen(_GCLEARSCREEN);
#endif

}

//////////////////////////// zDOSRectangle() ///////////////////////////////

void zLowLevelGraphics::zDOSRectangle(RECT * R, INT FgColor, INT BgColor) {
    // Draws a Filled, Outlined Rectangle (MS-DOS Only Function)
    //   Using Specified Foreground [FgColor] and Background Color [BgColor]

#ifdef MS_DOS_PLATFORM
    _setcolor(BgColor);
    _settextcolor(FgColor);
    _rectangle(_GFILLINTERIOR, R->left, R->top, R->right, R->bottom);
#endif

}

//////////////////////////// zDOSSelectFont() ///////////////////////////////

void zLowLevelGraphics::zDOSSelectFont(int FontNum) {
    // Selects a Particular Font (MS-DOS Only Function)

#ifdef MS_DOS_PLATFORM
    WCHAR Buf[4];
    sprintf(Buf, "n%d", FontNum);

    if (_setfont(Buf) < 0) {
        _outtext("Error: Can't Set font");
        exit(1);
    }
#endif

}

/////////////////////////// zDOSShowTextInRect() //////////////////////////

void zLowLevelGraphics::zDOSShowTextInRect(PWCHAR string, RECT R1,
        BOOL ShowText /* = TRUE */) {
    // Displays String [string] Inside Rectangle [R1]
    //  (MS-DOS Only Function)

    int RWidth  = abs(R1.left - R1.right);

    // Iterate thru the Available Fonts, and Pick
    //   the One that Will Most Closely Fit Our Text
    //   String Inside the Rectangle

    int SmallestDiff = 1000;
    int BestFontIndex = 0;
    for (int i = 1; i <= NumFonts; i++) {
        // Pick this Next Font, and Figure the Width of the String.
        zDOSSelectFont(i);
        int widthDifference = abs(zDOSTextWidth(string) - RWidth);

        // Found A Font That's a Better Fit?
        if (widthDifference < SmallestDiff) {
            SmallestDiff = widthDifference;
            BestFontIndex = i;
        }
    }

    // Pick the Closest Matching Font
    zDOSSelectFont(BestFontIndex);

    // Set the Position (Centered In the Rectangle)
    int xPos = R1.left + RWidth/2 - zDOSTextWidth(string)/2;
    int yPos = R1.top;

    // Show the String...
#ifdef MS_DOS_PLATFORM
    _moveto(xPos, yPos);

    if (ShowText) {
        _outgtext(string);
    }
#endif

    // Select Default Font Again
    zDOSSelectFont(zDEFAULT_FONT);
}

/////////////////////////// zDOSShowMessage() ////////////////////////////////

void zLowLevelGraphics::zDOSShowMessage(PWCHAR Message) {
    // Displays a Message Onscreen (MS-DOS Only Function)

    int MARGIN = 30;
    int Width = zDOSTextWidth(Message);

    // Show a Rectangle
    int XCenter = ScreenWidth  / 2;
    int YCenter = ScreenHeight / 2;
    RECT RMessage = {
        XCenter - Width/2 - MARGIN,
        YCenter - MARGIN,
        XCenter + Width/2 + MARGIN,
        YCenter + MARGIN
    };

    // Save Screen Behind Message
    DOSScreenSave R1(&RMessage);

    // Draw the Rectangle and Put a Border Around It
    zDOSRectangle(&RMessage, zBRT_WHITE, zBRT_BLUE);
    zDOSBevel(&RMessage);

    // Display the Message
#ifdef MS_DOS_PLATFORM
    _setcolor(zBRT_WHITE);
#endif

    int xPos = XCenter - Width/2 ;
    int yPos = YCenter - MARGIN/4;


    // Move Into Position and Draw This Line of Text
#ifdef MS_DOS_PLATFORM
    _moveto(xPos, yPos);
    _outgtext(Message);
#endif

    // Wait for a Key
    if (getch() == 0) {
        getch();
    }
}

///////////////////////////// zDOSSelectFill() ///////////////////////////////

void zLowLevelGraphics::zDOSSelectFill(int fill_style, int FillIndex) {
    // Selects a Given Fill Pattern [FillIndex] From Amongst
    //  the Available Fills for Style [fill_style] (MS-DOS Only Function)

#ifdef MS_DOS_PLATFORM
    static unsigned char mask[8][8] = {
        { 0x00, 0x00, 0x3C, 0x24, 0x24, 0x3C, 0x00,  0x00 },  // Box
        { 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC,  0xCC },  // Vertical Lines
        { 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF,  0x00 },  // Horizontal Lines
        { 0x33, 0xCC, 0x33, 0xCC, 0x33, 0xCC, 0x33,  0xCC },
        { 0x09, 0x12, 0x24, 0x49, 0x92, 0x24, 0x48,  0x90 },  // Diag Cross Hatch 1
        { 0x90, 0x48, 0x24, 0x92, 0x49, 0x24, 0x12,  0x09 },  // Diag Cross Hatch 2
        { 0x00, 0x66, 0x66, 0x00, 0x00, 0x66, 0x66,  0x00 },
        { 0x18, 0x24, 0x42, 0x81, 0x81, 0x42, 0x24,  0x18 }   // Diamond
    };

    // For Solid Fill, We Don't Need to do Anything Special...
    if (fill_style == zSOLID_FILL) {
        return;
    }

    // For Hatched Fills -- set Fill Mask
    FillIndex = FillIndex % 8;
    _setfillmask(mask[ FillIndex ]);
#endif

}

/////////////////////////////// zDOSBevel() ///////////////////////////////

void zLowLevelGraphics::zDOSBevel(RECT * bevelR) {
    // Puts a Bevel Around a Rectangle (MS-DOS Only Function)

    // Set Bevel Dimensions, If Necessary
    INT bevel = zBEVEL_OFFSET;

    // Calculate Perimeter Rects

    RECT topR = {
        bevelR->left,
        bevelR->top - bevel,
        bevelR->right,
        bevelR->top - 1
    };

    RECT botR = {
        bevelR->left,
        bevelR->bottom + 1,
        bevelR->right,
        bevelR->bottom + bevel
    };

    RECT leftR  = {
        bevelR->left - bevel,
        bevelR->top  - bevel,
        bevelR->left -1,
        bevelR->bottom + bevel
    };

    RECT rtR  = {
        bevelR->right + 1,
        bevelR->top    - bevel,
        bevelR->right  + bevel,
        bevelR->bottom + bevel
    };

    zDOSRectangle(&leftR, zBRT_BLACK, zBRT_BLACK);
    zDOSRectangle(&topR,  zBRT_BLACK, zBRT_BLACK);
    zDOSRectangle(&rtR,   zBRT_BLACK, zBRT_BLACK);
    zDOSRectangle(&botR,  zBRT_BLACK, zBRT_BLACK);
}

/////////////////////////////// zDOSDrawChar() ////////////////////////////

void zLowLevelGraphics::zDOSDrawChar(INT xPos, INT yPos,
                                     INT TextColor, INT BgColor, char c) {
    // Draws In One Character at [xPos, yPos] Using Specified Colors

    UNUSED_PARAMETERS(&BgColor);
    char string[] =
    { c, '\0' };

    // Draw In a 1-Byte String...

#ifdef MS_DOS_PLATFORM
    _setcolor(TextColor);
    _moveto(xPos, yPos);
    _outgtext(string);
#endif

}

//////////////////////////// zDOSTextHeight() /////////////////////////////

int zLowLevelGraphics::zDOSTextHeight(PWCHAR string) {
    // Returns Height of a Graphics String Using the Current Font
    // (MS-DOS Only Function)

#ifdef MS_DOS_PLATFORM
    _fontinfo fi;
    _getfontinfo(&fi);

    return (fi.pixheight);
#endif

}

////////////////////////////// zDOSTextWidth() /////////////////////////////

INT zLowLevelGraphics::zDOSTextWidth(PWCHAR string) {
    // Returns the Horizontal Width of a Text String [string]
    //   In Pixels, Using the Current Font (MS-DOS Only Function)


#ifdef MS_DOS_PLATFORM
    // Returns Length of a Graphics String Using Default Font
    return (_getgtextextent(string));
#endif

}

// Short Helper Functions
void zDOSShowTextInRect(PWCHAR string, zRECT RString, BOOL ShowText) {
    // Fits String [string] in [RString]

    zLowLevelGraphics::zDOSShowTextInRect(string, RString, ShowText);
}

void zDOSSelectDefaultFont() {
    // Selects the Default Font

    zLowLevelGraphics::zDOSSelectFont(zDEFAULT_FONT);
}

#endif
