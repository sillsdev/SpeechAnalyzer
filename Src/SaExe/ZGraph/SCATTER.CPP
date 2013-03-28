// SCATTER.CPP
//
////////////////////////////////////////////////////////////////////////////
//
//   This Module Consists of the ZGRAF Toolkit Graph Functions
//   Used to Make Scatter Graphs [The Code for the 3-D Scatter Graph
//   was Provided Courtesy of Martin Zeisak of Freising, Germany.
//   Thanks for Your Help, Martin!]
//
// ZGRAF C++ Multi-Platform Graph Toolkit v. 1.41, (c) 1996, ZGRAF Software
//
////////////////////////////////////////////////////////////////////////////
#include "scatter.h"

//////////////////////////////////////////////////////////////////////////
//                           zScatterGraph Class
//////////////////////////////////////////////////////////////////////////
BOOL zScatterGraph::zInitGraph()
{
    // Call Base-Class Implementation
    BOOL status = zGraph::zInitGraph();

    //
    // Establish Size of the Grid
    //
    if (_3D_flag)
    {
        zGetGridDimensions();
    }

    return (status);
}

void zScatterGraph::zGetGridDimensions()
{
    //
    // Sets the Dimensions of the 3-D Graph Grid and Sets Some
    //   Constant Multipliers Which Are Used in the Coordinate
    //   Scaling Calculations
    //

    // First Determine Max/Min in the Z-Data Array
    zSizeData();

    zend = zmax;
    zstart = zmin;
    z_axis_divisions = 5;
    yend = ymax;
    ystart = ymin;
    xend = xmax;
    xstart = xmin;

    GridWidth  = zRound((double)X_RIGHT - X_LEFT - z3D_GRID_X_SLANT_OFFSET);
    GridHeight = zRound((double)Y_BOTTOM - (z3D_GRID_Y_SLANT_OFFSET + Y_TOP));

    Multiplier1 = z3D_GRID_X_SLANT_OFFSET / (xend - xstart);
    Multiplier2 = GridWidth / (yend - ystart) ;
    Multiplier3 = z3D_GRID_Y_SLANT_OFFSET / (xend - xstart);
    Multiplier4 = GridHeight / (zend - zstart);
}


void zScatterGraph::zDraw3DGrid()
{
    // For the 3-D grid We Add Just a Small Refinement--
    //   More Lines on the Floor Plane

    // Call Base Class to Draw Basic 3-D Grid
    zGraph::zDraw3DGrid();

    //
    // Switch In the Grid Pen
    //
#ifdef WINDOWS_PLATFORM
    SelectObject(hDC, hGridPen);
#endif

#ifdef OS2_PLATFORM
    // Set the Grid Line Style
    GpiSetLineType(hPS, OS2_GridLineType);

    // Set Axis Color
    GpiSetColor(hPS, AxisColor);
#endif

#ifdef MS_DOS_PLATFORM
    // Select the Grid Mask
    _setlinestyle(GridMask);

    // Set Axis Color
    _setcolor((short)AxisColor);
#endif

#ifdef BGI_DOS_PLATFORM
    // Select the Type of Lines for the Grid
    setlinestyle(bgiGridType, 0, NORM_WIDTH);
    setcolor((int)AxisColor);
#endif

    //
    // Draw In the Individual Vertical Grid Segments
    //
    double grid_height = (double) Y_BOTTOM - zY_ADJUST * (Y_TOP + z3D_GRID_Y_SLANT_OFFSET);
    for (INT j = 0; j < z_axis_divisions; j++)
    {
        INT y_offset_1 = Y_BOTTOM - zY_ADJUST * zRound(grid_height * j / z_axis_divisions);
        INT y_offset_2 = y_offset_1 - zY_ADJUST * zRound(z3D_GRID_Y_SLANT_OFFSET);

        zMoveTo(X_LEFT, y_offset_1);
        zLineTo(X_LEFT + zRound(z3D_GRID_X_SLANT_OFFSET), y_offset_2);
        zLineTo(X_RIGHT, y_offset_2);
    }


#ifdef WINDOWS_PLATFORM
    // Select Axis Pen Again
    SelectObject(hDC, hAxisPen);
#endif

#ifdef OS2_PLATFORM
    // Back to Solid Lines
    GpiSetLineType(hPS, LINETYPE_SOLID);
#endif

#ifdef MS_DOS_PLATFORM
    // Select the Default Line Mask Again
    _setlinestyle(0xFFFF);
#endif

#ifdef BGI_DOS_PLATFORM
    // Back to Solid Lines
    setlinestyle(SOLID_LINE, 0, NORM_WIDTH);
#endif

}

