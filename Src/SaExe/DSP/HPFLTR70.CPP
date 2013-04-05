/**************************************************************************\
* MODULE:       HPFLTR70.CPP                                               *
* COPYRIGHT:    SUMMER INSTITUTE OF LINGUISTICS, 1997-2000                 *
* AUTHOR:       ALEC K. EPTING                                             *
*                                                                          *
*                                                                          *
* DESCRIPTION:                                                             *
*                                                                          *
* Digital highpass filter class with cut-in at 70 Hz.                      *
*                                                                          *
*                                                                          *
* PUBLIC MEMBER VARIABLES:                                                 *
*                                                                          *
*                                                                          *
* PUBLIC MEMBER FUNCTIONS:                                                 *
*                                                                          *
* Note: all arguments are passed using the C calling convention.           *
*                                                                          *
* Copyright                                                                *
*   Function to return character pointer to copyright notice.              *
*                                                                          *
*   Arguments:                                                             *
*     None                                                                 *
*                                                                          *
*   Return value:                                                          *
*     pointer to null-terminated string containing the copyright notice.   *
*                                                                          *
* Version                                                                  *
*   Function to return version of class as floating point number.          *
*                                                                          *
*   Arguments:                                                             *
*     None                                                                 *
*                                                                          *
*   Return value:                                                          *
*     floating point number containing the version number.                 *
*                                                                          *
* CreateObject                                                             *
*   Function to construct filtering object                                 *
*                                                                          *
*   Arguments:                                                             *
*     ppoHPFilter         address to contain pointer to filter object      *
*     wSmpRate            sampling rate in Hz                              *
*                                                                          *
*   Return value:  4-byte signed integer, defined in ERRCODES.H            *
*     DONE                      no fatal errors                            *
*     UNSUPP_SMP_RATE           fatal error: unsupported sampling rate     *
*                                  specified                               *
*     OUT_OF_MEMORY             fatal error: insufficient memory to        *
*                                  create object                           *
*                                                                          *
* Filter                                                                   *
*   Function to filter signal data in-place.                               *
*                                                                          *
*   Arguments:                                                             *
*     pProcessBuffer      pointer to process buffer of type                *
*                         unsigned byte (8-bit) or signed short (16-bit)   *
*     dwProcessLength     number of samples to filter                      *
*   Return value:                                                          *
*     INVALID_SIG_DATA_PTR           fatal error: invalid buffer pointer   *
*                                  specified                               *
*     INVALID_SIG_LEN           fatal error: invalid signal length         *
*                                  specified                               *
*                                                                          *
* ForwardPass                                                              *
*   Function to filter signal forward in time.                             *
*                                                                          *
*   Arguments:                                                             *
*     pProcessBuffer      pointer to process buffer of type                *
*                         unsigned byte (8-bit) or signed short (16-bit)   *
*     dwProcessLength     number of samples to filter                      *
*                                                                          *
*   Return value:                                                          *
*     INVALID_SIG_DATA_PTR           fatal error: invalid buffer pointer   *
*                                  specified                               *
*     INVALID_SIG_LEN           fatal error: invalid signal length         *
*                                  specified                               *
*                                                                          *
* BackwardPass                                                             *
*   Function to filter signal backward in time.                            *
*                                                                          *
*   Arguments:                                                             *
*     pProcessBuffer      pointer to process buffer of type                *
*                         unsigned byte (8-bit) or signed short (16-bit)   *
*     dwProcessLength     number of samples to filter                      *
*                                                                          *
*   Return value:                                                          *
*     INVALID_SIG_DATA_PTR           fatal error: invalid buffer pointer   *
*                                  specified                               *
*     INVALID_SIG_LEN           fatal error: invalid signal length         *
*                                  specified                               *
*                                                                          *
* GetBlockMin                                                              *
*   Function to get minimum value for processed block.                     *
*                                                                          *
*   Arguments:                                                             *
*     None                                                                 *
*                                                                          *
*   Return value:                                                          *
*     signed short (16-bit) containing minimum value having a range of     *
*     the data type specified for the Process buffer pointer in the        *
*     Filter, ForwardPass, and BackwardPass functions.                     *
*                                                                          *
* GetBlockMax                                                              *
*   Function to get maximum value for processed block.                     *
*                                                                          *
*   Arguments:                                                             *
*     None                                                                 *
*                                                                          *
*   Return value:                                                          *
*     signed short (16-bit) containing maximum value having a range of     *
*     the data type specified for the Process buffer pointer in the        *
*     Filter, ForwardPass, and BackwardPass functions.                     *
*                                                                          *
*                                                                          *
*                                                                          *
* TYPICAL CALLING SEQUENCE:                                                *
*          :                                                               *
*   #include "HPFltr70.h"                                                  *
*          :                                                               *
*   uint16 wSmpRate = (uint32)pDoc->GetSamplesPerSec();                     *
*   uint8  SignalBuffer[BUFFER_SIZE];                                      *
*          :                                                               *
*   (load uint8 sample data at SignalBuffer);                              *
*          :                                                               *
*   dspError_t Err = CHighPassFilter70::CreateObject(&poHighPass, wSmpRate);      *
*   if (Err) return;                                                       *
*   poHighPass->Filter(SignalBuffer, BUFFER_SIZE);                         *
*   delete poHighPass;                                                     *
*                                                                          *
*                                                                          *
*                                                                          *
* COMPILER:         Microsoft Visual C++ version 1.52                      *
* COMPILE OPTIONS:  /nologo /G3 /W3 /Zi /AL /YX /D "_DEBUG"                *
*                   /I "c:\msvc\dsp" /GA /Fd"SA.PDB" /Fp"SA.PCH"           *
*                   (/D "TEST_HPF" for testing)                            *
*                                                                          *
* LINK OPTIONS:     /NOLOGO /LIB:"lafxcwd" /LIB:"oldnames" /LIB:"libw"     *
*                   /LIB:"llibcew" /LIB:"dsp\fft86e" /NOD /NOE             *
*                   /PACKC:61440 /STACK:10120 /SEG:256 /ALIGN:64           *
*                   /ONERROR:NOEXE /CO /LIB:"commdlg.lib"                  *
*                   /LIB:"mmsystem.lib" /LIB:"shell.lib"                   *
*                                                                          *
* TO DO:            1. Provide internal buffer allocation option.          *
*                   2. Permit separate output buffer.                      *
*                   3. Prevent user from switching data types or           *
*                      filtering forward and backward passes               *
*                      out-of-sequence.                                    *
*                                                                          *
*                                                                          *
* CHANGE HISTORY:                                                          *
*   Date     By             Description                                    *
* 07/03/97  AKE    Copied floating point from highpass.c.                  *
* 07/04/97  AKE    Debugged and running.                                   *
* 07/11/97  AKE    Add fixed point assembler code.                         *
* 07/16/97  AKE    Filter gain moved ahead of lattice to reduce chance     *
*                  for overflow.  Floating point code modified             *
*                  accordingly.                                            *
* 07/28/97  AKE    Fixed point assembler code debugged and tested.         *
* 07/30/97  AKE    Increased precision for 8-bit data.  Tested.            *
* 07/31/97  AKE    Added support for 44.1 and 48 kHz sampling rates.       *
*                  Tested.                                                 *
* 11/05/97  AKE    Added conditional compile because variable offsets are  *
*                  different for debug v. release code.                    *
* 12/06/97  AKE    Changed variable offsets for DEBUG to reflect           *
*                  optimizations disabled.                                 *
* 04/01/00  AKE    Converted loop code to assembler, permitting all        *
*                  variable stack offsets to be hard-coded.  This          *
*                  circumvents the compiler's propensity to move           *
*                  variable locations every time a change to the filter    *
*                  source code or include files is made.                   *
* 04/05/00  AKE    Removed int8 and floating point code.  Set filter      *
*                  parameters during object construction based on          *
*                  specified sampling rate.  Added forward and backward    *
*                  passes.  Overloaded filter functions to enable sample   *
*                  data types to be determined automatically.  Passed      *
*                  buffer pointer to filter functions to permit buffers    *
*                  to be switched for each call.  Added workspace          *
*                  structure to ensure all stack offsets for filter        *
*                  function variables remain in a consistent location      *
*                  during compilation.                                     *
* 04/10/00  AKE    Added code to track max and min values.  Tested and     *
*                  released as v1.2.                                       *
* 06/01/00  AKE    Added conditional compile code to support 32 bit (not   *
*                  tested), v1.3.                                          *
* 06/23/00  AKE    Set default object pointer to NULL.                     *
* 07/11/00  AKE    Cleaned up for 32-bit.  Released as v1.4 (tested).      *
* 07/27/00  AKE    Fixed max and min tracking for unsigned byte data       *
*                  so that comparison is done on signed data.  Released    *
*                  as v1.5.                                                *
* 01/15/01  AKE    Converted to 32-bit.  Released as v2.0 (tested).        *
\**************************************************************************/
#include "stddsp.h"

