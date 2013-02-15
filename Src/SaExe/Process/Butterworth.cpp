// Butterworth.cpp: implementation of the CButterworth class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Butterworth.h"
#include "resource.h"
#include "isa_doc.h"
#include "sa_w_adj.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CIIRFilter::CIIRFilter(BOOL bDstWBench)
{
	m_pSourceProcess = NULL;
	m_bSrcWBenchProcess = TRUE;  
	m_bDstWBenchProcess = bDstWBench;  
	m_bFilterFilter = FALSE;
	m_nFilterFilterSilence = DEFAULT_FILTER_FILTER_SILENCE_SAMPLES;
}

CIIRFilter::~CIIRFilter()
{
}

static void StoreWaveData(int nData, int wSmpSize, void *pTargetData)
{
	// save data
	if (wSmpSize == 1) // 8 bit data
	{
		if (nData > 127) nData = 127;
		if (nData < -128) nData = -128;
		BYTE bData = (BYTE)(nData + 128);
		*((BYTE*)pTargetData) = bData;
	}
	else                  // 16 bit data
	{
		if (nData > 32767) nData = 32767;
		if (nData < -32768) nData = -32768;
		*((unsigned short *)pTargetData) = (unsigned short)nData;
	}
}

long CIIRFilter::Process(void* pCaller, ISaDoc* pDoc, int nProgress, int nLevel)
{
	TRACE("IIRFilter::Process %d %d\n",nProgress,nLevel);

	if (IsCanceled()) return MAKELONG(PROCESS_CANCELED, nProgress); // process canceled

	// check if nested workbench processes
	int nOldLevel = nLevel; // save original level
	CDataProcess* pLowerProcess = NULL;
	pLowerProcess = m_pSourceProcess;
	if (pLowerProcess)
	{
		TRACE("process lower\n");
		// there is at least one source processes to process first
		long lResult = pLowerProcess->Process(pCaller, pDoc, nProgress, ++nLevel);
		nLevel = (short int)LOWORD(lResult);
		nProgress = HIWORD(lResult);
	}

	if ((nLevel == nOldLevel) && (IsDataReady())) 
		return MAKELONG(--nLevel, nProgress); // data is already ready
	else
		SetDataInvalid();

	if (nLevel < 0) // memory allocation failed or previous processing error
	{
		if ((nLevel == PROCESS_CANCELED))
			CancelProcess(); // set your own cancel flag
		return MAKELONG(nLevel, nProgress);
	}

	TRACE("start process\n");
	// start process
	BeginWaitCursor(); // wait cursor
	if (!StartProcess(pCaller, IDS_STATTXT_PROCESSWBLP)) // start data processing
	{
		EndProcess(); // end data processing
		EndWaitCursor();
		return MAKELONG(PROCESS_ERROR, nProgress);
	}

	// create the temporary file
	if (!CreateTempFile(_T("IIR"))) // creating error
	{
		EndProcess(); // end data processing
		EndWaitCursor();
		SetDataReady(FALSE);
		return MAKELONG(PROCESS_ERROR, nProgress);
	}

	HPSTR pTargetData; // pointers to target data
	DWORD dwDataPos = 0; // data position pointer

	FmtParm* pFmtParm = pDoc->GetFmtParm(); // get sa parameters format member data
	WORD wDstSmpSize = WORD(m_bDstWBenchProcess ? pFmtParm->wBlockAlign / pFmtParm->wChannels : sizeof(short));

	DWORD dwBufferSize = GetBufferSize();

	m_bReverse = (!m_zReverseTransform.IsIdentity());

	// start processing loop
	if (!m_bReverse)
	{
		TRACE("forward\n");
		m_bReverse = FALSE;
		// get source data size
		DWORD dwDataSize;
		if (pLowerProcess)
		{
			dwDataSize = pLowerProcess->GetProcessedWaveDataSize();
		}
		else
		{
			dwDataSize = pDoc->GetUnprocessedDataSize(); // size of raw data
		}
		TRACE("dwDataSize=%d\n",dwDataSize);

		WORD wSrcSmpSize = WORD(m_bSrcWBenchProcess ? pFmtParm->wBlockAlign / pFmtParm->wChannels : sizeof(short));

		if (wDstSmpSize > wSrcSmpSize)
			dwBufferSize /= wDstSmpSize/wSrcSmpSize;

		m_nMinValue = INT_MAX;
		m_nMaxValue = INT_MIN;
		while (dwDataPos < dwDataSize)
		{
			// set progress bar
			//SetProgress(nProgress + (int)(100 * dwDataPos / dwDataSize / (DWORD)nLevel));
			if (IsCanceled()) return Exit(PROCESS_CANCELED); // process canceled

			pTargetData = m_lpData;
			DWORD dwBlockEnd = dwDataPos + dwBufferSize;
			if (dwBlockEnd > dwDataSize)
			{
				dwBlockEnd = dwDataSize;
			}
			DWORD dwBlockStart = dwDataPos;

			while (dwDataPos < dwBlockEnd)
			{
				int nData = ReadSourceData(dwDataPos, wSrcSmpSize, pDoc);
				dwDataPos+= wSrcSmpSize;

				// process data
				nData = round(m_zForwardTransform.Tick(double(nData)));

				if (nData > m_nMaxValue)
					m_nMaxValue = nData;
				if (nData < m_nMinValue)
					m_nMinValue = nData;

				StoreWaveData(nData, wDstSmpSize, pTargetData);
				pTargetData += wDstSmpSize;
			}
			WriteWaveDataBlock(dwBlockStart, m_lpData, (dwBlockEnd-dwBlockStart)*wDstSmpSize/wSrcSmpSize);
			dwDataPos = dwBlockEnd;
		}
		if (m_bFilterFilter)
		{
			// Append some silence to handle phase lag
			DWORD dwSettlingSize = FilterFilterSilenceSamples()*wSrcSmpSize;
			while (dwDataPos < dwDataSize + dwSettlingSize)
			{
				pTargetData = m_lpData;
				DWORD dwBlockEnd = dwDataPos + dwBufferSize;
				if (dwBlockEnd > dwDataSize + dwSettlingSize)
				{
					dwBlockEnd = dwDataSize + dwSettlingSize;
				}
				DWORD dwBlockStart = dwDataPos;

				while (dwDataPos < dwBlockEnd)
				{
					// process data
					dwDataPos+= wSrcSmpSize;

					int nData;
					nData = round(m_zForwardTransform.Tick(0.));

					StoreWaveData(nData, wDstSmpSize, pTargetData);
					pTargetData += wDstSmpSize;
				}
				WriteWaveDataBlock(dwBlockStart, m_lpData, dwBufferSize*wDstSmpSize/wSrcSmpSize);
			}
		}
		Dump("butterworth end forward");
	}
	else
	{
		TRACE("reverse\n");
		// process in reverse
		// first do forward pass
		CIIRFilter forwardPass(m_bDstWBenchProcess);

		forwardPass.SetSourceProcess(m_pSourceProcess, m_bSrcWBenchProcess);

		forwardPass.m_zForwardTransform *= m_zForwardTransform;
		forwardPass.SetFilterFilter(TRUE);
		forwardPass.SetFilterFilterSilenceSamples(FilterFilterSilenceSamples());

		long lResult = forwardPass.Process(pCaller, pDoc, nProgress, ++nLevel);
		nLevel = (short int)LOWORD(lResult);
		nProgress = HIWORD(lResult);
		if ((nLevel == PROCESS_CANCELED))
		{
			CancelProcess(); // set your own cancel flag
			return MAKELONG(nLevel, nProgress);
		}

		m_pSourceProcess = &forwardPass;

		forwardPass.Dump("forwardPass"	);

		WORD wSmpSize = wDstSmpSize;
		DWORD dwDataSize;
		if (forwardPass.m_pSourceProcess)
			dwDataSize = forwardPass.m_pSourceProcess->GetDataSize()*wSmpSize;
		else
			dwDataSize = (forwardPass.GetDataSize() - FilterFilterSilenceSamples())*wSmpSize;

		dwDataPos = forwardPass.GetDataSize()*wSmpSize;

		while (dwDataPos > dwDataSize)
		{
			// process silence 
			dwDataPos-= wSmpSize;
			int nData = ReadSourceData(dwDataPos, wSmpSize, pDoc);
			// process data
			nData = round(m_zReverseTransform.Tick(double(nData)));
		}
		m_nMinValue = INT_MAX;
		m_nMaxValue = INT_MIN;

		// start processing loop
		while (dwDataPos > 0)
		{
			// set progress bar
			//      SetProgress(nProgress + (int)(100 * (dwDataSize - dwDataPos) / dwDataSize / (DWORD)nLevel));
			if (IsCanceled()) 
			{
				m_pSourceProcess = forwardPass.m_pSourceProcess;
				return Exit(PROCESS_CANCELED); // process canceled
			}

			DWORD dwBlockEnd = dwDataPos - dwBufferSize;
			if (dwDataPos < dwBufferSize)
			{
				dwBlockEnd = 0;
				dwBufferSize = dwDataPos;
			}
			DWORD dwBlockStart = dwBlockEnd;

			pTargetData = m_lpData + dwBufferSize;
			TRACE("dwDataPos=%d\n",dwDataPos);
			while (dwDataPos > dwBlockEnd)
			{
				dwDataPos-= wSmpSize;
				int nData = ReadSourceData( dwDataPos, wSmpSize, pDoc);

				// process data
				nData = round(m_zReverseTransform.Tick(double(nData)));

				if (nData > m_nMaxValue)
					m_nMaxValue = nData;
				if (nData < m_nMinValue)
					m_nMinValue = nData;

				pTargetData -= wSmpSize;
				StoreWaveData(nData, wSmpSize, pTargetData);
			}
			WriteWaveDataBlock(dwBlockStart, m_lpData, dwBufferSize);
			dwDataPos = dwBlockEnd;
		}

		// Preserve source setting so that we can use it
		m_pSourceProcess = forwardPass.m_pSourceProcess;
		Dump("butterworth end reverse");
	}

	nProgress = nProgress + (int)(100 / nLevel); // calculate the actual progress
	// close the temporary file and read the status
	CloseTempFile(); // close the file
	EndProcess((nProgress >= 95)); // end data processing
	EndWaitCursor();
	SetDataReady();

	Dump("iirfilter end process");

	return MAKELONG(nLevel, nProgress);
}

