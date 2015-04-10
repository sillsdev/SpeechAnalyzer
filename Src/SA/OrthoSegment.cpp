#include "stdafx.h"
#include "OrthoSegment.h"

COrthographicSegment::COrthographicSegment(EAnnotation index, int master) :
    CDependentSegment(index,master) {
}

CFontTable * COrthographicSegment::NewFontTable() const {
    return new CFontTableANSI;
}

