#ifndef REFERENCE_SEGMENT_H
#define REFERENCE_SEGMENT_H

#include "DependentTextSegment.h"

class CReferenceSegment : public CDependentTextSegment
{
public:
    CReferenceSegment(int index, int master = -1);

    // Attributes
private:
    virtual CFontTable * NewFontTable() const;
};

#endif

