// Roots.cpp: implementation of the CRoots class.
//
//////////////////////////////////////////////////////////////////////

#include "stddsp.h"
#include "Roots.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

using std::complex;
using std::vector;
using std::sqrt;
using std::abs;
using std::back_inserter;

typedef complex<double> CDBL;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


// template to allow calling with different types of polynomials
template<class T, class CT> bool Laguerre(const T *coefficients, int32 nOrder, complex<CT> *root, int32 *its)
{
  vector<CDBL> dCoefficients;
  CDBL dRoot = *root;

  dCoefficients.reserve(nOrder+1);
  copy(coefficients, coefficients + nOrder + 1, back_inserter(dCoefficients));

  bool bResult = Laguerre(&dCoefficients[0], nOrder, &dRoot, its);

  *root = T(dRoot);

  return bResult;
}

template<class CT> bool Laguerre(const CDBL *coefficients, int32 nOrder, complex<CT> *root, int32 *its = NULL)
{
  CDBL dRoot = *root;

  bool bResult = LaguerreComplexDouble(coefficients, nOrder, &dRoot, its);

  *root = complex<CT>(dRoot);

  return bResult;
}

// Based on Laguerre's method as documented in "Numerical Recipes for C"
//
// The algoritjhm is in the public domain.
//
// Creates a new estimated root by finding the distance from the root
// using the assumption that all the other roots are equidistant
bool LaguerreComplexDouble(const CDBL *coefficients, int32 nOrder, CDBL *rootEstimate, int32 *iterations = NULL)
{
  const double nudges[] = {1/16., 2/16., 3/16., 4/16., 5/16., 6/16., 7/16., 8/16., 9/16., 10/16., 11/16., 12/16., 13/16., 14/16., 15/16.};
  const int nudge = 16;
  const int maxIter = nudge*sizeof(nudges)/sizeof(double);
  CDBL &root = *rootEstimate;
  const double epsilon = std::numeric_limits<double>::epsilon() * 16;  
  int32 extraPasses = 0;
  double error = 0;

  int32 iter;
  for(iter = 1; iter < maxIter; )
  {
    CDBL distance;

    // Evaluate polynomial, first derivative, and second derivative
    error = 0;
    CDBL polynomial = 0;
    CDBL first = 0;
    CDBL second = 0;
    double absRoot = abs(root);
    for(int32 m=nOrder; m>=0; m--)
    {
      second = second*root + first;
      first = first*root + polynomial;
      polynomial = polynomial*root + coefficients[m];
      error = error*absRoot + abs(polynomial); // error is the integral of polynomial evaluation errors
    }
    
    error *= epsilon;  // Worst case estimate of round off error in estimating
    
    if(abs(polynomial) == 0 || (abs(polynomial) < error && extraPasses++ == 2))
      break; // We are within desired precision of root
    
    // Laguerres distance calculation
    CDBL G = first/polynomial;
    CDBL H = G*G - second/polynomial;
    
    CDBL sqRoot = sqrt((nOrder - 1.) * (double(nOrder)*H - G*G));
    
    CDBL denominatorPlus = G + sqRoot;
    CDBL denominatorMinus = G - sqRoot;
    
    if(abs(denominatorPlus) > abs(denominatorMinus))
      distance = double(nOrder)/denominatorPlus;
    else
      distance = double(nOrder)/denominatorMinus; 
    
    CDBL rootOld = root;
    
    root = root - distance;
    if(root == rootOld)
      break; // we are as close as we are going to get

    if(iter++ % nudge == 0)
    {
      // We are not converging nudge root (it should be somewhere around here)
      root += distance*nudges[iter/nudge]; 
      extraPasses = 0;
    }
  }

  if(fabs(root.imag()) < abs(root)*error)
    root = root.real();

  if(iterations)
    *iterations = iter;

  return iter >= maxIter;
}

using std::copy;
using std::sort;

bool lessAbs(CDBL &first, CDBL &second)
{
  return abs(first) < abs(second);
}

bool roots(double Coeff[], int32 nOrder, CDBL Roots[], CDBL Seed[], int32 nSeeds, bool bRefine, bool bPolish)
{
  vector<CDBL> deflatedCoefficients;

  deflatedCoefficients.reserve(nOrder+1);

  copy(Coeff, Coeff+nOrder+1, back_inserter(deflatedCoefficients));

  ASSERT(deflatedCoefficients.size() == unsigned(nOrder+1));

  int32 nRoot = 0;

  for(int32 nSeed = 0; nRoot < nOrder; nSeed++)
  {
    CDBL root;
    if(nSeed < nSeeds)
      root = Seed[nSeed];
    else 
      root = 1;

    int32 nDeflatedOrder = nOrder - nRoot;

    bool error = Laguerre(&deflatedCoefficients[0], nDeflatedOrder, &root);

    if(error && nSeed < nSeeds)
      continue; // this seed failed to converge skip it

    if(error)
      return true;  // We do not have any more initial guesses
      
    Roots[nRoot++] = root;

    CDBL remainder = 0;

    for(int32 n = nDeflatedOrder; n >= 0; n--)
    {
      CDBL c = deflatedCoefficients[n];
      deflatedCoefficients[n] = remainder;
      remainder = c + remainder*root;
    }
  }

  if(bRefine)
  {
    // According to Numerical Recipes in C 9.5, 
    // we should deflate our polynomial in order of increasing abs(root)
    // So here we sort our previous results and recalculate in the more stable order.
    vector<CDBL> sortedSeeds;

    copy(&Roots[0], &Roots[0]+nOrder, back_inserter(sortedSeeds));

    sort(sortedSeeds.begin(), sortedSeeds.end(), lessAbs);
    
    roots(Coeff, nOrder, Roots, &sortedSeeds[0], nOrder, false, false);

#ifdef ROOT_DISTANCE
    double distance;
    double error;
    for( int32 n = 0; n < nOrder; n++)
    {
      distance = abs(Roots[n] - sortedSeeds[n]);
      error = distance/abs(Roots[n]);
    }
#endif
  }

  if(bPolish)
  {
    // Deflation causes some inherent accuracy loss
    // Recalculate the roots using the undeflated polynomial (called polishing)
    const double maxError = sqrt(std::numeric_limits<double>::epsilon());

    copy(Coeff, Coeff+nOrder+1, deflatedCoefficients.begin()); // overwrite coefficients
    for( int32 n = 0; n < nOrder; n++)
    {
      CDBL root = Roots[n];
      Laguerre(&deflatedCoefficients[0], nOrder, &root);

      double distance = abs(Roots[n] - root);
      double error = distance/abs(Roots[n]);

      if(error < maxError)  // Suspect root loss
        Roots[n] = root;
    }
  }
  
  return false;
}
