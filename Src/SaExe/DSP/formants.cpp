#include "stddsp.h"

#define FORMANTS_CPP
#define COPYRIGHT_DATE  "2000"
#define VERSION_NUMBER  "0.1"

#include <math.h>
#include "MathX.h"
#include "Scale.h"
#include "Spectgrm.h"
#include "Formants.h"

////////////////////////////////////////////////////////////////////////////////////////
// Class function to return copyright notice.                                         //
////////////////////////////////////////////////////////////////////////////////////////
char *CFormantPicker::Copyright(void)
{
 static char Notice[] = {"Formant Picker, Version " VERSION_NUMBER "\n" 
                         "Copyright (c) " COPYRIGHT_DATE " by Summer Institute of Linguistics. " 
                         "All rights reserved.\n"};
 return(Notice);
}

////////////////////////////////////////////////////////////////////////////////////////
// Class function to return version of class.                                         //
////////////////////////////////////////////////////////////////////////////////////////
float CFormantPicker::Version(void)
{
 return((float)atof(VERSION_NUMBER));
}


////////////////////////////////////////////////////////////////////////////////////////
// Class function to validate formant picker parameters and construct object.         //
////////////////////////////////////////////////////////////////////////////////////////
dspError_t CFormantPicker::CreateObject(CFormantPicker **ppFormantPicker, uint16 nMaxNumFormants, short nSource)
{
  if (!ppFormantPicker) return(Code(INVALID_PARM_PTR));
  *ppFormantPicker = NULL;
  if (!nMaxNumFormants) return(Code(INVALID_NUM_FORMANTS));
  uint16 nMaxNumFreq = (uint16)(nMaxNumFormants + 1);  //include pitch at F[0]
  FORMANT_VALUES *FormantTable = (FORMANT_VALUES *)new FORMANT_VALUES[nMaxNumFreq];
  if (!FormantTable) return(Code(OUT_OF_MEMORY));
  *ppFormantPicker = new CFormantPicker(FormantTable, nMaxNumFormants, nSource);
  if (!*ppFormantPicker) return(Code(OUT_OF_MEMORY));
  dspError_t Err = (*ppFormantPicker)->ValidateObject();
  if (Err)
  {
    delete *ppFormantPicker;
    *ppFormantPicker = NULL;
    return(Code(OUT_OF_MEMORY));
  }
  return(DONE);  
}

////////////////////////////////////////////////////////////////////////////////////////
// Formant picker object constructor.                                                 //
////////////////////////////////////////////////////////////////////////////////////////
CFormantPicker::CFormantPicker(FORMANT_VALUES *FormantTable, uint16 nMaxNumFormants, short nSource):
                CPeakPicker(nMaxNumFormants+NUM_EXTRA_PEAKS_ALLOWED) 
{
  m_FormantTable = FormantTable;
  m_nMaxNumFormants = nMaxNumFormants;
  m_nSource = (short)(nSource + 1);   //adjust for zero based formant array indexing
}

////////////////////////////////////////////////////////////////////////////////////////
// Formant picker object destructor.                                                  //
////////////////////////////////////////////////////////////////////////////////////////
CFormantPicker::~CFormantPicker()
{
  delete m_FormantTable;
}


////////////////////////////////////////////////////////////////////////////////////////
// Formant Grid                                                                       //
////////////////////////////////////////////////////////////////////////////////////////
const float fMaxBandwidth = 800.F;
//                                     Unknown           Men            Women          Children   Musical Instrument
//                                    Low   High      Low   High      Low   High      Low   High      Low   High
static RANGE FormantRange[5][5] = {
                                   {{  50.,  500.}, {  50.,  200.}, { 200.,  350.}, { 350.,  500.}, {  50.,  500.}}, // F0    //!!base on Grappl range
                                   {{ 100., 1100.}, { 100.,  900.}, { 300.,  900.}, { 400., 1100.}, { 500., 1500.}}, // F1
                                   //{{ 700., 3100.}, { 700., 2500.}, { 800., 2900.}, {1000., 3200.}, {1500., 2500.}}, // F2
								   {{ 700., 3100.}, { 700., 2200.}, { 800., 2700.}, {1000., 3000.}, {1500., 2500.}}, // F2
                                   {{1600., 4000.}, {1600., 3200.}, {1800., 3500.}, {2700., 4000.}, {2500., 3500.}}, // F3
                                   {{3000., 5000.}, {2700., 3900.}, {3200., 4500.}, {3500., 4800.}, {3500., 4500.}}, // F4
                                  };

