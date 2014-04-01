/////////////////////////////////////////////////////////////////////////////
// sa_menu.h:
// Interface of the CLayoutMenu class
// Author: Urs Ruchti
// copyright 1996 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////
#ifndef _SA_MENU_H
#define _SA_MENU_H

//###########################################################################
// CLayoutMenu custom menu

class CLayoutMenu : public CMenu
{

    // Construction/destruction/creation
public:
    CLayoutMenu();
    virtual ~CLayoutMenu();

    // Attributes
private:

    // Operations
public:
    void AppendLayoutMenuItem(UINT nID, int nNumber);
    virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMIS);
    virtual void DrawItem(LPDRAWITEMSTRUCT lpDIS);
};

#endif //_SA_MENU_H
