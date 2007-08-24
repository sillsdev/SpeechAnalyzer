#ifndef _ISA_DOC__
#define _ISA_DOC__



struct FmtParm;
struct SaParm;
class MusicParm;
class UttParm;
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
class CHilbert;
class CProcessSpectrum;
class CProcessSpectrogram;
class CProcessWavelet;
class CProcessFormants;
class CFormantTracker;
class CProcessDurations;
class CProcessFragments;
class CProcessSDP;
class CProcessRatio;
class CProcessPOA;
class CProcessGlottis;
class CProcessTonalWeightChart;

class ISaDoc
{
  friend CProcessDoc;

public:
  virtual int           GetWbProcess() = 0;
  virtual FmtParm*      GetFmtParm() = 0;
//  virtual void          GetFmtParm(FmtParm*) = 0;
  virtual int           GetGender() = 0;
  virtual const MusicParm*    GetMusicParm() const = 0;
  virtual SaParm*       GetSaParm() = 0;
  virtual void          GetSaParm(SaParm*) = 0;
  virtual const UttParm*     GetUttParm() = 0;
  virtual void          GetUttParm(UttParm*, BOOL bOriginal=FALSE) = 0;
  virtual DWORD         GetDataSize() = 0;
  virtual DWORD         GetUnprocessedDataSize() = 0;
  virtual HPSTR         GetWaveData(DWORD dwOffset, BOOL bBlockBegin = FALSE) = 0;
  virtual int           GetWaveData(DWORD dwOffset, BOOL*) = 0;
  virtual HPSTR         GetUnprocessedWaveData(DWORD dwOffset, BOOL bBlockBegin = FALSE, BOOL bAdjusted = TRUE) = 0;
  virtual DWORD         GetWaveBufferIndex() = 0;
  virtual CSegment* GetSegment(int nIndex) = 0;
  virtual void NotifyFragmentDone(void *pCaller) = 0;

  virtual BOOL          IsBackgroundProcessing() = 0;

  virtual CProcessDoc* GetUnprocessed() = 0;
  virtual CProcessAdjust* GetAdjust() = 0;
  virtual CProcessFragments* GetFragments()= 0;
  virtual CProcessLoudness* GetLoudness() = 0;
  virtual CProcessSmoothLoudness* GetSmoothLoudness() = 0;
  virtual CProcessZCross* GetZCross() = 0;
  virtual CProcessPitch* GetPitch() = 0;
  virtual CProcessCustomPitch* GetCustomPitch() = 0;
  virtual CProcessSmoothedPitch* GetSmoothedPitch() = 0;
  virtual CProcessChange* GetChange() = 0;
  virtual CProcessRaw* GetRaw() = 0;
  virtual CHilbert* GetHilbert() = 0;
  virtual CProcessSpectrogram* GetSpectrogram(bool bRealTime) = 0;
	virtual CProcessWavelet* GetWavelet() = 0;
  virtual CProcessSpectrum* GetSpectrum() = 0;
  virtual CProcessGrappl* GetGrappl() = 0;
  virtual CProcessMelogram* GetMelogram() = 0;
  virtual CProcessFormants* GetFormants() = 0;
  virtual CFormantTracker* GetFormantTracker() = 0;
  virtual CProcessDurations* GetDurations() = 0;
  virtual CProcessSDP* GetSDP(int nIndex) = 0;
  virtual CProcessRatio* GetRatio() = 0;
  virtual CProcessPOA* GetPOA() = 0;
  virtual CProcessGlottis* GetGlottalWave() = 0;
  virtual CProcessTonalWeightChart* GetTonalWeightChart() = 0;

private:
  virtual const CSaString& GetRawDataWrk(int nIndex) const = 0;
};

#endif
