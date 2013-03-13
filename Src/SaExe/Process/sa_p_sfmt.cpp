/////////////////////////////////////////////////////////////////////////////
// sa_p_sfmt.cpp:
// Implementation of the CProcessSpectroFormants class. 
// Author: Steve MacLean
// copyright 2001 JAARS Inc. SIL
//
// Revision History
// 6/27/2001 SDM Split from process contained in CProcessSpectrogram
// 9/1/2001  AKE Changed formant tracking to be pitch synchronous
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "sa_proc.h"
#include "sa_p_sfmt.h"
#include "sa_p_zcr.h"
#include "sa_p_gra.h"
#include "sa_p_fra.h"
#include "sa_p_fmt.h"

#include "resource.h"
#include "isa_doc.h"
#include "math.h"
#include "dsp\spectgrm.h"
#include "dsp\dspTypes.h"

#ifdef DUMP_FORMANT_TRACKS
#include "Segment.h"
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

//###########################################################################
// CProcessSpectroFormants
// class to calculate spectrogram for wave data. The class creates an object
// of the class Spectrogram that does the calculation. The results are stored
// in different temporary files (see base class). Spectrogram needs the whole
// raw data to calculate in memory, so the class CProcessSpectroFormants creates
// a temporary second buffer, into which it copies all the raw data needed
// for the calculation.


/////////////////////////////////////////////////////////////////////////////
// CProcessSpectroFormants construction/destruction/creation

/***************************************************************************/
// CProcessSpectroFormants::CProcessSpectroFormants Constructor
/***************************************************************************/
CProcessSpectroFormants::CProcessSpectroFormants()
{
}

/***************************************************************************/
// CProcessSpectroFormants::~CProcessSpectroFormants Destructor
/***************************************************************************/
CProcessSpectroFormants::~CProcessSpectroFormants()
{
}

/////////////////////////////////////////////////////////////////////////////
// CProcessSpectroFormants helper functions

/***************************************************************************/
// CProcessSpectroFormants::Process Processing spectrogram data
// The processed spectrogram data is stored in a temporary file. To create
// it helper functions of the base class are used. While processing a process
// bar, placed on the status bar, has to be updated. The level tells which
// processing level this process has been called, start process start on
// which processing percentage this process starts (for the progress bar).
// The status bar process bar will be updated depending on the level and the
// progress start. The return value returns the highest level througout the
// calling queue, or -1 in case of an error in the lower word of the long
// value and the end process progress percentage in the higher word. This
// function needs a pointer to the view instead the pointer to the document
// like other process calls. It calculates spectrogram data.
/***************************************************************************/
long CProcessSpectroFormants::Process(void* /*pCaller*/, CView* /*pSaView*/, int /*nWidth*/, int /*nHeight*/,
									  int nProgress, int /*nLevel*/)
{   
	TRACE(_T("Process: CProcessSpectroFormants\n"));
	return MAKELONG(PROCESS_ERROR, nProgress);
}

/***************************************************************************/
// CProcessSpectroFormants::GetFormant Read formant data
// Reads a slice of processed data from the temporary file into the processed
// data buffer and returns the pointer to the data. The returned pointer 
// points to a slice of formant data. pCaller is a pointer to the
// calling plot and enables this function to get the process index of the
// plot. nIndex is the horizontal index in the formant data.  The function 
// returns NULL on error.
/***************************************************************************/
FORMANT_FREQ* CProcessSpectroFormants::GetFormant(DWORD dwIndex)
{
	// read the data
	size_t sSize = sizeof(FORMANT_FREQ);
	return (FORMANT_FREQ*) GetProcessedObject(dwIndex, sSize);
}

