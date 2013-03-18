// POLAR.CPP
//
////////////////////////////////////////////////////////////////////////////
//
//  This Module Consists of the ZGRAF Toolkit Graph Functions
//  Used to Make the Polar Graphs
//
// ZGRAF C++ Multi-Platform Graph Toolkit v. 1.41, (c) 1996, ZGRAF Software
//
////////////////////////////////////////////////////////////////////////////
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//
// Platform-Dependent Includes...
//
#ifdef WINDOWS_PLATFORM
#include <windows.h>
#endif

#ifdef OS2_PLATFORM
#define INCL_GPI
#include <os2.h>
#endif

#ifdef MS_DOS_PLATFORM
#include <graph.h>
#endif


#include "toolkit.h"                             // Toolkit Header File
#include "zgraph.h"                              // Basic Graph Class


////////////////////////////////////////////////////////////////////////////
//                          zPolarGraph Class
////////////////////////////////////////////////////////////////////////////

class zPolarGraph : public zGraph {
protected:
    INT    radius;
    double AspectRatio;           // Aspect Ratio of the Device
    POINT  ptCenter;              // Coordinates of Center of Polar Grid
    double rad_max;               // The Maximum Radius Value in the Data

public:
    zPolarGraph(zGraphStruct * zG) : zGraph(zG)
    { }
    ~zPolarGraph()
    { }
    BOOL zCheckForLegalData();    // Checks For Legal Data
    void zDraw();                 // Draws the Polar Graph
    void zDrawGrid();             // Draws the Polar Graph Grid
    void zShowYAxisNumbers();     // Displays Numbers Along Y-Axis
    BOOL zScaleData();            // Scales Input Data
    BOOL zInitGraph();            // Stuff To Do Before Making Graph
    void zTerminateGraph();       // Stuff To Do On Graph Termination
};


BOOL zPolarGraph::zInitGraph() {
    //
    // This Function Is Used to Set Up for Making the Polar Graph.
    // Converts Rectangular Input Data to Polar Form, if Necessary,
    // Determines the Largest Radius Value, Sets Graph Radius, and
    // Gets Device Aspect Ratio.
    //

    // Call Base Class
    BOOL status = zGraph::zInitGraph();


    double rad, theta;
    rad_max = 0.0 ;

    //
    // If Input Data is in Rect. Format, Replace It With the Polar
    //    Equivalent
    //
    if (x_axis_style == zRECTANGULAR ||  y_axis_style == zRECTANGULAR) {
        for (INT i = 0; i < n; i++) {
            rad =  sqrt(Xdata[i] * Xdata[i] + Ydata[i] * Ydata[i]) ;
            if (rad > rad_max) {
                rad_max = rad;
            }
            if (Xdata[i] != 0.0) {
                theta = atan2(Ydata[i], Xdata[i]);
            } else {
                theta = 0.0;
            }

            //
            // Replace data with Polar Equivalent
            //
            Xdata[i] = rad;
            Ydata[i] = theta;
        }
    } else {
        //
        // If Input Data is Already In Polar Format...
        //
        for (INT i = 0; i < n; i++)
            if (Xdata[i] > rad_max) {
                rad_max = Xdata[i];
            }
    }

    //
    // Set the Graph Radius
    //
    radius = min(abs(Y_TOP - Y_BOTTOM), abs(X_RIGHT - X_LEFT))/2;

    //
    // Compute the X/Y Aspect Ratio for the Device
    //
#ifdef WINDOWS_PLATFORM
    AspectRatio = (double) GetDeviceCaps(hDC, LOGPIXELSX) /
                  (double) GetDeviceCaps(hDC, LOGPIXELSY) ;
#else
    AspectRatio = 1.0;
#endif

    return (status);
}

void zPolarGraph::zTerminateGraph() {
    //
    // Restores Original Data Format, if Necessary
    //
    double xtmp, ytmp;

    //
    // If Input Data Was in Rect. Format, We Now Replace Polar
    //    Data with its Rectangular Equivalent
    //
    if (x_axis_style == zRECTANGULAR  ||  y_axis_style == zRECTANGULAR) {
        for (INT i = 0; i < n; i++) {
            xtmp =  Xdata[i] * cos(Ydata[i]) ;
            ytmp =  Xdata[i] * sin(Ydata[i]) ;

            Xdata[i] = xtmp;
            Ydata[i] = ytmp;
        }
    }

    // Call Base Class Implementation
    zGraph::zTerminateGraph();
}

