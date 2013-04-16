//***************************************************************************
//* sa_p_wavelet.cpp (v1.0)                                                 *
//* ----------------                                                        *
//* Project          : Speech Analyzer                                      *
//* Author           : Andy Heitke                                          *
//* Date Started     : 6/11/01                                              *
//* Customer Name    : JAARS / SIL                                          *
//* Description      : This is the main implementation file for the Wavelet *
//*                    process in SA                                        *
//* Revision History : 7/30/01 ARH - Imported file into Speech Analyzer     *
//*                                  v2.0 project                           *
//***************************************************************************
#include "stdafx.h"
#include "Process.h"
#include "isa_doc.h"

#include "sa_p_wavelet.h"

#include "resource.h"
#include "math.h"
#include "dsp\dspTypes.h"

#pragma warning(push, 3)
#pragma warning(disable : 4284)
#pragma warning(disable : 4146)
#include <stack>
#include <deque>
#pragma warning(pop)

// S T A T I C  V A R I A B L E S
//**************************************************************************
static double h_coeff4[] = {0.2303778133088964,
                            0.7148465705529154,
                            0.6308807679298587,
                            -.0279837694168599,
                            -.1870348117190931,
                            0.0308413818355607,
                            0.0328830116668852,
                            -.0105974017850690
                           };

static double g_coeff4[] = {-.0105974017850690,
                            -.0328830116668852,
                            0.0308413818355607,
                            0.1870348117190931,
                            -.0279837694168599,
                            -.6308807679298587,
                            0.7148465705529154,
                            0.2303778133088964
                           };

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

using namespace std;

CProcessWavelet::CProcessWavelet()
{
}

CProcessWavelet::~CProcessWavelet()
{
}

// Process
//*  Description   :
//*  Parameters    :
//*  Preconditions : None
//*  Postcondtions : None
//*  Returns       :
//**************************************************************************
long CProcessWavelet::Process(void * pCaller, ISaDoc * , int nWidth, int /*nHeight*/,
                              int nProgress, int nLevel)
{
    UNUSED_ALWAYS(nWidth);

    //TRACE(_T("Process: CProcessWavelet\n"));


    //**************************************************************************
    // Basic Error Checking
    //**************************************************************************

    // check canceled
    if (data_status & PROCESS_CANCEL)
    {
        return MAKELONG(PROCESS_CANCELED, nProgress);    // process canceled
    }

    // check if data ready
    if (data_status & DATA_READY)
    {
        return MAKELONG(--nLevel, nProgress);
    }

    //**************************************************************************
    // Compute
    //**************************************************************************

    // wait cursor
    BeginWaitCursor();

    if (!StartProcess(pCaller, IDS_STATTXT_PROCESSWVL))   // memory allocation failed
    {
        EndProcess(); // end data processing
        EndWaitCursor();
        return MAKELONG(PROCESS_ERROR, nProgress);
    }

    nProgress = nProgress + (int)(100 / nLevel); // calculate the actual progress
    EndProcess((nProgress >= 95)); // end data processing
    EndWaitCursor();

    // close the temporary file and read the status

    data_status = TRUE;
    return MAKELONG(nLevel, nProgress);
}

//**************************************************************************
// W A V E L E T  N O D E  C L A S S
//**************************************************************************
CWaveletNode::CWaveletNode()
{

    parent_node = NULL;
    left_node = NULL;
    right_node = NULL;

    data = NULL;
    dwDataSize = 0;

    upper_freq = -1;
    lower_freq = -1;

}

// Deconstructor
//*  Description   : Deletes the node from memory
//*  Parameters    : None
//*  Preconditions : None
//*  Postcondtions : None
//*  Returns       : None
//**************************************************************************
CWaveletNode::~CWaveletNode()
{
    if (left_node != NULL)
    {
        delete left_node;
    }
    if (right_node != NULL)
    {
        delete right_node;
    }
}

