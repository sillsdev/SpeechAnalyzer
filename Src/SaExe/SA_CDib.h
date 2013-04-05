/////////////////////////////////////////////////////////////////////////////
// SA_CDIB.h
// Interface of the CDib class.
// Author:
// copyright 199?-1999 JAARS Inc. SIL
//
// Revision History
//   1.06.6U4
//        SDM Added Save, CopyToClipboard, Construct, CaptureWindow
//        SDM Changed const functions to be declared const
//   1.06.6U5
//        SDM Added bClient parameter to Capture Window
/////////////////////////////////////////////////////////////////////////////
#ifndef _SA_CDIB_
#define _SA_CDIB_

class CDib : public CObject
{
    DECLARE_SERIAL(CDib)

public:
    CDib();
    CDib(CDC * pDC, int nBt = 0, BOOL bCompr = FALSE);
    ~CDib();

    void Construct(CDC * pDC, int nBt = 0, BOOL bCompr = FALSE);
    // nBt = 0 means use default bits/pixel
    // These methods work great but are disabled because
    // currently they are not needed.

    virtual void Serialize(CArchive & ar);
    void CopyToClipboard(CWnd *) const;
    void CaptureWindow(CWnd *, CRect rectCrop = CRect(0,0,0,0), BOOL bClient = FALSE);
    void Save();

    BOOL Read(CFile * pFile);
    BOOL Write(CFile * pFile);
    CBitmap * MakeBitmap(CDC * pDC, CSize & bmSize); // bmsize returned
    BOOL Display(CDC *, CPoint origin);
    BOOL Stretch(CDC *, CPoint origin, CSize size);
    BOOL Paint(CDC * pDC,
               const CRect & targRect);
    void CreateDIBPalette();
    WORD NumColors(void) const;
    int GetColorBits() const;  // bits per pixel
    DWORD GetLength() const;
    CSize GetSize() const;
    void SetMonoColors(DWORD dwForeground, DWORD dwBackground);
    BOOL GetMonoColors(DWORD & dwForeground, DWORD & dwBackground);
    BOOL HasPalette(void);
    BYTE static MakeGrey(COLORREF & rgb);
    void GoGreyScale(void);

private:
    BOOL AllocateMemory(BOOL bRealloc = FALSE);

    char * m_lpBuf;     // DIB data buffer
    DWORD m_dwLength;   // total buffer length, including file header
    int m_nBits;        // number of color bits per pixel
    //pointers for internal use
    LPBITMAPFILEHEADER m_lpBMFH;
    LPBITMAPINFOHEADER m_lpBMIH;
    LPBITMAPINFO m_lpBMI;
    LPSTR m_lpDibData;
    CPalette * m_pPal;
};


#endif




