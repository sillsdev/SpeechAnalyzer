#ifndef PHONEMICSEGMENT_H
#define PHONEMICSEGMENT_H

#include "DependentSegment.h"

class CPhonemicSegment : public CDependentSegment
{
public:
    CPhonemicSegment(EAnnotation index, int master = -1);
	virtual bool Filter();
	virtual bool Filter( CString & text);

private:
    virtual CFontTable * NewFontTable() const;
};

#endif

