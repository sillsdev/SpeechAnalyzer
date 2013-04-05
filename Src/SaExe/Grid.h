#ifndef _GRID_H
#define _GRID_H

// grid settings

class CObjectOStream;
class CObjectIStream;

class CGrid
{
public:
    void WriteProperties(CObjectOStream & obs);
    BOOL ReadProperties(CObjectIStream & obs);
    void Init();

    BOOL bXGrid;            // TRUE, if x-grid enabled
    BOOL bYGrid;            // TRUE, if y-grid enabled
    int nXStyle;            // x-grid style
    int nYStyle;            // y-grid style
};

#endif
