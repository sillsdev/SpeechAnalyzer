#define MATHX_CPP
#include "stddsp.h"
#include <math.h>
#include "MathX.h"


/////////////////////////////////////////////////////////////////////////////
// other functions

double Round(double x)
{
    return floor(x + 0.5);
}

static char Copyright[] = {"Copyright (c) 1986-1992 Numerical Recipes Software\n"};
float BessI0(float x)
{
    float ax,ans;
    double y;

    if ((ax=(float)fabs(x)) < 3.75)
    {
        y=x/3.75;
        y*=y;
        ans=(float)(1.0+y*(3.5156229+y*(3.0899424+y*(1.2067492
                                        +y*(0.2659732+y*(0.360768e-1+y*0.45813e-2))))));
    }
    else
    {
        y=3.75/ax;
        ans=(float)((exp(ax)/sqrt(ax))*(0.39894228+y*(0.1328592e-1
                                        +y*(0.225319e-2+y*(-0.157565e-2+y*(0.916281e-2
                                                +y*(-0.2057706e-1+y*(0.2635537e-1+y*(-0.1647633e-1
                                                        +y*0.392377e-2)))))))));
    }
    return ans;
}

bool IsSingleBitOn(unsigned short data)
{
    bool result = false;     // default to false

    while (data != 0)
    {
        if (data==1)
        {
            result = true;
        }
        if (data&1)
        {
            break;
        }
        data = data >> 1;
    }
    return(*(bool *)&result);
}

