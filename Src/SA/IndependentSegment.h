#ifndef INDEPENDENTSEGMENT_H
#define INDEPENDENTSEGMENT_H

#include "Segment.h"
#include "Sa_Doc.h"
#include "SA_View.h"

//###########################################################################
// CIndependentSegment data processing

class CIndependentSegment : public CSegment {
    // Operations
public:
    CIndependentSegment(EAnnotation index, int master = -1);
    virtual int CheckPosition(ISaDoc *,DWORD dwStart,DWORD dwStop, EMode nMode=MODE_AUTOMATIC,BOOL bOverlap=TRUE) const;
    virtual void LimitPosition(CSaDoc *,DWORD & dwStart,DWORD & dwStop, ELimit nMode=LIMIT_MOVING_BOTH) const;
    DWORD CalculateDuration(ISaDoc * pDoc, const int nIndex) const;
    // add a segment
    virtual void Add(CSaDoc * pDoc, CSaView * pView, DWORD dwStart, CSaString & szString, bool bDelimiter = false, bool bCheck = true);
};

#endif
