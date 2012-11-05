 #ifndef HISTGRAM_H
 #define HISTGRAM_H
 
 #include "dspTypes.h"
 //#include "MathX.h" 
 #include "Error.h"
 #include "Signal.h"  

 
 enum {COUNTS = 0, PDF = 1, CDF = 2};
 
 typedef struct{ uint16  nBins;        // Number of bins in histogram
          short * pBinDivs; // Pointer to an array containing the bin divisions
          } HIST_PARMS;

               
 class CHistogram{
           public:
             static char *Copyright(void);
             static float Version(void);
             static dspError_t CreateObject(CHistogram** ppoHistogram, HIST_PARMS &stHistParms, 
                                     PROC_PARMS &stProcParms);     
             dspError_t GenerateHistogram(void);
             dspError_t GetBin(int32 & nBinValue, uint16 nBinNum, double fCoeff, uint16 wGraphForm);
             dspError_t GetBinByData(int32 & nBinValue, int16 nData, double fCoeff, uint16 wGraphForm);
             dspError_t GetBinNum(int32 & nBinNum, int16 nData);
             dspError_t GetHistogram(short* pBins, double fCoeff, uint16 wGraphForm);
             ~CHistogram();                
             virtual const HIST_PARMS& GetHistogramParms() const { return m_stHistParms;} // return histogram parms as const
             virtual short GetMaxValue(void* /*pCaller*/) { return m_nMaxValue;} // return maximum value
             virtual short GetMinValue(void* /*pCaller*/) { return m_nMinValue;} // return minimum value
             virtual uint32 GetTotalCounts(void* /*pCaller*/) { return m_dwTotalCounts;} // return total counts
           private:  
             CHistogram(HIST_PARMS &stHistParms, PROC_PARMS &stProcParms);
             dspError_t Process(uint8 * pBuffer);
             dspError_t Process(short * pBuffer);                   
             static void FreeHistMem(void);
             // member variables
             PROC_PARMS   m_stProcParms;
             HIST_PARMS   m_stHistParms;
             void *  m_pSigBfr;
             int8       m_sbSmpFormat;
             uint32          m_dwBatchOffset;
             short*      m_pHistogram;
             short      m_nMaxValue;
             short      m_nMinValue;
             uint32      m_dwTotalCounts;
            };   
 
#endif
