/////////////////////////////////////////////////////////////////////////////
//                   ZGRAPH.H -- Header File for zGraph Class
//
// ZGRAF C++ Multi-Platform Graph Toolkit v. 1.41, (c) 1996, ZGRAF Software
/////////////////////////////////////////////////////////////////////////////
#ifndef _ZGRAPH_H_
#define _ZGRAPH_H_

#include "lowlevel.h"              // Low-Level Graphics Class

class zGraph : public zLowLevelGraphics {

public: // ## sa needs to have access
    //
    // Data for Graph Grid
    //
    BOOL GraphOpen           ;   // Set TRUE In Constructor/Test in Dest.
    BOOL GridVisible         ;   // ON = Show Grid, OFF = Don't Show Grid
    INT GridStyle           ;   //   zNO_GRID = No Grid
    INT GridHeight          ;   // Height of Grid in Device Units
    INT GridWidth           ;   // Width of Grid in Device Units
    zRECT RGraphAxis          ;   // Rectangular Area Bounded by Graph Axes
    INT x_axis_divisions    ;   // Number of Graph X-Axis Divisions
    INT y_axis_divisions    ;   // Number of Graph Y-Axis Divisions

    INT     X_LEFT              ;   // These Are
    INT     X_RIGHT             ;   //    the Individual
    INT     Y_TOP               ;   //       Components of the
    INT     Y_BOTTOM            ;   //          RGraphAxis Rectangle


    //
    // Graph Titles
    //
    PWCHAR   HorizTitle          ;   // Graph's Horizontal Title
    WCHAR   ItemTitle
    [ zMAX_ITEM_TITLES + 1]  ;   // Graph's Item Titles
    WCHAR   LegendTitle
    [ zMAX_DATA_SETS + 1]    ;   // Graph's Legend Titles
    PWCHAR   MainTitle           ;   // Graph's Main Title
    PWCHAR   VertTitle           ;   // Graph's Vertical Title

    //
    // Data/Axis Ranges
    //
    double  xmax, xmin          ;   // Min., Max. values in X-Data Array
    double  ymax, ymin          ;   // Min., Max. values in Y-Data Array
    double  zmax, zmin          ;   // Min., Max. values in Z-Data Array

    double  xstart, ystart, zstart; // Data Ranges for Manual Scaling
    double  xend,   yend,   zend  ;

    INT     x_axis_style        ;   // X-Axis Style: zLINEAR or zLOG
    INT     y_axis_style        ;   // Y-Axis Style: zLINEAR or ZLOG

    //
    // Graph Data and Pointers to Data Arrays
    //
    INT     n                   ;   // The Number of Data Points
    double * Xdata              ;   // Pointer to X-Data Array
    double * Ydata              ;   // Pointer to Y-Data Array
    double * Zdata              ;   // Pointer to Z-Data or Other Data Array
    INT   *  sym                ;   // Pointer to Array of Symbols


    //
    // Pointers (for Dynamically Created Arrays)
    //
    INT   *  xnew            ;
    INT   *  ynew            ;
    double * YWorkData       ;

    //
    // Misc. Data
    //
    BOOL    _3D_flag            ;  // TRUE for a 3-D graph, FALSE Otherwise
    BOOL    auto_scaling        ;  // TRUE for Auto-Scaling, FALSE Otherwise
    INT     digit_format        ;  // Style in Which Axes Digits are Shown
    INT     digit_precision     ;  // Significant Digits in Axis Numbers
    INT     bkg_gradient_mode   ;  // Type of Bkg. Graded Color Spectrum
    //   = zTOP_DOWN, zBOTTOM_UP, zCENTER,
    //   zNO_GRADIENT
    INT     bkg_gradient_color_scheme;
    // Color Scheme for Bkg. Gradient
    //   = zWHITE_GRADIENT, zBLUE_GRADIENT, etc.
    BOOL    draw_forward        ;  // TRUE=Front-to-Back, FALSE=Back-to-Front
    //  (Drawing Order, 3-D Area and Bar Graphs)
    PWCHAR   function            ;  // Pointer to ASCII Function, e.g. F(X,Y)

