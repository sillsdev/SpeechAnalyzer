#ifndef TEXTSEGMENT_H
#define TEXTSEGMENT_H

#include "DependentSegment.h"

//###########################################################################
// CTextSegment data processing

class CTextSegment : public CDependentSegment {
public:
    CTextSegment() = delete;

	// insert a new segment
    virtual BOOL Insert(int nIndex, LPCTSTR pszString, bool delimiter, DWORD dwStart, DWORD dwDuration);
	// append new segment
    virtual BOOL Append(LPCTSTR pszString, bool delimiter, DWORD dwStart, DWORD dwDuration);
	// add new segment
    virtual void Add(CSaDoc * pModel, CSaView * pView, DWORD dwStart, CSaString & szString, bool bDelimiter, bool bCheck);
    virtual int CheckPosition(ISaDoc *,DWORD dwStart,DWORD dwStop, EMode nMode=MODE_AUTOMATIC, BOOL bUnused = TRUE) const;
    virtual void LimitPosition(CSaDoc *,DWORD & dwStart,DWORD & dwStop, ELimit nMode=LIMIT_MOVING_BOTH) const;
    DWORD CalculateDuration(ISaDoc * pModel, const int nIndex) const;
};

#endif