// CreateTree
//*  Description   : Builds a tree based on the tree definition passed in.
//*                                  uses a typical stack algorithm to do this, where an "L"
//*                                  creates a leaf and an "S" joins the next two things on the stack
//*  Parameters    : char *tree_definition - a char string containing the directions for the tree
//*                                CWaveletNode **root_node - a pointer to a pointer.  Tells me where to store the tree
//*  Preconditions : None
//*  Postcondtions : *root_node contains the tree
//*  Returns       : TRUE - if the tree was built successfully
//*                                  FALSE - bad tree definition, out of memory
//**************************************************************************
BOOL CreateTree(char * tree_definition, CWaveletNode ** root_node)
{

    stack<CWaveletNode *> tempStack;
    unsigned long str_index;

    CWaveletNode * tempNode;

    // Parse the string
    for (str_index = 0; str_index < strlen(tree_definition); str_index++)
    {
        switch (tree_definition[str_index])
        {
        case 'l': // Create a leaf
        case 'L':
            tempNode = new CWaveletNode();
            tempStack.push(tempNode);
            break;

        case 's':   // Join two nodes
        case 'S':
            tempNode = new CWaveletNode();

            tempNode->SetLeftNode(tempStack.top());
            tempStack.pop();

            tempNode->SetRightNode(tempStack.top());
            tempStack.pop();

            tempStack.push(tempNode);
            break;

        default:
            // delete any memory allocated on the stack
            while (!tempStack.empty())
            {
                tempNode = tempStack.top();
                delete tempNode;
                tempStack.pop();
            }

            return FALSE;
            break;
        }
    }

    *root_node = tempStack.top();
    tempStack.pop();

    // The stack should be empty at the end
    if (tempStack.empty())
    {
        return TRUE;
    }
    else
    {
        while (!tempStack.empty())
        {
            tempNode = tempStack.top();
            delete tempNode;
            tempStack.pop();
        }

        return FALSE;
    }

}

// DoMRAAnalysis
//*  Description   : A Recursive algorithm that follows the tree and creates wavelet data
//*                                  for each node.  The data is split at each branch
//*  Parameters    : None
//*  Preconditions : The tree structure should be generated using "CreateTree"
//*  Postcondtions : Each node contains wavelet coefficients with increasing frequency resolution
//*                                  as you travel down the tree
//*  Returns       : TRUE on sucess
//*                  FALSE on error
//**************************************************************************
BOOL CWaveletNode::DoMRAAnalysisTree()          // this is a (candy) Wrapper function
{
    return _DoMRAAnalysisTree(1);
}

BOOL CWaveletNode::_DoMRAAnalysisTree(long stride)
{

    // Compute the new frequencies
    double mid = lower_freq + ((upper_freq - lower_freq) / 2);

    if ((left_node != NULL) && (right_node != NULL))        // both leaves need to be split futher
    {
        // Allocate the memory of the leaves

        if (!left_node->SetDataNode(NULL, dwDataSize, lower_freq, mid))
        {
            return FALSE;
        }
        if (!right_node->SetDataNode(NULL, dwDataSize, mid, upper_freq))
        {
            return FALSE;
        }

        // Store basic leaf information
        left_node->SetParentNode(this);
        right_node->SetParentNode(this);


        // do the transform, storing the high band in the left node, and the low band
        // in the right
        if (!WaveletTransformNode(left_node->GetDataPtr(),
                                  right_node->GetDataPtr(),
                                  DEBAUCHES4,
                                  stride))
        {
            return FALSE;
        }


        // Next time we will have two more signals per band (because we don't downsample, 2X extra info is generated)
        stride *= 2;
        if (!left_node->_DoMRAAnalysisTree(stride))
        {
            return FALSE;
        }
        if (!right_node->_DoMRAAnalysisTree(stride))
        {
            return FALSE;
        }

        return TRUE;

    }

    // The tree will usually be balanced so the above code will usually be called, but just in
    // case someone wants only half of the data, the following two blocks are used


    // the left leaf needs splitting
    if ((left_node != NULL) && (right_node == NULL))
    {
        // Allocated the memory of the leaf
        if (!left_node->SetDataNode(NULL, dwDataSize, lower_freq, mid))
        {
            return FALSE;
        }

        // Store basic leaf information
        left_node->SetParentNode(this);

        // Do the transform, storing the low band and throwing away the high band
        if (!WaveletTransformNode(left_node->GetDataPtr(),
                                  NULL,
                                  DEBAUCHES4,
                                  stride))
        {
            return FALSE;
        }

        stride *= 2;
        if (!left_node->_DoMRAAnalysisTree(stride))
        {
            return FALSE;
        }
    }




    // the right leaf needs splitting
    if ((left_node == NULL) && (right_node != NULL))
    {
        // Allocated the memory of the leaf
        if (!right_node->SetDataNode(NULL, dwDataSize, mid, upper_freq))
        {
            return FALSE;
        }

        // Store basic leaf information
        right_node->SetParentNode(this);


        // Do the transform, storing the high band and throwing away the low band
        if (!WaveletTransformNode(NULL,
                                  right_node->GetDataPtr(),
                                  DEBAUCHES4,
                                  stride))
        {
            return FALSE;
        }


        stride *= 2;
        if (!right_node->_DoMRAAnalysisTree(stride))
        {
            return FALSE;
        }
    }



    // We are done
    if ((left_node == NULL) && (right_node == NULL))
    {
        return TRUE;
    }



    return FALSE;

}

