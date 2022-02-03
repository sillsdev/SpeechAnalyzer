#pragma once

class CProcessGrappl;
class CProcessSpectrum;
class CMusicParm;
class CUttParm;
class CProcessZCross;
class CProcessFragments;

enum EGender {
    male, female, child
};

__interface Model {
    DWORD GetSampleSize();
    DWORD GetSamplesPerSec();
    DWORD GetNumSamples();
    HPSTR GetWaveData(DWORD dwOffset, BOOL bBlockBegin = FALSE);
    DWORD GetWaveBufferIndex();
    uint32 GetDataSize();
    DWORD GetSignalBandWidth();
    CProcessGrappl* GetGrappl();
    CProcessSpectrum* GetSpectrum();
    EGender GetGender();
    UINT GetBlockAlign(bool singleChannel = false);
    WORD GetBitsPerSample();
    CMusicParm* GetMusicParm();
    CUttParm* GetUttParm();
    CProcessZCross* GetZCross();
    CProcessFragments* GetFragments();
    bool Is16Bit();
};

__interface ProgressStatusBar {
    void* GetProcessOwner();
    void SetProgress(int percent);
    void SetProcessOwner(void*, void*);
};

__interface MainFrame {
    ProgressStatusBar* GetProgressStatusBar();
    void ShowDataStatusBar(BOOL);
};

__interface App {
    void ErrorMessage(UINT nTextID, LPCTSTR pszText1 = NULL, LPCTSTR pszText2 = NULL);
    int AfxMessageBox(UINT nIDPrompt, UINT nType = MB_OK, UINT nIDHelp = (UINT)-1);
};

// interface for CCmdTarget
__interface CmdTarget {
    void BeginWaitCursor();
    void EndWaitCursor();
};

__interface View {
    DWORD GetStartCursorPosition();
    DWORD GetStopCursorPosition();
    Model* GetDocument();
    MainFrame* GetMainWnd();
    App* GetApp();
    CmdTarget& GetTarget();
};

__interface Context {
    App & GetApp();
    View* GetView();
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
    //bool bReadString(LPCSTR pszMarker, LPSTR szResult, size_t len);
    bool bReadBool(LPCSTR pszMarker, BOOL& b);
    //bool bReadCOLORREF(LPCSTR pszMarker, COLORREF& rgb);
    bool bReadInteger(LPCSTR pszMarker, int& i);
    //bool bReadDouble(LPCSTR pszMarker, double& i);
    bool bReadUInt(LPCSTR pszMarker, UINT& ui);
    //bool bReadNumberOf(LPCSTR pszMarker, unsigned long& num);
    //bool bReadDWord(LPCSTR pszMarker, DWORD& dw);
    bool bEnd(LPCSTR pszMarker);
    //bool bEndWithQualifier(LPCSTR pszMarker, LPCSTR pszQualifier);
    //void ReadMarkedString();
    //void PeekMarkedString(LPCSTR* ppszMarker, LPSTR pszString, size_t len, BOOL bTrimWhiteSpace = TRUE);
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
    //void WriteDouble(LPCSTR pszMarker, double i);
    void WriteUInt(LPCSTR pszMarker, UINT u, LPCSTR pszComment = NULL);
    //void WriteNewline();
};


