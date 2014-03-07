// MISC.CPP
//
///////////////////////////////////////////////////////////////////////////
//
//   This Module Contains Some Misc. Toolkit Functions.
//   It Also Contains the F-2D and G-3D Expression-Parsing Algorithm...
//   (Some Portions Were Adapted from the "TCALC" Spreadsheet Example that
//   Comes with the Borland Turbo-C++ or BC++ package).
//
//  ZGRAF C++ Multi-Platform Graph Toolkit v. 1.41, (c) 1996, ZGRAF Software
//
///////////////////////////////////////////////////////////////////////////
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

//
// Platform-Dependent Includes...
//
#ifdef WINDOWS_PLATFORM
#include <windows.h>
#endif

#include "toolkit.h"                 // Toolkit Header File
#include "lowlevel.h"                // Low-Level Graphics Functions


//
// Defines (Used in this Module Only)
//
#define PLUS            0
#define MINUS           1
#define TIMES           2
#define DIVIDE          3
#define EXP             4
#define COLON           5
#define OPAREN          6
#define CPAREN          7
#define NUM             8
#define CELL            9
#define FUNC            10
#define EOLN            11
#define BAD             12
#define MAXFUNCNAMELEN  5
#define PARSERSTACKSIZE 20
#define TRUE            1
#define FALSE           0
#define MAXROWS         100
#define MAXCOLS         100

//
// Structures (Used In this Module Only)
//
struct TOKENREC                     // Token Structure for Parsing Algorithm
{
    INT state;
    union
    {
        double value;
        WCHAR funcname[MAXFUNCNAMELEN + 1];
    } x;
};

//
// Local Variables, Arrays, Declarations
//
static struct TOKENREC stack[PARSERSTACKSIZE], curtoken;
INT    stacktop, tokentype, error;
WCHAR  * input, isformula;
static double XValue, YValue;     // These are the F(X) and G(X,Y) variables...

//
// Function Prototypes (Functions in this Module)
//
INT    formulastart(WCHAR ** input, INT * col, INT * row);
INT    gotostate(INT production);
INT    isfunc(WCHAR * s);
INT    nexttoken(void);
struct TOKENREC pop(void);
void   push(struct TOKENREC * token);
void   reduce(INT reduction);
void   shift(INT state);


#if defined(WINDOWS_PLATFORM)

////////////////////////////////////////////////////////////////////////////
// zColorTable[] is a Table of 64 Arbitrarily-Chosen Colors
// Used In The Graphs.  The First 16 are the Standard Pure VGA Colors.
// Feel Free to Modify Or Extend This List...
////////////////////////////////////////////////////////////////////////////

static zRGB zColorTable[ 64 ] =
{
    //
    // Note:  First 2 Rows Are the "Pure" Windows VGA Colors,
    //        Remaining 48 Colors are "Dithered" Variations
    //

    // Drk Blk   // Drk Blue  // Drk Grn   // Drk Cyan  // Drk Red   // Drk Mag   // Drk Yel   // Gray
    0x00000000L, 0x00800000L, 0x00008000L, 0x00808000L, 0x00000080L, 0x00800080L, 0x00008080L, 0x00C0C0C0L,

    // Brt Blk   // Brt Blue  // Brt Grn   // Brt Cyan  // Brt Red   // Brt Mag   // Drk Yel   // Brt Wht
    0x00808080L, 0x00FF0000L, 0x0000FF00L, 0x00FFFF00L, 0x000000FFL, 0x00FF00FFL, 0x0000FFFFL, 0x00FFFFFFL,

    0x00404040l, 0x00E0E0E0l, 0x00C0C0FFl, 0x00C0E0FFl, 0x00E0FFFFl, 0x00C0FFC0l, 0x00FFFFC0l, 0x00FFC0C0l,
    0x00707070l, 0x000000C0l, 0x000040C0l, 0x0000C0C0l, 0x0000C000l, 0x00C0C000l, 0x00C00000l, 0x00C000C0l,
    0x00202020l, 0x00FF80FFl, 0x008080FFl, 0x0080C0FFl, 0x0080FFFFl, 0x0080FF80l, 0x00FFFF80l, 0x00FF8080l,
    0x00404040l, 0x00D0D0D0l, 0x00004080l, 0x00008080l, 0x00909090l, 0x00FFC000l, 0x008000FFl, 0x00200020l,
    0x00505050l, 0x00E0E0E0l, 0x00FFC0FFl, 0x000080FFl, 0x0060FFC0l, 0x00808000l, 0x00202000l, 0x00002020l,
    0x00303030l, 0x00000040l, 0x00404080l, 0x00004040l, 0x00004000l, 0x00404000l, 0x00400000l, 0x00400040l
};
#endif