// WaveletTransform
//*  Description   : Splits the data using a wavelet transform into a high band
//*                  and low band.  No decimation here, so we have redundant data
//*  Parameters    : double *pFinalLow (out) - place to put the low band coefficients
//*                                  double *pFinalHigh (out) - place to put the high band coeffients
//*                                  long wavelet_type - only Daubieches 4 is implemented use DEBAUCHES4
//*                                  int stride - how much the signals are seperated / how many signals there are
//*                                                             (because of not downsampling)
//*  Preconditions : pFinalLow, pFinalHigh - contain enough allocated memory to store results
//*  Postcondtions : pFinalLow and pFinalHigh contain the transformed data
//*  Returns       : TRUE on sucess
//*                                  FALSE otherwise
//**************************************************************************
BOOL CWaveletNode::WaveletTransformNode(long * pFinalLow,
                                        long * pFinalHigh,
                                        long wavelet_type,
                                        long stride)
{
    long data_index;
    long coefficient_index;
    long filter_index;

    double datapoint;               // temp for getting each datapoint

    ASSERT(data != NULL);
    ASSERT(pFinalLow != NULL);
    ASSERT(pFinalHigh != NULL);

    switch (wavelet_type)
    {
    case DEBAUCHES4:
        // zero the arrays
        for (data_index = 0; data_index < dwDataSize; data_index++)
        {
            pFinalLow[data_index] = 0;
            pFinalHigh[data_index] = 0;
        }


        // Do the high pass
        if (pFinalHigh != NULL)
        {

            // Transform each datapoint
            filter_index = 0;
            for (data_index = 0; data_index < dwDataSize; data_index++)
            {

                // Do the convolution
                coefficient_index = 0;
                for (filter_index = (-4 * stride); filter_index < (4 * stride); filter_index+=stride)
                {
                    coefficient_index++;

                    // Check for wrap around to the left
                    if ((data_index + filter_index) < 0)
                    {
                        datapoint = data[dwDataSize + data_index + filter_index];
                    }

                    // Check for wrap around to the right
                    else if ((data_index + filter_index) > dwDataSize)
                    {
                        datapoint = data[dwDataSize - data_index + filter_index];
                    }

                    // Normal case
                    else
                    {
                        datapoint = data[data_index + filter_index];
                    }

                    // Normal convolution formula: final[x] = transform[y] * data[x + y]
                    (pFinalHigh)[data_index] += long(g_coeff4[coefficient_index] * datapoint);
                }
            }
        }



        // Do the low pass
        if (pFinalLow != NULL)
        {

            // Transform each datapoint
            filter_index = 0;
            for (data_index = 0; data_index < dwDataSize; data_index++)
            {

                // Do the convolution
                coefficient_index = 0;
                for (filter_index = (-4 * stride); filter_index < (4 * stride); filter_index+=stride)
                {
                    coefficient_index++;

                    // Check for wrap around to the left
                    if ((data_index + filter_index) < 0)
                    {
                        datapoint = data[dwDataSize + data_index + filter_index];
                    }

                    // Check for wrap around to the right
                    else if ((data_index + filter_index) > dwDataSize)
                    {
                        datapoint = data[dwDataSize - data_index + filter_index];
                    }

                    // Normal case
                    else
                    {
                        datapoint = data[data_index + filter_index];
                    }

                    // Normal convolution formula: final[x] = transform[y] * data[x + y]
                    (pFinalLow)[data_index] += long(h_coeff4[coefficient_index] * datapoint);
                }
            }
        }
        return TRUE;

    default:
        return FALSE;
    }
    return FALSE;
}

//**************************************************************************
// T R A N S F O R M  R O U T I N E S
//**************************************************************************
// TransformEnergyTree
//*  Description   : Calls TransformEnergy for each node in the tree
//*  Parameters    : None
//*  Preconditions : The tree should be created and populated using DoMRAAnalysisTree
//*  Postcondtions : Calls TransformEnergy for each node in the tree
//*  Returns       : TRUE
//**************************************************************************
BOOL CWaveletNode::TransformEnergyTree()
{
    CWaveletNode * left_node;
    CWaveletNode * right_node;

    left_node = GetLeftNode();
    right_node = GetRightNode();

    // do transform
    TransformEnergyNode();

    if (left_node != NULL)
    {
        left_node->TransformEnergyTree();
    }
    if (right_node != NULL)
    {
        right_node->TransformEnergyTree();
    }

    return TRUE;
}