#define HPFLTR70_CPP
#define COPYRIGHT_DATE  "2000"
#define VERSION_NUMBER  "2.0"

#include <math.h>
#include "MathX.h"
#include "HPFltr70.h"
#include <stdio.h>

////////////////////////////////////////////////////////////////////////////////////////
// Class function to return copyright notice.                                         //
////////////////////////////////////////////////////////////////////////////////////////
char * CHighPassFilter70::Copyright(void)
{
    static char Notice[] = {"Highpass Filter Version " VERSION_NUMBER "\n"
                            "Copyright (c) " COPYRIGHT_DATE " by Summer Institute of Linguistics. "
                            "All rights reserved.\n"
                           };
    return(Notice);
}

////////////////////////////////////////////////////////////////////////////////////////
// Class function to return version of class.                                         //
////////////////////////////////////////////////////////////////////////////////////////
float CHighPassFilter70::Version(void)
{
    return((float)atof(VERSION_NUMBER));
}

#undef COPYRIGHT_DATE
#undef VERSION_NUMBER

////////////////////////////////////////////////////////////////////////////////////////
// Class function to construct filter object if parameters are valid.                 //
////////////////////////////////////////////////////////////////////////////////////////
dspError_t CHighPassFilter70::CreateObject(CHighPassFilter70 ** ppoHPFilter, uint16 wSmpRate)
{
// Validate parameters passed.
    if (!ppoHPFilter)
    {
        return(Code(INVALID_PARM_PTR));    //address of pointer to filter object
    }
    //  must not be NULL
    *ppoHPFilter = NULL;
    dspError_t Err = ValidateSignalParms(wSmpRate);       //check signal parameter
    if (Err)
    {
        return(Err);
    }

// Construct filter object.
    *ppoHPFilter = new CHighPassFilter70(wSmpRate);
    if (!*ppoHPFilter)
    {
        return(Code(OUT_OF_MEMORY));
    }

    return(DONE);
}


////////////////////////////////////////////////////////////////////////////////////////
// Class function to validate signal parameters.                                      //
////////////////////////////////////////////////////////////////////////////////////////
dspError_t CHighPassFilter70::ValidateSignalParms(uint16 wSmpRate)
{
    if (wSmpRate != 11025 && wSmpRate != 16000 && wSmpRate != 22050 &&
            wSmpRate != 44100 && wSmpRate != 48000)
    {
        return(Code(UNSUPP_SMP_RATE));    //  valid Microsoft Windows frequencies
    }

    return(DONE);
}