    INT     LegendPosition      ;  // Graph Legend Position
    //   (zLEFT or zBOTTOM)
    INT     item_title_format   ;  // Item Title Display Format
    //   (zTEXT_STRINGS, zNUMBERS, zNO_ITEM_TITLES)
    BOOL    line_connect        ;  // TRUE if Connect Lines Between Data Pts
    INT     num_sets            ;  // The Number of Individual Data Sets
    INT     num_samples         ;  // The Number of Individual Data Samples
    INT     quality             ;  // Graph Quality [G(X,Y) Graph Only]
    zRECT   RLegend             ;  // Rectangular Area Used By Graph Legend

    BOOL    show_legend_percents;  // TRUE if Showing Legend Percents
    BOOL    symbols_flag        ;  // TRUE if Show Graph Symbols, else FALSE
    BOOL    show_axis_titles    ;  // TRUE if Showing X- and Y-axis Titles

    double  XCoordMultiplier    ;  // Slope For X-data Scaling Equation
    double  XCoordOffset        ;  // Intercept For X-data Scaling Equation
    double  YCoordMultiplier    ;  // Slope For Y-data Scaling Equation
    double  YCoordOffset        ;  // Intercept For Y-data Scaling Equation

    //
    // Class Member Functions
    //
public:
    zGraph(SGraph * zGS1 = NULL); // Constructor
    ~zGraph();                          // Destructor

    void zCalculateRGBValues(BYTE Intens,
                             BYTE & Red, BYTE & Green, BYTE & Blue);

    void zConvertValue(double, PWCHAR, size_t len);

    zPALETTE zCreateGradedColorPalette();

    void zDeleteArrays();
    void zFillBackground();
    void zFillBkgWithGradedColor();
    void zGetSetsAndSamples();
    void zGetLongestLegendString(PWCHAR);
    void zInitGraphData(
        SGraph * zGS1 = NULL);
    BOOL zScaleYData(
        INT slant_offset = 0);
    BOOL zScale3DGraphData();
    void zSelectDefaultGraphColors();
    void zSetAxisDivisions(INT, INT);
    void zSetData(INT n_1,
                  double * Xdata_1,
                  double * Ydata_1 = NULL,
                  double * Zdata_1 = NULL);
    void zSetDigitFormat(INT);
    void zSetDigitPrecision(INT);
    void zSetGraphAxesRect();
    void zSetGraphColors(
        zRGB axis_color,
        zRGB bkg_color,
        zRGB * graph_colors);
    void zSetGraphTitles(
        PWCHAR MainTitle1 ,
        PWCHAR HorizTitle1,
        PWCHAR VertTitle1);
    void zSetItemTitles(
        WCHAR ItemTitle1[]);
    void zSetLegendRect();
    void zSetLegendTitles(WCHAR LegendTitle1[]);
    void zSetSymbols(INT * symbols_1);
    void zSetUpGraphLayout();
    void zSizeData();
    void zzShowXAxisNumbers(INT, INT);
    void zzShowYAxisNumbers(INT, INT);
    void zzDrawGraphAxes(
        BOOL show_x_values = TRUE,
        BOOL show_y_values = TRUE);

