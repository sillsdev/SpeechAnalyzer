// LOWLEVEL.H

/////////////////////////////////////////////////////////////////////////////
//         Header File for ZGRAF Toolkit Low-Level Graphics Functions
//
// ZGRAF C++ Multi-Platform Graph Toolkit v. 1.41, (c) 1996, ZGRAF Software
/////////////////////////////////////////////////////////////////////////////

#ifndef _LOWLEVEL_H_
#define _LOWLEVEL_H_

#include "zrect.h"
#include "zfont.h"

////////////////////////////////////////////////////////////////////////
//      zLowLevelGraphics -- A C++ Base Class With Low-Level Graphics
//                           Data and Methods
////////////////////////////////////////////////////////////////////////

class zLowLevelGraphics
{
protected:
	//
	// Rectangular Graph Window Area
	//
	zRECT   RWindow                ;   // Rectangular Graph Window Area

	//
	// Screen Information (Needed Only for MS-DOS Platforms)
	//
	static RECT RScreen            ;   // The Screen Area Rectangle
	static INT  ScreenWidth        ;   // The Width  of the Screen In Pixels
	static INT  ScreenHeight       ;   // The Height of the Screen In Pixels
	static INT  NumFonts           ;   // The Number of Fonts We Support

	//
	// Data For Pens and Brushes
	//
	BOOL pens_created              ;   // Set TRUE After Creating Pens
	BOOL brushes_created           ;   // Set TRUE After Creating Brushes

	//
	// Data for Graph Colors
	//
	zRGB AxisColor                 ;   // The Axis Color
	zRGB BkgColor                  ;   // The Background Color
	zRGB GraphColor
		[ zMAX_GRAPH_COLORS + 1]    ;   // The Graph Colors
	zPATTERN ColorPattern
		[ zMAX_GRAPH_COLORS + 1]    ;   // The Color Patterns
	//  (Used In OS/2--Not Used in Windows!)
	INT shading_style              ;   // Shading Style (zNO_SHADING,
	//    zLIGHT_SHADING, zHEAVY_SHADING, etc.)
	zRGB CurrentColor              ;   // The Current Graph Color


	//
	// Font Data
	//
	INT   charsize                 ;   // Width of 1 Text Character, in Pixels
	INT   charheight               ;   // Height of 1 Text Character, in Pixels
	zFONT AxisLabelFont            ;   // Font For Numbers or Strings Along Graph Axes
	zFONT AxisTitleFont            ;   // Font For Horizontal or Vertical Axis Titles
	zFONT LegendTitleFont          ;   // Font For Legend Titles

	//
	// Grid Information
	//
	INT  GridPenStyle              ;   // Pen Style for Grid
	INT  fill_style                ;   //   For Solid Fill   : zSOLID_FILL
	//   For Colored Hatch: zCOLOR_HATCH_FILL
	//   For Mono Hatch   : zMONO_HATCH_FILL

	//
	// Platform-Dependent Data Entities
	//
#ifdef WINDOWS_PLATFORM            // For Win 16, Win 32 Platforms
	HDC     hDC                 ;   // The DC Used For the Graph
	HBITMAP hBitmap
		[ zMAX_GRAPH_COLORS + 1] ;   // Used When Creating Brushes
	HBRUSH  hFillBrush
		[ zMAX_GRAPH_COLORS + 1] ;   // Used for Fill Brushes
	HPEN    hOldPen             ;   // The Old Pen
	HBRUSH  hOldBrush           ;   // The Old Brush
	HFONT   hOldFont            ;   // The Old Font
	HRGN    hOldClipRgn         ;   // The Old Clipping Region

	HPEN    hLinePen
		[ zMAX_GRAPH_COLORS + 1] ;   // Used For Graph Pens
	HPEN    hAxisPen            ;   // Pen Used to Draw Graph Axes
	HPEN    hGridPen            ;   // Pen Used to Draw Graph Grid
	HRGN    hClipRgn            ;   // Graph Clipping Region
#endif


#ifdef OS2_PLATFORM                // For OS/2 PM Platforms
	HWND     hWnd               ;   // OS/2 HWND (Needed for "WinRealizePalette()" Only)
	HPS      hPS                ;   // The PS Used for the Graph
	MATRIXLF mat                ;   // Coordinate-Transform Matrix
	INT      OS2_GridLineType   ;   // Type of Lines for Grid
#endif


#ifdef MS_DOS_PLATFORM             // For Microsoft MS-DOS Graphics Platform
	INT      zAlignFormat       ;   // Alignment Pattern
	unsigned GridMask           ;   // Mask Used to Draw Grid
#endif


#ifdef BGI_DOS_PLATFORM            // For Borland BGI DOS Graphics Platform
	INT bgiGridType             ;   // Type of Lines for Grid
#endif


