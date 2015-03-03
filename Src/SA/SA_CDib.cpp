/////////////////////////////////////////////////////////////////////////////
// SA_CDIB.cpp:
// Implementation of the CDib class.
// Author:
// copyright -1999 JAARS Inc. SIL
//
// Revision History
//   1.06.6U4
//        SDM Added Save, CopyToClipboard, Construct, CaptureWindow
//   1.06.6U5
//        SDM Added bClient parameter to Capture Window
//   1.5Test10.8
//        SDM Added ErrorMessages for printing failures
/////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "sa_cdib.h"
#include "sa.h"
#include <windowsx.h>  // for GlobalAllocPtr
#include <process.h>
#include "FileUtils.h"

IMPLEMENT_SERIAL(CDib, CObject, 0)

///////////////////////////////////////////////////////////////////
CDib::CDib() {
    m_dwLength = 0L;
    m_nBits    = 0;
    m_lpBuf    = NULL;
    m_pPal = NULL;
}

// SDM 1.06.6U4 split function
///////////////////////////////////////////////////////////////////
CDib::CDib(CDC * pDC, int nBt, BOOL bCompr)
// pDC is memory DC ptr
// nBt is color bits per pixel (default = 0)
// bCompr is compression (default = TRUE)
{
    m_dwLength = 0L;
    m_nBits    = 0;
    m_lpBuf    = NULL;
    m_pPal = NULL;
    Construct(pDC, nBt, bCompr);
}

