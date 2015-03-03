// Printer dimensions
#ifndef _PRINTDIM_H
#define _PRINTDIM_H


typedef struct SFraction {
    int numerator;
    int denominator;
} fraction;

inline int fract_multiply(fraction f, int scaler) {
    return ((f.numerator * scaler) / f.denominator);
}

const fraction Print_Title_Height = {1,2};  // 1/2 inch.
const fraction Print_Title_Offset = {1,6};
const int Print_Title_Size = 30;
const fraction Print_Margin = { 1, 2 };
const fraction Print_Graph_Seperator = { 1, 10 };
const int Small_Font_DPI = 150;
const int Standard_DPI   = 100;
const int Graph_Inset = 4;
const int ANNOT_PRINT_HEIGHT = 30;
const int LEGEND_PRINT_WIDTH = 70;
const int XSCALE_PRINT_HEIGHT = 30;

#endif //_PRINTDIM_H
