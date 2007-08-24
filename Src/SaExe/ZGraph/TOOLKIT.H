// TOOLKIT.H

/////////////////////////////////////////////////////////////////////////////
//             Header File for ZGRAF Toolkit Graph Functions
//
// ZGRAF C++ Multi-Platform Graph Toolkit v. 1.41, (c) 1996, ZGRAF Software
/////////////////////////////////////////////////////////////////////////////

#ifndef _TOOLKIT_H
#define _TOOLKIT_H


/////////////////////////////////////////////////////////////////////////////
//  Defines
/////////////////////////////////////////////////////////////////////////////

//
// Application-Dependent Defines
//
#if ! defined(WINDOWS_PLATFORM)  &&  ! defined(OS2_PLATFORM)  && ! defined(MS_DOS_PLATFORM)  && ! defined(BGI_DOS_PLATFORM)
#error Please define one of WINDOWS_PLATFORM, OS2_PLATFORM, MS_DOS_PLATFORM, or BGI_DOS_PLATFORM.
#endif

#ifdef zDLL                         // If Building a Win 16 DLL
#define zGRAPH_TYPE   _export
#else
#define zGRAPH_TYPE              // If Building Win 16, Win 32, OS/2 App.
#endif

//

//
// General Defines
//
#define ON                          1
#define OFF                         0
#ifndef PI
#define PI                  3.14159265358979323846264338327950288419716939937511
#endif
#define TWOPI             (2*PI)
#define zDELTA              0.0000001
#define zY_ADJUST                   1   // Set to "-1" If Coord. System
// Has Y-Values Opposite to Windows
// (i.e., Decreasing Downward)
//
// Maximum Limits
//
#define zMAX_DATA_SETS             10   // Maximum Number of Individual
//   Data Sets Appearing in Graphs
#define zMAX_ITEM_TITLES           30   // Maximum Number of Item Titles
//   to Appear in a Graph
#define zMAX_DATA_POINTS         1024   // Maximum Number of Data Points
//   In A Single Graph
#define zMAX_GRAPH_COLORS          10   // Maximum Number of Graph Colors
//  (Same as zMAX_DATA_SETS)

//
// Misc. Other Defines for Toolkit
//
#define zDEFAULT                    0   // Value Used to Zero "zGraphStruct"

#define zDEFAULT_AXIS_DIVISIONS     5
#define zDEFAULT_X_AXIS_DIVISIONS   5   // Default Num. of Axis Divisions
#define zDEFAULT_Y_AXIS_DIVISIONS   5   //   to Use for the X- and Y- Axes

#define zDEFAULT_FLOAT_PRECISION    2   // Default Num. of Signif. Digits to
//   Display in Numbers Along Axes
#define zDEFAULT_CONTOUR_LEVELS     8   // Default Number of Contour
//   Graph Levels
// Set the Number of X- or Y-Axis
#define zBESTFIT_DIVISIONS         -1   //   Divisions to This Value to
//   Auto-Detect an "Optimum" Number
//   of Axis Divisions

#define zTRUE_TYPE_FONT_NAME   "Arial"  // We Use this True-Type Font To Show
//   Legends and Titles on Graphs


#define zGRAPH_LEGEND_X_OFFSET          (0.05  * RWindow.Width())
#define zGRAPH_LEGEND_X_OFFSET2         (0.25  * RWindow.Width())
#define zGRAPH_LEGEND_WIDTH             (0.20  * RWindow.Width())

#define zGRAPH_LEGEND_Y_OFFSET          (0.20  * RWindow.Height())
#define zGRAPH_LEGEND_Y_OFFSET2         (0.80  * RWindow.Height())
#define zGRAPH_LEGEND_HEIGHT            (0.60  * RWindow.Height())
#define zGRAPH_LEGEND_Y_OFFSET3         (0.825 * RWindow.Height())
#define zGRAPH_LEGEND_Y_OFFSET4         (0.975 * RWindow.Height())

#define zMAIN_GRAPH_AREA_X_OFFSET       (0.30  * RWindow.Width())

#ifdef originalSizeZGraph

#define zX_AXIS_OFFSET_LEFT             (0.35  * RWindow.Width())
#define zX_AXIS_OFFSET_LEFT2            (0.15  * RWindow.Width())
#define zX_AXIS_OFFSET_RIGHT            (0.85  * RWindow.Width())
#define zY_AXIS_OFFSET_TOP              (0.15  * RWindow.Height())
#define zY_AXIS_OFFSET_BOTTOM           (0.85  * RWindow.Height())
#define zY_AXIS_OFFSET_BOTTOM2          (0.70  * RWindow.Height())

