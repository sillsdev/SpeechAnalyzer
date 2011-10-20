// ZGRAPH.CPP
//
/////////////////////////////////////////////////////////////////////////////
//   This Module Consists of the ZGRAPH Class Functions.  The ZGRAPH
//   Class is Used as the Base Graph Class, from which all other
//   Graph Classes are Derived.
//
// ZGRAF C++ Multi-Platform Graph Toolkit v. 1.41, (c) 1996, ZGRAF Software
//
/////////////////////////////////////////////////////////////////////////////
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


//
// Platform-Dependent Includes...
//
#ifdef WINDOWS_PLATFORM
#include <windows.h>
#include "zfont.h"
#endif

#ifdef OS2_PLATFORM
#define INCL_WIN
#define INCL_GPI
#include <os2.h>
#endif


#include "toolkit.h"                             // Toolkit Header File
#include "zgraph.h"                              // Basic Graph Class


#ifdef MS_DOS_PLATFORM
#include "graph.h"
#endif

#ifdef BGI_DOS_PLATFORM
#include <graphics.h>
#endif

//
// Variables/Arrays Local to this Module
//
static WCHAR zbuf[ 175 ] ;                        // Working Buffer


//////////////////////////////////////////////////////////////////////////
///////////////////////////// zGraph Class ///////////////////////////////
//////////////////////////////////////////////////////////////////////////

////////////////////////////// zGraph() ////////////////////////////////

zGraph::zGraph(zGraphStruct *zGS1 /* = NULL */)
{
	//
	// Constructor for ZGRAPH Class.  Initializes as Necessary
	//   for Making Graph
	//
	zInitGraphData(zGS1);                   // Initialize, Set Defaults

	GraphOpen = TRUE;
}

////////////////////////////// ~zGraph() ////////////////////////////////

zGraph::~zGraph()
{
	//
	// Destructor for ZGRAPH Class
	//
	zTerminateGraph();
}

///////////////////////////// zInitGraph() //////////////////////////////

void zGraph::zInitGraphData(zGraphStruct *zGS1 /* = NULL */)
{
	//
	// Initializes and Sets a Bunch of Defaults for the Graph
	//
	zGraphStruct *zG;
	BOOL bUserPassedStructure = TRUE;

	//
	// If User Didn't Pass a zGraphStruct, Create a Default One,
	//   and Zero It Out...
	//
	if (! zGS1)
	{
		bUserPassedStructure = FALSE;

		zG = new zGraphStruct();
		memset(zG, zDEFAULT, sizeof(zGraphStruct));
	}
	else
		// Otherwise We Just Grab the Structure the User Passed Us...
		zG = zGS1;


	xnew = ynew = NULL;                         // NULL Out Pointers to
	YWorkData   = NULL;                         //  Dynamically-Created Arrays
	show_legend_percents = TRUE;
	pens_created = brushes_created = FALSE;     // Start With No Objects


	//
	// Set Graph Parameters and Switches
	//
#ifdef WINDOWS_PLATFORM
	zSetGraphDC(zG->hdc);                    // Windows--Save Graph HDC
#endif

#ifdef OS2_PLATFORM
	zSetGraphHPS(zG->hps);                   // OS2--Save HPS
	zSetGraphHWND(zG->hwnd);                 // OS2--Save HWND
#endif

	zSetGraphWindow(zG->RWindow);               // Save Graph Window Area
	zSetBkgGradient(zG->bkg_gradient_mode);     // Set Bkg. Gradient Mode
	zSetBkgGradientColorScheme(
		zG->bkg_gradient_color_scheme);        // Set Bkg. Gradient Color Scheme
	zSetGraphColors(zG->axis_color,
		zG->bkg_color,
		zG->plot_color);                         // Set Graph Axis/Plot Colors
	zSetGraphTitles(zG->title, zG->H_Title,
		zG->V_Title);                            // Set Graph Titles
	zSetLegendPosition(zG->legend_position);    // Set Legend Position
	zSetItemTitleFormat(zG->item_title_format); // Set Item Title Format
	zSetLegendTitles(zG->LegendTitles);         // Set Legend Titles
	zSetItemTitles(zG->ItemTitles);             // Set the Item Titles

	zSetData(zG->n,
		zG->xdata,                               // Assign the Graph Data
		zG->ydata,
		zG->zdata);
	zSetXDataRange(zG->xstart, zG->xend);       // Sets X-, Y-, and Z-Data
	zSetYDataRange(zG->ystart, zG->yend);       //   Ranges for Manual
	zSetZDataRange(zG->zstart, zG->zend);       //      Graph Scaling Option
	zSetSymbols(zG->symbols);                   // Assign the Graph Symbols

	zSetShadingStyle(zG->shading_style);        // Set Shading Style
	zSetAutoScaling(zG->auto_scale);            // Set Scaling Option
	zSetLineConnect(zG->line_connect);          // Connecting Line Option
	zSetSymFlag(zG->symbols_flag);              // Graph Symbols
	zSetGridFlag(zG->grid_flag);                // Graph Grid
	zSet3DFlag(zG->_3D_flag);                   // 3-D Graph Option
	zSetBrushFillStyle(zG->fill_style);         // Brush Fill Style
	zSetAxisStyle(zG->x_axis_style,
		zG->y_axis_style);                       // Set Graph Axes Style
	zSetGridStyle(z2D_GRID);                    // Default to 2-D Grid
	zSetGridPenStyle(zG->grid_pen_style);       // Set Grid Pen Style
	zSetGraphQuality(zG->quality);              // Set Graph Quality
	zSetFunction(zG->function);                 // Set Function [F(X,Y)]
	zSetStackingDirection(zG->draw_forward);    // Set Stacking Direction
	zSetAxisDivisions(zG->x_axis_divisions,     // Set the Default Number of
		zG->y_axis_divisions);                   //   Div's Along X, Y Axis
	zSetDigitFormat(zG->digit_format);          // Select Fixed, Float Pt.,
	//   Integer, Sci. Notation
	//   for Axis Numbers
	zSetDigitPrecision(zG->digit_precision);    // Set Axis Digit Precision
	zSetUpFonts();                              // Set Up Fonts for the Graph


	// Destroy Dynamically-Created [zGraphStruct], if Necessary...
	if (! bUserPassedStructure)
		delete zG;


}


////////////////////////// zSetUpGraphLayout() ////////////////////////////

void zGraph::zSetUpGraphLayout()
{
	//
	// Sets Up Rectangular Areas Within the Window Where the Graph
	//   Legend and the Graph Itself Will Be Displayed
	//

	//
	// Set Up the Rectangular Area for the Graph Axes
	//
	zSetGraphAxesRect();

	//
	// Compute Graph Legend Rectangle
	//
	zSetLegendRect();
}

////////////////////////// zTerminateGraph() //////////////////////////////

void zGraph::zTerminateGraph()
{
	//
	// This Function Gets Called When We're Done Drawing a Graph
	//
	if (GraphOpen)
	{
		zDeleteArrays();                        // Kill Dynamically-Created Arrays
		zRestoreGraphics();                     // Restore Old Graphics Objects
		zDeletePens();                          // Delete Pen Objects
		zDeleteBrushes();                       // Delete Brush Objects
	}

	// Now Set Flag So We Don't Do This Again...
	GraphOpen = FALSE;
}

//////////////////////////// zDeleteArrays() //////////////////////////////

void zGraph::zDeleteArrays()
{
	//
	// Deletes any Dynamically-Created Working Arrays
	//
	if (xnew != NULL)
		delete [] xnew;

	if (ynew != NULL)
		delete [] ynew;

	if (YWorkData != NULL)
		delete [] YWorkData ;
}

////////////////////////// zSetGraphTitles() //////////////////////////////

void zGraph::zSetGraphTitles(PWCHAR MainTitle1, PWCHAR HorizTitle1,
							 PWCHAR VertTitle1)
{
	//
	// Sets the Graph Titles (Main, Horizontal, Vertical)
	//
	MainTitle  = MainTitle1  ;
	HorizTitle = HorizTitle1 ;
	VertTitle  = VertTitle1  ;

	show_axis_titles = TRUE  ;
}

////////////////////////// zSetLegendTitles() /////////////////////////////

void zGraph::zSetLegendTitles(WCHAR LegendTitle1[])
{
	//
	// Sets the Graph Legend Titles
	//
	if (LegendPosition != zNO_LEGEND)
	{
		for (INT i = 0; i < zMAX_DATA_SETS; i++)
			LegendTitle[ i ] = LegendTitle1[ i ] ;
	}
}

////////////////////////// zSetItemTitles() ////////////////////////////

void zGraph::zSetItemTitles(WCHAR ItemTitle1[])
{
	//
	// Sets the Graph Item Titles
	//
	if (item_title_format != zNO_ITEM_TITLES)
	{
		// We Will Show the Item Titles

		for (INT i = 0; i < zMAX_ITEM_TITLES; i++)
			ItemTitle[ i ] = ItemTitle1[ i ] ;
	}
}

////////////////////////// zSetGraphColors() ////////////////////////////

void zGraph::zSetGraphColors(zRGB axis_color,
							 zRGB bkg_color, zRGB *graph_color)
{
	//
	// Sets the Colors to Be Used for the Graph Axis [axis_color] and
	//   the Brushes/Pens in the Graph ([graph_color] array).  NOTE:
	//   If User Didn't Pass an Array of Colors for Brushes/Pens,
	//   We Just Use Our Default Colors.
	//

	if (graph_color == NULL)
		zSelectDefaultGraphColors();                // Use Default Graph Colors
	else
	{
		for (INT i = 0; i < zMAX_GRAPH_COLORS; i++)
			GraphColor[ i ] = graph_color[ i ];
	}

	AxisColor = axis_color;
	BkgColor  = bkg_color;
}

/////////////////////// zSelectDefaultGraphColors() ////////////////////////

