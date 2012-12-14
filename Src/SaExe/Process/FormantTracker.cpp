// FormantTracker.cpp: implementation of the CFormantTracker class.
//
// Author: Steve MacLean
// copyright 2003 SIL
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "sa_doc.h"
#include "sa_proc.h"
#include "dsp\ZTransform.h"
#include "dsp\DspWins.h"
#include "dsp\signal.h"
#include "dsp\Grappl.h"
#include "dsp\Formants.h"
#include "FormantTracker.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CAnalyticLpcAnalysis::CAnalyticLpcAnalysis(const VECTOR_CDBL &signal, int nOrder, const CAnalyticLpcAnalysis *base) : m_nOrder(nOrder)
{
	BuildAutoCorrelation(signal, base);
	BuildPredictorReflectionCoefficients(base);
}

void CAnalyticLpcAnalysis::BuildAutoCorrelation(const VECTOR_CDBL &signal, const CAnalyticLpcAnalysis *base)
{
	if (base)
		m_autocorrelation = base->m_autocorrelation;
	else
		m_autocorrelation.clear();

	if (m_autocorrelation.size() < unsigned(m_nOrder + 1))
	{
		for (int i = m_autocorrelation.size(); i <= m_nOrder; i++)
		{
			CDBL value = 0;
			for (int j=signal.size() - 1; j >= i; j--)
				value += signal[j]*std::conj(signal[j-i]);
			m_autocorrelation.push_back(value);
		}
	}
	else
	{
		m_autocorrelation.resize(m_nOrder + 1);
	}
}

void CAnalyticLpcAnalysis::BuildPredictorReflectionCoefficients(const CAnalyticLpcAnalysis *base)
{
	CDBL &e = m_error;
	VECTOR_CDBL workingPrediction[2];

	UNUSED_ALWAYS(base);

	e = m_autocorrelation[0];

	workingPrediction[0].resize(m_nOrder+1);
	workingPrediction[1].resize(m_nOrder+1);

	workingPrediction[0][0] = -1.;
	workingPrediction[1][0] = -1.;

	m_reflection.resize(m_nOrder);

	for (int i = 1; i<=m_nOrder; i++)
	{
		CDBL ki = m_autocorrelation[i];
		VECTOR_CDBL &lastPrediction = workingPrediction[(i-1)&1];
		VECTOR_CDBL &Prediction = workingPrediction[(i)&1];

		for(int j = 1; j < i; j++)
			ki -= lastPrediction[j]*m_autocorrelation[i - j];

		ki /= e;

		m_reflection[i-1] = -ki;  // reflection coefficient is negative of ki (PARCOR)
		Prediction[i] = ki;

		for(int j = 1; j < i; j++)
			Prediction[j] = lastPrediction[j-1] - ki*lastPrediction[i-j-1];

		e *= (CDBL(1) - ki*ki);
	}

	m_prediction.swap(workingPrediction[(m_nOrder)&1]);
}

class CProcessTrackState : public CFormantTracker::CTrackState
{
public:
	CProcessTrackState() 
	{;}
	virtual ~CProcessTrackState() 
	{;}

	typedef std::complex<double> CDBL;
	typedef std::vector<CDBL> VECTOR_CDBL;
	typedef std::deque<CDBL> DEQUE_CDBL;
	typedef double DBL;
	typedef std::vector<DBL> VECTOR_DBL;

	virtual DEQUE_CDBL & GetData() 
	{ 
		return m_data;
	}
	virtual VECTOR_DBL & GetWindow() 
	{ 
		return m_window;
	}

	// Previous track position
	virtual VECTOR_CDBL & GetTrackIn() 
	{
		return m_trackIn;
	}

	// Result track position
	virtual VECTOR_CDBL & GetTrackOut() 
	{
		return m_trackOut;
	}

	// Working intermediates to eliminate memory thrashing
	virtual VECTOR_CDBL & GetWindowed() 
	{
		return m_windowed;
	}
	virtual VECTOR_CDBL & GetFiltered() 
	{
		return m_filtered;
	}
	virtual VECTOR_CDBL & GetZeroFilterCDBL() 
	{
		return m_zeroFilterCDBL;
	}
	virtual VECTOR_DBL & GetZeroFilterDBL() 
	{
		return m_zeroFilterDBL;
	}

protected:
	DEQUE_CDBL m_data;
	VECTOR_DBL m_window;