// TransformEnergyNode
//*  Description   : Squares all the datapoints to obtain unsigned energy information
//*  Parameters    : None
//*  Preconditions : data should be allocated and populated
//*  Postcondtions : each datapoint is squared
//*  Returns       : TRUE
//**************************************************************************
BOOL CWaveletNode::TransformEnergyNode()
{
    long data_index;

    // do transform
    for (data_index = 0; data_index < dwDataSize; data_index++)
    {
        data[data_index] *= data[data_index];
    }
    return TRUE;
}

// TransformLogScalingTree
//*  Description   : Calls TransformLogScaling for each node in the tree
//*  Parameters    : double high - what value the function should use as the max for the log scalling
//*  Preconditions : The tree should be created and populated using DoMRAAnalysisTree
//*  Postcondtions : Calls TransformLogScaling for each node in the tree
//*  Returns       : TRUE
//**************************************************************************
BOOL CWaveletNode::TransformLogScalingTree(double high)
{
    CWaveletNode * left_node;
    CWaveletNode * right_node;

    left_node = GetLeftNode();
    right_node = GetRightNode();

    // do transform
    TransformLogScalingNode(high);

    if (left_node != NULL)
    {
        left_node->TransformLogScalingTree(high);
    }
    if (right_node != NULL)
    {
        right_node->TransformLogScalingTree(high);
    }

    return TRUE;
}

// TransformLogScalingNode
//*  Description   : Does a log scaling
//*  Parameters    : double high - what is the maximum value to be used for scaling
//*  Preconditions : data should be allocated and populated
//*  Postcondtions : Data is scaled logarithmically
//*  Returns       : TRUE
//**************************************************************************
BOOL CWaveletNode::TransformLogScalingNode(double high)
{
    long data_index;


    // do log scaling
    for (data_index = 0; data_index < dwDataSize; data_index++)
    {
        if (data[data_index] < (.0001 * (double)high))
        {
            data[data_index] = long(.0001 * (double)high);
        }
        data[data_index] = long(-log(data[data_index] / high));
    }
    return TRUE;
}

// TransformSmoothingTree
//*  Description   : Calls TransformSmoothingTree for each node in the tree
//*  Parameters    : None
//*  Preconditions : data should be allocated and populated
//*  Postcondtions : Calls TransformSmoothingTree for each node in the tree
//*  Returns       : TRUE
//**************************************************************************
BOOL CWaveletNode::TransformSmoothingTree()
{
    CWaveletNode * left_node;
    CWaveletNode * right_node;

    left_node = GetLeftNode();
    right_node = GetRightNode();

    // do transform
    TransformSmoothingNode();

    if (left_node != NULL)
    {
        left_node->TransformSmoothingTree();
    }
    if (right_node != NULL)
    {
        right_node->TransformSmoothingTree();
    }

    return TRUE;
}

// TransformSmoothing
//*  Description   : Does a smoothing on the data using a convolution
//*  Parameters    : None
//*  Preconditions : data should be allocated and populated
//*  Postcondtions : Data is smoothed
//*  Returns       : TRUE
//**************************************************************************
BOOL CWaveletNode::TransformSmoothingNode()
{

    long num_coeff = 26;
    double smooth[26];

    long filter_index;
    long data_index;
    long coefficient_index;

    double datapoint;

    for (long x = 0; x < num_coeff; x++)
    {
        smooth[x] = 1.0/double(num_coeff);
    }

    // Transform each datapoint
    filter_index = 0;
    for (data_index = 0; data_index < dwDataSize; data_index++)
    {

        // Do the convolution
        coefficient_index = 0;
        for (filter_index = (-num_coeff / 2); filter_index < (num_coeff / 2); filter_index++)
        {
            coefficient_index++;

            // Check for wrap around to the left
            if ((data_index + filter_index) < 0)
            {
                datapoint = 0;
            }

            // Check for wrap around to the right
            else if ((data_index + filter_index) > dwDataSize)
            {
                datapoint = 0;
            }

            // Normal case
            else
            {
                datapoint = data[data_index + filter_index];
            }

            // Normal convolution formula: final[x] = transform[y] * data[x + y]
            data[data_index] += long(smooth[coefficient_index] * datapoint);

        }
    }
    return TRUE;
}

