#ifndef TONESEGMENT_H
#define TONESEGMENT_H

#include "DependentSegment.h"

//###########################################################################
// CToneSegment data processing

class CToneSegment : public CDependentSegment {
public:
    CToneSegment() = delete;

private:
    virtual CFontTable * NewFontTable() const;
};

#endif

