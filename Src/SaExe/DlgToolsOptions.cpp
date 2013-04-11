//////////////////////////////////////////////////////////////////////////////
// toolsOptions.cpp:
// Implementation of the CDlgOptionsViewPage (property page)
//                       CDlgOptionsColorPage (property page)
//                       CDlgOptionsFontPage (property page)
//                       CDlgOptionsMemoryPage (property page)
//                       CDlgOptionsSavePage (property page)
//                       CDlgToolsOptions (property sheet)
//
// Author: Urs Ruchti
// copyright 2000 JAARS Inc. SIL
//
// Revision History
//
// 01/12/2000
//    SDM   Extract from sa_dlg2.cpp
//
/////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "DlgToolsOptions.h"
#include "Process\Process.h"
#include "Process\sa_p_fra.h"
#include "mainfrm.h"
#include "sa_doc.h"
#include "sa.h"
#include "sa_annot.h"
#include "sa_view.h"

//###########################################################################
// CDlgOptionsViewPage property page
// Displays all important controls to change the view of this application.

/////////////////////////////////////////////////////////////////////////////
// CDlgOptionsViewPage message map

BEGIN_MESSAGE_MAP(CDlgOptionsViewPage, CPropertyPage)
    //{{AFX_MSG_MAP(CDlgOptionsViewPage)
    ON_BN_CLICKED(ID_VIEW_STATUSBAR, OnModifiedStatusbar)
    ON_CBN_SELCHANGE(IDC_VIEW_UNITSMODE, OnModified)
    ON_BN_CLICKED(ID_VIEW_TOOLBAR, OnModified)
    ON_BN_CLICKED(ID_VIEW_TASKBAR, OnModified)
    ON_BN_CLICKED(IDC_VIEW_TONEABOVE, OnModified)
    ON_BN_CLICKED(IDC_SCROLLBARZOOM, OnModified)
    ON_BN_CLICKED(IDC_XGRID, OnModified)
    ON_BN_CLICKED(IDC_YGRID, OnModified)
    ON_CBN_SELCHANGE(IDC_VIEWCAPTION, OnModified)
    ON_CBN_SELCHANGE(IDC_XGRIDTYPE, OnModified)
    ON_CBN_SELCHANGE(IDC_YGRIDTYPE, OnModified)
    ON_BN_CLICKED(IDC_SNAPTOSAMPLE, OnModifiedRange)
    ON_BN_CLICKED(IDC_SNAPTOZEROCROSSING, OnModifiedRange)
    ON_BN_CLICKED(IDC_SNAPTOFRAGMENT, OnModifiedRange)
    ON_COMMAND(IDC_VIEW_ANIMRATESCROLL, OnAnimationRateScroll)
    ON_BN_CLICKED(IDC_VIEW_UPDATE_STATIC, OnModifiedRange)
    ON_BN_CLICKED(IDC_VIEW_UPDATE_DYNAMIC, OnModifiedRange)
    ON_BN_CLICKED(IDC_VIEW_ANIMATE, OnModifiedAnimate)
    ON_EN_CHANGE(IDC_VIEW_ANIMRATEEDIT, OnChange)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgOptionsViewPage construction/destruction/creation

/***************************************************************************/
// CDlgOptionsViewPage::CDlgOptionsViewPage Constructor
/***************************************************************************/
CDlgOptionsViewPage::CDlgOptionsViewPage() : CPropertyPage(CDlgOptionsViewPage::IDD)
{

    m_bStatusbar = FALSE;
    m_bToolbar = FALSE;
    m_bScrollZoom = FALSE;
    m_nCaptionStyle = MiniWithCaption;
    m_bXGrid = TRUE;
    m_bYGrid = TRUE;
    m_nPitchMode = 0;
    m_nPosMode = 0;
    m_bToneAbove = FALSE;
    m_bTaskbar = FALSE;
    SetGridStyle(0,0);
}

/***************************************************************************/
// CDlgOptionsViewPage::DoDataExchange Data exchange
/***************************************************************************/
void CDlgOptionsViewPage::DoDataExchange(CDataExchange * pDX)
{
    CPropertyPage::DoDataExchange(pDX);
    DDX_Check(pDX, ID_VIEW_STATUSBAR, m_bStatusbar);
    DDX_Check(pDX, ID_VIEW_TOOLBAR, m_bToolbar);
    DDX_Check(pDX, IDC_SCROLLBARZOOM, m_bScrollZoom);
    DDX_CBIndex(pDX, IDC_VIEWCAPTION, m_nCaptionStyle);
    DDX_Check(pDX, IDC_XGRID, m_bXGrid);
    DDX_Check(pDX, IDC_YGRID, m_bYGrid);
    DDX_CBIndex(pDX, IDC_XGRIDTYPE, m_nXStyleIndex);
    DDX_CBIndex(pDX, IDC_YGRIDTYPE, m_nYStyleIndex);
    DDX_Radio(pDX, IDC_SNAPTOSAMPLE, m_nCursorAlignment);
    DDX_CBIndex(pDX, IDC_VIEW_UNITSMODE, m_nPosMode);
    DDX_Check(pDX, IDC_VIEW_TONEABOVE, m_bToneAbove);
    DDX_Radio(pDX, IDC_VIEW_UPDATE_STATIC, m_nGraphUpdateMode);
    DDX_Check(pDX, IDC_VIEW_ANIMATE, m_bAnimate);
    DDX_Text(pDX, IDC_VIEW_ANIMRATEEDIT, m_nAnimationRate);
    DDX_Check(pDX, ID_VIEW_TASKBAR, m_bTaskbar);
}