// TransformFitWindowNode
//*  Description   : Scales the data's magnitude so it fits into the window
//*  Parameters    : CRect *rWnd - the rectangle that the data should fitinto
//*  Preconditions : data should be allocated and populated
//*  Postcondtions :
//*  Returns       : TRUE - on sucess
//*                                  FALSE - on error
//**************************************************************************

BOOL CWaveletNode::TransformFitWindowNode(CRect * rWnd)
{
    long data_index;
    double max;
    double fVScale;

    max = GetMaxNode();

    // Scale our data to fit within our width
    fVScale = rWnd->Height() / max;


    // do transform
    for (data_index = 0; data_index < dwDataSize; data_index++)
    {
        data[data_index] *= long(fVScale);                              // step through the data
        data[data_index] += (rWnd->Height() / 2);       // Center in the window
    }


    return TRUE;
}

//**************************************************************************
// D R A W I N G  R O U T I N E S
//**************************************************************************
// DrawAllColorBands
//*  Description   : A recursive algorithm to draw a color band for the entire tree
//*  Parameters    : unsigned char *pBits - Address of memory to draw on
//*  Preconditions : Data should be populated to all leaves, pBits should be a valid
//*                                  drawing surface
//*  Postcondtions : Wavelet transform is drawn to the screen
//*  Returns       : TRUE - on sucess
//*                                  FALSE - on error
//**************************************************************************
BOOL CWaveletNode::DrawColorBandTree(unsigned char * pBits,     // this is a (candy) Wrapper function
                                     CRect * rWnd,
                                     double high,
                                     double start,
                                     double end)
{
    long num_leaves;
    long thickness;


    num_leaves = GetNumLeaves();
    thickness = long(floor((double)rWnd->Height() / (double)num_leaves));           // Compute the thickness so we use all screeb space

    _DrawColorBandTree(pBits, rWnd, thickness, 0, high, start, end);

    return TRUE;

}
long CWaveletNode::_DrawColorBandTree(unsigned char * pBits,
                                      CRect * rWnd,
                                      long thickness,
                                      long y,
                                      double high,
                                      double start,
                                      double end)
{
    // if the left node is a leaf, draw it!
    if (left_node->IsLeaf())
    {
        left_node->DrawColorBandNode(pBits, rWnd, thickness, y, high, start, end);
        y += thickness;
    }
    else     // Otherwise, beautiful, beautiful recursion
    {
        y = left_node->_DrawColorBandTree(pBits, rWnd, thickness, y, high, start, end);
    }

    // if the right node is a leaf, draw it!
    if (right_node->IsLeaf())
    {
        right_node->DrawColorBandNode(pBits, rWnd, thickness, y, high, start, end);
        y += thickness;
    }
    else     // Otherwise, beautiful, beautiful recursion
    {
        y = right_node->_DrawColorBandTree(pBits, rWnd, thickness, y, high, start, end);
    }

    return y;
}

// DrawColorBand
//*  Description   : This function takes a data set, fits it into our palette,
//*                  and then draws the data according to the thickness and y_start
//*  Parameters    : unsigned char *pBits - pointer to drawing area
//*                                  CRect *rWnd - the window that we are drawing into
//*                                  thickness - thickness in pixels (how many lines to draw)
//*                                  y_start - starting y location
//*  Preconditions : pBits must point to a bitmap of suffiencient size, and
//*                                  contain an init'ed palette
//*  Postcondtions : Data is drawn to the screen
//*  Returns       : true on sucess
//*                                  false otherwise
//**************************************************************************
BOOL CWaveletNode::DrawColorBandNode(unsigned char * pBits, CRect * rWnd, long thickness, long y_start, double high, double start, double end)
{
    double ColorScale, datapoint;
    double fHScale;

    long num_colors = 234;      // Todo: get the actual number from somewhere
    long final_color;

    double data_index;


    ASSERT(pBits != NULL);


    ColorScale = (num_colors / high);
    fHScale = (double)((double)(end - start)/ (double)rWnd->Width()); // Scale our data to fit our width


    // Draw
    for (long y = y_start; y < (y_start + thickness); y++)      // Draw "thickness" num of lines
    {
        data_index = start;

        for (long x = 0; x < rWnd->Width(); x++)
        {
            datapoint = data[(long)floor(data_index)];
            data_index += fHScale;
            if (data_index > dwDataSize)
            {
                break;
            }

            // FIX THIS!!!! we can't have the mod 4 here!!!
            final_color  = (long)((double)datapoint * ColorScale);
            pBits[y * (rWnd->Width() + (4 - (rWnd->Width() % 4))) + x] = unsigned char(num_colors - final_color);
        }
    }
    return TRUE;
}