// SDM 1.06.6U4 split function
void CDib::Construct(CDC * pDC, int nBt, BOOL bCompr)
// pDC is memory DC ptr
// nBt is color bits per pixel (default = 0)
// bCompr is compression (default = TRUE)
{
    // constructs a DIB from the contents of a bitmap
    BITMAP bm;
    int    nPaletteSize;

    if (m_lpBuf) {
        GlobalFreePtr(m_lpBuf);  // free the DIB memory
        m_lpBuf = NULL;
    }
    delete m_pPal;
    m_pPal = NULL;

    CBitmap * pEmptyBitmap = new CBitmap;
    pEmptyBitmap->CreateCompatibleBitmap(pDC, 0, 0);
    CBitmap * pBitmap = (CBitmap *)(pDC->SelectObject(pEmptyBitmap));
    pBitmap->GetObject(sizeof(bm), &bm);
    if ((nBt == 1) || (nBt == 4) || (nBt == 8) || (nBt == 24)) {
        m_nBits = nBt;
    } else {
        // nBt = 0
        m_nBits = bm.bmPlanes * bm.bmBitsPixel; // color bits per pixel
    }

    if (m_nBits > 8) {
        m_nBits = 24;
    }

    if (m_nBits == 1) {
        nPaletteSize = 2;
    } else {
        if (m_nBits == 4) {
            nPaletteSize = 16;
        } else {
            if (m_nBits == 8) {
                nPaletteSize = 256;
            } else {
                nPaletteSize = 0; // no palette for 24-bit display
            }
        }
    }
    // fills out row to 4-byte boundary
    DWORD dwBytes = ((DWORD) bm.bmWidth * m_nBits) / 32;
    if (((DWORD) bm.bmWidth * m_nBits) % 32) {
        dwBytes ++;
    }
    dwBytes *= 4;

    m_dwLength = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) +
                 sizeof(RGBQUAD) * nPaletteSize;
    if (!AllocateMemory()) {
        return;
    }

    m_lpBMIH->biSize = sizeof(BITMAPINFOHEADER);
    m_lpBMIH->biWidth = bm.bmWidth;
    m_lpBMIH->biHeight = bm.bmHeight;
    m_lpBMIH->biPlanes = 1;
    m_lpBMIH->biBitCount = WORD(m_nBits); // 1, 4, 8, or 24
    if (bCompr && (m_nBits == 4)) {
        m_lpBMIH->biCompression = BI_RLE4;
    } else {
        if (bCompr && (m_nBits == 8)) {
            m_lpBMIH->biCompression = BI_RLE8;
        } else {
            m_lpBMIH->biCompression = BI_RGB;
        }
    }
    m_lpBMIH->biSizeImage = 0;
    m_lpBMIH->biXPelsPerMeter = 0;
    m_lpBMIH->biYPelsPerMeter = 0;
    m_lpBMIH->biClrUsed = 0;
    m_lpBMIH->biClrImportant = 0;

    // calls GetDIBits with null data pointer to get size of DIB
    ::GetDIBits(pDC->GetSafeHdc(), (HBITMAP) pBitmap->GetSafeHandle(),
                0, (WORD) bm.bmHeight, NULL, m_lpBMI, DIB_RGB_COLORS);

    if (m_lpBMIH->biSizeImage == 0) {
        m_dwLength += dwBytes * bm.bmHeight;
        m_lpBMIH->biCompression = BI_RGB;
        // escape route for device drivers that don't do compression
        TRACE(_T("Can't do compression\n"));
    } else {
        m_dwLength += m_lpBMIH->biSizeImage;
    }
    if (!AllocateMemory(TRUE)) {
        return;
    }
    m_lpDibData = (LPSTR) m_lpBMIH + sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * nPaletteSize;
    m_lpBMFH->bfType = 0x4d42; // 'BM'
    m_lpBMFH->bfSize = m_dwLength;
    m_lpBMFH->bfReserved1 = 0;
    m_lpBMFH->bfReserved2 = 0;
    m_lpBMFH->bfOffBits = (char *) m_lpDibData - m_lpBuf;

    // second GetDIBits call to make DIB
    if (!::GetDIBits(pDC->GetSafeHdc(), (HBITMAP)
                     pBitmap->GetSafeHandle(), 0, (WORD) bm.bmHeight, m_lpDibData,
                     m_lpBMI, DIB_RGB_COLORS)) {
        m_dwLength = 0L;
    }

    PALETTEENTRY * pe = NULL;
    UINT entries = 0;

    if (GetDeviceCaps(pDC->m_hDC, RASTERCAPS) & RC_PALETTE) {
        entries = GetDeviceCaps(pDC->m_hDC,SIZEPALETTE);
        if (entries) {
            pe = new PALETTEENTRY[entries];
            GetSystemPaletteEntries(pDC->m_hDC, 0, entries, pe);
        }
    }

    if (pe && entries && nPaletteSize) {
        int end = min((int)entries,nPaletteSize);
        for (int i = 0; i < end; i++) {
            m_lpBMI->bmiColors[i].rgbRed = pe[i].peRed;
            m_lpBMI->bmiColors[i].rgbGreen = pe[i].peGreen;
            m_lpBMI->bmiColors[i].rgbBlue = pe[i].peBlue;
            m_lpBMI->bmiColors[i].rgbReserved = 0;
        }
    }

    if (pe) {
        delete [] pe;
    }

    delete pDC->SelectObject(pBitmap); // delete pEmptyBitmap
}



///////////////////////////////////////////////////////////////////
CDib::~CDib() {
    if (m_lpBuf) {
        GlobalFreePtr(m_lpBuf);  // free the DIB memory
    }
    delete m_pPal;
}

///////////////////////////////////////////////////////////////////
void CDib::Serialize(CArchive & ar) {
    ar.Flush();
    if (ar.IsStoring()) {
        Write(ar.GetFile());
    } else {
        Read(ar.GetFile());
    }
}

