// sa_dplot.h : header file
//

class CMainFrame;

/////////////////////////////////////////////////////////////////////////////
// CDisplayPlot window

class CDisplayPlot
{
public:
    CDisplayPlot(CString & szPlot);
    virtual ~CDisplayPlot();

    CMainFrame * m_pMainFrame;
    HMENU m_hNewMenu;
    HACCEL m_hNewAccel;
    int m_nPopup;
    BOOL m_bStatusBar;
    BOOL m_bToolBar;
    BOOL m_bTaskBar;
    int m_nCaptionStyle;
    BOOL m_bScrollZoom;
    CDialog * m_pModal;
};

/////////////////////////////////////////////////////////////////////////////
