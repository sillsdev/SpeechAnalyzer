#ifndef INDEPENDENTSEGMENT_H
#define INDEPENDENTSEGMENT_H

#include "Segment.h"
#include "SA_View.h"

class CSaDoc;

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
    virtual void Add(CSaDoc * pDoc, CSaView * pView, DWORD dwStart, CSaString & szString, bool bDelimiter, bool bCheck);
	virtual int Add( DWORD offset, DWORD duration);
	virtual void AddAt( int index, DWORD offset, DWORD duration);
};

#endif
