// Butterworth.h: interface for the CButterworth class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BUTTERWORTH_H__DB9351E2_3282_11D5_9FE4_00E098784E13__INCLUDED_)
#define AFX_BUTTERWORTH_H__DB9351E2_3282_11D5_9FE4_00E098784E13__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Sa_proc.h"
#include "dsp\ZTransform.h"

class CIIRFilter : public CDataProcess 
{
public:
	CIIRFilter(BOOL bDstWBench = TRUE);
	virtual ~CIIRFilter();
	
	void SetSourceProcess(CDataProcess * pSourceProcess, BOOL bWBenchProcess = TRUE);
	void SetOutputType(BOOL bWBenchProcess = TRUE);
	virtual long Process(void* pCaller, ISaDoc*, int nProgress = 0, int nLevel = 1);
	void SetFilterFilterSilenceSamples(int forwardSamples) {m_nFilterFilterSilence = forwardSamples > 0 ? forwardSamples : 1;}
	int FilterFilterSilenceSamples() { return m_nFilterFilterSilence; }

	CZTransform GetForward() { return m_zForwardTransform;};

protected:
	CZTransform m_zForwardTransform;
	CZTransform m_zReverseTransform;

private:
	enum
	{
		DEFAULT_FILTER_FILTER_SILENCE_SAMPLES = 4096
	};

	BOOL m_bReverse;
	void SetFilterFilter(BOOL bSet) { m_bFilterFilter = bSet;}
	BOOL m_bFilterFilter;
	int m_nFilterFilterSilence;
	static int round(double value) { return (value >= 0.) ? int(value + 0.5) : int(value - 0.5); }
	BOOL m_bSrcWBenchProcess;
	BOOL m_bDstWBenchProcess;
	CDataProcess * m_pSourceProcess;
	int ReadSourceData(DWORD dwDataPos, int wSmpSize, ISaDoc *pDoc);
	BOOL WriteWaveDataBlock(DWORD dwPosition, HPSTR lpData, DWORD dwDataLength); // write a block into the temporary file
};

class CButterworth : public CIIRFilter
{
public:
	CButterworth(BOOL bWorkBenchOutput = TRUE);
	virtual ~CButterworth();

	void LowPass(int nOrder, double dFrequency, double dScale=1.);
	void HighPass(int nOrder, double dFrequency, double dScale=1.);
	void BandPass(int nOrder, double dFrequency, double dBandwidth, double dScale=1.);

	void SetFilterFilter(BOOL bSet) { m_bFilterFilter = bSet; SetReverse(bSet);}

	virtual long Process(void* pCaller, ISaDoc*, int nProgress = 0, int nLevel = 1);
	void ConfigureProcess(double dSampling);

	double ForwardTick( double data) { return m_zForwardTransform.Tick(data);};

private:
	enum FilterType
	{
		kftUndefined,
		kftHighPass,
		kftLowPass,
		kftBandPass
	};
	BOOL m_bFilterFilter;
	BOOL m_bReverse;
	int m_nOrder;
	FilterType m_ftFilterType;
	double m_dSampling;
	double m_dFrequency;
	double m_dBandwidth;
	double m_dScale;

	void CascadeLowPass(CZTransform &zTransform, int nOrder, double dFrequency, double dSampling, double &tau);
	void CascadeHighPass(CZTransform &zTransform, int nOrder, double dFrequency, double dSampling, double &tau);
	void CascadeScale(CZTransform &zTransform, double dScale);
	void ClearFilter();

	void SetReverse(BOOL bSet) { m_bReverse = bSet;}
	double FilterFilterNorm(int nOrder) const;
	static int round(double value) { return (value >= 0.) ? int(value + 0.5) : int(value - 0.5); }
};

class CHilbert : public CIIRFilter
{
public:
	CHilbert( CDataProcess * pSourceProcess = NULL, BOOL bWBenchProcess = FALSE);

private:
	static const double Pole1000x96dB[];

	CZTransform AllPass(double pole);
	CZTransform DelayHalf();
};

#endif // !defined(AFX_BUTTERWORTH_H__DB9351E2_3282_11D5_9FE4_00E098784E13__INCLUDED_)