////////////////////////////////////////////////////////////////////////////////////////
// Filter object constructor.                                                         //
////////////////////////////////////////////////////////////////////////////////////////
/**************************************************************************************/
/*                                                                                    */
/*                              SECOND ORDER IIR FILTER                               */
/*                                                                                    */
/*                                                                                    */
/*                                                    g                               */
/*   SignalBfr[i] -->--sig-----.--->---r10--------.--->--fsig---+---> SignalBfr[i]    */
/*                      |      |        | -1      |             |                     */
/*                      ^      V        ^z        V             ^                     */
/*                      |      |  f11   |    b11  |             |                     */
/*                    -128     .--->---r11----<---.            128                    */
/*                   convert   |        | -1      |          convert                  */
/*                     to      V        ^z        V            to                     */
/*                   signed    |  f12   |    b12  |          unsigned                 */
/*                  if 8-bit   .--->---r12----<---.          if 8-bit                 */
/*                                                                                    */
/*                                                                                    */
/* This network was selected because the feedforward and feedback products are summed */
/* together.  For the given coefficients, the products are approximately of the same  */
/* magnitude but opposite in sign, and thus tend to cancel each other.  This feature  */
/* is especially suited to preventing overflow with fixed point arithmetic.           */
/*                                                                                    */
/**************************************************************************************/


/*  Filter coefficient matrices for 2nd order Chebyshev II highpass filter with cut-in */
/*  at 70 Hz.  Computed using MathCad 4.0 Signal Processing Function Pack (see         */
/*  HPCHEBY2.MCD).                                                                     */
/*                                                                                     */
/* const float C11025[3][2] = {    // sampling rate 11,025 Hz                          */
/*                              0.9787470F,  1.F       ,                               */
/*                             -1.9574338F, -1.9569821F,                               */
/*                              0.9787470F,  0.9579458F                                */
/*                            };                                                       */
/* const float C16000[3][2] = {    // sampling rate 16,000 Hz                          */
/*                              0.9853059F,  1.F       ,                               */
/*                             -1.9705834F, -1.9703671F,                               */
/*                              0.9853059F,  0.9708278F                                */
/*                            };                                                       */
/* const float C22050[3][2] = {    // sampling rate 22,050 Hz                          */
/*                              0.9893158F,  1.F       ,                               */
/*                             -1.9786164F, -1.9785022F,                               */
/*                              0.9893158F,  0.9787458F                                */
/*                            };                                                       */
/* For fixed point arithmetic, signal data is assumed to be in signed 11.21 format for */
/* 8-bit samples, signed 19.13 format for 16-bit.  Internal storage register r10       */
/* is assumed to be in 9.23 format for 8-bit samples, 17.15 format for 16-bit.         */
/* Internal storage register pairs r11 and r12 are assumed to be in 11.53 format for   */
/* for 8-bit samples, 19.45 format for 16-bit.  Feedback coefficients b11 and b12 are  */
/* in 2.30 format, while feedforward coefficients f11 and f12 (not used because it     */
/* happens to be exactly equal to 1) are in 2.30 format.  The overall filter gain g is */
/* in 2.30 format.  The formats were carefully chosen to prevent overflow while at the */
/* same time maximizing precision.                                                     */


CHighPassFilter70::CHighPassFilter70(uint16 wSmpRate)
{

// Set filter coefficients and gain for the specified sampling frequency.
    SetFilterParms(wSmpRate);

// Set flags.
    m_bFirstForwardPass = true;
    m_bFirstBackwardPass = true;
#ifdef TEST_HPF
    m_bFirstTestPass = true;
#endif
}


////////////////////////////////////////////////////////////////////////////////////////
// Object function to set filter coefficients and gain.                               //
////////////////////////////////////////////////////////////////////////////////////////
void CHighPassFilter70::SetFilterParms(uint16 wSmpRate)
{
    switch (wSmpRate)
    {
        // for 2.30 format, scaling factor s = pow(2.,30)
    case 11025:                      // 70 Hz cut-in high pass at 11025 Hz sampling rate
        m_b11 =  2101293491;            // (int32)Round(-C11025[1][1]*s);
        m_b12 = -1028586461;            // (int32)Round(-C11025[2][1]*s);
        m_f11 = -2147417569;            // (int32)Round(C11025[1][0]/C11025[0][0]*s);
        // m_f12 =  1073741824;            // (int32)Round(C11025[2][0]/C11025[0][0]*s);
        m_g   =  1050921613;            // (int32)Round(C11025[0][0]*s);
        break;

    case 16000:                      // 70 Hz cut-in high pass at 16000 Hz sampling rate
        m_b11 =  2115665567;            // (int32)Round(-C16000[1][1]*s);
        m_b12 = -1042418362;            // (int32)Round(-C16000[2][1]*s);
        m_f11 = -2147452273;            // (int32)Round(C16000[1][0]/C16000[0][0]*s);
        // m_f12 =  1073741824;            // (int32)Round(C16000[2][0]/C16000[0][0]*s);
        m_g   =  1057964167;            // (int32)Round(C16000[0][0]*s);
        break;

    case 22050:                       // 70 Hz cut-in high pass at 22050 Hz sampling rate
        m_b11 =  2124400588;             // (int32)Round(-C22050[1][1]*s);
        m_b12 = -1050920255;             // (int32)Round(-C22050[2][1]*s);
        m_f11 = -2147467128;             // (int32)Round(C22050[1][0]/C22050[0][0]*s);
        // m_f12 =  1073741824;             // (int32)Round(C22050[2][0]/C22050[0][0]*s);
        m_g   =  1062269752;             // (int32)Round(C22050[0][0]*s);
        break;

    case 44100:                       // 70 Hz cut-in high pass at 44100 Hz sampling rate
        m_b11 =  2135945678;             // (int32)Round(-C44100[1][1]*s);
        m_b12 = -1062269579;             // (int32)Round(-C44100[2][1]*s);
        m_f11 = -2147479518;             // (int32)Round(C44100[1][0]/C44100[0][0]*s);
        // m_f12 =  1073741824;             // (int32)Round(C44100[2][0]/C44100[0][0]*s);
        m_g   =  1067990297;             // (int32)Round(C44100[0][0]*s);
        break;

    case 48000:                       // 70 Hz cut-in high pass at 48000 Hz sampling rate
        m_b11 =  2136883419;             // (int32)Round(-C48000[1][1]*s);
        m_b12 = -1063197098;             // (int32)Round(-C48000[2][1]*s);
        m_f11 = -2147480162;             // (int32)Round(C48000[1][0]/C48000[0][0]*s);
        // m_f12 =  1073741824;             // (int32)Round(C48000[2][0]/C48000[0][0]*s);
        m_g   =  1068456452;             // (int32)Round(C48000[0][0]*s);
    }
}

