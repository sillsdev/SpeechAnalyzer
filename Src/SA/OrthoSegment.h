#ifndef ORTHOSEGMENT_H
#define ORTHOSEGMENT_H

#include "DependentSegment.h"

class COrthoSegment : public CDependentSegment
{
public:
    COrthoSegment(EAnnotation index, int master = -1);

private:
    virtual CFontTable * NewFontTable() const;
};

#endif