void CIIRFilter::SetSourceProcess(CDataProcess * pSourceProcess, BOOL bWBenchProcess)
{
	m_bSrcWBenchProcess = bWBenchProcess;

	if (m_pSourceProcess != pSourceProcess)
		SetDataInvalid();

	m_pSourceProcess = pSourceProcess;
}

int CIIRFilter::ReadSourceData(DWORD dwDataPos, int wSmpSize, ISaDoc* pDoc)
{
	CDataProcess* pLowerProcess = m_pSourceProcess;

	if (!pLowerProcess)
	{
		pLowerProcess = pDoc->GetAdjust();  // This is the default node
	}

	// read data
	int nData;
	if (wSmpSize == 1) // 8 bit data
	{
		void* pSourceData = pLowerProcess->GetProcessedObject(dwDataPos, 1, m_bReverse);
		if (!pSourceData)
		{
			ASSERT(FALSE);
			TRACE(_T("Failed reading source data\n"));
			return 0;
		}
		BYTE bData;
		bData = *((BYTE*)pSourceData); // data range is 0...255 (128 is center)
		nData = bData - 128;
	}
	else                  // 16 bit data
	{
		void* pSourceData = pLowerProcess->GetProcessedObject(dwDataPos>>1, 2, m_bReverse);
		if (!pSourceData)
		{
			ASSERT(FALSE);
			TRACE(_T("Failed reading source data\n"));
			return 0;
		}
		nData = *((short int*)pSourceData);
	}

	return nData;
}

