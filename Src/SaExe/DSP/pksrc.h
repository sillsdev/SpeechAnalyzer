#ifndef PKSRC_H
#define PKSRC_H

// A PeakSource is an iterator over peaks in a sampled function.
// The Search() function limits this interface to working on a
// particular kind of sample function.  This is a time trade-off.
// Since a spectrogram needs to use a peak source for every time
// slice, it is not practical to create and destroy a peak source for
// every one.  Thus, it must be possible to reset a PeakSource to work
// on different spectra.
//
// Note: The order in which peaks are returned is not specified.
// Concrete subclasses of PeakSource define their own order.
//
// This is an instance of the Strategy design pattern, implemented as
// an Iterator.

class PeakSource {
public:
    virtual ~PeakSource() {};

    // Tell this peak source to search the given array for peaks, and
    // advance to the first peak.
    // The pointers start and end must not be null, and must satisfy
    // start <= end.
    virtual void Search(const float * start, const float * end) = 0;

    // Move to the next peak.
    // It is an error to call this if IsDone() returns true.
    virtual void Next() = 0;

    // Get the current peak location and value.
    // It is an error to call this if IsDone() returns true.
    // The location is the offset from the start pointer passed to Search().
    // The location value is a float rather than an int so a peak
    // source could locate peaks between samples.
    virtual void Get(float & location, float & value) const = 0;

    // Return true if there are no more peaks available to this iterator.
    virtual int IsDone() const = 0;

protected:
    PeakSource() {};

private:
    // These should never be called
    PeakSource(const PeakSource &);
    const PeakSource & operator=(const PeakSource &);
};

#endif