	//
	// Class Member Functions
	//
public:
	zLowLevelGraphics()                   ;   // Constructor
	~zLowLevelGraphics()                  ;   // Destructor

	//
	// For Drawing Text
	//
	void zAlignText(INT)                  ;   // Sets Text Alignment Format
	INT  zTextWidth(PWCHAR string)         ;   // Gets the Width  of a Text String
	INT  zTextHeight(PWCHAR string)        ;   // Gets the Height of a Text String
	void zGetDefaultTextSize()            ;   // Gets the Default Text Height, Width
	void zDraw_Symbol(
		unsigned xPos,
		unsigned yPos,
		INT  symbol,
		INT  color_index)                  ;    // Draws In a Symbol
	void zDrawTextString(
		INT x1,
		INT y1,
		PWCHAR s)                           ;    // Draws In a Text String
	void zShowStringInRect(
		PWCHAR string,
		RECT *RString)                     ;    // Draws a String In Rectangle
	void zSetUpFonts()                    ;    // Sets Up Fonts

	//
	// For Saving/Restoring Graphics Configuration
	//
	void zRestoreGraphics()               ;    // Restores Graphics System
	void zSaveGraphics()                  ;    // Saves Graphics System

	//
	// For Setting Colors
	//
	void zActivateBrushes()               ;    // Sets Up Brushes/Fills
	void zActivatePens()                  ;    // Sets Up Lines
	void zDeleteBrushes()                 ;    // Destroys Brushes/Fills
	void zDeletePens()                    ;    // Destroys Pens
	void zSetBkgColor(zRGB bkg_color)     ;    // Sets the Bkg. Color
	void zSelectGraphColor(INT index)     ;    // Selects a Graph Color By Index
	void zSelectAxisColor()               ;    // Selects the Axis Color
	zFILL zCreateShadingBrush(zRGB color) ;    // Creates a Brush for Shading

	//
	// For Drawing Rectangles
	//
	void zDrawRect(RECT *R)               ;   // Draws a Rectangle
	void zDrawFilledRect(
		INT  left,
		INT  top,
		INT  right,
		INT  bottom,
		BYTE Red,
		BYTE Green,
		BYTE Blue)                         ;   // Draws a Filled R-G-B Rectangle
	void zNormalizeRect(RECT *R)          ;   // Normalizes a Rectangle

	//
	// For Drawing Lines
	//
	void zMoveTo(INT xPos, INT yPos)      ;   // Moves Pen to a Point
	void zLineTo(INT xPos, INT yPos)      ;   // Draws a Line to a Point
	void zDrawLine(INT left, INT top,
		INT right, INT bottom)          ;   // Draws a Line Between Points


	//
	// For Drawing Other Graphics Entities
	//
	void zDraw3DBar(
		INT left,  INT top,
		INT right, INT bottom,
		INT depth, zPATTERN Pattern,
		zFILL InteriorColor,
		zLINE BoundaryColor)               ;   // Draws a 3-Dimensional Bar

	void zDrawPolygon(POINT *PtArray,
		INT        NumPoints,
		zPATTERN   Pattern,
		zFILL      InteriorColor,
		zLINE      BoundaryColor)          ;   // Draws a Filled Polygon

	void zDrawCircle(INT x1, INT y1,
		INT radius1, double AspectRatio)   ;   // Draws a Circle

	void zDrawWedge(
		POINT P1, POINT P2,
		POINT P3, POINT P4,
		POINT P5, POINT P6,
		zPATTERN  Pattern,
		zFILL     InteriorColor,
		zLINE     BoundaryColor,
		BOOL      DrawFullWedge = TRUE)    ;   // Draws a 3-D Wedge

