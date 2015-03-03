#ifndef QPKSRC_H
#define QPKSRC_H

#include "pksrc.h"

// A CQuadraticPeakSource is an iterator over peaks in a function.
// The peaks are returned in order of increasing location.
// See class CPeakSource for more documentation.

class CQuadraticPeakSource : public CPeakSource {
public:

    // Construct a Quadratic peak source with the given concavity
    // threshold.
    CQuadraticPeakSource(float threshold);

    virtual void Search(const float * start, const float * end);
    virtual void Next();
    virtual void Get(float & location, float & value) const;
    virtual int IsDone() const;

private:
    // These should never be called
    CQuadraticPeakSource(const CQuadraticPeakSource &);
    const CQuadraticPeakSource & operator=(const CQuadraticPeakSource &);

    const float * myStart;
    const float * myMark;
    const float * myEnd;
    float myThreshold;
    float myLocation, myValue;
    int imDone;
};

#endif