#else

#define zX_AXIS_OFFSET_LEFT             (0.  * RWindow.Width())
#define zX_AXIS_OFFSET_LEFT2            (0.  * RWindow.Width())
#define zX_AXIS_OFFSET_RIGHT            (1.  * RWindow.Width())
#define zY_AXIS_OFFSET_TOP              (0.  * RWindow.Height())
#define zY_AXIS_OFFSET_BOTTOM           (1.  * RWindow.Height())
#define zY_AXIS_OFFSET_BOTTOM2          (1.  * RWindow.Height())

#endif

#define z3D_GRID_X_SLANT_OFFSET         (0.20  * RWindow.Width())
#define z3D_GRID_Y_SLANT_OFFSET         (0.25  * RWindow.Height())

#define zID_BOX_HT                      (0.05  * RWindow.Height())
#define zID_BOX_WIDTH                   (0.07  * RWindow.Width())
#define zWIDTH_SPACE_RATIO               0.80       // Used in Bargraph

#define zNUM_GRADED_PALETTE_ENTRIES     64  // Use This Many Entries In
//   Graded Color Palettes


//
// Strings, Error Messages, etc.
//
#define zAT_LEAST_2_SAMPLES_REQD _T("ERROR--Must Have >= 2 Samples in Data!")
#define zCANNOT_CREATE_PALETTE   _T("Unable to Create Graded Color Palette!")
#define zCANNOT_SCALE_X          _T("Cannot Scale Data:  xmax = xmin")
#define zCANT_GET_WIN_INI_DRIVER _T("Cannot Get Printer Driver Info. in WIN.INI")
#define zDIRECTING_GDI_CALLS     _T("Directing GDI Calls...")
#define zFUNCTION_SYNTAX_ERROR   _T("Syntax Error in F(X,Y)...")
#define zGENERAL_ERROR_MSG       _T("Error")
#define zHDC_INVALID             _T("Error--HDC Invalid or Not Set!")
#define zHPS_INVALID             _T("Error--HPS Invalid or Not Set!")
#define zILLEGAL_Y_COORDINATE    _T("Illegal y-coord. (y[i] <= 0.0) in data")
#define zILLEGAL_X_COORDINATE    _T("Illegal x-coord. (x[i] <= 0.0) in data")
#define zLOW_MEMORY_XYZ_GRAPH    _T("Insufficient Memory--XYZ Graph!")
#define zLOW_MEMORY_SCALE_XY     _T("Insufficient Memory--zGraph::zScaleData()")
#define zLOW_MEMORY_SCALE_Y      _T("Insufficient Memory--zGraph::zScaleYData()!")
#define zLOW_MEMORY_SMITH_GRAPH  _T("Insufficient Memory--Smith Graph")
#define zLOW_MEMORY_PALETTE      _T("Insufficient Memory--Cannot Create Color Palette")
#define zLOW_MEMORY_PIE_GRAPH    _T("Insufficient Memory--Piegraph Polygon Array")
#define zLOW_MEMORY_POLAR_GRAPH  _T("Insufficient Memory--Polar Graph")
#define zLOW_MEM_CONTOUR_GRAPH   _T("Insufficient Memory--Contour Graph")
#define zMAX_POLYGONS_SET_TOO_LOW _T("Contour Graph Error: [zMAX_POLYGONS] Value Set Too Low!")
#define zMISSING_NUM_ROWS_OR_COLS _T("Did Not Specify Number of Rows or Columns In Data!")
#define zNO_LOG_SCALE_DEFINED    _T("You Must Specify [zLOG] For At Least One Axis")
#define zNOT_ENOUGH_ITEM_TITLES  _T("Not Enough Item Titles [Too Many Axis Divisions?]")
#define zPREPARING_FOR_PRINT_MSG _T("Preparing for Print...")
#define zPRINT_DLG_RESOURCE_NAME _T("Printing")
#define zSPOOLING                _T("Spooling...")
#define zSTACK_ERROR             _T("\nStack Error...")
#define zTOO_MANY_CONTOUR_LEVELS _T("You Requested Too Many Contour Levels!")
#define zTOO_MANY_DATA_SETS      _T("Too Many Data Sets!")
#define zILLEGAL_NUM_PIE_SECTORS _T("Too Many/Too Few Pie Sectors!")
#define zWINDOW_RECT_INVALID     _T("Error--Window Rectangle [RWindow] Invalid/Not Set")
#define zZGRAF_GRAPH_PRINT       _T("ZGRAF Graph Print")


