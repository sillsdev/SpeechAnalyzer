// LOG.CPP
//
/////////////////////////////////////////////////////////////////////////////
//
//   This Module Consists of the ZGRAF Toolkit Graph Functions
//      Used to Make the Log Graphs
//
// ZGRAF C++ Multi-Platform Graph Toolkit v. 1.41, (c) 1996, ZGRAF Software
//
/////////////////////////////////////////////////////////////////////////////
#include <math.h>
#include <stdio.h>
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


#include "toolkit.h"                             // Toolkit Header File
#include "zgraph.h"                              // Basic Graph Class

////////////////////////////////////////////////////////////////////////////
//                          zLogGraph Class
////////////////////////////////////////////////////////////////////////////

class zLogGraph : public zGraph
{
  protected:
    WCHAR zbuf[ 256 ];
    double  xlow                ;   // Used by Log Graph
    double  ylow                ;   // Used by Log Graph
    
  public:
    zLogGraph(zGraphStruct *zG) : zGraph(zG)
    {
    }
    ~zLogGraph()
    {
    }
    
    BOOL zCheckForLegalData();    // Checks For Legal Data (Base-Class Override)
    void zDraw();                 // Draws a Graph -- THE GUTS
    void zDrawGrid();             // Draws the Log Graph Grid
    BOOL zScaleData();            // Scales the Data
    void zShowYAxisNumbers();     // Displays Numbers Along Y-Axis (Override)
    void zShowXAxisNumbers();     // Displays Numbers Along Y-Axis (Override)
    INT  zLogger(double, double); // To Calc. Log. Divisions
};

INT zLogGraph::zLogger(double X, double Y)
{
  //
  // This Function Is Used in Conjunction with the Log Graph
  //   to Calculate Logarithmic Divisions
  //
  INT i;

  if (X > 0)
    i = zRound(log10(X) * Y) ;
  else
    i = 0;
  return (i);
}

void zLogGraph::zShowXAxisNumbers()
{
  //
  // Put Number Values in Place Along the Graph's X-Axis.  Both
  //   Logarithmic and Linear Scales are Allowed, as Controlled by the
  //   [x_axis_style] Variable.
  //
  double x_value = 0, x_inc = (xmax - xmin) / x_axis_divisions;
  INT xtmp = zRound((double)(X_RIGHT - X_LEFT) / x_axis_divisions);
  INT j = X_LEFT - charsize ;


  if (x_axis_style == zLOG)
    x_value = pow(10.0, xmin);

  for (INT i = 0; i <= x_axis_divisions; i++)
  {
    if (x_axis_style == zLINEAR)              // For Linear Scales
      x_value = xmin + (double)i * x_inc;
    else                                      // For Log Scales
      x_value *= 10.0;
    // Set Last Value to [xmax]
    if (i == x_axis_divisions  &&  x_axis_style == zLINEAR)
      zConvertValue(xmax, zbuf, _countof(zbuf));
    else
      zConvertValue(x_value, zbuf, _countof(zbuf));

    // Draw In this X-Axis Label
    AxisLabelFont.zDrawTextString(j, Y_BOTTOM + zY_ADJUST * 5,  zbuf);

    j += xtmp;
  }
}

void zLogGraph::zShowYAxisNumbers()

{
  //
  // Puts Number Values in Place along the Y-Axis.  The Labels Extend
  //   from the y-coordinate [y_bottom] to y-coordinate [y_top].  Both
  //   LOG and LINEAR Scales are allowed, as controlled by the
  //   [y_axis_style] variable.
  //
  INT y_bottom = Y_BOTTOM, y_top = Y_TOP;
  INT ystep = zRound((double)(y_bottom - y_top) / y_axis_divisions) ;
  INT j, k = 0;
  double y_value = 0, y_inc = (ymax - ymin) / y_axis_divisions ;

  //
  // We Need to Consider the Special Case Where Y-data is
  //   Single-Valued.  In this Case Only One Axis Value Label is Needed!
  //
  if (ymin == ymax)
  {
    zConvertValue(ymin, zbuf, _countof(zbuf));

    // Use Right Justification For Y-Axis Numbers--It Looks Cleaner
    AxisLabelFont.zRightAlignText(
      X_LEFT - AxisLabelFont.zGetFontWidth(),
      Y_TOP + zY_ADJUST * (y_bottom - y_top)/2,
      zbuf);

    return ;
  }

  //
  // Otherwise, For the Regular Case, Where Y-Data is Multi-Valued...
  //
  if (y_axis_style == zLOG)
    y_value = pow(10.0, ymin);


  for (j = y_bottom; j >= (y_top - ystep/2);  k++, j -= ystep)
  {
    if (y_axis_style == zLINEAR)      // For Linear Scales
      y_value = ymin + (double)k * y_inc;
    else
      y_value *= 10.0 ;              // For Log Scales

    // Set Last Value to [ymax]
    if (k >= y_axis_divisions  &&  y_axis_style == zLINEAR)
      zConvertValue(ymax, zbuf, _countof(zbuf));
    else
      zConvertValue(y_value, zbuf, _countof(zbuf));

    // Draw in this Y-axis Value Label
    AxisLabelFont.zRightAlignText(
      X_LEFT - AxisLabelFont.zGetFontWidth(),
      j,
      zbuf);
  }
}


