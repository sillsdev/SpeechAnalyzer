#ifndef TEMPLATE_H
#define TEMPLATE_H

#define First float
#define Second float
#define tpair pair_float_float
#include "tpair.h"
#undef tpair
#undef First
#undef Second

#define Type pair_float_float
#define tpqueue pq_pair_float_float
#define tvector vector_pair_float_float
#include "tvector.h"
#include "tpq.h"
#undef Type
#undef tpqueue
#undef tvector

#define Type float
#define tpqueue pq_float
#define tvector vector_float
#include "tvector.h"
#include "tpq.h"
#undef Type
#undef tpqueue
#undef tvector

struct rpair_float_float : public pair_float_float {
};

// This is defined the opposite of what it looks like so we can get a
// priority queue that goes in increasing order rather than decreasing.
int operator <(const rpair_float_float &, const rpair_float_float &);

#define Type rpair_float_float
#define tpqueue pq_rpair_float_float
#define tvector vector_rpair_float_float
#include "tvector.h"
#include "tpq.h"
#undef Type
#undef tpqueue
#undef tvector

#endif
