#include "stdafx.h"
#include "ToneSegment.h"

CToneSegment::CToneSegment( int index, int master) : 
CDependentSegment(index,master) 
{
}

CFontTable* CToneSegment::NewFontTable() const 
{
	return new CFontTableANSI;
}