// ScatterPlotTree
//*  Description   : Does not scatter plot the entire graph, but rather does
//*                                  a scatter plot of the xth leaf determined by "which_leaf"
//*  Parameters    : CDC *pDC - pointer to the device context to draw on
//*                                  CRect *rWnd - the rectangle to draw into
//*                                  COLORREF crColor - What color to draw with
//*                                  long which_leaf - how many leafs to skip before drawing
//*                                  double start - where to start in the data
//*                                  double end - where to end in the data
//*  Preconditions : Data should be allocated and populated to all leaves
//*  Postcondtions : ScatterPlotDataNode is called on the correct node
//*  Returns       : TRUE - on success
//*                                  FALSE - otherwise
//**************************************************************************
BOOL CWaveletNode::ScatterPlotDataTree(CDC * pDC,
                                       CRect * rWnd,
                                       COLORREF crColor,
                                       long which_leaf,
                                       double start, double end)
{
    CWaveletNode * node_ptr;
    node_ptr = GetNode(which_leaf);
    if (node_ptr == NULL)
    {
        return FALSE;
    }
    return node_ptr->ScatterPlotDataNode(pDC, rWnd, crColor, start, end);
}

// ScatterPlotDataNode
//*  Description   : Draw a simple scatter plot of node data
//*  Parameters    : CDC *pDC - where to draw the data
//*                                CRect *rWnd - The rectangle to draw within
//*                                COLORREF crColor - color to use in plotting
//*                                  double start - where to start in the data
//*                                  double end - where to end in the data
//*  Preconditions : Data should be allocated and populated
//*  Postcondtions : Data is plotted to the DC
//*  Returns       : TRUE
//**************************************************************************
BOOL CWaveletNode::ScatterPlotDataNode(CDC * pDC, CRect * rWnd, COLORREF crColor, double start, double end)
{

    // Scaling variables
    double fHScale;

    double data_index;
    long datapoint;


    // temp
    struct tagPOINT point_coords;

    // Get the color
    CPen penPlot(PS_SOLID, 1, crColor);
    pDC->SelectObject(&penPlot);

    // Scale the data to fit in our window
    fHScale = (double)((double)(end - start)/ (double)rWnd->Width());

    data_index = start;
    pDC->MoveTo(0, data[0]);

    // Do the drawing
    for (double x = 0; x < rWnd->Width(); x++)
    {

        datapoint = data[(long)floor(data_index)];                                          // Get the data
        data_index += fHScale;                                                                                  // Advance the data_index
        //data_index++;


        point_coords.x = long(x);
        point_coords.y = datapoint;
        point_coords.y = rWnd->Height() - point_coords.y;                               // Need to print it upside down, because bitmaps are funny look'n

        //pDC->SetPixel(point_coords, crColor);                                                 // for drawing pixels
        pDC->LineTo(point_coords);                                                                          // for connecting the dots


    }
    return TRUE;
}

// GetNode
//*  Description   : Returns the nth leaf given by level, where a leaf is a node with
//*                                a null left and right node
//*  Parameters    : long level - which leaf to return
//*  Preconditions : the tree should be built using CreateTree
//*  Postcondtions : None
//*  Returns       : NULL - if the leaf was not found, otherwise
//*                                  pointer to the Node
//**************************************************************************
CWaveletNode * CWaveletNode::GetNode(long level)
{
    return _GetNode(level, true);
}       // Wrapper

CWaveletNode * CWaveletNode::_GetNode(long level, bool reset)
{
    CWaveletNode * finalNode = NULL;
    // Use a static variable to keep track of our count between static function calls
    static long count;


    if (reset == true)
    {
        count = 0;      // Reset the count only on the first call
        finalNode = NULL;
    }

    // We found a leaf
    if (IsLeaf())
    {
        count++;
        if (count == level)
        {
            return this;
        }
    }

    if (left_node != NULL)
    {
        finalNode = left_node->_GetNode(level, false);
        if (count == level)
        {
            return finalNode;
        }
    }

    if (right_node != NULL)
    {
        finalNode = right_node->_GetNode(level, false);
        if (count == level)
        {
            return finalNode;
        }

    }
    return finalNode;
}

