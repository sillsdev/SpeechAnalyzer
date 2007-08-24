#define ZROOTS_C

#ifndef SHOW_ALL_WARNINGS
#pragma message("reminder " __FILE__ "(20) : reminder : some warning messages disabled" )
#pragma warning(disable : 4244) // warning C4244: 'initializing' : conversion from 'int' to 'unsigned short', possible loss of data
#endif

//#define DUMP_ROOTS
#include <stdio.h>
#include <math.h>
#include "complex.h"
#include "error.h"
//#define NRANSI
#include "nrutil.h"
#define EPSS 1.0e-7F
#define MR 8
#define MT 10
#define MAXIT (MT*MR)

#error 

ERR laguer(COMPLEX_RECT_FLOAT a[], int m, COMPLEX_RECT_FLOAT *x, int *its)
{
	int iter,j;
	float abx,abp,abm,err;
	COMPLEX_RECT_FLOAT dx,x1,b,d,f,g,h,sq,gp,gm,g2;
	static float frac[MR+1] = {0.0F,0.5F,0.25F,0.75F,0.13F,0.38F,0.62F,0.88F,1.0F};

	for (iter=1;iter<=MAXIT;iter++) 
	{
		*its=iter;
		b=a[m];
		err=Cabs(b);
		d=f=Complex(0.0,0.0);
		abx=Cabs(*x);
		for (j=m-1;j>=0;j--) 
		{
			f=Cadd(Cmul(*x,f),d);
			d=Cadd(Cmul(*x,d),b);
			b=Cadd(Cmul(*x,b),a[j]);
			err=Cabs(b)+abx*err;
		}
		err *= EPSS;
		if (Cabs(b) <= err) return(DONE);
		g=Cdiv(d,b);
		g2=Cmul(g,g);
		h=Csub(g2,RCmul(2.0,Cdiv(f,b)));
		sq=Csqrt(RCmul((float) (m-1),Csub(RCmul((float) m,h),g2)));
		gp=Cadd(g,sq);
		gm=Csub(g,sq);
		abp=Cabs(gp);
		abm=Cabs(gm);
		if (abp < abm) gp=gm;
		dx=((FMAX(abp,abm) > 0.0 ? Cdiv(Complex((float) m,0.0),gp)
			: RCmul(exp(log(1+abx)),Complex(cos((float)iter),sin((float)iter)))));
		x1=Csub(*x,dx);
		if (x->real == x1.real && x->imag == x1.imag) return(DONE);
		if (iter % MT) *x=x1;
		else *x=Csub(*x,RCmul(frac[iter/MT],dx));
	}
	return(Code(FAILED_TO_CONVERGE));
}
#undef EPSS
#undef MR
#undef MT
#undef MAXIT
#undef NRANSI
/* (C) Copr. 1986-92 Numerical Recipes Software *!>p.#. */


#include <math.h>
#include <malloc.h>
#include "typedefs.h"
#include "zroots.h"
#include "error.h"
#define EPS 2.0e-6


ERR zroots(double Coeff[], int nOrder, COMPLEX_RECT_FLOAT Roots[], 
		   COMPLEX_RECT_FLOAT Seed[], int nSeeds,
		   BOOLFLAG bRefine, BOOLFLAG bSort)
{
	int i, j, j1, k, nIterations;
	COMPLEX_RECT_FLOAT Root, OldCoeff, NewCoeff;
	COMPLEX_RECT_FLOAT *ComplexCoeff, *DeflatedCoeff;
	ERR Err;
#ifdef DUMP_ROOTS
    FILE *RootsDump;
	FILE *CoeffDump;
	CoeffDump = fopen("coeff.txt", "w");
	for (i=0; i<= nOrder; i++) fprintf(CoeffDump, "%f ", Coeff[i]);
	fclose(CoeffDump);
#endif
    if (!Coeff) return(Code(INVALID_PARM_PTR));
    if (!nOrder) return(Code(INVALID_ORDER));
	if (!Roots) return(Code(INVALID_PARM_PTR));
	if (nSeeds && !Seed) return(Code(INVALID_PARM_PTR));
	
	ComplexCoeff = calloc(nOrder+1, sizeof(COMPLEX_RECT_FLOAT));	
	if (!ComplexCoeff) return(Code(OUT_OF_MEMORY));
	DeflatedCoeff = calloc(nOrder+1, sizeof(COMPLEX_RECT_FLOAT));
	if (!DeflatedCoeff) return(Code(OUT_OF_MEMORY));

	for (j=0; j <= nOrder; j++) ComplexCoeff[j]= DeflatedCoeff[j] = Complex((float)Coeff[j], 0.F);
	for (j=nOrder, k=0; j>=1; j--) 
	{
		if (k < nSeeds) Root = Seed[k++];
		else Root=Complex(0.0,0.0);
		Err = laguer(DeflatedCoeff,j,&Root,&nIterations);
		if (Err) 
		{
			free(ComplexCoeff);
			free(DeflatedCoeff);
			return(Err);
		}
		if (fabs(Root.imag) <= 2.0*EPS*fabs(Root.real)) Root.imag=0.0;
		Roots[j-1]=Root;
		NewCoeff=DeflatedCoeff[j];
		for (j1=j-1; j1>=0; j1--) 
		{
			OldCoeff=DeflatedCoeff[j1];
			DeflatedCoeff[j1]=NewCoeff;
			NewCoeff=Cadd(Cmul(Root,NewCoeff),OldCoeff);
		}
	}
	if (bRefine)
		for (j=0; j<nOrder; j++)
			laguer(ComplexCoeff,nOrder,&Roots[j],&nIterations);

#ifdef DUMP_ROOTS
    bSort = 1; 
#endif
    if (bSort)
		for (j=1; j<nOrder; j++) 
		{
			Root=Roots[j];
			for (i=j-1;i>=0;i--) 
			{
				if (Roots[i].real <= Root.real) break;
				Roots[i+1]=Roots[i];
			}
			Roots[i+1]=Root;
        }
#ifdef DUMP_ROOTS
	RootsDump = fopen("roots.txt", "w");
	for (i = 0; i < nOrder; i++)
	{
	  if (Roots[i].imag >=0.F) fprintf(RootsDump, "%f+%fi\n", Roots[i].real, Roots[i].imag);
	  else fprintf(RootsDump, "%f-%fi\n", Roots[i].real, fabs(Roots[i].imag));
	}
	fclose(RootsDump);
#endif
	free(ComplexCoeff);
	free(DeflatedCoeff);
	return(DONE);
}
#undef EPS
/* (C) Copr. 1986-92 Numerical Recipes Software *!>p.#. */
