/**************************************************************************\
* MODULE:       HISTGRAM.CPP                                               *
* COPYRIGHT:    SUMMER INSTITUTE OF LINGUISTICS, 1999                      *
* AUTHOR:       COREY L. WENGER                                            *
*                                                                          *
*                                                                          *
* DESCRIPTION:                                                             *
*                                                                          *
* Class to compute a histogram from a signal given the bin parameters.     *
*                                                                          *
* TYPICAL APPLICATION:                                                     *
*                                                                          *
* Histograms show the distribution of a signal, similar to the probability *
* density function. It is useful for finding the most frequently           *
* occurring values. It is also useful for fitting mathematical models of   *
* the probability density function.                                        *
*                                                                          *
* PUBLIC MEMBER VARIABLES:                                                 *
*                                                                          *
*                                                                          *
* PUBLIC MEMBER FUNCTIONS:                                                 *
*                                                                          *
* Note: all arguments are passed using the C calling convention.           *
*                                                                          *
* Copyright                                                                *
*   Function to retrieve pointer to copyright notice                       *
*   Arguments:    None                                                     *
*   Return value: pointer to null-terminated string containing             *
*                 copyright notice                                         *
*                                                                          *
* Version                                                                  *
*   Function to return version number of class                             *
*   Arguments:     None                                                    *
*   Return value:  Version number in 4-byte floating point format          *
*                                                                          *
* CreateObject                                                             *
*   Function to validate Histogram settings, construct a Histogram object, *
*   and initialize it.                                                     *
*                                                                          *
*   Arguments:                                                             *
*     ppoHistogram,       address to contain pointer to Histogram object   *
*     stHistParms,        structure (as defined in HISTGRAM.H) containg    *
*                         the number of bins and the bin division values.  *
*     stProcParms         structure (as defined in SIGNAL.H) containing    *
*                         pointer to process data buffer, size of the      *
*                         process data buffer, starting sample for the     *
*                         process block, and the number of samples in the  *
*                         process block.                                   *
*                                                                          *
*   Return value:  4-byte signed integer, defined in ERRCODES.H            *
*     DONE                      no fatal errors                            *
*     INVALID_BFR_PTR           fatal error: null pointer to data buffer   *
*                                            specified                     *
*     INVALID_BFR_SIZE          fatal error: invalid data buffer size      *
*                                            (in bytes) specified          *
*     INVALID_BLOCK             fatal error: invalid data block starting   *
*                                            sample specified              *
*     INVALID_BLOCK_LEN         fatal error: invalid data block length     *
*                                            specified                     *
*     INVALID_SIG_LEN           fatal error: invalid number of signal      *
*                                            samples specified             *
*     UNSUPP_SMP_DATA_FMT       fatal error: unsupported sample data       *
*                                            format                        *
*     OUT_OF_MEMORY             fatal error: insufficient memory for       *
*                                            buffers                       *
*                                                                          *
* GenerateHistogram                                                        *
*   Function to commence histogram generation with the appropriate         *
*   Process procedure (depending on data format)                           *
*                                                                          *
*   Arguments:    None                                                     *
*   Return value: 4-byte signed integer, defined in ERRCODES.H             *
*                 Simply returns what is passed back from Process()        *
*                                                                          *
* GetBin                                                                   *
*   Function to return data from the requested bin.                        *
*                                                                          *
*   Arguments:                                                             *
*     nBinValue   Histogram value for desired bin                          *
*     nBinNum     Number of desired bin                                    *
*     fCoeff      Desired precision for bin value                          *
*     wGraphForm  Form of histogram (COUNTS, PDF, or CDF)                  *
*                                                                          *
*   Return value:  4-byte signed integer, defined in ERRCODES.H            *
*                                                                          *
* GetHistogram                                                             *
*   Function to return the full histogram.                                 *
*                                                                          *
*   Arguments:                                                             *
*     pBins       Pointer to histogram bins (returned data)                *
*     fCoeff      Desired precision for bin value                          *
*     wGraphForm  Form of histogram (COUNTS, PDF, or CDF)                  *
*                                                                          *
*   Return value:  4-byte signed integer, defined in ERRCODES.H            *
*                                                                          *
* GetMaxValue                                                              *
*   Function to return maximum bin value.                                  *
*                                                                          *
*   Arguments:                                                             *
*     pCaller     Pointer to calling object.                               *
*                                                                          *
* GetMinValue                                                              *
*   Function to return minimum bin value.                                  *
*                                                                          *
*   Arguments:                                                             *
*     pCaller     Pointer to calling object.                               *
*                                                                          *
* GetTotalCounts                                                           *
*   Function to return total number of counts in all bins.                 *
*                                                                          *
*   Arguments:                                                             *
*     pCaller     Pointer to calling object.                               *
*                                                                          *
*                                                                          *
* PRACTICAL CONSIDERATIONS:                                                *
*                                                                          *
*   1. The purpose of fCoeff is to fit the processed data into the         *
*      available range of a short int32. It the caller's responsibility      *
*      to estimate fCoeff.                                                 *
*                                                                          *
* TYPICAL CALLING SEQUENCE:                                                *
*          :                                                               *
*   #include "dsp\histgram.h"                                              *
*          :                                                               *
*   PROC_PARMS stProcParms;                                                *
*   HIST_PARMS stHistParms;                                                *
*   CHistogram* ppoHist;                                                   *
*   dspError_t Err;                                                               *
*                                                                          *
*   stHistParms.nBins = 10;                                                *
*   stHistParms.pBinDivs = (short *)new short[stHistParms.nBins + 1]    *
*   stProcParms.dwBufferSize = pMain->GetBufferSize();                     *
*   stProcParms.dwBufferStart = (void *)pSpeech;                           *
*   stProcParms.dwBlockOffset = 0;                                         *
*   stProcParms.dwBlockLength = stProcParms.dwBufferSize;                  *
*   if (dwDataSize < stProcParms.dwBufferSize)                             *
*     stProcParms.dwBlockLength = dwDataSize;                              *
*   stProcParms.stSigParms.Length = dwDataSize;                            *
*   stProcParms.stSigParms.SmpDataFmt = PCM_2SSHORT;                       *
*   stProcParms.stSigParms.SmpRate = 22050;                                *
*   dspError_t Err = CHistogram::CreateObject(&ppoHist, stHistParms, stProcParms);*
*   if (Err) return(Err);                                                  *
*   Err = ppoHist->GenerateHistogram();                                    *
*     if (Err)                                                             *
*   Err = ppoHist->GetHistogram((short *)m_lpData, fCoeff, COUNTS);  *
*   m_nMaxValue = ppoHist->GetMaxValue(this);                              *
*                :                                                         *
*   delete ppoHist;                                                        *
*                                                                          *
*                                                                          *
* TEST DRIVER:                                                             *
*   Main (no arguments or return value); available when compiled for       *
*   QuickWin with TEST_HISTGRAM defined (/D"TEST_HISTGRAM" compile option).*
*                                                                          *
*                                                                          *
* COMPILER:         Microsoft Visual C++ version 1.0                       *
* COMPILE OPTIONS:  /nologo /f- /G3 /W3 /Zi /AL /YX /Od /D "_DEBUG" /I     *
*                   "c:\msvc\asapdsp" /FR /GA /Fd"SA.PDB" /Fp"SA.PCH"      *
* LINK OPTIONS:     /NOLOGO /LIB:"lafxcwd" /LIB:"oldnames" /LIB:"libw"     *
*                   /LIB:"llibcew" /NOD /PACKC:61440 /STACK:10240          *
*                   /ALIGN:16 /ONERROR:NOEXE /CO /LIB:"commdlg.lib"        *
*                   /LIB:"mmsystem.lib" /LIB:"olecli.lib"                  *
*                   /LIB:"olesvr.lib" /LIB:"shell.lib"                     *
*                                                                          *
* TO DO:            1. Test Driver                                         *
*                   2. Modify Process() to allow overlapping and gapping   *
*                      bins                                                *
*                       - need to spec both bin edges in HISTPARM          *
*                       - may be useful for filter banks                   *
*                   3. Adjust fCoeff during Process()                      *
*                   4.                                                     *
*                   5.                                                     *
*                                                                          *
* CHANGE HISTORY:                                                          *
*   Date     By             Description                                    *
*  3/15/00  CLW    Initial coding and debug.                               *
\**************************************************************************/
#include "stddsp.h"
#define HISTGRAM_CPP
#define COPYRIGHT_DATE  "2000"
#define VERSION_NUMBER  "1.0"

