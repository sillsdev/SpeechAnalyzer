#ifndef ORTHOSEGMENT_H
#define ORTHOSEGMENT_H

#include "DependentSegment.h"

class COrthographicSegment : public CDependentSegment {
public:
    COrthographicSegment(EAnnotation index, int master = -1);

private:
    virtual CFontTable * NewFontTable() const;
};

#endif

