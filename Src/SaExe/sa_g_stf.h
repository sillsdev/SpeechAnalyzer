/////////////////////////////////////////////////////////////////////////////
// sa_g_stf.h:
// Interface of the CPlotStaff class
// Author: Todd Jones
// copyright 2000 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////

#define IDM_CONVERT 106 // was 105
#define IDM_EXPORT  107
#define IDM_IMPORT  108

//###########################################################################
// CPlotStaff plot window

class CPlotStaff : public CPlotWnd
{
    DECLARE_DYNCREATE(CPlotStaff)

// Construction/destruction/creation
public:
    CPlotStaff();
    virtual ~CPlotStaff();
    virtual CPlotWnd * NewCopy(void);
    virtual void GraphHasFocus(BOOL bFocus);

    // Attributes
private:
    RECT LastSize;
    HWND StaffControl;
    CSaView * m_pView;         // DDO - 08/14/00

    char * NoteNum2Name(double dMIDINumber, char * sMusique, size_t len, BOOL bHalfFlatSharp); // DDO - 08/14/00
    double QNotes2Dur(double dNoteQtrNotes, char * sDuration, size_t len);
    double Note2String(double dNoteQtrNotes, double dMIDINumber, CString & sMusique, double dNoteTol, BOOL bHalfFlatSharp);

    // Operations
public:
    virtual void OnDraw(CDC * pDC, CRect rWnd, CRect rClip, CSaView * pView);
    void SetFocusedGraph(CGraphWnd * cgw)
    {
        if (m_pView)
        {
            m_pView->SetFocusedGraph(cgw);
        }
        OnSetFocus(cgw);
    };
    void SetWindowText(LPCTSTR lpsz);
    void SetModifiedFlag(BOOL Modified);
    int GetWindowText(LPTSTR lpsz, int cbMax);
    int GetWindowTextLength(void);
    int GetTempo(void);
    int SaveAsMIDI(char * szFilename);
    int ExportFile();
    int ImportFile();
    int PlaySelection(BOOL bMidi = TRUE, BOOL bWave = FALSE);
    int PausePlay(void);
    int StopPlay(void);
    int LoopPlay(void);
    int ChooseVoice(void);
    int ChooseTempo(void);
    void Convert(void);   // DDO - 08/14/00
    int HideButtons(void);

    virtual BOOL PreTranslateMessage(MSG * pMsg);

    // Generated message map functions
protected:
    //{{AFX_MSG(CPlotStaff)
    afx_msg void OnSetFocus(CWnd *)
    {
        if ((HWND)StaffControl)
        {
            ::SetFocus((HWND)StaffControl);
        }
    }
    afx_msg void OnParentNotify(UINT msg,LPARAM lParam);
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};