// SDM 1.06.6U5 capture client or entire window area
// SDM 1.06.6U4 moved function from mainframe
///////////////////////////////////////////////////////////////////
void CDib::CaptureWindow(CWnd * pCaptureThis, CRect rectCrop, BOOL bClient) {
    if (pCaptureThis) {
        pCaptureThis->UpdateWindow();
    }
    CDC * pScreen = NULL;
    if (bClient) {
        pScreen = new CClientDC(pCaptureThis);
    } else {
        pScreen = new CWindowDC(pCaptureThis);
    }
    CRect memRect;

    pScreen->GetClipBox(&memRect);
    memRect -= rectCrop;

    // create a temporary DC for the reading the screen
    CDC memDC;
    if (!memDC.CreateCompatibleDC(pScreen)) {
        ASSERT(0);
    } else {
        // create a bitmap to read the screen into and select it
        // into the temporary DC
        CBitmap bitmapForPrint;
        if (!bitmapForPrint.CreateCompatibleBitmap(pScreen, memRect.Width(), memRect.Height())) {
            ASSERT(0);
        } else {
            CBitmap * oldBitmap = (CBitmap *)memDC.SelectObject(&bitmapForPrint);
            if (!oldBitmap) {
                ASSERT(0);
            } else {
                // BitBlt the screen data into the bitmap
                if (!memDC.BitBlt(0,0, memRect.Width(), memRect.Height(), pScreen, memRect.left, memRect.top, SRCCOPY)) {
                    ASSERT(0);
                } else {
                    // create a device independent bitmap from the regular bitmap
                    Construct(&memDC, 0, FALSE);
                }
                memDC.SelectObject((HBITMAP)oldBitmap->GetSafeHandle());
            }
        }
    }
    delete pScreen;
}


// SDM 1.06.6U4
///////////////////////////////////////////////////////////////////
void CDib::CopyToClipboard(CWnd * pWnd) const {
    if ((!pWnd) || (m_lpDibData == NULL)) {
        return;
    }

    size_t szColors=0;
    if (m_lpBMIH->biClrUsed) {
        if (m_lpBMIH->biClrUsed%2) {
            m_lpBMIH->biClrUsed++;    // must use even colors to keep data on DWORD boundary
        }
        szColors = (size_t) m_lpBMIH->biClrUsed * sizeof(RGBQUAD);
    } else {
        szColors = NumColors() * sizeof(RGBQUAD);
    }

    HGLOBAL hDIB = GlobalAlloc(GMEM_MOVEABLE, m_lpBMIH->biSize+m_lpBMIH->biSizeImage+szColors);
    if (hDIB == NULL) {
        if (hDIB) {
            GlobalFree(hDIB);
        }
        return;
    }

    LPSTR lpDIB = (char *)GlobalLock(hDIB);
    ASSERT(lpDIB);
    if (lpDIB) {
        LPBITMAPINFO lpBitmapInfo = (LPBITMAPINFO) lpDIB;

        _fmemcpy(lpDIB, m_lpBMI, (size_t)m_lpBMIH->biSize);
        _fmemcpy(lpBitmapInfo->bmiColors, m_lpBMI->bmiColors, szColors);
        char * dest = lpDIB+m_lpBMIH->biSize+szColors;
        char * src = m_lpDibData;
        DWORD size = m_lpBMIH->biSizeImage;
        while (size > 0x4000) {
            _fmemcpy(dest, src, (size_t)0x4000);
            src+= 0x4000;
            dest+= 0x4000;
            size -= 0x4000;
        }
        _fmemcpy(dest, src, (size_t)size);
        GlobalUnlock(hDIB);
    } else {
        if (hDIB) {
            GlobalFree(hDIB);
        }
        return;
    }

    if (pWnd->OpenClipboard()) {
        EmptyClipboard();
        SetClipboardData(CF_DIB, hDIB);
        CloseClipboard();
    }
}

