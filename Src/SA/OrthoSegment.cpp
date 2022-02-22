#include "stdafx.h"
#include "OrthoSegment.h"
#include "sa_ipa.h"

CFontTable * COrthographicSegment::NewFontTable() const {
    return new CFontTableANSI;
}