BOOL zLogGraph::zCheckForLegalData()
{
  //
  // Returns TRUE if User Entered Valid Data for a Log Graph,
  //

  // Call Base Class
  zGraph::zCheckForLegalData();

  //
  // For This Graph, We Need to Know the Max. and Min. Values
  //   in the Data Before We Can Verify That Graph Can Be
  //   Drawn.  So Let's Size the Data Now (Normally We'd Wait
  //   Until We Do the Scaling...)
  //
  zSizeData();

  //
  // Make Sure User Specified At Least One Logarithmic Axis!
  //
  if (x_axis_style != zLOG  &&  y_axis_style != zLOG)
  {
    zDisplayError(zNO_LOG_SCALE_DEFINED);
    return (FALSE);
  }

  //
  // Check Data to See if Illegal Values Occur In Data For Logarithmic
  //  Graphs
  //
  if (x_axis_style == zLINEAR  &&  y_axis_style == zLOG)
  {
    xlow = xmin;
    if (ymin > 0.0)
      ylow = pow(10.0, floor(log10(ymin)));
    else
    {
      zDisplayError(zILLEGAL_Y_COORDINATE);
      return (FALSE);
    }
  }
  else if (y_axis_style == zLINEAR  &&  x_axis_style == zLOG)
  {
    ylow = ymin;
    if (xmin > 0.0)
      xlow = pow(10.0, floor(log10(xmin)));
    else
    {
      zDisplayError(zILLEGAL_X_COORDINATE);
      return (FALSE);
    }
  }
  else
  {
    if (xmin <= 0.0)
    {
      zDisplayError(zILLEGAL_X_COORDINATE);
      return (FALSE);
    }
    if (ymin <= 0.0)
    {
      zDisplayError(zILLEGAL_Y_COORDINATE);
      return (FALSE);
    }
    xlow = pow(10.0, floor(log10(xmin)));
    ylow = pow(10.0, floor(log10(ymin)));
  }

  return (TRUE);
}


void zLogGraph::zDrawGrid()
{
  //
  // Puts a 2-D Flat Graph Grid in Place.  Note:  It is Assumed
  //   that [x_axis_style] and [y_axis_style] are set to either
  //   [zLINEAR] for Linear Divisions or to [zLOG] for Logarithmic
  //   Divisions.
  //
  INT i, j, k;

  //
  // Return Now If We're Not Supposed to Draw the Grid
  //
  if (! zIsGridVisible())
    return;

  //
  // Draw Verical Grid Lines In
  //
  double tmp1 = (double) GridWidth / x_axis_divisions;

  for (j = 0; j < x_axis_divisions; j++)
  {
    i = X_LEFT + zRound((double)GridWidth * j / x_axis_divisions) ;

    if (x_axis_style == zLINEAR)                  // For Linear Case
      zDrawLine(i, Y_TOP, i, Y_BOTTOM);
    else                                          // For Logarithmic Case
    {
      if (j == 0)
        zDrawLine((i + zLogger(1.0, tmp1)), Y_TOP,
        (i + zLogger(1.0, tmp1)), Y_BOTTOM) ;

      for (k = 2; k <= 10; k++)
      {
        zDrawLine((i + zLogger((double)k, tmp1)), Y_TOP,
          (i + zLogger((double)k, tmp1)), Y_BOTTOM);
      }
    }
  }

  //
  // Draw Horizontal Grid Lines In
  //
  tmp1 = (double) GridHeight / y_axis_divisions;
  for (j = 0; j < y_axis_divisions; j++)
  {
    i = Y_BOTTOM - zY_ADJUST *
      zRound((double)GridHeight * j / y_axis_divisions) ;

    if (y_axis_style == zLINEAR)              // For Linear Case
      zDrawLine(X_LEFT, i, X_RIGHT, i);
    else                                      // For Logarithmic Case
    {
      INT y_Val = (i - zY_ADJUST * zLogger(1.0, tmp1));

      if (j == 0)
        zDrawLine(X_LEFT, y_Val, X_RIGHT, y_Val) ;

      for (k = 2; k <= 10; k++)
      {
        y_Val = (i - zY_ADJUST * zLogger((double)k, tmp1));

        zDrawLine(X_LEFT, y_Val, X_RIGHT, y_Val);
      }
    }
  }
}

