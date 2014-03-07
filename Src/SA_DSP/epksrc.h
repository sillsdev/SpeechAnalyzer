#ifndef EPKSRC_H
#define EPKSRC_H

#include "pksrc.h"
#include "template.h"

// An CEnergyPeakSource is a Decorator class.  It finds all peaks from
// a given CPeakSource and returns them in order of energy from most
// energetic to least energetic.
//
// See class CPeakSource for more documentation.
//
// This is an instance of the Decorator design pattern.

class CEnergyPeakSource : public CPeakSource
{
public:
    // Construct this peak source as a wrapper around the given peak
    // source.
    CEnergyPeakSource(CPeakSource & ps);
    ~CEnergyPeakSource();

    virtual void Search(const float * start, const float * end);
    virtual void Next();
    virtual void Get(float & location, float & value) const;
    virtual int IsDone() const;

private:
    // These should never be called
    CEnergyPeakSource(const CEnergyPeakSource &);
    const CEnergyPeakSource & operator=(const CEnergyPeakSource &);

    CPeakSource & myPS;
    pq_pair_float_float * myQ;
};

#endif
