#ifndef SIGNAL_H
 #define SIGNAL_H
 #include "dspTypes.h"
 
 enum{PCM_UBYTE=-1, PCM_2SBYTE=1, PCM_2SSHORT=2};
 enum SIG_SOURCE{UNKNOWN_HUMAN=-1, MAN=0, WOMAN=1, CHILD=2, MUSICAL_INSTRUMENT=3}; 

 typedef struct SIG_PARMS {
   SIG_PARMS() { Start = 0; Length = 0; SmpDataFmt = 0; SmpRate = 0; Source = UNKNOWN_HUMAN; }
   void *Start;
   uint32 Length;
   int8 SmpDataFmt;
   uint16 SmpRate;
   SIG_SOURCE Source;
 }SIG_PARMS;
 
 // PROC_PARMS added by CLW 1/5/99
 typedef struct{
                void        *pBufferStart;  // pointer to buffer containing data to be processed
                uint32    dwBufferSize;  // size of buffer in bytes
                uint32    dwBlockOffset;  // sample number at which processing begins
                uint32    dwBlockLength;  // number of samples in process data
                SIG_PARMS  stSigParms;
         }PROC_PARMS;         
               
#endif              
