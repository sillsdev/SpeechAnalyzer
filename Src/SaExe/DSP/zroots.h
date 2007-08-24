#ifndef ZROOTS_H
#define ZROOTS_H
 #include "complex.h"
 #include "error.h"

 ERR zroots(double Coeff[], int nOrder, COMPLEX_RECT_FLOAT Roots[], COMPLEX_RECT_FLOAT Seed[], int nSeeds, BOOLFLAG bRefine, BOOLFLAG bSort);
#error
 ERR laguer(COMPLEX_RECT_FLOAT Coeff[], int nOrder, COMPLEX_RECT_FLOAT *Root, int *nIterations);

#endif
