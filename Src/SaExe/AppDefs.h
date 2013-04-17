//{{NO_DEPENDENCIES}}
// Application specific include file.
// Included by stdafx.h
// copyright 2000 JAARS Inc. SIL
//
// Revision History
//   1.06.5
//        SDM added user messages WM_USER_CHANGEANNOTATION, WM_USER_CHANGECURSOR
//            added defines for cursor scrolling and phonetic segment add
//   1.06.6
//        SDM added SLOW_CLICK_TIME_LIMIT
//   1.06.6U2
//        SDM added ADD_SYLLABLE_TIME
//        SDM removed nWordCount from SaParm
//        SDM added MIN_EDIT_TIME
//   1.06.8
//        SDM added SPACE_DELIMITER & EDIT_DELIMITER_REPLACEMENT
//   1.5Test8
//        SDM added changes by CLW in 1.07a
//   1.5Test8.1
//        SDM Changed to RIFF version 7.1
//        SDM added szFreeTranslation and szTranscriber to SourceParm
//        SDM added reference annotation
//   1.5Test8.2
//        SDM added WM_USER_GRAPH_ORDERCHANGED
//        SDM added MAX_AUTOSEGMENT_MOVE_TIME
//   1.5Test8.3
//        SDM added EXPORT_FORMANT_SIZE
//   1.5Test10.0
//        SDM removed unused code
//        SDM moved CParseParm & CSegmentParm to Param.h & Param.cpp
//   1.5Test10.2
//        SDM added MAX_ADD_JOIN_TIME
//   1.5Test11.1A
//        RLJ Changed CPitchParm, CSpectroParm, and CSpectrumParm from
//              structures here to classes in Param.h.
//   05/31/2000
//        RLJ Added OpenModeType
/////////////////////////////////////////////////////////////////////////////
#ifndef _APPDEFS_H
#define _APPDEFS_H

// system defines
#define SM_CAPTION                      _T("Phonology Assistant") // caption text of SM  //Name changed  -ALB
//#define RIFF_VERSION                    ((float)9.0)// RIFF structure version
#define RIFF_VERSION_UNKNOWN            ((float)-1.0) // RIFF structure version
#define MMIO_BUFFER_SIZE                16384       // buffer size for wave data
#define MAX_SCREEN_WIDTH                2048        // maximum width of screen in pixels

// data processing defines
enum ESearchOptions
{
    // search options
    LINEAR_SEARCH,
    BINARY_SEARCH
};

enum EUpdateMode
{
    // graph update mode
    STATIC_UPDATE =  0,
    DYNAMIC_UPDATE = 1
};

#define UNDEFINED_DATA                  -1          // data not yet defined
#define UNDEFINED_OFFSET                0xFFFFFFFF  // offset not yet defined
#define CALCULATION_INTERVAL(samplingRate)            (100*22050/samplingRate)         // number of raw data samples taken to build a processed data result
#define ZCROSS_SAMPLEWINDOWWIDTH(samplingRate)        CALCULATION_INTERVAL(samplingRate)  // number of samples in the calculation window to calculate zero crossing
#define CALCULATION_DATALENGTH(samplingRate)          (300*22050/samplingRate)         // length of data used for calculation
#define MAX_CALCULATION_FREQUENCY       500         // permissible parameter range
#define PRECISION_MULTIPLIER            10          // multiplier to amount precision
#define MEL_MULT                        100         // multiplier for melogram data
#define MEL_ACCURACY                    5           // accuracy of melogram data in cents
#define MAX_ECHO_DELAY                  1000        // maximal echo delay in milliseconds
#define MAX_ANIMATION_RATE              60          // animation rate limited only by processor speed

// annotation windows defines
#define PHONETIC_DEFAULT_FONT           _T("Doulos SIL") // default font faces
#define TONE_DEFAULT_FONT               _T("Doulos SIL")
#define PHONEMIC_DEFAULT_FONT           _T("Doulos SIL")
#define ORTHOGRAPHIC_DEFAULT_FONT       _T("Doulos SIL")
#define GLOSS_DEFAULT_FONT              _T("Doulos SIL")
#define REFERENCE_DEFAULT_FONT          _T("Doulos SIL")
#define MUSIC_PHRASE_DEFAULT_FONT       _T("Doulos SIL")

#define PHONETIC_DEFAULT_FONTSIZE       12          // default font point sizes
#define TONE_DEFAULT_FONTSIZE           18
#define PHONEMIC_DEFAULT_FONTSIZE       12
#define ORTHOGRAPHIC_DEFAULT_FONTSIZE   10
#define GLOSS_DEFAULT_FONTSIZE          10
#define REFERENCE_DEFAULT_FONTSIZE      12
#define MUSIC_PHRASE_DEFAULT_FONTSIZE   12

