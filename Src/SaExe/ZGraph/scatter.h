// SCATTER.H

/////////////////////////////////////////////////////////////////////////////
//                   SCATTER.H -- Header File for zScatterGraph Class
//
// ZGRAF C++ Multi-Platform Graph Toolkit v. 1.41, (c) 1996, ZGRAF Software
/////////////////////////////////////////////////////////////////////////////

#ifndef _ZSCATTER_H_
#define _ZSCATTER_H_

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

#ifdef MS_DOS_PLATFORM
#include <graph.h>
#endif

#ifdef BGI_DOS_PLATFORM
#include <graphics.h>
#endif


#include "toolkit.h"                             // Toolkit Header File
#include "zgraph.h"                              // Basic Graph Class


class zScatterGraph : public zGraph
{
protected:
	double Multiplier1;                  // Used In Mapping 3-D to 2-D
	double Multiplier2;                  //   Coordinates
	double Multiplier3;
	double Multiplier4;
	// Attributes
public:
	INT  z_axis_divisions;       // Number of Graph Z-Axis Divisions

	// Operations
public:
	zScatterGraph(zGraphStruct *zG) : zGraph(zG) { }
	~zScatterGraph() {}
	BOOL zInitGraph();           // Stuff to Do When Initializing Graph
	void zDraw3DGrid();          // Base-Class Override--Draws 3D Grid
	void zDrawDataPoints();
	void zDrawGraphAxes();       // Draws In the Graph Axes
	void zDraw2DDataPoints();    // Draws In Data Points on 2-D Scatter Graph
	void zDraw3DDataPoints();    // Draws In Data Points on 3-D Scatter Graph
	void zGetGridDimensions();   // Gets Size of 3-D Grid
	BOOL zScaleData();           // Performs Data Scaling
	POINT zMapCoordinates(       // Converts 3-D Point to 2-D
		double X1,
		double Y1,
		double Z1);
	void zDraw(void);            // Draws the Graph
	void zFlip(void);            // Flip Data Points for Flip F2 F1 graph
};

#endif