////////////////////////////////////////////////////////////////////////////////////////
// Filter object destructor.                                                          //
////////////////////////////////////////////////////////////////////////////////////////
CHighPassFilter70::~CHighPassFilter70()
{
}

////////////////////////////////////////////////////////////////////////////////////////
// Object function to highpass filter sample data in waveform buffer.                 //
////////////////////////////////////////////////////////////////////////////////////////
#pragma optimize( "", off  )
dspError_t CHighPassFilter70::Filter(uint8 * pProcessBuffer, uint32 dwProcessLength)
{
    dspError_t Err = ForwardPass(pProcessBuffer, dwProcessLength);
    if (Err)
    {
        return(Err);
    }

#ifdef TEST_HPF
    Err = DoBackwardPass(pProcessBuffer, dwProcessLength);
#else
    Err = BackwardPass(pProcessBuffer, dwProcessLength);
#endif
    if (Err)
    {
        return(Err);
    }

    return(Err);
}


dspError_t CHighPassFilter70::Filter(short * pProcessBuffer, uint32 dwProcessLength)
{
    dspError_t Err = ForwardPass(pProcessBuffer, dwProcessLength);
    if (Err)
    {
        return(Err);
    }

#ifdef TEST_HPF
    Err = DoBackwardPass(pProcessBuffer, dwProcessLength);
#else
    Err = BackwardPass(pProcessBuffer, dwProcessLength);
#endif
    if (Err)
    {
        return(Err);
    }

    return(Err);
}


////////////////////////////////////////////////////////////////////////////////////////
// Workspace definitions, based on HPF_WORKSPACE structure in Hpfltr70.h              //
////////////////////////////////////////////////////////////////////////////////////////
#define F11    Workspace.f11
#define B11    Workspace.b11
#define B12    Workspace.b12
#define G      Workspace.g
#define R10    Workspace.r10
#define R11L   Workspace.r11L
#define R11H   Workspace.r11H
#define R12L   Workspace.r12L
#define R12H   Workspace.r12H
#define SIG    Workspace.Sig
#define PSIG   Workspace.pSig
#define PSTART Workspace.pStart
#define PEND   Workspace.pEnd
#define MIN    Workspace.nMin
#define MAX    Workspace.nMax

////////////////////////////////////////////////////////////////////////////////////////
// Object function to perform forward-in-time high pass filtering on data in buffer.  //
////////////////////////////////////////////////////////////////////////////////////////
dspError_t CHighPassFilter70::ForwardPass(uint8 * pProcessBuffer, uint32 dwProcessLength)
{
    /* Validate parameters.                                                                */
    if (!pProcessBuffer)
    {
        return(Code(INVALID_SIG_DATA_PTR));
    }
    if (!dwProcessLength)
    {
        return(Code(INVALID_SIG_LEN));
    }

    /* If first call, clear object internal registers.                                     */
    if (m_bFirstForwardPass)
    {
        m_r11H = 0;
        m_r11L = 0;
        m_r12H = 0;
        m_r12L = 0;

        m_bFirstForwardPass = false;
    }

    /* Transfer filter parameters from object to workspace.                                */
    HPF_WORKSPACE Workspace;

    Workspace.f11  = m_f11;
    Workspace.b11  = m_b11;
    Workspace.b12  = m_b12;
    Workspace.g    = m_g;
    Workspace.r11H = m_r11H;
    Workspace.r11L = m_r11L;
    Workspace.r12H = m_r12H;
    Workspace.r12L = m_r12L;

    /* Initialize minimum and maximum values for this block.                               */
    Workspace.nMin = 127;
    Workspace.nMax = -128;

    /* Perform forward pass filtering on signal waveform buffer in-place.                  */
    Workspace.pStart = (uint8 *)pProcessBuffer;
    Workspace.pEnd = Workspace.pStart + dwProcessLength - 1;

    for (Workspace.pSig = Workspace.pStart; Workspace.pSig <= Workspace.pEnd; Workspace.pSig++)
    {
        __asm
        {

//      Calculate r10.
            mov ebx, DWORD PTR [PSIG]    // load pointer to signal sample
            movsx eax, BYTE PTR [ebx]    // get signal sample
            xor eax, 0xFFFFFF80          // convert to signed byte
            shl eax,21                   // convert to 11.21 format
            mov SIG, eax                 // store in sig
            mov edx, R11L                // get low dword of r11
            add eax, R11H                // accumulate high dword of r11, low unchanged
            js  UB_F11                   // negative?
            add edx, 0x20000000          //   no, round to 23rd binary place
            adc eax, 0                   //     carry into hi dword
            jmp UB_F12
            UB_F11:  sub edx, 0x20000000          //   yes, round to 23rd binary place
            sbb eax, 0                   //     carry into hi dword
            UB_F12:  shld eax, edx, 2             // convert to 9.23 format
            mov R10, eax                 // store in r10

//      Calculate filter output.
            imul G                       // multiply by g
            cmp edx, 0
            js  UB_F21                   // product negative?
            add edx, 0x00100000          //   no, round to nearest integer
            sar edx, 21                  //     truncate fraction in hi dword
            cmp dx, 0x007F
            jle UB_F22                   //     overflow max pos signal value?
            mov dx, 0x007F               //       yes, clamp at maximum
            jmp UB_F22
            UB_F21:  sub edx, 0x00100000          //   yes, round to nearest integer
            sar edx, 21                  //     truncate fraction in hi dword
            cmp dx, 0xFF80
            jge UB_F22                   //     overflow max neg signal value?
            mov dx, 0xFF80               //       yes, clamp at maximum
            UB_F22:  cmp dx, MAX                  // compare to maximum
            jle UB_F23
            mov MAX, dx                  // if greater, save as new max
            jmp UB_F24
            UB_F23:  cmp dx, MIN                  // compare to minimum
            jge UB_F24
            mov MIN, dx                  // if less, save as new minimum
            UB_F24:  xor dx, 0xFF80               // convert to unsigned byte
            mov BYTE PTR [ebx], dl       // store at pSig

//      Calculate r11.
            mov eax, R10                 // get r10
            imul B11                     // multiply by b11
            mov ebx, eax                 // save low dword of product
            mov ecx, edx                 // save hi dword
            mov eax, SIG                 // get sig
            shl eax,2                    // convert to 9.23 format
            imul F11                     // multiply by f11
            add eax, ebx                 // accumulate low dword of product
            add edx, ecx                 // accumulate hi dword
            add eax, R12L                // accumulate low dword of r12
            add edx, R12H                // accumulate hi dword of r12
            mov R11L, eax                // store low dword of r11
            mov R11H, edx                // store hi dword of r11

//      Calculate r12.
            mov eax, R10                 // get r10
            imul B12                     // multiply by b12
            /**********  this block of code removed because f12 = 1.0 ...*****************\
            |  mov ebx, eax                 // save low dword of product                    |
            |  mov ecx, edx                 // save hi dowrd                                |
            |  mov eax, SIG                 // get sig                                      |
            |  shl eax,2                    // convert to 9.23 format                       |
            |  imul  F12                    // multiply by f12                              |
            |  add eax, ebx                 // accumulate low dword of product              |
            |  add edx, ecx                 // accumulate hi dword of product               |
            \**********  replaced with the following...***********************************/
            add edx, SIG                 // add sig to hi dword of product, low unchanged
            /*****************************************************************************/
            mov R12L, eax                // store low dword of r12
            mov R12H, edx                // store hi dword of r12
        }
    }

    /* Store filter parameters in object.                                                  */
    m_r11H = Workspace.r11H;
    m_r11L = Workspace.r11L;
    m_r12H = Workspace.r12H;
    m_r12L = Workspace.r12L;
    m_nBlockMin = (short)(Workspace.nMin + 128);
    m_nBlockMax = (short)(Workspace.nMax + 128);

    return(DONE);
}