    //
    // Inline Functions
    //
    inline RECT zGetRGraphAxis() {
        return (RECT)RGraphAxis;      // Returns Axis Rectangle
    }
    inline BOOL zIsGridVisible() {            // Returns TRUE if Graph Grid ON
        return (GridVisible);
    }
    inline void zSet3DFlag(BOOL _3D) {        // Sets 3-D Flag ON/OFF
        _3D_flag = _3D;
    }
    inline void zSetAutoScaling(BOOL auto_sc) { // Turns Auto-Scaling On/Off
        auto_scaling = auto_sc;
    }
    inline void zSetAxisStyle(INT x1, INT y1) { // Sets the Graph Axes Style
        x_axis_style = x1;                   //   set to zLOG or zLINEAR
        y_axis_style = y1;
    }         //   set to zLOG or zLINEAR
    inline void zSetFunction(PWCHAR f) {       // Assigns function [F(X,Y)]
        function = f;
    }
    inline void zSetBkgGradient(int b1) {     // Sets Color Bkg. Gradient Type
        bkg_gradient_mode = b1;
    }
    inline void zSetBkgGradientColorScheme(int b1) {
        bkg_gradient_color_scheme = b1;    // Set Bkg. Gradient Color Scheme
    }
    inline void zSetGraphQuality(INT q) {
        quality = q;                  // Sets Graph Quality
    }
    inline void zSetGridFlag(INT gf) {        // Sets Graph Grid Display
        GridVisible = gf;             //   to ON / OFF
    }
    inline void zSetGridStyle(INT gs) {       // Sets Graph Grid Type
        GridStyle = gs;               //   to NONE / 2D / 3D
    }
    inline void zSetLegendPosition(INT lp) {  // Sets Legend Pos (at Left or
        LegendPosition = lp;    //   at Bottom of Window)
    }
    inline void zSetItemTitleFormat(INT itf) { // Sets Item Title Format
        item_title_format = itf;    //   (zTEXT_STRINGS, etc.)
    }
    inline void zSetLineConnect(BOOL lflag) { // If TRUE, We Draw Lines
        line_connect = lflag;         //    Between the Data Pts.
    }
    inline void zSetShadingStyle(INT s) {
        shading_style = s;            // Sets Shading Style
    }
    inline void zSetStackingDirection(BOOL s) { // Sets Drawing (Stacking)
        draw_forward = s;             //   Direction
    }
    inline void zSetSymFlag(BOOL sym_flag) {  // If TRUE We Display the
        symbols_flag = sym_flag;      //    Symbols, Else We Don't
    }
    inline void zSetXDataRange(               // Sets X-Data Range for
        double xs1, double xe1) {              //   Manual Graph Scaling
        xstart = xs1;
        xend = xe1;
    }
    inline void zSetYDataRange(               // Sets Y-Data Range for
        double ys1, double ye1) {              //   Manual Graph Scaling
        ystart = ys1;
        yend = ye1;
    }
    inline void zSetZDataRange(               // Sets Z-Data Range for
        double zs1, double ze1) {              //   Manual Graph Scaling
        zstart = zs1;
        zend = ze1;
    }


    //
    // Virtual Functions, Can be Overridden in Derived Classes...
    //
    virtual BOOL zCheckForLegalData();         // Validates Graph Data
    virtual void zDraw() { }                   // Override to Draw Graph
    virtual void zDrawGraphAxes();             // Draws/Labels the Graph Axes
    virtual void zDrawDataPoints();            // Draws in Data Points on Graph
    virtual void zDrawGrid();                  // Draws a Grid On the Graph
    virtual void zDraw2DGrid();                // Draws a 2-D Grid
    virtual void zDraw3DGrid();                // Draws a 3-D Grid
    virtual void zDrawLegend(
        BOOL UseColorFills = TRUE);             // Draws In the Graph Legend
    virtual BOOL zInitGraph();                 // Set Up For Making the Graph
    virtual BOOL zScaleData();                 // Scales Data for the Graph
    virtual void zShowGraphTitles();           // Shows Graph Titles
    virtual void zShowXAxisNumbers();          // Draws Numbers Along the X-Axis
    virtual void zShowYAxisNumbers();          // Draws Numbers Along the Y-Axis
    virtual void zTerminateGraph();            // Call When Done With Graph

    //
    // For Determining Best-Fit Axis Divisions
    //
    void zFindBestFit(double MinVal, double maxVal,
                      int & NumTicks, double & DistBetweenTicks);
    void zCalcTicks(double & MinVal, double & MaxVal, int & AxisDiv);
    void zGetBestFitAxisDivisions();


};

#endif

// EOF -- ZGRAPH.H