	VECTOR_CDBL m_trackIn;
	VECTOR_CDBL m_trackOut;
	VECTOR_CDBL m_windowed;
	VECTOR_CDBL m_filtered;
	VECTOR_CDBL m_zeroFilterCDBL;
	VECTOR_DBL m_zeroFilterDBL;
};

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

const double pi = 3.14159265358979323846264338327950288419716939937511;

CFormantTracker::CFormantTracker(CDataProcess &Real, CDataProcess &Imag, CDataProcess &Pitch)
{
	m_pReal = &Real;
	m_pImag = &Imag;
	m_pPitch = &Pitch;
}

CFormantTracker::~CFormantTracker()
{

}

void CFormantTracker::BuildTrack(CTrackState &state, double samplingRate, int pitch)
{
	// Get Data
	const DEQUE_CDBL &data = state.GetData();

	// Apply Window
	const VECTOR_DBL &window = state.GetWindow();
	VECTOR_CDBL &windowed = state.GetWindowed();

	ASSERT(window.size() == data.size());

	const VECTOR_CDBL & trackIn = state.GetTrackIn();
	VECTOR_CDBL & trackOut = state.GetTrackOut();

	unsigned int tracks = trackIn.size();
	trackOut.resize(tracks);
	unsigned int tracksCalculate;
	tracksCalculate = tracks < 4 ? tracks : 4;

	unsigned int zeroFilterOrder = (trackIn.size() - 1)*(FormantTrackerOptions.m_bAzfAddConjugateZeroes ? 2 : 1) + 1;
	VECTOR_CDBL &filtered = state.GetFiltered();
	filtered.resize(window.size() + zeroFilterOrder);

	if (windowed.size() != window.size() + 2*zeroFilterOrder)
	{
		windowed.clear();
		windowed.assign(window.size() + 2*zeroFilterOrder, 0);
	}
	for(int i = window.size() - 1; i >= 0 ; i--)
	{
		double w = window[i];
		CDBL d = data[i];
		CDBL r = w*d;
		windowed[i + zeroFilterOrder] = r;
	}

	double radiusAZF = exp(-FormantTrackerOptions.m_dAzfMinBandwidth/samplingRate*pi); 
	double radiusDTF = exp(-FormantTrackerOptions.m_dDtfMinBandwidth/samplingRate*pi); 

	double pitchAngle = 2. * pi * pitch / samplingRate;
	CDBL pitchTrack(cos(pitchAngle), sin(pitchAngle));

	for (unsigned int formant = 1; formant <= tracksCalculate; formant++)
	{
		// Build DTF
		CDBL denominator[] = { 1, trackIn[formant].imag() > 0 ? -trackIn[formant]/std::abs(trackIn[formant])*radiusDTF : 0};
		CZTransformCDBL DTF(1, NULL, denominator);

		// Build All Zero Filter
		CZTransformCDBL AZF;

		// Put a zero at pitch so formant 1 doesn't track pitch
		if(FormantTrackerOptions.m_bAzfAddConjugateZeroes && pitchTrack.imag())
		{
			CDBL numerator[] = { 1, -2 * pitchTrack.real() * radiusAZF, radiusAZF * radiusAZF };
			AZF *= CZTransformCDBL(2, numerator, NULL);
		}
		else
		{
			CDBL numerator[] = { 1, -pitchTrack*radiusAZF };
			AZF *= CZTransformCDBL(1, numerator, NULL);
		}

		unsigned int zero = 0;
		for (zero = 1; zero < tracks; zero++)
		{
			if (zero == formant)
				continue;

			CDBL track = (zero < formant) && FormantTrackerOptions.m_bAzfMostRecent ? trackOut[zero] : trackIn[zero];

			if (FormantTrackerOptions.m_bAzfDiscardLpcBandwidth)
				track /= std::abs(track);

			if (FormantTrackerOptions.m_bAzfAddConjugateZeroes && track.imag())
			{
				CDBL numerator[] = { 1, -2*track.real()*radiusAZF, std::norm(track)*radiusAZF*radiusAZF };
				AZF *= CZTransformCDBL(2, numerator, NULL);
			}
			else
			{
				CDBL numerator[] = { 1, -track };
				AZF *= CZTransformCDBL(1, numerator, NULL);
			}
		}

		if (FormantTrackerOptions.m_bAzfAddConjugateZeroes)
		{
			// The filter is real... CDBL*double is faster than CDBL*CDBL so this is an optimization
			VECTOR_DBL &filter = state.GetZeroFilterDBL();

			// Flatten AZF
			filter.resize(zeroFilterOrder+1);
			filter[0] = AZF.Tick(1).real();
			for(unsigned int z=1; z<=zeroFilterOrder; z++)
				filter[z] = AZF.Tick(0).real();

			// Apply AZF & DTF
			for(unsigned int i = zeroFilterOrder; i < windowed.size() ; i++)
			{
				CDBL z = 0;

				for ( unsigned int j = 0; j <= zeroFilterOrder; j++)
					z += windowed[i-j]*filter[j];

				filtered[i - zeroFilterOrder] = DTF.Tick(z);
			}
		}
		else
		{
			// The filter is complex... 
			VECTOR_CDBL &filter = state.GetZeroFilterCDBL();

			// Flatten AZF
			filter.resize(zeroFilterOrder);
			filter[0] = AZF.Tick(1);
			for (unsigned int z=1; z<zeroFilterOrder; z++)
				filter[z] = AZF.Tick(0);

			// Apply AZF & DTF
			for (unsigned int i = zeroFilterOrder; i < windowed.size() ; i++)
			{
				CDBL z = 0;
				for (unsigned int j = 0; j <= zeroFilterOrder; j++)
					z+= windowed[i-j]*filter[j];
				filtered[i - zeroFilterOrder] = DTF.Tick(z);
			}
		}

		// Calculate LPC
		CAnalyticLpcAnalysis lpc(filtered, 1);

		// Store Frequency & BW
		CDBL predictor = lpc.GetPredictor()[1];
		trackOut[formant] = predictor;
		if (formant==1) 
		{
			//for (int i=0;i<filtered.size();i++) 
			//{
			//	TRACE("filtered[%d]=%f\n",i,filtered[i]);
			//}
			//TRACE("trackOut[%d]=%f\n",formant,predictor);
		}
	}
}

