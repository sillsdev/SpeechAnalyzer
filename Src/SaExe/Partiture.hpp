/////////////////////////////////////////////////////////////////////////////
// Partiture.hpp:
// Interface for Music Staff Control.
// Author: Tim Erickson
// copyright 2000 JAARS Inc. SIL
//
// Revision History
// 07/26/2000
//    RLJ Add NotesUp, NotesDown, GoHome procedures
// 08/28/2000
//    TRE Add Unicode support for 32-bit
//
////////////////////////////////////////////////////////////////////////////

#import "speechtoolsutils.tlb" no_namespace named_guids

#ifndef PARTITURE_HPP
#define PARTITURE_HPP

extern "C" {

#define DELTA 64

#define NUMBER_OF_NAMES ((unsigned char)7)
#define NAME_C 0
#define NAME_D 1
#define NAME_E 2
#define NAME_F 3
#define NAME_G 4
#define NAME_A 5
#define NAME_B 6

#define NUMBER_OF_ACCIDENTALS ((unsigned char)9)
#define DOUBLE_FLAT 0
#define DOTTED_FLAT 1
#define FLAT 2
#define HALF_FLAT 3
#define NATURAL 4
#define HALF_SHARP 5
#define SHARP 6
#define DOTTED_SHARP 7
#define DOUBLE_SHARP 8

#define OCTAVE (NUMBER_OF_ACCIDENTALS*NUMBER_OF_NAMES)
#define OCTAVE_0 0
#define OCTAVE_1 (1*OCTAVE)
#define OCTAVE_2 (2*OCTAVE)
#define OCTAVE_3 (3*OCTAVE)
#define OCTAVE_4 (4*OCTAVE)
#define OCTAVE_5 (5*OCTAVE)
#define OCTAVE_6 (6*OCTAVE)
#define OCTAVE_7 (7*OCTAVE)

#define ANNOT_SPACE '&'
#define ANNOT_PHRASE2 ';'
#define ANNOT_PHRASE ','
#define ANNOT_BAR '['
#define ANNOT_THICK_BAR ']'
#define ANNOT_UNDEFINED '^'

#define DISPLAY_WHOLE ((WCHAR)(0xE012))
#define DISPLAY_WHOLE_REST ((WCHAR)(0xE010))
#define DISPLAY_WHOLE_TOO_HIGH ((WCHAR)(0xE011))
#define DISPLAY_WHOLE_TOO_LOW ((WCHAR)(0xE011))
#define DISPLAY_HALF ((WCHAR)(0xE027))
#define DISPLAY_HALF_REST ((WCHAR)(0xE025))
#define DISPLAY_HALF_TOO_HIGH ((WCHAR)(0xE026))
#define DISPLAY_HALF_TOO_LOW ((WCHAR)(0xE026))
#define DISPLAY_QUARTER ((WCHAR)(0xE03C))
#define DISPLAY_QUARTER_REST ((WCHAR)(0xE03A))
#define DISPLAY_QUARTER_TOO_HIGH ((WCHAR)(0xE03B))
#define DISPLAY_QUARTER_TOO_LOW ((WCHAR)(0xE03B))
#define DISPLAY_EIGHTH ((WCHAR)(0xE051))
#define DISPLAY_EIGHTH_REST ((WCHAR)(0xE04F))
#define DISPLAY_EIGHTH_TOO_HIGH ((WCHAR)(0xE050))
#define DISPLAY_EIGHTH_TOO_LOW ((WCHAR)(0xE050))
#define DISPLAY_SIXTEENTH ((WCHAR)(0xE066))
#define DISPLAY_SIXTEENTH_REST ((WCHAR)(0xE064))
#define DISPLAY_SIXTEENTH_TOO_HIGH ((WCHAR)(0xE065))
#define DISPLAY_SIXTEENTH_TOO_LOW ((WCHAR)(0xE065))
#define DISPLAY_GRACE ((WCHAR)(0xE07B))
#define DISPLAY_GRACE_REST ((WCHAR)(0xE079))
#define DISPLAY_GRACE_TOO_HIGH ((WCHAR)(0xE07A))
#define DISPLAY_GRACE_TOO_LOW ((WCHAR)(0xE07A))
#define DISPLAY_TRIPLET ((WCHAR)(0x2083))
#define DISPLAY_TRIPLET_ABOVE ((WCHAR)(0x00B3))
#define DISPLAY_QUINTUPLET ((WCHAR)(0x2085))
#define DISPLAY_QUINTUPLET_ABOVE ((WCHAR)(0x2075))
#define DISPLAY_TREBLE_8VA ((WCHAR)(0xE004))
#define DISPLAY_TREBLE ((WCHAR)(0xE005))
#define DISPLAY_TREBLE_8VB ((WCHAR)(0xE006))
#define DISPLAY_BASS ((WCHAR)(0xE007))
#define DISPLAY_BASS_8VB ((WCHAR)(0xE008))
#define DISPLAY_DOTTED_FLAT ((WCHAR)(0xE090))
#define DISPLAY_FLAT ((WCHAR)(0xE0A3))
#define DISPLAY_HALF_FLAT ((WCHAR)(0xE0B6))
#define DISPLAY_NATURAL ((WCHAR)(0x0388))
#define DISPLAY_HALF_SHARP ((WCHAR)(0xE0C9))
#define DISPLAY_SHARP ((WCHAR)(0xE0DC))
#define DISPLAY_DOTTED_SHARP ((WCHAR)(0xE0EF))
#define DISPLAY_DOT ((WCHAR)(0xE102))
#define DISPLAY_LONG_RISE ((WCHAR)(0xE110))
#define DISPLAY_LONG_FALL ((WCHAR)(0xE113))
#define DISPLAY_SHORT_RISE ((WCHAR)(0xE116))
#define DISPLAY_SHORT_FALL ((WCHAR)(0xE119))
#define DISPLAY_APOSTROPHE ((WCHAR)(0xE000))
#define DISPLAY_QUOTE ((WCHAR)(0xE001))
#define DISPLAY_TIE ((WCHAR)(0xE11C))
#define DISPLAY_BAR ((WCHAR)(0xE002))
#define DISPLAY_THICK_BAR ((WCHAR)(0xE003))
#define DISPLAY_SMALL_SPACE ((WCHAR)(0xE00A))
#define DISPLAY_CONTINUED ((WCHAR)(0xE00B))
#define DISPLAY_UNDEFINED ((WCHAR)(0xE009))
#define DISPLAY_PLAY_ICON ((WCHAR)(0x25B6))
#define DISPLAY_NOTE_ICON ((WCHAR)(0xE059)) //(0x1E85))
#define DISPLAY_STOP_ICON ((WCHAR)(0x25A0))

#define DISPLAY_BKG GetSysColor(COLOR_WINDOW)
#define DISPLAY_ACTIVE_TEXT GetSysColor(COLOR_WINDOWTEXT)
#define DISPLAY_INACTIVE_TEXT GetSysColor(COLOR_GRAYTEXT)
#define DISPLAY_SELECT_BKG GetSysColor(COLOR_HIGHLIGHT)
#define DISPLAY_SELECT_TEXT GetSysColor(COLOR_HIGHLIGHTTEXT)
#define DISPLAY_CURSOR GetSysColor(COLOR_ACTIVECAPTION)

#define MUSIQUE_FONT_SIZE (0xE120)

#define IDM_PLAY 101
#define IDM_PAUSE 102
#define IDM_STOP 103
#define IDM_LOOP 104

#define PLAY_TIMER_INTERVAL (100)

typedef unsigned char  UBYTE;
typedef unsigned short USHORT;
typedef unsigned long  ULONG;

// **********
// STRUCTURES
// **********

union CHNK_ID {
  ULONG FOURCC;
  char Name[4];
};

struct CHNK_HDR {
  CHNK_ID Id;
  ULONG Size;
  CHNK_HDR(){Id.FOURCC=0L;Size=0L;};
};

#pragma pack(1)
struct MTHDCHNK {
  CHNK_HDR Header;
  WORD Format;
  WORD NTrks;
  WORD Division;
  MTHDCHNK(){Format=NTrks=Division=0;};
};
#pragma pack()

enum MusiqueBytes{
  Name,
    Accidental,
    Octave,
    Onset,
    Duration,
    ModDuration,
    Conclusion
};

struct Note
{
  unsigned char Name; // 'A' to 'G' or 'R'
  unsigned char Accidental; // "$#+=-@!"
  unsigned char Octave; // '0' to '6'
  unsigned char Onset; // "*|~?\""
  // (up long, up short, down long, down short, normal)
  unsigned char Duration; // "whqisz"
  // (whole, half, quarter, eighth, 16th, grace)
  unsigned char Duration2; // "n.tv"
  // (normal, dotted, triplet,quintuplet)
  unsigned char Conclusion; // "\\`/\' "
  // (up long, up short, down long, down short, normal)
  // or '_' (tied to next note)

  int Value; // 0-900 or -1 (rest)
  WCHAR DisplayOnset, DisplayAccidental, DisplayModDur,
    DisplayNote, DisplayConclusion;
  unsigned DisplayLen(void)
  { return (DisplayOnset!=0) + (DisplayAccidental!=0) +
  (DisplayModDur!=0) + (DisplayNote!=0) + (DisplayConclusion!=0); }


  Note(int Clef=1);
  Note(const unsigned char *Definition,int Clef);
  ~Note();
  Note &operator=(Note &n);
  void CalcValue(int Clef);
  void CalcOnset(int Clef);
  void CalcAccidental(int Clef);
  void CalcModDuration(int Clef);
  void CalcConclusion(int Clef);
  BOOL SetClef(int Clef);
  BOOL Set(unsigned char c,int Clef);
  BOOL Set(const unsigned char *Definition,int Clef);
  BOOL UpStep(void);
  BOOL DownStep(void);
  BOOL UpOctave(void);
  BOOL DownOctave(void);
  BOOL InClef(int Clef);
  unsigned MIDINote();
  unsigned long MIDIDuration();
};//Note

typedef Note *pNote;

struct PartSelectionMS
{
  double begin;
  double end;
};

struct PartWindowData
{
  BOOL Active;
  BOOL Selecting;
  BOOL Control;
  BOOL Enabled;
  unsigned SelectBegin;
  unsigned SelectEnd;
  unsigned SelectMin() {return min(SelectBegin,SelectEnd);};
  unsigned SelectMax() {return max(SelectBegin,SelectEnd);};
  struct
  {
    int Tempo;
    unsigned nMin;
    unsigned nMax;
    unsigned SelectBegin;
    unsigned SelectEnd;
    unsigned ViewBegin;
    DWORD dwStartTime;
    PartSelectionMS selectionTime;
  } m_sPlay;
  PartSelectionMS m_sSelectionMS;
  unsigned ViewBegin;
  unsigned ViewWidthPixels;
  pNote *Melody;
  unsigned MelodySize;
  unsigned MelodyBufferSize;
  HFONT hFont;
  int FontCharWidth[MUSIQUE_FONT_SIZE];
  int Clef;
  int Tempo;
  BOOL Modified;
  HWND hMyself;
  HWND hMyParent;
  HWND hPlayButton;
  HWND hPauseButton;
  HWND hStopButton;
  HWND hLoopButton;
  HWND hVoiceButton;
  char Instrument;

  MTHDCHNK* m_stMThdChunk;
  CHNK_HDR* m_stMTrkHeader;

  PartWindowData(HWND hWnd,int Size=DELTA);
  ~PartWindowData();
  void Grow(int Size=DELTA);
  int WhichClef();
  void SetClef(int NewClef);
  WCHAR *PartEncode(unsigned From,unsigned Through);
  int wcslenLimited(WCHAR* pWchar, int nPixels);
  char *GetString(unsigned From, unsigned Through);
  int ReplaceString(char *String,unsigned From,unsigned Through);
  int Paint();
  unsigned Width(unsigned From, unsigned Through);
  unsigned Position(unsigned Pixels);
  void AutoScrollRight(void);
  DWORD Ticks2Dur(DWORD dwNoteTicks, DWORD dwTolerance, char* sMusique);
  BOOL MusicXMLIn(const char *filename);
  BOOL MusicXMLOut(const char *filename);
  BOOL MIDIIn(const char *filename);
  BOOL MIDIOut(const char *filename);
  BOOL RTFOut(const char *filename);
  BOOL TXTIn(const char *filename);
  BOOL TXTOut(const char *filename);
  BOOL NotesUp(HWND hWnd);
  BOOL NotesDown(HWND hWnd);
  BOOL GoHome(HWND hWnd);
  BOOL Valid(void){return m_stMThdChunk!=NULL;};
};//PartWindowData

}
#endif
