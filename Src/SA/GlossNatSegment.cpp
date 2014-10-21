#include "Stdafx.h"
#include "GlossNatSegment.h"

CGlossNatSegment::CGlossNatSegment(int index, int master) : CDependentTextSegment(index, master)
{
}

CFontTable * CGlossNatSegment::NewFontTable() const
{
    return new CFontTableANSI();
}
