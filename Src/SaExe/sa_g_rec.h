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

class CPlotRecording : public CPlotRawData
{   
  DECLARE_DYNCREATE(CPlotRecording)

// Construction/destruction/creation
public:
  CPlotRecording();
  virtual ~CPlotRecording();
  virtual CPlotWnd * NewCopy(void);
  virtual void CopyTo(CPlotRecording * pTarg);


  // Attributes
private:
  DWORD m_dwRecDataFrame;        // number of data points to display in the Recording plot window
  //int   m_nInnerPixLeft;
  //int   m_nInnerPixRight;

  // Operations
public:
  virtual void SetMagnify(double, BOOL bRedraw = FALSE);      // set magnify factor
  // don't want to allow clients to change selection if this is the recording plot
  virtual void  SetHighLightArea(DWORD /*dwStart*/, DWORD /*dwStop*/, BOOL /*bRedraw*/ = TRUE, BOOL /*bSecondSelection*/ = FALSE) {;}  

protected:
  virtual double GetDataPosition(int)  {return 0L;}        // 08/29/2000 - DDO
  virtual DWORD AdjustDataFrame(int iWidth);                     // 08/29/2000 - DDO
  virtual BOOL  SetLegendScale();                                // 08/29/2000 - DDO
  //virtual BOOL  EraseBkgnd(CDC *pDC);                          // 10/24/2000 - DDO
  //        void  SetHighlightInfo();                            // 10/24/2000 - DDO

  virtual DWORD GetAreaPosition();                         // return area position
  virtual DWORD GetAreaLength(CRect * pRwnd = NULL);       // return area length


  // Generated message map functions
protected:
  //{{AFX_MSG(CPlotRecording)
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()

  virtual void OnPaint();
};

#endif //_sa_g_rec_H
