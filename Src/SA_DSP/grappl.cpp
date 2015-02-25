/*************************************************************************

  GRAPPL - Generic Rapid Accurate Pitch Period Locator
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  Copyright (c) 1997 P.R.Brassett for SIL

  History:

  14-02-97 v1.0  Coded in new continuous form by PRB, for initial
      use in WinCECIL v2.2 Beta 1
  20-04-01 TRE spring cleaning

*************************************************************************/
#include  "grappl.h"
#include <memory.h>

#define    UNSET    -1
#define    UUNSET    65535U  /* UNSET for unsigned data type */

typedef uint8 * Ucharptr;
typedef int16 * Shortptr;

//#define    SysParams    (((pWorkspace)(work))->sysparms)
#define    Maxdata    0x2000  /* must be power of 2 */
#define    Dataindx(i)  (((uint16)(i))&0x1FFF)
#define    Wave(i)    (((pWorkspace)(work))->wave[Dataindx(i)])
#define    sWave(i)  (((pWorkspace)(work))->swave[Dataindx(i)])
#define    Maxcross  0x0800 /* must be power of 2 */
#define    Crossindx(i)  (((uint16)(i))&0x07FF)
#define    Cross(i)  (((pWorkspace)(work))->cross[Crossindx(i)])
#define    Maxres    1400 // SDM changed from 1500 4/26/2001 Arbitrary fix
#define    Ures(i)    (((pWorkspace)(work))->userres[i])
#define    Sres(i)    (((pWorkspace)(work))->sysres[i])
#define    Lag(i)    (((pWorkspace)(work))->lagelem[i])

/* GRAPPL states */
enum EGRAPPL_STATES {  Idle,WantIn,WantOut };

/* hardcoded parameter values */
enum EGRAPPL_PARAMS
{
    Badcoeff=-20000,		/* invalid acf coeff */
    Baseweighting=20,		/* base for freq reliability weights for contouring algorithm */
    NsampAcf=15,			/* no. of samples for acf calcs */
    NsampMag=21,			/* no. of samples for magnitude calcs */
    Maxlag=10,				/* max. zero-crossing lags to consider */
    Optimrange_pc100=500,	/* optimise best zero-crossing acfs +/- 5% */
    Nsmoothpass=2			/* no. of smoothing passes to generate smooth16 */
};

/* private data structures */
/* acf data for particular zero-crossing interval */
typedef struct SLagelem
{
    int16  nxing;			/* no of zero-crossings this dist represents */
    int16  coeff;			/* acf coeff for this dist */
    uint16 dist16;			/* dist in data samples (x16) */
} Lagelem;
typedef Lagelem * pLagelem;

/* raw pitch estimate */
typedef struct SFraw
{
    int16  pitch16;			/* pitch estimate in Hz (x16) */
    int16  coeff;			/* associated acf coeff */
    int16  weight;			/* associated weight */
} Fraw;
typedef Fraw * pFraw;

/* private intermediate results data */
typedef struct SSysres
{
    uint8 voiced;			/* data is voiced */
    uint8 selected;			/* used in locating tone contours */
    int16  magnitude;		/* smoothed magnitude in range 0-100 */
    int16  weight;			/* final point weight */
    Fraw  fraw[2];			/* raw pitch estimates */
} Sysres;
typedef Sysres * pSres;

/* pitch contour tracking */
typedef struct SRoute
{
    int16  ipos,isubpos;
    int16  itarget;
    int16  direc;
    int32  weight,subweight;
    int32  magnitude,submag;
    int16  pitch16;
    int16  prevpitch16;
    int16  confirm;
} Route;

struct SSysparms
{
    /* overall flow control */
    int16 state;		/* Idle, Wantin or Wantout */
    int16 error;		/* error state */

    /* user parameters */
    int16 reslag;
    int16 maxinterp_pc10;
    int16 minmeanweight;
    int16 minsigpoints;
    int16 maxchange_pc10;
    int16 minvoiced16;
    int16 calcint;
    int16 maxpitch;
    int16 minpitch;
    int16 mode;
    int16 eightbit;
    int32 smoothfreq;
    int32 sampfreq;

    /* derived parameters */
    uint16 minlen16;			/* min pitch period (x16) (from maxpitch) */
    uint16 maxlen16;			/* max pitch period (x16) (from minpitch) */
    uint16 acfrange;			/* span of acf calculations */
    uint16 magrange;			/* span of magnitude calcs */
    uint16 smagrange;			/* span of mangitude calcs on smoothed wavesform */

    /* public input buffer tracking */
    int16 inalldone;			/* all input data supplied */
    pGrappl inptr;				/* pointer to next input data value */
    uint16 incount;				/* input data values pending */
    uint32 dataindx;			/* indx of data sample corresponding to next calc pos */

    /* private input buffer management */
    uint16 loc_bytesreqd;
    uint16 loc_isubtract,loc_iadd;
    uint32 loc_lcentre,loc_lpos;
    uint16 loc_filterlen;
    int32 loc_filtersum;
    int16 loc_positive;

    /* zero crossing data */
    int16 xindx;				/* next indx for zero-crossing data */
    int16 xcount;				/* number of valid zero-crossings in array */

    /* pitch calculation */
    int16 nlag;
    Lagelem prev,prevprev,prev2;
    Lagelem bestlag,bestlag2;

    /* private calculation vars */
    uint16 loc_mstep,loc_mstartpos;
    uint16 loc_smstep,loc_smstartpos;
    int16 loc_maxlow,loc_maxhigh;

    /* results data */
    int16 ncalc;				/* number of valid results in output buffer */
    int16 nres;					/* number of results returned to user */
    int16 calcpending;			/* number of results pending compared with data in */
    int16 calcfrac;				/* data samples over */
    int16 outalldone;			/* true if all results returned */

    /* results smoothing */
    int16 lastfsmooth;			/* used for joining smooth16 across unvoiced sections */
    int16 prevwt[2];			/* weights of select16 values before start of res block */
    int16 prevfch[2];			/* select16 values preceding this results block */
};
typedef SSysparms * Sysparmsptr;

struct SWorkspace
{
    int32 check;				/* check double word */
    int8 wave[Maxdata];			/* private (wrapping) buffer of waveform data */
    int8 swave[Maxdata];		/* ditto, smoothed waveform */
    uint32 cross[Maxcross];		/* (wrapping) buffer of zero-crossing posns */
    SGrapplResult userres[Maxres]; /* buffer for results returned to user */
    Sysres sysres[Maxres];		/* buffer for related intermediate results */
    Lagelem lagelem[Maxlag+5];  /* potential raw pitch estimates */
    SSysparms sysparms;			/* all other 'static' variables */
};

typedef SWorkspace * pWorkspace;

/* function prototypes */
static bool update_data(pGrappl,int16);
static bool update_results(pGrappl,int16);

static void calculate(pGrappl,int16);
static int16 calculate_acf16(pGrappl,uint16);
static void calculate_addprev(pGrappl);
static void calculate_choosebest2(pGrappl);
static void calculate_lagacfs(pGrappl);
static int16 calculate_magnitude(pGrappl,int16);
static void calculate_nextgap(pGrappl,int16 *,int16 *,int16);
static void calculate_optimise(pGrappl,pLagelem,int16,int16);
static void calculate_pitch(pGrappl);
static void calculate_selectbest(pGrappl);
static void calculate_voiced(pGrappl);
static void calculate_weights(pGrappl);

static void magnitude(pGrappl,int16);

static void select(pGrappl);
static int16 select_continuous(pGrappl,int16,int16,int16);
static void select_section(pGrappl,int16,int16);
static int16 select_route(pGrappl,int16,int16,int16 *,int16 *,int16 *,int16 *);
static int16 select_routenext(pGrappl,Route *);

static void smooth(pGrappl);
static void smooth_section(pGrappl,int16,int16,int16,int16);

static void parse(pGrappl);