long CFormantTracker::Process(void* pCaller, ISaDoc* pDoc, int nProgress, int nLevel)
{
	if (IsCanceled()) 
		return MAKELONG(PROCESS_CANCELED, nProgress); // process canceled

	int nOldLevel = nLevel;			// save original level
	int nOldProgress = nProgress;	// save original progress

	if ((m_pReal == NULL) || (m_pImag == NULL) || (m_pPitch == NULL))
		return MAKELONG(PROCESS_ERROR, nProgress);

	if (!m_pReal->IsDataReady())
	{
		SetDataInvalid();
		long lResult = m_pReal->Process(pCaller, pDoc, nProgress, ++nLevel);
		nLevel = (short int)LOWORD(lResult);
		nProgress = HIWORD(lResult);
	}

	if ((nLevel >= 0) && !m_pImag->IsDataReady())
	{
		SetDataInvalid();
		long lResult = m_pImag->Process( pCaller, pDoc, nProgress, ++nLevel);
		nLevel = (short int)LOWORD(lResult);
		nProgress = HIWORD(lResult);
	}

	if ((nLevel >= 0) && !m_pPitch->IsDataReady())
	{
		SetDataInvalid();
		for (int i = 0; i < 3 && !m_pPitch->IsDataReady(); i++) // make sure pitch is finished!
		{
			long lResult = m_pPitch->Process(pCaller, pDoc, nProgress, ++nLevel);
			nLevel = (short int)LOWORD(lResult);
			nProgress = HIWORD(lResult);
		}
	}

	m_pReal->Dump(__FILE__);
	m_pImag->Dump(__FILE__);
	m_pPitch->Dump(__FILE__);

	if ((nLevel == nOldLevel) && (IsDataReady())) 
		return MAKELONG(--nLevel, nProgress); // data is already ready

	nLevel = nOldLevel; // reset level and progress so progress bar is more realistic
	nProgress = nOldProgress;

	// get source data size
	DWORD dwDataSize = m_pReal->GetDataSize();
	TRACE("m_pReal dwDataSize=%lu\n",dwDataSize);

	if (nLevel < 0) // memory allocation failed or previous processing error
	{
		if ((nLevel == PROCESS_CANCELED))
			CancelProcess(); // set your own cancel flag
		return MAKELONG(nLevel, nProgress);
	}

	// start process
	BeginWaitCursor(); // wait cursor
	if (!StartProcess(pCaller, IDS_STATTXT_PROCESSFMT)) // start data processing
	{
		EndProcess(); // end data processing
		EndWaitCursor();
		return MAKELONG(PROCESS_ERROR, nProgress);
	}

	// create the temporary file
	if (!CreateTempFile(_T("FT"))) // creating error
	{
		EndProcess(); // end data processing
		EndWaitCursor();
		SetDataReady(FALSE);
		return MAKELONG(PROCESS_ERROR, nProgress);
	}

	DWORD dwDataPos = 0; // data position pointer

	CProcessTrackState state;

	// Set the initial tracking estimates
	double samplingRate = pDoc->GetFmtParm()->dwSamplesPerSec;
	double bandwidthInHertz = 250;
	double radius = exp(-bandwidthInHertz/samplingRate*pi); 

	int formants = int(samplingRate/2000);
	state.GetTrackIn().push_back(std::polar(radius, double(0)));
	state.GetTrackOut().push_back(std::polar(radius, double(0)));
	for (int i=0; i < formants; i++)
	{
		double frequency = 2*pi*(i*1000+500)/samplingRate;
		CDBL formantEstimate(std::polar(radius, frequency));
		TRACE("%d radius=%f frequency=%f polar=%f\n",i,radius,frequency,std::polar(radius, frequency));
		state.GetTrackIn().push_back(formantEstimate);
		state.GetTrackOut().push_back(formantEstimate);
	}
	// create the window
	DspWin window = DspWin::FromBandwidth(FormantTrackerOptions.m_dWindowBandwidth,pDoc->GetFmtParm()->dwSamplesPerSec,FormantTrackerOptions.m_nWindowType);
	state.GetWindow().assign(window.WindowDouble(),window.WindowDouble()+window.Length());

	for (int i=0;i<3;i++) 
	{
		TRACE("to[%d]=%f\n",i,state.GetTrackOut()[i]);
	}

	// determine processing interval
	DWORD dwInterval = DWORD(samplingRate / FormantTrackerOptions.m_dUpdateRate);

	//Determine initial data preload
	DWORD dwInitial = ((dwDataSize % dwInterval) + window.Length() + dwInterval)/2;

	// preload the data deque with zeroes
	state.GetData().resize(state.GetWindow().size(), 0);

	AdvanceData(state, dwDataPos, dwInitial);

	// start processing loop
	for (int i=0;i<3;i++) 
	{
		TRACE("to[%d]=%f\n",i,state.GetTrackOut()[i]);
	}

	TRACE("dwDataSize=%d\n",dwDataSize);

 	while (dwDataPos < dwDataSize)
	{
		// set progress bar
		SetProgress(nProgress + (int)(100 * dwDataPos / dwDataSize / (DWORD)nLevel));
		if (IsCanceled()) return Exit(PROCESS_CANCELED); // process canceled

		BOOL bRes = TRUE;
		int pitch = m_pPitch->GetProcessedData((DWORD) (dwDataPos / Grappl_calc_intvl), &bRes) / PRECISION_MULTIPLIER;

		BuildTrack( state, samplingRate, pitch);
		WriteTrack( state, samplingRate, pitch);

		state.GetTrackIn() = state.GetTrackOut();

		AdvanceData(state, dwDataPos + dwInitial, dwInterval);

		dwDataPos += dwInterval;
	}    

	nProgress = nProgress + (int)(100 / nLevel); // calculate the actual progress

	// close the temporary file and read the status
	CloseTempFile(); // close the file
	EndProcess((nProgress >= 95)); // end data processing
	EndWaitCursor();
	SetDataReady();
	return MAKELONG(nLevel, nProgress);
}