/***************************************************************************/
// CDlgOptionsViewPage::OnInitDialog Dialog initialization
// The grid line combo boxes are sub-classed with a user drawn control.
/***************************************************************************/
BOOL CDlgOptionsViewPage::OnInitDialog()
{

    static const int StyleList[] =
    {
        // This list is based on the interpretation of Style numbers in the rest of SA.
        0,
        PS_SOLID,
        PS_DASH,
        PS_DOT,
        PS_DASHDOT,
        PS_DASHDOTDOT,
        -1
    };

    CPropertyPage::OnInitDialog();
    // subclass the control
    VERIFY(m_xGridlines.SubclassDlgItem(IDC_XGRIDTYPE, this));
    VERIFY(m_yGridlines.SubclassDlgItem(IDC_YGRIDTYPE, this));
    // add pen styles to the combobox
    for (int n=1; IndexToStyle[n] != -1; n++)
    {
        m_xGridlines.AddLineItem(StyleList[IndexToStyle[n]]);
        m_yGridlines.AddLineItem(StyleList[IndexToStyle[n]]);
    }
    // if no statusbar disable statusbar readout modes
    if (!m_bStatusbar)
    {
        GetDlgItem(IDC_VIEW_UNITSTEXT)->EnableWindow(FALSE);
        GetDlgItem(IDC_VIEW_UNITSMODE)->EnableWindow(FALSE);
    }
    // build and place the animation frame rate spin control
    m_SpinAnimationRate.Init(IDC_VIEW_ANIMRATESCROLL, this);
    // enable cursor alignment fragment option if fragmenter done
    CMainFrame * pMDIFrameWnd = (CMainFrame *)AfxGetMainWnd();
    CView * pView = pMDIFrameWnd->GetCurrSaView();
    CSaDoc * pDoc = (CSaDoc *) pView->GetDocument();
    CProcessFragments * pFragmenter = pDoc->GetFragments();
    if (pFragmenter->IsDataReady())
    {
        GetDlgItem(IDC_SNAPTOFRAGMENT)->EnableWindow(TRUE);
    }
    else
    {
        GetDlgItem(IDC_SNAPTOFRAGMENT)->EnableWindow(FALSE);
    }

    // enable dynamic update based on cursor alignment
    CSaView * pSaView = (CSaView *)pView;
    BOOL bEnable = (pSaView->GetCursorAlignment() == ALIGN_AT_FRAGMENT);

    GetDlgItem(IDC_VIEW_UPDATE_DYNAMIC)->EnableWindow(bEnable);
    GetDlgItem(IDC_VIEW_UPDATE_STATIC)->EnableWindow(bEnable);

    bEnable &= (pSaView->GetGraphUpdateMode() == DYNAMIC_UPDATE);

    GetDlgItem(IDC_VIEW_ANIMATE)->EnableWindow(bEnable);

    bEnable &= pSaView->IsAnimationRequested();

    m_SpinAnimationRate.EnableWindow(bEnable);
    GetDlgItem(IDC_VIEW_ANIMRATETEXT)->EnableWindow(bEnable);
    UpdateData(FALSE);
    return TRUE;  // return TRUE  unless you set the focus to a control
}

/***************************************************************************/
// CDlgOptionsViewPage::OnModifiedStatusbar Status bar data modified
/***************************************************************************/
void CDlgOptionsViewPage::OnModifiedStatusbar()
{

    UpdateData(TRUE);
    if (m_bStatusbar)
    {
        GetDlgItem(IDC_VIEW_UNITSTEXT)->EnableWindow(TRUE);
        GetDlgItem(IDC_VIEW_UNITSMODE)->EnableWindow(TRUE);
    }
    else
    {
        GetDlgItem(IDC_VIEW_UNITSTEXT)->EnableWindow(FALSE);
        GetDlgItem(IDC_VIEW_UNITSMODE)->EnableWindow(FALSE);
    }
    SetModified(TRUE); // data modified, enable apply button
}

/***************************************************************************/
// CDlgOptionsViewPage::OnModified Data modified
/***************************************************************************/
void CDlgOptionsViewPage::OnModified()
{

    SetModified(TRUE); // data modified, enable apply button
}

/***************************************************************************/
// CDlgOptionsViewPage::OnModifiedRange Cursor Alignment range modified
/***************************************************************************/
void CDlgOptionsViewPage::OnModifiedRange()
{

    int nOldCursorAlignment = m_nCursorAlignment;
    int nOldGraphUpdateMode = m_nGraphUpdateMode;
    UpdateData(TRUE);

    BOOL bEnable = (m_nCursorAlignment == ALIGN_AT_FRAGMENT);
    if (m_nCursorAlignment != nOldCursorAlignment)
    {
        bEnable = (m_nCursorAlignment == ALIGN_AT_FRAGMENT);
        GetDlgItem(IDC_VIEW_UPDATE_DYNAMIC)->EnableWindow(bEnable);
        GetDlgItem(IDC_VIEW_UPDATE_STATIC)->EnableWindow(bEnable);

        OnModifiedAnimate();
    }
    if (m_nGraphUpdateMode != nOldGraphUpdateMode)
    {
        OnModifiedAnimate();
    }
    bEnable &= (m_nGraphUpdateMode == DYNAMIC_UPDATE);
    GetDlgItem(IDC_VIEW_ANIMATE)->EnableWindow(bEnable);
}

/***************************************************************************/
// CDlgParametersViewPage::OnModifiedAnimate Animate on/off changed
/***************************************************************************/
void CDlgOptionsViewPage::OnModifiedAnimate()
{

    UpdateData(TRUE);

    m_bAnimate = (BOOL)IsDlgButtonChecked(IDC_VIEW_ANIMATE);

    BOOL bEnable = m_bAnimate && (m_nGraphUpdateMode == DYNAMIC_UPDATE);
    m_SpinAnimationRate.EnableWindow(bEnable);
    GetDlgItem(IDC_VIEW_ANIMRATETEXT)->EnableWindow(bEnable);
    SetModified(TRUE); // data modified, enable apply button
    m_bModified = TRUE;
}

/***************************************************************************/
// CDlgOptionsViewPage::OnAnimationRateScroll Animation spin control hit
/***************************************************************************/
void CDlgOptionsViewPage::OnAnimationRateScroll()
{

    int nOptions[] = {1, 2, 5, 10, 30, 60};   // only options allowed
    int nOptionsCount = sizeof(nOptions)/sizeof(*nOptions);
    int nData = GetDlgItemInt(IDC_VIEW_ANIMRATEEDIT, NULL, TRUE);
    int nIndex = 0;
    for (; nIndex < nOptionsCount; nIndex++)
        if (nData == nOptions[nIndex])
        {
            break;
        }
    if (m_SpinAnimationRate.UpperButtonClicked())
    {
        nIndex++;
    }
    else
    {
        nIndex--;
    }
    if (nIndex >= nOptionsCount)
    {
        nIndex = nOptionsCount - 1;
    }
    if (nIndex < 0)
    {
        nIndex = 0;
    }
    if (m_nAnimationRate != nOptions[nIndex])
    {
        m_nAnimationRate = nOptions[nIndex];
        m_bModified = TRUE;
        SetModified(TRUE); // data modified, enable apply button
        SetDlgItemInt(IDC_VIEW_ANIMRATEEDIT, m_nAnimationRate, TRUE);
    }
}

/***************************************************************************/
// CDlgOptionsViewPage::OnChange page changed
/***************************************************************************/
void CDlgOptionsViewPage::OnChange()
{

    m_bModified = TRUE;
    SetModified(TRUE); // data modified, enable apply button
}

const int CDlgOptionsViewPage::IndexToStyle[] =
{

    // refers to entries in StyleList in Init
    0,
    3,
    5,
    4,
    2,
    1,
    -1 // Terminate List
};


/***************************************************************************/
// CDlgOptionsViewPage::SetGridStyle
/***************************************************************************/
void CDlgOptionsViewPage::SetGridStyle(int nXStyle, int nYStyle)
{

    int n;
    for (n=0; IndexToStyle[n] != -1; n++)
    {
        if (nXStyle == IndexToStyle[n])
        {
            m_nXStyleIndex = n;
            break;
        }
    }

    for (n=0; IndexToStyle[n] != -1; n++)
    {
        if (nYStyle == IndexToStyle[n])
        {
            m_nYStyleIndex = n;
            break;
        }
    }
}