/////////////////////////////////////////////////////////////////////////////
//  Macros
/////////////////////////////////////////////////////////////////////////////

#ifndef IBM_COMPILER    // IBM CSet++ Compiler Already Has These Defined!
#define min(X, Y)  (((X) < (Y)) ? (X) : (Y))
#define max(X, Y)  (((X) > (Y)) ? (X) : (Y))
#endif

#define SWAP(X, Y) ((X)^=(Y)^=(X)^=(Y))


/////////////////////////////////////////////////////////////////////////////
//  Enums
/////////////////////////////////////////////////////////////////////////////

enum _Output_Devices_                   // Output Devices to Which GDI Calls
{                                       //   Are to Be Directed
  _zSCREEN_  = 0,                     //   (Screen = Default)
    _zPRINTER_ = 1
};

enum _The_Basic_Colors_
{
  zDARK_BLACK  ,  //  0               // The Basic 16 VGA Colors
    zDARK_BLUE  ,   //  1
    zDARK_GREEN ,   //  2
    zDARK_CYAN  ,   //  3
    zDARK_RED   ,   //  4
    zDARK_MAGENTA,  //  5
    zDARK_YELLOW,   //  6
    zDARK_WHITE ,   //  7
    zBRT_BLACK  ,   //  8
    zBRT_BLUE   ,   //  9
    zBRT_GREEN  ,   //  10
    zBRT_CYAN   ,   //  11
    zBRT_RED    ,   //  12
    zBRT_MAGENTA,   //  13
    zBRT_YELLOW ,   //  14
    zBRT_WHITE      //  15
};

enum _zText_Alignment_Options_         // Used to Justify Position of Text
{
  zALIGN_RIGHT_TOP = 0,
    zALIGN_LEFT_TOP  = 1,
    zALIGN_CENTER    = 2
};

enum _zSmith_Graph_Data_Options_
{
  zIMPEDANCE        = -1,              // Plot Point as an Impedance
    zREFLECTION_COEFF = 0                // Plot Point as a Reflect. Coeff.
};

enum _zGraph_Axis_Styles_
{                                       // Graph Axis Types
  zLINEAR = 0,
    zLOG    = 1,
    zPOLAR  = 2,
    z3D     = 3,
    zRECTANGULAR  = 4
};

enum _zGrid_Style_Options_              // Graph Grid Styles
{
  z2D_GRID = 0,
    z3D_GRID = 1,
    zNO_GRID = 2
};

enum zAxisLabelDirections
{
  zHORIZONTAL,
    zVERTICAL
};

enum _Numeric_Digit_Formats_    // Formats for Numbers Along Graph Axes
{
  zROUND_INTEGER   = -1,       // Show/Round Numbers to Nearest Whole Int.
    zFLOAT_PT        =  0,       // Show Numbers Using Floating Pt (Default)
    zSCI_NOTATION    =  1,       // Show Numbers in Sci. Notation
    zFIXED           =  2        // User Specifies Number of Sig. Digits
};

enum _Fill_Pattern_Options_      // Hatch and Solid Fill Patterns
{
  zSOLID_FILL       = 0,
    zCOLOR_HATCH_FILL = 1,
    zMONO_HATCH_FILL  = 2
};

enum _Grid_Pen_Style_Options_   // Pen Styles Used for Drawing Grid
{
  zSOLID     = 0,              // Solid Lines
    zDASHED       ,              // Dashed Lines
    zDOTTED       ,              // etc.
    zDASH_DOT     ,
    zDASH_DOT_DOT
};

enum _G3D_Graph_Qual_Options_   // 3-D Surface Graph Quality Settings
{
  zG3D_QUALITY_MED  = 0 ,
    zG3D_QUALITY_LOW  = 1 ,
    zG3D_QUALITY_HI   = 2 ,
    zG3D_MULTI_COLOR  = 4 ,
    zG3D_SINGLE_COLOR = 8
};


