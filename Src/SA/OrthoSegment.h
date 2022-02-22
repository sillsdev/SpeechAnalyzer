#ifndef ORTHOSEGMENT_H
#define ORTHOSEGMENT_H

#include "DependentSegment.h"

class COrthographicSegment : public CDependentSegment {
public:
    COrthographicSegment() = delete;

private:
    virtual CFontTable * NewFontTable() const;
};

#endif