BOOL zPolarGraph::zCheckForLegalData() {
    //
    // Returns TRUE if User Entered Valid Data for a Given Graph,
    //  Else Returns FALSE
    //

    // Call Base Class
    zGraph::zCheckForLegalData();

    if (num_sets > zMAX_DATA_SETS) {
        zDisplayError(zTOO_MANY_DATA_SETS);
        return FALSE;
    }
    return TRUE;
}

void zPolarGraph::zShowYAxisNumbers() {
    //
    // Displays the Y-Axis Number Labels on the Polar Graph
    //

    double y_value, y_inc = rad_max / y_axis_divisions ;
    INT y_step = zRound(0.5 * (double)(Y_BOTTOM - Y_TOP) / y_axis_divisions) ;
    INT j, k;

    //
    // Use Right Justification For Y-Axis Numbers--It Looks Cleaner
    //
    zAlignText(zALIGN_RIGHT_TOP);

    //
    // Create Dynamic Work Array
    //
    PWCHAR buf1 = new WCHAR[ 100 ];

    //
    // Display the Divisions...
    //
    k = 0;
    do {
        y_value = -rad_max + (double)k * y_inc;
        zConvertValue(y_value, buf1, 100);

        j = Y_BOTTOM - y_step * k ;

        //
        // Display the Value
        //
        AxisLabelFont.zDrawTextString(X_LEFT - charsize, j - charsize, buf1);

    } while (++k <= (2 * y_axis_divisions)) ;

    //
    // Kill Dynamic Array
    //
    delete [] buf1;

    //
    // Switch Back to Normal Text Justification
    //
    zAlignText(zALIGN_LEFT_TOP);

}

void zPolarGraph::zDrawGrid() {
    //
    // Draws in the Grid On the Polar Graph In If We're Supposed to...
    //
    ptCenter = RGraphAxis.Center();

    zActivatePens();

    // Select the Axis Color
    zSelectAxisColor();

    //
    // See If We Should Draw the Grid In.  If So, Do It...
    //
    if (zIsGridVisible()) {
        //
        // Draw in Circular Segments
        //
        for (INT i = 0; i < y_axis_divisions; i++) {
            zDrawCircle(ptCenter.x, ptCenter.y,
                        zRound((double)(i + 1) / y_axis_divisions * radius),
                        AspectRatio);
        }

        //
        // Draw in the Horiz., Vert. Axes
        //
        zDrawLine(ptCenter.x, Y_TOP     , ptCenter.x, Y_BOTTOM);
        zDrawLine(X_LEFT,     ptCenter.y, X_RIGHT   , ptCenter.y);
    }
}

BOOL zPolarGraph::zScaleData() {
    //
    // Scale Data as Required to fit In Grid and to Maintain
    //  Appropriate Aspect Ratio
    //

    //
    // Dynamically Allocate Some Arrays to Hold the Transformed Data
    //
    xnew = new INT[ n + 1 ];
    if (!xnew) {
        zDisplayError(zLOW_MEMORY_POLAR_GRAPH);
        return FALSE;
    }

    ynew = new INT[ n + 1 ];
    if (!ynew) {
        zDisplayError(zLOW_MEMORY_POLAR_GRAPH);
        return FALSE;
    }

    for (INT j = 0; j < n ; j++) {
        double rprime = Xdata[j] * radius / rad_max ;
        xnew[j] = ptCenter.x + zRound(rprime * cos(Ydata[j])) ;
        ynew[j] = ptCenter.y - zRound(rprime * sin(Ydata[j]) * AspectRatio) ;
    }

    return TRUE;
}

void zPolarGraph::zDraw() {
    //
    // Draws the Polar Graph
    //

    // See If User Specified Improper Input...
    if (! zCheckForLegalData()) {
        return;
    }

    // Set Up For Making the Graph
    zInitGraph();

    // Put Graph Grid In Place
    zDrawGrid();

    // Scale the Data
    zScaleData();

    // Put Graph's Y-Axis Number Labels in Place
    zShowYAxisNumbers();

    // Graph the Data
    zDrawDataPoints();

    // Display the Legend
    zDrawLegend(FALSE);

    // Show Graph Titles
    zShowGraphTitles();

    // Clean Up
    zTerminateGraph();
}

////////////////////////////// PolarGraph() ////////////////////////////////
//  Plots [n] data points, (x[i], y[i]) or (rad[i], theta[i])
//  for 0 <= i < n.  Data is Plotted on a Polar Graph Grid
////////////////////////////////////////////////////////////////////////////

void zGRAPH_TYPE  PolarGraph(zGraphStruct * zG) {
    // Declare an Instance of the zPolarGraph Class...
    zPolarGraph zPolar(zG);

    // Draw In the Polar Graph
    zPolar.zDraw();
}

// EOF -- POLAR.CPP

