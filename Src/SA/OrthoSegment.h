#ifndef ORTHOSEGMENT_H
#define ORTHOSEGMENT_H

#include "DependentSegment.h"

class COrthographicSegment : public CDependentSegment {
public:
    COrthographicSegment(Context& context, EAnnotation index, int master = -1) : CDependentSegment(context,index,master) {}
    COrthographicSegment() = delete;

private:
    virtual CFontTable * NewFontTable() const;
};

#endif