/**** PUBLIC ROUTINES ********************************************************/

/* generate 'Divide by Zero' on bad work pointer */
#define CHECKVAL 123456789L
static void Check(pGrappl work)
{
    if (((pWorkspace)(work))->check != CHECKVAL)
    {
        int16 i=2;
        i=2/(i-2);
    }
}


int16 grapplGetError(pGrappl work)
{
    /* return current error state */
    SSysparms * SysParams = &(((pWorkspace)(work))->sysparms);

    Check(work);

    return(SysParams->error);
}

bool grapplGetResults( pGrappl work,pGrappl_res * res,int16 * nres, bool * alldone)
{
    /* return next block of results */
    SSysparms * SysParams = &(((pWorkspace)(work))->sysparms);
    Check(work);

    *nres=0;
    /* check expecting this call */
    if (SysParams->state != WantOut)
    {
        if (SysParams->state == Idle)
        {
            SysParams->error=E_idle;
        }
        return false;
    }
    else
    {
        if (update_results(work,false))
        {
            /* results returned */
            *res=&(Ures(0));
            *nres=SysParams->nres;

            if (SysParams->outalldone)
            {
                /* flag all done */
                *alldone=true;
                SysParams->state=Idle;
            }
            return true;
        }
        else
        {
            /* need more input data */
            SysParams->state=WantIn;
            return false;
        }
    }
}

bool grapplInit(pGrappl work,pGrappl_parms parms)
{
    /* initialise GRAPPL invocation */
    SSysparms * SysParams = &(((pWorkspace)(work))->sysparms);
    ((pWorkspace)(work))->check=CHECKVAL;

    SysParams->state=WantIn;
    SysParams->error=false;

    /* copy across and check/modify user parameters */
    SysParams->reslag=(parms->reslag == 0?Maxres:parms->reslag);
    if (SysParams->reslag < 10)
    {
        SysParams->error=E_badreslag;
    }
    if (SysParams->reslag > Maxres)
    {
        SysParams->reslag=Maxres;
    }
    SysParams->maxinterp_pc10=(parms->maxinterp_pc10 < 0?0:parms->maxinterp_pc10);
    SysParams->minmeanweight=(parms->minmeanweight < 0?0:parms->minmeanweight);
    SysParams->minsigpoints=(parms->minsigpoints < 1?1:parms->minsigpoints);
    SysParams->maxchange_pc10=(parms->maxchange_pc10 < 0?0:parms->maxchange_pc10);
    SysParams->minvoiced16=(parms->minvoiced16 < 0?0:parms->minvoiced16);
    SysParams->calcint=(parms->calcint < 1?1:parms->calcint);
    SysParams->maxpitch=parms->maxpitch;
    SysParams->minpitch=parms->minpitch;
    if ((SysParams->maxpitch < 20) || 
		(SysParams->minpitch < 20) || 
		(SysParams->maxpitch > 4000) ||
        (SysParams->minpitch > 1000))
    {
        SysParams->error=E_badfrange;    // CLW 4/5/00
    }
    else
    {
        if (SysParams->minpitch *5 > SysParams->maxpitch *4)
        {
            SysParams->error=E_badfgap;
        }
    }
    SysParams->mode=parms->mode;
    if ((SysParams->mode == 0) || 
		(SysParams->mode & ~(Grappl_magnitude|Grappl_rawpitch|Grappl_fullpitch)) != 0)
    {
        SysParams->error=E_badmode;
    }
    SysParams->eightbit=parms->eightbit;
    SysParams->smoothfreq=parms->smoothfreq;
    if ((SysParams->smoothfreq != 0L) && 
		((SysParams->smoothfreq < 500L) || (SysParams->smoothfreq > 22000L)))
    {
        SysParams->error=E_badsmooth;    // CLW 4/5/00
    }
    SysParams->sampfreq=parms->sampfreq;
    if ((SysParams->sampfreq < 5000L) || (SysParams->sampfreq > 96000L))
    {
        SysParams->error=E_badsfreq;
    }

    if (! SysParams->error)
    {
        /* calculate derived variables */
        SysParams->minlen16=(uint16)((SysParams->sampfreq*16L+SysParams->maxpitch/2)/ SysParams->maxpitch);
        SysParams->maxlen16=(uint16)((SysParams->sampfreq*16L+SysParams->minpitch/2)/ SysParams->minpitch);
        SysParams->acfrange=(SysParams->maxlen16+8)/16;
        SysParams->magrange=SysParams->acfrange;
        SysParams->smagrange=SysParams->magrange/2;

        SysParams->inalldone=SysParams->outalldone=false;
        SysParams->inptr=0L;
        SysParams->incount=0;
        SysParams->xindx=0;
        SysParams->xcount=0;
        SysParams->ncalc=SysParams->nres=0;
        SysParams->calcpending=0;
        SysParams->calcfrac=SysParams->calcint-1;
        SysParams->lastfsmooth=UNSET;
        SysParams->prevwt[0]=SysParams->prevwt[1]=0;
        SysParams->prevfch[0]=SysParams->prevfch[1]=UNSET;

        /* initialise system */
        if (!update_results(work,true))
        {
            SysParams->error=E_internal;
        }
    }
    return (!SysParams->error);
}

/**
* work pointer to output buffer
* data pointer to input data
* length length of input data
* alldone true if this is the last sample
*/
bool grapplSetInbuff( pGrappl work, pGrappl data, uint16 length, bool alldone)
{
    /* declare more input data */
    SSysparms * SysParams = &(((pWorkspace)(work))->sysparms);
    int32 pending;

    Check(work);

    /* check expecting input data */
    if (SysParams->state != WantIn)
    {
        SysParams->error=(SysParams->state == Idle?E_idle:E_badin);
        return false;
    }
    /* reset input data variables */
    SysParams->inptr=data;
    SysParams->incount=length;
    pending=(int32)SysParams->calcfrac+length;
    SysParams->calcfrac=(int16)(pending % SysParams->calcint);
    SysParams->calcpending+=(int16)(pending/SysParams->calcint);
    if (alldone)
    {
        SysParams->inalldone=true;
    }
    SysParams->state=WantOut;
    return true;
}

uint32 grapplWorkspace(void)
{
    /* return space to allocate as workspace, plus safety margin */
#define  WDEBUG 0
#if  WDEBUG
    {
        /* on first call, print details of workspace component sizes */
        enum{ Tab1=16,Tab2=23 };
        static int16 init=true;

        if (init)
        {
            init=false;
            io_io("\r\ncheck% %i\r\n",Tab1,sizeof(int32));
            io_io("wave[%i]% %i\r\n",Maxdata,Tab1,Maxdata*sizeof(int8));
            io_io("swave[%i]% %i\r\n",Maxdata,Tab1,Maxdata*sizeof(int8));
            io_io("cross[%i]% %i\r\n",Maxcross,Tab1,Maxcross*sizeof(int32));
            io_io("userres[%i]% %i% (%i*%i)\r\n",Maxres,Tab1,
                  Maxres*sizeof(Grappl_res),Tab2,Maxres,
                  sizeof(Grappl_res));
            io_io("sysres[%i]% %i% (%i*%i)\r\n",Maxres,Tab1,
                  Maxres*sizeof(Sysres),Tab2,Maxres,Maxres,
                  sizeof(Sysres));
            io_io("lagelem[%i]% %i\r\n",Maxlag+5,Tab1,(Maxlag+5)*
                  sizeof(Lagelem));
            io_io("sysparms% %i\r\n",Tab1,sizeof(Sysparms));
            io_io("--------\r\nWorkspace% %l\r\n",Tab1,
                  (int32)sizeof(Workspace));
        }
    }
#endif
    return(sizeof(SWorkspace)+10U);
}