BOOL CIIRFilter::WriteWaveDataBlock(DWORD dwPosition, HPSTR lpData, DWORD dwDataLength)
{
	return WriteDataBlock(dwPosition, lpData, dwDataLength, sizeof(char));
}

CButterworth::CButterworth(BOOL bWorkbenchOutput) : CIIRFilter(bWorkbenchOutput)
{
	m_bFilterFilter = FALSE;
	m_bReverse = FALSE;
	m_nOrder = 0;
	m_ftFilterType = kftUndefined;
	m_dSampling = 0.;
	m_dFrequency = 0.;
	m_dBandwidth = 0.;
	m_dScale = 1;
}

CButterworth::~CButterworth()
{
}

static const double pi = 3.14159265358979323846264338327950288419716939937511;

// ButterworthPole()
// The poles of a normalized butterworth filter are equally spaced 
// around the unit circle in the complex plane.  All poles are in the 
// left plane.  The poles are symmetric about the Real axis, and 
// are not placed on the imaginary axis.  Function returns a pole.  
// The poles are numbered counter clockwise (90-270 degrees)
inline static std::complex<double> ButterworthPole(int nOrder, int nPole)
{

	double dSpacing = pi/ nOrder;
	double dAngle = (pi + dSpacing)/2. + dSpacing*nPole;
	return std::complex<double>(cos(dAngle), sin(dAngle));
}

