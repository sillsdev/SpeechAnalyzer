#include "stdafx.h"
#include "objectostream.h"
#include "objectistream.h"
#include "grid.h"

static LPCSTR psz_grid   = "grid";
static LPCSTR psz_xgrid   = "xgrid";
static LPCSTR psz_ygrid   = "ygrid";
static LPCSTR psz_xstyle = "xstyle";
static LPCSTR psz_ystyle = "ystyle";

void CGrid::WriteProperties(CObjectOStream & obs)
{
    obs.WriteBeginMarker(psz_grid);
    obs.WriteNewline();

    // write out properties
    obs.WriteBool(psz_xgrid,  bXGrid);
    obs.WriteBool(psz_ygrid,  bYGrid);
    obs.WriteInteger(psz_xstyle, nXStyle);
    obs.WriteInteger(psz_ystyle, nYStyle);

    obs.WriteEndMarker(psz_grid);
}

BOOL CGrid::ReadProperties(CObjectIStream & obs)
{
    if (!obs.bAtBackslash() || !obs.bReadBeginMarker(psz_grid))
    {
        return FALSE;
    }

    while (!obs.bAtEnd())
    {
        if (obs.bReadBool(psz_xgrid, bXGrid));
        else if (obs.bReadBool(psz_ygrid, bYGrid));
        else if (obs.bReadInteger(psz_xstyle, nXStyle));
        else if (obs.bReadInteger(psz_ystyle, nYStyle));
        else if (obs.bEnd(psz_grid))
        {
            break;
        }
    }

    return TRUE;
}


void CGrid::Init()
{
    // init the gridlines
    bXGrid  = TRUE; // x-grid enabled
    bYGrid  = TRUE; // y-grid enabled
    nXStyle = 0;   // x-grid style dot
    nYStyle = 0;   // y-grid style dot
}