/**** UPDATE ************************************************************/
static bool update_data(pGrappl work,int16 init)
{
    /* get enough data in to calculate results for next point */
    SSysparms * SysParams = &(((pWorkspace)(work))->sysparms);
    if (init)
    {
        /* setup sampling widths etc, and calc how much data needs to be read
          before ready to calculate results for first point */
        uint16 sampwidth,i;

        SysParams->loc_filterlen=(uint16)(SysParams->smoothfreq?(SysParams->sampfreq+
                                          SysParams->smoothfreq/2)/SysParams->smoothfreq:1); // CLW 4/5/00
        sampwidth=(uint16)(((SysParams->maxlen16>>3)+SysParams->loc_filterlen)*
                           11L/10L+20);
        if (sampwidth >= Maxdata)
        {
            return(false);    /* fatal error */
        }
        SysParams->loc_bytesreqd=sampwidth/2;
        /* initialise buffer contents and moving-average filter */
        for (i=0; i < SysParams->loc_bytesreqd; i++)
        {
            Wave(i)=0;
            sWave(i)=0;
        }
        SysParams->loc_filtersum=128*SysParams->loc_filterlen;
        /* initialise indices into data buffers */
        SysParams->loc_iadd=SysParams->loc_bytesreqd;
        SysParams->loc_lpos=SysParams->loc_iadd;
        SysParams->loc_isubtract=SysParams->loc_iadd-SysParams->loc_filterlen;
        SysParams->loc_lcentre=SysParams->loc_isubtract+SysParams->loc_filterlen/2;
        SysParams->loc_positive=UNSET;
        return(true); /* initialisation successful */
    }
    else
    {
        /* normal case; return true if enough data available to do next calculation */
        int8 ch,sch;
        int16 smooth=(SysParams->mode & (Grappl_rawpitch|Grappl_fullpitch));

        while (SysParams->loc_bytesreqd && SysParams->incount)
        {
            /* read in next data sample and convert to signed char */
            if (SysParams->eightbit)
            {
                ch=(int8)(int16(*Ucharptr(SysParams->inptr))              -128);
                SysParams->inptr = Ucharptr(SysParams->inptr) + 1;
            }
            else
            {
                ch=(int8)(((int32(*Shortptr(SysParams->inptr))+0x8000L)>>8) -128);
                SysParams->inptr = Shortptr(SysParams->inptr) + 1;
            }
            /* copy into raw waveform buffer */
            Wave(SysParams->loc_iadd)=ch;
            if (smooth)
            {
                /* get next smoothed waveform value */
                sch=(SysParams->loc_filterlen?(int8)(SysParams->loc_filtersum/
                                                     SysParams->loc_filterlen-128):ch);
                sWave(SysParams->loc_lcentre)=sch;
                SysParams->loc_filtersum+=(int16)ch-(int16)
                                          Wave(SysParams->loc_isubtract);
                /* track neg/pos transitions */
                if (SysParams->loc_positive == UNSET)
                {
                    SysParams->loc_positive=(sch > 0);
                }
                if (sch > 0)
                {
                    if (! SysParams->loc_positive)
                    {
                        /* neg->pos transition located - store it */
                        Cross(SysParams->xindx)=SysParams->loc_lcentre;
                        SysParams->xindx++;
                        if (SysParams->xcount < Maxcross)
                        {
                            SysParams->xcount++;
                        }
                        SysParams->loc_positive=true;
                    }
                }
                else
                {
                    SysParams->loc_positive=false;
                }
                SysParams->loc_lcentre++;
                SysParams->loc_isubtract++;
            }
            SysParams->loc_iadd++;
            SysParams->loc_bytesreqd--;
            SysParams->incount--;
        }
        if (SysParams->loc_bytesreqd)
        {
            /* no more bytes left in input buffer but more needed */
            if (SysParams->inalldone)
            {
                /* passed end of data - pad with zeros */
                while (SysParams->loc_bytesreqd)
                {
                    Wave(SysParams->loc_iadd)=0;
                    if (smooth)
                    {
                        SysParams->loc_filtersum-=(int16)
                                                  Wave(SysParams->loc_isubtract);
                        sWave(SysParams->loc_lcentre)=(int8)
                                                      (SysParams->loc_filtersum/
                                                       SysParams->loc_filterlen);
                        SysParams->loc_lcentre++;
                        SysParams->loc_isubtract++;
                    }
                    SysParams->loc_iadd++;
                    SysParams->loc_bytesreqd--;
                }
            }
            else
            {
                return(false);    /* need more input data */
            }
        }

        /* data buffers ready for next result calc */
        SysParams->dataindx=SysParams->loc_lpos; /* centre of data for calc purposes */
        SysParams->loc_lpos+=SysParams->calcint;
        SysParams->loc_bytesreqd=SysParams->calcint; /* reset for next call */
        return(true);
    }
}

static bool update_results(pGrappl work,int16 init)
{
    /* attempt to fill result buffer with fresh results */
    SSysparms * SysParams = &(((pWorkspace)(work))->sysparms);
    if (init)
    {
        /* initialise system */
        magnitude(work,true);
        calculate(work,true);
        return(update_data(work,true));
    }
    else
    {
        /* normal case */
        int16 i,results,calcdone;

        /* move up any pending results */
        if (SysParams->nres < SysParams->ncalc)
        {
            for (i=SysParams->nres; i < SysParams->ncalc; i++)
            {
                Ures(i-SysParams->nres)=Ures(i);
                Sres(i-SysParams->nres)=Sres(i);
            }
        }
        /* adjust counters */
        SysParams->ncalc-=SysParams->nres;
        SysParams->nres=0;

        /* get new raw results */
        while (SysParams->ncalc < SysParams->reslag && SysParams->calcpending)
        {
            /* get enough wave data for next calc */
            if (! update_data(work,false))
            {
                break;
            }
            /* calculate mag16 and/or fcalc16 results */
            if (SysParams->mode & Grappl_magnitude)
            {
                magnitude(work,false);
            }
            if (SysParams->mode & (Grappl_rawpitch|Grappl_fullpitch))
            {
                calculate(work,false);
            }
            SysParams->ncalc++;
            SysParams->calcpending--;
        }
        /* decide whether to return results yet or not */
        calcdone=(SysParams->inalldone && SysParams->calcpending == 0);
        results=(SysParams->ncalc >= SysParams->reslag || (calcdone && SysParams->ncalc > 0));
        if (results)
        {
            /* going to return results to user */
            SysParams->nres=SysParams->ncalc;
            if (SysParams->mode & Grappl_fullpitch)
            {
                /* do pitch contour tracking */
                select(work);
                if (! calcdone)
                {
                    parse(work);    /* decide where to break (resets SysParams->nres) */
                }
                smooth(work);
            }
            if (calcdone)
            {
                SysParams->outalldone=true;
            }
            return true;
        }
        else
        {
            return false;
        }
    }
}

/**** MAGNITUDE *********************************************************/

static  void  magnitude(pGrappl work,int16 init)
{
    /* calc abs magnitude x16 for current point */
    SSysparms * SysParams = &(((pWorkspace)(work))->sysparms);
    if (init)
    {
        /* initialise system */
        SysParams->loc_mstep=SysParams->magrange/(NsampMag-1);
        SysParams->loc_mstartpos=((SysParams->loc_mstep*(NsampMag-1))>>1);
    }
    else
    {
        int16 i,val,sum;
        uint16 pos;

        i=NsampMag;
        sum=0;
        pos=Dataindx(SysParams->dataindx)-SysParams->loc_mstartpos;
        do
        {
            val=Wave(pos);
            if (val < 0)
            {
                val=-val;
            }
            sum+=val;
            pos+=SysParams->loc_mstep;
        }
        while (--i);
        Ures(SysParams->ncalc).mag16=
            (int16)(((((int32)sum)<<4)+(NsampMag>>1))/NsampMag);
    }
}

/**** CALCULATE *********************************************************/

