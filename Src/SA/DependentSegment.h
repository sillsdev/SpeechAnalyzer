#ifndef DEPENDENTSEGMENT_H
#define DEPENDENTSEGMENT_H

#include "Segment.h"
#include "ISa_Doc.h"

//###########################################################################
// CDependentSegment data processing

class CDependentSegment : public CSegment {
public:
    CDependentSegment(Context& context, EAnnotation index, int master = -1) : CSegment(context,index,master) {};
    CDependentSegment() = delete;
	// adjust cursors to the nearest phonetic segment to the given position SDM 1.06.1.2 Added function
    void AdjustCursorsToMaster(CDocument * pSaDoc, BOOL bAdjust = TRUE, DWORD * pdwOffset = NULL, DWORD * pdwStop = NULL) const; 
    int AdjustPositionToMaster(CDocument * pSaDoc, DWORD & pdwOffset, DWORD & pdwStop) const;
    int CheckPositionToMaster(ISaDoc * pModel, DWORD dwStart, DWORD dwStop, EMode nMode) const;
    virtual int CheckPosition(ISaDoc * pModel, DWORD dwStart, DWORD dwStop, EMode nMode=MODE_AUTOMATIC, BOOL /*bUnused*/=TRUE) const;
    virtual void LimitPosition(CSaDoc *,DWORD & dwStart,DWORD & dwStop, ELimit nMode=LIMIT_MOVING_BOTH) const;
	// add a segment
    virtual void Add(CSaDoc * pModel, CSaView * pView, DWORD dwStart, CSaString & szString, bool bDelimiter, bool bCheck); 
	// remove a segment
    virtual void Remove(CSaDoc * pModel, int index, BOOL bCheck); 
	virtual int Add( DWORD offset, DWORD duration);

protected:
	// find the nearest master segment to the given position
    int AlignOffsetToMaster(ISaDoc *, DWORD * pdwOffset) const;
	// find the nearest master segment to the given position
    int AlignStopToMaster(CDocument *, DWORD * pdwStop) const;  
};

#endif