// SDM 1.06.6U4 moved function from mainframe
///////////////////////////////////////////////////////////////////
void CDib::Save(void) {
    if (this) {
        static TCHAR szFilter[] = _T("Pictures (*.png) |*.png| Pictures (*.bmp) |*.bmp||");
        CFileDialog dlg(FALSE, NULL, _T("SA1"), OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, szFilter);

        CSaString szPathnameExtended;
        int nResult;
        while ((nResult = dlg.DoModal()) == IDOK) {
            szPathnameExtended = dlg.GetPathName();
            if (dlg.GetFileExt().IsEmpty()) {
                CSaString szExtension;

                if (dlg.m_ofn.nFilterIndex == 2) {
                    szExtension = ".bmp";
                } else {
                    szExtension = ".png";
                }

                szPathnameExtended += szExtension;

                CFileStatus status;
                if (CFile::GetStatus(szPathnameExtended, status)) {
                    CSaString szPrompt = szPathnameExtended + _T(" already exists.\nDo you want to replace it?");
                    if (AfxMessageBox(szPrompt, MB_YESNO) == IDNO) {
                        // User does not want to overwrite
                        // dlg.m_ofn.lpstrInitialDir = dlg.GetFolderPath();
                        continue;
                    }
                }
            }
            break;
        }

        if (nResult == IDOK) {
            CFile file;
            CSaString szBmpFilename;
            CSaString szPngFilename;

            if (dlg.m_ofn.nFilterIndex == 1) {
                szPngFilename = szPathnameExtended;
                FileUtils::GetTempFileName(_T("TMP"), szBmpFilename.GetBuffer(_MAX_PATH), MAX_PATH);
                szBmpFilename.ReleaseBuffer();
                // delete temporary file automatically created by GetTempFileName
                FileUtils::RemoveFile(szBmpFilename);
                // This is likely but not guaranteed to be unique, but bmp2png insist that bmp's have ".bmp" extension
                szBmpFilename += L".bmp";
            } else {
                szBmpFilename =  szPathnameExtended;
            }

            if (!file.Open(szBmpFilename, CFile::modeCreate | CFile::modeWrite)) {
                AfxMessageBox(IDS_ERROR_SCREEN_CAPTURE_OPEN);
            } else {
                file.SeekToBegin();
                CArchive ar(&file, CArchive::store);
                Serialize(ar);
                ar.Close();
                file.Close();
                if (!szPngFilename.IsEmpty()) {
                    try {
                        CSaString szB2PPath = AfxGetApp()->m_pszHelpFilePath;
                        szB2PPath = szB2PPath.Left(szB2PPath.ReverseFind('\\')) + _T("\\bmp2png.exe");
                        CSaString szQuotedBmp = _T("\"") + szBmpFilename + _T("\"");
                        CSaString szQuotedPng = _T("\"") + szPngFilename + _T("\"");
                        int nResult = _wspawnlp(_P_WAIT, szB2PPath, _T("bmp2png.exe"), /*_T("-L"),*/ _T("-E"), _T("-Q"), _T("-O"), (LPCTSTR)szQuotedPng, (LPCTSTR)szQuotedBmp, NULL);
                        if (nResult != 0) {
                            AfxMessageBox(IDS_ERROR_SCREEN_CAPTURE_SAVE);
                            TRACE(_T("bmp2png result = %d\n"), nResult);
                            FileUtils::RemoveFile(szBmpFilename); // delete temporary bmp
                        }
                    } catch (...) {
                        TRACE(_T("Exception\n"));
                    }
                }
            }
        }
    } else {
        ASSERT(0);
    }
}

///////////////////////////////////////////////////////////////////
BOOL CDib::Read(CFile * pFile) {
    // file assumed to be open
    ASSERT(m_dwLength == 0L); // DIB must be empty
    m_dwLength = pFile->GetLength();
    if (!AllocateMemory()) {
        return FALSE;
    }
    DWORD dwCount = pFile->Read(m_lpBuf, m_dwLength);
    if (dwCount != m_dwLength) {
        AfxMessageBox(_T("Read error"));
        return FALSE;
    }
    if (m_lpBMFH->bfType != 0x4d42) {
        AfxMessageBox(_T("Invalid bitmap file"));
        return FALSE;
    }
    ASSERT((m_lpBMIH->biBitCount == 1) || (m_lpBMIH->biBitCount == 4) ||
           (m_lpBMIH->biBitCount == 8) || (m_lpBMIH->biBitCount == 24));
    m_lpDibData = (LPSTR) m_lpBMFH + m_lpBMFH->bfOffBits;
    m_nBits = m_lpBMIH->biBitCount;
    return TRUE;
}