enum _Data_Marker_Options_      // Symbols/Markers Used to Identify Data
{                               //   Points On Graphs (e.g., on X-Y Graphs)
  zCROSS             = 1    ,
    zCIRCLE            = 2    ,
    zDIAMOND           = 4    ,
    zHUGEDOT           = 0x1B , // ## created for SA
    zBIGDOT            = 0x1C , // ## created for SA
    zDOT               = 0x1D , // ## created for SA
    zTRIANGLE          = 0x1E ,
    zINVERTED_TRIANGLE = 0x1F ,
    zBOX               = 0xFE
};

enum _Graph_Legend_Options_     // Graph Legend and Positioning Options
{
  zLEFT      = 0,              // Put Legend to the Left of the Graph
    zBOTTOM    = 1,              // Put Legend at Bottom of the Graph
    zNO_LEGEND = 2               // Don't Draw a Graph Legend
} ;

enum _Graph_Item_Title_Options_ // Graph Item Title Display Options
{
  zTEXT_STRINGS   = 0,         // Put Strings (in the ItemTitles[] Array)
    zNUMBERS        = 1,         // Put Numbers Along the Axes
    zNO_ITEM_TITLES = 2
};

enum _Pie_Graph_Sector_Styles_
{
  zNORMAL        = 0,     // Normal Pie Sector
    zEXPLODED      = 1      // Exploded Pie Sector
};

enum _Bkg_Color_Gradient_Options_
{
  zNO_GRADIENT   = 0,
    zTOP_DOWN      = 1,
    zBOTTOM_UP     = 2,
    zCENTER_OUT    = 4,
    zINTO_CENTER   = 8
};

enum _Bkg_Gradient_Color_Schemes_
{
  zWHITE_GRADIENT           = 0 ,
    zBLUE_GRADIENT            = 1 ,
    zGREEN_GRADIENT           = 2 ,
    zCYAN_GRADIENT            = 3 ,
    zRED_GRADIENT             = 4 ,
    zMAGENTA_GRADIENT         = 5 ,
    zYELLOW_GRADIENT          = 6 ,
    zRED_TO_MAGENTA_GRADIENT  = 7 ,
    zMAGENTA_TO_BLUE_GRADIENT = 8 ,
    zBLUE_TO_CYAN_GRADIENT    = 9 ,
    zCYAN_TO_GREEN_GRADIENT   = 10,
    zGREEN_TO_YELLOW_GRADIENT = 11,
    zYELLOW_TO_RED_GRADIENT   = 12,
    zRED_TO_BLUE_GRADIENT     = 13,
    zBLUE_TO_GREEN_GRADIENT   = 14,
    zGREEN_TO_RED_GRADIENT    = 15
};

enum _3D_Shading_Options_      // Shading Options for 3-D Graphs
{
  zNO_SHADING         = 0,    // Don't Do Any Shading
    zLIGHT_SHADING      = 1,    // Use Light Shading
    zMEDIUM_SHADING     = 2,    // Use Medium Shading
    zHEAVY_SHADING      = 3     // Use Heavy Shading
};

enum _Print_Result_Codes_
{
  zCANNOT_PRINT =  -1,
    zUSER_ABORTED =  -2,
    zSUCCESS      =   0
};


/////////////////////////////////////////////////////////////////////////////
//  Platform-Dependent Constructs
/////////////////////////////////////////////////////////////////////////////

#ifdef WINDOWS_PLATFORM
#define  INT        int         // 16- to 32-bit O.S. Portability Desired...
typedef  COLORREF   zRGB;       // RGB Color Definition
typedef  HBRUSH     zFILL;      // Windows Fills Are Done With Brushes
typedef  HPEN       zLINE;      // Windows Lines are Drawn With Pens
typedef  INT        zPATTERN;   // Not Really Used In Windows
typedef  HPALETTE   zPALETTE;   // For a Color Palette
#endif

#ifdef OS2_PLATFORM                // Adapt OS/2 PM to Windows Equivalences
typedef  LONG       zRGB;       // RGB Color Definition
typedef  zRGB       zFILL;      // OS/2 PM Fills Are Done With RGB Colors
typedef  zRGB       zLINE;      // OS/2 PM Lines Are Done With RGB Colors
typedef  LONG       zPATTERN;   // OS/2 PM Pattern Used to Fill Polygons
typedef  HPAL       zPALETTE;   // For a Color Palette

#define  LPTSTR     char *       // Adapt to Windows Memory Equiv.
#define  lstrlen   strlen
#define  lstrcpy   strcpy

#ifndef IBM_COMPILER            // Already Defined for IBM CSet++
#define _fstrchr   strchr
#endif