#include <math.h>
#include "histgram.h"


////////////////////////////////////////////////////////////////////////////////////////
// Class function to return copyright notice.                                         //
////////////////////////////////////////////////////////////////////////////////////////
char *CHistogram::Copyright(void)
{
 static char Notice[] = {"Histogram, Version " VERSION_NUMBER "\n" 
                         "Copyright (c) " COPYRIGHT_DATE " by Summer Institute of Linguistics. " 
                         "All rights reserved.\n"};
 return(Notice);
}

////////////////////////////////////////////////////////////////////////////////////////
// Class function to return version of class.                                         //
////////////////////////////////////////////////////////////////////////////////////////
float CHistogram::Version(void)
{
 return((float)atof(VERSION_NUMBER));
}


////////////////////////////////////////////////////////////////////////////////////////
// Macro definitions.                                                                 //
////////////////////////////////////////////////////////////////////////////////////////
#define RetMemErr  { return(Code(OUT_OF_MEMORY)); }


////////////////////////////////////////////////////////////////////////////////////////
// Class function to validate Histogram settings and construct object.                //
////////////////////////////////////////////////////////////////////////////////////////
dspError_t CHistogram::CreateObject(CHistogram** ppoHistogram, HIST_PARMS &stHistParms,
                                PROC_PARMS &stProcParms)
{ // validate process parameters
  if (!stProcParms.pBufferStart) return(Code(INVALID_BFR_PTR));
  if (!stProcParms.dwBufferSize) return(Code(INVALID_BFR_SIZE));
  if (stProcParms.dwBlockOffset > stProcParms.stSigParms.Length) return(Code(INVALID_BLOCK));
  if (!stProcParms.dwBlockLength) return(Code(INVALID_BLOCK_LEN));
  if (!stProcParms.stSigParms.Length) return(Code(INVALID_SIG_LEN));
  if (stProcParms.stSigParms.SmpDataFmt != PCM_UBYTE && stProcParms.stSigParms.SmpDataFmt != PCM_2SSHORT)
      return(Code(UNSUPP_SMP_DATA_FMT));
  if (stProcParms.dwBlockOffset + stProcParms.dwBlockLength > stProcParms.stSigParms.Length)
     return(Code(INVALID_BLOCK_LEN));
  // validate histogram parameters
  if (!stHistParms.nBins) return(Code(INVALID_BIN_COUNT));
  for (uint16 i = 0; i < (stHistParms.nBins - 1); i++)
  { if (stHistParms.pBinDivs[i] >= stHistParms.pBinDivs[i+1])
      return(Code(INVALID_BIN_DIVS));
  }
  
  *ppoHistogram = new CHistogram(stHistParms, stProcParms);
  if (!*ppoHistogram) RetMemErr;
  return(DONE);   
}

