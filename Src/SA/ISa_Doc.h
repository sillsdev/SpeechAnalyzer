#ifndef _ISA_DOC__
#define _ISA_DOC__

class CFmtParm;
class CMusicParm;
class CUttParm;
class CSaString;
class CSegment;

class CProcessDoc;
class CProcessAdjust;
class CProcessLoudness;
class CProcessSmoothLoudness;
class CProcessZCross;
class CProcessPitch;
class CProcessCustomPitch;
class CProcessSmoothedPitch;
class CProcessGrappl;
class CProcessMelogram;
class CProcessChange;
class CProcessRaw;
class CProcessHilbert;
class CProcessSpectrum;
class CProcessSpectrogram;
class CProcessWavelet;
class CProcessFormants;
class CProcessFormantTracker;
class CProcessDurations;
class CProcessFragments;
class CProcessSDP;
class CProcessRatio;
class CProcessPOA;
class CProcessGlottis;
class CProcessTonalWeightChart;
class CSaParam;
enum EGender;

__interface ISaDoc {

public:
    EGender GetGender();
    int GetWbProcess();
    const CMusicParm * GetMusicParm() const;
    const CUttParm * GetUttParm();
    void GetUttParm(CUttParm *, BOOL bOriginal=FALSE);
    void GetFmtParm(CFmtParm & format, bool processed);

    DWORD GetDataSize() const;
    DWORD GetRawDataSize() const;
    HPSTR GetWaveData(DWORD dwOffset, BOOL bBlockBegin = FALSE);
    int GetWaveData(DWORD dwOffset, BOOL *);
    void * GetUnprocessedDataBlock(DWORD dwByteOffset, size_t sObjectSize, BOOL bReverse);
    DWORD GetUnprocessedBufferIndex(size_t nSize);

    HPSTR GetAdjustedUnprocessedWaveData(DWORD dwOffset);
    HPSTR GetUnprocessedWaveData(DWORD dwOffset, BOOL bBlockBegin);
    DWORD GetUnprocessedWaveDataBufferSize();
    DWORD GetWaveBufferIndex();
    DWORD GetWaveDataBufferSize();

    CSegment * GetSegment(int nIndex);
    void NotifyFragmentDone(void * pCaller);
    BOOL IsBackgroundProcessing();
    CProcessDoc * GetUnprocessed();
    CProcessAdjust * GetAdjust();
    CProcessFragments * GetFragments()= 0;
    CProcessLoudness * GetLoudness();
    CProcessSmoothLoudness * GetSmoothLoudness();
    CProcessZCross * GetZCross();
    CProcessPitch * GetPitch();
    CProcessCustomPitch * GetCustomPitch();
    CProcessSmoothedPitch * GetSmoothedPitch();
    CProcessChange * GetChange();
    CProcessRaw * GetRaw();
    CProcessHilbert * GetHilbert();
    CProcessSpectrogram * GetSpectrogram();
    CProcessSpectrogram* GetSnapshot();
    CProcessWavelet * GetWavelet();
    CProcessSpectrum * GetSpectrum();
    CProcessGrappl * GetGrappl();
    CProcessMelogram * GetMelogram();
    CProcessFormants * GetFormants();
    CProcessFormantTracker * GetFormantTracker();
    CProcessDurations * GetDurations();
    CProcessSDP * GetSDP(int nIndex);
    CProcessRatio * GetRatio();
    CProcessPOA * GetPOA();
    CProcessGlottis * GetGlottalWave();
    CProcessTonalWeightChart * GetTonalWeightChart();
    LPCTSTR GetProcessFilename();
    // return the number of samples for a single channel
    DWORD GetSelectedChannel();
    double GetTimeFromBytes(DWORD dwSize);
    DWORD GetBytesFromTime(double fSize);
    // this is bytes per sample
    DWORD GetSampleSize() const;
    DWORD GetSamplesPerSec();
    WORD GetBitsPerSample();
    WORD GetBlockAlign(bool singleChannel=false);
    bool Is16Bit();
    bool IsPCM();
    DWORD GetAvgBytesPerSec();
    DWORD GetNumChannels() const;
    DWORD GetNumSamples() const;
    DWORD GetSignalBandWidth();
};

#endif
