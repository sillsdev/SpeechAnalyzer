// Roots.h: interface for the CRoots class.
//
//////////////////////////////////////////////////////////////////////

#ifndef ROOTS_H
#define ROOTS_H

using std::complex;
typedef complex<double> CDBL;

bool roots(double Coeff[], int32 nOrder, CDBL Roots[], CDBL Seed[], int32 nSeeds, bool bRefine = true, bool bPolish = false);

#endif
