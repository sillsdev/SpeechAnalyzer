#ifndef INDEPENDENTSEGMENT_H
#define INDEPENDENTSEGMENT_H

#include "sa_segm.h"
#include "Sa_Doc.h"

//###########################################################################
// CIndependentSegment data processing

class CIndependentSegment : public CSegment
{
	// Operations
public:
	CIndependentSegment(int index, int master = -1);
	virtual int CheckPosition( CSaDoc*,DWORD dwStart,DWORD dwStop, EMode nMode=MODE_AUTOMATIC,BOOL bOverlap=TRUE) const;
	virtual void LimitPosition( CSaDoc*,DWORD& dwStart,DWORD& dwStop, int nMode=LIMIT_MOVING_BOTH) const;
};

#endif