void CFormantTracker::AdvanceData( CTrackState &state, DWORD dwDataPos, int nSamples)
{
	DEQUE_CDBL &data = state.GetData();
	DWORD dwSize = m_pReal->GetDataSize();
	double fSizeFactor = dwSize / m_pImag->GetDataSize();
	DWORD dwImagPos = (DWORD)((double)dwDataPos / fSizeFactor); // imaginary data position
	unsigned int nImagSmpSize = sizeof(short) / (unsigned int)fSizeFactor; // imaginary data sample size

	if (nSamples > 0)
	{
		if(unsigned int(nSamples) > data.size())
		{
			AdvanceData( state, dwDataPos + nSamples - data.size(), data.size());
		}
		else
		{
			data.erase(data.begin(), data.begin() + nSamples);

			for(int i=0; i< nSamples; i++)
			{
				double dReal = 0;
				double dImag = 0;

				if (dwDataPos+i < dwSize) 
				{
					dReal = *reinterpret_cast<short *>(m_pReal->GetProcessedObject(dwDataPos + i, sizeof(short)));
					if (nImagSmpSize == 1)
						dImag = *reinterpret_cast<char *>(m_pImag->GetProcessedObject(dwImagPos + i, sizeof(char)));
					else
						dImag = *reinterpret_cast<short *>(m_pImag->GetProcessedObject(dwImagPos + i, sizeof(short)));
				}

				CDBL sample(dReal, dImag);

				data.push_back(sample);
			}
		}
	}
	else
	{
		if(unsigned int(-nSamples) > data.size())
		{
			AdvanceData(state, dwDataPos - nSamples + data.size(), data.size());
		}
		else
		{
			data.erase(data.end() - nSamples, data.end());

			for(int i=0; i< nSamples; i++)
			{
				double dReal = 0;
				double dImag = 0;

				if ((dwDataPos >= DWORD(i)) && (dwDataPos < dwSize))
				{
					dReal = *reinterpret_cast<short *>(m_pReal->GetProcessedObject(dwDataPos-i, sizeof(short), TRUE));
					dImag = *reinterpret_cast<short *>(m_pImag->GetProcessedObject(dwDataPos-i, sizeof(short), TRUE));
				}

				CDBL sample(dReal, dImag);

				data.push_front(sample);
			}
		}
	}
}