void CButterworth::CascadeLowPass(CZTransform &zTransform, int nOrder, double dFilterCutoffFreq, double dSamplingFreq, double &tau)
{
	double numerator[3];
	double denominator[3];

	double dDigitalCutoffFreq = (dFilterCutoffFreq/dSamplingFreq)*2.*pi;
	double dAnalogCutoffFreq = 2.*tan(dDigitalCutoffFreq/2.);  // warp to analog frequency

	double tauSq = tau*tau;

	for(int i = 0;i < (nOrder+1)/2;i++)
	{
		std::complex<double> pole = ButterworthPole(nOrder, i);

		// Since the Butterworth pole is normalized to 1 
		// it must be scaled to the desired analog filter cutoff
		pole *= dAnalogCutoffFreq;

		// Generate digital filter
		if ((i * 2 + 1) == nOrder)  // last one is single real pole
		{
			// each first order analog filter takes the general form
			//
			//               -pole/(s - pole)
			//
			// Substituting the bilinear transformation 
			//                          -1          -1
			//              s = 2 (1 - z  ) / (1 + z  )
			//
			// yields the following coefficients
			denominator[0] = 1.;
			denominator[1] = (2 + pole.real())/(pole.real()-2.);
			numerator[0] = pole.real()/(pole.real() - 2);
			numerator[1] = numerator[0];
			zTransform *= CZTransform(1, numerator, denominator);

			double lp = fabs(denominator[1]) > 1e-6 ? log(fabs(denominator[1])) : log(1e-6);
			tauSq += 1/(lp*lp) + 1;
			break;
		}
		else   // conjugate pole pairs
		{ 
			// each second order analog filter takes the general form
			//
			//               -pole      -pole*
			//              --------   ---------  , where * denotes conjugate    
			//             (s - pole) (s - pole*)
			//
			//                           2
			//                     |pole|
			//           = --------------------------
			//              2                        2
			//             s  - 2 s Re(pole) + |pole|
			//
			// Substituting the bilinear transformation 
			//                          -1          -1
			//              s = 2 (1 - z  ) / (1 + z  )
			//
			// yields the following coefficients

			double PoleMagSquared = (pole.real()*pole.real() + pole.imag()*pole.imag());
			double NormFactor = 4. - 4.* pole.real() + PoleMagSquared;
			denominator[0] = 1.;
			denominator[1] = (2.*PoleMagSquared - 8.)/NormFactor;
			denominator[2] = (4. + 4.*pole.real() + PoleMagSquared) / NormFactor;
			numerator[0] = PoleMagSquared / NormFactor;
			numerator[1] = 2. * numerator[0];
			numerator[2] = numerator[0];
			zTransform *= CZTransform(2, numerator, denominator);

			double lp = fabs(denominator[2]) > 1e-6 ? log(fabs(denominator[2]))/2 : log(1e-6)/2;
			tauSq += 2/(lp*lp) + 2;
		}
	}
	tau = sqrt(tauSq);
}

