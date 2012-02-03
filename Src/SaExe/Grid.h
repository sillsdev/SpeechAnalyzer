#ifndef _GRID_H
#define _GRID_H

// grid settings

class Object_ostream;
class Object_istream;

class Grid
{
public:
  BOOL         bXGrid;            // TRUE, if x-grid enabled
  BOOL         bYGrid;            // TRUE, if y-grid enabled
  int          nXStyle;           // x-grid style
  int          nYStyle;           // y-grid style

  void WriteProperties(Object_ostream& obs);
  BOOL ReadProperties(Object_istream& obs);

  void Init();
};

#endif //_GRID_H