/***************************************************************************/
// CProcessSpectroFormants::ExtractFormants  Calculate formant tracks for each 
// fragment within the specified range, smooth if requested, and store them in 
// the spectrogram formant track process temp file.
/***************************************************************************/
long CProcessSpectroFormants::ExtractFormants(ISaDoc *pDoc, DWORD dwWaveDataStart, DWORD dwWaveDataLength, BOOL bSmooth, 
											  int nProgress, int nLevel)
{
	TRACE(_T("Process: CProcessSpectroFormants\n"));
	// check canceled
	if (IsCanceled())
		return MAKELONG(PROCESS_CANCELED, nProgress); // process canceled
	// check if data ready
	if (IsDataReady())
		return MAKELONG(--nLevel, nProgress);    
	FmtParm* pFmtParm = pDoc->GetFmtParm();  // get sa parameters format member data
	WORD nSmpSize = (WORD)(pFmtParm->wBlockAlign / pFmtParm->wChannels);

	if (!StartProcess(this, IDS_STATTXT_PROCESSFMT))
	{ 
		EndProcess(); // end data processing
		return MAKELONG(PROCESS_ERROR, nProgress);
	}

	DWORD dwDataStart = 0;
	DWORD dwDataLength = pDoc->GetDataSize();

	UNUSED_ALWAYS(dwDataStart);
	UNUSED_ALWAYS(dwDataLength);

	CProcessZCross* pZeroCrossCount = pDoc->GetZCross();
	short int nResult = LOWORD(pZeroCrossCount->Process(this, pDoc, nProgress, nLevel+1000)); // process data
	if (pZeroCrossCount->IsDataReady())
	{ 
		// Finish pitch processing if necessary.
		CProcessGrappl* pAutoPitch = (CProcessGrappl*)pDoc->GetGrappl();
		nResult = LOWORD(pAutoPitch->Process(this, pDoc, nProgress, nLevel+1000)); // process data
		if (pAutoPitch->IsDataReady())
		{
			// Finish fragmenting if necessary.
			CProcessFragments* pFragments = (CProcessFragments*)pDoc->GetFragments();
			nResult = LOWORD(pFragments->Process(this, pDoc, nProgress, nLevel+1000)); // process data

			// If waveform fragmented successfully, generate formant data.
			if (pFragments->IsDataReady() && pZeroCrossCount->IsDataReady())
			{
				DWORD dwFirstFragment = pFragments->GetFragmentIndex(dwWaveDataStart/nSmpSize);
				DWORD dwLastFragment = pFragments->GetFragmentIndex((dwWaveDataStart+dwWaveDataLength-nSmpSize)/nSmpSize);

				CProcessFormants* pFormants = (CProcessFormants*)pDoc->GetFormants();
				SPECT_PROC_SELECT SpectraSelected;
				SpectraSelected.bCepstralSpectrum = FALSE;    // turn off to reduce processing time
				SpectraSelected.bLpcSpectrum = TRUE;          // use Lpc method for estimating formants  
				BOOL bFormantTracking = TRUE;
				pFormants->ResetTracking();
				pFormants->SetDataInvalid();

				// open the temporary file
				if (!CreateTempFile(_T("SFM")))
				{ // error opening file
					ErrorMessage(IDS_ERROR_OPENTEMPFILE, GetProcessFileName());
					SetDataInvalid();
					return MAKELONG(PROCESS_ERROR, nProgress);
				}

#ifdef DUMP_FORMANT_TRACKS
				char DumpFile[MAX_PATH];
				strcpy(DumpFile, pDoc->GetFileStatus()->m_szFullName);
				strcpy(&DumpFile[strlen(DumpFile)-4], "_Formant.tbl");
				FILE *hDump = fopen(DumpFile, "w");
				fprintf(hDump, "Frag\tSmp\tLen\tID\tF0\tF1\tBW1\tE1\tF2\tBW2\tE2\tF3\tBW3\tE3\t%% Err\tNV\n");
#endif

				// Extract formants for each fragment.
				FORMANT_FREQ FormantFreq;
				FormantFreq.F[0] = (float)NA;
				BOOL bVoiced = TRUE, bFricative = FALSE;
				DWORD dwVoicedFragStart = (DWORD)UNDEFINED_DATA;

				DWORD dwFragmentIndex;
				for (dwFragmentIndex = dwFirstFragment; dwFragmentIndex <= dwLastFragment; dwFragmentIndex++)
				{ 
					int nMyProgress = int(100*double(dwFragmentIndex - dwFirstFragment)/(dwLastFragment - dwFirstFragment));
					SetProgress(nMyProgress);
					if (IsCanceled() || nResult < 0)
						break;
					// for each spectrum
					FRAG_PARMS FragmentParmInfo = pFragments->GetFragmentParms(dwFragmentIndex);  // get fragment parameters
					DWORD dwFrameStartIndex, dwFrameEndIndex;

					dwFrameStartIndex = DWORD(FragmentParmInfo.dwOffset+FragmentParmInfo.wLength/2.-0.005*pFmtParm->dwSamplesPerSec);
					if (dwFrameStartIndex == UNDEFINED_OFFSET || dwFrameStartIndex > FragmentParmInfo.dwOffset + FragmentParmInfo.wLength)
						dwFrameStartIndex = 0;
					if (dwFrameStartIndex > FragmentParmInfo.dwOffset)
						dwFrameStartIndex = FragmentParmInfo.dwOffset;

					dwFrameEndIndex = DWORD(FragmentParmInfo.dwOffset+FragmentParmInfo.wLength/2.+0.005*pFmtParm->dwSamplesPerSec);
					if (dwFrameEndIndex == UNDEFINED_OFFSET || dwFrameEndIndex < FragmentParmInfo.dwOffset)
						dwFrameEndIndex = pDoc->GetDataSize()/nSmpSize;
					if (dwFrameEndIndex < FragmentParmInfo.dwOffset+FragmentParmInfo.wLength)
						dwFrameEndIndex = FragmentParmInfo.dwOffset+FragmentParmInfo.wLength;

					BOOL bValidCount;
					short nZeroCrossCount = (short)(pZeroCrossCount->GetProcessedData(dwFrameStartIndex/CALCULATION_INTERVAL(pFmtParm->dwSamplesPerSec), &bValidCount));
					double fZeroCrossRate;
					if (bValidCount) 
					{ // calculate zero crossing rate
						UINT nCalcDataLength = CALCULATION_DATALENGTH(pFmtParm->dwSamplesPerSec) * pFmtParm->dwSamplesPerSec/22050;  //!!based on min pitch?
						fZeroCrossRate = (double)nZeroCrossCount * (double)pFmtParm->dwSamplesPerSec / (double)nCalcDataLength;
						bFricative = (fZeroCrossRate >= FRICTION_THRESHOLD);  // is a fricative if at or above threshold
						//	  bFricative = FALSE;  //!! REMOVE THIS ONCE FRICATIVE THRESHOLD IS ACCURATELY DETERMINED
					}
					else bFricative = FALSE;  

					DWORD dwFrameStart = dwFrameStartIndex * (DWORD)nSmpSize;  // byte offset for beginning of fragment
					DWORD dwFrameSize = (dwFrameEndIndex) * (DWORD)nSmpSize - dwFrameStart;  // size of frame in bytes

					bVoiced = pAutoPitch->IsVoiced(pDoc, dwFrameStart) &&    // beginning of fragment is voiced
						pAutoPitch->IsVoiced(pDoc, dwFrameStart+dwFrameSize-nSmpSize);  // end of fragment is voiced        

					// Accumulate formants across a voiced, non-fricative contour.
					if (TRUE || (bVoiced && !bFricative))
					{ // voiced but not a fricative
						if (dwVoicedFragStart == (DWORD)UNDEFINED_DATA) 
							dwVoicedFragStart = dwFragmentIndex;  // save index to first voiced spectrum
						nResult = LOWORD(pFormants->Process(this, pDoc, bFormantTracking,
							dwFrameStart, dwFrameSize, SpectraSelected, nMyProgress, nLevel+1000)); // compute formant frequencies
						if (!pFormants->IsDataReady()) break;
					}

					// At the end of voiced run, clean up formant tracks and save in spectrogram formant track temp file.  
					else 
					{ // unvoiced -- end of voiced run
						if (pFormants->IsDataReady())
						{
							if (bSmooth) 
								pFormants->SmoothMedian(pDoc);   // apply median smoother to formant data if formant frames are contiguous
							DWORD dwFormantFrame = 0;
							for (DWORD dwFormantIndex = dwVoicedFragStart; dwFormantIndex < dwFragmentIndex; dwFormantIndex++)
							{ // update formant frequencies
								FORMANT_FREQ FormantPwr;
								FORMANT_FRAME *pFormantFrame = pFormants->GetFormantFrame(dwFormantFrame++);
								float MaxPowerInDecibels = FLT_MAX_NEG;
								for (USHORT nFormant = 1; nFormant <= MAX_NUM_FORMANTS; nFormant++)
								{
									if (pFormantFrame->Formant[nFormant].Lpc.PowerInDecibels != (float)NA &&
										MaxPowerInDecibels > pFormantFrame->Formant[nFormant].Lpc.PowerInDecibels)    // find max power
										MaxPowerInDecibels = pFormantFrame->Formant[nFormant].Lpc.PowerInDecibels;
								}
#ifdef DUMP_FORMANT_TRACKS
								FRAG_PARMS FragmentParm = pFragments->GetFragmentParms(dwFormantIndex);
								fprintf(hDump, "%lu\t%lu\t%u", dwFormantIndex, FragmentParm.dwOffset, FragmentParm.wLength); 
								CSegment* pSegments = pDoc->GetSegment(PHONETIC);
								const bWithin = TRUE;
								int nSegment = pSegments->FindFromPosition(FragmentParm.dwOffset*nSmpSize, bWithin);
								CString Annot = pSegments->GetSegmentString(nSegment);
								fprintf(hDump, "\t%s", nSegment < 0 ? " " : (LPCTSTR)Annot);
#endif
								for (USHORT nFormant = 0; nFormant <= MAX_NUM_FORMANTS; nFormant++)
								{
									FormantFreq.F[nFormant] = pFormantFrame->Formant[nFormant].Lpc.FrequencyInHertz;
#ifdef DUMP_FORMANT_TRACKS
									if (nFormant <= 3) fprintf(hDump, "\t%4.0f", pFormantFrame->Formant[nFormant].Lpc.FrequencyInHertz); 
#endif
									if (nFormant >= 1)
									{
										if (FormantFreq.F[nFormant] != (float)NA && MaxPowerInDecibels != FLT_MAX_NEG)
										{ // suppress formants with relatively low power compared to that of highest energy formant
											// in the current frame
											if (MaxPowerInDecibels - pFormantFrame->Formant[nFormant].Lpc.PowerInDecibels > RELATIVE_POWER_THRESHOLD)
												FormantFreq.F[nFormant] = (float)NA;    
											// or with the formant power of previous frame    
											//                         if (dwFormantFrame && FormantPwr.F[nFormant] != FLT_MAX_NEG && 
											//                             (FormantPwr.F[nFormant] - pFormantFrame->Formant[nFormant].Lpc.PowerInDecibels > RELATIVE_POWER_THRESHOLD))
											FormantFreq.F[nFormant] = (float)NA; 
										}    
										FormantPwr.F[nFormant] = pFormantFrame->Formant[nFormant].Lpc.PowerInDecibels;
#ifdef DUMP_FORMANT_TRACKS
										if (nFormant <= 3) 
										{
											if (pFormantFrame->Formant[nFormant].Lpc.BandwidthInHertz == (float)NA)
												fprintf(hDump, "\tN/A\tN/A");
											else fprintf(hDump, "\t%3.0f\t%3.0f", 
												pFormantFrame->Formant[nFormant].Lpc.BandwidthInHertz, pFormantFrame->Formant[nFormant].Lpc.PowerInDecibels); 
										}
#endif
									} 
								}
#ifdef DUMP_FORMANT_TRACKS
								if (pFormantFrame->LpcErrorInPercent == (double)NA)
									fprintf(hDump, "\tN/A"); 
								else fprintf(hDump, "\t%4.1f", pFormantFrame->LpcErrorInPercent); 

								CString NearestVowel = pFormants->FindNearestVowel(FormantFreq);
								BOOL bDifferent = strcmp((LPCTSTR)Annot, (LPCTSTR)NearestVowel);
								if (bDifferent) fprintf(hDump, "\t%s*\n", (LPCTSTR)NearestVowel);
								else fprintf(hDump, "\t%s\n", (LPCTSTR)NearestVowel);
#endif
								try
								{ // write the formant frequenciess
									Write((HPSTR)&FormantFreq, (UINT)sizeof(FormantFreq)); 
								}
								catch(CFileException e)
								{ // error writing file
									pFormants->ResetTracking();
									ErrorMessage(IDS_ERROR_WRITETEMPFILE, GetProcessFileName());
									return Exit(PROCESS_ERROR); // error, writing failed
								}
							}
							pFormants->ResetTracking();
							pFormants->SetDataInvalid();
							dwVoicedFragStart = (DWORD)UNDEFINED_DATA;
						}  
						if (FormantFreq.F[0] != (float)UNVOICED)
						{ // update formant frequencies for this frame
							FormantFreq.F[0] = (float)UNVOICED;  // set fundamental frequency to unvoiced value.
							for (unsigned int nFormant = 1; nFormant <= MAX_NUM_FORMANTS; nFormant++)
								FormantFreq.F[nFormant] = (float)NA;  // all others not available
						}
						try
						{ // write unvoiced formant frame
							Write((HPSTR)&FormantFreq, (UINT)sizeof(FormantFreq));
						}  
						catch(CFileException e)
						{ // error writing file
							pFormants->ResetTracking();
							ErrorMessage(IDS_ERROR_WRITETEMPFILE, GetProcessFileName());
							return Exit(PROCESS_ERROR); // error, writing failed
						}
					}  
				}  


				// Never went unvoiced -- process formant frames.
				if (pFormants->IsDataReady() && !IsCanceled())// && bVoiced && !bFricative)
				{
					if (pFormants->IsDataReady())
					{
						if (bSmooth) pFormants->SmoothMedian(pDoc);   // apply median smoother to formant data if frames are continguous
						DWORD dwFormantFrame = 0;
						for (DWORD dwFormantIndex = dwVoicedFragStart; dwFormantIndex < dwFragmentIndex; dwFormantIndex++)
						{
							FORMANT_FREQ FormantPwr;
							FORMANT_FRAME *pFormantFrame = pFormants->GetFormantFrame(dwFormantFrame++);
							//                FILE *hDump = fopen("formants.txt", "w");                
							float MaxPowerInDecibels = FLT_MAX_NEG;
							for (USHORT nFormant = 0; nFormant <= MAX_NUM_FORMANTS; nFormant++)
								if (pFormantFrame->Formant[nFormant].Lpc.PowerInDecibels != (float)NA &&
									MaxPowerInDecibels > pFormantFrame->Formant[nFormant].Lpc.PowerInDecibels)
									MaxPowerInDecibels = pFormantFrame->Formant[nFormant].Lpc.PowerInDecibels;  // find max power
#ifdef DUMP_FORMANT_TRACKS
							FRAG_PARMS FragmentParm = pFragments->GetFragmentParms(dwFormantIndex);
							fprintf(hDump, "%lu\t%lu\t%u", dwFormantIndex, FragmentParm.dwOffset, FragmentParm.wLength); 
							CSegment* pSegments = pDoc->GetSegment(PHONETIC);
							const bWithin = TRUE;
							int nSegment = pSegments->FindFromPosition(FragmentParm.dwOffset*nSmpSize, bWithin);
							fprintf(hDump, "\t%s", nSegment < 0 ? " " : (LPCTSTR)pSegments->GetSegmentString(nSegment));
#endif
							for (USHORT nFormant = 0; nFormant <= MAX_NUM_FORMANTS; nFormant++)
							{
								FormantFreq.F[nFormant] = pFormantFrame->Formant[nFormant].Lpc.FrequencyInHertz;
#ifdef DUMP_FORMANT_TRACKS
								if (nFormant <= 3) fprintf(hDump, "\t%4.0f", pFormantFrame->Formant[nFormant].Lpc.FrequencyInHertz); 
#endif
								if (nFormant >= 1)
								{
									if (FormantFreq.F[nFormant] != (float)NA && MaxPowerInDecibels != FLT_MAX_NEG)
									{ // suppress formants with relatively low energy compared to that of highest energy formant
										// in the current frame
										if (MaxPowerInDecibels - pFormantFrame->Formant[nFormant].Lpc.PowerInDecibels > RELATIVE_POWER_THRESHOLD)
											FormantFreq.F[nFormant] = (float)NA;    
										// or with the formant power of previous frame    
										//                      if (dwFormantFrame && FormantPwr.F[nFormant] != FLT_MAX_NEG && 
										//                          (FormantPwr.F[nFormant] - pFormantFrame->Formant[nFormant].Lpc.PowerInDecibels > RELATIVE_POWER_THRESHOLD))
										FormantFreq.F[nFormant] = (float)NA;    
									}    
									FormantPwr.F[nFormant] = pFormantFrame->Formant[nFormant].Lpc.PowerInDecibels;
#ifdef DUMP_FORMANT_TRACKS
									if (nFormant <= 3) 
									{
										if (pFormantFrame->Formant[nFormant].Lpc.BandwidthInHertz == (float)NA)
											fprintf(hDump, "\tN/A\tN/A");
										else fprintf(hDump, "\t%3.0f\t%3.0f", 
											pFormantFrame->Formant[nFormant].Lpc.BandwidthInHertz, pFormantFrame->Formant[nFormant].Lpc.PowerInDecibels); 
									}
#endif
								} 
							}
#ifdef DUMP_FORMANT_TRACKS
							fprintf(hDump, "\t%s\n", (LPCTSTR)pFormants->FindNearestVowel());
#endif
							try
							{ // write the formants
								Write((HPSTR)&FormantFreq, (UINT)sizeof(FormantFreq));  
							}  
							catch(CFileException e)
							{ // error writing file
								pFormants->ResetTracking();
								ErrorMessage(IDS_ERROR_WRITETEMPFILE, GetProcessFileName());
								return Exit(PROCESS_ERROR); // error, writing failed
							}
						}
						pFormants->ResetTracking();  
					}          
				}    
#ifdef DUMP_FORMANT_TRACKS					 
				fclose(hDump);
#endif
			}  
		}
	}  

	// Handle cancellation or processing errors.
	if (nResult < 0)
	{
		if (nResult == PROCESS_CANCELED) 
			CancelProcess();
		return Exit(nResult);
	}    

	// Update progress and indicate data is ready.
	nProgress = nProgress + (int)(100 / nLevel); // calculate the actual progress
	SetProgress(nProgress);
	EndProcess((nProgress >= 95)); // end data processing
	EndWaitCursor();
	// close the temporary file and read the status
	CloseTempFile(); // close the file
	SetDataReady();
	return MAKELONG(nLevel, nProgress);        

}

/***************************************************************************/
// CProcessSpectroFormants::AreFormantTracksReady Return TRUE if formant data is ready
/***************************************************************************/
BOOL CProcessSpectroFormants::AreFormantTracksReady()
{
	if (IsCanceled()) return FALSE;
	return IsDataReady();
}