/* Location of the pitch period must be done as efficiently as possible.
   Auto-correlation coefficients are used, and calculation is reduced by
   searching for the pitch period close to the zero-crossing intervals around
   the calculation position. In case there is no zero-crossing corresponding to
   the true pitch period at this calculation position, the two previous pitch
   period estimates are also used as starting points to search for the pitch period.

   These estimates are thinned to the 2-3 most likely, and these are reduced to two
   which are optimised and returned as the two best guesses for fcalc16.
*/

static  void  calculate(pGrappl work,int16 init)
{
    SSysparms * SysParams = &(((pWorkspace)(work))->sysparms);
    if (init)
    {
        /* initialise system */
        SysParams->prev.coeff=SysParams->prev2.coeff=SysParams->prevprev.coeff=Badcoeff;
        (void)calculate_magnitude(work,true);
    }
    else
    {
        /* determine whether voiced or not */
        calculate_voiced(work);
        if (Sres(SysParams->ncalc).voiced)
        {
            /* get two fraw pitch estimates and return best as fcalc16 */
            calculate_pitch(work);
            Ures(SysParams->ncalc).fcalc16=Sres(SysParams->ncalc).fraw[0].pitch16;
            Ures(SysParams->ncalc).acfcoeff = Sres(SysParams->ncalc).fraw[0].coeff;
            /* shuffle along prev values */
            SysParams->prevprev=SysParams->prev;
            SysParams->prev=SysParams->bestlag;
            SysParams->prev2=SysParams->bestlag2;
        }
        else
        {
            /* this point unvoiced */
            Ures(SysParams->ncalc).fcalc16=UNSET;
            Ures(SysParams->ncalc).acfcoeff=UNSET;
            SysParams->prevprev=SysParams->prev;
            SysParams->prev.coeff=SysParams->prev2.coeff=Badcoeff;
        }
        /* calculate a 'likelihood' weight for each fraw pitch estimate */
        calculate_weights(work);
    }
}

static  void  calculate_addprev(pGrappl work)
{
    /* merge in previous fcalc16 wavelengths as starting estimates for acfs */
    SSysparms * SysParams = &(((pWorkspace)(work))->sysparms);
    int16 itest=0;
    int16 nxing=0;
    int16 percent10=0;
    int16 coeff=0;
    int16 j=0;
    int16 k=0;
    pLagelem lptr;
    uint16 dist16,mean;
    int32 diff;

    for (itest=0; itest < 3; itest++)
    {
        dist16=UUNSET;
        switch (itest)
        {
        case 0:  /* previous fcalc16 estimate */
            if (SysParams->prev.coeff > Badcoeff)
            {
                dist16=SysParams->prev.dist16;
                nxing=SysParams->prev.nxing;
            }
            break;

        case 1:  /* extrapolation from two prev fcalc16 estimates */
            if (SysParams->prev.coeff == Badcoeff ||
                    SysParams->prevprev.coeff == Badcoeff)
            {
                break;
            }
            diff=(int32)SysParams->prev.dist16-(int32)SysParams->prevprev.dist16;
            if (diff < 0)
            {
                diff=-diff;
            }
            mean=(SysParams->prev.dist16+SysParams->prevprev.dist16+1)>>1;
            percent10=(int16)((diff*1000L+(mean>>1))/mean);
            if (percent10 > 200 || percent10 < 10)
            {
                break;
            }
            /* get extrapolated value */
            dist16=(uint16)((((int32)(SysParams->prev.dist16)<<1))-(int32)SysParams->prevprev.dist16);
            /* check in range */
            if (dist16 < SysParams->minlen16 || dist16 >
                    SysParams->maxlen16)
            {
                dist16=UUNSET;
            }
            else
            {
                nxing=SysParams->prev.nxing;
            }
            break;

        case 2:  /* second best prev fcalc16 estimate */
            if (SysParams->prev2.coeff > Badcoeff)
            {
                dist16=SysParams->prev2.dist16;
                nxing=SysParams->prev2.nxing;
            }
            break;
        }
        if (dist16 == UUNSET)
        {
            continue;
        }

        /* fill in Lagelem fields maintaining correct dist16 order */
        coeff=calculate_acf16(work,dist16);
        for (j=0; j < SysParams->nlag; j++)
            if (Lag(j).dist16 > dist16)
            {
                break;
            }
        SysParams->nlag++;
        for (k=SysParams->nlag; k > j; k--)
        {
            Lag(k)=Lag(k-1);
        }
        lptr=&Lag(j);
        lptr->dist16=dist16;
        lptr->coeff=coeff;
        lptr->nxing=nxing;
    }
}

static  int16  calculate_acf16(pGrappl work,uint16 offset16)
{
    /* get auto-correlation coefficient for this interval at current data pos */
    SSysparms * SysParams = &(((pWorkspace)(work))->sysparms);
    int16 coeff,nsamp=NsampAcf;
    uint16 leftpos,step,whole,frac;
    int32 diffsum,abssum;

    /* centre sampling around utterance pos */
    step=SysParams->acfrange/(NsampAcf-1);
    whole=offset16>>4;
    frac=offset16&15;
    leftpos=Dataindx(SysParams->dataindx)-((step*(NsampAcf-1)+whole)>>1);

    if (frac)
    {
        /* acf lag is not integral number of data samples */
        uint16 rightpos;
        int16 val1,val2,diff;

        diffsum=abssum=0;
        do
        {
            val1=(sWave(leftpos))<<4;
            rightpos=leftpos+whole;
            /* interpolate second value */
            val2=sWave(rightpos)*(16-frac)+sWave(rightpos+1)*frac;
            diff=val1-val2;
            diffsum+=(diff < 0?-diff:diff);
            abssum+=(val1 < 0?-val1:val1)+(val2 < 0?-val2:val2);
            leftpos+=step;
        }
        while (--nsamp);
    }
    else
    {
        /* acf lag integral number of data samples, use faster code */
        int16 v1,v2,dsum=0,asum=0;

        do
        {
            v1=sWave(leftpos);
            v2=sWave(leftpos+whole);
            dsum+=(v1 < v2?v2-v1:v1-v2);
            asum+=(v1 < 0?-v1:v1)+(v2 < 0?-v2:v2);
            leftpos+=step;
        }
        while (--nsamp);
        diffsum=((int32)dsum)<<4;
        abssum=((int32)asum)<<4;
    }
    coeff=(int16)(10000L-(diffsum*10000L+(abssum>>1))/(1L+abssum));
    /* return coefficient in range 0 to 20000 */
    return(coeff);
}

static  void  calculate_choosebest2(pGrappl work)
{
    /* find best two of three pitch period estimates */
    SSysparms * SysParams = &(((pWorkspace)(work))->sysparms);
    int16 i,dist16,ires,ibest,best1,best2,temp;
    pLagelem lptr=&Lag(0);
    Fraw ftemp;
    pSres thiss=&Sres(SysParams->ncalc);
    enum { Coeff_fix=200 };

    /* make sure we have three valid coeffs */
    for (i=SysParams->nlag; i < 3; i++)
    {
        lptr[i].coeff=Badcoeff;
    }

    best1=best2=UNSET;

    switch (SysParams->nlag)
    {
    case 0:
        break;

    case 1:
        best1=0;
        break;

    case 2:
    case 3:
        ibest=(SysParams->nlag == 2?1:(lptr[1].coeff >
                                       lptr[2].coeff?1:2));
        if (lptr[0].coeff+Coeff_fix >= lptr[ibest].coeff)
        {
            best1=0;
            best2=ibest;
        }
        else
        {
            best1=ibest;
            best2=0;
        }
        break;
    }

    /* have chosen up to best 2 */
    /* now optimise and fill in fraw fields */
    for (ires=0; ires < 2; ires++)
    {
        ibest=(ires?best2:best1);
        if (ibest == UNSET || Lag(ibest).coeff == Badcoeff)
        {
            thiss->fraw[ires].pitch16=UNSET;
            thiss->fraw[ires].coeff=Badcoeff;
        }
        else
        {
            if (ires == 0)
            {
                /* do two-stage optimisation on best */
                calculate_optimise(work,&Lag(ibest),
                                   Optimrange_pc100,8);
                calculate_optimise(work,&Lag(ibest),
                                   Optimrange_pc100/8,2);
            }
            else  calculate_optimise(work,&Lag(ibest),
                                         Optimrange_pc100/2,2); /* one-stage opt on 2nd best */
            dist16=Lag(ibest).dist16;
            thiss->fraw[ires].pitch16=(int16)((SysParams->sampfreq*256L+
                                               (dist16>>1))/dist16);
            thiss->fraw[ires].coeff=Lag(ibest).coeff;
        }
    }
    /* check optimising hasn't reversed order */
    if (thiss->fraw[1].coeff > thiss->fraw[0].coeff+Coeff_fix)
    {
        ftemp=thiss->fraw[0];
        thiss->fraw[0]=thiss->fraw[1];
        thiss->fraw[1]=ftemp;
        temp=best1;
        best1=best2;
        best2=temp;
    }

    /* save bestlag info for future use */
    if (best1 == UNSET)
    {
        SysParams->bestlag.coeff=Badcoeff;
    }
    else
    {
        SysParams->bestlag=Lag(best1);
    }
    if (best2 == UNSET)
    {
        SysParams->bestlag2.coeff=Badcoeff;
    }
    else
    {
        SysParams->bestlag2=Lag(best2);
    }
}

