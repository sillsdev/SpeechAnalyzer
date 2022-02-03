//***************************************************************************
//* sa_g_wavelet.h (v1.0)                                                   *
//* ----------------                                                        *
//* Project          : Speech Analyzer                                      *
//* Author           : Andy Heitke                                          *
//* Date Started     : 6/11/01                                              *
//* Customer Name    : JAARS / SIL                                          *
//* Description      : This is the main header file for the Wavelet         *
//*                    process in SA                                        *
//* Revision History : 7/30/01 ARH - Imported file into Speech Analyzer     *
//*                                  v2.0 project                           *
//***************************************************************************
#ifndef _SA_P_WAVELET_H
#define _SA_P_WAVELET_H

#include "Process.h"
#include "param.h"

#define DEBAUCHES4      4

//**************************************************************************
// CPlotWavelet Class
//**************************************************************************
class CProcessWavelet : public CProcess {
private:
    BOOL data_status;               // Is the data ready for display?
    CWaveletParm m_WaveletParm;     // wavelet parameters

public:
    // Function to get data from SA
    BOOL Get_Raw_Data(long** pDataOut, DWORD* dwDataSize, Model* pModel);
    long Process(void* pCaller, Model* pModel, int nWidth, int nHeight, int nProgress = 0, int nLevel = 1);
};

//**************************************************************************
// CWaveletNode Class
//**************************************************************************
class CWaveletNode {

private:
    // Typical tree links
    CWaveletNode* parent_node;
    CWaveletNode* left_node;
    CWaveletNode* right_node;

    // Data of this node
    long* data;
    long dwDataSize;

    // The frequency bounds of this node
    double upper_freq;
    double lower_freq;

private:
    // Main Helper functions
    BOOL _DoMRAAnalysisTree(long stride);                                            // Recursive routine
    BOOL WaveletTransformNode(long* pFinalLow,                                  // Convolution and most wavelet work
            long* pFinalHigh,                             // done here
            long wavelet_type,
            long stride);

    // Drawing helper routines
    long  _DrawColorBandTree(unsigned char* pBits,                           // Recursive routine
            CRect* rWnd,
            long thickness,
            long y,
            double high,
            double start,
            double end);

    BOOL                     DrawColorBandNode(unsigned char* pBits,                           // Draws one color band
            CRect* rWnd,
            long thickness,
            long y_start,
            double high,
            double start,
            double end);

    // Private helper functions
    double               _GetMaxTree(double max);                                                           // recursive routine
    double               _GetMaxTreeBounds(double max, long start, long end);   // recursive routine
    CWaveletNode* _GetNode(long level, bool reset);                                      // recursive routine

public:
    CWaveletNode();
    ~CWaveletNode();

    // Main functions
    BOOL  DoMRAAnalysisTree();    // wraps _DoMRAAnalysisTree
    BOOL  DrawColorBandTree(unsigned char* pBits,                  // wraps _DrawColorBandTree
            CRect* rWnd,
            double high,
            double start,
            double end);

    // Transform Functions - these are used for post processing after the wavelet has been generated
    BOOL  TransformEnergyNode();
    BOOL  TransformLogScalingNode(double high);
    BOOL  TransformSmoothingNode();
    BOOL  TransformEnergyTree();
    BOOL  TransformLogScalingTree(double high);
    BOOL  TransformSmoothingTree();
    BOOL  TransformFitWindowNode(CRect* rWnd);

    // Accessor methods
    BOOL  SetDataNode(long* data, DWORD dwDataSize, double _lower_freq, double _upper_freq);
    BOOL  SetLeftNode(CWaveletNode* node) {
        right_node = node;
        return TRUE;
    }
    BOOL  SetRightNode(CWaveletNode* node) {
        left_node = node;
        return TRUE;
    }
    BOOL  SetParentNode(CWaveletNode* node) {
        parent_node = node;
        return TRUE;
    }

    BOOL  SetUpperFrequencyBound(double freq) {
        upper_freq = freq;
        return TRUE;
    }
    BOOL  SetLowerFrequencyBound(double freq) {
        lower_freq = freq;
        return TRUE;
    }

    double              GetUpperFrequencyBound() {
        return upper_freq;
    }
    double              GetLowerFrequencyBound() {
        return lower_freq;
    }

    double GetMaxNode();
    double GetMaxNodeBounds(long start, long end);             // Gets the maximum datapoint between a starting and ending datapoint (wraps _GetMaxTreeBounds)

    double GetMinNode();
    double GetMinNodeBounds(long start, long end);

    double GetMaxTree();             // wraps _GetMaxTree
    double GetMaxTreeBounds(long start, long end);
    long GetNumLeaves();

    CWaveletNode* GetNode(long which_leaf);    // wraps _GetNode

    long* GetDataPtr() {
        return data;
    }
    CWaveletNode* GetLeftNode() {
        return left_node;
    }
    CWaveletNode* GetRightNode() {
        return right_node;
    }
    BOOL IsLeaf() {
        if ((left_node == NULL) && (right_node == NULL)) {
            return TRUE;
        } else {
            return FALSE;
        }
    }
};

BOOL CreateTree(char* tree_definition, CWaveletNode** root_node);

#endif //_SA_P_WAVELET_H
