#ifndef REFERENCE_SEGMENT_H
#define REFERENCE_SEGMENT_H

#include "TextSegment.h"

class CReferenceSegment : public CTextSegment {
public:
    CReferenceSegment(Context& context, EAnnotation index, int master = -1) : CTextSegment(context, index, master) {};
    CReferenceSegment() = delete;

    virtual void LimitPosition(CSaDoc *,DWORD & dwStart,DWORD & dwStop, ELimit nMode=LIMIT_MOVING_BOTH) const;
	// insert a new segment with text
    virtual BOOL Append(LPCTSTR pszString, bool delimiter, DWORD dwStart, DWORD dwDuration);     
	// insert a new segment with text
    virtual BOOL Insert(int nIndex, LPCTSTR pszString, bool delimiter, DWORD dwStart, DWORD dwDuration);
    int CheckPositionToMaster(ISaDoc *,DWORD dwStart,DWORD dwStop, EMode nMode) const;
	// add a segment
    virtual void Add(CSaDoc * pModel, CSaView * pView, DWORD dwStart, CSaString & szString, bool bDelimiter, bool bCheck); 

private:
    virtual CFontTable * NewFontTable() const;
};

#endif