	void zSetPixel(INT xPos,
		INT  yPos,
		zRGB color)                     ;   // Sets the Color of One Pixel

	//
	// Inline Functions
	//
	inline void zSetGridPenStyle(INT gps)        // Sets Grid Pen Style to
	{ GridPenStyle = gps; }                   //    zNORMAL, etc.
	inline void zSetBrushFillStyle(INT fs)       // Sets Brush Fill Style
	{ fill_style = fs; }
	inline zRGB zGetCurrentColor()               // Gets the Current Graph
	{ return CurrentColor; }                  //   Color
	inline void zSetCurrentColor(zRGB color1)    // Sets the Current Graph
	{ CurrentColor = color1; }                //   Color
	inline void zSetGraphWindow(RECT *R1)        // Sets the Graph Window Area
	{
		RWindow = *R1;
		RWindow.Normalize();
	}


	//
	// Static Member Functions
	//
	static INT  GetScreenHeight() { return ScreenHeight; }
	static INT  GetScreenWidth()  { return ScreenWidth;  }
	static RECT GetRScreen()      { return RScreen;      }
	static INT  GetNumFonts()     { return NumFonts;     }

	static INT   zRectHeight(RECT *R);           // Finds Height of Rect.
	static INT   zRectWidth(RECT *R);            // Finds Width of Rect.
	static POINT zRectCenter(RECT *R);           // Finds Center of Rect.
	static INT   zRound(double d);               // Rounds Off a Number
	static void  zSetRect(RECT *, INT, INT,      // Sets a Rectangle
		INT, INT);
	static void  UNUSED_PARAMETERS(              // This Is Used to Kill
		void *, ...) { }                          //   Unused Param. Warnings

	///////////////////////////////////////////////////////////////////////
	// Member Functions for Individual Platforms
	///////////////////////////////////////////////////////////////////////

	//
	// For Windows
	//
#ifdef WINDOWS_PLATFORM
	inline void zSetGraphDC(HDC hdc1)         // Sets the Graph DC
	{ hDC = hdc1;                }

	HFONT zCreateNewFont(RECT *RString,
		PWCHAR string,
		BOOL Bold,
		BOOL LimitHeight);
#endif

#ifdef OS2_PLATFORM
	inline void zSetGraphHPS(HPS hps1)
	{ hPS = hps1;   }                      // Sets the Graph HPS
	inline void zSetGraphHWND(HWND hwnd1)
	{ hWnd = hwnd1; }                      // Sets the Graph HWND
#endif

#if defined(MS_DOS_PLATFORM) || defined(BGI_DOS_PLATFORM)

	//
	// Member Functions, Methods
	//

	static void zDOSBevel(RECT *bevelR);    // Draws a Bevel Around a Rect.
	static void zDOSShowTextInRect(
		PWCHAR string,
		RECT R1,
		BOOL ShowText = TRUE)          ;    // Draws a String In Rectangle
	static void zDOSDrawString(
		INT   xPos,
		INT   yPos,
		INT   TextColor,
		INT   BgColor,
		PWCHAR string)                     ;    // Draws a String With FG, BG Colors
	static void zDOSDrawChar(INT xPos,
		INT yPos,
		INT TextColor,
		INT BgColor,
		char c)                            ; // Draws a Character In Color
	static INT  zDOSTextWidth(PWCHAR str)  ; // Gets the Width  of a Text String
	static INT  zDOSTextHeight(PWCHAR str) ; // Gets the Height of a Text String
	static void zDOSShowMessage(PWCHAR Msg); // Shows a Message Box
	static void zDOSOpenGraphics()        ; // Opens Graphics Subsystem
	static void zDOSCloseGraphics()       ; // Closes Graphics Subsystem
	static void zDOSClearScreen()         ; // Clears the Screen
	static void zDOSSelectFill(
		INT fill_style,
		INT FillIndex)                  ; // Selects a Fill
	static void zDOSSelectFont(INT FontNo); // Selects a Given Font
	static void zDOSRectangle(RECT *R,
		INT FgColor,
		INT BgColor)                    ; // Draws a Rectangle With
	//   Given FG and BG Colors
#endif

};

#endif

// EOF -- LOWLEVEL.H