void CFormantTracker::WriteTrack( CTrackState &state, double samplingRate, int pitch)
{
	FORMANT_FREQ FormantFreq;
	VECTOR_CDBL &pRoots = state.GetTrackOut();
	BOOL bIsDataValid = pRoots.size() && (pitch > 0);

	FormantFreq.F[0] = float(bIsDataValid ? atan2(pRoots[0].imag(), pRoots[0].real())*samplingRate/2/pi : UNDEFINED_DATA);
	for (unsigned int i=1; i< MAX_NUM_FORMANTS; i++)
	{
		bIsDataValid = (i < pRoots.size()) && (pitch > 0);
		FormantFreq.F[i] = float(bIsDataValid ? atan2(pRoots[i].imag(), pRoots[i].real())*samplingRate/2/pi : UNDEFINED_DATA);
	}

	// write unvoiced formant frame
	Write((HPSTR)&FormantFreq, (UINT)sizeof(FormantFreq));
}

/***************************************************************************/
// CFormantTracker::GetFormant Read formant data
// Reads a slice of processed data from the temporary file into the processed
// data buffer and returns the pointer to the data. The returned pointer 
// points to a slice of formant data. pCaller is a pointer to the
// calling plot and enables this function to get the process index of the
// plot. nIndex is the horizontal index in the formant data.  The function 
// returns NULL on error.
/***************************************************************************/
FORMANT_FREQ* CFormantTracker::GetFormant(DWORD dwIndex)
{
	TRACE("GetFormant %d\n",dwIndex);
	// read the data
	size_t sSize = sizeof(FORMANT_FREQ);
	return (FORMANT_FREQ*)GetProcessedObject(dwIndex, sSize);
}

