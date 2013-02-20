#include "Stdafx.h"
#include "PhonemicSegment.h"

//###########################################################################
// CPhonemicSegment data processing

CPhonemicSegment::CPhonemicSegment(int index, int master) : CDependentSegment(index,master) 
{
}

CSegment::TpInputFilterProc CPhonemicSegment::GetInputFilter(void) const 
{
	return gIPAInputFilter;
}

CFontTable* CPhonemicSegment::NewFontTable() const 
{
	return new CFontTableIPA;
};