static  int16  calculate_findbest(pGrappl work,int16 avoid)
{
    /* return largest coeff in array, optionally ignoring one (that we've already found) */
    SSysparms * SysParams = &(((pWorkspace)(work))->sysparms);
    int16 i,bestindx,bestcoeff;

    for (i=0,bestindx=UNSET,bestcoeff=Badcoeff; i < SysParams->nlag; i++)
    {
        if (i != avoid && Lag(i).coeff > bestcoeff)
        {
            bestindx=i;
            bestcoeff=Lag(i).coeff;
        }
    }
    return(bestindx);
}

static int32 forever()
{
    return true;
};

static  void  calculate_lagacfs(pGrappl work)
{
    /* set up seed acf values for up to first Maxlag zero-crossings in pitch period range */
    SSysparms * SysParams = &(((pWorkspace)(work))->sysparms);
    int16 istart,iend,ilag;
    uint16 dist16;
    uint32 ldist;
    pLagelem lptr;

    /* initialise locator of increasing zero-crossing intervals spanning current data pos */
    calculate_nextgap(work,&istart,&iend,true); /* initialise */
    ilag=0;
    /* do rough acf for all zero-crossing gaps within range */
    while (forever())
    {
        if (istart < 0)
        {
            break;
        }
        /* no more crossings */
        ldist=Cross(iend)-Cross(istart);
        if (ldist > (uint32)(SysParams->maxlen16>>4))
        {
            break;    /* all done */
        }
        if (ldist >= (uint32)(SysParams->minlen16>>4))
        {
            if (ilag >= Maxlag)
            {
                /* array full */
                break;
            }
            /* setup up Lagelem fields */
            dist16=((uint16)ldist)<<4;
            lptr=&Lag(ilag);
            lptr->nxing=iend-istart;
            lptr->coeff=calculate_acf16(work,dist16);
            lptr->dist16=dist16;
            ilag++;
        }
        /* get next crossing */
        calculate_nextgap(work,&istart,&iend,false);
    }
    SysParams->nlag=ilag;
}

static  int16  calculate_magnitude(pGrappl work,int16 init)
{
    /* calc abs magnitude x16 for smoothed waveform (to determine voicing) */
    SSysparms * SysParams = &(((pWorkspace)(work))->sysparms);
    int16 i,val,mean;
    int32 pos,sum;
    int16 pt[NsampMag];

    if (init)
    {
        SysParams->loc_smstep=SysParams->smagrange/(NsampMag-1);
        SysParams->loc_smstartpos=((SysParams->loc_smstep*(NsampMag-1))>>1);
        return(0);
    }
    else
    {
        /* load in points and calculate local mean */
        pos=Dataindx(SysParams->dataindx)-SysParams->loc_smstartpos;
        for (i=0,sum=0; i < NsampMag; i++,pos+=SysParams->loc_smstep)
        {
            pt[i]=(sWave(pos))<<4;
            sum+=pt[i];
        }
        mean=(int16)((sum+(NsampMag>>1))/NsampMag);

        for (i=0,sum=0; i < NsampMag; i++)
        {
            val=pt[i]-mean;
            sum+=(val < 0?-val:val);
        }
        sum=(sum+(NsampMag>>1))/NsampMag;

        return((int16)sum);
    }
}

static  void  calculate_nextgap(pGrappl work,int16 * istart,int16 * iend,
                                int16 init)
{
    /* find next appropriate zero-crossing gap interval
      Note that the zero-crossing buffer wraps round, but everything is a power of 2
      so our indices wrap and index correctly also */
    SSysparms * SysParams = &(((pWorkspace)(work))->sysparms);
    int16 low,high=0,mid;

    if (init)
    {
        /* binary chop to find crossings which straddle pos */
        if (SysParams->xcount < 2)
        {
            low=-1;    /* no gap found */
        }
        else
        {
            /* get range of indices */
            high=(int16)Crossindx(SysParams->xindx)-1;
            low=(int16)Crossindx(SysParams->xindx)-SysParams->xcount;
            if (low < 0)
            {
                low+=Maxcross;
                high+=Maxcross;
            }
            SysParams->loc_maxlow=low;
            SysParams->loc_maxhigh=high;
            /* chop fairly close */
            while (high > low+1)
            {
                mid=(low+high)/2;
                if (Cross(mid) > SysParams->dataindx)
                {
                    high=mid;
                }
                else
                {
                    low=mid;
                }
            }
            /* position on best adjacent pair */
            while (Cross(low) >= SysParams->dataindx && low > SysParams->loc_maxlow)
            {
                low--;
            }
            while (low < SysParams->loc_maxhigh && Cross(low+1) <
                    SysParams->dataindx)
            {
                low++;
            }
            high=low+1;
            if (high > SysParams->loc_maxhigh)
            {
                high--;
                low--;
            }
            if (Cross(low) >= SysParams->dataindx ||
                    Cross(high) < SysParams->dataindx)
            {
                low=-1;
            }
        }
    }
    else
    {
        /* extend by one crossing, keeping as centred as poss */
        low=*istart;
        high=*iend;
        if (low == SysParams->loc_maxlow && high == SysParams->loc_maxhigh)
        {
            *istart=-1;
            return;
        }
        if (low == SysParams->loc_maxlow)
        {
            high++;
        }
        else
        {
            if (high == SysParams->loc_maxhigh)
            {
                low--;
            }
            else
            {
                if (SysParams->dataindx-Cross(low-1) >
                        Cross(high+1)-SysParams->dataindx)
                {
                    high++;
                }
                else
                {
                    low--;
                }
            }
        }
    }
    /* return indices of crossing positions */
    *istart=low;
    *iend=high;
}

static  void  calculate_optimise(pGrappl work,pLagelem lelem,
                                 int16 range_pc100,int16 npos)
{
    /* optimise for specified lag to min of 1/16ths of a data sample interval */
    SSysparms * SysParams = &(((pWorkspace)(work))->sysparms);
    int16 bestcoeff,coeff,i;
    uint16 bestdist16,range16,step16,dist16;

    if (lelem->dist16 == UUNSET)
    {
        return;
    }

    bestcoeff=lelem->coeff;
    bestdist16=lelem->dist16;
    npos=(npos+1)/2;
    range16=(uint16)((lelem->dist16*(int32)range_pc100+5000L)/10000L);
    step16=(range16+npos/2)/npos;
    if (step16 < 1)
    {
        step16=1;
    }
    for (i=-npos; i <= npos; i++)
    {
        if (i == 0)
        {
            continue;
        }
        dist16=lelem->dist16+i*step16;
        coeff=calculate_acf16(work,dist16);
        if (coeff > bestcoeff)
        {
            bestcoeff=coeff;
            bestdist16=dist16;
        }
    }
    lelem->dist16=bestdist16;
    lelem->coeff=bestcoeff;
    if (lelem->dist16 > SysParams->maxlen16 || lelem->dist16 <
            SysParams->minlen16)
    {
        lelem->dist16=UUNSET;
        lelem->coeff=Badcoeff;
    }
}

