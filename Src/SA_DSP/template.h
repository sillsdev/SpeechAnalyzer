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
#define CTPQueue pq_pair_float_float
#define CTVector vector_pair_float_float
#include "tvector.h"
#include "tpq.h"
#undef Type
#undef CTPQueue
#undef CTVector

#define Type float
#define CTPQueue pq_float
#define CTVector vector_float
#include "tvector.h"
#include "tpq.h"
#undef Type
#undef CTPQueue
#undef CTVector

struct rpair_float_float : public pair_float_float
{
};

// This is defined the opposite of what it looks like so we can get a
// priority queue that goes in increasing order rather than decreasing.
int operator <(const rpair_float_float &, const rpair_float_float &);

#define Type rpair_float_float
#define CTPQueue pq_rpair_float_float
#define CTVector vector_rpair_float_float
#include "tvector.h"
#include "tpq.h"
#undef Type
#undef CTPQueue
#undef CTVector

#endif
