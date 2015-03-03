#include "Stdafx.h"
#include "ReferenceSegment.h"

CReferenceSegment::CReferenceSegment(EAnnotation index, int master) : CDependentTextSegment(index, master) {
}

CFontTable * CReferenceSegment::NewFontTable() const {
    return new CFontTableANSI();
}