// Cascades a highpass nOrder butterworth filter to current working filter
void CButterworth::CascadeHighPass(CZTransform &zTransform, int nOrder, double dFilterCutoffFreq, double dSamplingFreq, double &tau)
{
	double numerator[3];
	double denominator[3];

	double dDigitalCutoffFreq = (dFilterCutoffFreq/dSamplingFreq)*2.*pi;
	double dAnalogCutoffFreq = 2.*tan(dDigitalCutoffFreq/2.);  // warp to analog frequency

	double tauSq = tau*tau;

	for(int i = 0;i < (nOrder+1)/2;i++)
	{
		std::complex<double> pole = ButterworthPole(nOrder, i);

		// Since the Butterworth pole is normalized to 1 
		// it must be scaled to the desired analog filter cutoff
		pole /= dAnalogCutoffFreq;

		// Generate digital filter. 
		// The low pass stages are converted to high pass stages using the tranformation
		//     s -> 1/s
		// 
		// It suffices to invert the bilinear transform
		//                            -1          -1
		//              s = 0.5 (1 + z  ) / (1 - z  )
		//
		// and substitute into the low pass equations, yielding the following coefficients
		if ((i * 2 + 1) == nOrder)  // last one is single real pole
		{ // first order section
			denominator[0] = 1.;
			denominator[1] = (1. + 2.*pole.real()) / (1. - 2.*pole.real());
			numerator[0] = 2.*pole.real() / (2.*pole.real() - 1.);
			numerator[1] = -numerator[0];
			zTransform *= CZTransform(1, numerator, denominator);

			double lp = fabs(denominator[1]) > 1e-6 ? log(fabs(denominator[1])) : log(1e-6);
			tauSq += 1/(lp*lp) + 1;
			break;
		}
		else
		{ // second order section, taking complex conjugate pole pairs
			double dPoleMagSquared = (pole.real()*pole.real() + pole.imag()*pole.imag());
			double dNormFactor = 1. - 4.* pole.real() + 4.* dPoleMagSquared;
			denominator[0] = 1.;
			denominator[1] = (2. - 8.*dPoleMagSquared) / dNormFactor;
			denominator[2] = (1. + 4.*pole.real() + 4.*dPoleMagSquared) / dNormFactor;
			numerator[0] = (4.*dPoleMagSquared) / dNormFactor;
			numerator[1] = -2.*numerator[0];
			numerator[2] = numerator[0];
			zTransform *= CZTransform(2, numerator, denominator);

			double lp = fabs(denominator[2]) > 1e-6 ? log(fabs(denominator[2]))/2 : log(1e-6)/2;
			tauSq += 2/(lp*lp) + 2;
		}
	}
	tau = sqrt(tauSq);
}


// Cascades a scale term to scale output
void CButterworth::CascadeScale(CZTransform &zTransform, double dScale)
{
	zTransform *= CZTransform(0, &dScale, NULL);
}


void CButterworth::ClearFilter()
{
	m_zForwardTransform = CZTransform(0, NULL, NULL);
	m_zReverseTransform = CZTransform(0, NULL, NULL);
}


