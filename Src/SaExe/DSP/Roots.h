// Roots.h: interface for the CRoots class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ROOTS_H__839C103E_8D0B_49F0_BF05_61E6AC12E568__INCLUDED_)
#define AFX_ROOTS_H__839C103E_8D0B_49F0_BF05_61E6AC12E568__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

using std::complex;
typedef complex<double> CDBL;

bool roots(double Coeff[], int32 nOrder, CDBL Roots[], CDBL Seed[], int32 nSeeds, bool bRefine = true, bool bPolish = false);

#endif // !defined(AFX_ROOTS_H__839C103E_8D0B_49F0_BF05_61E6AC12E568__INCLUDED_)