////////////////////////////////////////////////////////////////////////////////////////
// Object function to get formant locations and their log magnitudes.                 //
////////////////////////////////////////////////////////////////////////////////////////
dspError_t CFormantPicker::PickFormants(FORMANT_VALUES **ppFormantTable, uint16 *pFormantCount,
                                 float *pSmoothLogPwrSpectrum, uint16 nSpectrumLength, 
                                 double dSpectralResolution, float fPitch)
{
  if (!pFormantCount) return(Code(INVALID_PARM_PTR));
  *pFormantCount = 0;  
  if (!ppFormantTable) return(Code(INVALID_PARM_PTR));
  if (!pSmoothLogPwrSpectrum) return(Code(INVALID_SIG_DATA_PTR));
  if (nSpectrumLength < 3) return(Code(INVALID_SIG_LEN));
  
  uint32 dwBumpCount;
  BUMP_TABLE_ENTRY *BumpTable;
  dspError_t Err = GetBumps(&BumpTable, &dwBumpCount, (float *)pSmoothLogPwrSpectrum, (uint32)nSpectrumLength);
  if (Err) return(Err);
   
  m_nFormantCount = 0;
  m_FormantTable[0].FrequencyInHertz = fPitch;
  m_FormantTable[0].BandwidthInHertz = (float)UNDEFINED_DATA;
  m_FormantTable[0].PowerInDecibels = FLT_MAX_NEG;
  
  
  enum MATCH_RESULT {SKIP_BUMP, FOUND_FORMANT, NO_FORMANT};
  for (uint32 i = 0; i < dwBumpCount; i++)   
  {
    MATCH_RESULT MatchResult = SKIP_BUMP;
    float fBumpFrequency = (float)(BumpTable[i].Distance * dSpectralResolution);    
    uint32 nFormant = m_nFormantCount+1;
    if (nFormant <= m_nMaxNumFormants)
    { 
      if (nFormant <= 4)
      {
		  if (fBumpFrequency > ::FormantRange[nFormant][m_nSource].Low)
          MatchResult = (fBumpFrequency < ::FormantRange[nFormant][m_nSource].High)? FOUND_FORMANT: NO_FORMANT;
      }
      else
      {
        float fNominalFrequency = (nFormant-1)*1000.F + 500.F;
        if (fBumpFrequency > 0.85F*fNominalFrequency)
        MatchResult = (fBumpFrequency < 1.5F*fNominalFrequency)? FOUND_FORMANT: NO_FORMANT;
      } 

       switch ((int32)MatchResult)
      {
        case FOUND_FORMANT:
          if (BumpTable[i].Amplitude < 0.F) return(Code(INVALID_MAGNITUDE));
          m_FormantTable[++m_nFormantCount].FrequencyInHertz = fBumpFrequency;
		  m_FormantTable[m_nFormantCount].BandwidthInHertz = (float)UNDEFINED_DATA;
          m_FormantTable[m_nFormantCount].PowerInDecibels = 10.F*BumpTable[i].Amplitude;
          break;
        case NO_FORMANT:                                                     
          m_FormantTable[++m_nFormantCount].FrequencyInHertz = (float)NA;
		  m_FormantTable[m_nFormantCount].BandwidthInHertz = (float)UNDEFINED_DATA;
          m_FormantTable[m_nFormantCount].PowerInDecibels = MIN_LOG_MAG;
          break;
      }
    }  
    else break;  
  }    
  for ( ; i < (uint32)m_nMaxNumFormants; i++) 
  {
    m_FormantTable[i].FrequencyInHertz = (float)UNDEFINED_DATA;
	m_FormantTable[i].BandwidthInHertz = (float)UNDEFINED_DATA;
    m_FormantTable[i].PowerInDecibels = FLT_MAX_NEG;
  }
  
  *ppFormantTable = m_FormantTable;
  *pFormantCount = m_nFormantCount;
  
  return(DONE);
}

////////////////////////////////////////////////////////////////////////////////////////
// Static object function to get formant frequency range for a particular source      //
// (i.e. gender for speech) and formant index.  This function does not require that   //
// an object be constructed.                                                          //
////////////////////////////////////////////////////////////////////////////////////////
RANGE CFormantPicker::FormantRange(int32 nSource, uint32 nFormantIndex)
{
  nSource = (short)(nSource + 1);   //adjust for zero based formant array indexing
  if (nSource > 4) nSource = 0;  // unknown
  if (nFormantIndex <= 4) return(::FormantRange[nFormantIndex][nSource]);
  RANGE FormantFreqRange;
  float fNominalFrequency = (nFormantIndex-1)*1000.F + 500.F;
  FormantFreqRange.Low = 0.85F*fNominalFrequency;
  FormantFreqRange.High = 1.5F*fNominalFrequency;
  return(FormantFreqRange);
}

////////////////////////////////////////////////////////////////////////////////////////
// Static object function to determine if the frequency for a particular formant      //
// index is in range.  This function does not require that an object be constructed.  //
////////////////////////////////////////////////////////////////////////////////////////
bool CFormantPicker::FormantInRange(int32 nSource, uint16 nFormantIndex, float fFormantFrequency)
{
  nSource = (short)(nSource + 1);   //adjust for zero based formant array indexing
  if (nSource > 4) nSource = 0;  // unknown
  if (nFormantIndex <= 4)
      return((fFormantFrequency >= ::FormantRange[nFormantIndex][nSource].Low) &&
             (fFormantFrequency <= ::FormantRange[nFormantIndex][nSource].High));
  float fNominalFrequency = (nFormantIndex-1)*1000.F + 500.F;
  return((fFormantFrequency >= 0.85F*fNominalFrequency) &&
         (fFormantFrequency <= 1.5F*fNominalFrequency));

}

////////////////////////////////////////////////////////////////////////////////////////
// Static object function to determine if the bandwidth for a particular formant      //
// index is in range.  This function does not require that an object be constructed.  //
////////////////////////////////////////////////////////////////////////////////////////
bool CFormantPicker::BandwidthInRange(int32 /* nSource */, uint16 /* nFormantIndex */, float fFormantBandwidth)
{
  return(fFormantBandwidth <= fMaxBandwidth);
}