dspError_t CHighPassFilter70::ForwardPass(short * pProcessBuffer, uint32 dwProcessLength)
{
    /* Validate parameters.                                                               */
    if (!pProcessBuffer)
    {
        return(Code(INVALID_SIG_DATA_PTR));
    }
    if (!dwProcessLength)
    {
        return(Code(INVALID_SIG_LEN));
    }

    /* If first call, clear object internal registers.                                     */
    if (m_bFirstForwardPass)
    {
        m_r11H = 0;
        m_r11L = 0;
        m_r12H = 0;
        m_r12L = 0;

        m_bFirstForwardPass = false;
    }

    /* Transfer filter parameters from object to workspace.                                */
    HPF_WORKSPACE Workspace;

    Workspace.f11  = m_f11;
    Workspace.b11  = m_b11;
    Workspace.b12  = m_b12;
    Workspace.g    = m_g;
    Workspace.r11H = m_r11H;
    Workspace.r11L = m_r11L;
    Workspace.r12H = m_r12H;
    Workspace.r12L = m_r12L;

    /* Initialize minimum and maximum values for this block.                               */
    Workspace.nMin = 32767;
    Workspace.nMax = -32768;

    /* Perform forward pass filtering on signal waveform buffer in-place.                  */
    Workspace.pStart = (uint8 *)pProcessBuffer;
    Workspace.pEnd = Workspace.pStart + dwProcessLength*2 - 2;

    for (Workspace.pSig = Workspace.pStart; Workspace.pSig <= Workspace.pEnd; Workspace.pSig += 2)
    {
        __asm
        {
//      Calculate r10.
            mov ebx, DWORD PTR [PSIG]    // load pointer to signal sample
            movsx eax, WORD PTR [ebx]    // get signal sample
            shl eax,13                   // convert to 19.13 format
            mov SIG, eax                 // store in sig
            mov edx, R11L                // get low dword of r11
            add eax, R11H                // accumulate high dword of r11, low unchanged
            js  SS_F11                   // negative?
            add edx, 0x20000000          //   no, round to 15th binary place
            adc eax, 0                   //     carry into hi dword
            jmp SS_F12
            SS_F11:  sub edx, 0x20000000          //   yes, round to 15th binary place
            sbb eax, 0                   //     carry into hi dword
            SS_F12:  shld eax, edx, 2             // convert to 17.15 format
            mov R10, eax                 // store in r10

//      Calculate filter output.
            imul G                       // multiply by g
            cmp edx, 0
            js  SS_F21                   // product negative?
            add edx, 0x00001000          //   no, round to nearest integer
            sar edx, 13                  //     truncate fraction in hi dword
            cmp edx, 0x00007FFF
            jle SS_F22                   //     overflow max pos signal value?
            mov dx, 0x7FFF               //       yes, clamp at maximum
            jmp SS_F22
            SS_F21:  sub edx, 0x00001000          //   yes, round to nearest integer
            sar edx, 13                  //     truncate fraction in hi dword
            cmp edx, 0xFFFF8000
            jge SS_F22                   //     overflow max neg signal value?
            mov dx, 0x8000               //       yes, clamp at maximum
            SS_F22:  cmp dx, MAX                  // compare to maximum
            jle SS_F23
            mov MAX, dx                  // if greater, save as new max
            jmp SS_F24
            SS_F23:  cmp dx, MIN                  // compare to minimum
            jge SS_F24
            mov MIN, dx                  // if less, save as new minimum
            SS_F24:  mov WORD PTR [ebx], dx       // store at pSig

//      Calculate r11.
            mov eax, R10                 // get r10
            imul B11                     // multiply by b11
            mov ebx, eax                 // save low dword of product
            mov ecx, edx                 // save hi dword
            mov eax, SIG                 // get sig
            shl eax,2                    // convert to 17.15 format
            imul F11                     // multiply by f11
            add eax, ebx                 // accumulate low dword of product
            add edx, ecx                 // accumulate hi dword
            add eax, R12L                // accumulate low dword of r12
            add edx, R12H                // accumulate hi dword of r12
            mov R11L, eax                // store low dword of r11
            mov R11H, edx                // store hi dword of r11

//      Calculate r12.
            mov eax, R10                 // get r10
            imul B12                     // multiply by b12
            /**********  this block of code removed because f12 = 1.0 ...*****************\
            |  mov ebx, eax                 // save low dword of product                    |
            |  mov ecx, edx                 // save hi dowrd                                |
            |  mov eax, SIG                 // get sig                                      |
            |  shl eax,2                    // convert to 17.15 format                      |
            |  imul  F12                    // multiply by f12                              |
            |  add eax, ebx                 // accumulate low dword of product              |
            |  add edx, ecx                 // accumulate hi dword of product               |
            \**********  replaced with the following...***********************************/
            add edx, SIG                 // add sig to hi dword of product, low unchanged
            /*****************************************************************************/
            mov R12L, eax                // store low dword of r12
            mov R12H, edx                // store hi dword of r12
        }
    }

    /* Store filter parameters in object.                                                  */
    m_r11H = Workspace.r11H;
    m_r11L = Workspace.r11L;
    m_r12H = Workspace.r12H;
    m_r12L = Workspace.r12L;
    m_nBlockMin = Workspace.nMin;
    m_nBlockMax = Workspace.nMax;

    return(DONE);
}