// GetMax
//*  Description   : Gets the max value of this node's data
//*  Parameters    : none
//*  Preconditions : Data should be allocated and populated
//*  Postcondtions : none
//*  Returns       : the max of the data
//**************************************************************************
double CWaveletNode::GetMaxNode()
{
    ASSERT(data != NULL);

    double max = 0;

    for (long index = 0; index < dwDataSize; index++)
        if (data[index] > max)
        {
            max = data[index];
        }

    return max;
}

// GetMaxNodeBounds
//*  Description   : Gets the max value of this node's data
//*  Parameters    : long start, end - bounds to look for the max within
//*  Preconditions : Data should be allocated and populated
//*  Postcondtions : none
//*  Returns       : the max of the data
//**************************************************************************
double CWaveletNode::GetMaxNodeBounds(long start, long end)
{
    ASSERT(data != NULL);

    double max = 0;

    for (long index = start; index < end; index++)
        if (data[index] > max)
        {
            max = data[index];
        }

    return max;
}

// GetMinNode
//*  Description   : Gets the min value of this node's data
//*  Parameters    : none
//*  Preconditions : Data should be allocated and populated
//*  Postcondtions : none
//*  Returns       : the min of the data
//**************************************************************************
double CWaveletNode::GetMinNode()
{
    ASSERT(data != NULL);

    double min = 0xFFFFFFFF;

    for (long index = 0; index < dwDataSize; index++)
        if (data[index] < min)
        {
            min = data[index];
        }

    return min;
}

// GetMinNodeBounds
//*  Description   : Gets the min value of this node's data
//*  Parameters    : long start, end - bounds to look for the max within
//*  Preconditions : Data should be allocated and populated
//*  Postcondtions : none
//*  Returns       : the min of the data
//**************************************************************************
double CWaveletNode::GetMinNodeBounds(long start, long end)
{
    ASSERT(data != NULL);

    double min = 0xFFFFFFFF;

    for (long index = start; index < end; index++)
        if (data[index] < min)
        {
            min = data[index];
        }

    return min;
}

// SetData
//*  Description   : Allocates the memory for and copies incoming data
//*  Notes               : if _data is NULL, the memory is just allocated to the buffer
//*  Parameters    : long *_data - pointer to incoming data
//*                                  DWORD _dwDataSize - data size
//*                                  double _lower_freq, _upper_freq - inits local variables
//*  Preconditions : None
//*  Postcondtions : None
//*  Returns       : TRUE - on success
//*                                  FALSE - cannot allocate memory
//**************************************************************************
BOOL CWaveletNode::SetDataNode(long * _data, DWORD _dwDataSize, double _lower_freq, double _upper_freq)
{
    dwDataSize = _dwDataSize;
    lower_freq = _lower_freq;
    upper_freq = _upper_freq;

    data = (long *)malloc(dwDataSize * sizeof(long));
    if (data == NULL)
    {
        return FALSE;
    }


    if (_data != NULL)
    {
        memcpy(data, _data, dwDataSize * sizeof(long));
    }

    return TRUE;

}

// GetNumLeaves
//*  Description   : Gets the number of leaves (nodes with both left & right nodes = null)
//*                                  in the tree
//*  Parameters    : None
//*  Preconditions : None
//*  Postcondtions : None
//*  Returns       : number of leaves in the tree
//**************************************************************************
long CWaveletNode::GetNumLeaves()
{
    long count = 0;

    if (left_node->IsLeaf())
    {
        count++;
    }
    else
    {
        count += left_node->GetNumLeaves();
    }

    if (right_node->IsLeaf())
    {
        count++;
    }
    else
    {
        count += right_node->GetNumLeaves();
    }

    return count;
}


// GetMaxAllLeaves
//*  Description   : Gets the maximum data point of all nodes in the tree
//*  Parameters    : None
//*  Preconditions : Data should be allocated and populated to all leaves
//*  Postcondtions : None
//*  Returns       : the maximum datapoint of all nodes in the tree
//**************************************************************************
double CWaveletNode::GetMaxTree()           // Wrapper function for the recursive routine
{
    return _GetMaxTree(0);
}

double CWaveletNode::_GetMaxTree(double max)
{

    if (left_node->IsLeaf())
    {
        if (left_node->GetMaxNode() > max)
        {
            max = left_node->GetMaxNode();
        }
    }
    else
    {
        max = left_node->_GetMaxTree(max);
    }


    if (right_node->IsLeaf())
    {
        if (right_node->GetMaxNode() > max)
        {
            max = right_node->GetMaxNode();
        }
    }
    else
    {
        max = right_node->_GetMaxTree(max);
    }

    return max;
}

