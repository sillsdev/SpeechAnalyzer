// Pseudo-templates for data structures
// Here, we include the .hpp files as well.
// This causes implementations to be generated.
#include "stddsp.h"
#include "template.h"

#define First float
#define Second float
#define tpair pair_float_float
#include "tpair.hpp"
#undef tpair
#undef First
#undef Second

#define Type pair_float_float
#define CTPQueue pq_pair_float_float
#define CTVector vector_pair_float_float
#include "tvector.hpp"
#include "tpq.hpp"
#undef Type
#undef CTPQueue
#undef CTVector

#define Type float
#define CTPQueue pq_float
#define CTVector vector_float
#include "tvector.hpp"
#include "tpq.hpp"
#undef Type
#undef CTPQueue
#undef CTVector

#define Type rpair_float_float
#define CTPQueue pq_rpair_float_float
#define CTVector vector_rpair_float_float
#include "tvector.hpp"
#include "tpq.hpp"
#undef Type
#undef CTPQueue
#undef CTVector

int32 operator <(const rpair_float_float & x, const rpair_float_float & y) {
    return x.first > y.first;
}