double CButterworth::FilterFilterNorm(int nOrder) const
{
	if (m_bFilterFilter)
		return exp(log(sqrt(2.) - 1.)/(2*nOrder));
	else
		return 1.;
}


/***************************************************************************/
// CButterworth::Process 
/***************************************************************************/
/***************************************************************************/
// CProcessWbLowpass::Process Processing new raw data with a lowpass function
// The processed change data is stored in a temporary file. To create it
// helper functions of the base class are used. While processing a process
// bar, placed on the status bar, has to be updated. The level tells which
// processing level this process has been called, start process start on
// which processing percentage this process starts (for the progress bar).
// The status bar process bar will be updated depending on the level and the
// progress start. The return value returns the highest level througout the
// calling queue, or -1 in case of an error in the lower word of the long
// value and the end process progress percentage in the higher word.
/***************************************************************************/
long CButterworth::Process(void* pCaller, ISaDoc* pDoc, int nProgress, int nLevel)
{
	if (IsCanceled()) return MAKELONG(PROCESS_CANCELED, nProgress); // process canceled
	// check if nested workbench processes

	if (IsDataReady())
		return MAKELONG(--nLevel, nProgress); // data is already ready

	FmtParm* pFmtParm = pDoc->GetFmtParm(); // get sa parameters format member data

	ConfigureProcess(pFmtParm->dwSamplesPerSec);

	return CIIRFilter::Process(pCaller, pDoc, nProgress, nLevel);
}


void CButterworth::HighPass(int nOrder, double dFrequency, double dScale)
{
	if (m_nOrder != nOrder || m_dFrequency != dFrequency || m_dScale != dScale || m_ftFilterType != kftHighPass)
		SetDataInvalid();

	m_nOrder = nOrder;
	m_dFrequency = dFrequency;
	m_ftFilterType = kftHighPass;
	m_dScale = dScale;
}

void CButterworth::LowPass(int nOrder, double dFrequency, double dScale)
{
	if (m_nOrder != nOrder || m_dFrequency != dFrequency || m_dScale != dScale || m_ftFilterType != kftLowPass)
		SetDataInvalid();

	m_nOrder = nOrder;
	m_dFrequency = dFrequency;
	m_ftFilterType = kftLowPass;
	m_dScale = dScale;
}

void CButterworth::BandPass(int nOrder, double dFrequency, double dBandwidth, double dScale)
{
	if (m_nOrder != nOrder || m_dFrequency != dFrequency || m_dBandwidth != dBandwidth || m_dScale != dScale || m_ftFilterType != kftBandPass)
		SetDataInvalid();

	m_nOrder = nOrder;
	m_dFrequency = dFrequency;
	m_dBandwidth = dBandwidth;
	m_ftFilterType = kftBandPass;
	m_dScale = dScale;
}