//////////////////////////////// zMakeColor() ///////////////////////////////

zRGB zMakeColor(BYTE red, BYTE green, BYTE blue)
{
    // Makes a Full 32-Bit Color From Individual R, G, B Components.
    //   Returns the Color

#ifdef WINDOWS_PLATFORM
    // For Windows, There's a Handy Macro Already!

    COLORREF Color = RGB(red, green, blue);

#endif

#if defined(MS_DOS_PLATFORM)
    UNUSED_PARAMETERS(&red, &green, &blue);

    // Operation Not Yet Defined!
    zRGB Color = 0;
#endif

    return ((zRGB) Color);

}

///////////////////////////////// RGB_color() ///////////////////////////////

zRGB RGB_color(INT color)
{
    //
    // Converts an Integer Color Value In the Range (0-63)
    //   to an RGB color In the Table Above
    //

    if (color < 0  ||  color > 63)
    {
        color = 0;
    }


#ifdef WINDOWS_PLATFORM
    // For Windows, Just Return the Color
    zRGB rgb = zColorTable[ color ];
    return (rgb);
#endif

#if defined(MS_DOS_PLATFORM)
    // Operation Not Yet Defined!
    return (zRGB)(color);
#endif

}

///////////////////////////////// HOURGLASS() ////////////////////////////////

void HOURGLASS(INT mode)
{
    // This Function May Be Used to Display The Hourglass Windows Cursor
    //   During Periods of Massive CPU Activity

#ifdef WINDOWS_PLATFORM
    static HCURSOR hCursor;

    if (mode == ON)            // Display the Hourglass Cursor
    {
        hCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
        ShowCursor(TRUE);
    }
    else if (mode == OFF)      // Switch back to the Regular (Mouse) Cursor
    {
        ShowCursor(FALSE);
        SetCursor(hCursor);
    }
#endif

#if defined(MS_DOS_PLATFORM)
    // Kill Unused Parameter Warning.
    UNUSED_PARAMETERS(&mode);

    // [Put Code to Show/Hide An Hourglass Cursor Here]
#endif
}

/////////////////////////// zDisplayMessage() ///////////////////////////////

void zDisplayMessage(PWCHAR title, PWCHAR message)
{
    //
    // Displays message [message] in a Message Box onscreen;
    //   [title] is the message box title
    //

#ifdef WINDOWS_PLATFORM
    MessageBoxW(NULL, message, title,
                MB_OK | MB_ICONINFORMATION | MB_SYSTEMMODAL);

    // Note:  MB_SYSTEMMODAL forces User to Press OK before continuing...
#endif

#if defined(MS_DOS_PLATFORM)
    UNUSED_PARAMETERS(&title);                    // Kill Compiler Warning
    zLowLevelGraphics::zDOSShowMessage(message);  // Show a Message Box
#endif

}


//////////////////////////// zDisplayError() /////////////////////////////

void zDisplayError(PWCHAR message)
{
    //
    // Displays error message [message] in a Message Box onscreen;
    //
#ifdef WINDOWS_PLATFORM
    MessageBoxW(NULL, message, zGENERAL_ERROR_MSG,
                MB_OK | MB_ICONEXCLAMATION);
#endif

#if defined(MS_DOS_PLATFORM)
    zLowLevelGraphics::zDOSShowMessage(message);    // Show a Message Box
#endif

}

////////////////////////// GetPercentOfRect() /////////////////////////////

RECT GetPercentOfRect(RECT  R,
                      double LeftPercent,
                      double TopPercent,
                      double RightPercent,
                      double BottomPercent)
{
    // This is Just a "RECT" Function Equivalent to
    //   zRECT::ScaleViaPercentageOffsets() That We Can Call From Anywhere...

    static RECT RNew;

    INT RWidth  = abs(R.left - R.right);
    INT RHeight = abs(R.top  - R.bottom);

    RNew.left   = R.left + zLowLevelGraphics::zRound(LeftPercent   * RWidth);
    RNew.top    = R.top  + zLowLevelGraphics::zRound(TopPercent    * RHeight);
    RNew.right  = R.left + zLowLevelGraphics::zRound(RightPercent  * RWidth);
    RNew.bottom = R.top  + zLowLevelGraphics::zRound(BottomPercent * RHeight);

    return (RNew);
}

