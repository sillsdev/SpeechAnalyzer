/////////////////////////////////////////////////////////////////////////////
// sa_menu.cpp:
// Implementation of the CLayoutMenu class.
// Author: Urs Ruchti
// copyright 1996 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "sa_menu.h"
#include "sa_view.h"
#include "sa_dlwnd.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

static void DrawLayoutItem(LPDRAWITEMSTRUCT lpDIS, BOOL bMenu);
// included here because 1.52c linker bug... duplicated from sa_view.h
static int GetNumberOfGraphsInLayout(UINT nLayoutID);

// Defines
#define LAYOUT_BOX_WIDTH     52
#define LAYOUT_BOX_HEIGHT    24
#define LAYOUT_DRAW_WIDTH    32

//###########################################################################
// CLayoutMenu

/////////////////////////////////////////////////////////////////////////////
// CLayoutMenu construction/destruction/creation

/***************************************************************************/
// CLayoutMenu::CLayoutMenu Constructor
/***************************************************************************/
CLayoutMenu::CLayoutMenu()
{
  VERIFY(CreateMenu());
  // append the items
  for (int nLoop = 0; nLoop <= (ID_LAYOUT_LAST - ID_LAYOUT_FIRST); nLoop++)
  {
    AppendLayoutMenuItem(ID_LAYOUT_FIRST + nLoop, nLoop);
  }
}

/***************************************************************************/
// CLayoutMenu::CLayoutMenu Destructor
/***************************************************************************/
CLayoutMenu::~CLayoutMenu()
{
  DestroyMenu();
  ASSERT(m_hMenu == NULL);
}

/////////////////////////////////////////////////////////////////////////////
// CLayoutMenu helper functions

/***************************************************************************/
// CLayoutMenu::AppendLayoutMenuItem Appends a layout item
/***************************************************************************/
void CLayoutMenu::AppendLayoutMenuItem(UINT nID, int nNumber)
{
  VERIFY(AppendMenu(MF_ENABLED | MF_OWNERDRAW, nID, (LPCTSTR)nNumber));
}

/***************************************************************************/
// CLayoutMenu::MeasureItem Define the item size
/***************************************************************************/
void CLayoutMenu::MeasureItem(LPMEASUREITEMSTRUCT lpMIS)
{
  // all items are of fixed size
  lpMIS->itemWidth = LAYOUT_BOX_WIDTH;
  if(lpMIS->itemID == ID_LAYOUT_4A)
    lpMIS->itemHeight = LAYOUT_BOX_HEIGHT - 2;
  else
    lpMIS->itemHeight = LAYOUT_BOX_HEIGHT;
}

/***************************************************************************/
// CLayoutMenu::DrawItem Draw the items
/***************************************************************************/
void CLayoutMenu::DrawItem(LPDRAWITEMSTRUCT lpDIS)
{
  DrawLayoutItem(lpDIS, TRUE);
}

//###########################################################################
// CLayoutListBox

/////////////////////////////////////////////////////////////////////////////
// CLayoutListBox construction/destruction/creation

/***************************************************************************/
// CLayoutListBox::CLayoutListBox Constructor
/***************************************************************************/
void CLayoutListBox::SelectLayoutWithCount(int number)
{
  ResetContent();

  if(number != -1)
  {
    if(number < 1) number = 1;
    if(number > 6) number = 6;
  }

  m_number = number;

  int nSelect = -1;

  for(int nLoop=0;GetLayoutFromSelection(nLoop);nLoop++)
  {
    AppendLayoutItem(GetLayoutFromSelection(nLoop) - ID_LAYOUT_FIRST);
    if(GetLayoutFromSelection(nLoop) == GetPreferredLayout())
      nSelect = GetCount() - 1;
  }
  ASSERT(GetCount());

  SetCurSel(nSelect);
}

/***************************************************************************/
// CLayoutListBox::GetLayoutFromSelection get layout ID from selection index
/***************************************************************************/
CLayoutListBox::GetLayoutFromSelection(int nIndex)
{
  const int one[] = { ID_LAYOUT_1,0 };
  const int two[] = { ID_LAYOUT_2A,ID_LAYOUT_2B,ID_LAYOUT_2C,0 };
  const int three[] = { ID_LAYOUT_3A,ID_LAYOUT_3B,ID_LAYOUT_3C,0 };
  const int four[] = { ID_LAYOUT_4A,ID_LAYOUT_4B,ID_LAYOUT_4C,0 };
  const int five[] = { ID_LAYOUT_5,0 };
  const int six[] = { ID_LAYOUT_6A,ID_LAYOUT_6B,0 };
  const int *any[] = { one,two,three,four,five,six };

  if(nIndex == -1) return GetPreferredLayout();

  if(m_number <= 6 && m_number >= 1 )
  {
    for(int nLoop = 0; nLoop < nIndex; nLoop++)
      if(any[m_number-1][nIndex] == 0)
        return 0;
    return any[m_number-1][nIndex];
  }
  return nIndex + ID_LAYOUT_FIRST;
}


