#include "stdafx.h"
#include "ToneSegment.h"
#include "sa_ipa.h"

CFontTable * CToneSegment::NewFontTable() const {
    return new CFontTableANSI;
}

