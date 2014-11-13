#ifndef DEPENDENTSEGMENT_H
#define DEPENDENTSEGMENT_H

#include "Segment.h"
#include "ISa_Doc.h"

//###########################################################################
// CDependentSegment data processing

class CDependentSegment : public CSegment
{
public:
    CDependentSegment(EAnnotation index, int master = -1): CSegment(index,master) {};

    // Attributes
private:

    // Operations
protected:
    int AlignOffsetToMaster(ISaDoc *, DWORD * pdwOffset) const; // find the nearest master segment to the given position
    int AlignStopToMaster(CDocument *, DWORD * pdwStop) const; // find the nearest master segment to the given position
public:
    void AdjustCursorsToMaster(CDocument * pSaDoc, BOOL bAdjust = TRUE, DWORD * pdwOffset = NULL, DWORD * pdwStop = NULL) const; // adjust cursors to the nearest phonetic segment to the given position SDM 1.06.1.2 Added function
    int AdjustPositionToMaster(CDocument * pSaDoc, DWORD & pdwOffset, DWORD & pdwStop) const;
    int CheckPositionToMaster(ISaDoc * pDoc, DWORD dwStart, DWORD dwStop, EMode nMode) const;
    virtual int CheckPosition(ISaDoc * pDoc, DWORD dwStart, DWORD dwStop, EMode nMode=MODE_AUTOMATIC, BOOL /*bUnused*/=TRUE) const;
    virtual void LimitPosition(CSaDoc *,DWORD & dwStart,DWORD & dwStop, ELimit nMode=LIMIT_MOVING_BOTH) const;
    virtual void Add(CSaDoc * pDoc, DWORD dwStart, CSaString & szString, BOOL bDelimiter = FALSE, BOOL bCheck = TRUE); // add a segment
};

#endif