////////////////////////////////////////////////////////////////////////////////////////
// Object function to perform backward-in-time high pass filtering on data in buffer. //
////////////////////////////////////////////////////////////////////////////////////////
#ifdef TEST_HPF
dspError_t CHighPassFilter70::DoBackwardPass(uint8 * pProcessBuffer, uint32 dwProcessLength)
#else
dspError_t CHighPassFilter70::BackwardPass(uint8 * pProcessBuffer, uint32 dwProcessLength)
#endif
{
    /* Validate parameters.                                                                */
    if (!pProcessBuffer)
    {
        return(Code(INVALID_SIG_DATA_PTR));
    }
    if (!dwProcessLength)
    {
        return(Code(INVALID_SIG_LEN));
    }

    /* If first call, clear object internal registers.                                     */
    if (m_bFirstBackwardPass)
    {
        m_r11H = 0;
        m_r11L = 0;
        m_r12H = 0;
        m_r12L = 0;

        m_bFirstBackwardPass = false;
    }

    /* Transfer filter parameters from object to workspace.                                */
    HPF_WORKSPACE Workspace;

    Workspace.f11  = m_f11;
    Workspace.b11  = m_b11;
    Workspace.b12  = m_b12;
    Workspace.g    = m_g;
    Workspace.r11H = m_r11H;
    Workspace.r11L = m_r11L;
    Workspace.r12H = m_r12H;
    Workspace.r12L = m_r12L;

    /* Initialize minimum and maximum values for this block.                               */
    Workspace.nMin = 127;
    Workspace.nMax = -128;

    /* Perform backward pass filtering on signal waveform buffer in-place.                 */
    Workspace.pStart = (uint8 *)pProcessBuffer;
    Workspace.pEnd = Workspace.pStart + dwProcessLength - 1;

    for (Workspace.pSig = Workspace.pEnd; Workspace.pSig >= Workspace.pStart; Workspace.pSig--)
    {
        __asm
        {

//      Calculate r10.
            mov ebx, DWORD PTR [PSIG]    // load pointer to signal sample
            movsx eax, BYTE PTR [ebx]    // get signal sample
            xor eax, 0xFFFFFF80          // convert to signed byte
            shl eax,21                   // convert to 11.21 format
            mov SIG, eax                 // store in sig
            mov edx, R11L                // get low dword of r11
            add eax, R11H                // accumulate high dword of r11, low unchanged
            js  UB_B11                   // negative?
            add edx, 0x20000000          //   no, round to 23rd binary place
            adc eax, 0                   //     carry into hi dword
            jmp UB_B12
            UB_B11:  sub edx, 0x20000000          //   yes, round to 23rd binary place
            sbb eax, 0                   //     carry into hi dword
            UB_B12:  shld eax, edx, 2             // convert to 9.23 format
            mov R10, eax                 // store in r10

//      Calculate filter output.
            imul G                       // multiply by g
            cmp edx, 0
            js  UB_B21                   // product negative?
            add edx, 0x00100000          //   no, round to nearest integer
            sar edx, 21                  //     truncate fraction in hi dword
            cmp dx, 0x007F
            jle UB_B22                   //     overflow max pos signal value?
            mov dx, 0x007F               //       yes, clamp at maximum
            jmp UB_B22
            UB_B21:  sub edx, 0x00100000          //   yes, round to nearest integer
            sar edx, 21                  //     truncate fraction in hi dword
            cmp dx, 0xFF80
            jge UB_B22                   //     overflow max neg signal value?
            mov dx, 0xFF80               //       yes, clamp at maximum
            UB_B22:  cmp dx, MAX                  // compare to maximum
            jle UB_B23
            mov MAX, dx                  // if greater, save as new max
            jmp UB_B24
            UB_B23:  cmp dx, MIN                  // compare to minimum
            jge UB_B24
            mov MIN, dx                  // if less, save as new minimum
            UB_B24:  xor dx, 0xFF80               // convert to unsigned byte
            mov BYTE PTR [ebx], dl       // store at pSig

//      Calculate r11.
            mov eax, R10                 // get r10
            imul B11                     // multiply by b11
            mov ebx, eax                 // save low dword of product
            mov ecx, edx                 // save hi dword
            mov eax, SIG                 // get sig
            shl eax,2                    // convert to 9.23 format
            imul F11                     // multiply by f11
            add eax, ebx                 // accumulate low dword of product
            add edx, ecx                 // accumulate hi dword
            add eax, R12L                // accumulate low dword of r12
            add edx, R12H                // accumulate hi dword of r12
            mov R11L, eax                // store low dword of r11
            mov R11H, edx                // store hi dword of r11

//      Calculate r12.
            mov eax, R10                 // get r10
            imul B12                     // multiply by b12
            /**********  this block of code removed because f12 = 1.0 ...*****************\
            |  mov ebx, eax                 // save low dword of product                    |
            |  mov ecx, edx                 // save hi dowrd                                |
            |  mov eax, SIG                 // get sig                                      |
            |  shl eax,2                    // convert to 9.23 format                       |
            |  imul  F12                    // multiply by f12                              |
            |  add eax, ebx                 // accumulate low dword of product              |
            |  add edx, ecx                 // accumulate hi dword of product               |
            \**********  replaced with the following...***********************************/
            add edx, SIG                 // add sig to hi dword of product, low unchanged
            /*****************************************************************************/
            mov R12L, eax                // store low dword of r12
            mov R12H, edx                // store hi dword of r12
        }
    }

    /* Store filter parameters in object.                                                  */
    m_r11H = Workspace.r11H;
    m_r11L = Workspace.r11L;
    m_r12H = Workspace.r12H;
    m_r12L = Workspace.r12L;
    m_nBlockMin = (short)(Workspace.nMin + 128);
    m_nBlockMax = (short)(Workspace.nMax + 128);

    return(DONE);
}


