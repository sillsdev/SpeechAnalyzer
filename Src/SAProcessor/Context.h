#pragma once
#ifndef _CONTEXT_H
#define _CONTEXT_H

#include "Formants.h"
#include "windowsettings.h"

class CProcessGrappl;
class CProcessSpectrum;
class CMusicParm;
class CUttParm;
class CProcessZCross;
class CProcessFragments;
class CWbProcess;
class CProcessDoc;
class CProcessAdjust;
class CProcessFragments;
class CProcessLoudness;
class CProcessSmoothLoudness;
class CProcessZCross;
class CProcessPitch;
class CProcessCustomPitch;
class CProcessSmoothedPitch;
class CProcessChange;
class CProcessRaw;
class CProcessHilbert;
class CProcessSpectrogram;
class CProcessSpectrogram;
class CProcessWavelet;
class CProcessSpectrum;
class CProcessGrappl;
class CProcessMelogram;
class CProcessFormants;
class CProcessFormantTracker;
class CProcessDurations;
class CProcessSDP;
class CProcessRatio;
class CProcessPOA;
class CProcessGlottis;
class CProcessTonalWeightChart;
class CVowelFormantSet;
class CVowelFormantsVector;
class CVowelFormantSets;
struct SResearchSettings;

typedef char* BPTR;

enum EGender {
    male = 0,
    female = 1,
    child = 2
};

enum WbDialogType {
    Plain,WbEquation,WbEcho,WbReverb,WbLoPass,WbHiPass,WbBandPass
};

enum ProcessorType {
    PROCESSDFLT = -1,
    BACKGNDFRA,
    BACKGNDGRA,
    PROCESSCHA,
    PROCESSCPI,
    PROCESSDUR,
    PROCESSFMT,
    PROCESSFRA,
    PROCESSGRA,
    PROCESSLOU,
    PROCESSMEL,
    PROCESSPIT,
    PROCESSPOA,
    PROCESSRAT,
    PROCESSRAW,
    PROCESSSLO,
    PROCESSSPG,
    PROCESSSPI,
    PROCESSSPU,
    PROCESSTWC,
    PROCESSWBECHO,
    PROCESSWBEQU,
    PROCESSWBLP,
    PROCESSWBREV,
    PROCESSWVL,
    PROCESSZCR,
    SEGMENTING,
    PARSING,
    PROCESSWBGENERATOR,
};

__interface PhoneticSegment {
    // will have problem with string
    LPCTSTR GetContent();
    DWORD GetDurationAt(int index) const;
    DWORD GetDuration(const int nIndex) const;
    int GetDurationSize() const;
};

__interface Model {
    BOOL IsBackgroundProcessing();
    DWORD GetSampleSize() const;
    DWORD GetSamplesPerSec();
    DWORD GetNumChannels() const;
    DWORD GetNumSamples() const;
    BPTR GetWaveData(DWORD dwOffset, BOOL bBlockBegin = FALSE);
    DWORD GetWaveBufferIndex();
    DWORD GetDataSize() const;
    DWORD GetSignalBandWidth();
    EGender GetGender();
    WORD GetBlockAlign(bool singleChannel = false);
    WORD GetBitsPerSample();
    const CMusicParm* GetMusicParm() const;
    const CUttParm* GetUttParm();
    void GetUttParm(CUttParm*, BOOL bOriginal = FALSE);
    bool Is16Bit();
    void * GetUnprocessedDataBlock(DWORD dwByteOffset, size_t sObjectSize, BOOL bReverse);
    BPTR GetAdjustedUnprocessedWaveData(DWORD dwOffset);
    BPTR GetUnprocessedWaveData(DWORD dwOffset, BOOL bBlockBegin);
    DWORD GetUnprocessedBufferIndex(size_t nSize);
    int GetWbProcess();
    LPCTSTR GetProcessFilename();
    DWORD GetSelectedChannel();
    string GetSegmentContent(int index);
    CProcessAdjust* GetAdjust();
    CProcessChange* GetChange();
    CProcessCustomPitch* GetCustomPitch();
    CProcessDoc* GetUnprocessed();
    CProcessDurations* GetDurations();
    CProcessFormantTracker* GetFormantTracker();
    CProcessFormants* GetFormants();
    CProcessFragments* GetFragments() = 0;
    CProcessGlottis* GetGlottalWave();
    CProcessGrappl* GetGrappl();
    CProcessHilbert* GetHilbert();
    CProcessLoudness* GetLoudness();
    CProcessMelogram* GetMelogram();
    CProcessPOA* GetPOA();
    CProcessPitch* GetPitch();
    CProcessRatio* GetRatio();
    CProcessRaw* GetRaw();
    CProcessSDP* GetSDP(int nIndex);
    CProcessSmoothLoudness* GetSmoothLoudness();
    CProcessSmoothedPitch* GetSmoothedPitch();
    CProcessSpectrogram* GetSnapshot();
    CProcessSpectrogram* GetSpectrogram();
    CProcessSpectrum* GetSpectrum();
    CProcessTonalWeightChart* GetTonalWeightChart();
    CProcessWavelet* GetWavelet();
    CProcessZCross* GetZCross();
    void NotifyFragmentDone(void* pCaller);
    int GetProcessorText(ProcessorType processorType);

    // returns GetSegment(PHONETIC)
    PhoneticSegment * GetPhoneticSegment();
};

__interface IProcess {
    long Process(void* pCaller, int nProgress = 0, int nLevel = 1);
    // return processed data pointer to object staring at dwOffset
    void* GetProcessedObject(LPCTSTR szName, int selectedChannel, int numChannels, int sampleSize, DWORD dwIndex, size_t sObjectSize, BOOL bReverse = FALSE);
    // return the size of the data in bytes for a single channel
    DWORD GetProcessedModelWaveDataSize();
    //  return the number of samples for a single channel
    DWORD GetNumSamples() const;
};

__interface MainFrame {
    CProcess* GetProgressOwner();
    void SetProgress(int percent);
    void SetProgressOwner(CProcess*, void*, ProcessorType processorType);
    void ClearProgressOwner(CProcess*);
    void ShowDataStatusBar(bool);
    CWbProcess* GetWbProcess(int nProcess, int nFilter);
};

__interface App {
    CVowelFormantSets & GetVowelSets();
    const CVowelFormantSet& GetDefaultVowelSet();
    const CVowelFormantsVector& GetVowelVector(int nGender);
    SResearchSettings GetResearchSettings();

    // return CPlot3D::GetChartRange(nFormant, nGender);
    SRange Get3DChartRange(int nFormant, int nGender);

    void ErrorMessage(UINT nTextID, LPCTSTR pszText1 = NULL, LPCTSTR pszText2 = NULL);
    // uses IDS_ERROR_GRAPPLSPACE
    void GrapplErrorMessage( LPCTSTR pszText1 = NULL, LPCTSTR pszText2 = NULL);
    int AfxMessageBox(UINT nIDPrompt, UINT nType = MB_OK, UINT nIDHelp = (UINT)-1);
};

__interface CmdTarget {
    // use CCmdTarget
    void BeginWaitCursor();
    void EndWaitCursor();
};

struct Context {
    Context(App& app, Model& model, MainFrame& frame, CmdTarget & target) : app(app), model(model), frame(frame), target(target) {}
    App& app;
    Model& model;
    MainFrame& frame;
    CmdTarget& target;
};

#endif