/////////////////////////// UNUSED_PARAMETERS() //////////////////////////

void UNUSED_PARAMETERS(void *, ...)
{
    // This Function Is Used to Kill Compiler "Unused Parameter" Warnings
    //   It Doesn't Do Anything!
}

///////////////////////////////////////////////////////////////////////////
// The Following Functions are Used for the Function-Parser Algorithm
///////////////////////////////////////////////////////////////////////////

INT isfunc(WCHAR * s)       // Returns TRUE if the string is a legal function,
{
    // or FALSE otherwise
    INT len = _tcslen(s);

    if (_tcsncmp(s, input, len) == 0)
    {
        wcsncpy_s(curtoken.x.funcname, 6, input, len);
        curtoken.x.funcname[len] = 0;
        input += len;
        return (TRUE);
    }
    return (FALSE);
}

// *************************************************************************

INT nexttoken(void)              // Gets the next token from the input stream
{
    WCHAR * start, numstring[180];
    INT decimal, len, numlen;

    while (*input == ' ')
    {
        input++;
    }
    if (*input == 0)
    {
        return (EOLN);
    }
    if (*input == 'X')           // Added code here to handle the 'X' variable
    {
        input++;
        curtoken.x.value = XValue;
        return (NUM);
    }
    if (*input == 'Y')           // Added code here to handle the 'Y' variable
    {
        input++;
        curtoken.x.value = YValue;
        return (NUM);
    }

    if (strchr("0123456789.", *input))
    {
        start = input;
        len = 0;
        decimal = FALSE;
        while ((isdigit(*input)) || ((*input == '.') && (!decimal)))
        {
            if (*input == '.')
            {
                decimal = TRUE;
            }
            input++;
            len++;
        }
        if ((len == 1) && (start[0] == '.'))
        {
            return (BAD);
        }
        if (*input == 'E')
        {
            input++;
            len++;
            if (strchr("+-", *input) != NULL)
            {
                input++;
                len++;
            }
            numlen = 0;
            while ((isdigit(*input)) && (++numlen <= 3))
            {
                input++;
                len++;
            }
        }
        wcsncpy_s(numstring, _countof(numstring), start, len);
        numstring[len] = 0;
        curtoken.x.value = _tcstod((PWCHAR)numstring, NULL);
        //// DEBUG
        //    if (errno == ERANGE)
        //       return (BAD);
        return (NUM);
    }
    else if (isalpha(*input))
    {
        if
        (isfunc(_T("ABS")) ||
                isfunc(_T("ACOS")) ||
                isfunc(_T("ASIN")) ||
                isfunc(_T("ATAN")) ||
                isfunc(_T("COSH")) ||
                isfunc(_T("COS")) ||
                isfunc(_T("EXP")) ||
                isfunc(_T("LOG10")) ||
                isfunc(_T("LOG")) ||
                isfunc(_T("POW10")) ||
                isfunc(_T("ROUND")) ||
                isfunc(_T("SINH")) ||
                isfunc(_T("SIN")) ||
                isfunc(_T("SQRT")) ||
                isfunc(_T("SQR")) ||
                isfunc(_T("TANH")) ||
                isfunc(_T("TAN")) ||
                isfunc(_T("TRUNC")))
        {
            return (FUNC);
        }

        else
        {
            return (BAD);
        }
    }
    else switch (*(input++))
        {
        case '+' :
            return (PLUS);
        case '-' :
            return (MINUS);
        case '*' :
            return (TIMES);
        case '/' :
            return (DIVIDE);
        case '^' :
            return (EXP);
        case ':' :
            return (COLON);
        case '(' :
            return (OPAREN);
        case ')' :
            return (CPAREN);
        default  :
            return (BAD);
        } /* switch */
}

// **************************************************************************

void push(struct TOKENREC * token)      // Pushes a new token onto the stack
{
    if (stacktop == PARSERSTACKSIZE - 1)
    {
        // printf(zSTACK_ERROR);
        error = TRUE;
    }
    else
    {
        stack[++stacktop] = *token;
    }
}