///////////////////////////////////////////////////////////////////
BOOL CDib::Write(CFile * pFile) {
    try {
        pFile->Write(m_lpBuf, m_dwLength);
    } catch (const CException & e) {
        AfxMessageBox(_T("Write error--possible disk full condition"));
        return FALSE;
    }

    return TRUE;
}

///////////////////////////////////////////////////////////////////
CBitmap * CDib::MakeBitmap(CDC * pDC, CSize & bmSize) {
    // replaces the DC's existing bitmap with a new one from the DIB
    // returns the old one
    BITMAP bm;
    DWORD  dwFore, dwBack;
    // checks to see whether DIB buffer is properly loaded
    if (m_dwLength == 0L) {
        bmSize.cx = bmSize.cy = 0;
        return NULL;
    }

    // this code conditions the DC for mono or color
    int nPlanes = pDC->GetDeviceCaps(PLANES);
    int nBitsPixel = pDC->GetDeviceCaps(BITSPIXEL);
    CBitmap * pConfigBitmap = new CBitmap;
    char bits[100];
    if (m_lpBMIH->biBitCount == 1) {
        pConfigBitmap->CreateBitmap(1, 1, 1, 1, bits);
    } else {
        pConfigBitmap->CreateBitmap(1, 1, nPlanes, nBitsPixel, bits);
    }
    CBitmap * pOriginalBitmap =
        (CBitmap *) pDC->SelectObject(pConfigBitmap);

    // CreateDIBitmap "switches bits" for mono bitmaps, depending on colors,
    //  so we'll fool it
    if (GetMonoColors(dwFore, dwBack)) {
        SetMonoColors(0L, 0xFFFFFFL);
    }

    HBITMAP hBitmap = ::CreateDIBitmap(pDC->GetSafeHdc(), m_lpBMIH,
                                       CBM_INIT, (CONST BYTE *)(m_lpBuf + m_lpBMFH->bfOffBits),
                                       m_lpBMI, DIB_RGB_COLORS);
    if (hBitmap == NULL) {
        TRACE(_T("null bitmap\n"));
        delete pDC->SelectObject(pOriginalBitmap); // delete config bitmap
        return NULL; // untested error logic
    }

    SetMonoColors(dwFore, dwBack);

    // Can't use CBitmap::FromHandle here because we need to
    //  delete the object later
    CBitmap * pBitmap = new CBitmap;
    pBitmap->Attach(hBitmap);
    pBitmap->GetObject(sizeof(bm), &bm);
    bmSize.cx = bm.bmWidth;
    bmSize.cy = bm.bmHeight;
    delete pDC->SelectObject(pBitmap); // delete configuration bitmap
    return pOriginalBitmap;
}

///////////////////////////////////////////////////////////////////
BOOL CDib::Display(CDC * pDC, CPoint origin) {
    // direct to device--bypass the GDI bitmap
    if (!m_lpBuf) {
        return FALSE; // nothing to display
    }
    if (!::SetDIBitsToDevice(pDC->GetSafeHdc(), origin.x, origin.y,
                             (WORD) m_lpBMIH->biWidth, (WORD) m_lpBMIH->biHeight, 0, 0, 0,
                             (WORD) m_lpBMIH->biHeight, m_lpDibData, m_lpBMI,
                             DIB_RGB_COLORS)) {
        return FALSE;
    }
    return TRUE;
}

///////////////////////////////////////////////////////////////////
BOOL CDib::Stretch(CDC * pDC, CPoint origin, CSize size) {
    // direct to device--bypass the GDI bitmap
    if (!m_lpBuf) {
        return FALSE; // nothing to display
    }
    if (!::StretchDIBits(pDC->GetSafeHdc(), origin.x, origin.y,
                         size.cx, size.cy, 0, 0, (WORD) m_lpBMIH->biWidth,
                         (WORD) m_lpBMIH->biHeight, m_lpDibData, m_lpBMI,
                         DIB_RGB_COLORS, SRCCOPY)) {
        return FALSE;
    }
    return TRUE;
}



