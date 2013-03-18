#ifndef  DSP_TYPES_H
#define DSP_TYPES_H

// #include <stddef.h>


#define MAX_INT           32767
#define MIN_INT         (-32768)
#define INT_RANGE  (MAX_INT - MIN_INT + 1)
#define MAX_SHORT        MAX_INT
#define MIN_SHORT        MIN_INT
#define SHORT_RANGE      INT_RANGE
#define MAX_LONG     2147483647
#define MIN_LONG   (-2147483648)
#define LONG_RANGE (MAX_LONG - MIN_LONG + 1)

// Variable type definitions
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef signed char int8;
typedef signed short int16;
typedef signed int int32;
typedef int32 dspError_t;

#endif
