#ifndef TONESEGMENT_H
#define TONESEGMENT_H

#include "DependentSegment.h"

//###########################################################################
// CToneSegment data processing

class CToneSegment : public CDependentSegment {
public:
    CToneSegment(Context& context, EAnnotation index, int master = -1) : CDependentSegment(context,index,master) {}
    CToneSegment() = delete;

private:
    virtual CFontTable * NewFontTable() const;
};

#endif