static  void  calculate_pitch(pGrappl work)
{
    /* get two fraw best pitch estimates for current position */
    /* get acfs for 0-crossings */
    calculate_lagacfs(work);

    /* add in acfs for previous match, and extrapolated version, if
       available */
    calculate_addprev(work);

    /* get best two, and first at shorter lag which is within 20% of
       best, if any */
    calculate_selectbest(work);

    /* determine best two matches and return */
    calculate_choosebest2(work);
}

static  void  calculate_selectbest(pGrappl work)
{
    /* reduce lags to two best, and a third if at shorter lag than
      first two and within 20% of best */
    SSysparms * SysParams = &(((pWorkspace)(work))->sysparms);
    Lagelem bestlag,bestlag2;
    pLagelem lptr=&Lag(0);
    int16 i,indx1,indx2,indx3,percent10,j;

    // eliminate initialization warning
    memset(&bestlag,0,sizeof(Lagelem));

    /* discard worse of any pairs within 20% of each other */
    for (i=0; i < SysParams->nlag-1;)
    {
        percent10=(int16)(((int32)lptr[i+1].dist16-(int32)lptr[i].dist16)*2000L/
                          ((int32)lptr[i].dist16+(int32)lptr[i+1].dist16));
        if (percent10 <= 200)
        {
            SysParams->nlag--;
            if (lptr[i+1].coeff > lptr[i].coeff)
            {
                lptr[i]=lptr[i+1];
            }
            for (j=i+1; j < SysParams->nlag; j++)
            {
                lptr[j]=lptr[j+1];
            }
        }
        else
        {
            i++;
        }
    }

    indx2=indx3=UNSET;
    /* get best */
    indx1=calculate_findbest(work,UNSET);
    if (indx1 != UNSET)
    {
        bestlag=lptr[indx1];
        /* get second best */
        indx2=calculate_findbest(work,indx1);
        if (indx2 != UNSET)
        {
            bestlag2=lptr[indx2];
            j=(indx2 < indx1?indx2:indx1);
            for (i=0; i < j; i++)
            {
                /* smaller lag within required amount? */
                if (bestlag.coeff-lptr[i].coeff <= 1000)
                {
                    indx3=i;
                    break;
                }
            }
        }
    }
    i=0;
    /* and third 'special' */
    if (indx3 != UNSET)
    {
        lptr[i++]=lptr[indx3];
    }
    if (indx2 != UNSET)
    {
        lptr[i++]=(indx1 > indx2?bestlag2:bestlag);
        lptr[i++]=(indx1 > indx2?bestlag:bestlag2);
    }
    else  if (indx1 != UNSET)
    {
        lptr[i++]=bestlag;
    }
    /* return in lag length order */
    SysParams->nlag=i;
}

static  void  calculate_voiced(pGrappl work)
{
    /* determine whether position is voiced or not */
    SSysparms * SysParams = &(((pWorkspace)(work))->sysparms);
    pSres thiss=&Sres(SysParams->ncalc);

    thiss->magnitude=calculate_magnitude(work,false);
    thiss->voiced=(thiss->magnitude >= SysParams->minvoiced16);

    /* convert abs mag to scaled value between 0 and 100 for later use */
    thiss->magnitude=(thiss->magnitude+4)>>3;
    if (thiss->magnitude > 100)
    {
        thiss->magnitude=100;
    }
}

static  void  calculate_weights(pGrappl work)
{
    /* calculate a 'likelihood' weight for each pitch guess; variable
      component lies between 0 and 1000 */
    /* each component part of these estimates is constrained to range 0-100 */
    SSysparms * SysParams = &(((pWorkspace)(work))->sysparms);
    int16 diff,i,mpart,dpart[2],apart;
    pFraw fraw;
    pSres thiss=&Sres(SysParams->ncalc);

    if (thiss->voiced)
    {
        /* contribution from magnitude */
        mpart=thiss->magnitude;

        /* contribution from coeff difference */
        if (thiss->fraw[1].coeff == Badcoeff)
        {
            diff=100;
        }
        else
        {
            diff=(thiss->fraw[0].coeff-thiss->fraw[1].coeff)/50+50;
            if (diff < -100)
            {
                diff=-100;
            }
            if (diff > 100)
            {
                diff=100;
            }
        }
        dpart[0]=diff;
        dpart[1]=100-diff;

        for (i=0; i < 2; i++)
        {
            /* calculate weight of each pitch estimate from components */
            fraw=&thiss->fraw[i];
            if (fraw->coeff == Badcoeff)
            {
                fraw->weight=0;
            }
            else
            {
                apart=(fraw->coeff-5000)/50;
                if (apart < 0)
                {
                    apart=0;
                }
                fraw->weight=(int16)(((int32)mpart*dpart[i]*apart
                                      +500L)/1000L);
            }
        }
    }
}


/**** SELECT ************************************************************/

static  void  select(pGrappl work)
{
    /* find each voiced section in results workspace and generate zero
      or more continuous tone contours from it */
    SSysparms * SysParams = &(((pWorkspace)(work))->sysparms);
    int16 indx,voiced,istart=UNSET,iend;

    voiced=false;
    for (indx=0; indx < SysParams->ncalc+1; indx++)
    {
        if (voiced)
        {
            if (indx == SysParams->ncalc || ! Sres(indx).voiced ||
                    Ures(indx).fcalc16 == UNSET)
            {
                /* process this voiced section */
                voiced=false;
                iend=indx-1;
                select_section(work,istart,iend);
            }
        }
        else
        {
            if (indx != SysParams->ncalc && Sres(indx).voiced &&
                    Ures(indx).fcalc16 != UNSET)
            {
                voiced=true;
                istart=indx;
            }
        }
        if (! voiced && indx < SysParams->ncalc)
        {
            /* mark unvoiced sections */
            Sres(indx).selected=false;
            Ures(indx).fselect16=UNSET;
        }
    }
}

static  int16  select_continuous(pGrappl work,int16 this16,int16 mid16,
                                 int16 next16)
{
    /* return true if this16 is 'continuous' with other two */
    SSysparms * SysParams = &(((pWorkspace)(work))->sysparms);
    int16 mean16,change_pc10;

    /* first check in range */
    if (this16 > (SysParams->maxpitch<<4) || this16 < (SysParams->minpitch<<4))
    {
        return(false);
    }

	/* protect against divide-by-zero */
	if ((this16+mid16)==0) return (false);
    /* then check % change from previous */
    change_pc10=(int16)((this16-mid16)*2000L/(this16+mid16));
    if (change_pc10 < 0)
    {
        change_pc10=-change_pc10;
    }
    if (change_pc10 > SysParams->maxchange_pc10)
    {
        return(false);
    }

    /* then look at % bend */
    mean16=(short)(((int32)this16+(int32)next16)/2); //CLW 4/5/00
    change_pc10=(int16)((mean16-mid16)*2000L/(mean16+mid16));
    if (change_pc10 < 0)
    {
        change_pc10=-change_pc10;
    }
    if (change_pc10 > SysParams->maxchange_pc10)
    {
        return(false);
    }

    return(true);
}