/*************************************************************************
*
* PaintDIB()
*
* Parameters:
*
* HDC hDC          - DC to do output to
*
* LPRECT lpDCRect  - rectangle on DC to do output to
*
* HDIB hDIB        - handle to global memory with a DIB spec
*                    in it followed by the DIB bits
*
* LPRECT lpDIBRect - rectangle of DIB to output into lpDCRect
*
* CPalette* pPal   - pointer to CPalette containing DIB's palette
*
* Return Value:
*
* BOOL             - TRUE if DIB was drawn, FALSE otherwise
*
* Description:
*   Painting routine for a DIB.  Calls StretchDIBits() or
*   SetDIBitsToDevice() to paint the DIB.  The DIB is
*   output to the specified DC, at the coordinates given
*   in lpDCRect.  The area of the DIB to be output is
*   given by lpDIBRect.
*
************************************************************************/
BOOL CDib::Paint(CDC * pDC,
                 const CRect & tRect) {
    BOOL     bSuccess=FALSE;      // Success/fail flag
    CPalette * pOldPal=NULL;        // Previous palette
    CRect    targRect(tRect);

    if (m_pPal == NULL) {
        CreateDIBPalette();
    }

    // Get the DIB's palette, then select it into DC
    if (m_pPal != NULL) {
        // Select as background since we have
        // already realized in forground if needed
        pOldPal = pDC->SelectPalette(m_pPal, TRUE);
    }

    /* Make sure to use the stretching mode best for color pictures */
    ::SetStretchBltMode(pDC->GetSafeHdc(), COLORONCOLOR);

    int tw = targRect.Width();
    int th = targRect.Height();

    bSuccess = ::StretchDIBits(pDC->GetSafeHdc(),               // hDC
                               targRect.left,                   // DestX
                               targRect.top,                    // DestY
                               tw,                              // nDestWidth
                               th,                              // nDestHeight
                               0,                               // SrcX
                               0,                               // SrcY
                               (WORD)m_lpBMIH->biWidth,         // wSrcWidth
                               (WORD)m_lpBMIH->biHeight,        // wSrcHeight
                               m_lpDibData,                     // lpBits
                               m_lpBMI,
                               DIB_RGB_COLORS,                  // wUsage
                               SRCCOPY);                        // dwROP

    if (bSuccess == 0) { // SDM 1.5Test10.8
        CSaString szError = "Unable to scale color range. Please set your \nvideo card to 256 colors and try again.";
        ((CSaApp *)AfxGetApp())->ErrorMessage(szError);
    }

    /* Reselect old palette */
    if (pOldPal != NULL) {
        pDC->SelectPalette(pOldPal, TRUE);
    }

    return bSuccess;
}