////////////////////////////////////////////////////////////////////////////////////////
// Histogram object constructor.                                                      //
////////////////////////////////////////////////////////////////////////////////////////
CHistogram::CHistogram(HIST_PARMS &stHistParms, PROC_PARMS &stProcParms) 
{
  // Update object member variables.
  m_stProcParms = stProcParms;
  m_stHistParms = stHistParms;
  m_pSigBfr = stProcParms.pBufferStart;
  m_dwBatchOffset = m_stProcParms.dwBlockOffset;
  m_sbSmpFormat = stProcParms.stSigParms.SmpDataFmt;
  m_pHistogram = new short[m_stHistParms.nBins];
  m_nMinValue = 0x7FFF; // SDM was 0xFFFF  which became -1...
  m_nMaxValue = 0;
  m_dwTotalCounts = 0;
  // zero out bins
  uint16 j;
  for (j = 0; j < m_stHistParms.nBins; j++)
  {
    m_pHistogram[j] = 0;
  }
  m_stHistParms.pBinDivs = new short[m_stHistParms.nBins + 1];
  for (j = 0; j <= m_stHistParms.nBins; j++)
  {
    m_stHistParms.pBinDivs[j] = stHistParms.pBinDivs[j];
  }

}

////////////////////////////////////////////////////////////////////////////////////////
// Histogram object destructor.                                                       //
////////////////////////////////////////////////////////////////////////////////////////
CHistogram::~CHistogram()
{ 
 if (m_pHistogram)
 { 
   delete [] m_pHistogram;
   m_pHistogram = NULL;
 }
 if(m_stHistParms.pBinDivs)
 {
   delete [] m_stHistParms.pBinDivs;
   m_stHistParms.pBinDivs = NULL;
 }
}