/***************************************************************************/
// CDlgOptionsViewPage::GetGridStyle
/***************************************************************************/
void CDlgOptionsViewPage::GetGridStyle(int * nXStyle, int * nYStyle)
{

    if (nXStyle)
    {
        *nXStyle = IndexToStyle[m_nXStyleIndex];
    }

    if (nYStyle)
    {
        *nYStyle = IndexToStyle[m_nYStyleIndex];
    }
}

/***************************************************************************
* CDlgOptionsViewPage::EnableDynamicUpdate
* enable dynamic graph update mode
***************************************************************************/
void CDlgOptionsViewPage::EnableDynamicUpdate(BOOL bState)
{

    GetDlgItem(IDC_VIEW_UPDATE_DYNAMIC)->EnableWindow(bState);
}

//###########################################################################
// CDlgOptionsColorPage property page
// Displays all important controls to change the color scheme of this appli-
// cation.

BEGIN_MESSAGE_MAP(CDlgOptionsColorPage, CPropertyPage)
    ON_BN_CLICKED(IDC_DEFAULT, OnDefault)
    ON_BN_CLICKED(IDC_SYSTEM, OnSystem)
    ON_BN_CLICKED(IDC_CHGCOLOR_ANNOT, OnChgColorAnnot)
    ON_BN_CLICKED(IDC_CHGCOLOR_GRAPH, OnChgColorGraph)
    ON_BN_CLICKED(IDC_CHGCOLOR_OVRLY, OnChgColorOvrly)
    ON_BN_CLICKED(IDC_CHGCOLOR_SCALE, OnChgColorScale)
END_MESSAGE_MAP()

/***************************************************************************/
// CComboColor::MeasureItem
/***************************************************************************/
void CComboColor::MeasureItem(LPMEASUREITEMSTRUCT lpMIS)
{

    lpMIS->itemHeight = 15;
}

/***************************************************************************/
// CComboColor::DrawItem Item drawing
/***************************************************************************/
void CComboColor::DrawItem(LPDRAWITEMSTRUCT lpDIS)
{

    RECT     rc;
    CDC   *  pDC               = CDC::FromHandle(lpDIS->hDC);
    int      nTextTop          = lpDIS->rcItem.top + (((lpDIS->rcItem.bottom - lpDIS->rcItem.top + 1) > 16) ? 2 : 1);
    int      nTextLeft         = lpDIS->rcItem.left + (((lpDIS->rcItem.bottom - lpDIS->rcItem.top + 1) > 16) ? 1 : 2);
    int      nBackmode         = pDC->SetBkMode(TRANSPARENT);
    COLORREF colorBk           = pDC->GetBkColor();
    COLORREF colorTx           = pDC->GetTextColor();
    ColorComboInfo * pColorInfo = (ColorComboInfo *)lpDIS->itemData;

    rc.left   = lpDIS->rcItem.right  - 15;
    rc.top    = lpDIS->rcItem.top    + 2;
    rc.right  = lpDIS->rcItem.right  - 2;
    rc.bottom = lpDIS->rcItem.bottom - 2;

    if ((lpDIS->itemAction & ODA_DRAWENTIRE) || (lpDIS->itemID == -1) ||
            (!(lpDIS->itemState & ODS_SELECTED) && (lpDIS->itemAction & ODA_SELECT)))
    {
        CBrush brush(GetSysColor(COLOR_WINDOW));
        pDC->FillRect(&lpDIS->rcItem, &brush);
        if (lpDIS->itemID == -1)
        {
            return;
        }

        pDC->SetTextColor(GetSysColor(COLOR_WINDOWTEXT));
        pDC->TextOut(nTextLeft, nTextTop, pColorInfo->pszColorItem, _tcslen(pColorInfo->pszColorItem));

        CBrush brush2(GetSysColor(COLOR_WINDOWTEXT));
        pDC->FrameRect(&rc, &brush2);
    }

    if ((lpDIS->itemState & ODS_SELECTED) && (lpDIS->itemAction & (ODA_SELECT | ODA_DRAWENTIRE)))
    {
        CBrush brush(GetSysColor(COLOR_HIGHLIGHT));
        pDC->FillRect(&lpDIS->rcItem, &brush);

        pDC->SetTextColor(GetSysColor(COLOR_HIGHLIGHTTEXT));
        pDC->TextOut(nTextLeft, nTextTop, pColorInfo->pszColorItem, _tcslen(pColorInfo->pszColorItem));

        CBrush brush2(GetSysColor(COLOR_HIGHLIGHTTEXT));
        pDC->FrameRect(&rc, &brush2);
    }

    rc.left++;
    rc.top++;
    rc.right--;
    rc.bottom--;
    CBrush brush(pColorInfo->cColor);
    pDC->FillRect(&rc, &brush);

    if (lpDIS->itemState & ODS_FOCUS)
    {
        pDC->DrawFocusRect(&lpDIS->rcItem);
    }

    pDC->SetBkColor(colorBk);
    pDC->SetTextColor(colorTx);
    pDC->SetBkMode(nBackmode);
}

/***************************************************************************/
/***************************************************************************/
ColorComboInfo GraphColorsComboInfo[] =
{

    {_T("Background"),             0L},
    {_T("Highlighted Background"), 0L},
    {_T("Axes"),                   0L},
    {_T("First Data"),             0L},
    {_T("Second Data"),            0L},
    {_T("Third Data"),             0L},
    {_T("Fourth Data"),            0L},
    {_T("Fifth Data"),             0L},
    {_T("Sixth Data"),             0L},
    {_T("Highlighted Data"),       0L},
    {_T("Phonetic Boundaries"),    0L},
    {_T("Gridlines"),              0L},
    {_T("Begin Cursor"),           0L},
    {_T("End Cursor"),             0L},
    {_T("Private Cursor"),         0L},
    {NULL,                     0L}
};

ColorComboInfo AnnotColorsComboInfo[] =
{

    {_T("Phonetic Background"),    0L},
    {_T("Phonetic Fonts"),         0L},
    {_T("Tone Background"),        0L},
    {_T("Tone Fonts"),             0L},
    {_T("Phonemic Background"),    0L},
    {_T("Phonemic Fonts"),         0L},
    {_T("Orthographic Background"), 0L},
    {_T("Orthographic Fonts"),      0L},
    {_T("Gloss Background"),       0L},
    {_T("Gloss Fonts"),            0L},
    {NULL,                     0L}
};

ColorComboInfo ScaleColorsComboInfo[] =
{

    {_T("Background"), 0L},
    {_T("Lines"),      0L},
    {_T("Fonts"),      0L},
    {NULL,         0L}
};

ColorComboInfo OvrlyColorsComboInfo[] =
{

    {_T("Overlay 1 Data"), 0L},
    {_T("Overlay 2 Data"), 0L},
    {_T("Overlay 3 Data"), 0L},
    {_T("Overlay 4 Data"), 0L},
    {_T("Overlay 5 Data"), 0L},
    {NULL,             0L}
};

