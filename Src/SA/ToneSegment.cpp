#include "stdafx.h"
#include "ToneSegment.h"
#include "sa_ipa.h"

CToneSegment::CToneSegment(EAnnotation index, int master) :
    CDependentSegment(index,master) {
}

CFontTable * CToneSegment::NewFontTable() const {
    return new CFontTableANSI;
}