static  int16  select_route(pGrappl work,int16 istart,int16 iend,
                            int16 * ipos,int16 * weight,int16 * isstart,int16 * isend)
{
    /* find a connected pitch trace, starting with the largest weighted
      point which has not yet been selected, or, if ipos is set, use
      ipos and actually select the points; return false if all done */
    int16 i,bestwt,wt,p16[3],indx;
    Route rt;

    rt.confirm=(*ipos != UNSET);

    if (! rt.confirm)
    {
        /* find seed position */
        for (i=istart,bestwt=UNSET; i <= iend; i++)
        {
            if (! Sres(i).selected && Sres(i).fraw[0].weight >
                    bestwt)
            {
                bestwt=Sres(i).fraw[0].weight;
                *ipos=i;
            }
        }
    }
    if (*ipos == UNSET)
    {
        return(false);    /* return if no more to trace */
    }

    /* this is a seed position - must have at least 3 valid points */
    /* initialise tracking mechanism */
    for (i=0,rt.weight=0,rt.magnitude=0; i < 3; i++)
    {
        indx=*ipos+i-1;
        p16[i]=Sres(indx).fraw[0].pitch16;
        Sres(indx).selected=true;
        wt=Sres(indx).fraw[0].weight;
        rt.weight+=wt;
        rt.magnitude+=Sres(indx).magnitude;
        if (rt.confirm)
        {
            Ures(indx).fselect16=p16[i];
            Sres(indx).weight=Baseweighting+wt;
        }
    }

    rt.submag=rt.subweight=0;

    /* extend forwards */
    rt.pitch16=p16[2];
    rt.prevpitch16=p16[1];
    rt.ipos=rt.isubpos=*ipos+1;
    rt.itarget=iend;
    rt.direc=+1;
    while (select_routenext(work,&rt));
    *isend=rt.ipos;

    /* extend backwards */
    rt.pitch16=p16[0];
    rt.prevpitch16=p16[1];
    rt.ipos=rt.isubpos=*ipos-1;
    rt.itarget=istart;
    rt.direc=-1;
    while (select_routenext(work,&rt));
    *isstart=rt.ipos;

    /* calc mean weighting (in range 0-1000) */
    *weight=(int16)((rt.weight*100L+(rt.magnitude>>1))/(rt.magnitude == 0?
                    1:rt.magnitude));
    if (rt.confirm)
    {
        /* clear remaining sections on either side, if any */
        for (i=istart; i < *isstart; i++)
        {
            Sres(i).selected=false;
        }
        for (i=*isend+1; i <= iend; i++)
        {
            Sres(i).selected=false;
        }
    }
    return(true);
}

static  int16  select_routenext(pGrappl work,Route * rt)
{
    /* try and join up next data point to growing contour */
    /* trace ends must be points at 'actual' frequency, i.e. not second best or mult/div
       of original, so as well as the tracking position, we have the last position at
       which an fcalc16 freq was continuous with the contour */
    int16 found,indx,this16,wt=0,idiv,i,actual_join=false;

    if (rt->isubpos == rt->itarget)
    {
        return(false);
    }

    found=false;

    indx=rt->isubpos+(rt->direc > 0?1:-1);

    /* first try best match */
    this16=Sres(indx).fraw[0].pitch16;
    if (select_continuous(work,this16,rt->pitch16,rt->prevpitch16))
    {
        found=true;
        Sres(indx).selected=true;
        wt=Sres(indx).fraw[0].weight;
        actual_join=true;
    }
    if (! found && Sres(indx).fraw[1].pitch16 != UNSET)
    {
        /* try second guess at pitch */
        this16=Sres(indx).fraw[1].pitch16;
        if (select_continuous(work,this16,rt->pitch16,rt->prevpitch16))
        {
            found=true;
            wt=Sres(indx).fraw[1].weight/2; /* give less weight */
        }
    }
    if (! found)
    {
        /* try multiple or division of best and second best */
        for (i=0; i < 2; i++)
        {
            this16=Sres(indx).fraw[i].pitch16;
            if (this16 == UNSET)
            {
                break;
            }
            if (this16 > rt->pitch16)
            {
                idiv=(this16+((rt->pitch16)>>1))/(rt->pitch16);
                if (idiv > 1)
                {
                    this16/=idiv;
                }
            }
            else
            {
                idiv=(rt->pitch16+(this16>>1))/this16;
                if (idiv > 1)
                {
                    this16*=idiv;
                }
            }
            if (idiv > 1)
            {
                if (select_continuous(work,this16,rt->pitch16,
                                      rt->prevpitch16))
                {
                    found=true;
                    wt=0; /* zero weight */
                    break;
                }
            }
        }
    }

    if (found)
    {
        if (actual_join)
        {
            /* best fraw joins onto growing contour */
            rt->ipos=indx;
            rt->magnitude+=Sres(indx).magnitude+rt->submag;
            rt->submag=0;
            rt->weight+=wt+rt->subweight;
            rt->subweight=0;
        }
        else
        {
            /* these components will be added when the contour extension is
              confirmed by connecting as an 'actual_join'; until then they
              represent tentative contour extensions */
            rt->submag+=Sres(indx).magnitude;
            rt->subweight+=wt;
        }
        rt->prevpitch16=rt->pitch16;
        rt->pitch16=this16;
        if (rt->confirm)
        {
            /* fill in results fields */
            Ures(indx).fselect16=this16;
            Sres(indx).weight=wt+Baseweighting;
            Sres(indx).selected=true;
        }
        rt->isubpos=indx;
    }
    return(found);
}

static  void  select_section(pGrappl work,int16 istart,int16 iend)
{
    /* interpret a voiced section as zero or more continuous pitch contours */
    SSysparms * SysParams = &(((pWorkspace)(work))->sysparms);
    int16 i,ibestpos=UNSET,ipos,isstart,isend,bestwt,weight,f1,f2,f3;

    /* clear 'has been looked at' flag */
    for (i=istart; i <= iend; i++)
    {
        Sres(i).selected=false;
    }

    /* check potentially big enough to keep */
    if (iend-istart+1 >= SysParams->minsigpoints)
    {
        /* flag all points as processed which are not the middle of three
           contiguous best raw points */
        for (i=istart; i <= iend; i++)
        {
            if (i == istart || i == iend)
            {
                Sres(i).selected=true;
            }
            else
            {
                f1=Sres(i-1).fraw[0].pitch16;
                f2=Sres(i).fraw[0].pitch16;
                f3=Sres(i+1).fraw[0].pitch16;
                if (f1 == UNSET || f2 == UNSET || f3 == UNSET ||
                        ! select_continuous(work,f1,f2,f3) ||
                        ! select_continuous(work,f3,f2,f1))
                {
                    Sres(i).selected=true;
                }
            }
        }
        /* find all pitch traces until every acceptable start point
           has been used at least once and determine best */
        bestwt=0;
        ipos=UNSET;
        while (select_route(work,istart,iend,&ipos,&weight,&isstart,
                            &isend))
        {
            if (weight > bestwt && weight >= SysParams->minmeanweight &&
                    isend-isstart+1 >= SysParams->minsigpoints)
            {
                ibestpos=ipos;
                bestwt=weight;
            }
            ipos=UNSET;
        }
    }

    if (ibestpos != UNSET)
    {
        /* choose and confirm best suitable connected path */
        (void)select_route(work,istart,iend,&ibestpos,&weight,&isstart,
                           &isend);
        /* may be end sections which were not included and need
           recursively checking also */
        if (isstart-1 >= istart)
        {
            Ures(isstart-1).fselect16=UNSET;    /* force gap */
        }
        if (isstart-2 >= istart)
        {
            select_section(work,istart,(int16)(isstart-2));
        }
        if (isend+1 <= iend)
        {
            Ures(isend+1).fselect16=UNSET;
        }
        if (isend+2 <= iend)
        {
            select_section(work,(int16)(isend+2),iend);
        }
    }
    else
    {
        /* no pitch contour to return for this section */
        for (i=istart; i <= iend; i++)
        {
            Sres(i).selected=false;
            Ures(i).fselect16=UNSET;
        }
    }
}

/**** SMOOTH *********************************************************/

