/////////////////////////////////////////////////////////////////////////////
// sa_g_mel.h:
// Interface of the CPlotMelogram class
// Author: Todd Jones
// copyright 2000 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////

//###########################################################################
// CPlotMelogram plot window

class CPlotMelogram : public CPlotWnd
{
    DECLARE_DYNCREATE(CPlotMelogram)

// Construction/destruction/creation
public:
    CPlotMelogram();
    virtual ~CPlotMelogram();
    virtual CPlotWnd * NewCopy(void);

    // Attributes
private:
    BOOL m_bTWC; // TWC is present

    // Operations
public:
    virtual CGrid GetGrid() const;
    virtual void OnDraw(CDC * pDC, CRect rWnd, CRect rClip, CSaView * pView);
    void SetTWC(BOOL bValue)
    {
        m_bTWC = bValue;
    }
    BOOL GetTWC() const
    {
        return m_bTWC;
    }
    virtual int GetPenThickness() const;

    static bool GetScaleValues(CSaDoc * pDoc, double * dMaxSemitone,double * dMinSemitone);

    // Generated message map functions
protected:
    //{{AFX_MSG(CPlotMelogram)
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};