/***************************************************************************/
// CLayoutListBox::GetLayoutFromSelection get layout ID from selection index
/***************************************************************************/
int CLayoutListBox::GetPreferredLayout(int number)
{
  const int preferred[] =
  {
    ID_LAYOUT_1,ID_LAYOUT_2A,ID_LAYOUT_3A,ID_LAYOUT_4B,ID_LAYOUT_5,ID_LAYOUT_6B,0
  };

  if(number <= 6 && number >= 1 )
    return preferred[number-1];
  else
    return -1;
}


/***************************************************************************/
// CLayoutListBox::CLayoutListBox Destructor
/***************************************************************************/
CLayoutListBox::~CLayoutListBox()
{
  //DestroyMenu();
  //ASSERT(m_hMenu == NULL);
}

/////////////////////////////////////////////////////////////////////////////
// CLayoutListBox helper functions

/***************************************************************************/
// CLayoutListBox::AppendLayoutMenuItem Appends a layout item
/***************************************************************************/
void CLayoutListBox::AppendLayoutItem(int nNumber)
{
  AddString((LPCTSTR)nNumber);
}

/***************************************************************************/
// CLayoutListBox::MeasureItem Define the item size
/***************************************************************************/
void CLayoutListBox::MeasureItem(LPMEASUREITEMSTRUCT lpMIS)
{
  // all items are of fixed size
  lpMIS->itemWidth = LAYOUT_BOX_WIDTH;
  lpMIS->itemHeight = LAYOUT_BOX_HEIGHT + 12;
}

/***************************************************************************/
// CLayoutListBox::DrawItem Draw the items
/***************************************************************************/
void CLayoutListBox::DrawItem(LPDRAWITEMSTRUCT lpDIS)
{
  DrawLayoutItem(lpDIS, FALSE);
}