static  void  smooth(pGrappl work)
{
    /* find each selected section and smooth it */
    SSysparms * SysParams = &(((pWorkspace)(work))->sysparms);
    int16 indx,selected,iprevend,prevpitch16,istart=0,iend,f1,f2,
                                             change,i,lastindx;

    selected=SysParams->prevfch[0] != UNSET;
    iprevend=-1;
    prevpitch16=SysParams->lastfsmooth;
    lastindx=SysParams->nres+2; /* go beyond a bit if poss to smooth over join */
    if (lastindx > SysParams->ncalc)
    {
        lastindx=SysParams->ncalc;
    }
    for (indx=0; indx < lastindx; indx++)
    {
        if (selected)
        {
            if (Ures(indx).fselect16 == UNSET)
            {
                /* if this is a single gap try to interpolate across */
                if (indx < lastindx-1 &&
                        Ures(indx+1).fselect16 != UNSET)
                {
                    f1=(indx > 0?Ures(indx-1).fselect16:
                        SysParams->prevfch[0]);
                    f2=Ures(indx+1).fselect16;
                    change=(int16)((f1-f2)*2000L/(f1+f2));
                    if (change < 0)
                    {
                        change=-change;
                    }
                    if (change <= SysParams->maxinterp_pc10)
                    {
                        Ures(indx).fselect16=
                            (f1+f2+1)/2;
                        Sres(indx).weight=Baseweighting;
                        continue;/* don't break here */
                    }
                }
                selected=false;
                iend=indx-1;
                if (iend >= 0)
                {
                    /* smooth this section */
                    smooth_section(work,istart,iend,
                                   iprevend,prevpitch16);
                    iprevend=iend;
                    prevpitch16=Ures(iend).fsmooth16;
                }
            }
        }
        else
        {
            if (Ures(indx).fselect16 != UNSET)
            {
                selected=true;
                istart=indx;
            }
        }
        if (! selected)
        {
            Ures(indx).fsmooth16=UNSET;
        }
    }
    /* ensure final section is smoothed and any preceding unvoiced section correctly
       handled */
    smooth_section(work,(int16)(selected?istart:lastindx),(int16)(lastindx-1),iprevend,
                   prevpitch16);

    /* save values to subsequently paste over join */
    /* last fsmooth value for continuity of values across unvoiced sections */
    SysParams->lastfsmooth=Ures(SysParams->nres-1).fsmooth16;
    if (SysParams->lastfsmooth < 0)
    {
        SysParams->lastfsmooth=-SysParams->lastfsmooth;
    }
    /* last two fselect values as basis for smoothing start of next section if voiced */
    for (i=0; i < 2; i++)
    {
        indx=SysParams->nres-i-1;
        if (indx >= 0 && Ures(indx).fselect16 != UNSET)
        {
            SysParams->prevwt[i]=Sres(indx).weight;
            SysParams->prevfch[i]=Ures(indx).fselect16;
        }
        else
        {
            SysParams->prevwt[i]=0;
            SysParams->prevfch[i]=UNSET;
        }
    }
}

static  void  smooth_section(pGrappl work,int16 istart,int16 iend,
                             int16 iprevend,int16 prevpitch16)
{
    /* smooth the specified section (which is a continuous pitch trace)
      and then interpolate across the previous section and store
      as negative values */
    SSysparms * SysParams = &(((pWorkspace)(work))->sysparms);
    int16 store[3],i,j,idiv,wt,pitch16,ipass,fch=0;
    int32 lval;


    for (ipass=0; ipass < Nsmoothpass; ipass++)
    {
        /* smooth the selected section */
        for (i=istart; i <= iend+2; i++)
        {
            if (i <= iend)
            {
                /* calculate smoothed value and hold in temp storage */
                idiv=0;
                lval=0;
                for (j=i-2; j <= i+2; j++)
                {
                    /* sum weighted values */
                    if (j > iend)
                    {
                        continue;
                    }
                    if (j >= istart)
                    {
                        wt=Sres(j).weight;
                        fch=(ipass?Ures(j).fsmooth16:
                             Ures(j).fselect16);
                    }
                    else
                    {
                        /* start of result block */
                        if (istart == 0)
                        {
                            wt=SysParams->prevwt[-j-1];
                            fch=SysParams->prevfch[-j-1];
                        }
                        else
                        {
                            wt=0;
                        }
                    }
                    if (wt)
                    {
                        if (j == i-1 || j == i+1)
                        {
                            wt=(wt<<1);
                        }
                        if (j == i)
                        {
                            wt=(wt<<2);
                        }
                        idiv+=wt;
                        lval+=(int32)wt*fch;
                    }
                }
                store[i % 3]=(int16)((lval+(idiv>>1))/idiv);
            }
            if (i-2 >= istart)
            {
                /* output from storage two behind */
                Ures(i-2).fsmooth16=store[(i+1) % 3];
            }
        }
    }

    /* interpolate across the unvoiced gap */
    if (istart <= iend)
    {
        /* real data */
        pitch16=Ures(istart).fsmooth16;
        if (prevpitch16 == UNSET)
        {
            prevpitch16=SysParams->lastfsmooth;
        }
        if (prevpitch16 == UNSET)
        {
            prevpitch16=pitch16;
        }
    }
    else
    {
        /* just interpolating to end of utterance */
        if (prevpitch16 == UNSET)
        {
            prevpitch16=SysParams->lastfsmooth;
        }
        if (prevpitch16 == UNSET)
        {
            prevpitch16=(SysParams->minpitch+SysParams->maxpitch)*8;
        }
        pitch16=prevpitch16;
    }
    for (i=iprevend+1; i < istart; i++)
    {
        Sres(i).selected=false;
        Ures(i).fsmooth16=-(int16)(((int32)pitch16*(i-iprevend)+
                                    (int32)prevpitch16*(istart-i))/(istart-iprevend));
    }
}

/**** PARSE **********************************************************/

static  void  parse(pGrappl work)
{
    /* determine where best to break results so that break will be pasted over when
      next group of results are returned; reset SysParams->nres to no. of results to return */
    SSysparms * SysParams = &(((pWorkspace)(work))->sysparms);
    int16 indx,imid,lastok=0,voiced,ibreak,iend;

    imid=SysParams->ncalc/2;

    /* look for a voiced->unvoiced boundary in latter half of results and break
       here by preference */
    voiced=true;
    ibreak=iend=UNSET;
    for (indx=SysParams->ncalc-1; indx >= imid; indx--)
    {
        if (Ures(indx).fcalc16 == UNSET)
        {
            if (voiced)
            {
                /* mark as end of unvoiced section */
                iend=indx;
                voiced=false;
            }
        }
        else
        {
            if (! voiced)
            {
                /* transition found - break here */
                ibreak=indx+1;
                break;
            }
        }
    }
    if (ibreak == UNSET)
    {
        /* find some other convenient place to break */
        if (iend != UNSET)
        {
            /* first half is unvoiced; break within the unvoiced section */
            ibreak=(iend+imid+1)/2;
        }
    }
    if (ibreak == UNSET)
    {
        /* whole of latter half is voiced */
        /* try to break on 'real' pitch point to ensure continuity - try for
           first such point after midway */
        lastok=SysParams->ncalc-SysParams->minsigpoints-2;
        for (indx=imid; indx < lastok; indx++)
        {
            if (Ures(indx).fselect16 == Ures(indx).fcalc16)
            {
                ibreak=indx;
                break;
            }
        }
    }
    if (ibreak == UNSET)
    {
        /* try likewise before the halfway point */
        for (indx=(imid < lastok?imid:lastok); indx > 0; indx--)
        {
            if (Ures(indx).fselect16 == Ures(indx).fcalc16)
            {
                ibreak=indx;
                break;
            }
        }
    }
    if (ibreak == UNSET)
    {
        ibreak=imid;    /* in rare case of no place yet found */
    }

    SysParams->nres=ibreak;
}