typedef struct tagRECT          //
{                               // Define a RECT (Like OS/2's "RECTL")
  int left   ;                 //
  int top    ;
  int right  ;
  int bottom ;

} RECT ;

typedef struct tagPOINT         // Define a Point (like OS/2's "POINTL")
{
  int x;
  int y;

} POINT;
#endif

#ifdef MS_DOS_PLATFORM             // Microsoft DOS Graphics Platform
#define  TRUE       1
#define  FALSE      0
#define  BOOL       int
#define  BYTE       unsigned char
#define  INT        int
typedef  int        zRGB;       // RGB Color Definition
typedef  zRGB       zFILL;      // Fills Are Done With RGB Colors
typedef  zRGB       zLINE;      // Lines Are Done With RGB Colors
typedef  long       zPATTERN;   // Pattern Used to Fill Polygons
typedef  long       zPALETTE;   // For a Color Palette

#define  LPTSTR         char *   // Adapt to Windows Memory Equiv.
#define  lstrlen       strlen
#define  lstrcpy       strcpy
#define            __far

#define  zDEFAULT_FONT 5        // The Default Font (Helvetica Font Set)

typedef struct tagRECT          //
{                               // Define a RECT
  int left   ;                 //
  int top    ;
  int right  ;
  int bottom ;

} RECT ;

typedef struct tagPOINT         // Define a Point
{
  int x;
  int y;

} POINT;
#endif

#ifdef BGI_DOS_PLATFORM            // Borland BGI DOS Graphics Platform
#define  TRUE       1
#define  FALSE      0
#define  BOOL       int
#define  BYTE       unsigned char
#define  INT        int
typedef  int        zRGB;       // RGB Color Definition
typedef  zRGB       zFILL;      // Fills Are Done With RGB Colors
typedef  zRGB       zLINE;      // Lines Are Done With RGB Colors
typedef  long       zPATTERN;   // Pattern Used to Fill Polygons
typedef  long       zPALETTE;   // For a Color Palette

#define  LPTSTR          char *  // Adapt to Windows Memory Equiv.
#define  lstrlen        strlen
#define  lstrcpy        strcpy
#define             far
#define  zDEFAULT_FONT  2       // The Default Font Size (Small Font)

typedef struct tagRECT          //
{                               // Define a RECT
  int left   ;                 //
  int top    ;
  int right  ;
  int bottom ;

} RECT ;

typedef struct tagPOINT         // Define a Point
{
  int x;
  int y;

} POINT;

#endif

/////////////////////////////////////////////////////////////////////////////
//  Structures
/////////////////////////////////////////////////////////////////////////////

