/////////////////////////////////////////////////////////////////////////////
// sa_g_rec.h:
// Interface of the CPlotRecording class
// Author: Todd Jones
// copyright 2000 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////
#ifndef _sa_g_rec_H
#define _sa_g_rec_H

//###########################################################################
// CPlotRecording plot window

class CPlotRecording : public CPlotRawData {
    DECLARE_DYNCREATE(CPlotRecording)

// Construction/destruction/creation
public:
    CPlotRecording();
    virtual ~CPlotRecording();
    virtual CPlotWnd * NewCopy(void);
    virtual void CopyTo(CPlotRecording * pTarg);
	// set magnify factor
    virtual void SetMagnify(double, BOOL bRedraw = FALSE);      
	virtual void ClearHighLightArea();
    // don't want to allow clients to change selection if this is the recording plot
    virtual void SetHighLightArea(DWORD dwStart, DWORD dwStop, BOOL bRedraw, BOOL bSecondSelection);

protected:
    virtual double GetDataPosition(int);
    virtual DWORD AdjustDataFrame(int iWidth);
    virtual BOOL SetLegendScale();

	// return area position
    virtual DWORD GetAreaPosition();
	// return area length
    virtual DWORD GetAreaLength(CRect * pRwnd = NULL);

    DECLARE_MESSAGE_MAP()

    virtual void OnPaint();

private:
	// number of data points to display in the Recording plot window
    DWORD m_dwRecDataFrame;        
};

#endif //_sa_g_rec_H
