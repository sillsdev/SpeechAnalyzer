/************** include file for GRAPPL *************/
#ifndef _GRAPPL_H_

#include "dspTypes.h"

#define _GRAPPL_H_

/* calculation parameters */
#define Grappl_calc_intvl   100
#define Grappl_scale_factor  10

/* calculation modes */
#define  Grappl_magnitude  1
#define  Grappl_rawpitch    2
#define  Grappl_fullpitch  4

/* error returns */
#define  E_noerror  0		/* no error occurred */
#define  E_idle    1		/* not currently initialised */
#define  E_badin    2		/* grapplSetInbuff called with previous input data still pending */
#define  E_badfrange  3		/* 'minpitch' or 'maxpitch' outside permitted range of 20-1000Hz */
#define  E_badfgap  4		/* 'maxpitch' less than 25% greater than 'minpitch' */
#define  E_badmode  5		/* 'mode' parameter setting invalid */
#define  E_badsmooth  6		/* 'smoothfreq' outside permitted range (500-2500Hz, or 0=no smoothing) */
#define  E_badsfreq  7		/* 'sampfreq' outside permitted range (5000-50000Hz) */
#define  E_badreslag  8		/* 'reslag' value invalid (must be at least 10, or 0=use max possible) */
#define  E_internal  99		/* invalid internal state: report to ICS */

typedef void * pGrappl;		/* generic 32-bit pointer */

/* user input parameters */
typedef  struct
{
    int32 sampfreq;			/* acoustic data sampling frequency */
    int16 eightbit;			/* true for 8-bit data */
    int16 mode;				/* calculation mode */
    int32 smoothfreq;		/* moving-average filter equivalent freq */
    int16 minpitch;			/* min permissible pitch frequency */
    int16 maxpitch;			/* max permissible pitch frequency */
    int16 calcint;			/* result calculation interval in data samples */
    int16 minvoiced16;		/* min smoothed amplitude for voicing (x16) */
    int16 maxchange_pc10;	/* max % change (x10) between successive pitch res */
    int16 minsigpoints;		/* min points in significant pitch contour */
    int16 minmeanweight;	/* min mean weight for significant pitch contour */
    int16 maxinterp_pc10;	/* max % change (x10) to join contours across 1-pt gap */
    int16 reslag;			/* max results to hold in store (0=maximum possible) */
} Grappl_parms;
typedef Grappl_parms * pGrappl_parms;

/* output result structure */
typedef  struct
{
    int16  mag16;      /* short-term abs magnitude (8-bit x16, 16-bit /16) */
    int16  fcalc16;    /* best guess at point pitch frequency (x16) */
    int16  fselect16;    /* corrected guess at point pitch frequency within contour (x16) */
    int16  fsmooth16;    /* smoothed pitch contour point (x16) */
    int16  acfcoeff;     /* autocorrelation function coefficient for best lag */
} Grappl_res;

typedef Grappl_res * pGrappl_res;

/* function prototypes */
int16  grapplGetError(pGrappl);
bool grapplGetResults(pGrappl work,pGrappl_res * results,int16 * numResults,bool  * alldone);
bool grapplInit(pGrappl,pGrappl_parms);
bool grapplSetInbuff(pGrappl work, pGrappl data, uint16 length, bool allDone);
uint32 grapplWorkspace(void);

#endif //_GRAPPL_H_
