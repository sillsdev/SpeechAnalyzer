#ifndef PHONEMICSEGMENT_H
#define PHONEMICSEGMENT_H

#include "DependentSegment.h"

class CPhonemicSegment : public CDependentSegment {
public:
    CPhonemicSegment() = delete;

    virtual bool Filter();
    virtual bool Filter(CString & text);

private:
    virtual CFontTable * NewFontTable() const;
};

#endif

