#include "Stdafx.h"
#include "GlossNatSegment.h"

CGlossNatSegment::CGlossNatSegment(EAnnotation index, int master) : CDependentTextSegment(index, master) {
}

CFontTable * CGlossNatSegment::NewFontTable() const {
    return new CFontTableANSI();
}