/*************************************************************************
*
* CreateDIBPalette()
*
* Parameter:
*
* HDIB hDIB        - specifies the DIB
*
* Return Value:
*
* HPALETTE         - specifies the palette
*
* Description:
*
* This function creates a palette from a DIB by allocating memory for the
* logical palette, reading and storing the colors from the DIB's color table
* into the logical palette, creating a palette from this logical palette,
* and then returning the palette's handle. This allows the DIB to be
* displayed using the best possible colors (important for DIBs with 256 or
* more colors).
*
************************************************************************/
void CDib::CreateDIBPalette() {
    LPLOGPALETTE lpPal = NULL; // pointer to a logical palette
    HANDLE hLogPal;            // handle to a logical palette
    int i;                     // loop index
    WORD wNumColors;           // number of colors in color table
    //LPSTR lpbi;              // pointer to packed-DIB
    LPBITMAPINFO lpbmi;        // pointer to BITMAPINFO structure (Win3.0)
    //LPBITMAPCOREINFO lpbmc;  // pointer to BITMAPCOREINFO structure (old)
    BOOL bResult = FALSE;

    m_pPal = new CPalette;  // Create a new palette

    if (m_pPal == NULL) {
        return;
    }

    /* get pointer to BITMAPINFO (Win 3.0) */
    lpbmi = m_lpBMI;

    /* get the number of colors in the DIB */
    wNumColors = NumColors();

    if (wNumColors != 0) {
        /* allocate memory block for logical palette */
        hLogPal = ::GlobalAlloc(GHND, sizeof(LOGPALETTE)
                                + sizeof(PALETTEENTRY)
                                * wNumColors);

        if (hLogPal) {
            lpPal = (LPLOGPALETTE) ::GlobalLock((HGLOBAL) hLogPal);
        }

        if (!lpPal) {
            delete m_pPal;
            m_pPal = NULL;
            return;
        }

        /* set version and number of palette entries */
        lpPal->palNumEntries = (WORD)wNumColors;

        for (i = 0; i < (int)wNumColors; i++) {
            lpPal->palPalEntry[i].peRed = lpbmi->bmiColors[i].rgbRed;
            lpPal->palPalEntry[i].peGreen = lpbmi->bmiColors[i].rgbGreen;
            lpPal->palPalEntry[i].peBlue = lpbmi->bmiColors[i].rgbBlue;
            lpPal->palPalEntry[i].peFlags = 0;
        }


        /* create the palette and get a handle to it */
        bResult = m_pPal->CreatePalette(lpPal);
        ::GlobalUnlock((HGLOBAL) hLogPal);
        ::GlobalFree((HGLOBAL) hLogPal);
    }

    if (!bResult) {
        delete m_pPal;
        m_pPal = NULL;
    }
}



///////////////////////////////////////////////////////////////////
WORD CDib::NumColors(void) const {
    /* return number of colors based on bits per pixel */
    switch (GetColorBits()) {
    case 1:
        return 2;
    case 4:
        return 16;
    case 8:
        return 256;
    default:
        return 0;
    }
}



///////////////////////////////////////////////////////////////////
int CDib::GetColorBits() const {
    return m_nBits;
}

///////////////////////////////////////////////////////////////////
DWORD CDib::GetLength() const {
    return m_dwLength;
}


///////////////////////////////////////////////////////////////////
CSize CDib::GetSize() const {
    return CSize((int) m_lpBMIH->biWidth, (int) m_lpBMIH->biHeight);
}

///////////////////////////////////////////////////////////////////
void CDib::SetMonoColors(DWORD dwForeground, DWORD dwBackground) {
    if (m_nBits != 1) {
        return;
    }
    unsigned long * pPalette = (unsigned long *)
                               ((LPSTR) m_lpBMIH + sizeof(BITMAPINFOHEADER));
    *pPalette = dwForeground;
    *(++pPalette) = dwBackground;
    return;
}

///////////////////////////////////////////////////////////////////
BOOL CDib::GetMonoColors(DWORD & dwForeground, DWORD & dwBackground) {
    if (m_nBits != 1) {
        return FALSE;
    }
    unsigned long * pPalette = (unsigned long *)
                               ((LPSTR) m_lpBMIH + sizeof(BITMAPINFOHEADER));
    dwForeground = *pPalette;
    dwBackground = *(++pPalette);
    return TRUE;
}

///////////////////////////////////////////////////////////////////
BOOL CDib::AllocateMemory(BOOL bRealloc) { // bRealloc default = FALSE
    if (bRealloc) {
        m_lpBuf = (char *) GlobalReAllocPtr(m_lpBuf,
                                            m_dwLength, GHND);
    } else {
        m_lpBuf = (char *) GlobalAllocPtr(GHND, m_dwLength);
    }
    if (!m_lpBuf) {
        AfxMessageBox(_T("Unable to allocate DIB memory"));
        m_dwLength = 0L;
        m_nBits = 0;
        return FALSE;
    }
    m_lpBMFH = (LPBITMAPFILEHEADER) m_lpBuf;
    m_lpBMIH = (LPBITMAPINFOHEADER)(m_lpBuf + sizeof(BITMAPFILEHEADER));
    m_lpBMI = (LPBITMAPINFO) m_lpBMIH;
    return TRUE;
}