////////////////////////////////////////////////////////////////////////////////////////
// Class function to free memory allocated for LPC parameters.                        //
////////////////////////////////////////////////////////////////////////////////////////
void CHistogram::FreeHistMem(void)    
{
}

////////////////////////////////////////////////////////////////////////////////////////
// GenerateHistogram routine.                                                         //
////////////////////////////////////////////////////////////////////////////////////////
dspError_t CHistogram::GenerateHistogram(void)
{
 switch(m_sbSmpFormat)
   {
    case PCM_UBYTE:
      return(Process((uint8 *)m_stProcParms.pBufferStart));
    case PCM_2SSHORT:
      return(Process((short *)m_stProcParms.pBufferStart));
    default:
      return(DONE);    
   }                             
}

////////////////////////////////////////////////////////////////////////////////////////
// GetBin routine.                                                                    //
////////////////////////////////////////////////////////////////////////////////////////
dspError_t CHistogram::GetBin(int32 & nBinValue, uint16 nBinNum, double fCoeff, uint16 wGraphForm)
{ 
  switch (wGraphForm)
  { 
  case COUNTS:
    nBinValue = (short)((double)m_pHistogram[nBinNum] / fCoeff);
    break;
    
  case PDF:
    nBinValue = (short)(m_pHistogram[nBinNum] / ((double)m_stProcParms.dwBlockLength * fCoeff));
    break;
    
  case CDF:
    {
      uint32 dwCumCounts = 0;
	  for (uint32 i = 0; i <= nBinNum; i++)
        dwCumCounts += m_pHistogram[i];
      nBinValue = (short)(dwCumCounts / ((double)m_stProcParms.dwBlockLength * fCoeff));
    }
    break;
  default:
    nBinValue = 0;
  }
   
  return (DONE);
}

dspError_t CHistogram::GetBinByData(int32 & nBinValue, int16 nData, double fCoeff, uint16 wGraphForm)
{
  int32 nBinNum = -1;
  GetBinNum(nBinNum, nData);

  nBinValue = 0;

  if(nBinNum < 0)
    return (DONE);

  return GetBin(nBinValue, nBinNum, fCoeff, wGraphForm);
}

// Get bin number for data
// set nBinNum to -1 if data does not fit in a bin
dspError_t CHistogram::GetBinNum(int32 & nBinNum, int16 nData)
{
  nBinNum = -1;
  if (nData < m_stHistParms.pBinDivs[0]) 
    return (DONE);
  if (nData > m_stHistParms.pBinDivs[m_stHistParms.nBins]) 
    return (DONE);

  int32 nMinBin = 0;
  int32 nMaxBin = (uint16)(m_stHistParms.nBins - 1);
  while (nMinBin != nMaxBin)
  { 
    int32 nMidBin = (uint16)((nMaxBin + nMinBin + 1) / 2);
    if (nData < m_stHistParms.pBinDivs[nMidBin])
      nMaxBin = (uint16)(nMidBin - 1);
    else
      nMinBin = nMidBin;
  }
  
  nBinNum = nMinBin;
  return (DONE);
}

