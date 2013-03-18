#include "stdafx.h"
#include "OrthoSegment.h"

COrthoSegment::COrthoSegment(int index, int master) :
    CDependentSegment(index,master) {
}

CFontTable * COrthoSegment::NewFontTable() const {
    return new CFontTableANSI;
}