void zScatterGraph::zDrawDataPoints()
{
    if (_3D_flag)         // If Making a 3-D Style Graph
    {
        zDraw3DDataPoints();
    }
    else                 // If Making a 2-D Style Graph
    {
        zDraw2DDataPoints();
    }
}

void zScatterGraph::zDraw2DDataPoints()
{
    //
    //  Plots Data Points for a Scatter Graph.  This Function is
    //    Overriden From the Base zGraph Class...
    //
    zActivatePens();

    // Select the First Graph Color
    zSelectGraphColor(0);

    //
    // Draw in the Data Points
    //
    INT color_index = 0;
    for (INT j = 0; j < n; j++)          // For Each Data Point. . .
    {
        INT symbol = abs(sym[j]);         // This is the symbol to display

        //
        // Choose New Color For Each New Symbol Set
        //
        if (j > 0  && (symbol != abs(sym[j-1])))
        {
            ++color_index ;
        }

        if (symbols_flag)
        {
            zDraw_Symbol(xnew[j], ynew[j], symbol, color_index);
        }
        else
        {
            //
            // Draw Each Data Point.  Note -- 1 Pixel Is Too Small to See (!)
            //   So Draw a Little Cluster of Pixels at Each Data Point...
            //
            int l = 1;
            //if (n == 1) l *= 2; // if only one data point to draw, make it bigger
            if (j == n-1)
            {
                l *= 2;    // make last data point larger   AKE 7/31/2001
            }
            for (int i = -l; i <= l; i++)
            {
                for (int k = -l; k <= l; k++)
                {
                    // Set New Color
                    zSelectGraphColor(color_index);

                    // Set One Pixel
                    zSetPixel(xnew[j] + k, ynew[j] + i, zGetCurrentColor());          // tdg
                }
            }
        }
    }

    //
    // Save the Number of Data Sets Found in the Data--It Will Be
    //  Used in Making the Graph Legend!
    //
    num_sets = color_index + 1;
}

void zScatterGraph::zDraw3DDataPoints()
{
    //
    // Draws a 3-D Graph Composed of Discrete Points
    //

    //
    // Set Up Pens
    //
    zActivatePens();

    //
    // Draw Graph.  Graph is Drawn Back-to-Front, to Eliminate the Need
    //   for any Elaborate Hidden-Line Removal Agorithm.
    //
    INT color_index = 0;

    // Select the First Graph Color
    zSelectGraphColor(0);

    for (INT j = 0; j < n; j++)
    {
        //
        // Map the Current Data Point into our (X,Y) Window Area
        //
        POINT NewPoint = zMapCoordinates(Xdata[j], Ydata[j], Zdata[j]);


        //
        // This is the symbol to display
        //
        INT symbol = abs(sym[j]);

        //
        // Choose New PEN color for each NEW symbol set.
        //
        if (j > 0  && (symbol != abs(sym[j-1])))
        {
            ++color_index ;

            // Set New Color
            zSelectGraphColor(color_index);

        }

        //
        // Display the symbol if we're supposed to...
        //
        if (symbols_flag)
        {
            zDraw_Symbol(NewPoint.x, NewPoint.y, symbol, color_index);
        }
        else
        {
            int l = 1;
            if (n == 1)
            {
                l *= 2;    // if only one data point to draw, make it bigger
            }
            for (INT i = -l; i <= l; i++)
            {
                for (INT k = -l; k <= l; k++)
                {
                    // Set New Color
                    zSelectGraphColor(color_index);

                    // Set This Pixel Color
                    zSetPixel(NewPoint.x + k, NewPoint.y + i,
                              zGetCurrentColor());
                }
            }
        }

        if (line_connect)
        {
            // Set New Color
            zSelectGraphColor(color_index);

            // Figure the Coordinates of a Point on the Floor
            POINT FlatPoint = zMapCoordinates(Xdata[j], Ydata[j], zstart);

            zDrawLine(NewPoint.x, NewPoint.y, FlatPoint.x, FlatPoint.y);
        }
    }

    //
    // Save the Number of Data Sets Found in the Data--It Will Be
    //  Used in Making the Graph Legend!
    //
    num_sets = color_index + 1;

}

