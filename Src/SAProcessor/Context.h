#pragma once

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

enum EGender {
    male, female, child
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
};

__interface PhoneticSegment {
    // will have problem with CString
    LPCTSTR GetContent();
    DWORD GetDurationAt(int index) const;
    DWORD GetDuration(const int nIndex) const;
    int GetDurationSize() const;
};

__interface Model {
    BOOL IsBackgroundProcessing();
    DWORD GetSampleSize();
    DWORD GetSamplesPerSec();
    DWORD GetNumChannels() const;
    DWORD GetNumSamples();
    HPSTR GetWaveData(DWORD dwOffset, BOOL bBlockBegin = FALSE);
    DWORD GetWaveBufferIndex();
    uint32 GetDataSize();
    DWORD GetSignalBandWidth();
    EGender GetGender();
    UINT GetBlockAlign(bool singleChannel = false);
    WORD GetBitsPerSample();
    CMusicParm* GetMusicParm();
    CUttParm* GetUttParm();
    void GetUttParm(CUttParm*, BOOL bOriginal = FALSE);
    bool Is16Bit();
    void * GetUnprocessedDataBlock(DWORD dwByteOffset, size_t sObjectSize, BOOL bReverse);
    HPSTR GetAdjustedUnprocessedWaveData(DWORD dwOffset);
    HPSTR GetUnprocessedWaveData(DWORD dwOffset, BOOL bBlockBegin);
    DWORD GetUnprocessedBufferIndex(size_t nSize);
    int GetWbProcess();
    LPCTSTR GetProcessFilename();
    DWORD GetSelectedChannel();
    // should return GetSegment(index)->GetContext();
    string GetSegmentContext(int index);
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

__interface ProgressStatusBar {
    void* GetProcessOwner();
    void SetProgress(int percent);
    void SetProcessOwner(void*, void*, ProcessorType processorType = PROCESSDFLT);
};

__interface MainFrame {
    ProgressStatusBar* GetProgressStatusBar();
    void ShowDataStatusBar(BOOL);
    CWbProcess* GetWbProcess(int nProcess, int nFilter);
};

__interface ObjectIStream {
    bool bAtEnd();
    bool bAtBackslash();
    //bool bAtBeginMarker();
    //bool bAtBeginOrEndMarker();
    //bool bAtBeginMarker(LPCSTR pszMarker, LPCSTR pszName = NULL); // True if at the desired begin marker followed by the requested name
    //bool bAtEndMarker(LPCSTR pszMarker);
    //bool bFail();
    bool bReadBeginMarker(LPCSTR pszMarker, LPSTR psName, size_t size);
    bool bReadBeginMarker(LPCSTR pszMarker);
    //bool bReadEndMarker(LPCSTR pszMarker);
    //void SkipToEndMarker(LPCSTR pszMarker);
    //bool bReadBeginMarkerWithQualifier(LPCSTR pszMarker, LPCSTR pszQualifier);
    //bool bReadEndMarkerWithQualifier(LPCSTR pszMarker, LPCSTR pszQualifier);
    bool bReadString(LPCSTR pszMarker, LPSTR szResult, size_t len);
    bool bReadBool(LPCSTR pszMarker, BOOL& b);
    //bool bReadCOLORREF(LPCSTR pszMarker, COLORREF& rgb);
    bool bReadInteger(LPCSTR pszMarker, int& i);
    bool bReadDouble(LPCSTR pszMarker, double& i);
    bool bReadUInt(LPCSTR pszMarker, UINT& ui);
    //bool bReadNumberOf(LPCSTR pszMarker, unsigned long& num);
    //bool bReadDWord(LPCSTR pszMarker, DWORD& dw);
    bool bEnd(LPCSTR pszMarker);
    //bool bEndWithQualifier(LPCSTR pszMarker, LPCSTR pszQualifier);
    //void ReadMarkedString();
    //void PeekMarkedString(LPCSTR* ppszMarker, LPSTR pszString, size_t len, BOOL bTrimWhiteSpace = TRUE);
    size_t GetBufferSize() const;
};

__interface ObjectIStreamFactory {
    // call constructor on CObjectIStream
    ObjectIStream * factory(LPCSTR name);
};

__interface ObjectOStream {
    void WriteBeginMarker(LPCSTR pszMarker, LPCSTR pszName = "");
    void WriteEndMarker(LPCSTR pszMarker);
    //void WriteBeginMarkerWithQualifier(LPCSTR pszMarker,LPCSTR pszQualifier,LPCSTR pszName = "");
    //void WriteEndMarkerWithQualifier(LPCSTR pszMarker, LPCSTR pszQualifier);
    void WriteString(LPCSTR pszMarker, LPCSTR psz);
    void WriteString(LPCSTR pszMarker, LPCSTR pszQualifier, LPCSTR psz);
    void WriteBool(LPCSTR pszMarker, BOOL b);
    //void WriteCOLORREF(LPCSTR pszMarker, COLORREF rgb);
    void WriteInteger(LPCSTR pszMarker, int i, LPCSTR pszComment = NULL);
    void WriteDouble(LPCSTR pszMarker, double i);
    void WriteUInt(LPCSTR pszMarker, UINT u, LPCSTR pszComment = NULL);
    //void WriteNewline();
};

__interface ObjectOStreamFactory {
    // call constructor on CObjectIStream
    ObjectOStream * factory(LPCSTR name);
};

__interface ResearchSettings {
    int getLpcCepstralSmooth();
    int getLpcCepstralSharp();
    int getSpectrumLpcOrderFsMult();
    int getSpectrumLpcOrderAuxMax();
    int getSpectrumLpcMethod();
    int getSpectrumLpcOrderExtra();
    CWindowSettings getWindow();
    void setWindow(CWindowSettings);
};

__interface App {
    CVowelFormantSets & GetVowelSets();
    const CVowelFormantSet& GetDefaultVowelSet();
    const CVowelFormantsVector& GetVowelVector(int nGender);
    ObjectOStreamFactory * getObjectOStreamFactory();
    ObjectIStreamFactory * getObjectIStreamFactory();
    ResearchSettings& getResearchSettings();

    // return CPlot3D::GetChartRange(nFormant, nGender);
    SRange Get3DChartRange(int nFormant, int nGender);

    void ErrorMessage(UINT nTextID, LPCTSTR pszText1 = NULL, LPCTSTR pszText2 = NULL);
    // uses IDS_ERROR_GRAPPLSPACE
    void GrapplErrorMessage( LPCTSTR pszText1 = NULL, LPCTSTR pszText2 = NULL);
    int AfxMessageBox(UINT nIDPrompt, UINT nType = MB_OK, UINT nIDHelp = (UINT)-1);
};

// interface for CCmdTarget
__interface CmdTarget {
    void BeginWaitCursor();
    void EndWaitCursor();
};

__interface View {
    DWORD GetDataFrame();
    DWORD GetStartCursorPosition();
    DWORD GetStopCursorPosition();
    Model* GetDocument();
    MainFrame* GetMainWnd();
    App* GetApp();
    CmdTarget * GetTarget();
    double GetDataPosition(int nWndWidth);
};

__interface Context {
    App * GetApp();
    View * GetView();
};