/***************************************************************************/
// CDlgOptionsColorPage::CDlgOptionsColorPage Constructor
/***************************************************************************/
CDlgOptionsColorPage::CDlgOptionsColorPage() : CPropertyPage(CDlgOptionsColorPage::IDD)
{

    m_nGraphSelect = 0;
    m_nAnnotationSelect = -1;
    m_nScaleSelect = -1;
    m_nOverlaySelect = -1;
    m_cColors.SetupDefault(); // set the internal color structure back to default
}

/***************************************************************************/
// CDlgOptionsColorPage::DoDataExchange Data exchange
/***************************************************************************/
void CDlgOptionsColorPage::DoDataExchange(CDataExchange * pDX)
{
    CPropertyPage::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CDlgOptionsColorPage)
    DDX_CBIndex(pDX, IDC_COLORSGRAPHSETUP, m_nGraphSelect);
    DDX_CBIndex(pDX, IDC_COLORSANNOTSETUP, m_nAnnotationSelect);
    DDX_CBIndex(pDX, IDC_COLORSSCALESETUP, m_nScaleSelect);
    DDX_CBIndex(pDX, IDC_COLORSOVERLAY,    m_nOverlaySelect);
}

/***************************************************************************/
// CDlgOptionsColorPage::FillColorComboBoxInfo
// DDO - 08/12/00
/***************************************************************************/
void CDlgOptionsColorPage::FillColorComboBoxInfo(BOOL bAddStrings)
{

    int i = 0;
    for (; GraphColorsComboInfo[i].pszColorItem; i++)
    {
        if (bAddStrings)
        {
            m_GraphItemColors.AddString(NULL);
        }
        GraphColorsComboInfo[i].cColor = *(&m_cColors.cPlotBkg + i);
        m_GraphItemColors.SetItemDataPtr(i, &GraphColorsComboInfo[i]);
    }

    for (i = 0; AnnotColorsComboInfo[i].pszColorItem; i++)
    {
        if (bAddStrings)
        {
            m_AnnotItemColors.AddString(NULL);
        }
        if (i % 2)
        {
            AnnotColorsComboInfo[i].cColor = *(&m_cColors.cAnnotationFont[PHONETIC] + i / 2);
        }
        else
        {
            AnnotColorsComboInfo[i].cColor = *(&m_cColors.cAnnotationBkg[PHONETIC] + i / 2);
        }
        m_AnnotItemColors.SetItemDataPtr(i, &AnnotColorsComboInfo[i]);
    }

    for (i = 0; ScaleColorsComboInfo[i].pszColorItem; i++)
    {
        if (bAddStrings)
        {
            m_ScaleItemColors.AddString(NULL);
        }
        ScaleColorsComboInfo[i].cColor = *(&m_cColors.cScaleBkg + i);
        m_ScaleItemColors.SetItemDataPtr(i, &ScaleColorsComboInfo[i]);
    }

    for (i = 0; OvrlyColorsComboInfo[i].pszColorItem; i++)
    {
        if (bAddStrings)
        {
            m_OvrlyItemColors.AddString(NULL);
        }
        OvrlyColorsComboInfo[i].cColor = m_cColors.cPlotData[i];
        m_OvrlyItemColors.SetItemDataPtr(i, &OvrlyColorsComboInfo[i]);
    }

    //************************************************************
    // If bAddStrings is false, it means we're here because the
    // user changed one of the colors so we need to force the
    // combo boxes to redraw so they're displaying the color
    // change.
    //************************************************************
    if (!bAddStrings)
    {
        GetDlgItem(IDC_COLORSGRAPHSETUP)->Invalidate();
        GetDlgItem(IDC_COLORSANNOTSETUP)->Invalidate();
        GetDlgItem(IDC_COLORSSCALESETUP)->Invalidate();
        GetDlgItem(IDC_COLORSOVERLAY)->Invalidate();
    }
}

/***************************************************************************/
// CDlgOptionsColorPage::OnInitDialog Dialog initialization
/***************************************************************************/
BOOL CDlgOptionsColorPage::OnInitDialog()
{

    CPropertyPage::OnInitDialog();
    m_bColorsChanged = FALSE;

    //**************************************************************
    // The following added by DDO - 08/12/00
    //**************************************************************
    m_GraphItemColors.SubclassDlgItem(IDC_COLORSGRAPHSETUP, this);
    m_AnnotItemColors.SubclassDlgItem(IDC_COLORSANNOTSETUP, this);
    m_ScaleItemColors.SubclassDlgItem(IDC_COLORSSCALESETUP, this);
    m_OvrlyItemColors.SubclassDlgItem(IDC_COLORSOVERLAY,    this);
    FillColorComboBoxInfo(TRUE);
    m_nGraphSelect      = 0;
    m_nAnnotationSelect = 0;
    m_nScaleSelect      = 0;
    m_nOverlaySelect    = 0;
    UpdateData(FALSE);

    LOGFONT logFont;
    CFont * pFont = GetDlgItem(IDC_STATIC1)->GetFont(); // get the standard font
    pFont->GetObject(sizeof(LOGFONT), (void *)&logFont); // fill up logFont
    logFont.lfWeight = FW_NORMAL;                       // make font not bold
    m_Font.CreateFontIndirect(&logFont);                // create the modified font

    GetDlgItem(IDC_STATIC1)->SetFont(&m_Font);
    GetDlgItem(IDC_STATIC2)->SetFont(&m_Font);
    //**************************************************************

    return TRUE;
}

/***************************************************************************/
// DDO - 08/12/00
/***************************************************************************/
BOOL CDlgOptionsColorPage::ChangeColor(COLORREF * pColor)
{

    CColorDialog dlgColor(*pColor);
    if (dlgColor.DoModal() == IDOK)
    {
        if (*pColor != dlgColor.m_cc.rgbResult)
        {
            *pColor = dlgColor.m_cc.rgbResult;
            m_bColorsChanged = TRUE;
            SetModified();
            FillColorComboBoxInfo(FALSE);
            return TRUE;
        }
    }
    return FALSE;
}

/***************************************************************************/
// DDO - 08/12/00
/***************************************************************************/
void CDlgOptionsColorPage::OnChgColorGraph()
{

    UpdateData(TRUE);
    if (m_nGraphSelect >= 0)
    {
        ChangeColor(&m_cColors.cPlotBkg  + m_nGraphSelect);
    }
    GetDlgItem(IDC_COLORSGRAPHSETUP)->SetFocus();
}

/***************************************************************************/
// DDO - 08/12/00
/***************************************************************************/
void CDlgOptionsColorPage::OnChgColorAnnot()
{

    UpdateData(TRUE);
    if (m_nAnnotationSelect >= 0)
    {
        if (m_nAnnotationSelect % 2)
        {
            ChangeColor(&m_cColors.cAnnotationFont[PHONETIC] + m_nAnnotationSelect / 2);
        }
        else
        {
            ChangeColor(&m_cColors.cAnnotationBkg[PHONETIC] + m_nAnnotationSelect / 2);
        }

        GetDlgItem(IDC_COLORSANNOTSETUP)->SetFocus();
    }
}

