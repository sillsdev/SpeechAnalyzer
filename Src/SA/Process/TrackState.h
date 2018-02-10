#ifndef TRACKSTATE_H
#define TRACKSTATE_H

#include "Process\TrackState.h"

typedef std::complex<double> CDBL;
typedef std::vector<CDBL> VECTOR_CDBL;
typedef std::deque<CDBL> DEQUE_CDBL;
typedef double DBL;
typedef std::vector<DBL> VECTOR_DBL;

class STrackState {
public:
    DEQUE_CDBL data;
    // the filter window
    VECTOR_DBL window;

    // Previous track position
    VECTOR_CDBL trackIn;

    // the results
    VECTOR_CDBL trackOut;

    // Working intermediates to eliminate memory thrashing
    VECTOR_CDBL windowed;
    VECTOR_CDBL filtered;
    VECTOR_CDBL zeroFilterCDBL;
    VECTOR_DBL zeroFilterDBL;
};

#endif
