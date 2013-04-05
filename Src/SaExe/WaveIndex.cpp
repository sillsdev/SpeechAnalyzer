#include "Stdafx.h"
#include "WaveIndex.h"
#include "Sa_Doc.h"

CWaveIndex CWaveIndex::FromSamples( CSaDoc & doc, DWORD samples)
{
	return CWaveIndex( doc.GetBlockAlign(), doc.GetSampleSize(), samples);
}

CWaveIndex CWaveIndex::FromBytes( CSaDoc & doc, DWORD bytes, bool singleChannel)
{
	DWORD blockAlign = doc.GetBlockAlign();
	DWORD bytesPerSample = doc.GetSampleSize();
	DWORD divisor = (singleChannel)?bytesPerSample:blockAlign;
	DWORD samples = bytes/divisor;
	return CWaveIndex(blockAlign,bytesPerSample,samples);
}

DWORD CWaveIndex::GetBytes( bool singleChannel) const
{
	return (singleChannel) ? (samples*bytesPerSample) : (samples*blockAlign);
}

DWORD CWaveIndex::GetSamples() const
{
	return samples;
}

CWaveIndex::CWaveIndex( DWORD blockAlign, DWORD bytesPerSample, DWORD samples)
{
	CWaveIndex::blockAlign = blockAlign;
	CWaveIndex::bytesPerSample = bytesPerSample;
	CWaveIndex::samples = samples;
}

CWaveIndex::~CWaveIndex()
{
}

bool CWaveIndex::operator<(const CWaveIndex & right) const
{
	return GetSamples()<right.GetSamples();
}

