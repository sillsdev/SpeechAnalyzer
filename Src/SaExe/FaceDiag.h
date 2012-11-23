#ifndef FACEDIAG_H
#define FACEDIAG_H

typedef struct SVOCAL_TRACT_COORD{
	int32 X;
	int32 Y;
	double RadAng;
} VOCAL_TRACT_COORD;

#define GLOTTIS_DIAMETER  31   //from FACE.BMP, extends from 304 to 335 in x dimension
#define TONGUE_ULX         5
#define TONGUE_ULY         4
#define TONGUE_LRX        23
#define TONGUE_LRY        30

#define JAW_ULX            3
#define JAW_ULY            5
#define JAW_LRX          156
#define JAW_LRY          218


#endif




