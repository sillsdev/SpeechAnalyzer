// ZRECT.H

/////////////////////////////////////////////////////////////////////////////
//                   ZRECT.H -- Header File for zRECT Class
//
//  ZGRAF C++ Multi-Platform Graph Toolkit v. 1.41, (c) 1996, ZGRAF Software
//
//  [This Class Was Borrowed Very Heavily From the MFC "CRect" Class]
/////////////////////////////////////////////////////////////////////////////


#ifndef _ZRECT_H_
#define _ZRECT_H_

#include <math.h>
#include "toolkit.h"
///////////////////////////////////////////////////////////////
// This Class Is Used to Manipulate and Perform Operations on
//   Rectangles
///////////////////////////////////////////////////////////////

class zRECT : public tagRECT
{
public:
  //
  // Constructor
  //
  inline zRECT()
  { left = 0; top = 0; right = 0; bottom = 0; }

  inline zRECT(int l, int t, int r, int b)
  { left = l; top = t; right = r; bottom = b; }

  inline zRECT(const RECT& RSource)
  { zCopyRect(this, &RSource); }

  inline zRECT(const RECT *lpRSource)
  { zCopyRect(this, lpRSource); }

  inline zRECT(POINT topLeft, POINT bottomRight)
  {
    left   = topLeft.x;
    top    = topLeft.y;
    right  = bottomRight.x;
    bottom = bottomRight.y;
  }

  inline INT Height() const
  {
    // Height of Rectangle
    return (abs(top - bottom));
  }

  inline INT Width() const
  {
    // Width of Rectangle
    return (abs(left - right));
  }

  inline POINT Center()
  {
    // Gets Center Point of Rectangle
    POINT ptCenter;

    ptCenter.x = (left + right) / 2;
    ptCenter.y = (top  + bottom)/ 2;

    return (ptCenter);
  }


  inline operator RECT *()
  { return this; }

  inline operator const RECT *() const
  { return this; }

  inline BOOL PtInRect(POINT point) const
  { return (left < point.x)  &&  (right  > point.x)  &&
  (top  < point.y)  &&  (bottom > point.y); }

  inline void SetRect(int x1, int y1, int x2, int y2)
  { zSetRect(this, x1, y1, x2, y2); }

  inline void SetRect(POINT topLeft, POINT bottomRight)
  { zSetRect(this, topLeft.x, topLeft.y, bottomRight.x, bottomRight.y); }

  inline void CopyRect(const RECT *lpRect)
  { zCopyRect(this, lpRect); }

  inline BOOL EqualRect(const RECT *lpRect)
  { return zEqualRect(this, lpRect); }

  inline void operator=(const RECT& srcRECT)
  { zCopyRect(this, &srcRECT); }

  inline BOOL operator==(const RECT& rect)
  { return zEqualRect(this, &rect); }

  inline BOOL operator!=(const RECT& rect)
  { return ! zEqualRect(this, &rect); }

  inline BOOL IsValid()
  {
    // Returns TRUE for a Good Rectangle

    return ((right != 0) ? TRUE : FALSE);
  }

  void SetComponents(INT &x1, INT &y1, INT &x2, INT &y2)
  {
    // Sets the Four Component Values from a Rectangle

    x1 = left;
    x2 = right;
    y1 = top;
    y2 = bottom;
  }

  void Normalize()
  {
    //
    // Normalizes a Rectangle, so that
    //   [left] <= [right]  and  [top] <= [bottom]
    //
    if (right < left)
      SWAP(left, right);

    if (top > bottom)
      SWAP(top, bottom);
  }

  void ScaleViaPercentageOffsets(zRECT  RInput,
    double LeftPercent,
    double TopPercent,
    double RightPercent,
    double BottomPercent)
  {
    // Scales a Rectangle Down By An Amount
    //   Based Upon the Percentage of the Rectangle Components Given

    left   = RInput.left + zRound(LeftPercent   * RInput.Width());
    top    = RInput.top  + zRound(TopPercent    * RInput.Height());
    right  = RInput.left + zRound(RightPercent  * RInput.Width());
    bottom = RInput.top  + zRound(BottomPercent * RInput.Height());
  }