void zGraph::zSelectDefaultGraphColors()
{
	//
	// Sets the Default Colors to Be Used by Brushes/Pens in the Graph
	//
	for (INT i = 0; i < zMAX_GRAPH_COLORS; i++)
		GraphColor[ i ] = RGB_color(i + 1) ;      // Start with DARK BLUE

	AxisColor = RGB_color(zBRT_WHITE);
}

///////////////////////////// zSetData() /////////////////////////////////

void zGraph::zSetData(INT n_1,
					  double *Xdata_1, double *Ydata_1, double *Zdata_1)
{
	//
	// Sets the Number of Data Points in the Graph, and also
	//    Sets the X, Y, Z Data Pointers to Point to the Graph Data
	//    The Last 2 Args. are Default Args (they default to NULL).
	//
	n = n_1 ;                    // Number of Data Points in Graph

	Xdata = Xdata_1;             // Set the Data
	Ydata = Ydata_1;             //   Point
	Zdata = Zdata_1;             //     Arrays
}

////////////////////////// zSetSymbols() //////////////////////////////////

void zGraph::zSetSymbols(INT *symbols_1)
{
	//
	// Sets the Array of Symbols to Be Used in Drawing the Graph
	//
	sym = symbols_1;
}

/////////////////////////////// zSizeData() //////////////////////////////

void zGraph::zSizeData()
{
	//
	// Determines the Max/Min Within the Range of Data Points.
	//   Specifically, the Minimum and Maximum Values of the X-Data Array
	//   (if it Exists) and the Y-Data Arrays are Determined By
	//   this Function.
	//

	//
	// If We are Automatically Scaling X,Y points (i.e., if User
	//   Specified Auto-Scaling...
	//
	if (auto_scaling)
	{
		if (Xdata != NULL)
		{
			// If User Supplied an X- Data Array, We'll  Find the Max. and
			//   Min. X- Values
			xmax = xmin = Xdata[0] ;
		}
		else
		{
			// If User Didn't Supply An X- Data Array, Don't Waste Our Time.
			xmax  = xmin = 0.0;
		}

		if (Zdata != NULL)
		{
			// If User Supplied an Z- Data Array, We'll  Find the Max. and
			//   Min. Z- Values
			zmax = zmin = Zdata[0] ;
		}
		else
		{
			// If User Didn't Supply An Z- Data Array, Don't Waste Our Time.
			zmax  = zmin = 0.0;
		}


		// Assume that the User ALWAYS Supplies a Y- Data Array
		ymax = ymin = Ydata[0] ;

		for (INT i = 1; i < n; i++)
		{
			// Scan for Max/Min X- Values
			if (Xdata != NULL)
			{
				if (Xdata[i] > xmax)
					xmax = Xdata[i];

				if (Xdata[i] < xmin)
					xmin = Xdata[i];
			}

			// Scan for Max/Min Z- Values
			if (Zdata != NULL)
			{
				if (Zdata[i] > zmax)
					zmax = Zdata[i];
				if (Zdata[i] < zmin)
					zmin = Zdata[i];
			}

			// Scan for Max/Min Y-Values
			if (Ydata[i] > ymax)
				ymax = Ydata[i];

			if (Ydata[i] < ymin)
				ymin = Ydata[i];
		}
	}
	else
	{
		//
		// Otherwise, If User Specified Manual Scaling, the Minima and Maxima
		//   are User-Defined Values!
		//
		xmin = xstart;  ymin = ystart;  zmin = zstart;
		xmax = xend  ;  ymax = yend  ;  zmax = zend;
	}
}

////////////////////////// zSetAxisDivisions() ////////////////////////////

void zGraph::zSetAxisDivisions(INT x_divisions, INT y_divisions)
{
	//
	// Sets the Number of Axis Divisions along the
	//   X- and Y- Axes.  If Either Argument is Set to [zDEFAULT],
	//   Then a default axis-division value is used for that axis.
	//
	//   SPECIAL CASE:  If A Negative Value for [x_divisions] and/or
	//                  [y_divisions] Is Used, Then The Program
	//                  WILL AUTOMATICALLY DETERMINE THE NUMBER
	//                  OF DIVISIONS To Use For That Axis, Based
	//                  Upon a Best-Fit Scaling Algorithm.  See the
	//                  "zGetBestFitAxisDivisions()" Function for
	//                  More Details...
	//
	x_axis_divisions = ((x_divisions == zDEFAULT)
		? zDEFAULT_X_AXIS_DIVISIONS : x_divisions);

	y_axis_divisions = ((y_divisions == zDEFAULT)
		? zDEFAULT_Y_AXIS_DIVISIONS : y_divisions);
}

////////////////////////// zSetDigitFormat() ////////////////////////////

void zGraph::zSetDigitFormat(INT digit_format_1)
{
	//
	// Sets the Style for Which the User Wants to Display Numbers
	//   along the Graph Axes.
	//
	digit_format = digit_format_1;
}

////////////////////////// zSetDigitPrecision() ////////////////////////////

void zGraph::zSetDigitPrecision(INT precision_1)
{
	//
	// Sets the Precision [Number of Significant Digits] for Numbers
	//   Put Along the X- and Y- Axes.  If Argument is Set to [zDEFAULT],
	//   a Default Value is Used.
	//
	digit_precision = precision_1;
}

////////////////////////// zGetSetsAndSamples() //////////////////////////

void zGraph::zGetSetsAndSamples()
{
	//
	// Determines the Number of Sets and Samples Appearing In the Data.
	//   NOTE:  Each Data Set Has a Common Symbol Value.
	//   Note that this Function Expects Data to be Resident in the
	//   Data Arrays in a Logical Order, i.e., Data Should Be in the Form:
	//
	//     SET       SAMPLE     SYMBOL
	// -----------------------------
	//       1         1        A
	//       1         2        A
	//       .         .        .
	//       .         .        .
	//       1        [n]       A
	// -----------------------------
	//       2         1        B
	//       2         2        B
	//       .         .        .
	//       .         .        .
	//       2        [n]       B
	// -----------------------------
	//     (etc.)
	//

	num_samples = 0;
	num_sets    = 1;

	for (INT i = 0; i < n; i++)
	{
		if (i > 0)
		{
			// If the Symbol Value Changes We Have a New Data Set

			if (sym[ i ] != sym[i - 1])
				num_sets++ ;
		}

		// Tally the Number of Samples for the First Set Only.
		//   (All the Other Sets Must Have the Same Number of Samples)

		if (num_sets == 1)
			num_samples++ ;
	}
}

////////////////////// zGetLongestLegendString() ///////////////////////////

void zGraph::zGetLongestLegendString(PWCHAR TemplateBuf)
{
	//
	// Finds the Longest Legend String and Returns a Template
	//   for it in [TemplateBuf].  The Template is Just Composed
	//   of a String With an Average Character (i.e., "A") Width...
	//

	//
	// Find the Longest Legend String
	//
	INT j, Len, MaxLen = 0;
	for (j = 0; j < zMAX_DATA_SETS  &&  j < num_sets; j++)
	{
		if ((Len = _tcslen(&(LegendTitle[ j ]))) > MaxLen)
			MaxLen = Len;
	}

	//
	// Make a String of A's as Long as the Longest String...
	//
	for (j = 0; j < MaxLen  &&  j < 80; j++)
		TemplateBuf[ j ] = 'A';
	TemplateBuf[ j ] = '\0';

}

////////////////////////// zConvertValue() ////////////////////////////////

void zGraph::zConvertValue(double d, PWCHAR buf)
{
	//
	// Converts a double value [d] to a string [Buf], Based Upon the Desired
	//   Numeric Format that the User Selects.  This Function is Used to
	//   Display Digits Along the Graph Axes in a Variety of Different
	//   Formats...
	//
#define BUF_SIZE 32
	static WCHAR TempBuf[BUF_SIZE];

	switch (digit_format)
	{
	case zSCI_NOTATION:           // If Using Scientific Notation
		{
			char buffer[BUF_SIZE];
			memset(buffer,0,BUF_SIZE);
			_gcvt_s( buffer, BUF_SIZE, d, digit_precision);
			// perform wide char conversion
			for (unsigned int i=0;i<BUF_SIZE;i++) {
				TempBuf[i] = buffer[i];
			}
		}
		_tcscpy(buf, TempBuf);
		return;

		// If User Wants Whole Numbers Only,
	case zROUND_INTEGER:          //   Round to Nearest Whole Integer
		_stprintf(TempBuf, _T("%-6.0lf"), d);
		_tcscpy(buf, TempBuf);
		break;


	case zFIXED:                   // User Specified Number of Sig. Digits
		_stprintf(TempBuf, _T("%-6.*lf"), //   That Should Be Shown After
			digit_precision, d);     //   the Decimal Point
		_tcscpy(buf, TempBuf);
		break;

	case zFLOAT_PT:               // If Just Using
	default:                      //    Regular Floating Pt. Format
		_stprintf(TempBuf, _T("%-6.*lf"), zDEFAULT_FLOAT_PRECISION, d);
		_tcscpy(buf, TempBuf);
		{
			//
			// Trim Any Spaces and Zeroes Off the End of the String.
			//   Get Rid Of the Decimal Point, if it is Not Followed
			//   By any Significant Digits...
			//

			PWCHAR pStart = _tcschr(buf, '.');
			if (pStart!= NULL)
			{
				PWCHAR p = &buf[ _tcslen(buf) - 1 ];
				INT count = 0;
				while ((*p == '0' || *p == '.' ||  *p == ' ')
					&&  p >= pStart)
				{
					*p-- = '\0';
					if (++count > 20)
						break;
				}
			}
		}
		return;
	}

	//
	// Trim Off Any Spaces at End Of String
	//
	INT pos = _tcslen(buf) - 1;
	while (pos > 0  &&  buf[ pos ] == ' ')
		buf[ pos-- ] = 0;
}

////////////////////////// zShowXAxisNumbers() ////////////////////////////