// ***************************************************************************

struct TOKENREC pop(void)       // Pops the top token off of the stack
{
    return (stack[stacktop--]);
}

// ***************************************************************************

INT gotostate(INT production)   // Finds the new state based on the last
// production and the top state
{
    INT state = stack[stacktop].state;

    if (production <= 3)
    {
        switch (state)
        {
        case 0 :
            return (1);
        case 9 :
            return (19);
        case 20 :
            return (28);
        }
    }
    else if (production <= 6)
    {
        switch (state)
        {
        case 0 :
        case 9 :
        case 20 :
            return (2);
        case 12 :
            return (21);
        case 13 :
            return (22);
        }
    }
    else if (production <= 8)
    {
        switch (state)
        {
        case 0  :
        case 9  :
        case 12 :
        case 13 :
        case 20 :
            return (3);
        case 14 :
            return (23);
        case 15 :
            return (24);
        case 16 :
            return (25);
        }
    }
    else if (production <= 10)
    {
        switch (state)
        {
        case 0  :
        case 9  :
        case 12 :
        case 13 :
        case 14 :
        case 15 :
        case 16 :
        case 20 :
            return (4);
        }
    }
    else if (production <= 12)
    {
        switch (state)
        {
        case 0  :
        case 9  :
        case 12 :
        case 13 :
        case 14 :
        case 15 :
        case 16 :
        case 20 :
            return (6);
        case 5  :
            return (17);
        }
    }
    else
    {
        switch (state)
        {
        case 0  :
        case 5  :
        case 9  :
        case 12 :
        case 13 :
        case 14 :
        case 15 :
        case 16 :
        case 20 :
            return (8);
        }
    }
    return (30);
}

// *************************************************************************

void shift(INT state)                       // Shifts a token onto the stack
{
    curtoken.state = state;
    push(&curtoken);
    tokentype = nexttoken();
}

// *************************************************************************

void reduce(INT reduction)                  // Completes a reduction
{
    struct TOKENREC token1, token2;

    switch (reduction)
    {
    case 1 :
        token1 = pop();
        pop();
        token2 = pop();
        curtoken.x.value = token1.x.value + token2.x.value;
        break;
    case 2 :
        token1 = pop();
        pop();
        token2 = pop();
        curtoken.x.value = token2.x.value - token1.x.value;
        break;
    case 4 :
        token1 = pop();
        pop();
        token2 = pop();
        curtoken.x.value = token1.x.value * token2.x.value;
        break;
    case 5 :
        token1 = pop();
        pop();
        token2 = pop();
        if (token1.x.value == 0)
        {
            curtoken.x.value = HUGE_VAL;
        }
        else
        {
            curtoken.x.value = token2.x.value / token1.x.value;
        }
        break;
    case 7 :
        token1 = pop();
        pop();
        token2 = pop();
        curtoken.x.value = pow(token2.x.value, token1.x.value);
        break;
    case 9 :
        token1 = pop();
        pop();
        curtoken.x.value = -token1.x.value;
        break;
    case 11 :     // Deleted a Bunch of Code Here -- We'll Never do this case...
        break;
    case 13 :     // Ditto...
        break;
    case 14 :
        pop();
        curtoken = pop();
        pop();
        break;
    case 16 :
        pop();
        curtoken = pop();
        pop();
        token1 = pop();
        if (_tcscmp(token1.x.funcname, _T("ABS")) == 0)
        {
            curtoken.x.value = fabs(curtoken.x.value);
        }
        else if (_tcscmp(token1.x.funcname, _T("ACOS")) == 0)
        {
            curtoken.x.value = acos(curtoken.x.value);
        }
        else if (_tcscmp(token1.x.funcname, _T("ASIN")) == 0)
        {
            curtoken.x.value = asin(curtoken.x.value);
        }
        else if (_tcscmp(token1.x.funcname, _T("ATAN")) == 0)
        {
            curtoken.x.value = atan(curtoken.x.value);
        }
        else if (_tcscmp(token1.x.funcname, _T("COSH")) == 0)
        {
            curtoken.x.value = cosh(curtoken.x.value);
        }
        else if (_tcscmp(token1.x.funcname, _T("COS")) == 0)
        {
            curtoken.x.value = cos(curtoken.x.value);
        }
        else if (_tcscmp(token1.x.funcname, _T("EXP")) == 0)
        {
            curtoken.x.value = exp(curtoken.x.value);
        }
        else if (_tcscmp(token1.x.funcname, _T("LOG10")) == 0)
        {
            curtoken.x.value = log10(curtoken.x.value);
        }
        else if (_tcscmp(token1.x.funcname, _T("LOG")) == 0)
        {
            curtoken.x.value = log(curtoken.x.value);
        }
        else if (_tcscmp(token1.x.funcname, _T("ROUND")) == 0)
        {
            curtoken.x.value = (INT)(curtoken.x.value + 0.5);
        }
        else if (_tcscmp(token1.x.funcname, _T("POW10")) == 0)
        {
            curtoken.x.value = pow(10.0, curtoken.x.value);
        }
        else if (_tcscmp(token1.x.funcname, _T("SINH")) == 0)
        {
            curtoken.x.value = sinh(curtoken.x.value);
        }
        else if (_tcscmp(token1.x.funcname, _T("SIN")) == 0)
        {
            curtoken.x.value = sin(curtoken.x.value);
        }
        else if (_tcscmp(token1.x.funcname, _T("SQRT")) == 0)
        {
            curtoken.x.value = sqrt(curtoken.x.value);
        }
        else if (_tcscmp(token1.x.funcname, _T("SQR")) == 0)
        {
            curtoken.x.value *= curtoken.x.value;
        }
        else if (_tcscmp(token1.x.funcname, _T("TANH")) == 0)
        {
            curtoken.x.value = tanh(curtoken.x.value);
        }
        else if (_tcscmp(token1.x.funcname, _T("TAN")) == 0)
        {
            curtoken.x.value = tan(curtoken.x.value);
        }
        else if (_tcscmp(token1.x.funcname, _T("TRUNC")) == 0)
        {
            curtoken.x.value = (INT)curtoken.x.value;
        }
        break;
    case 3 :
    case 6 :
    case 8 :
    case 10 :
    case 12 :
    case 15 :
        curtoken = pop();
        break;
    }
    curtoken.state = gotostate(reduction);
    push(&curtoken);
}