/***************************************************************************/
// DDO - 08/12/00
/***************************************************************************/
void CDlgOptionsColorPage::OnChgColorScale()
{

    UpdateData(TRUE);
    if (m_nScaleSelect >= 0)
    {
        ChangeColor(&m_cColors.cScaleBkg + m_nScaleSelect);
    }
    GetDlgItem(IDC_COLORSSCALESETUP)->SetFocus();
}

/***************************************************************************/
// DDO - 08/12/00
/***************************************************************************/
void CDlgOptionsColorPage::OnChgColorOvrly()
{

    UpdateData(TRUE);
    if (m_nOverlaySelect >= 0)
    {
        ChangeColor(&(m_cColors.cPlotData[m_nOverlaySelect]));
    }
    GetDlgItem(IDC_COLORSOVERLAY)->SetFocus();
}

/***************************************************************************/
// CDlgOptionsColorPage::OnDefault Button default hit
/***************************************************************************/
void CDlgOptionsColorPage::OnDefault()
{

    m_cColors.SetupDefault(FALSE);
    m_bColorsChanged = TRUE;
    SetModified();
    FillColorComboBoxInfo(FALSE);  // DDO - 08/12/00
}

/***************************************************************************/
// CDlgOptionsColorPage::OnSystem Button system colors hit
/***************************************************************************/
void CDlgOptionsColorPage::OnSystem()
{

    m_cColors.SetupDefault(); // set the internal color structure to system
    m_bColorsChanged = TRUE;
    SetModified();
    FillColorComboBoxInfo(FALSE);  // DDO - 08/12/00
}

//###########################################################################
// CDlgOptionsFontPage property page
// Displays all important controls to change the font of this application.

BEGIN_MESSAGE_MAP(CDlgOptionsFontPage, CPropertyPage)
    ON_BN_CLICKED(IDC_FONT, OnFont)
    ON_LBN_SELCHANGE(IDC_ANNOTLIST, OnSelChangeAnnotList)
END_MESSAGE_MAP()

/***************************************************************************/
// Added the following strings.  DDO - 08/14/00
/***************************************************************************/
static LPCTSTR pszFontAnnot[] =
{

    _T("Reference"),
    _T("Phonetic"),
    _T("Tone"),
    _T("Phonemic"),
    _T("Orthographic"),
    _T("Gloss"),
    _T("Phrase L1"),
    _T("Phrase L2"),
    _T("Phrase L3"),
    _T("Phrase L4"),
    NULL
};

/***************************************************************************/
// Added the following array.  DDO - 10/11/00
/***************************************************************************/
static const int anAnnotDisplayOrder[] =
{

    REFERENCE,
    PHONETIC,
    TONE,
    PHONEMIC,
    ORTHO,
    GLOSS,
    MUSIC_PL1,
    MUSIC_PL2,
    MUSIC_PL3,
    MUSIC_PL4
};

/***************************************************************************/
// CDlgOptionsFontPage::CDlgOptionsFontPage Constructor
/***************************************************************************/
CDlgOptionsFontPage::CDlgOptionsFontPage() : CPropertyPage(CDlgOptionsFontPage::IDD)
{

    m_szAnnotFontInfo = "";
    m_nAnnotListIndex = -1;
    m_szSampleText = "";
    m_bUseUnicodeEncoding = CSaApp::m_bUseUnicodeEncoding;
    m_bFontChanged = FALSE;
    m_pSampleFont = NULL;
}

/***************************************************************************/
/***************************************************************************/
CDlgOptionsFontPage::~CDlgOptionsFontPage()
{
    if (m_pSampleFont)
    {
        delete m_pSampleFont;
    }
}

/***************************************************************************/
// DDO - 08/14/00
/***************************************************************************/
BOOL CDlgOptionsFontPage::OnInitDialog()
{

    CPropertyPage::OnInitDialog();
    LOGFONT logFont;
    CFont * pFont = GetDlgItem(IDC_ANNOTFONT)->GetFont(); // get the standard font
    pFont->GetObject(sizeof(LOGFONT), (void *)&logFont);  // fill up logFont
    logFont.lfWeight = FW_NORMAL;                         // make font not bold
    m_Font.CreateFontIndirect(&logFont);                  // create the modified font

    GetDlgItem(IDC_ANNOTFONT)->SetFont(&m_Font);

    for (int i = 0; pszFontAnnot[i]; i++)
    {
        m_lbAnnotList.AddString(pszFontAnnot[i]);
        m_lbAnnotList.SetItemData(i, anAnnotDisplayOrder[i]);
    }

    m_nAnnotListIndex = 0;
    UpdateData(FALSE);
    OnSelChangeAnnotList();
    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CDlgOptionsFontPage helper functions

/***************************************************************************/
// CDlgOptionsFontPage::DoDataExchange Data exchange
/***************************************************************************/
void CDlgOptionsFontPage::DoDataExchange(CDataExchange * pDX)
{

    CPropertyPage::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_ANNOTLIST, m_lbAnnotList);
    DDX_Text(pDX, IDC_ANNOTFONT, m_szAnnotFontInfo);
    DDX_LBIndex(pDX, IDC_ANNOTLIST, m_nAnnotListIndex);
    DDX_Text(pDX, IDC_SAMPLETEXT, m_szSampleText);
    DDX_Check(pDX, IDC_USE_UNICODE_ENCODING, m_bUseUnicodeEncoding);
}

/////////////////////////////////////////////////////////////////////////////
// CDlgOptionsFontPage message handlers
/***************************************************************************/
// DDO - 08/14/00
/***************************************************************************/
void CDlgOptionsFontPage::OnSelChangeAnnotList()
{

    UpdateData(TRUE);
    int nIndex = (int)m_lbAnnotList.GetItemData(m_nAnnotListIndex);

    //**************************************************************
    // This calculation converts the stored font size, whose units
    // are in pixels, to the equivalent in points. Doing this
    // requires knowing how many pixels are in a logical inch and
    // how many points there are in an inch. (There are 72 points
    // per inch.)
    //**************************************************************
    int nPoints = (int)m_GraphFontSizes.GetAt(nIndex);

    //**************************************************************
    // Build a string to display for the user so he knows what the
    // current font face and point size is for the currently
    // selected annotation.
    //**************************************************************
    CString szSize;
    _itow_s(nPoints, szSize.GetBuffer(3), 3, 10);
    szSize.ReleaseBuffer();
    m_szAnnotFontInfo = "Current Setting:  " + m_GraphFonts.GetAt(nIndex);
    m_szAnnotFontInfo += (", " + szSize + " pt.");

    //**************************************************************
    // Build an appropriate sample string. If the font face name
    // is empty (which should never happen) then display no sample
    // text. If it's the pitch font then display a string that
    // looks like a valid pitch pattern. Otherwise display
    // something readable.
    //**************************************************************
    if (m_GraphFonts.GetAt(nIndex).IsEmpty())
    {
        m_szSampleText = "";
    }
    else if (m_GraphFonts.GetAt(nIndex) == TONE_DEFAULT_FONT)
    {
        m_szSampleText = "0200e00e00x00\x91";
    }
    else
    {
        m_szSampleText = "Sample Text";
    }

    //**************************************************************
    // Update the sample text and font face & size static controls.
    //**************************************************************
    UpdateData(FALSE);

    //**************************************************************
    // If there is text in the sample font static control then
    // change its font to be that of the font face name for the
    // currently selected annotation. Use the selected pt size
    //**************************************************************
    if (!m_szSampleText.IsEmpty())
    {
        if (m_pSampleFont)
        {
            delete m_pSampleFont;
        }
        m_pSampleFont = new CFont;
        CAnnotationWnd::CreateAnnotationFont(m_pSampleFont, nPoints, m_GraphFonts.GetAt(nIndex));
        GetDlgItem(IDC_SAMPLETEXT)->SetFont(m_pSampleFont);
    }
}