//
// Following is The "zGraphStruct" Definition...
//
typedef struct zGraphStruct
{

#ifdef WINDOWS_PLATFORM
  HDC    hdc              ; // Standard Windows HDC
#endif

#ifdef OS2_PLATFORM
  HWND   hwnd             ; // Standard OS/2 HWND
  HPS    hps              ; // Standard OS/2 PM HPS
#endif

  RECT   *RWindow         ; // RECT Containing Window Client-Area Dimensions
  double *xdata           ; // X-Data Array
  double *ydata           ; // Y-Data Array
  double *zdata           ; // Z-Data Array
  INT    *symbols         ; // The Array of Symbols
  INT    n                ; // The Number of Data Pts.

  zRGB   axis_color       ; // Color of Graph Axis
  zRGB   bkg_color        ; // Graph Background Color
  zRGB   *plot_color      ; // The Array of Plot Colors

  INT    fill_style       ; // Graph Area Fill Style

  PWCHAR  title            ; // Main Graph Title
  PWCHAR  H_Title          ; // Horizontal Axis Title
  PWCHAR  V_Title          ; // Vertical Axis Title

  WCHAR LegendTitles[zMAX_DATA_SETS    + 1] ; // The Legend Titles (A Group of Strings)
  WCHAR ItemTitles  [zMAX_ITEM_TITLES  + 1] ; // The Item Titles (A Group of Strings)

  INT  legend_position    ; // Graph Legend Position
  //    (zLEFT / zBOTTOM / zNO_LEGEND)
  INT  item_title_format  ; // Graph Item Title Display Format
  //    (zTEXT_STRINGS / zNUMBERS / zNO_ITEM_TITLES)

  BOOL line_connect       ; // FALSE = Don't connect lines, TRUE = Do Connect
  BOOL symbols_flag       ; // FALSE = Don't show symbols, TRUE = Do Show
  BOOL grid_flag          ; // ON = Show Grid, OFF = Hide Grid
  BOOL auto_scale         ; // ON = Automatic Scale, OFF = Manual Scale
  BOOL draw_forward       ; // TRUE = Front-to-Back, FALSE = Back-to-Front
  BOOL _3D_flag           ; // TRUE for 3-D flag, FALSE for 2-D

  INT  bkg_gradient_mode  ; // Type of Graded Bkg. Color Spectrum
  //  [zNO_GRADIENT, zTOP_DOWN, zCENTER, zBOTTOM_UP]

  INT  bkg_gradient_color_scheme;  // = zWHITE_GRADIENT, zBLUE_GRADIENT, etc.

  INT  x_axis_style       ; // = zLINEAR (the Default) or zLOG
  INT  y_axis_style       ; // = zLINEAR (the Default) or zLOG
  INT  x_axis_divisions   ; // Number of X-Axis Divisions (-1 = Auto-Determine)
  INT  y_axis_divisions   ; // Number of Y-Axis Divisions (-1 = Auto-Determine)

  INT  digit_format       ; // Style to Use for Displaying Numbers Along
  //   Graph Axes [i.e., one of:  zROUND_INTEGER,
  //   zFLOAT_PT, zSCI_NOTATION, zFIXED]
  INT  digit_precision    ; // Number of Signif. Digits to Show in Numbers

  INT  grid_pen_style     ; // Pen Style Used for Drawing Grid (zSOLID, etc.)
  INT  shading_style      ; // Shading Style (zNO_SHADING, zLIGHT_SHADING, etc.)

  double xstart, xend     ; // X-Data Ranges for Manual Scaling
  double ystart, yend     ; // Y-Data Ranges for Manual Scaling
  double zstart, zend     ; // Z-Data Ranges for Manual Scaling

  INT   quality           ; // Precision of Z = F(X,Y) Graph
  PWCHAR function          ; // This is a Function [F(X) or F(X,Y)]

  char Reserved[ 128 ]    ; // Unused Now, But Reserved for Future Use!

} zGraphStruct ;



/////////////////////////////////////////////////////////////////////////////
//  Function Prototypes
/////////////////////////////////////////////////////////////////////////////

//
// Functions in ZPRINT.CPP
//
#ifdef WINDOWS_PLATFORM
BOOL ShowPrintSetupDialog(HWND hWnd);
INT zPrintGraph(HWND hwnd,
                HINSTANCE ghInstance,
                void (FAR *GraphFunctionPtr)(zGraphStruct *),
                zGraphStruct *zG);
#endif

#ifdef OS2_PLATFORM
void  zPrintGraph(HWND hwnd,
                  void (FAR *GraphFunctionPtr)(zGraphStruct *),
                  zGraphStruct *zG);
#endif

//
// Functions in MISC.CPP
//
void    zDisplayMessage(PWCHAR title, PWCHAR message);
void    zDisplayError(PWCHAR message);
void    HOURGLASS(INT);
double  parse(LPCSTR s, INT *att, double X, double Y = 0.0);
zRGB    zGRAPH_TYPE RGB_color(INT color);
zRGB    zMakeColor(BYTE red, BYTE green, BYTE blue);
RECT    GetPercentOfRect(RECT  R,
                         double LeftPercent,
                         double TopPercent,
                         double RightPercent,
                         double BottomPercent);
void    UNUSED_PARAMETERS(void *, ...);

/////////////////////////////////////////////////////////////////////////////
//  The Graph Function Prototypes
/////////////////////////////////////////////////////////////////////////////

void zGRAPH_TYPE  AreaGraph   (zGraphStruct *zG);
void zGRAPH_TYPE  LogGraph    (zGraphStruct *zG);
void zGRAPH_TYPE  BarGraph    (zGraphStruct *zG);
void zGRAPH_TYPE  XYGraph     (zGraphStruct *zG);
void zGRAPH_TYPE  PieGraph    (zGraphStruct *zG);
void zGRAPH_TYPE  XYZGraph    (zGraphStruct *zG);
void zGRAPH_TYPE  PolarGraph  (zGraphStruct *zG);
void zGRAPH_TYPE  ScatterGraph(zGraphStruct *zG);
void zGRAPH_TYPE  SmithGraph  (zGraphStruct *zG);
void zGRAPH_TYPE  ContourGraph(zGraphStruct *zG);

#endif

// EOF -- TOOLKIT.H