#ifdef TEST_HPF
dspError_t CHighPassFilter70::DoBackwardPass(short * pProcessBuffer, uint32 dwProcessLength)
#else
dspError_t CHighPassFilter70::BackwardPass(short * pProcessBuffer, uint32 dwProcessLength)
#endif
{
    /* Validate parameters.                                                                */
    if (!pProcessBuffer)
    {
        return(Code(INVALID_SIG_DATA_PTR));
    }
    if (!dwProcessLength)
    {
        return(Code(INVALID_SIG_LEN));
    }

    /* If first call, clear object internal registers.                                     */
    if (m_bFirstBackwardPass)
    {
        m_r11H = 0;
        m_r11L = 0;
        m_r12H = 0;
        m_r12L = 0;

        m_bFirstBackwardPass = false;
    }

    /* Transfer filter parameters from object to workspace.                                */
    HPF_WORKSPACE Workspace;

    Workspace.f11  = m_f11;
    Workspace.b11  = m_b11;
    Workspace.b12  = m_b12;
    Workspace.g    = m_g;
    Workspace.r11H = m_r11H;
    Workspace.r11L = m_r11L;
    Workspace.r12H = m_r12H;
    Workspace.r12L = m_r12L;

    /* Initialize minimum and maximum values for this block.                               */
    Workspace.nMin = 32767;
    Workspace.nMax = -32768;

    /* Perform backward pass filtering on signal waveform buffer in-place.                 */
    Workspace.pStart = (uint8 *)pProcessBuffer;
    Workspace.pEnd = Workspace.pStart + dwProcessLength*2 - 2;

    for (Workspace.pSig = Workspace.pEnd; Workspace.pSig >= Workspace.pStart; Workspace.pSig -= 2)
    {
        __asm
        {

//      Calculate r10.
            mov ebx, DWORD PTR [PSIG]    // load pointer to signal sample
            movsx eax, WORD PTR [ebx]    // get signal sample
            shl eax,13                   // convert to 19.13 format
            mov SIG, eax                 // store in sig
            mov edx, R11L                // get low dword of r11
            add eax, R11H                // accumulate high dword of r11, low unchanged
            js  SS_B11                   // negative?
            add edx, 0x20000000          //   no, round to 15th binary place
            adc eax, 0                   //     carry into hi dword
            jmp SS_B12
            SS_B11:  sub edx, 0x20000000          //   yes, round to 15th binary place
            sbb eax, 0                   //     carry into hi dword
            SS_B12:  shld eax, edx, 2             // convert to 17.15 format
            mov R10, eax                 // store in r10

//      Calculate filter output.
            imul G                       // multiply by g
            cmp edx, 0
            js  SS_B21                   // product negative?
            add edx, 0x00001000          //   no, round to nearest integer
            sar edx, 13                  //     truncate fraction in hi dword
            cmp edx, 0x00007FFF
            jle SS_B22                   //     overflow max pos signal value?
            mov dx, 0x7FFF               //       yes, clamp at maximum
            jmp SS_B22
            SS_B21:  sub edx, 0x00001000          //   yes, round to nearest integer
            sar edx, 13                  //     truncate fraction in hi dword
            cmp edx, 0xFFFF8000
            jge SS_B22                   //     overflow max neg signal value?
            mov dx, 0x8000               //       yes, clamp at maximum
            SS_B22:  cmp dx, MAX                  // compare to maximum
            jle SS_B23
            mov MAX, dx                  // if greater, save as new max
            jmp SS_B24
            SS_B23:  cmp dx, MIN                  // compare to minimum
            jge SS_B24
            mov MIN, dx                  // if less, save as new minimum
            SS_B24:  mov WORD PTR [ebx], dx       // store at pSig

//      Calculate r11.
            mov eax, R10                 // get r10
            imul B11                     // multiply by b11
            mov ebx, eax                 // save low dword of product
            mov ecx, edx                 // save hi dword
            mov eax, SIG                 // get sig
            shl eax,2                    // convert to 17.15 format
            imul F11                     // multiply by f11
            add eax, ebx                 // accumulate low dword of product
            add edx, ecx                 // accumulate hi dword
            add eax, R12L                // accumulate low dword of r12
            add edx, R12H                // accumulate hi dword of r12
            mov R11L, eax                // store low dword of r11
            mov R11H, edx                // store hi dword of r11

//      Calculate r12.
            mov eax, R10                 // get r10
            imul B12                     // multiply by b12
            /**********  this block of code removed because f12 = 1.0 ...*****************\
            |  mov ebx, eax                 // save low dword of product                    |
            |  mov ecx, edx                 // save hi dowrd                                |
            |  mov eax, SIG                 // get sig                                      |
            |  shl eax,2                    // convert to 17.15 format                      |
            |  imul  F12                    // multiply by f12                              |
            |  add eax, ebx                 // accumulate low dword of product              |
            |  add edx, ecx                 // accumulate hi dword of product               |
            \**********  replaced with the following...***********************************/
            add edx, SIG                 // add sig to hi dword of product, low unchanged
            /*****************************************************************************/
            mov R12L, eax                // store low dword of r12
            mov R12H, edx                // store hi dword of r12
        }
    }

    /* Store filter parameters in object.                                                  */
    m_r11H = Workspace.r11H;
    m_r11L = Workspace.r11L;
    m_r12H = Workspace.r12H;
    m_r12L = Workspace.r12L;
    m_nBlockMin = Workspace.nMin;
    m_nBlockMax = Workspace.nMax;

    return(DONE);
}