#define WORD_DELIMITER                  _T('#')         // '#' delimits words in text annotation windows
#define TEXT_DELIMITER                  _T('!')         // '!' delimits text in text annotation windows
#define SPACE_DELIMITER             0x20            // ' ' space
#define EDIT_DELIMITER_REPLACEMENT      _T('.')         // Replace Delimiters with this character

#define SEGMENT_DEFAULT_CHAR            _T('\xFFFD')    // default 0xFFFD

#define MIN_EDIT_SEGMENT_TIME           (0.005 /*5ms*/)
#define MIN_ADD_SEGMENT_TIME            (0.020 /*20ms*/)
#define DEFAULT_ADD_SEGMENT_TIME        (0.100 /*100ms*/)
#define MAX_AUTOSEGMENT_MOVE_GLOSS_TIME (0.050 /*50ms*/)
#define ADD_SYLLABLE_TIME               (0.010 /*10ms*/)
#define CURSOR_MOVE_AUTOREPEAT_FIRST    (0.500 /*500ms*/)
#define MAX_ADD_JOIN_TIME               (0.010 /*10ms*/)
#define CURSOR_MOVE_AUTOREPEAT_RATE     (0.25 /*Scroll Quarter Speed of Recording*/)
#define SLOW_CLICK_TIME_LIMIT           (3.0 /* 3 Seconds */)
#define EXPORT_FORMANT_SIZE             (0.005 /* 5 mSec*/)
// view layout
#define MAX_GRAPHS_NUMBER               10           // max number of graphs in MDI child window
#define LINE_SCROLL_PIXELWIDTH          4           // number of pixels to scroll on line scroll
#define ZOOM_SCROLL_RESOLUTION          10          // zoom scroll steps per page (window)
#define CURSOR_WINDOW_HALFWIDTH         8           // half width of cursor windows
#define CURSOR_MIN_DISTANCE             CURSOR_WINDOW_HALFWIDTH // minimum distance between cursors
#define STATUSPANE_1_WIDTH              95          // width of the data statusbar panes
#define STATUSPANE_2_WIDTH              95
#define STATUSPANE_3_WIDTH              95
#define STATUSPANE_4_WIDTH              95
#define PROGRESSPANE_2_WIDTH            70          // width of the progress statusbar panes
#define PROGRESSPANE_3_WIDTH            150

// custom menu defines
#define ID_LAYOUT_FIRST                 0x7000
#define ID_LAYOUT_1                     ID_LAYOUT_FIRST + 0
#define ID_LAYOUT_2A                    ID_LAYOUT_FIRST + 1
#define ID_LAYOUT_2B                    ID_LAYOUT_FIRST + 2
#define ID_LAYOUT_2C                    ID_LAYOUT_FIRST + 3
#define ID_LAYOUT_3A                    ID_LAYOUT_FIRST + 4
#define ID_LAYOUT_3B                    ID_LAYOUT_FIRST + 5
#define ID_LAYOUT_3C                    ID_LAYOUT_FIRST + 6
#define ID_LAYOUT_4A                    ID_LAYOUT_FIRST + 7
#define ID_LAYOUT_4B                    ID_LAYOUT_FIRST + 8
#define ID_LAYOUT_4C                    ID_LAYOUT_FIRST + 9
#define ID_LAYOUT_5                     ID_LAYOUT_FIRST + 10
#define ID_LAYOUT_6A                    ID_LAYOUT_FIRST + 11
#define ID_LAYOUT_6B                    ID_LAYOUT_FIRST + 12
#define ID_LAYOUT_LAST                  ID_LAYOUT_FIRST + 12

// cursor definitions
enum ECursorSelect
{
    // selection
	UNDEFINED_CURSOR = -1,
    START_CURSOR = 1,
    STOP_CURSOR = 2
};

enum ESnapDirection
{
    // direction:
    SNAP_BOTH  = 0x0000,  //  snaps in both directions (must be 0)
    SNAP_RIGHT = 0x0001,  //  snaps only to the right
    SNAP_LEFT  = 0x0002,  //  snaps only to the left
};
enum ECursorAlignment
{
    // alignment
    ALIGN_AT_SAMPLE         =  0,
    ALIGN_AT_ZERO_CROSSING  =  1,
    ALIGN_AT_FRAGMENT       =  2,
    ALIGN_AT_SEGMENT        =  3,   // !!future use
    ALIGN_USER_SETTING       =  1000 // use global user setting
};

