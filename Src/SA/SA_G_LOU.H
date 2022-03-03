/////////////////////////////////////////////////////////////////////////////
// sa_g_lou.h:
// Interface of the CPlotLoudness class
// Author: Urs Ruchti
// copyright 1996 JAARS Inc. SIL
/////////////////////////////////////////////////////////////////////////////
#ifndef _SA_G_LOU_H
#define _SA_G_LOU_H

#include "sa_context.h"

class CPlotLoudness : public CPlotWnd {
    DECLARE_DYNCREATE(CPlotLoudness)

public:
    CPlotLoudness();
    virtual CPlotWnd * NewCopy(void);
    virtual void CopyTo(CPlotWnd * pTarg);

    virtual void OnDraw( CDC * pDC, CRect rWnd, CRect rClip, SaContext& context);

protected:
    DECLARE_MESSAGE_MAP()
};

#endif