/***************************************************************************/
// Draw the items for CLayoutMenu & CLayoutListBox
/***************************************************************************/
static void DrawLayoutItem(LPDRAWITEMSTRUCT lpDIS, BOOL bMenu)
{
  CDC* pDC = CDC::FromHandle(lpDIS->hDC);
  // prepare text
  CString szText;
  if(lpDIS->itemData >= 0 && lpDIS->itemData <= ID_LAYOUT_LAST - ID_LAYOUT_FIRST)
  {
    const char *szLabels[] = {"1","2a","2b","2c","3a","3b","3c","4a","4b","4c","5","6a","6b"};
    szText = szLabels[lpDIS->itemData];
  }
  else
  {
    szText = "?";
  }

  // create pens, brushes and text colors appropriate to the item state
  COLORREF oldTextColor;
  CPen penLightGray, penDarkGray, penLightText, penDarkText;
  CBrush br;
  int nOldBkMode = pDC->SetBkMode(TRANSPARENT); // text should not change background
  if (lpDIS->itemState & ODS_GRAYED)
  {
    // grayed item
    penLightGray.CreatePen(PS_SOLID, 1, GetSysColor(COLOR_BTNHIGHLIGHT));
    penDarkGray.CreatePen(PS_SOLID, 1, GetSysColor(COLOR_BTNSHADOW));
    penLightText.CreatePen(PS_SOLID, 1, GetSysColor(COLOR_BTNHIGHLIGHT));
    penDarkText.CreatePen(PS_SOLID, 1, GetSysColor(COLOR_BTNSHADOW));
    if(bMenu)
      br.CreateSolidBrush(GetSysColor(COLOR_MENU));
    else
      br.CreateSolidBrush(GetSysColor(COLOR_WINDOW));
    oldTextColor = pDC->SetTextColor(GetSysColor(COLOR_GRAYTEXT));
  }
  else
  {
    if (lpDIS->itemState & ODS_SELECTED)
    {
      // selected item
      penLightGray.CreatePen(PS_SOLID, 1, GetSysColor(COLOR_BTNHIGHLIGHT));
      penDarkGray.CreatePen(PS_SOLID, 1, GetSysColor(COLOR_BTNSHADOW));
      penLightText.CreatePen(PS_SOLID, 1, GetSysColor(COLOR_HIGHLIGHTTEXT));
      penDarkText.CreatePen(PS_SOLID, 1, GetSysColor(COLOR_HIGHLIGHTTEXT));
      br.CreateSolidBrush(GetSysColor(COLOR_HIGHLIGHT));
      oldTextColor = pDC->SetTextColor(GetSysColor(COLOR_HIGHLIGHTTEXT));
    }
    else
    {
      // unselected (normal) item
      penLightGray.CreatePen(PS_SOLID, 1, GetSysColor(COLOR_BTNHIGHLIGHT));
      penDarkGray.CreatePen(PS_SOLID, 1, GetSysColor(COLOR_BTNSHADOW));
      if(bMenu)
      {
        penLightText.CreatePen(PS_SOLID, 1, GetSysColor(COLOR_MENUTEXT));
        penDarkText.CreatePen(PS_SOLID, 1, GetSysColor(COLOR_MENUTEXT));
        br.CreateSolidBrush(GetSysColor(COLOR_MENU));
      }
      else
      {
        penLightText.CreatePen(PS_SOLID, 1, GetSysColor(COLOR_WINDOWTEXT));
        penDarkText.CreatePen(PS_SOLID, 1, GetSysColor(COLOR_WINDOWTEXT));
        br.CreateSolidBrush(GetSysColor(COLOR_WINDOW));
      }

      oldTextColor = pDC->SetTextColor(GetSysColor(COLOR_MENUTEXT));
    }
  }

  // draw the items
  CRect rDraw(lpDIS->rcItem.right - LAYOUT_DRAW_WIDTH - 2, lpDIS->rcItem.top + 1,
    lpDIS->rcItem.right - 2, lpDIS->rcItem.bottom - 2);
  CRect rText = lpDIS->rcItem;
  rText.right = rDraw.left - 3;
  CPen* pOldPen = pDC->SelectObject(&penDarkGray);
  if ((lpDIS->itemAction & ODA_DRAWENTIRE)
    || (lpDIS->itemAction & ODA_SELECT))
  {
    // item is to draw normal, selected or grayed out
    pDC->FillRect(&lpDIS->rcItem, &br); // fill the background
    // draw the number
    pDC->DrawText(szText, szText.GetLength(), rText, DT_SINGLELINE | DT_RIGHT | DT_VCENTER | DT_NOCLIP);
    // draw the 3D frame
    pDC->MoveTo(rDraw.left, rDraw.bottom);
    pDC->LineTo(rDraw.left, rDraw.top);
    pDC->LineTo(rDraw.right, rDraw.top);
    pDC->SelectObject(&penLightGray);
    pDC->LineTo(rDraw.right, rDraw.bottom);
    pDC->LineTo(rDraw.left, rDraw.bottom);
    // draw the inside frame
    rDraw.InflateRect(-1, -1);
    pDC->SelectObject(&penDarkText);
    pDC->MoveTo(rDraw.left, rDraw.bottom);
    pDC->LineTo(rDraw.left, rDraw.top);
    pDC->LineTo(rDraw.right, rDraw.top);
    pDC->SelectObject(&penLightText);
    pDC->LineTo(rDraw.right, rDraw.bottom);
    pDC->LineTo(rDraw.left, rDraw.bottom);
    rDraw.top += 1;
    pDC->SelectObject(&penDarkText);
    pDC->MoveTo(rDraw.left, rDraw.top);
    pDC->LineTo(rDraw.right, rDraw.top);
    // draw the GraphSubRect border lines
    pDC->SelectObject(&penDarkText);
    for(int nGraph = 0; nGraph < CSaView::GetNumberOfGraphsInLayout(ID_LAYOUT_FIRST + (int)lpDIS->itemData); nGraph++)
    {
      CRect rGraph;

      CSaView::GetGraphSubRect(ID_LAYOUT_FIRST + (int)lpDIS->itemData, &rDraw, &rGraph, nGraph);

      pDC->MoveTo(rGraph.left, rGraph.bottom);
      pDC->LineTo(rGraph.left, rGraph.top);
      pDC->LineTo(rGraph.right, rGraph.top);
      pDC->LineTo(rGraph.right, rGraph.bottom);
      pDC->LineTo(rGraph.left, rGraph.bottom);
    }

    // put the checkmark (a kind of transparent bitmap), not if grayed
    if (((lpDIS->itemState & ODS_CHECKED) || ((lpDIS->itemState & ODS_SELECTED) && !bMenu)) && !(lpDIS->itemState & ODS_GRAYED))
    {
      DWORD dwCheckSize = GetMenuCheckMarkDimensions(); // checkmark dimensions
      CDC* pDisplayMemDC;
      CBitmap* pbmp, *pbmpOld;
      // create the memory DC
      pDisplayMemDC = new CDC;
      pDisplayMemDC->CreateCompatibleDC(pDC);
      // load and select the bitmap
      pbmp = new CBitmap;
      pbmp->LoadOEMBitmap(OBM_CHECK);
      pbmpOld = (CBitmap*)pDisplayMemDC->SelectObject(pbmp);

      COLORREF crOldBk = pDC->SetBkColor((lpDIS->itemState & ODS_SELECTED) ?
        GetSysColor(COLOR_HIGHLIGHT) : GetSysColor(COLOR_MENU));
      pDC->SetTextColor((lpDIS->itemState & ODS_SELECTED) ?
        GetSysColor(COLOR_HIGHLIGHTTEXT) : GetSysColor(COLOR_MENUTEXT));
      // put it to the screen
      pDC->BitBlt(lpDIS->rcItem.left + 2,
        lpDIS->rcItem.top +
        (lpDIS->rcItem.bottom - lpDIS->rcItem.top - HIWORD(dwCheckSize)) / 2,
        LOWORD(dwCheckSize), HIWORD(dwCheckSize), pDisplayMemDC, 0, 0, SRCCOPY );
      // restore
      delete pDisplayMemDC->SelectObject(pbmpOld);
      delete pDisplayMemDC;
      pDC->SetBkColor(crOldBk);
    }
  }
  // restore previous settings
  pDC->SelectObject(pOldPen);
  pDC->SetTextColor(oldTextColor);
  pDC->SetBkMode(nOldBkMode);
}

