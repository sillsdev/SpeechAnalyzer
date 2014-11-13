#ifndef TONESEGMENT_H
#define TONESEGMENT_H

#include "DependentSegment.h"

//###########################################################################
// CToneSegment data processing

class CToneSegment : public CDependentSegment
{
    // Operations
public:
    CToneSegment(EAnnotation index, int master = -1);

    // Attributes
private:
    virtual CFontTable * NewFontTable() const;
};

#endif