/***************************************************************************/
// CDlgOptionsFontPage::OnFont Button font hit
/***************************************************************************/
void CDlgOptionsFontPage::OnFont()
{

    UpdateData(TRUE); // get newest selection
    int nIndex = (int)m_lbAnnotList.GetItemData(m_nAnnotListIndex);
    // prepare the dialog
    LOGFONT logFont;
    lstrcpy(logFont.lfFaceName, m_GraphFonts.GetAt(nIndex));
    CDC * pDC = GetDC();
    logFont.lfHeight = -MulDiv(m_GraphFontSizes.GetAt(nIndex), pDC->GetDeviceCaps(LOGPIXELSY), 72);
    ReleaseDC(pDC);
    CFontDialog dlgFont(&logFont, CF_SCREENFONTS | CF_NOSCRIPTSEL | CF_ENABLETEMPLATE);

    dlgFont.m_cf.lpTemplateName = MAKEINTRESOURCE(IDD_FONT_TEMPLATE_ANNOTATION);
    dlgFont.m_cf.hInstance = AfxFindResourceHandle(MAKEINTRESOURCE(IDD_FONT_TEMPLATE_ANNOTATION),RT_DIALOG);

    if (dlgFont.DoModal() == IDOK)
    {
        m_GraphFonts.SetAt(nIndex, dlgFont.GetFaceName());
        m_GraphFontSizes.SetAt(nIndex, (dlgFont.GetSize() + 5)/10);
        m_bFontChanged = TRUE;
        SetModified();
        OnSelChangeAnnotList();
    }
}

//###########################################################################
// CDlgOptionsSavePage property page
// Displays all important controls to change the saving behaviour of this
// application.

BEGIN_MESSAGE_MAP(CDlgOptionsSavePage, CPropertyPage)
    ON_BN_CLICKED(IDC_TEMPTEMPLATE, OnSaveTempDefaultTemplate)
    ON_BN_CLICKED(IDC_PERMTEMPLATE, OnSavePermDefaultTemplate)
    ON_BN_CLICKED(ID_SAVEOPENFILES, OnReopenFiles)
    ON_BN_CLICKED(ID_SHOWSTARTUPDLG, OnShowsStartupDlg)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgOptionsSavePage construction/destruction/creation

CDlgOptionsSavePage::CDlgOptionsSavePage() : CPropertyPage(CDlgOptionsSavePage::IDD)
{
    m_saveOpenFiles = FALSE;  // tdg - 09/03/97
    m_showStartupDlg = FALSE; // DDO - 08/03/00
    m_szPermGraphs = "";
    m_szTempGraphs = "";
    m_szPermCurrLabel = "";
    m_szTempCurrLabel = "";
    CMainFrame * pMainWnd = (CMainFrame *)AfxGetMainWnd();

    m_showStartupDlg = pMainWnd->GetShowStartupDlg();              // DDO - 08/03/00
    m_saveOpenFiles = pMainWnd->GetSaveOpenFiles();                // tdg - 09/03/97
    //m_saveOnExit = pMainWnd->GetSaveOnExit();                    // DDO - 08/03/00 Don't need setting anymore
}

/////////////////////////////////////////////////////////////////////////////
// CDlgOptionsSavePage helper functions

void CDlgOptionsSavePage::DoDataExchange(CDataExchange * pDX)
{

    CPropertyPage::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_TEMPLAYOUTICON, m_TempIcon);
    DDX_Control(pDX, IDC_PERMLAYOUTICON, m_PermIcon);
    DDX_Check(pDX, ID_SAVEOPENFILES, m_saveOpenFiles);    // tdg - 09/03/97
    DDX_Check(pDX, ID_SHOWSTARTUPDLG, m_showStartupDlg);
    DDX_Text(pDX, IDC_STATIC_PERM, m_szPermGraphs);
    DDX_Text(pDX, IDC_STATIC_TEMP, m_szTempGraphs);
    DDX_Text(pDX, IDC_STATIC4, m_szPermCurrLabel);
    DDX_Text(pDX, IDC_STATIC5, m_szTempCurrLabel);
}

/***************************************************************************/
// CDlgOptionsSavePage::OnInitDialog Dialog initialization
//
// DDO - 08/03/00, Modified this a little in order to get rid of the bold
//                 font on several dialog static controls. Also added other
//                 static controls to the list of those whose font is
//                 changed.
/***************************************************************************/
BOOL CDlgOptionsSavePage::OnInitDialog()
{

    CPropertyPage::OnInitDialog();

    LOGFONT logFont;
    CFont * pFont = GetDlgItem(IDC_STATIC1)->GetFont(); // get the standard font
    pFont->GetObject(sizeof(LOGFONT), (void *)&logFont); // fill up logFont
    logFont.lfWeight = FW_NORMAL;                       // make font not bold
    m_Font.CreateFontIndirect(&logFont);                // create the modified font

    GetDlgItem(IDC_STATIC1)->SetFont(&m_Font);
    GetDlgItem(IDC_STATIC2)->SetFont(&m_Font);
    GetDlgItem(IDC_STATIC3)->SetFont(&m_Font);
    GetDlgItem(IDC_STATIC4)->SetFont(&m_Font);
    GetDlgItem(IDC_STATIC5)->SetFont(&m_Font);
    GetDlgItem(IDC_STATIC6)->SetFont(&m_Font);
    GetDlgItem(IDC_STATIC_PERM)->SetFont(&m_Font);
    GetDlgItem(IDC_STATIC_TEMP)->SetFont(&m_Font);

    ShowCurrentDefaultViews(TRUE);
    ShowCurrentDefaultViews(FALSE);
    SetStartDlgCheckHelp();
    GetDlgItem(ID_SHOWSTARTUPDLG)->EnableWindow(!m_saveOpenFiles);

    return TRUE;
}
/***************************************************************************/
// CDlgOptionsSavePage::ShowCurrentDefaultViews()
// Added by DDO - 08/07/00
/***************************************************************************/
void CDlgOptionsSavePage::SetStartDlgCheckHelp()
{

    int nShowState = (m_showStartupDlg && m_saveOpenFiles) ? SW_SHOWNORMAL : SW_HIDE;
    GetDlgItem(IDC_STATIC6)->ShowWindow(nShowState);
}