// GetMaxTreeBounds
//*  Description   : Gets the maximum data point of all nodes in the tree, between given bounds
//*  Parameters    : long start, end - starting and ending bounds to search between
//*  Preconditions : Data should be allocated and populated to all leaves
//*  Postcondtions : None
//*  Returns       : the maximum between the bounds of all nodes
//**************************************************************************
double CWaveletNode::GetMaxTreeBounds(long start, long end)         // Wrapper function for the recursive routine
{
    return _GetMaxTreeBounds(0, start, end);
}
double CWaveletNode::_GetMaxTreeBounds(double max, long start, long end)
{

    if (left_node->IsLeaf())
    {
        if (left_node->GetMaxNodeBounds(start, end) > max)
        {
            max = left_node->GetMaxNodeBounds(start, end);
        }
    }
    else
    {
        max = left_node->_GetMaxTreeBounds(max, start, end);
    }


    if (right_node->IsLeaf())
    {
        if (right_node->GetMaxNodeBounds(start, end) > max)
        {
            max = right_node->GetMaxNodeBounds(start, end);
        }
    }
    else
    {
        max = right_node->_GetMaxTreeBounds(max, start, end);
    }

    return max;
}

//**************************************************************************
// CProcessWavelet helper functions
//**************************************************************************
// Get_Raw_Data
//*  Description   : A messy function based off of the sa_g_raw onDraw function
//*                  It returns an allocated pointer containing the raw data
//*                                  of the waveform
//*  Parameters    : int **pDataOut (out) - a pointer to the pointer to receive
//*                                                                                 the data
//*                                  DWORD *dwDataSize (out) - a pointer to the dword to recieve
//*                                                                                      how much data we have
//*  Preconditions : None
//*  Postcondtions : pDataOut contains the raw data of the wave form currently being viewd
//*  Returns       : TRUE on sucess
//*                                  FALSE otherwise
//**************************************************************************
BOOL CProcessWavelet::Get_Raw_Data(long ** pDataOut, DWORD * dwDataSizeOut, ISaDoc * pDoc)
{
    HPSTR pData;                                // actual data pointer


    DWORD dwBufferSizeBytes;
    DWORD dwDataPos;
    DWORD dwDataPosBytes;
    DWORD dwChunkSizeBytes;
    DWORD dwDataSizeBytes;

    short data;

    // Total number SA can give me at one time
    dwBufferSizeBytes = GetBufferSize(); // data buffer size


    // Total number of bytes in waveform
    dwDataSizeBytes = pDoc->GetDataSize();


    *pDataOut = (long *)malloc(dwDataSizeBytes * 2);

    if (*pDataOut == NULL)
    {
        return FALSE;
    }

    // Start at the beginning
    dwDataPos = 0;
    dwDataPosBytes = 0;

    // check if we can read this thing in one pass, or if we need to start with one chunk
    if (dwDataSizeBytes < dwBufferSizeBytes)
    {
        dwChunkSizeBytes = dwDataSizeBytes;
    }
    else
    {
        dwChunkSizeBytes = dwBufferSizeBytes;
    }



    // Get the data
    while (1)
    {
        pData = pDoc->GetWaveData(dwDataPosBytes, TRUE);

        // Get the bitrate (8 or 16bit data)

        // FIX: pad the data with 0's if we have 8 bit data

        // copy the data into our buffer
        for (DWORD index = 0; index < dwChunkSizeBytes; index+=2)
        {
            data = MAKEWORD(pData[index], pData[index + 1]);

            (*pDataOut)[dwDataPos] = data;
            dwDataPos++;
            dwDataPosBytes+=2;
        }

        //memcpy(pDataOut, pData, dwChunkSize);

        //dwDataPos += dwChunkSize;
        //pDataOut += dwBufferSize;

        // see if we are done
        if (dwDataPosBytes == dwDataSizeBytes)
        {
            break;
        }

        // compute the next chunk size
        if ((dwDataSizeBytes - dwDataPosBytes) < dwBufferSizeBytes)
        {
            dwChunkSizeBytes = (dwDataSizeBytes - dwDataPosBytes);
        }
        else
        {
            dwChunkSizeBytes = dwBufferSizeBytes;
        }
    }
    *dwDataSizeOut = (dwDataSizeBytes / 2);
    return TRUE;
}