// ***************************************************************************

double parse(LPCSTR s, INT * att, double X, double Y)
{
    //
    // Parses the string s - and evaluates the function at (X) or
    // at (X, Y).  Note:  [Y] is a default Argument (defaults to 0.0).
    // Function returns the value of the evaluated string, and puts
    // the attribute in [att]: TEXT = 0, VALUE = 1, FORMULA = 2.
    //
    struct TOKENREC firsttoken;
    char accepted = FALSE;
    char copy[180];
    WCHAR wcopy[180];

    // Set the X and Y-Values

    XValue = X;
    YValue = Y;

    error = FALSE;
    isformula = FALSE;
    input = (PWCHAR)wcopy;

    // The Next Line May Need to Be Handled Slightly Differently
    //   Depending Upon What Platform We're On...

#ifdef WINDOWS_PLATFORM
    strcpy_s(copy, sizeof(copy), s);
    _strupr_s(copy, sizeof(copy));
#endif

#if defined(MS_DOS_PLATFORM)
    strupr(strcpy(copy,s));
#endif

    size_t copied;
    errno_t err = mbstowcs_s(&copied, input, 180, copy, 180);

    stacktop = -1;
    firsttoken.state = 0;
    firsttoken.x.value = 0;
    push(&firsttoken);
    tokentype = nexttoken();
    do
    {
        switch (stack[stacktop].state)
        {
        case 0 :
        case 9 :
        case 12 :
        case 13 :
        case 14 :
        case 15 :
        case 16 :
        case 20 :
            if (tokentype == NUM)
            {
                shift(10);
            }
            else if (tokentype == CELL)
            {
                shift(7);
            }
            else if (tokentype == FUNC)
            {
                shift(11);
            }
            else if (tokentype == MINUS)
            {
                shift(5);
            }
            else if (tokentype == OPAREN)
            {
                shift(9);
            }
            else
            {
                error = TRUE;
            }
            break;
        case 1 :
            if (tokentype == EOLN)
            {
                accepted = TRUE;
            }
            else if (tokentype == PLUS)
            {
                shift(12);
            }
            else if (tokentype == MINUS)
            {
                shift(13);
            }
            else
            {
                error = TRUE;
            }
            break;
        case 2 :
            if (tokentype == TIMES)
            {
                shift(14);
            }
            else if (tokentype == DIVIDE)
            {
                shift(15);
            }
            else
            {
                reduce(3);
            }
            break;
        case 3 :
            reduce(6);
            break;
        case 4 :
            if (tokentype == EXP)
            {
                shift(16);
            }
            else
            {
                reduce(8);
            }
            break;
        case 5 :
            if (tokentype == NUM)
            {
                shift(10);
            }
            else if (tokentype == CELL)
            {
                shift(7);
            }
            else if (tokentype == FUNC)
            {
                shift(11);
            }
            else if (tokentype == OPAREN)
            {
                shift(9);
            }
            else
            {
                error = TRUE;
            }
            break;
        case 6 :
            reduce(10);
            break;
        case 7 :
            if (tokentype == COLON)
            {
                shift(18);
            }
            else
            {
                reduce(13);
            }
            break;
        case 8 :
            reduce(12);
            break;
        case 10 :
            reduce(15);
            break;
        case 11 :
            if (tokentype == OPAREN)
            {
                shift(20);
            }
            else
            {
                error = TRUE;
            }
            break;
        case 17 :
            reduce(9);
            break;
        case 18 :
            if (tokentype == CELL)
            {
                shift(26);
            }
            else
            {
                error = TRUE;
            }
            break;
        case 19 :
            if (tokentype == PLUS)
            {
                shift(12);
            }
            else if (tokentype == MINUS)
            {
                shift(13);
            }
            else if (tokentype == CPAREN)
            {
                shift(27);
            }
            else
            {
                error = TRUE;
            }
            break;
        case 21 :
            if (tokentype == TIMES)
            {
                shift(14);
            }
            else if (tokentype == DIVIDE)
            {
                shift(15);
            }
            else
            {
                reduce(1);
            }
            break;
        case 22 :
            if (tokentype == TIMES)
            {
                shift(14);
            }
            else if (tokentype == DIVIDE)
            {
                shift(15);
            }
            else
            {
                reduce(2);
            }
            break;
        case 23 :
            reduce(4);
            break;
        case 24 :
            reduce(5);
            break;
        case 25 :
            reduce(7);
            break;
        case 26 :
            reduce(11);
            break;
        case 27 :
            reduce(14);
            break;
        case 28 :
            if (tokentype == PLUS)
            {
                shift(12);
            }
            else if (tokentype == MINUS)
            {
                shift(13);
            }
            else if (tokentype == CPAREN)
            {
                shift(29);
            }
            else
            {
                error = TRUE;
            }
            break;
        case 29 :
            reduce(16);
            break;
        case 30 :
            error = TRUE;
            break;
        }
    }
    while ((!accepted) && (!error));
    if (error)
    {
        *att = 0;     /* TEXT */
        return (0);
    }
    if (isformula)
    {
        *att = 2   ;    /*  FORMULA */
    }
    else
    {
        *att = 1   ;    /*  VALUE */
    }

    return (stack[stacktop].x.value);
}