BOOL zScatterGraph::zScaleData()
{
    // Call Base Class to Handle X- and Y- Components
    BOOL bStatus = zGraph::zScaleData();

    return (bStatus);
}

POINT zScatterGraph::zMapCoordinates(double X1, double Y1, double Z1)
{
    // Maps a 3-D Point Into 2-D Space.  See File XYZ.CPP for Full
    //   Details

    double XPrime_XComponent = (xend - X1)   * Multiplier1;
    double XPrime_YComponent = (Y1 - ystart) * Multiplier2;

    //
    // Compute the Component Adjustments for the New YPrime Variable
    //
    double YPrime_XComponent = (xend - X1)   * Multiplier3;
    double YPrime_ZComponent = (Z1 - zstart) * Multiplier4;

    POINT pt =
    {
        (X_LEFT   + (INT)(XPrime_XComponent + XPrime_YComponent)),
        (Y_BOTTOM - (INT)(YPrime_XComponent + YPrime_ZComponent))
    };

    return (pt);
}

void zScatterGraph::zDrawGraphAxes()
{
    //
    // Displays Numbers along the X-, Y-, and Z-axes for the 3-D Graph
    //
    if (! _3D_flag)
    {
        // For 2-D Graph, Just Use the Base-Class Implementation
        zGraph::zDrawGraphAxes();
        return;
    }

    zActivatePens();


    //
    // 3-D Graph's           Z
    // Axes Orientation:     |
    //                       |
    //                       |
    //                       ---------- Y
    //                     /
    //                   /
    //                X/
    //

    //
    // Put in Numbers Along the Y-Axis
    //  NOTE:  The Y-Axis Is Normally Our X-Axis!
    //
    WCHAR zbuf[20];
    INT i, j, k;
    INT y_left = X_LEFT,
        y_right = X_RIGHT - zRound(z3D_GRID_X_SLANT_OFFSET);

    double y_inc = (ymax - ymin) / y_axis_divisions;
    double ytmp  = (double)(y_right - y_left) / y_axis_divisions;


    // For Each Y-Axis Graph Division...
    for (i = 0; i <= y_axis_divisions; i++)
    {
        double y_value = ymin + (double)i * y_inc;

        // Show This Axis Number.  Set Last Value to [ymax]
        zConvertValue(y_value, zbuf, _countof(zbuf));

        // Draw In this Text/Number String
        j = y_left - charsize + zRound(ytmp * i);


        INT yCorrection = zY_ADJUST * 5;

#ifndef WINDOWS_PLATFORM
        yCorrection += 3 * charsize / 2;   // A Small Correction for OS/2
#endif

        // Draw the String
        AxisLabelFont.zDrawTextString(j,
                                      Y_BOTTOM + yCorrection,
                                      zbuf);
    }

    //
    // Put in Numbers Along the X-Axis
    //
    double x_value, x_inc = (xmax - xmin) / x_axis_divisions ;

    for (i = 0; i <= x_axis_divisions; i+=2)
    {
        // Calculate coordinates for text
        INT irev = draw_forward?x_axis_divisions-i:i;
        j = X_LEFT + GridWidth + zRound(z3D_GRID_X_SLANT_OFFSET * irev / x_axis_divisions);     // horizontal
        k = Y_BOTTOM - zY_ADJUST * zRound(z3D_GRID_Y_SLANT_OFFSET * irev / x_axis_divisions);   // vertical

        // Calculate tic mark value
        x_value = xmin + (double)i * x_inc;

        // Convert tic mark value to text string
        zConvertValue(x_value, zbuf, _countof(zbuf));

        // Draw in this X-Axis Value Label
        AxisLabelFont.zDrawTextString(j + charsize*3, k - charheight/2, zbuf);
    }


    //
    // Put in Numbers Along the Z-Axis
    //   NOTE--The Z-Axis Is Normally Our Y-Axis!
    //
    INT z_bottom = Y_BOTTOM,
        z_top = Y_TOP + zRound(z3D_GRID_Y_SLANT_OFFSET);

    double z_value, z_inc = (zmax - zmin) / z_axis_divisions ;
    double zstep = (double)(z_bottom - z_top) / z_axis_divisions;

    // Draw In the Z-Axis Divisions
    for (i = 0; i <= z_axis_divisions;  i++)
    {
        // Calculate tic mark value
        z_value = zmin + (double)i * z_inc;

        // Convert tic mark value to text string
        zConvertValue(z_value, zbuf, _countof(zbuf));

        // Draw in this Z-Axis Value Label
        // Use Right Justification For Axis Numbers--It Looks Cleaner
        j = zRound(z_bottom - i*zstep);
        AxisLabelFont.zRightAlignText(X_LEFT - charsize, j + charheight/2, zbuf);
    }

}