  void ScaleViaAbsoluteOffsets(zRECT  RInput,
    double Left,
    double Top,
    double Right,
    double Bottom)
  {
    // Scales a Rectangle Down By Absolute Offsets

    left   = RInput.left + zRound(Left);
    top    = RInput.top  + zRound(Top);
    right  = RInput.left + zRound(Right);
    bottom = RInput.top  + zRound(Bottom);
  }

  void SetTo(double x1, double y1,
    double x2, double y2)
  {
    left   = zRound(x1);
    top    = zRound(y1);
    right  = zRound(x2);
    bottom = zRound(y2);
  }

  void FromCenterPt(POINT ptCenter, INT w, INT h)
  {
    // Sets a Rectangle from a Center Point, and
    //   Height, Width Measurements

    left   = ptCenter.x - w/2;
    top    = ptCenter.y - h/2;
    right  = ptCenter.x + w/2;
    bottom = ptCenter.y + h/2;
  }

  void FromCenterXY(INT xCenter, INT yCenter, INT w, INT h)
  {
    // Sets a Rectangle from a Center Point, and
    //   Height, Width Measurements

    left   = xCenter - w/2;
    top    = yCenter - h/2;
    right  = xCenter + w/2;
    bottom = yCenter + h/2;
  }

  void OffsetBy(RECT R1)
  {
    left   += R1.left;
    right  += R1.right;
    top    += R1.top;
    bottom += R1.bottom;
  }

  void Shrink(INT delta_x, INT delta_y)
  {
    // Shrinks a Rectangle by [delta_x] in the
    //  X-direction and [delta_y] in the Y-direction
    //  Expands Rectangle if the delta values are negative

    left   += delta_x;
    top    += delta_y;
    right  -= delta_x;
    bottom -= delta_y;
  }

  static INT zRound(double d)
  {
    //
    // This Function Is Used For Rounding Purposes.  It Returns the closest
    // integer to [d], a double variable
    //
    double LowerLimit = floor(d);

    if ((d - LowerLimit) >= 0.5)
      return ((INT)LowerLimit + 1);
    else
      return ((INT)LowerLimit);
  }

private:
  inline void zCopyRect(RECT *RDest, const RECT *RSource)
  {
    RDest->left    = RSource->left  ;
    RDest->top     = RSource->top   ;
    RDest->right   = RSource->right ;
    RDest->bottom  = RSource->bottom;
  }

  inline void zSetRect(RECT *RDest, INT l, INT t, INT r, INT b)
  {
    RDest->left    = l ;
    RDest->top     = t ;
    RDest->right   = r ;
    RDest->bottom  = b ;
  }

  inline BOOL zEqualRect(const RECT *R1, const RECT *R2)
  { return (R1->left  == R2->left)   &&  (R1->top    == R2->top)  &&
  (R1->right == R2->right)  &&  (R1->bottom == R2->bottom); }

public:
#ifdef OS2_PLATFORM

  // Provide Rectangle Conversion Routines -- RECTL to RECT
  //   and Vice-Versa

  static RECT RECTL_TO_RECT(RECTL R)
  {
    RECT R1;
    R1.left   = R.xLeft  ;
    R1.top    = R.yTop   ;
    R1.right  = R.xRight ;
    R1.bottom = R.yBottom;

    return R1;
  }

  static RECTL RECT_TO_RECTL(RECT R)
  {
    RECTL R1;

    R1.xLeft   = R.left   ;
    R1.yTop    = R.top    ;
    R1.xRight  = R.right  ;
    R1.yBottom = R.bottom ;

    return R1;
  }
#endif

};

#endif

// EOF -- ZRECT.H