/***************************************************************************/
// CDlgOptionsSavePage::ShowCurrentDefaultViews()
// Added by DDO - 08/07/00
/***************************************************************************/
void CDlgOptionsSavePage::ShowCurrentDefaultViews(BOOL bPermanent)
{

    UINT     nIconID;
    UINT     nLayout;
    CStatic * pIcon;

    if (bPermanent)
    {
        nLayout = ((CMainFrame *)AfxGetMainWnd())->GetPermLayout();
        pIcon   = &m_PermIcon;
        m_szPermGraphs = ((CMainFrame *)AfxGetMainWnd())->GetPermGraphNames();
        m_szPermCurrLabel = (m_szPermGraphs.IsEmpty()) ? "" : "Current Setting:";
        m_szTempGraphs = "";
        m_szTempCurrLabel = "";
        GetDlgItem(IDC_TEMPLAYOUTICON)->ShowWindow(SW_HIDE);
    }
    else
    {
        nLayout = ((CMainFrame *)AfxGetMainWnd())->GetTempLayout();
        pIcon = &m_TempIcon;
        m_szTempGraphs = ((CMainFrame *)AfxGetMainWnd())->GetTempGraphNames();
        m_szTempCurrLabel = (m_szTempGraphs.IsEmpty()) ? "" : "Current Setting:";
        GetDlgItem(IDC_TEMPLAYOUTICON)->ShowWindow(SW_SHOWNORMAL);
    }

    switch (nLayout)
    {
    case ID_LAYOUT_1:
        nIconID = IDI_LAYOUT1;
        break;
    case ID_LAYOUT_2A:
        nIconID = IDI_LAYOUT2A;
        break;
    case ID_LAYOUT_2B:
        nIconID = IDI_LAYOUT2B;
        break;
    case ID_LAYOUT_2C:
        nIconID = IDI_LAYOUT2C;
        break;
    case ID_LAYOUT_3A:
        nIconID = IDI_LAYOUT3A;
        break;
    case ID_LAYOUT_3B:
        nIconID = IDI_LAYOUT3B;
        break;
    case ID_LAYOUT_3C:
        nIconID = IDI_LAYOUT3C;
        break;
    case ID_LAYOUT_4A:
        nIconID = IDI_LAYOUT4A;
        break;
    case ID_LAYOUT_4B:
        nIconID = IDI_LAYOUT4B;
        break;
    case ID_LAYOUT_4C:
        nIconID = IDI_LAYOUT4C;
        break;
    case ID_LAYOUT_5:
        nIconID = IDI_LAYOUT5;
        break;
    case ID_LAYOUT_6A:
        nIconID = IDI_LAYOUT6A;
        break;
    case ID_LAYOUT_6B:
        nIconID = IDI_LAYOUT6B;
        break;
    default:
        nIconID = 0;
    }

    if (nIconID)
    {
        pIcon->SetIcon(((CSaApp *)AfxGetApp())->LoadIcon(nIconID));
    }
    UpdateData(FALSE);
}

/***************************************************************************/
// CDlgOptionsSavePage::OnSaveTempDefaultTemplate()
/***************************************************************************/
void CDlgOptionsSavePage::OnSaveTempDefaultTemplate()
{

    ((CMainFrame *)AfxGetMainWnd())->OnSetDefaultGraphs(FALSE);
    ShowCurrentDefaultViews(FALSE);
}

/***************************************************************************/
// CDlgOptionsSavePage::OnSavePermDefaultTemplate
/***************************************************************************/
void CDlgOptionsSavePage::OnSavePermDefaultTemplate()
{

    ((CMainFrame *)AfxGetMainWnd())->OnSetDefaultGraphs(TRUE);
    ShowCurrentDefaultViews(TRUE);
    ((CDlgToolsOptions *)GetParent())->ApplyNow();
}

/***************************************************************************/
/***************************************************************************/
void CDlgOptionsSavePage::OnReopenFiles()
{

    UpdateData();
    SetStartDlgCheckHelp();
    GetDlgItem(ID_SHOWSTARTUPDLG)->EnableWindow(!m_saveOpenFiles);
}

/***************************************************************************/
/***************************************************************************/
void CDlgOptionsSavePage::OnShowsStartupDlg()
{

    UpdateData();
    SetStartDlgCheckHelp();
}

//###########################################################################
// CDlgOptionsAudioPage property page
// Displays all important controls to change the saving behaviour of this
// application.

BEGIN_MESSAGE_MAP(CDlgOptionsAudioPage, CPropertyPage)
END_MESSAGE_MAP()

CDlgOptionsAudioPage::CDlgOptionsAudioPage() : CPropertyPage(CDlgOptionsAudioPage::IDD)
{
    CMainFrame * pMainWnd = (CMainFrame *)AfxGetMainWnd();
    m_bShowAdvancedAudio = pMainWnd->GetShowAdvancedAudio();

}

void CDlgOptionsAudioPage::DoDataExchange(CDataExchange * pDX)
{

    CPropertyPage::DoDataExchange(pDX);
    DDX_Check(pDX, IDC_SHOW_ADVANCED_OPTIONS, m_bShowAdvancedAudio);
}

BOOL CDlgOptionsAudioPage::OnInitDialog()
{

    CPropertyPage::OnInitDialog();
    return TRUE;
}

//###########################################################################
// CDlgToolsOptions property sheet
// Displays all important controls to customize this application.

IMPLEMENT_DYNAMIC(CDlgToolsOptions, CPropertySheet)

BEGIN_MESSAGE_MAP(CDlgToolsOptions, CPropertySheet)
    ON_WM_CREATE()
    ON_COMMAND(ID_APPLY_NOW, OnApplyNow)
    ON_COMMAND(IDHELP, OnHelpToolsOptions)
END_MESSAGE_MAP()

/***************************************************************************/
// CDlgToolsOptions::CDlgToolsOptions Constructor
/***************************************************************************/
CDlgToolsOptions::CDlgToolsOptions(LPCTSTR pszCaption, CWnd * pParent) :
    CPropertySheet(pszCaption, pParent, 0)
{

    AddPage(&m_dlgViewPage);
    AddPage(&m_dlgColorPage);
    AddPage(&m_dlgFontPage);
    AddPage(&m_dlgSavePage);
    AddPage(&m_dlgAudioPage);
}

/***************************************************************************/
// CDlgToolsOptions::DoDataExchange Data exchange
/***************************************************************************/
void CDlgToolsOptions::DoDataExchange(CDataExchange * pDX)
{

    CPropertySheet::DoDataExchange(pDX);
}

/***************************************************************************/
// CDlgToolsOptions::OnCreate Dialog creation
/***************************************************************************/
int CDlgToolsOptions::OnCreate(LPCREATESTRUCT lpCreateStruct)
{

    if (CPropertySheet::OnCreate(lpCreateStruct) == -1)
    {
        return -1;
    }
    return 0;
}