void zScatterGraph::zDraw()
{
    //
    // Prepare to Draw the Graph
    //
    zInitGraph();                              // Initialize Graph
    zScaleData();                              // Scale the X,Y Data

    //
    // Draw In the Graph...
    //
    zDrawGraphAxes();                          // Draw Graph Axes
    zDrawGrid();                               // Draw Graph Grid
    zDrawDataPoints();                         // Plot the Data Points
    zDrawLegend(FALSE);                        // Draw in the Graph Legend
    zShowGraphTitles();                        // Show Graph Titles

    //
    // Clean Up and Exit...
    //
    zTerminateGraph();                         // Terminate Graph
}



void zScatterGraph::zFlip()                      // This routine Flips data for F2 F1 & (F2-F1) F1 Charts
{
    INT grid_width  = abs(X_RIGHT - X_LEFT);     //  Taken from zGraph.cpp   // tdg
    INT grid_height = abs(Y_TOP - Y_BOTTOM);

    double a = (double) grid_width / (xmax - xmin);
    double b = (double) X_LEFT - a * xmin;
    double c = zY_ADJUST * (double) grid_height / (ymin - ymax);
    double d = (double) Y_TOP - c * ymax ;
    int j;

    for (j = 0; j < n; j++)
    {
        //
        // Scale X-Coordinate
        //
        xnew[ j ] = (INT)(a * ((xmax + xmin) - Xdata[ j ]) + b) ;

        //
        // Scale Y-Coordinate
        //
        ynew[ j ] = (INT)(c * ((ymin + ymax) - Ydata[ j ]) + d) ;
    }
}



//////////////////////////  ScatterGraph() /////////////////////////////////

void zGRAPH_TYPE  ScatterGraph(zGraphStruct * zG)
{
    // Declare an Instance of the "zScatterGraph" Class
    zScatterGraph zScatter(zG);

    // Draw It
    zScatter.zDraw();
}


// EOF -- SCATTER.CPP