void CButterworth::ConfigureProcess(double dSampling)
{
	double tau = 0;
	ClearFilter();
	if (m_bReverse)
	{
		double rTau = 0;
		switch(m_ftFilterType)
		{
		case kftHighPass:
			CascadeHighPass(m_zReverseTransform, m_nOrder, m_dFrequency*FilterFilterNorm(m_nOrder), dSampling, rTau);
			break;
		case kftLowPass:
			CascadeLowPass(m_zReverseTransform, m_nOrder, m_dFrequency/FilterFilterNorm(m_nOrder), dSampling, rTau);
			break;
		case kftBandPass:
			// Assume a low pass followed by a high pass will suffice
			//   We should probably adjust the filter for center frequency gain
			//   But this is good enough for now...
			CascadeLowPass(m_zReverseTransform, m_nOrder, (m_dFrequency + m_dBandwidth/2.)/FilterFilterNorm(m_nOrder), dSampling, rTau);
			CascadeHighPass(m_zReverseTransform, m_nOrder, (m_dFrequency - m_dBandwidth/2.)*FilterFilterNorm(m_nOrder), dSampling, rTau);
			break;
		default:
			// null filter... We could assert here
			TRACE(_T("Filter not configured\n"));
		}
	}
	switch(m_ftFilterType)
	{
	case kftHighPass:
		CascadeHighPass(m_zForwardTransform, m_nOrder, m_dFrequency*FilterFilterNorm(m_nOrder), dSampling, tau);
		CascadeScale(m_zForwardTransform, m_dScale);
		break;
	case kftLowPass:
		CascadeLowPass(m_zForwardTransform, m_nOrder, m_dFrequency/FilterFilterNorm(m_nOrder), dSampling, tau);
		CascadeScale(m_zForwardTransform, m_dScale);
		break;
	case kftBandPass:
		// Assume a low pass followed by a high pass will suffice
		//   We should probably adjust the filter for center frequency gain
		//   But this is good enough for now...
		CascadeLowPass(m_zForwardTransform, m_nOrder, (m_dFrequency + m_dBandwidth/2.)/FilterFilterNorm(m_nOrder), dSampling, tau);
		CascadeHighPass(m_zForwardTransform, m_nOrder, (m_dFrequency - m_dBandwidth/2.)*FilterFilterNorm(m_nOrder), dSampling, tau);
		CascadeScale(m_zForwardTransform, m_dScale);
		break;
	default:
		// null filter... We could assert here
		TRACE(_T("Filter not configured\n"));
	}
	SetFilterFilterSilenceSamples(int(tau*11 + 1));
}


const double CHilbert::Pole1000x96dB[] =
{
	9.9935401489275E-001,
	9.9795415607235E-001,
	9.9621092946349E-001,
	9.9383204108577E-001,
	9.9042122918253E-001,
	9.8541559170058E-001,
	9.7800017740675E-001,
	9.6699192390373E-001,
	9.5068735052105E-001,
	9.2667756716555E-001,
	8.9165794724631E-001,
	8.4130973059560E-001,
	7.7041566881683E-001,
	6.7347367595098E-001,
	5.4609671365400E-001,
	3.8722723362882E-001,
	2.0147243962615E-001,
	0 // Real Pole
};

CHilbert::CHilbert( CDataProcess * pSourceProcess, BOOL bWBenchProcess)
{
	pSourceProcess->Dump("hilbert");

	const double *poles = Pole1000x96dB;
	double fTauSq=0;
	double rTauSq=0;
	int poleLast = 0;

	while (poles[poleLast] != 0.)
		poleLast++;

	for(int i=poleLast & 0x1; i < poleLast; i+=2)
	{
		double flp;
		m_zForwardTransform *= AllPass(poles[i]);
		flp = log(poles[i]);
		fTauSq += 2/(flp*flp);
	}
	m_zForwardTransform *= DelayHalf(); // Introduce a single clock delay

	for(int i=(poleLast+1)&0x1; i < poleLast; i+=2)
	{
		double rlp;
		m_zReverseTransform *= AllPass(poles[i]);
		rlp = log(poles[i]);
		rTauSq += 2/(rlp*rlp);
	}

	double fTau = sqrt(fTauSq);
	double rTau = sqrt(rTauSq);  UNUSED_ALWAYS(rTau);

	SetFilterFilterSilenceSamples(int(fTau*11+1.5)+poleLast);
	SetSourceProcess( pSourceProcess, bWBenchProcess);
}

CZTransform CHilbert::AllPass(double pole)
{
	double numerator[3];
	double denominator[3];
	double beta = pole*pole;

	numerator[0] = beta;
	numerator[1] = 0;
	numerator[2] = -1;
	denominator[0] = 1.;
	denominator[1] = 0;
	denominator[2] = -beta;

	CZTransform result(2, numerator, denominator);
	return result;
}

CZTransform CHilbert::DelayHalf()
{
	double numerator[2];

	numerator[0] = 0;
	numerator[1] = 0.5;

	return CZTransform(1, numerator, NULL);
}