////////////////////////////////////////////////////////////////////////////////////////
// Test driver for filter.                                                            //
//    Produces sine wave test patterns for 8 and 16 bit data at Windows supported     //
//    sampling rates, which are run forward and backward in time through the highpass //
//    filter and stored in text files HPsrKbBu.frq, where sr is the sampling rate,    //
//    b is the sample size in bits, u is present for unsigned data, and frq is  the   //
//    sine wave frequency (7, 70, or 700 Hz).                                         //
////////////////////////////////////////////////////////////////////////////////////////
#pragma optimize( "", on   )

#ifdef TEST_HPF
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "mathx.h"

dspError_t CHighPassFilter70::BackwardPass(uint8 * pProcessBuffer, uint32 dwProcessLength)
{
    if (m_bFirstTestPass)
    {
        GenTestSuite((void *)pProcessBuffer, dwProcessLength, sizeof(uint8));
    }
    else
    {
        DoBackwardPass((uint8 *)pProcessBuffer, dwProcessLength);
        m_bFirstTestPass = false;
    }
    return(DONE);
}

dspError_t CHighPassFilter70::BackwardPass(short * pProcessBuffer, uint32 dwProcessLength)
{
    if (m_bFirstTestPass)
    {
        GenTestSuite((void *)pProcessBuffer, dwProcessLength, sizeof(short));
    }
    else
    {
        DoBackwardPass((short *)pProcessBuffer, dwProcessLength);
        m_bFirstTestPass = false;
    }
    return DONE;
}

void CHighPassFilter70::GenTestSuite(void * pProcessBuffer, uint32 dwProcessLength, uint16 wSmpSize)
{
    uint16 wSmpRate[5] = {11025, 16000, 22050, 44100, 48000};
    uint16 wSigFreq[3] = {7, 70, 700};
    char FileName[13];
    FILE * FltrOut;

    uint32 dwBfrSize = dwProcessLength * wSmpSize;
    for (uint16 i = 0; i < sizeof(wSmpRate)/sizeof(wSmpRate[0]); i++)
    {
        for (uint16 j = 0; j < sizeof(wSigFreq)/sizeof(wSigFreq[0]); j++)
        {
            // Test 8 bit unsigned sample data.
            uint32 dwSineLength = dwBfrSize / sizeof(uint8);

            sprintf(FileName, "hp%uk%dbu.%d", wSmpRate[i]/1000,
                    (int32)sizeof(uint8)*8, (int32)wSigFreq[j]);
            FltrOut = fopen(FileName, "w");

            for (uint32 n = 0; n < dwSineLength; n++)
                ((uint8 *)pProcessBuffer)[n] = (uint8)Round((127.*sin(2.*PI*(double)wSigFreq[j]*
                                               (double)n/(double)wSmpRate[i]) + 128.));
            SetFilterParms(wSmpRate[i]);
            Filter((uint8 *)pProcessBuffer, dwSineLength);

            for (n = 0; n < dwSineLength; n++)
            {
                fprintf(FltrOut, "%d ", (short)((uint8 *)pProcessBuffer)[n]);
            }

            fclose(FltrOut);


            // Test 16 bit signed sample data.
            dwSineLength = dwBfrSize / sizeof(short);

            sprintf(FileName, "hp%uk%db.%d", wSmpRate[i]/1000,
                    (int32)wSmpSize*8, (int32)wSigFreq[j]);

            FltrOut = fopen(FileName, "w");

            for (n = 0; n < dwSineLength; n++)
                ((short *)pProcessBuffer)[n] = (short)Round((32767.*sin(2.*PI*(double)wSigFreq[j]*
                                               (double)n/(double)wSmpRate[i])));
            SetFilterParms(wSmpRate[i]);
            Filter((short *)pProcessBuffer, dwSineLength);

            for (n = 0; n < dwSineLength; n++)
            {
                fprintf(FltrOut, "%d ", ((short *)pProcessBuffer)[n]);
            }

            fclose(FltrOut);
        }
    }
}

#endif