/*************************************************************************
* CDib::HasPalette - returns TRUE if the bitmap has a palette.
*/
inline BOOL CDib::HasPalette(void) {
    return (NumColors()>0);
}




/*************************************************************************
* CDib::MakeGrey - turns the passed in rgb value grey.
*/
BYTE CDib::MakeGrey(COLORREF & rgb) {
    BYTE r = GetRValue(rgb);
    BYTE g = GetGValue(rgb);
    BYTE b = GetBValue(rgb);
    BYTE grey = BYTE(((r*30) + (g*59) + (b*11))/100);

    rgb = RGB(grey,grey,grey);

    return grey;
}

/*************************************************************************
* CDib::RGB16 - like RBG(), but takes 2 bytes which should be the hi and
* lo bytes of a 16 bit color value.
*
* 16 bit colors values go like this:
*
*          [------HI-BYTE-------]     [------LO-BYTE--------]
*          7  6  5  4  3  2  1  0     7  6  5  4  3  2  1  0
*          0 [----RED------][---GREEN---------][---BLUE-----]
*/
inline COLORREF RGB16(BYTE hi, BYTE lo) {
    BYTE red    = BYTE((hi & 0x7c) << 1);
    BYTE green  = BYTE(((lo & 0xe0) >> 2) + ((hi & 0x03) << 6));
    BYTE blue   = BYTE((lo & 0x1f) << 3);

    return RGB(red,green,blue);
}




/*************************************************************************
* CDib::GetRGB16 - gets a 16bit color from a COLORREF.
* 16 bit colors values go like this:
*
*          HI-BYTE                            LO-BYTE
*         15 14 13 12 11 10  9  8      7  6  5  4  3  2  1  0
*          0 [----RED------][---GREEN---------][---BLUE-----]
*
*  Since each component of the color is represented with 5 bits instead
*  of 8, you have to right shift by 3 bits to go from the 8 bit color
*  component value to the 5 bit color component value.
*/
inline WORD GetRGB16(COLORREF rgb) {
    BYTE red    = BYTE(GetRValue(rgb) >> 3);
    BYTE green  = BYTE(GetGValue(rgb) >> 3);
    BYTE blue   = BYTE(GetBValue(rgb) >> 3);

    return WORD((red << 10) + (green << 5) + blue);
}



/*************************************************************************
* CDib::GoGreyScale - turns the bitmap grey.
*/
void CDib::GoGreyScale(void) {
    if (HasPalette()) {
        for (int i = 0; i < (int)NumColors(); i++) {
            COLORREF c = RGB(m_lpBMI->bmiColors[i].rgbRed,
                             m_lpBMI->bmiColors[i].rgbGreen,
                             m_lpBMI->bmiColors[i].rgbBlue);

            m_lpBMI->bmiColors[i].rgbRed        =
                m_lpBMI->bmiColors[i].rgbGreen      =
                    m_lpBMI->bmiColors[i].rgbBlue       = MakeGrey(c);
            m_lpBMI->bmiColors[i].rgbReserved   = 0;
        }
    } else {
        ASSERT(GetColorBits()==16 || GetColorBits()==24);
        ASSERT(m_lpBMIH->biSizeImage > 0);

        if (GetColorBits()==24) {
            BYTE * p = (BYTE *)m_lpDibData;
            BYTE * end = (p+m_lpBMIH->biSizeImage);
            for (; p < end; p+=3) {
                COLORREF c = RGB(p[0],p[1],p[2]);
                BYTE grey = MakeGrey(c);
                p[2] = grey;
                p[1] = grey;
                p[0] = grey;
            }
        } else {
            BYTE * p = (BYTE *)m_lpDibData;
            BYTE * end = (p+m_lpBMIH->biSizeImage);
            for (; p < end; p+=2) {
                COLORREF c = RGB16(p[1],p[0]);
                MakeGrey(c);
                WORD rgb16 = GetRGB16(c);
                p[1] = HIBYTE(rgb16);
                p[0] = LOBYTE(rgb16);
            }
        }
    }
}
