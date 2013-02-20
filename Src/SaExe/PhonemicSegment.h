#ifndef PHONEMICSEGMENT_H
#define PHONEMICSEGMENT_H

#include "DependentSegment.h"

//###########################################################################
// CPhonemicSegment data processing

class CPhonemicSegment : public CDependentSegment {
	// Operations
public:
	CPhonemicSegment(int index, int master = -1);
	virtual CSegment::TpInputFilterProc GetInputFilter() const; // filter function for input

	// Attributes
private:
	virtual CFontTable* NewFontTable() const;
};

#endif