void zGraph::zShowXAxisNumbers()
{
	//
	// This Function Puts Number Values (or Item Title Strings)
	//   in Place Along the Graph's Horizontal X-Axis
	//   [This Function Can Be Overriden in Derived Classes].
	//   The Logic Here Assumes the Lowest-Common-Denominator--
	//   i.e., Just a Basic Linearly-Scaled X-Axis.
	//   Both 2-D and 3-D Style Graphs Are Supported
	//

	if (_3D_flag)        // If Drawing a 3-D Style Graph
	{
		zzShowXAxisNumbers(
			X_LEFT,
			X_RIGHT - zRound(z3D_GRID_X_SLANT_OFFSET));
	}
	else                 // If Drawing a 2-D Style Graph
	{
		zzShowXAxisNumbers(X_LEFT, X_RIGHT);
	}
}

////////////////////////// zzShowXAxisNumbers() ////////////////////////////

void zGraph::zzShowXAxisNumbers(INT x_left, INT x_right)
{
	//
	// Puts Number Values [or Item Title Strings] in Place along the
	//   Horizontal X-Axis of a Graph.  The Labels Extend
	//   from the client window x-coordinates [x_left] to [x_right]
	//
	//   NOTE:  If the User Specified "zTEXT_STRINGS" in the
	//          "item_title_format" element, We Display the Text Strings in
	//          the "ItemTitle[]" Array.  Otherwise if the "item_title_format"
	//          element is set to "zNUMBERS", We Display the Regular
	//          X-Axis Numbers...
	//

	// Return Now If Not Displaying any Item Titles...
	if (item_title_format == zNO_ITEM_TITLES)
		return;

	double x_inc = (xmax - xmin) / x_axis_divisions;
	double xtmp  = (double)(x_right - x_left) / x_axis_divisions;


	// For Each X-Axis Graph Division...

	for (INT i = 0; i <= x_axis_divisions; i++)
	{
		double x_value = xmin + (double)i * x_inc;

		//
		// Should We Display a Number or an Item Title?
		//
		if (item_title_format == zTEXT_STRINGS)
		{
			//
			// Show a Text String
			//
			if (i < zMAX_ITEM_TITLES  &&  ItemTitle[i] != NULL)
				_stprintf(zbuf, _T("%s"), ItemTitle[i]);
			else
			{
				// Alert User that Not Enough Item Titles Were Given
				//  Or Perhaps There Are Too Many Axis Divisions!

				_stprintf(zbuf, _T("%s"), _T("?"));
			}
		}
		else
		{
			//
			// Show a Number.  Set Last Value to [xmax]
			//
			zConvertValue(((i == x_axis_divisions) ? xmax : x_value), zbuf);
		}


		// Figure the Size of the Font for this String
		INT yOffset = Y_BOTTOM + zY_ADJUST * 5;

		// Draw In this Text/Number String
		INT xTextStart =
			x_left - AxisLabelFont.zGetFontWidth() + zRound(xtmp * i);


		AxisLabelFont.zDrawTextString(xTextStart, yOffset, zbuf);
	}
}

////////////////////////// zShowYAxisNumbers() ////////////////////////////

void zGraph::zShowYAxisNumbers()
{
	//
	// This Function Puts Number Values in Place Along the Y-Axis.
	//   [This Function Can Be Overriden in Derived Classes]
	//   The Logic Here Assumes the Lowest-Common-Denominator--
	//   i.e., Just a Basic Linearly-Scaled Y-Axis.
	//   Both 2-D and 3-D Style Graphs Are Supported
	//

	if (_3D_flag)        // If Drawing a 3-D Style Graph
	{
		zzShowYAxisNumbers(
			Y_BOTTOM,
			Y_TOP + zRound(z3D_GRID_Y_SLANT_OFFSET));
	}
	else                 // If Drawing a 2-D Style Graph
	{
		zzShowYAxisNumbers(Y_BOTTOM, Y_TOP);
	}
}

////////////////////////// zzShowYAxisNumbers() ////////////////////////////

