/////////////////////////////////////////////////////////////////////////////
// sa_annot.h:
// Interface of the CGlossNatWnd classes.
//
// Author: Urs Ruchti
// copyright 2014 JAARS Inc. SIL
//
/////////////////////////////////////////////////////////////////////////////
#ifndef GLOSS_NAT_WND_H
#define GLOSS_NAT_WND_H

#include "SA_Annot.h"

class CGlossNatWnd : public CAnnotationWnd {
public:
    CGlossNatWnd(int nIndex);
    virtual void OnDraw(CDC * pDC, const CRect & printRect);
};

#endif


