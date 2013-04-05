#ifndef WAVEINDEX_H
#define WAVEINDEX_H

class CSaDoc;

class CWaveIndex
{
public:
	static CWaveIndex FromSamples( CSaDoc & doc, DWORD val);
	static CWaveIndex FromBytes(  CSaDoc & doc, DWORD val, bool singleChannel);
	DWORD GetBytes( bool singleChannel) const;
	DWORD GetSamples() const;
	virtual ~CWaveIndex();
    bool operator<(const CWaveIndex & right) const;

private:
	CWaveIndex( DWORD blockAlign, DWORD bytesPerSample, DWORD samples);
	DWORD samples;
	DWORD blockAlign;
	DWORD bytesPerSample;
};

#endif
