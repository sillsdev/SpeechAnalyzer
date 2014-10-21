#include "Stdafx.h"
#include "ReferenceSegment.h"

CReferenceSegment::CReferenceSegment(int index, int master) : CDependentTextSegment(index, master)
{
}

CFontTable * CReferenceSegment::NewFontTable() const
{
    return new CFontTableANSI();
}