void zGraph::zzShowYAxisNumbers(INT y_bottom, INT y_top)
{
	//
	// Puts Number Values in Place along the Y-Axis.  The Labels Extend
	//   from the client window y-coordinates [y_bottom] to [y_top].
	//

	double y_value, y_inc = (ymax - ymin) / y_axis_divisions ;
	INT ystep = zRound((double)(y_bottom - y_top) / y_axis_divisions) ;

	// Make a Correction for Very Small Window Heights!
	if (ystep <= 0)
		ystep = 1;

	INT j, k = 0;

	//
	// We Need to Consider the Special Case Where Y-data is
	//   Single-Valued.  In this Case Only One Axis Value Label is Needed!
	//
	if (ymin == ymax)
	{
		zConvertValue(ymin, zbuf);

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

	for (j = y_bottom; j >= (y_top - ystep/2);  k++, j -= ystep)
	{
		y_value = ymin + (double)k * y_inc;

		// Set Last Value to [ymax]
		zConvertValue(((k >= y_axis_divisions) ? ymax : y_value), zbuf);

		// Draw in this Y-axis Value Label
		AxisLabelFont.zRightAlignText(
			X_LEFT - AxisLabelFont.zGetFontWidth(),
			j,
			zbuf);
	}

}

////////////////////////// zSetGraphAxesRect() ////////////////////////////

void zGraph::zSetGraphAxesRect()
{
	//
	// Sets the Rectangle Bounds for the Graph Axes.  Note that the
	//   Graph Axes Rectangle Bounds are Dependent Upon Whether the
	//   Graph Legend is at the Left or at the Bottom of the Graph
	//   Window
	//
	RWindow.Normalize();


	switch (LegendPosition)
	{
	case zBOTTOM:
		//
		// If Legend Is to Appear at Bottom of Window
		//
		RGraphAxis.ScaleViaAbsoluteOffsets(
			RWindow,
			zX_AXIS_OFFSET_LEFT2   ,
			zY_AXIS_OFFSET_TOP     ,
			zX_AXIS_OFFSET_RIGHT   ,
			zY_AXIS_OFFSET_BOTTOM2);

		break;


	case zNO_LEGEND:
		//
		// If No Legend Is to Be Drawn For the Graph
		//
		RGraphAxis.ScaleViaAbsoluteOffsets(
			RWindow,
			zX_AXIS_OFFSET_LEFT2   ,
			zY_AXIS_OFFSET_TOP     ,
			zX_AXIS_OFFSET_RIGHT   ,
			zY_AXIS_OFFSET_BOTTOM) ;

		break;


	case zLEFT:
	default   :
		//
		// If Legend Is to Appear at Left Side of Window
		//
		RGraphAxis.ScaleViaAbsoluteOffsets(
			RWindow,
			zX_AXIS_OFFSET_LEFT    ,
			zY_AXIS_OFFSET_TOP     ,
			zX_AXIS_OFFSET_RIGHT   ,
			zY_AXIS_OFFSET_BOTTOM) ;

		break;
	}

	RGraphAxis.SetComponents(X_LEFT, Y_TOP, X_RIGHT, Y_BOTTOM);


	//
	// Set the Dimensions of the Grid Area, for Future Use
	//
	GridHeight = RGraphAxis.Height();
	GridWidth  = RGraphAxis.Width();

}

////////////////////////// zSetLegendRect() ////////////////////////////

void zGraph::zSetLegendRect()
{
	//
	// Sets the Rectangular Region to Be Used for the Graph Legend
	//
	switch (LegendPosition)
	{
	case zBOTTOM:
		//
		// If Legend Area Should Appear at Bottom of Graph
		//
		RLegend.SetTo(
			zX_AXIS_OFFSET_LEFT2     ,
			zGRAPH_LEGEND_Y_OFFSET3  ,
			zX_AXIS_OFFSET_RIGHT     ,
			zGRAPH_LEGEND_Y_OFFSET4) ;

		break;

	case zNO_LEGEND:
		//
		// If No Legend Is to Be Drawn
		//
		break;

	case zLEFT:
	default   :
		//
		// If Legend Area Should Appear at Left Side of Graph
		//
		RLegend.SetTo(
			zGRAPH_LEGEND_X_OFFSET   ,
			zGRAPH_LEGEND_Y_OFFSET   ,
			zGRAPH_LEGEND_X_OFFSET2  ,
			zGRAPH_LEGEND_Y_OFFSET2) ;

		break;

	}

}

////////////////////////// zDrawGraphAxes() ////////////////////////////

void zGraph::zDrawGraphAxes()
{
	//
	// Draws in X- and Y- Axes for a Graph.  The Default
	//  Implementation Just Handles the Basic 2-D or 3-D Style Graph,
	//  and Shows Values Along Both Axes
	//

	zzDrawGraphAxes(TRUE, TRUE);
}

////////////////////////// zzDrawGraphAxes() ////////////////////////////

void zGraph::zzDrawGraphAxes(BOOL show_x_values, BOOL show_y_values)
{
	//
	// Draws in X- and Y- Axes for a Graph.  [show_x_values] and
	// [show_y_values] are DEFAULT arguments [default value = TRUE], which
	// indicate whether we should put labels along the x- and/or y-axis.
	//

	zActivatePens();

	//
	// Draw In Numbers Along the Vertical (Y) Axis
	//
	if (show_y_values)
		zShowYAxisNumbers();

	//
	// Draw In Numbers Along the Horizontal (X) Axis
	//
	if (show_x_values)
		zShowXAxisNumbers();

	//
	// Now Put Enclosing Rectangle (with X-, Y-Axis) in Place
	//
	// NOTE:  No Special Axes Need to Be Drawn for the 3-D Graph, Since
	//        the "zDrawGrid()" Function Puts In the Axes Also...

	if (! _3D_flag)
	{
		RECT RTemp =
		{
			X_LEFT, Y_TOP, X_RIGHT, Y_BOTTOM
		};
		zDrawRect(&RTemp);
	}
}

////////////////////////// zCheckForLegalData() ////////////////////////////

BOOL zGraph::zCheckForLegalData()
{
	//
	// Returns TRUE if User Entered Valid Data for a Given Graph,
	//   Else FALSE.  NOTE:  This Is a VIRTUAL Function, and
	//   Should Be Overriden if You Derive another Class from the
	//   zGraph Class.  The Default Implementation Here Just
	//   Assumes a Basic X/Y Type Graph, With Linearly-Scaled
	//   Axes...
	//

	//
	// First We Find the Number of Data Sets and Data
	//   Samples Appearing in the Data
	//
	zGetSetsAndSamples();

	//
	// All Data Is Legal For Linear X- and Y- Axis Case...
	//
	return (TRUE);
}

/////////////////////////////// zDrawGrid()  /////////////////////////////

void zGraph::zDrawGrid()
{
	//
	// This Function is Used to Create the Grid for the Graph.
	//   The Default Implementation of This Function Handles
	//   2-D and 3-D Style Grids
	//

	//
	// See If We're to Make a 2-D or 3-D Style Grid
	//
	if (_3D_flag)
		zDraw3DGrid();       // 3-Dimensional Grid
	else
		zDraw2DGrid();       // 2-Dimensional Grid

}

/////////////////////////////// zDraw2DGrid()  /////////////////////////////

void zGraph::zDraw2DGrid()
{
	//
	// Puts a 2-D Flat Graph Grid in Place.  Note:  The Logic Here
	//   Assumes that [x_axis_style] and [y_axis_style] are set to
	//   [zLINEAR] for Linear Divisions.  This is The Default
	//   Implementation, Used to Handle the Basic X/Y-Type Graph
	//   With Linear Axes.  You Can OVERRIDE This Function in
	//   Your Derived Class, to Draw Other Grid Styles.
	//
	INT j;

	//
	// Return Now If We're Not Supposed to Draw the Grid
	//
	if (! zIsGridVisible())
		return;

	//
	// Select Grid Pen
	//
#ifdef WINDOWS_PLATFORM
	SelectObject(hDC, hGridPen);
#endif

#ifdef OS2_PLATFORM
	GpiSetLineType(hPS, OS2_GridLineType);
#endif

#ifdef MS_DOS_PLATFORM
	// Select the Grid Line Mask
	_setlinestyle(GridMask);
#endif

#ifdef BGI_DOS_PLATFORM
	// Select the Type of Lines for the Grid
	setlinestyle(bgiGridType, 0, NORM_WIDTH);
#endif

	//
	// Draw Vertical Grid Lines In
	//
	double tmp1 = (double) GridWidth / x_axis_divisions;

	for (j = 0; j < x_axis_divisions; j++)
	{
		INT xPos = X_LEFT + zRound(tmp1 * j) ;

		zDrawLine(xPos, Y_TOP, xPos, Y_BOTTOM);
	}

	//
	// Draw Horizontal Grid Lines In
	//
	tmp1 = (double) GridHeight / y_axis_divisions;

	for (j = 0; j < y_axis_divisions; j++)
	{
		INT yPos = Y_BOTTOM - zY_ADJUST * zRound(tmp1 * j) ;

		zDrawLine(X_LEFT, yPos, X_RIGHT, yPos);
	}

	//
	// Restore the Axis Pen Again
	//
#ifdef WINDOWS_PLATFORM
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

/////////////////////////////// zDraw3DGrid()  /////////////////////////////

void zGraph::zDraw3DGrid()
{
	//
	// Draws a 3-Dimensional Grid for the Graph
	//

	//
	// Calculate New Grid Dimensions
	//
	double grid_width  = (double) X_RIGHT - X_LEFT - z3D_GRID_X_SLANT_OFFSET;

	//
	// Change the Class Variables for the Adjusted Grid Height & Width
	//
	GridWidth  = zRound(grid_width);


	zActivatePens();

	//
	// Draw In the Outside Wire-Frame Box
	//
	zMoveTo(X_LEFT, Y_BOTTOM);

	// Bottom Edge
	zLineTo(X_RIGHT - zRound(z3D_GRID_X_SLANT_OFFSET), Y_BOTTOM);
	// Rt. Slant
	zLineTo(X_RIGHT, Y_BOTTOM - zY_ADJUST * zRound(z3D_GRID_Y_SLANT_OFFSET));
	// Rt. Back
	zLineTo(X_RIGHT, Y_TOP);
	// Top Back
	zLineTo(X_LEFT + zRound(z3D_GRID_X_SLANT_OFFSET), Y_TOP);
	// Left Slant
	zLineTo(X_LEFT, Y_TOP + zY_ADJUST * zRound(z3D_GRID_Y_SLANT_OFFSET));
	// Front Left
	zLineTo(X_LEFT, Y_BOTTOM);


	//
	// Return Now If We're Not Supposed to Draw in the Grid Lines
	//
	if (! zIsGridVisible())
		return;

#ifdef WINDOWS_PLATFORM
	// Select Grid Pen
	SelectObject(hDC, hGridPen);
#endif

#ifdef OS2_PLATFORM
	GpiSetLineType(hPS, OS2_GridLineType);
#endif

#ifdef MS_DOS_PLATFORM
	// Select the Grid Line Mask
	_setlinestyle(GridMask);
#endif

#ifdef BGI_DOS_PLATFORM
	// Select the Type of Lines for the Grid
	setlinestyle(bgiGridType, 0, NORM_WIDTH);
#endif

	//
	// Draw In the Individual Vertical Grid Segments
	//
	for (INT i = 0; i < y_axis_divisions; i++)
	{
		INT x_offset_1 = X_LEFT + zRound(grid_width * i / y_axis_divisions);
		INT x_offset_2 = x_offset_1 + zRound(z3D_GRID_X_SLANT_OFFSET);

		zMoveTo(x_offset_1, Y_BOTTOM);
		zLineTo(x_offset_2, Y_BOTTOM - zY_ADJUST * zRound(z3D_GRID_Y_SLANT_OFFSET));
	}

	//
	// Draw In the Individual Horizontal Grid Segments
	//
	for (INT i = 0; i < x_axis_divisions; i++)
	{
		INT x_offset_1 = X_LEFT + zRound(z3D_GRID_X_SLANT_OFFSET * i / x_axis_divisions);
		INT x_offset_2 = x_offset_1 + GridWidth;

		INT y_offset_1 = Y_BOTTOM - zY_ADJUST * zRound(z3D_GRID_Y_SLANT_OFFSET * i / x_axis_divisions);

		zMoveTo(x_offset_1, y_offset_1);
		zLineTo(x_offset_2, y_offset_1);
	}

	//
	// Draw In Any Other Line Segments Required to Finish Off the Grid
	//
	zMoveTo(X_RIGHT, Y_BOTTOM - zY_ADJUST * zRound(z3D_GRID_Y_SLANT_OFFSET));
	zLineTo(X_LEFT + zRound(z3D_GRID_X_SLANT_OFFSET),
		Y_BOTTOM - zY_ADJUST * zRound(z3D_GRID_Y_SLANT_OFFSET));
	zLineTo(X_LEFT + zRound(z3D_GRID_X_SLANT_OFFSET), Y_TOP);


	//
	// Restore Axis Pen Again
	//
#ifdef WINDOWS_PALTFORM
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

////////////////////////////// zScaleData()  /////////////////////////////

BOOL zGraph::zScaleData()
{
	//
	// Scales Values of X and Y coordinates to Fit a 2-D X/Y Style Graph.
	//      This is The Default Implementation, Used to Handle the Basic
	//      X/Y-Type Graph, With Linearly-Scaled Axes.   You Should Override
	//      This Function in Derived Classes, to Handle the Scaling
	//      Differently...
	//
	//      For Linear Scaling We Use the Linear Translation:
	//      xnew = [a] * x + [b],  ynew = [c] * y + [d], where the
	//      [a], [b], [c], and [d] Coefficients Are Determined So That:
	//
	//     (xmin, xmax) maps into the range (X_LEFT, X_RIGHT)
	//     (ymin, ymax) maps into the range (Y_BOTTOM, Y_TOP)
	//
	//  This Function Returns TRUE Normally, or FALSE if Error.
	//


	INT j;

	//
	// First We Size the Data, and Determine the Max. and Min.
	//   Values, if Appropriate
	//
	zSizeData();

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
		// Determine The Optimum Number of Axis Divisions to Use
		//   if User Didn't Specify Them.  Note:  [x_axis_divisions]
		//   and/or [y_axis_divisions] will be < 0 If User Wants
		//   Program to Best Fit the Number of Axis Divisions...
		//
		zGetBestFitAxisDivisions();

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
			xnew[ j ] = (INT) (a * Xdata[ j ] + b) ;

			//
			// Scale Y-Coordinate
			//
			ynew[ j ] = (INT) (c * Ydata[ j ] + d) ;
		}
	}


	return (TRUE);
}

//////////////////////////// zScale3DGraphData() ///////////////////////////

BOOL zGraph::zScale3DGraphData()
{
	//
	// Scales Data in the Ydata[] Array As Appropriate for a 3-D Graph.
	//
	return (zScaleYData(zRound(z3D_GRID_Y_SLANT_OFFSET)));
}

////////////////////////////// zScaleYData() /////////////////////////////

BOOL zGraph::zScaleYData(INT slant_offset /* = 0 */)
{
	//
	// Scales Data in the Ydata[] Array As Appropriate to Fit in a
	// Graph, By Scaling the Data Down From the Range: (ymin, ymax)
	// to the Range:
	//
	//    (0.0, Y_HEIGHT),
	//
	// where Y_HEIGHT is computed below.  The Scaled Values are Put Into
	// the YWorkData[] Array, for Use By Other Graph Functions.
	//
	// [slant_offset] is a default argument [default = 0], which is
	// zero for regular 2-D X/Y graphs, but non-zero for 3-D style graphs,
	// like the 3-D Bar and Area graphs
	//
	// This Function Returns TRUE normally or FALSE on Error
	//

	//
	// First We Size the Data, and Determine the Max. and Min.
	//   Values, if Appropriate
	//
	zSizeData();


	INT Y_HEIGHT = zRound(Y_BOTTOM - (Y_TOP + slant_offset));

	//
	// We'll Compute Slope and Intercept for Scaling Equation
	//
	if (ymax == ymin)
	{
		//
		// This Will Guard Against a Division By Zero Error In the
		//  Slope Equation Below
		//
		YCoordMultiplier = 0.0;
		YCoordOffset     = 0.5 * Y_HEIGHT;
	}
	else
	{
		//
		// The Normal Case, When [ymax] and [ymin] are Different...
		//

		// Determine The Optimum Number of Axis Divisions to Use
		//   if User Didn't Specify Them.  Note:  [x_axis_divisions]
		//   and/or [y_axis_divisions] will be < 0 If User Wants
		//   Program to Best Fit the Number of Axis Divisions...
		//
		zGetBestFitAxisDivisions();

		//
		// Now Calculate Slope and Intercept
		//
		double ymax_prime = Y_HEIGHT ;
		double ymin_prime = 0.0;

		YCoordMultiplier = (ymax_prime - ymin_prime) / (ymax - ymin);
		YCoordOffset     = ymin_prime - YCoordMultiplier * ymin;
	}

	//
	// Dynamically Allocate a Working Array to Hold the Scaled Data
	//
	YWorkData = new double[ n + 2 ];         // Create Temp. Work Array
	if (! YWorkData)
	{
		zDisplayError(zLOW_MEMORY_SCALE_Y);
		return (FALSE);
	}

	//
	// Now Scale the Data to Fit the Range
	//
	for (INT i = 0; i < n; i++)
		YWorkData[ i ] = YCoordMultiplier * Ydata[ i ] + YCoordOffset;


	return (TRUE);
}

//////////////////////////// zShowGraphTitles() ///////////////////////////

void zGraph::zShowGraphTitles()
{
	//
	// Displays the Graph Main Title, and Horizontal/Vertical Axis Titles.
	//

	INT TopMarginHt = abs(RWindow.top - Y_TOP);

	//
	// Show Main Title
	//
	RECT RMainTitle  =
	{
		X_LEFT        + zRound(0.15 * RGraphAxis.Width()),
		RWindow.top   + zRound(0.25 * TopMarginHt),
		X_RIGHT       - zRound(0.15 * RGraphAxis.Width()),
		Y_TOP         - zRound(0.25 * TopMarginHt)

	};

	zShowStringInRect(MainTitle, &RMainTitle);

	//
	// Quit Now if We're Not Displaying the X- and Y-Axis Titles
	//
	if (! show_axis_titles)
		return;

	//
	// Show X-Axis Title
	//
	INT XAxisTitlePos = (X_LEFT + ((_3D_flag != TRUE)
		? X_RIGHT : (X_RIGHT - zRound(z3D_GRID_X_SLANT_OFFSET)))) / 2
		- charsize * _tcslen(HorizTitle)/2;

	AxisTitleFont.zDrawTextString(
		XAxisTitlePos,
		Y_BOTTOM + zY_ADJUST * 3 * AxisTitleFont.zGetFontHeight() / 2,
		HorizTitle);

	//
	// Show Y-Axis Title
	//
	INT YAxisTitleHorizPos = X_LEFT - charsize * _tcslen(VertTitle)/2;
	INT YAxisTitleVertPos;
	if (_3D_flag)
	{
		double slope = z3D_GRID_Y_SLANT_OFFSET / z3D_GRID_X_SLANT_OFFSET;
		YAxisTitleVertPos = (Y_TOP + zRound(z3D_GRID_Y_SLANT_OFFSET)) -
			zRound(slope * (YAxisTitleHorizPos + (_tcslen(VertTitle) + 4)*charsize - X_LEFT)) -
			charheight;
	}
	else YAxisTitleVertPos = Y_TOP - zY_ADJUST * 2 * AxisTitleFont.zGetFontHeight();

	AxisTitleFont.zDrawTextString(YAxisTitleHorizPos, YAxisTitleVertPos, VertTitle);

}

////////////////////////// zGetBestFitAxisDivisions() //////////////////////

void zGraph::zGetBestFitAxisDivisions()
{
	//
	// This Function Determines an Optimum "Best-Fit" Number of Axis
	//   Divisions For a Graph's X- and/or Y-Axis.  The Number of
	//   X-Axis Divsions Is Determined Based Upon the [xmin]
	//   and [xmax] Values.  The Number of Y-Axis Divisions is
	//   Determined Based Upon the [ymin] and [ymax] Values for
	//   the Data Set.
	//
	//   NOTE:
	//       X-Axis Best-Fitting is Performed when:
	//           [x_axis_divisions] < 0
	//       Y-Axis Best-Fitting is Performed when
	//           [y_axis_divisions] < 0
	//
	//  Note That This Function Will Modify the Values of
	//  [x_axis_divisions], [xmin], [xmax] and/or
	//  [y_axis_divisions], [ymin], [ymax] as Appropriate.
	//

	if (x_axis_divisions < 0  &&  Xdata != NULL)
		zCalcTicks(xmin, xmax, x_axis_divisions);

	if (y_axis_divisions < 0  &&  Ydata != NULL)
		zCalcTicks(ymin, ymax, y_axis_divisions);

}

void zGraph::zCalcTicks(double &MinValue, double &MaxValue,
						INT &AxisDivisions)
{
	//
	// This Function Is Used In the Best-Fit Axis Algorithm
	//   [See "zGetBestFitAxisDivisions()" function].
	//   The Best-Fit Algorithm Used Here Was Suggested by
	//   Juergen Froehlich of Froehlich & Schade Gmbh, Deizisan,
	//   Germany.
	//

	double fMin = MinValue;
	double fMax = MaxValue;
	double tickabstand;

	//
	// If Min Value = Max Value, Just Use a Default Value
	//   for the Number of Axis Divisions--Don't Attempt
	//   to Use the Algorithm...
	//
	if (fabs(MaxValue - MinValue) < zDELTA)
	{
		AxisDivisions = zDEFAULT_AXIS_DIVISIONS;
		return ;
	}

	//
	// Calculate the number of ticks and the distance between the ticks
	//
	zFindBestFit(MinValue, MaxValue, AxisDivisions, tickabstand);

	//
	//  Rescale the limits for suitable values
	//
	MinValue = (double) floor(fMin / tickabstand) * tickabstand;
	MaxValue = (double) ceil (fMax / tickabstand) * tickabstand;

	//
	// With the rescaled limits, it is neccessary to recalc the
	//   number of ticks and the distance
	//
	zFindBestFit(MinValue, MaxValue, AxisDivisions, tickabstand);

	//
	// (The Following Code Can Be Removed If Desired,
	//   But We'd Prefer a Smaller Number of Axis-Divisions
	//   to a Larger Number, If Possible!)
	//
	if (AxisDivisions == 8  ||  AxisDivisions == 10)
		AxisDivisions /= 2;
}

void zGraph::zFindBestFit(double MinValue, double MaxValue,
						  INT &NumTicks, double &DistBetweenTicks)
{
	//
	// This Function Is Used In the Best-Fit Axis Algorithm
	//   [See "zGetBestFitAxisDivisions()" function].
	//   It Is Used to Find the Number of Ticks To Put
	//   Along a Graph Axis [NumTicks], And the Distance
	//   Between Ticks [DistBetweenTicks]
	//

	static INT iTickBase[ ] =
	{
		1, 2, 5
	};
	static INT NumTickBases = sizeof(iTickBase) / sizeof(iTickBase[ 0 ]);

	double dDiff = MaxValue - MinValue ;

	//
	//  Calculate Distance Between Two Ticks, Starting With 10 Ticks
	//
	DistBetweenTicks = dDiff / 10;

	//
	//  Round the distance to the next base power (10, 100, 1000 ...)
	//
	double fF = log10(DistBetweenTicks);
	double fFF = (double)pow(10.0, floor(fF));

	BOOL bFound = FALSE ;
	for (INT i = 0; i < NumTickBases; i++)
	{
		if (DistBetweenTicks <= iTickBase[ i ] * fFF)
		{
			DistBetweenTicks = iTickBase[ i ] * fFF ;
			bFound = TRUE ;
			break ;
		}
	}

	if (! bFound)
	{
		fF++;
		DistBetweenTicks = pow(10.0, floor(fF));
	}

	//
	// Get the Number of Ticks to Use...
	//
	NumTicks = zRound(dDiff / DistBetweenTicks) ;
}




//////////////////////////////////////////////////////////////////////////
//////////////////////// PLATFORM-SPECIFIC CODE //////////////////////////
//////////////////////////////////////////////////////////////////////////

////////////////////////// zInitGraph() ////////////////////////////

BOOL zGraph::zInitGraph()
{
	// This Function Initializes as Necessary for Making a Graph.
	//   This Function MUST Be Called Before Doing any Graph Drawing.
	//   Here We Validate a Number of Entities, and Establish Some
	//   Information about the Target Device Where the Graph Will Be
	//   Drawn.  Also We Set the Rectangles Where the Legend and
	//   the Graph Itself Eventually Will Be Displayed...
	//
	//   RETURNS: TRUE on Success, FALSE if an Error Is Encountered


	// Validate the HDC or HPS

#ifdef WINDOWS_PLATFORM
	if (! hDC)
	{
		zDisplayError(zHDC_INVALID);
		return FALSE;
	}
#endif

#ifdef OS2_PLATFORM
	if (! hPS)
	{
		zDisplayError(zHPS_INVALID);
		return FALSE;
	}
#endif


	// Validate the Window Rectangle
	if (! RWindow.IsValid())
	{
		zDisplayError(zWINDOW_RECT_INVALID);
		return FALSE;
	}

	zSaveGraphics();                            // Save Graphics Objects
	zGetDefaultTextSize();                      // Get Size of Default Text
	zFillBackground();                          // Fill In Graph Bkg. Color

	// Establish Graph and Legend Rectangle Areas
	zSetUpGraphLayout();

#ifdef WINDOWS_PLATFORM
	//
	//  We use TRANSPARENT mode because Windows has a "blank out"
	//  effect if you try to display too many points too close together using
	//  the default OPAQUE mode...
	//
	SetBkMode(hDC, TRANSPARENT);
#endif

	return TRUE;
}


////////////////////////////// zFillBackground() /////////////////////////////

void zGraph::zFillBackground()
{
	//
	//  Colors in the Graph Area of the Window [RWindow] with the Specified
	//     Background color [bkg_color]
	//

#ifdef WINDOWS_PLATFORM
	//
	// Create Brush to Be Used for Background Fill and Save the Old Brush
	//
	HBRUSH hBrush_bkg = CreateSolidBrush(BkgColor);
	HBRUSH hPrevBrush = (HBRUSH) SelectObject(hDC, hBrush_bkg);

	//
	// Fill in the Entire Graph Area with the Bkg. Color.
	//   If User Has Specified That Graph Is To Have a Single-Color Graded
	//   Spectrum For the Background, We Put It In...
	//
	RECT RWindow1 = RWindow;

	if (bkg_gradient_mode)
		zFillBkgWithGradedColor();
	else
		FillRect(hDC, &RWindow1, hBrush_bkg);

	// Set the Bkg. Color
	SetBkColor(hDC, BkgColor);

	//
	// Restore Old Brush and Delete Brush Used for Making Bkg Fill
	//
	SelectObject(hDC, hPrevBrush);
	DeleteObject(hBrush_bkg);
#endif


#ifdef OS2_PLATFORM
	//
	// Set the Fg. and Bkg. Color
	//
	GpiSetBackColor(hPS, BkgColor);
	GpiSetColor(hPS,     BkgColor);

	//
	// Force Overpaint Mode
	//
	GpiSetBackMix(hPS, BM_OVERPAINT);

	//
	// Define the Region to Be Colored In as the Whole Window Area
	//
	POINTL pt1 =
	{
		RWindow.left  ,  RWindow.top
	};
	POINTL pt2 =
	{
		RWindow.right ,  RWindow.bottom
	};

	if (bkg_gradient_mode)
		zFillBkgWithGradedColor();
	else
	{
		//
		// Use GpiBox() to Color In Interior, But Don't Show Border Outline
		//
		GpiMove(hPS, &pt1);
		GpiBox(hPS, DRO_OUTLINEFILL, &pt2, 0, 0);
	}

	//
	// Set the Background Mix So that Text Written Over It
	//   Won't Appear With the Wrong Bkg. Color...
	//
	GpiSetBackMix(hPS, BM_LEAVEALONE);
#endif


#ifdef MS_DOS_PLATFORM
	// Color In Background Area

	_setcolor((short)BkgColor);

	_rectangle(_GFILLINTERIOR, RWindow.left, RWindow.top,
		RWindow.right, RWindow.bottom);
#endif


#ifdef BGI_DOS_PLATFORM
	// Color In Background Area

	setcolor((int)BkgColor);
	setfillstyle(SOLID_FILL, BkgColor);

	// Don't Go Outside Screen Perimeter, Or We'll Die a Cruel Death!

	INT left   = max(1, RWindow.left);
	INT top    = max(1, RWindow.top);
	INT right  = min(getmaxx() - 1, RWindow.right);
	INT bottom = min(getmaxy() - 1, RWindow.bottom);

	bar3d(left, top, right, bottom, 0, 0);
#endif

}


//////////////////////////// zDrawLegend() //////////////////////////////

void zGraph::zDrawLegend(BOOL UseColorFills /* = TRUE */)
{
	//
	// Draws A Graph Legend Within the Rectangle [RLegend], showing
	//    [num_sets] separate items in the Legend.
	//
	//    NOTE:  The Graph Legend Is Drawn Either On the Left Side of
	//    the Graph Area, at the Bottom of the Graph Area, or Not at
	//    All.  It is Controlled by the [LegendPosition] Variable
	//
	//    The Legend Shows Rectangular Color Fills if the [UseColorFills]
	//    Default Argument Is Set TRUE; It Shows Lines if it is FALSE.
	//

	zRECT RLegendString[ zMAX_DATA_SETS ];  // Rectangle Where Each
	//  Legend ID Will Be Displayed

	//
	// Return Now if No Legend Is to Be Drawn
	//
	if (LegendPosition == zNO_LEGEND)
		return;


	POINT PtWindowCenter = RWindow.Center();

	//
	// Adjust the Top and Bottom of the Legend Box Area, Depending
	//   On How Many Data Sets there are...
	//
	double VertIDBoxSpacing  = 0.500 * zID_BOX_HT;
	double HorizIDBoxSpacing = 0.375 * zID_BOX_WIDTH;

	INT LegHeight = zRound(zID_BOX_HT * (1.5 * num_sets + 0.5));

	INT LegWidth  = zRound(num_sets * zID_BOX_WIDTH +
		(num_sets - 1) * HorizIDBoxSpacing);

	if (LegendPosition == zLEFT)
	{
		RLegend.top    = (PtWindowCenter.y - LegHeight / 2);
		RLegend.bottom = (PtWindowCenter.y + LegHeight / 2);
	}
	else
	{
		INT legend_half_width = 3 * charsize + LegWidth / 2;

		RLegend.left  = (PtWindowCenter.x - legend_half_width);
		RLegend.right = (PtWindowCenter.x + legend_half_width);
	}

	//
	// Calculate the Height and Width of the ID Box
	//
	INT IDBoxHeight = (LegendPosition == zLEFT)
		? zRound(zID_BOX_HT) : zRound (0.5 * RLegend.Height());
	INT IDBoxWidth  = (LegendPosition == zLEFT)
		? zRound(0.333 * RLegend.Width()) : zRound(zID_BOX_WIDTH);

	//
	// Center the ID Box and the Text to Its Right about the Center Line
	//  of the Legend Box
	//
	POINT ptCenter = RLegend.Center();

	INT x_IDBox = (LegendPosition == zLEFT) ?
		(ptCenter.x - charsize/2 - IDBoxWidth) :
	(ptCenter.x - LegWidth/2) ;

	INT x_text_start = (LegendPosition == zLEFT) ? ptCenter.x : x_IDBox;
	INT symbol_index = 0;

	RECT R1 = RLegend;
	zDrawRect(&R1);                 // Draw in Legend Box Border

	//
	// Set the Color of Numbers and Letters
	//
	zSelectAxisColor();

	//
	// Find the Longest Legend String, and Use This as a Template
	//
	WCHAR TemplateBuf[ 100 ];
	zGetLongestLegendString(TemplateBuf);

	//
	// Draw In the Legend ID Box/Line for Each Element...
	//
	for (INT index = 0; (index < zMAX_DATA_SETS)
		&& (index < num_sets);  index++)
	{
		//
		// Calc. this ID box's y-coord.
		//

		INT y_IDBox;

		if (LegendPosition == zLEFT)
		{
			//
			// Legend at Left
			//
			y_IDBox = RLegend.top
				+ zY_ADJUST * zRound(VertIDBoxSpacing) * (index + 1)
				+ zY_ADJUST * IDBoxHeight * (index);
		}
		else
		{
			//
			// Legend at Bottom
			//
			y_IDBox = ptCenter.y -
				zY_ADJUST * zRound(0.10 * RLegend.Height());

			if (index > 0)
				x_IDBox += zRound(HorizIDBoxSpacing + IDBoxWidth);
		}

		//
		// Use Brushes to Draw Rectangles if We're Making a Graph that Uses
		//   Fill Areas (e.g., Pie, Bar, Area), Otherwise Use Pens to Draw
		//   Lines for the Legend
		//
		if (UseColorFills)
		{
			zDraw3DBar(                             // Draw Border of Rect.
				x_IDBox, y_IDBox,
				x_IDBox + IDBoxWidth, y_IDBox + IDBoxHeight, 0,
				ColorPattern[ index ],

#ifdef WINDOWS_PLATFORM
				hFillBrush[ index ],
				hAxisPen
#endif

#ifdef OS2_PLATFORM
				GraphColor[ index ],
				AxisColor
#endif

#ifdef MS_DOS_PLATFORM
				GraphColor[ index ],
				AxisColor
#endif

#ifdef BGI_DOS_PLATFORM
				GraphColor[ index ],
				AxisColor
#endif

				);
		}
		else
		{
			// Select Appropriate Line Color
			zSelectGraphColor(index);

			//
			// Draw Legend Line In
			//
			INT yline = (LegendPosition == zLEFT) ?
				(y_IDBox + zY_ADJUST * IDBoxHeight / 2) :
			(y_IDBox + zY_ADJUST * IDBoxHeight / 4);

			zDrawLine(
				x_IDBox,
				yline,
				x_IDBox + IDBoxWidth,
				yline);

			//
			// Find Index of Next Symbol to Display
			//
			if (index > 0)
			{
				while (symbol_index < zMAX_DATA_POINTS  &&
					sym[ symbol_index ] == sym[ symbol_index + 1])
					symbol_index++ ;
				symbol_index++ ;
			}

			//
			// Draw In The Symbol that We Use at the Beginning and End
			//  of this Line, if We're Displaying Symbols
			//
			if (symbols_flag)
			{
				zDraw_Symbol(x_IDBox, yline - charsize/2,
					sym[ symbol_index ], index);

				zDraw_Symbol(x_IDBox + IDBoxWidth, yline - charsize/2,
					sym[ symbol_index ], index);
			}
		}

		//
		// Compute the Rectangle to Be Used for Displaying the ID Box Title
		//

		if (LegendPosition == zLEFT)
		{
			//
			// Legend at Left
			//
			RLegendString[ index ].left   =
				x_text_start + zRound(0.05 * RLegend.Width());
			RLegendString[ index ].top    =
				y_IDBox + zY_ADJUST * zRound(0.15 * IDBoxHeight);
			RLegendString[ index ].right  =
				RLegend.right - zRound(0.05 * RLegend.Width());
			RLegendString[ index ].bottom =
				y_IDBox + zY_ADJUST * zRound(0.85 * IDBoxHeight);
		}
		else
		{
			//
			// Legend at Bottom
			//
			RLegendString[ index ].left   = x_IDBox;
			RLegendString[ index ].top    =
				RLegend.top + zY_ADJUST * zRound(0.150 * RLegend.Height());
			RLegendString[ index ].right  = x_IDBox + IDBoxWidth;
			RLegendString[ index ].bottom =
				RLegend.top + zY_ADJUST * zRound(0.325 * RLegend.Height());
		}
	}

	// Now Draw In the Text For Each Legend String
	for (INT index = 0; (index < zMAX_DATA_SETS)
		&& (index < num_sets);  index++)
	{
		// Get This Legend Title
		_tcscpy(zbuf, &(LegendTitle[ index ]));


#ifdef OS2_PLATFORM
		zShowStringInRect(zbuf, RLegendString[ index ]);
#else   // For Windows, etc.

		// Pick an Appropriate Font On the First Pass
		if (index == 0)
			LegendTitleFont.zPickFont(zbuf, RLegendString[ index ]);

		// Show the Title Using the Selected Font
		if (LegendPosition == zLEFT)
			LegendTitleFont.zDrawTextString(RLegendString[ index ].left,
			RLegendString[ index ].top,
			zbuf);
		else
			// Legend at Bottom
			LegendTitleFont.zCenterText(zbuf, RLegendString[ index ]);
#endif
	}
}

///////////////////////////// zDrawDataPoints() ////////////////////////////

void zGraph::zDrawDataPoints()
{
	//
	//  Plots Data Points and/or the Lines Connecting the Data Points.
	//    This is THE GUTS of a Basic X/Y-Style Graph.  You'll Want to
	//    Override this Function in Derived Classes if You Plot Your
	//    Data Points in a Different Way...
	//

	//
	// Start With the First Pen Color
	//
	zSelectGraphColor(0);


#ifdef WINDOWS_PLATFORM
	//
	// Set the Clipping Region to Interior of the Rectangular Grid Area
	//
	if (! auto_scaling)
	{
		//
		// Save the Old Clipping Region
		//
		RECT RClip;
		GetClipBox(hDC, &RClip);

		hOldClipRgn = CreateRectRgn(RClip.left, RClip.top,
			RClip.right, RClip.bottom);

		//
		// Set New Clipping Region to Be the Interior of the
		//  Graph Area Rectangle.
		//
		hClipRgn    = CreateRectRgn(X_LEFT  - charsize / 2,
			Y_TOP    - charsize / 2,
			X_RIGHT  + charsize / 2,
			Y_BOTTOM + charsize / 2);


		SelectClipRgn(hDC, hClipRgn);
	}
#endif

#ifdef OS2_PLATFORM
	//
	// [Put Code to Set OS/2 Clipping Region Here...]
	//
#endif


	//
	// Now Draw in the Data Points
	//
	INT color_index = 0;
	for (INT j = 0; j < n; j++)          // For Each Data Point. . .
	{
		INT symbol = abs(sym[j]);         // This is the symbol to display

		//
		// Choose New Pen and Text Color For Each New Symbol Set
		//
		if (j > 0  &&  (symbol != abs(sym[j - 1])))
		{
			++color_index ;

			//
			// If a New Data Set, Switch to New Text Color
			//
			zSelectGraphColor(color_index);
		}

		//
		// Display symbol if we're supposed to...
		//
		if (symbols_flag)
			zDraw_Symbol(xnew[ j ], ynew[ j ], symbol, color_index);

		//
		// Try to Draw Lines if We Can Between the Points!
		//
		if (line_connect)
		{
			//
			// The First Point Doesn't Need a Connecting Line Drawn
			//
			if (j == 0  &&  sym[ j ] < 0)
			{
				zDraw_Symbol(xnew[j], ynew[j], symbol, color_index);
			}
			else if (j > 0)
			{
				//
				// Don't Draw Connecting Lines Between Points
				//   if the symbol is negative!
				//
				if (sym[ j ] < 0)
					zDraw_Symbol(xnew[ j ], ynew[ j ], symbol, color_index);
				else if (symbol == abs(sym[ j-1 ]))
					zDrawLine(xnew[ j ], ynew[ j ], xnew[ j - 1 ],
					ynew[ j - 1 ]);
			}
		}
	}

#ifdef WINDOWS_PLATFORM
	if (! auto_scaling)
	{
		//
		// Set Clipping Region Back to the Original Clipping Region Now
		//
		SelectClipRgn(hDC, hOldClipRgn);

		DeleteObject(hClipRgn);
		DeleteObject(hOldClipRgn);
	}
#endif

#ifdef OS2_PLATFORM
	//
	// [Put Code to Reset OS/2 Clipping Region Here...]
	//
#endif

	//
	// Save the Number of Data Sets Found in the Data--It Will Be
	//  Used in Making the Graph Legend!
	//
	num_sets = color_index + 1;
}


zPALETTE zGraph::zCreateGradedColorPalette()
{
	// Creates a Color Palette With a Smooth Gradient in Color Transitions.
	//    64 Distinct Color Shades are Used.  This Function Is Used to
	//    Fill in the Background With a "Dithered" Color Look.

#ifdef WINDOWS_PLATFORM

	//////////////////////////////////////////////////////////////////
	// Windows Code to Set Up Palette
	//////////////////////////////////////////////////////////////////

	LOGPALETTE *pPal;
	HPALETTE hLogPal;

	BYTE *ByteArray =  new BYTE[ sizeof(LOGPALETTE) +
		(zNUM_GRADED_PALETTE_ENTRIES - 1) * sizeof(PALETTEENTRY) ];
	// (Extra entry in LOGPAL struct)
	pPal = (LOGPALETTE *)ByteArray;
	if (! pPal)
		return (NULL);

	pPal->palVersion = 0x300;
	pPal->palNumEntries = zNUM_GRADED_PALETTE_ENTRIES;

	// Set Up the Color Array for Colors Within the Palette
	// Brighter shades are placed first for higher priority.

	BYTE color;
	INT  i;
	for (color = 255, i = 0;
		i < zNUM_GRADED_PALETTE_ENTRIES; color -= 4, i++)
	{
		// Set R,G,B, values for this Palette Entry
		zCalculateRGBValues(color,
			pPal->palPalEntry[i].peRed,
			pPal->palPalEntry[i].peGreen,
			pPal->palPalEntry[i].peBlue);

		pPal->palPalEntry[i].peFlags = 0;
	}

	// Create New Palette
	hLogPal = CreatePalette(pPal);

	// Free Allocated Memory
	delete [] ByteArray;

	// Select the New Palette
	SelectPalette(hDC, hLogPal, FALSE);
	RealizePalette(hDC);
	SelectObject(hDC, GetStockObject(NULL_PEN));


	// Return the Palette
	return ((zPALETTE) hLogPal);

#endif


#ifdef OS2_PLATFORM

	//////////////////////////////////////////////////////////////////
	// OS/2 Code to Set Up Palette
	//////////////////////////////////////////////////////////////////

	LONG  lColorTable[zNUM_GRADED_PALETTE_ENTRIES];
	ULONG cclr;
	BOOL  fPaletteCaps = TRUE;

	//  Create entry with shades of color if animation is supported,
	//    solid color otherwise.

	for (INT i = 0; i < zNUM_GRADED_PALETTE_ENTRIES; i++)
	{
		BYTE index = 222;     //  Use This Value if No Animation Support

		// INDEX = [zNUM_GRADED_PALETTE_ENTRIES] Points Evenly
		//         Spaced Between 1 AND 255

		if (fPaletteCaps)
			index = ((float) (i + 1) * 255.0) /
			(float) zNUM_GRADED_PALETTE_ENTRIES;

		// Get R,G,B, values for this Palette Entry
		BYTE R, G, B;
		zCalculateRGBValues(index, R, G, B);

		lColorTable[i] = PC_RESERVED * 16777216 +
			(LONG)(((LONG)R << 16) + ((LONG)G << 8) + (LONG)B);
	}

	// Now Create the Palette...

	HAB  hab; // Not Really Used!
	HPAL hPal = GpiCreatePalette(hab,
		0L, // Allows Dithering for Displays With <=16 Colors
		(ULONG) LCOLF_CONSECRGB,
		zNUM_GRADED_PALETTE_ENTRIES,
		(unsigned long *)&lColorTable[0]);

	if (hPal == NULLHANDLE  ||  hPal == GPI_ERROR)
	{
		// Error
		zDisplayError(_T("GpiCreatePalette Error"));
		// Use WinGetLastError(hab) to Get Error Details!
	}

	else if (GpiSelectPalette(hPS, hPal) == PAL_ERROR)
	{
		// Error
		zDisplayError(_T("GpiSelectPalette Error"));
		// Use WinGetLastError(hab) to Get Error Details!
	}

	else if (WinRealizePalette(hWnd, hPS, &cclr) == PAL_ERROR)
	{
		// Error
		zDisplayError(_T("WinRealizePalette Error"));
		// Use WinGetLastError(hab) to Get Error Details!
	}

	////////////////////////////////////////////////////////////////////
	// Noticed a Slight Problem:  It Seems Like Sometimes an Extra WM_PAINT
	//    Message Gets Sent When Realizing a New Palette, Forcing
	//    Graph to Redraw Too Many Times.  If You're a PM Expert and/or
	//    Know What's Causing this Problem, Please Let Me Know!   --JJ
	////////////////////////////////////////////////////////////////////

	return (zPALETTE)hPal;
#endif

#ifdef MS_DOS_PLATFORM
	// Operation Not Defined for this Platform
	return (zPALETTE)0;
#endif

#ifdef BGI_DOS_PLATFORM
	// Operation Not Defined for this Platform
	return (zPALETTE)0;
#endif

}

void zGraph:: zCalculateRGBValues(BYTE Intensity,
								  BYTE &Red, BYTE &Green, BYTE &Blue)
{
	// Sets Red, Green, and Blue Component Values For a Particular Background
	//   Color Scheme.  [Intensity] Is the Average Relative Intensity Desired
	//   NOTE:  Users May Wish to Extend this List, Adding Other Possible
	//          Color Combinations!


	// Start With All Components Turned Off
	Red = Blue = Green = 0;

	switch (bkg_gradient_color_scheme)
	{
		//
		// First, the Straight Hues (Variations of 1 Component Only)
		//
	case zBLUE_GRADIENT:                // Various Hues of Blue
		Blue = Intensity;
		break;

	case zGREEN_GRADIENT:               // Various Hues of Green
		Green = Intensity;
		break;

	case zCYAN_GRADIENT:                // Various Hues of Cyan
		Blue = Green = Intensity;
		break;

	case zRED_GRADIENT:                 // Various Hues of Red
		Red = Intensity;
		break;

	case zMAGENTA_GRADIENT:             // Various Hues of Magenta
		Red  = Blue = Intensity;
		break;

	case zYELLOW_GRADIENT:              // Various Hues of Yellow
		Red  = Green = Intensity;
		break;

		//
		// The Next Several "Rainbow" Hues Are Paths Around the
		//   Border of the Color Cube: Color = F(R,G,B)
		//
	case zRED_TO_MAGENTA_GRADIENT:      // Hues from Red to Magenta
		Red   = 0xFF;
		Blue  = Intensity;
		break;

	case zMAGENTA_TO_BLUE_GRADIENT:     // Hues from Magenta to Blue
		Blue  = 0xFF;
		Red   = Intensity;
		break;

	case zBLUE_TO_CYAN_GRADIENT:        // Hues from Blue to Cyan
		Blue  = 0xFF;
		Green = Intensity;
		break;

	case zCYAN_TO_GREEN_GRADIENT:       // Hues from Cyan to Green
		Green = 0xFF;
		Blue  = Intensity;
		break;

	case zGREEN_TO_YELLOW_GRADIENT:     // Hues from Green to Yellow
		Green = 0xFF;
		Red   = Intensity;
		break;

	case zYELLOW_TO_RED_GRADIENT:       // Hues from Yellow to Red
		Red    = 0xFF;
		Green  = Intensity;
		break;
		//
		// Other Variations
		//
	case zRED_TO_BLUE_GRADIENT:
		Red   = Intensity;
		Blue  = (unsigned char)(0xFF - Intensity);
		break;

	case zBLUE_TO_GREEN_GRADIENT:
		Blue  = Intensity;
		Green = (unsigned char)(0xFF - Intensity);
		break;

	case zGREEN_TO_RED_GRADIENT:
		Green = Intensity;
		Red   = (unsigned char)(0xFF - Intensity);
		break;

		//
		// The Default Gradient
		//
	case zWHITE_GRADIENT:               // Various Hues of White
	default             :
		Red = Blue = Green = Intensity;
		break;

	}

}


void zGraph::zFillBkgWithGradedColor()
{
	//
	//  Colors in the Background Area of the Window [RWindow] with a Smoothly-
	//    Graded Color Spectrum, Based Upon the Bkg Gradient Color Scheme
	//

#if defined(MS_DOS_PLATFORM) || defined(BGI_DOS_PLATFORM)
	// Operation Not Yet Defined For These Platforms
	return;
#else
	// Create an Appropriate Palette, Based Upon the RGB
	//   Components Appearing in the Bkg. Gradient Color Scheme

	zPALETTE hNewPal = zCreateGradedColorPalette();

	if (! hNewPal)
	{
		zDisplayError(zCANNOT_CREATE_PALETTE);
		return;
	}


	// Compute the "Band Size" for
	//   [zNUM_GRADED_PALETTE_ENTRIES] Separate Color Steps

	double yStep = (double)(RWindow.Height()) / zNUM_GRADED_PALETTE_ENTRIES;
	double xStep = (double)(RWindow.Width())  / zNUM_GRADED_PALETTE_ENTRIES;


	// Select Starting Color

	BYTE color;
	if (bkg_gradient_mode == zINTO_CENTER)
		color = 0;
	else
		color = 255;


	// For Each Color...

	for (INT i = 0; i < zNUM_GRADED_PALETTE_ENTRIES; i++)
	{
		// Get the Red, Green, Blue Percentages for This Color...

		BYTE Red, Green, Blue;

#ifdef WINDOWS_PLATFORM
		// Calculate the R, G, B  Component Values for This Color
		zCalculateRGBValues(color, Red, Green, Blue);
#endif

#ifdef OS2_PLATFORM
		// NOTE:  Because We Assign Colors Using Consecutive RGB
		//        Values, Just the LSB [Least Significant Byte] of the
		//        32-Bit LONG Color Word Is Used.  Here The Blue
		//        Component Is the LSB, So It LOOKS Like We're Only
		//        Changing Blue, With No Red or Green Components.  THIS
		//        IS NOT THE CASE!  In Fact The Palette Has Already Been
		//        Set Up and Realized At This Point, and We're Really
		//        Only Varying an Index.
		Red   = 0;
		Green = 0;
		Blue  = i;
#endif

		// Draw One Color Band...

		switch (bkg_gradient_mode)
		{
		case zINTO_CENTER:       // Draw Gradient From Outside of Screen Into Center
		case zCENTER_OUT:        // Draw Gradient From Center of Screen Outward
			{
				INT xCenter = RWindow.left + RWindow.Width()  / 2;
				INT yCenter = RWindow.top  + RWindow.Height() / 2;

				INT x1 = zRound(xStep *    i / 2.0);
				INT x2 = zRound(xStep * (i+1)/ 2.0);
				INT y1 = zRound(yStep *    i / 2.0);
				INT y2 = zRound(yStep * (i+1)/ 2.0);

				// Set Up Four Rectangles for Each Stretch of the Narrow
				//   Rectangular Band

				RECT RLeft =
				{
					xCenter - x2 - 1,
					yCenter - y2 - 1,
					xCenter - x1 + 1,
					yCenter + y2 + 1
				};

				RECT RTop =
				{
					xCenter - x2 - 1,
					yCenter - y2 - 1,
					xCenter + x2 + 1,
					yCenter - y1 + 1
				};

				RECT RRight =
				{
					xCenter + x1 - 1,
					yCenter - y2 - 1,
					xCenter + x2 + 1,
					yCenter + y2 + 1
				};

				RECT RBottom =
				{
					xCenter - x2 - 1,
					yCenter + y1 - 1,
					xCenter + x2 + 1,
					yCenter + y2 + 1
				};

				// Draw In this band

				zDrawFilledRect(RLeft.left, RLeft.top, RLeft.right,
					RLeft.bottom, Red, Green, Blue);
				zDrawFilledRect(RTop.left, RTop.top, RTop.right,
					RTop.bottom, Red, Green, Blue);
				zDrawFilledRect(RRight.left, RRight.top, RRight.right,
					RRight.bottom, Red, Green, Blue);
				zDrawFilledRect(RBottom.left, RBottom.top, RBottom.right,
					RBottom.bottom, Red, Green, Blue);

				color = (unsigned char)(color + ((bkg_gradient_mode == zINTO_CENTER) ? 4 : -4));
			}
			break;


		case zBOTTOM_UP:        // Draw Gradient From Bottom of Screen Up
			{
				INT yTop    = RWindow.bottom - zRound(yStep * (i + 1)) - 1;
				INT yBottom = RWindow.bottom - zRound(yStep * i)       + 1;

				yTop    = max(RWindow.top,    yTop);
				yTop    = min(RWindow.bottom, yTop);
				yBottom = max(RWindow.top,    yBottom);
				yBottom = min(RWindow.bottom, yBottom);

				zDrawFilledRect(
					RWindow.left,
					yTop,
					RWindow.right,
					yBottom,
					Red, Green, Blue);

				color -= 4;
			}
			break;

		case zTOP_DOWN:         // Draw Gradient From Top of Screen Downwards
		default:
			{
				INT yTop    = RWindow.top + zRound(yStep * i)     - 1;
				INT yBottom = RWindow.top + zRound(yStep * (i+1)) + 1;

				yTop    = max(RWindow.top,    yTop);
				yTop    = min(RWindow.bottom, yTop);
				yBottom = max(RWindow.top,    yBottom);
				yBottom = min(RWindow.bottom, yBottom);

				zDrawFilledRect(
					RWindow.left,
					yTop,
					RWindow.right,
					yBottom,
					Red, Green, Blue);

				color -= 4;
			}
			break;
		}
	}

#ifdef WINDOWS_PLATFORM
	// Switch Back to Regular Palette
	SelectPalette(hDC, (HPALETTE)GetStockObject(DEFAULT_PALETTE), TRUE);
	RealizePalette(hDC);
	DeleteObject(hNewPal);
#endif

#ifdef OS2_PLATFORM
	// Restore Old Palette
	GpiSelectPalette(hPS, NULLHANDLE);
	GpiDeletePalette(hNewPal);

	// We Want to Use 32-Bit RGB Color Index Values Again
	GpiCreateLogColorTable(hPS, LCOL_RESET, LCOLF_RGB, 0L, 0L, NULL);
#endif
#endif   // if defined(MS_DOS_PLATFORM ...
}


// EOF -- ZGRAPH.CPP