// *************************** formulastart() *****************************

INT formulastart(WCHAR ** input, INT * col, INT * row)
{
    //
    // Returns TRUE if the string is the start of a formula, FALSE otherwise.
    // Also returns the column and row of the formula
    //    (Note:  the (row,col) stuff is applicable only to spreadsheets
    //     but I didn't bother to strip it out...      -JJ)
    //

    INT len, maxlen = 20;                       // Added some variables here
    WCHAR * start, numstring[10];

    if (!isalpha(**input))
    {
        return (FALSE);
    }
    *col = *((*input)++) - 'A';
    if (isalpha(**input))
    {
        *col *= 26;
        *col += *((*input)++) - 'A' + 26;
    }
    if (*col >= MAXCOLS)
    {
        return (FALSE);
    }
    start = *input;
    for (len = 0; len < maxlen; len++)
    {
        if (!isdigit(*((*input)++)))
        {
            (*input)--;
            break;
        }
    }
    if (len == 0)
    {
        return (FALSE);
    }
    wcsncpy_s(numstring, _countof(numstring), start, len);
    numstring[len] = 0;
    *row = _ttoi(numstring) - 1;

    if ((*row >= MAXROWS) || (*row == -1))
    {
        return (FALSE);
    }
    return (TRUE);
}

// EOF -- MISC.CPP