////////////////////////////////////////////////////////////////////////////////////////
// GetHistogram routine.                                                              //
////////////////////////////////////////////////////////////////////////////////////////
dspError_t CHistogram::GetHistogram(short* pBins, double fCoeff, uint16 wGraphForm)
{ uint32 i, dwCumCounts;
  switch (wGraphForm)
  { case COUNTS:
      for (i = 0; i < m_stHistParms.nBins; i++)
        pBins[i] = (short)(m_pHistogram[i] / fCoeff);
      break;
      
    case PDF:
      for (i = 0; i < m_stHistParms.nBins; i++)
      {
        // Make max value 0 if the total number of counts is 0. CLW 9/26/00
        if (m_dwTotalCounts)
          pBins[i] = (short)((double)m_pHistogram[i] / ((double)m_dwTotalCounts * fCoeff) + 0.5);
        else
          pBins[i] = 0;
      }
      break;
      
    case CDF:
      for (i = 0, dwCumCounts = 0; i <= m_stHistParms.nBins; i++)
      { dwCumCounts += m_pHistogram[i];
        // Make max value 0 if the total number of counts is 0. CLW 9/26/00
        if (m_dwTotalCounts)
          pBins[i] = (short)((double)dwCumCounts / ((double)m_dwTotalCounts * fCoeff) + 0.5);
        else
          pBins[i] = 0;
      }
      break;
  }
  
  return (DONE);
}

////////////////////////////////////////////////////////////////////////////////////////
// Object function to calculate Histogram.                                            //
////////////////////////////////////////////////////////////////////////////////////////
dspError_t CHistogram::Process(uint8 * pBuffer)  //for 8-bit unsigned data
{ 
  uint32   dwBlockEnd = m_stProcParms.dwBlockOffset + m_stProcParms.dwBlockLength,
          dwBatchSize = m_stProcParms.dwBufferSize / m_sbSmpFormat;
  
  if ((m_dwBatchOffset + dwBatchSize) > dwBlockEnd)
    dwBatchSize = dwBlockEnd - m_dwBatchOffset;
  for (uint16 nSamp = 0; nSamp < dwBatchSize; nSamp++)
  { 
    int32 nBin = -1;
    
    GetBinNum(nBin, (short)pBuffer[nSamp]);
    
    if(nBin < 0) continue;
    
    m_pHistogram[nBin]++;
    m_dwTotalCounts++;
    if (m_pHistogram[nBin] > m_nMaxValue) m_nMaxValue = m_pHistogram[nBin];
    if (m_pHistogram[nBin] < m_nMinValue) m_nMinValue = m_pHistogram[nBin];
  }
  m_dwBatchOffset += dwBatchSize;
  
  if (m_dwBatchOffset < dwBlockEnd) return(HIST_BUFFER_CALLBACK);
  return(DONE);
}

////////////////////////////////////////////////////////////////////////////////////////
// Object function to calculate Histogram.                                            //
////////////////////////////////////////////////////////////////////////////////////////
dspError_t CHistogram::Process(short * pBuffer)  //for 16-bit signed data
{ 
  uint32   dwBlockEnd = m_stProcParms.dwBlockOffset + m_stProcParms.dwBlockLength,
          dwBatchSize = m_stProcParms.dwBufferSize / m_sbSmpFormat;
  
  if ((m_dwBatchOffset + dwBatchSize) > dwBlockEnd)
    dwBatchSize = dwBlockEnd - m_dwBatchOffset;
  for (uint16 nSamp = 0; nSamp < dwBatchSize; nSamp++)
  { 
    int32 nBin = -1;
    
    GetBinNum(nBin, (short)pBuffer[nSamp]);
    
    if(nBin < 0) continue;
    
    m_pHistogram[nBin]++;
    m_dwTotalCounts++;
    if (m_pHistogram[nBin] > m_nMaxValue) m_nMaxValue = m_pHistogram[nBin];
    if (m_pHistogram[nBin] < m_nMinValue) m_nMinValue = m_pHistogram[nBin];
  }
  m_dwBatchOffset += dwBatchSize;
  
  if (m_dwBatchOffset < dwBlockEnd) return(HIST_BUFFER_CALLBACK);
  return(DONE);
}
     

#ifdef TEST_HISTGRAM
#include <stdio.h>

void main(void);
void main(void)
{
 return;
}

#endif
