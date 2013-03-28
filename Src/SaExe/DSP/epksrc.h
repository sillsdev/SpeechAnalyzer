#ifndef EPKSRC_H
#define EPKSRC_H

#include "pksrc.h"
#include "template.h"

// An EnergyPeakSource is a Decorator class.  It finds all peaks from
// a given PeakSource and returns them in order of energy from most
// energetic to least energetic.
//
// See class PeakSource for more documentation.
//
// This is an instance of the Decorator design pattern.

class EnergyPeakSource : public PeakSource
{
public:
    // Construct this peak source as a wrapper around the given peak
    // source.
    EnergyPeakSource(PeakSource & ps);
    ~EnergyPeakSource();

    virtual void Search(const float * start, const float * end);
    virtual void Next();
    virtual void Get(float & location, float & value) const;
    virtual int IsDone() const;

private:
    // These should never be called
    EnergyPeakSource(const EnergyPeakSource &);
    const EnergyPeakSource & operator=(const EnergyPeakSource &);

    PeakSource & myPS;
    pq_pair_float_float * myQ;
};

#endif