// RIFF header definitions
#define FILE_FORMAT_PCM                 1           // PCM format in format chunk
#define FILE_FORMAT_UTT                 0           // record file format in sa chunk
#define FILE_FORMAT_WAV                 1           // record file format in sa chunk
#define FILE_FORMAT_MAC                 2           // record file format in sa chunk
#define FILE_FORMAT_TIMIT               3           // record file format in sa chunk
#define FILE_FORMAT_OTHER               4           // record file format in sa chunk
#define FILE_DESCRIPTION_SIZE           256         // file description string length
// user windows messages
#define WM_USER_SPEECHAPPLICATION       0x7FF0
#define SPEECH_WPARAM_SHOWSA            0xE001
#define SPEECH_WPARAM_SHOWSM            0xE002
#define SPEECH_WPARAM_CLOSE             0xE003
#define SPEECH_WPARAM_SHOWSAREC         0xE004
#define WM_USER_CURSOR_IN_FRAGMENT      WM_USER + 5
#define WM_USER_APPLY_TOOLSOPTIONS      WM_USER + 6
#define WM_USER_RECORDER                WM_USER + 7
#define WM_USER_PLAYER                  WM_USER + 8
#define WM_USER_SETUP_FNKEYS            WM_USER + 9
#define WM_USER_CHANGEVIEW              WM_USER + 10
#define WM_USER_GRAPH_DESTROYED         WM_USER + 11
#define WM_USER_GRAPH_STYLECHANGED      WM_USER + 12
#define WM_USER_GRAPH_GRIDCHANGED       WM_USER + 13
#define WM_USER_GRAPH_COLORCHANGED      WM_USER + 14
#define WM_USER_GRAPH_FONTCHANGED       WM_USER + 15
#define WM_USER_VIEW_SCROLLZOOMCHANGED  WM_USER + 16
#define WM_USER_VIEW_GRAPHUPDATECHANGED WM_USER + 17
#define WM_USER_VIEW_ANIMATIONCHANGED   WM_USER + 18
#define WM_USER_CANCEL_ANIMATION        WM_USER + 19
#define WM_USER_VIEW_CURSORALIGNCHANGED WM_USER + 20
#define WM_USER_INFO_GENDERCHANGED      WM_USER + 21

//SDM 1.06.5
#define WM_USER_IDLE_UPDATE             WM_USER + 22
#define WM_USER_GRAPH_ORDERCHANGED      WM_USER + 23 // SDM 1.5Test8.1
#define WM_USER_APP_MESSAGE             WM_USER + 24

#define WM_USER_AUTO_RESTART            WM_USER + 27
#define WM_USER_AUTOSAVE                WM_USER + 28

#define HINT_APP_DOC_LIST_CHANGED       0x123

// SA parameter flag mask
#define SA_FLAG_HIGHPASS                0x0020

// CECIL pitch default settings
#define CECIL_PITCH_MAXFREQ             500 // CLW changed back 4/27/00
#define CECIL_PITCH_MINFREQ             40
#define CECIL_PITCH_VOICING             32
#define CECIL_PITCH_CHANGE              10
#define CECIL_PITCH_GROUP               6
#define CECIL_PITCH_INTERPOL            7
// workbench defines
#define MAX_PROCESS_NUMBER              4           // number of workbench processes
#define MAX_FILTER_NUMBER               3           // number of filters in workbench processes
// global structure defines
typedef UINT MMRESULT;                  // MMIO function call results

// timer IDs
#define ID_TIMER_AUTOSAVE               0x1000
#define ID_TIMER_PLAYBACK				0x1001
#define ID_TIMER_DELAY					0x1002
#define ID_TIMER_MIDI					0x1003
#define ID_TIMER_FLASH					0x1004

struct SDPParm
{
    UINT         nPanes;            // number of mirror panes
    UINT         nUpperBound;       // upper display boundary
    int          nStepMode;         // steps mode
    UINT         nSteps;            // incremental steps in samples
    BOOL         bAverage;          // TRUE, if average used
};                                  // SDP parameters

enum EAnnotation                    // annotation windows
{
    PHONETIC=0,
    TONE,
    PHONEMIC,
    ORTHO,
    GLOSS,
    REFERENCE,
    MUSIC_PL1,
    MUSIC_PL2,
    MUSIC_PL3,
    MUSIC_PL4,
    ANNOT_WND_NUMBER                    // number of annotation windows
};

enum EPositionReadout                   // status bar position readout mode
{
    TIME,
    SAMPLES,
    BYTES,
};

enum EPitchReadout                      // status bar pitch readout mode
{
    HERTZ,
    HALFTONES,
};

enum EGraphCaption                       // graph caption style
{
    NoneThin,
    None,
    Mini,
    MiniWithCaption,
    Normal,
};

enum EOpenMode
{
    DEFAULT,
    PHONANALYSIS,
    MUSIANALYSIS,
};

enum EBoundary
{
    BOUNDARIES_EDIT_NULL = 0,
    BOUNDARIES_EDIT_SEGMENT_SIZE = 1,
    BOUNDARIES_EDIT_BOUNDARIES = 2
};

enum EWordFilenameConvention
{
    WFC_REF,
    WFC_GLOSS,
    WFC_REF_GLOSS
};

enum EPhraseFilenameConvention
{
    PFC_REF,
    PFC_GLOSS,
    PFC_REF_GLOSS,
    PFC_PHRASE
};

struct RGB
{
    long r;
    long g;
    long b;
};

#include "grid.h"
#include "param.h"
#endif  //_APPDEFS_H