/***************************************************************************/
// CDlgToolsOptions::OnInitDialog Dialog initialization
/***************************************************************************/
BOOL CDlgToolsOptions::OnInitDialog()
{

    CPropertySheet::OnInitDialog();
    ChangeButtons();
    return TRUE;
}

/***************************************************************************/
// CDlgToolsOptions::ChangeButtons Rearrange buttons on the dialog
/***************************************************************************/
void CDlgToolsOptions::ChangeButtons()
{

    CWnd * pWndOK = GetDlgItem(IDOK); // get pointers to the button objects
    CWnd * pWndCancel = GetDlgItem(IDCANCEL);
    CWnd * pWndApply = GetDlgItem(ID_APPLY_NOW);
    CRect rBtnOK, rBtnCancel, rBtnApply, rBtnHelp;

    // get button coordinates
    pWndOK->GetWindowRect(rBtnOK);
    ScreenToClient(rBtnOK);
    pWndCancel->GetWindowRect(rBtnCancel);
    ScreenToClient(rBtnCancel);
    pWndApply->GetWindowRect(rBtnApply);
    ScreenToClient(rBtnApply);

    // move buttons
    rBtnHelp = rBtnApply;
    int nOffset = rBtnOK.left - rBtnCancel.left;
    rBtnOK.OffsetRect(nOffset, 0);
    pWndOK->MoveWindow(rBtnOK);
    rBtnCancel.OffsetRect(nOffset, 0);
    pWndCancel->MoveWindow(rBtnCancel);
    rBtnApply.OffsetRect(nOffset, 0);
    pWndApply->MoveWindow(rBtnApply);

    // create Help button
    m_cHelp.Create(_T("&Help"), WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON, rBtnHelp, this, IDHELP);
    m_cHelp.SetFont(pWndOK->GetFont());
}

/***************************************************************************/
// CDlgToolsOptions::OnApplyNow Button apply hit
// The main frame window has to be informed, because it has to init some
// action concerning the mainframe.
/***************************************************************************/
void CDlgToolsOptions::OnApplyNow()
{

    GetActivePage()->UpdateData(TRUE); // retrieve data

    CMainFrame * pMainWnd = (CMainFrame *)AfxGetMainWnd();
    pMainWnd->SetToolSettings(GetSettings());
    AfxGetMainWnd()->SendMessage(WM_USER_APPLY_TOOLSOPTIONS, 0, 0);
    GetActivePage()->SetModified(FALSE);
    SendMessage(PSM_CANCELTOCLOSE, 0, 0L);
}

/***************************************************************************/
// CDlgToolsOptions::OnHelpToolsOptions Call Tools Options help
/***************************************************************************/
void CDlgToolsOptions::OnHelpToolsOptions()
{

    // create the pathname
    long nActiveIndex = GetActiveIndex();
    CString szPath = AfxGetApp()->m_pszHelpFilePath;
    szPath += "::/User_Interface/Menus/Tools/Options/";
    switch (nActiveIndex)
    {
    case 0:
        szPath += "View_tab_Options.htm";
        break;
    case 1:
        szPath += "Colors_tab_Options.htm";
        break;
    case 2:
        szPath += "Fonts_tab_Options.htm";
        break;
    case 3:
        szPath += "Startup_tab_Options.htm";
        break;
    default:
        szPath += "Options_overview.htm";
    }

    ::HtmlHelp(NULL, szPath, HH_DISPLAY_TOPIC, NULL);
}

CToolSettings CDlgToolsOptions::GetSettings()
{

    // view page
    CToolSettings settings;
    settings.m_bStatusbar = m_dlgViewPage.m_bStatusbar;
    settings.m_bToolbar = m_dlgViewPage.m_bToolbar;
    settings.m_bScrollZoom = m_dlgViewPage.m_bScrollZoom;
    settings.m_nCaptionStyle = m_dlgViewPage.m_nCaptionStyle;
    settings.m_bXGrid = m_dlgViewPage.m_bXGrid;
    settings.m_bYGrid = m_dlgViewPage.m_bYGrid;
    settings.m_nXStyleIndex = m_dlgViewPage.m_nXStyleIndex;
    settings.m_nYStyleIndex = m_dlgViewPage.m_nYStyleIndex;
    settings.m_nCursorAlignment = m_dlgViewPage.m_nCursorAlignment;
    settings.m_nPitchMode = m_dlgViewPage.m_nPitchMode;
    settings.m_nPosMode = m_dlgViewPage.m_nPosMode;
    settings.m_bToneAbove = m_dlgViewPage.m_bToneAbove;
    settings.m_nGraphUpdateMode = m_dlgViewPage.m_nGraphUpdateMode;
    settings.m_bAnimate = m_dlgViewPage.m_bAnimate;
    settings.m_nAnimationRate = m_dlgViewPage.m_nAnimationRate;
    settings.m_bTaskbar = m_dlgViewPage.m_bTaskbar;
    settings.m_nDlgXStyle = m_dlgViewPage.IndexToStyle[m_dlgViewPage.m_nXStyleIndex];
    settings.m_nDlgYStyle = m_dlgViewPage.IndexToStyle[m_dlgViewPage.m_nYStyleIndex];


    // color page
    settings.m_bColorsChanged = m_dlgColorPage.m_bColorsChanged;
    settings.m_cColors = m_dlgColorPage.m_cColors;
    settings.m_nGraphSelect = m_dlgColorPage.m_nGraphSelect;
    settings.m_nAnnotationSelect = m_dlgColorPage.m_nAnnotationSelect;
    settings.m_nScaleSelect = m_dlgColorPage.m_nScaleSelect;
    settings.m_nOverlaySelect = m_dlgColorPage.m_nOverlaySelect;

    //font page
    settings.m_bFontChanged = m_dlgFontPage.m_bFontChanged;
    settings.m_GraphFonts.RemoveAll();
    for (int i=0; i<m_dlgFontPage.m_GraphFonts.GetCount(); i++)
    {
        settings.m_GraphFonts.Add(m_dlgFontPage.m_GraphFonts.GetAt(i));
    }
    settings.m_GraphFontSizes.RemoveAll();
    for (int i=0; i<m_dlgFontPage.m_GraphFontSizes.GetCount(); i++)
    {
        settings.m_GraphFontSizes.Add(m_dlgFontPage.m_GraphFontSizes.GetAt(i));
    }
    settings.m_bUseUnicodeEncoding = m_dlgFontPage.m_bUseUnicodeEncoding;

    // save page
    settings.m_saveOpenFiles = m_dlgSavePage.m_saveOpenFiles;
    settings.m_showStartupDlg = m_dlgSavePage.m_showStartupDlg;
    settings.m_szPermGraphs = m_dlgSavePage.m_szPermGraphs;
    settings.m_szTempGraphs = m_dlgSavePage.m_szTempGraphs;
    settings.m_szPermCurrLabel = m_dlgSavePage.m_szPermCurrLabel;
    settings.m_szTempCurrLabel = m_dlgSavePage.m_szTempCurrLabel;

    // audio page
    settings.m_bShowAdvancedAudio = m_dlgAudioPage.m_bShowAdvancedAudio;
    return settings;
}