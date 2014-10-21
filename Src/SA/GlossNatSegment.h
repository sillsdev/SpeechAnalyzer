#ifndef GLOSS_NAT_SEGMENT_H
#define GLOSS_NAT_SEGMENT_H

#include "DependentTextSegment.h"

class CGlossNatSegment : public CDependentTextSegment
{
public:
    CGlossNatSegment(int index, int master = -1);

    // Attributes
private:
    virtual CFontTable * NewFontTable() const;
};

#endif

