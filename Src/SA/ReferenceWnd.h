/////////////////////////////////////////////////////////////////////////////
// sa_annot.h:
// Interface of the CLegendWnd
//                  CXScaleWnd
//                  CAnnotationWnd
//                  CPhoneticWnd
//                  CToneWnd
//                  CPhonemicWnd
//                  COrthographicWnd
//                  CGlossWnd
//                  CReferenceWnd classes.
// Author: Urs Ruchti
// copyright 2000 JAARS Inc. SIL
//
// Revision History
//   1.06.2
//         SDM removed parameterless constructors from CAnnotationWnd and sub classes
//         SDM added SetHintUpdateBoundaries and members m_bHintUpdateBoundaries...
//   1.06.4
//         SDM changed return type for CAnnotationWnd::GetFont
//         SDM added include for CFontTable
//   1.06.6
//         SDM added m_nSelectTickCount

//   1.06.6U4
//         SDM changed floats to double to fix precision errors in scrolling
//   1.5Test8.1
//         SDM added CReferenceWnd class
//         SDM changed parameters for ClegendWnd::OnDraw
//         SDM added member CAnnotationWnd::m_bOverlap
//         SDM changed parameters to CAnnotationWnd::SetHintUpdateBoundaries
//   07/20/2000
//         RLJ Added support for adjusting y-scale (semitone #) in Melogram
//   09/27/2000
//         DDO Added the function GetTWCXScaleWindowHeight() to the x-scale
//             window class.
/////////////////////////////////////////////////////////////////////////////
#ifndef REFERENCE_WND_H
#define REFERENCE_WND_H

#include "SA_Annot.h"

class CReferenceWnd : public CAnnotationWnd
{
public:
    CReferenceWnd(int nIndex);
    virtual void OnDraw(CDC * pDC, const CRect & printRect);
};

#endif