BOOL zLogGraph::zScaleData()
{
  //
  // Scales Values of X and Y coordinates to Fit an X/Y or Log Style Graph.
  //      For Linear Scaling We Use the Linear Translation:
  //      xnew = [a] * x + [b],  ynew = [c] * y + [d], where the
  //      [a], [b], [c], and [d] Coefficients Are Determined So That:
  //
  //     (xmin, xmax) maps into the range (X_LEFT, X_RIGHT)
  //     (ymin, ymax) maps into the range (Y_BOTTOM, Y_TOP)
  //
  //     A Slightly Different Algorithm is Used for Logarithmic Scaling
  //
  //  This Function Returns TRUE Normally, or FALSE if Error.
  //

  INT j;
  double x_inc = 0, y_inc = 0;

  //
  // NOTE-- Don't Need to Determine Max/Min Values Here, Since This Was
  //   Done In "zCheckForLegalData()"
  //

  //
  // Dynamically Allocate Some Arrays to Hold the Transformed Data
  //
  xnew = new INT[ n + 1 ];
  if (!xnew)
  {
    zDisplayError(zLOW_MEMORY_SCALE_XY);
    return FALSE;
  }

  ynew = new INT[ n + 1 ];
  if (!ynew)
  {
    zDisplayError(zLOW_MEMORY_SCALE_XY);
    return FALSE;
  }

  //
  // For Logarithmic Graphs, We Force the Origin to Start At the
  //    Power-of-10 Below the Smallest One Appearing In the Data,
  //    and Compute the X- or Y- Upper Bound Based Upon the Number of
  //    Axis Divisions the User Wants in the Graph
  //
  if (y_axis_style == zLOG)
  {
    ymin = ylow / 10.0;
    ymax = ymin * pow(10.0, (double)y_axis_divisions);

    ymax = log10(ymax), ymin = log10(ymin);
    y_inc = (double)(abs(Y_BOTTOM - Y_TOP)) / y_axis_divisions;

  }
  if (x_axis_style == zLOG)
  {
    xmin = xlow / 10.0;
    xmax = xmin * pow(10.0, (double) x_axis_divisions);

    xmax = log10(xmax), xmin = log10(xmin);
    x_inc = (double)(abs(X_RIGHT - X_LEFT)) / x_axis_divisions;
  }

  //
  // Note:  We Skip the Data Scaling/Translation for the Single-Valued
  //        Y-Data Case [where ymax = ymin]
  //
  if (ymax == ymin)
  {
    for (j = 0; j < n; j++)
    {
      xnew[ j ] = X_LEFT;
      ynew[ j ] = (Y_TOP + Y_BOTTOM) / 2 ;
    }
    xnew[ n - 1 ] = X_RIGHT;
  }
  else
  {
    //
    // Otherwise, We Translate the Data
    //
    INT grid_width  = abs(X_RIGHT - X_LEFT);
    INT grid_height = abs(Y_TOP - Y_BOTTOM);

    //
    // If Single-Valued X-Coordinate Data, Tell the User...
    //
    if (xmax == xmin)
    {
      zDisplayError(zCANNOT_SCALE_X);
      return (FALSE);
    }

    double a = (double) grid_width / (xmax - xmin);
    double b = (double) X_LEFT - a * xmin;

    double c = zY_ADJUST *
      (double) grid_height / (ymin - ymax);

    double d = (double) Y_TOP - c * ymax ;

    for (j = 0; j < n; j++)
    {
      //
      // Scale X-Coordinate
      //
      if (x_axis_style == zLINEAR)
        xnew[ j ] = (INT) (a * Xdata[ j ] + b) ;
      else
        xnew[ j ] = X_LEFT + zRound(x_inc * log10(Xdata[ j ] / xlow));

      //
      // Scale Y-Coordinate
      //
      if (y_axis_style == zLINEAR)
        ynew[ j ] = (INT) (c * Ydata[ j ] + d) ;
      else
        ynew[ j ] = Y_BOTTOM - zRound(y_inc * log10(Ydata[ j ] / ylow));

    }
  }

  //
  // Determine The Optimum Number of Axis Divisions to Use
  //   if User Didn't Specify Them.  Note:  [x_axis_divisions]
  //   and/or [y_axis_divisions] will be < 0 If User Wants
  //   Program to Best Fit the Number of Axis Divisions...
  //
  zGetBestFitAxisDivisions();

  return (TRUE);
}

void zLogGraph::zDraw()
{
  //
  // Draws a Log Graph
  //

  zInitGraph();                                 // Setup For Graphing

  if (zCheckForLegalData())                     // If Valid Data Exists
  {
    zScaleData();                              // Scale the X,Y Data
    zDrawGraphAxes();                          // Draw Graph Axes
    zDrawGrid();                               // Draw the Graph Grid
    zDrawDataPoints();                         // Plot the Data Points
    zDrawLegend(FALSE);                        // Draw in the Graph Legend
    zShowGraphTitles();                        // Show Graph Titles
  }

  zTerminateGraph();                            // Terminate Graphing
}


////////////////////////////// LogGraph() //////////////////////////////////
//  Plots [n] data points, (x[i], y[i]), 0 <= i < n.
//  Graph is plotted on one of three types of grids: LOG X - LOG Y,
//  LOG X - LINEAR Y, or LOG Y - LINEAR X
////////////////////////////////////////////////////////////////////////////

void LogGraph(zGraphStruct *zG)
{
  // Declare an Instance of the Log Graph
  zLogGraph zLog(zG);

  // Draw It
  zLog.zDraw();
}

// EOF -- LOG.CPP
