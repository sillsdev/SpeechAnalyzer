#ifndef PHONEMICSEGMENT_H
#define PHONEMICSEGMENT_H

#include "DependentSegment.h"

class CPhonemicSegment : public CDependentSegment
{
public:
    CPhonemicSegment(int index, int master = -1);
    virtual CSegment::TpInputFilterProc GetInputFilter() const; // filter function for input

private:
    virtual CFontTable * NewFontTable() const;
};

#endif

