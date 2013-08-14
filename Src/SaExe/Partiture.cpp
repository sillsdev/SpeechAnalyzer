/////////////////////////////////////////////////////////////////////////////
// Partiture.cpp:
// Implementation for Music Staff Control.
// Author: Tim Erickson
// copyright 2000 JAARS Inc. SIL
//
// Revision History
// 07/26/2000
//    RLJ Add NotesUp, NotesDown, GoHome procedures
//    RLJ TEMPORARY WORKAROUND:
//          Automatically do NotesUp, NotesDown, and GoHome after a Paste
// 08/16/2000
//    DDO Moved the processing of the delet key (in PartitureProc) to the
//        key up processing section rather than the key down section.
// 08/28/2000
//    TRE Switching to Unicode output for better staff rendering
// 09/08/2000
//    TRE Moved processing of some more keystrokes to key up processing
//    TRE Removed RLJ's temporary workaround
// 10/11/2000
//    TRE Moved processing of keys back from KeyUp to KeyDown
// 10/26/2000
//    TRE Improved input of invalid data, Clef determination
// 11/02/2000
//    TRE Added Convert command to the context menu
////////////////////////////////////////////////////////////////////////////
#undef _UNICODE
#undef UNICODE

#pragma warning(push, 3)  // these files generate a lot of warnings at level 4
#pragma warning(pop)

#include <stdlib.h>
#include <windows.h>
#include <tchar.h>
#include <fstream>
using std::ofstream;
using std::ifstream;
using std::ios;
using std::streampos;
#include <math.h>
#include <mmsystem.h>
#include <commdlg.h>
#include "Partiture.hpp"
#include "Clip.hpp"
#include "Resource.h"
#include "AppDefs.h"

#define TICKS_PER_Q (120)

static HINSTANCE hInstance = NULL;

const int ClefRange[5][2] =
{
    {
        ((NAME_G*NUMBER_OF_ACCIDENTALS)+DOUBLE_FLAT+OCTAVE_4),
        ((NAME_D*NUMBER_OF_ACCIDENTALS)+DOUBLE_SHARP+OCTAVE_7)
    }, // TREBLE-8VA
    {
        ((NAME_G*NUMBER_OF_ACCIDENTALS)+DOUBLE_FLAT+OCTAVE_3),
        ((NAME_D*NUMBER_OF_ACCIDENTALS)+DOUBLE_SHARP+OCTAVE_6)
    }, // TREBLE
    {
        ((NAME_G*NUMBER_OF_ACCIDENTALS)+DOUBLE_FLAT+OCTAVE_2),
        ((NAME_D*NUMBER_OF_ACCIDENTALS)+DOUBLE_SHARP+OCTAVE_5)
    }, // TREBLE-8VB
    {
        ((NAME_B*NUMBER_OF_ACCIDENTALS)+DOUBLE_FLAT+OCTAVE_1),
        ((NAME_F*NUMBER_OF_ACCIDENTALS)+DOUBLE_SHARP+OCTAVE_4)
    }, // BASS
    {
        ((NAME_B*NUMBER_OF_ACCIDENTALS)+DOUBLE_FLAT+OCTAVE_0),
        ((NAME_F*NUMBER_OF_ACCIDENTALS)+DOUBLE_SHARP+OCTAVE_3)
    }  // BASS-8VB
};

static char * NoteNum2Name(UBYTE byMIDINumber, char * sMusique, size_t len);

//
// SNote
//

SNote::SNote(int Clef) : Name('C'), Accidental('='),
    Onset('*'), Duration('q'), Duration2('n'), Conclusion(' '),
    Value(0), DisplayOnset('\0'), DisplayAccidental('\0'),
    DisplayModDur('\0'), DisplayNote('\0'), DisplayConclusion('\0')
{
    Octave = (BYTE)((Clef>2) ? '6' - Clef : '5' - Clef);
    CalcValue(Clef);
}

SNote::SNote(const unsigned char * Definition,int Clef)
{
    if (!Definition)
    {
        Name='C';
        Accidental='=';
        Octave='4';
        Onset='*';
        Duration='q';
        Duration2='n';
        Conclusion=' ';
        Value=0;
        DisplayOnset=DisplayAccidental=DisplayModDur=DisplayNote=DisplayConclusion='\0';
        CalcValue(Clef);
    }
    else if (!Set(Definition,Clef))     // error in definition
    {
        Name = ANNOT_UNDEFINED;
        Accidental=Octave=Onset=Duration=Duration2=Conclusion='\0';;
        Value=0;
        DisplayOnset=DisplayAccidental=DisplayModDur=DisplayNote=DisplayConclusion='\0';
        CalcValue(Clef);
    }
}

SNote::~SNote()
{
    Onset=Name=Accidental=Octave=Duration=Duration2=Conclusion='\0';
}

SNote & SNote::operator=(SNote & n)
{
    Name = n.Name;
    Accidental = n.Accidental;
    Octave = n.Octave;
    Onset = n.Onset;
    Duration = n.Duration;
    Duration2 = n.Duration2;
    Conclusion = n.Conclusion;
    Value = n.Value;
    DisplayOnset = n.DisplayOnset;
    DisplayAccidental = n.DisplayAccidental;
    DisplayModDur = n.DisplayModDur;
    DisplayNote = n.DisplayNote;
    DisplayConclusion = n.DisplayConclusion;
    return *this;
}

void SNote::CalcValue(int Clef)
{
    if (Name=='R')
    {
        Value = -1;
    }
    else if (Name>='A'&&Name<='G')
    {
        switch (Accidental)
        {
        case '$':
            Value = DOTTED_SHARP;
            break;
        case '#':
            Value = SHARP;
            break;
        case '+':
            Value = HALF_SHARP;
            break;
        case '=':
            Value = NATURAL;
            break;
        case '-':
            Value = HALF_FLAT;
            break;
        case '@':
            Value = FLAT;
            break;
        case '!':
            Value = DOTTED_FLAT;
            break;
        default :
            Value = 0;
            break;
        }
        unsigned int valueName = (Name < 'C' ? Name + 7 : Name); // adjust
        Value += (valueName - 'C')*NUMBER_OF_ACCIDENTALS;
        Value += (Octave - '0')*OCTAVE;
    }
    else
    {
        switch (Name)
        {
        case ANNOT_SPACE:
            DisplayNote=DISPLAY_SMALL_SPACE;
            break;
        case ANNOT_BAR:
            DisplayNote=DISPLAY_BAR;
            break;
        case ANNOT_THICK_BAR:
            DisplayNote=DISPLAY_THICK_BAR;
            break;
        case ANNOT_PHRASE:
            DisplayNote=DISPLAY_APOSTROPHE;
            break;
        case ANNOT_PHRASE2:
            DisplayNote=DISPLAY_QUOTE;
            break;
        default:
            DisplayNote=DISPLAY_UNDEFINED;
            break;
        }
        DisplayOnset=DisplayAccidental=DisplayModDur=DisplayConclusion='\0';
        Value = -2;
        return;
    }

    // DISPLAY
    switch (Duration)
    {
    case 'w': // whole note
        if (Value==-1)
        {
            DisplayNote=DISPLAY_WHOLE_REST;
        }
        else if (Value<ClefRange[Clef][0])
        {
            DisplayNote=DISPLAY_WHOLE_TOO_LOW;
        }
        else if (Value>ClefRange[Clef][1])
        {
            DisplayNote=DISPLAY_WHOLE_TOO_HIGH;
        }
        else
        {
            DisplayNote=(unsigned short)(DISPLAY_WHOLE+((Value-ClefRange[Clef][0])/NUMBER_OF_ACCIDENTALS));
        }
        break;
    case 'h': // half note
        if (Value==-1)
        {
            DisplayNote=DISPLAY_HALF_REST;
        }
        else if (Value<ClefRange[Clef][0])
        {
            DisplayNote=DISPLAY_HALF_TOO_LOW;
        }
        else if (Value>ClefRange[Clef][1])
        {
            DisplayNote=DISPLAY_HALF_TOO_HIGH;
        }
        else
        {
            DisplayNote=(unsigned short)(DISPLAY_HALF+((Value-ClefRange[Clef][0])/NUMBER_OF_ACCIDENTALS));
        }
        break;
    case 'q': // quarter note
        if (Value==-1)
        {
            DisplayNote=DISPLAY_QUARTER_REST;
        }
        else if (Value<ClefRange[Clef][0])
        {
            DisplayNote=DISPLAY_QUARTER_TOO_LOW;
        }
        else if (Value>ClefRange[Clef][1])
        {
            DisplayNote=DISPLAY_QUARTER_TOO_HIGH;
        }
        else
        {
            DisplayNote=(unsigned short)(DISPLAY_QUARTER+((Value-ClefRange[Clef][0])/NUMBER_OF_ACCIDENTALS));
        }
        break;
    case 'i': // eighth note
        if (Value==-1)
        {
            DisplayNote=DISPLAY_EIGHTH_REST;
        }
        else if (Value<ClefRange[Clef][0])
        {
            DisplayNote=DISPLAY_EIGHTH_TOO_LOW;
        }
        else if (Value>ClefRange[Clef][1])
        {
            DisplayNote=DISPLAY_EIGHTH_TOO_HIGH;
        }
        else
        {
            DisplayNote=(unsigned short)(DISPLAY_EIGHTH+((Value-ClefRange[Clef][0])/NUMBER_OF_ACCIDENTALS));
        }
        break;
    case 's': // sixteenth note
        if (Value==-1)
        {
            DisplayNote=DISPLAY_SIXTEENTH_REST;
        }
        else if (Value<ClefRange[Clef][0])
        {
            DisplayNote=DISPLAY_SIXTEENTH_TOO_LOW;
        }
        else if (Value>ClefRange[Clef][1])
        {
            DisplayNote=DISPLAY_SIXTEENTH_TOO_HIGH;
        }
        else
        {
            DisplayNote=(unsigned short)(DISPLAY_SIXTEENTH+((Value-ClefRange[Clef][0])/NUMBER_OF_ACCIDENTALS));
        }
        break;
    case 'z': // grace note
        if (Value==-1)
        {
            DisplayNote=DISPLAY_GRACE_REST;
        }
        else if (Value<ClefRange[Clef][0])
        {
            DisplayNote=DISPLAY_GRACE_TOO_LOW;
        }
        else if (Value>ClefRange[Clef][1])
        {
            DisplayNote=DISPLAY_GRACE_TOO_HIGH;
        }
        else
        {
            DisplayNote=(unsigned short)(DISPLAY_GRACE+((Value-ClefRange[Clef][0])/NUMBER_OF_ACCIDENTALS));
        }
        break;
    }

    CalcOnset(Clef);
    CalcAccidental(Clef);
    CalcModDuration(Clef);
    CalcConclusion(Clef);
}

void SNote::CalcOnset(int Clef)
{
    if (Value<0)   // rest or notation
    {
        DisplayOnset='\0';
        return;
    }
    switch (Onset)
    {
    case '*': // none
        DisplayOnset='\0';
        break;
    case '|': // long fall
        if (Value<ClefRange[Clef][0]||Value>ClefRange[Clef][1])
        {
            DisplayOnset=DISPLAY_LONG_FALL+1;    // OUT OF RANGE
        }
        else if (Value<ClefRange[Clef][0]+(NUMBER_OF_ACCIDENTALS*6))
        {
            DisplayOnset=DISPLAY_LONG_FALL;    // LOW
        }
        else if (Value<ClefRange[Clef][0]+(NUMBER_OF_ACCIDENTALS*10))
        {
            DisplayOnset=DISPLAY_LONG_FALL+1;    // MID
        }
        else
        {
            DisplayOnset=DISPLAY_LONG_FALL+2;    // HIGH
        }
        break;
    case '~': // short fall
        if (Value<ClefRange[Clef][0]||Value>ClefRange[Clef][1])
        {
            DisplayOnset=DISPLAY_SHORT_FALL+1;    // OUT OF RANGE
        }
        else if (Value<ClefRange[Clef][0]+(NUMBER_OF_ACCIDENTALS*6))
        {
            DisplayOnset=DISPLAY_SHORT_FALL;    // LOW
        }
        else if (Value<ClefRange[Clef][0]+(NUMBER_OF_ACCIDENTALS*10))
        {
            DisplayOnset=DISPLAY_SHORT_FALL+1;    // MID
        }
        else
        {
            DisplayOnset=DISPLAY_SHORT_FALL+2;    // HIGH
        }
        break;
    case '?': // long rise
        if (Value<ClefRange[Clef][0]||Value>ClefRange[Clef][1])
        {
            DisplayOnset=DISPLAY_LONG_RISE+1;    // OUT OF RANGE
        }
        else if (Value<ClefRange[Clef][0]+(NUMBER_OF_ACCIDENTALS*9))
        {
            DisplayOnset=DISPLAY_LONG_RISE;    // LOW
        }
        else if (Value<ClefRange[Clef][0]+(NUMBER_OF_ACCIDENTALS*13))
        {
            DisplayOnset=DISPLAY_LONG_RISE+1;    // MID
        }
        else
        {
            DisplayOnset=DISPLAY_LONG_RISE+2;    // HIGH
        }
        break;
    case '\"': // short rise
        if (Value<ClefRange[Clef][0]||Value>ClefRange[Clef][1])
        {
            DisplayOnset=DISPLAY_SHORT_RISE+1;    // OUT OF RANGE
        }
        else if (Value<ClefRange[Clef][0]+(NUMBER_OF_ACCIDENTALS*9))
        {
            DisplayOnset=DISPLAY_SHORT_RISE;    // LOW
        }
        else if (Value<ClefRange[Clef][0]+(NUMBER_OF_ACCIDENTALS*13))
        {
            DisplayOnset=DISPLAY_SHORT_RISE+1;    // MID
        }
        else
        {
            DisplayOnset=DISPLAY_SHORT_RISE+2;    // HIGH
        }
        break;
    }
}

void SNote::CalcAccidental(int Clef)
{
    if (Value<0)   // rest or notation
    {
        DisplayAccidental='\0';
        return;
    }
    switch (Accidental)
    {
    case '=': // natural (none)
        DisplayAccidental='\0';
        break;
    case '#': // sharp
        if (Value<ClefRange[Clef][0]||Value>ClefRange[Clef][1])
        {
            DisplayAccidental=DISPLAY_SHARP+5;    // OUT OF RANGE
        }
        else
            DisplayAccidental=(unsigned short)(DISPLAY_SHARP+((Value-ClefRange[Clef][0])
                                               /NUMBER_OF_ACCIDENTALS));
        break;
    case '@': // flat
        if (Value<ClefRange[Clef][0]||Value>ClefRange[Clef][1])
        {
            DisplayAccidental=DISPLAY_FLAT+5;    // OUT OF RANGE
        }
        else
            DisplayAccidental=(unsigned short)(DISPLAY_FLAT+((Value-ClefRange[Clef][0])
                                               /NUMBER_OF_ACCIDENTALS));
        break;
    case '+': // half-sharp
        if (Value<ClefRange[Clef][0]||Value>ClefRange[Clef][1])
        {
            DisplayAccidental=DISPLAY_HALF_SHARP+5;    // OUT OF RANGE
        }
        else
            DisplayAccidental=(unsigned short)(DISPLAY_HALF_SHARP+((Value-ClefRange[Clef][0])
                                               /NUMBER_OF_ACCIDENTALS));
        break;
    case '-': // half-flat
        if (Value<ClefRange[Clef][0]||Value>ClefRange[Clef][1])
        {
            DisplayAccidental=DISPLAY_HALF_FLAT+5;    // OUT OF RANGE
        }
        else
            DisplayAccidental=(unsigned short)(DISPLAY_HALF_FLAT+((Value-ClefRange[Clef][0])
                                               /NUMBER_OF_ACCIDENTALS));
        break;
    case '$': // 11/2 sharp
        if (Value<ClefRange[Clef][0]||Value>ClefRange[Clef][1])
        {
            DisplayAccidental=DISPLAY_DOTTED_SHARP+5;    // OUT OF RANGE
        }
        else
            DisplayAccidental=(unsigned short)(DISPLAY_DOTTED_SHARP+((Value-ClefRange[Clef][0])
                                               /NUMBER_OF_ACCIDENTALS));
        break;
    case '!': // 11/2 flat
        if (Value<ClefRange[Clef][0]||Value>ClefRange[Clef][1])
        {
            DisplayAccidental=DISPLAY_DOTTED_FLAT+5;    // OUT OF RANGE
        }
        else
            DisplayAccidental=(unsigned short)(DISPLAY_DOTTED_FLAT+((Value-ClefRange[Clef][0])
                                               /NUMBER_OF_ACCIDENTALS));
        break;
    }
}

void SNote::CalcModDuration(int Clef)
{
    if (Value == -2)   // notation
    {
        DisplayModDur='\0';
        return;
    }
    switch (Duration2)
    {
    case 'n': // normal
        DisplayModDur='\0';
        break;
    case '.': // dotted
        if (Value==-1)   // rest
        {
            if (Duration=='w' || Duration=='h')
            {
                DisplayModDur=DISPLAY_DOT+5;
            }
            else
            {
                DisplayModDur=DISPLAY_DOT+4;
            }
        }
        else if (Value<ClefRange[Clef][0]||Value>ClefRange[Clef][1])
        {
            DisplayModDur=DISPLAY_DOT+2;    // OUT OF RANGE
        }
        else if (Value<ClefRange[Clef][0]+(NUMBER_OF_ACCIDENTALS*9))
            DisplayModDur=(unsigned short)(DISPLAY_DOT+((Value-ClefRange[Clef][0])
                                           /(2*NUMBER_OF_ACCIDENTALS))); // LOW
        else
            DisplayModDur=(unsigned short)(DISPLAY_DOT+((Value+1-ClefRange[Clef][0])
                                           /(2*NUMBER_OF_ACCIDENTALS))); // HIGH
        break;
    case 't': // triplet
        if (Value<ClefRange[Clef][0]||Value>ClefRange[Clef][1])
        {
            DisplayModDur=DISPLAY_TRIPLET_ABOVE;    // OUT OF RANGE
        }
        else if (Value<ClefRange[Clef][0]+(NUMBER_OF_ACCIDENTALS*6))
        {
            DisplayModDur=DISPLAY_TRIPLET_ABOVE;    // ABOVE
        }
        else if (Value<ClefRange[Clef][0]+(NUMBER_OF_ACCIDENTALS*9))
        {
            DisplayModDur=DISPLAY_TRIPLET;    // BELOW
        }
        else if (Value<ClefRange[Clef][0]+(NUMBER_OF_ACCIDENTALS*13))
        {
            DisplayModDur=DISPLAY_TRIPLET_ABOVE;    // ABOVE
        }
        else
        {
            DisplayModDur=DISPLAY_TRIPLET;    // BELOW
        }
        break;
    case 'v': // quintuplet
        if (Value<ClefRange[Clef][0]||Value>ClefRange[Clef][1])
        {
            DisplayModDur=DISPLAY_QUINTUPLET_ABOVE;    // OUT OF RANGE
        }
        else if (Value<ClefRange[Clef][0]+(NUMBER_OF_ACCIDENTALS*6))
        {
            DisplayModDur=DISPLAY_QUINTUPLET_ABOVE;    // ABOVE
        }
        else if (Value<ClefRange[Clef][0]+(NUMBER_OF_ACCIDENTALS*9))
        {
            DisplayModDur=DISPLAY_QUINTUPLET;    // BELOW
        }
        else if (Value<ClefRange[Clef][0]+(NUMBER_OF_ACCIDENTALS*13))
        {
            DisplayModDur=DISPLAY_QUINTUPLET_ABOVE;    // ABOVE
        }
        else
        {
            DisplayModDur=DISPLAY_QUINTUPLET;    // BELOW
        }
        break;
    }
}

void SNote::CalcConclusion(int Clef)
{
    if (Value<0)   // rest or notation
    {
        DisplayConclusion='\0';
        return;
    }
    switch (Conclusion)
    {
    case ' ': // none
        DisplayConclusion = '\0';
        break;
    case '/': // long rise
        if (Value<ClefRange[Clef][0]||Value>ClefRange[Clef][1])
        {
            DisplayConclusion = DISPLAY_LONG_RISE+1;    // out of range
        }
        else if (Value<ClefRange[Clef][0]+(NUMBER_OF_ACCIDENTALS*6))
        {
            DisplayConclusion = DISPLAY_LONG_RISE;    // low
        }
        else if (Value<ClefRange[Clef][0]+(NUMBER_OF_ACCIDENTALS*10))
        {
            DisplayConclusion = DISPLAY_LONG_RISE+1;    // mid
        }
        else
        {
            DisplayConclusion = DISPLAY_LONG_RISE+2;    // high
        }
        break;
    case '\'': // short rise
        if (Value<ClefRange[Clef][0]||Value>ClefRange[Clef][1])
        {
            DisplayConclusion = DISPLAY_SHORT_RISE+1;    // out of range
        }
        else if (Value<ClefRange[Clef][0]+(NUMBER_OF_ACCIDENTALS*6))
        {
            DisplayConclusion = DISPLAY_SHORT_RISE;    // low
        }
        else if (Value<ClefRange[Clef][0]+(NUMBER_OF_ACCIDENTALS*10))
        {
            DisplayConclusion = DISPLAY_SHORT_RISE+1;    // mid
        }
        else
        {
            DisplayConclusion = DISPLAY_SHORT_RISE+2;    // high
        }
        break;
    case '\\': // long fall
        if (Value<ClefRange[Clef][0]||Value>ClefRange[Clef][1])
        {
            DisplayConclusion = DISPLAY_LONG_FALL+1;    // out of range
        }
        else if (Value<ClefRange[Clef][0]+(NUMBER_OF_ACCIDENTALS*9))
        {
            DisplayConclusion = DISPLAY_LONG_FALL;    // low
        }
        else if (Value<ClefRange[Clef][0]+(NUMBER_OF_ACCIDENTALS*13))
        {
            DisplayConclusion = DISPLAY_LONG_FALL+1;    // mid
        }
        else
        {
            DisplayConclusion = DISPLAY_LONG_FALL+2;    // high
        }
        break;
    case '`': // short fall
        if (Value<ClefRange[Clef][0]||Value>ClefRange[Clef][1])
        {
            DisplayConclusion = DISPLAY_SHORT_FALL+1;    // out of range
        }
        else if (Value<ClefRange[Clef][0]+(NUMBER_OF_ACCIDENTALS*9))
        {
            DisplayConclusion = DISPLAY_SHORT_FALL;    // low
        }
        else if (Value<ClefRange[Clef][0]+(NUMBER_OF_ACCIDENTALS*13))
        {
            DisplayConclusion = DISPLAY_SHORT_FALL+1;    // mid
        }
        else
        {
            DisplayConclusion = DISPLAY_SHORT_FALL+2;    // high
        }
        break;
    case '_': // tie
        if (Value < ClefRange[Clef][0] || Value > ClefRange[Clef][1])
        {
            DisplayConclusion = DISPLAY_TIE + 2;    // out of range (below)
        }
        else if (Value < ClefRange[Clef][0] + (NUMBER_OF_ACCIDENTALS * 4))
        {
            DisplayConclusion = DISPLAY_TIE + 3;    // far below
        }
        else if (Value < ClefRange[Clef][0] + (NUMBER_OF_ACCIDENTALS * 9))
        {
            DisplayConclusion = DISPLAY_TIE + 2;    // below
        }
        else if (Value < ClefRange[Clef][0] + (NUMBER_OF_ACCIDENTALS * 15))
        {
            DisplayConclusion = DISPLAY_TIE + 1;    // above
        }
        else if (Value < ClefRange[Clef][0] + (NUMBER_OF_ACCIDENTALS * 19))
        {
            DisplayConclusion = DISPLAY_TIE + 0;    // far above
        }
        else
        {
            DisplayConclusion = DISPLAY_TIE+1;    // below
        }
        break;
    }
}

BOOL SNote::SetClef(int Clef)
{
    CalcValue(Clef);
    return InClef(Clef);
}

BOOL SNote::Set(unsigned char c,int Clef)
{
    if ((c>='a' && c<= 'g')||c=='r')
    {
        c -= 'a' - 'A';
    }
    else if (c>='H' && c<='Z' && c!='R')
    {
        c += 'a' - 'A';
    }

    if (c=='R')
    {
        if (Name != c && Duration != 'z')
        {
            if (!Accidental)
            {
                Set((unsigned char *)"C=4*qn ",Clef);
            }
            Name = c;
            CalcValue(Clef);
            return TRUE;
        }
    }
    else if (c>='A' && c<='G')
    {
        if (Name != c)
        {
            if (!Accidental)
            {
                Set((unsigned char *)"C=4*qn ",Clef);
            }
            Name = c;
            CalcValue(Clef);
            return TRUE;
        }
    }
    else if (c>='0' && c<='7')
    {
        if (Octave != c)
        {
            if (!Accidental)
            {
                Set((unsigned char *)"C=4*qn ",Clef);
            }
            Octave = c;
            CalcValue(Clef);
            return TRUE;
        }
    }
    else if (strchr("$#+=-@!",c))
    {
        if (Accidental != c)
        {
            if (!Accidental)
            {
                Set((unsigned char *)"C=4*qn ",Clef);
            }
            Accidental = c;
            CalcValue(Clef);
            return TRUE;
        }
    }
    else if (strchr("whqisz",c))
    {
        if (Duration != c)
        {
            if (!Accidental)
            {
                Set((unsigned char *)"C=4*qn ",Clef);
            }
            if (c=='z')
            {
                Duration2='n';
            }
            Duration = c;
            CalcValue(Clef);
            return TRUE;
        }
    }
    else if (strchr("n.tv",c))
    {
        if (Duration2 != c && Duration != 'z')
        {
            if (!Accidental)
            {
                Set((unsigned char *)"C=4*qn ",Clef);
            }
            Duration2 = c;
            CalcModDuration(Clef);
            return TRUE;
        }
    }
    else if (strchr(" \\`/\'_",c))
    {
        if (Conclusion != c)
        {
            if (!Accidental)
            {
                Set((unsigned char *)"C=4*qn ",Clef);
            }
            Conclusion = c;
            CalcConclusion(Clef);
            return TRUE;
        }
    }
    else if (strchr("*|~?\"",c))
    {
        if (Onset != c)
        {
            if (!Accidental)
            {
                Set((unsigned char *)"C=4*qn ",Clef);
            }
            Onset = c;
            CalcOnset(Clef);
            return TRUE;
        }
    }
    else if (c==ANNOT_SPACE)
    {
        BOOL Result = Set((unsigned char *)"(SPACE)",Clef);
        CalcValue(Clef);
        return Result;
    }
    else if (c==ANNOT_BAR)
    {
        BOOL Result = Set((unsigned char *)"(-BAR-)",Clef);
        CalcValue(Clef);
        return Result;
    }
    else if (c==ANNOT_THICK_BAR)
    {
        BOOL Result = Set((unsigned char *)"(=BAR=)",Clef);
        CalcValue(Clef);
        return Result;
    }
    else if (c==ANNOT_PHRASE)
    {
        BOOL Result = Set((unsigned char *)"(\'PHR\')",Clef);
        CalcValue(Clef);
        return Result;
    }
    else if (c==ANNOT_PHRASE2)
    {
        BOOL Result = Set((unsigned char *)"(\"PHR\")",Clef);
        CalcValue(Clef);
        return Result;
    }
    return FALSE;
}

BOOL SNote::Set(const unsigned char * Definition,int Clef)
{
    // (validify data)
    if (!Definition)
    {
        return FALSE;
    }
    if (*Definition=='(')
    {
        if (Definition[6]!=')')
        {
            return FALSE;
        }

        if (!strncmp("FIELD",(char *)Definition+1,5))
        {
            Name = 0;
        }
        else if (!strncmp("-BAR-",(char *)Definition+1,5))
        {
            Name = ANNOT_BAR;
        }
        else if (!strncmp("=BAR=",(char *)Definition+1,5))
        {
            Name = ANNOT_THICK_BAR;
        }
        else if (!strncmp("\'PHR\'",(char *)Definition+1,5))
        {
            Name = ANNOT_PHRASE;
        }
        else if (!strncmp("\"PHR\"",(char *)Definition+1,5))
        {
            Name = ANNOT_PHRASE2;
        }
        else if (!strncmp("SPACE",(char *)Definition+1,5))
        {
            Name = ANNOT_SPACE;
        }
        else
        {
            Name = ANNOT_UNDEFINED;
        }

        Accidental=Octave=Onset=Duration=Duration2=Conclusion='\0';;
        CalcValue(Clef);
        return (Name != ANNOT_UNDEFINED);
    }
    else
    {
        if
        (
            (!strchr("ABCDEFGR",*Definition)) ||
            (!strchr("=#@+-$!",Definition[1])) ||
            (Definition[2]<'0'||Definition[2]>'7') ||
            (!strchr("*|~?\"",Definition[3])) ||
            (!strchr("qihwsz",Definition[4])) ||
            (!strchr("n.tv",Definition[5])) ||
            (!strchr(" \\`/\'_",Definition[6]))
        )
        {
            return FALSE;
        }

        Name = *Definition;
        Accidental = Definition[1];
        Octave = Definition[2];
        Onset = Definition[3];
        Duration = Definition[4];
        Duration2 = Definition[5];
        Conclusion = Definition[6];
        CalcValue(Clef);
        return TRUE;
    }
}

BOOL SNote::UpOctave(void)
{
    if (Octave>='7')
    {
        return FALSE;
    }
    ++Octave;
    return TRUE;
}

BOOL SNote::DownOctave(void)
{
    if (Octave<='0')
    {
        return FALSE;
    }
    --Octave;
    return TRUE;
}
BOOL SNote::UpStep(void)
{
    if (Name == 'B')
    {
        if (Octave>='7')
        {
            return FALSE;
        }
        ++Octave;
    }

    if (Name=='G')
    {
        Name='A';
    }
    else if (Name>='A' && Name<='F')
    {
        ++Name;
    }
    else
    {
        return FALSE;
    }

    return TRUE;
}

BOOL SNote::DownStep(void)
{
    if (Name == 'C')
    {
        if (Octave<='0')
        {
            return FALSE;
        }
        --Octave;
    }

    if (Name=='A')
    {
        Name='G';
    }
    else if (Name>='B' && Name<='G')
    {
        --Name;
    }
    else
    {
        return FALSE;
    }

    return TRUE;
}

BOOL SNote::InClef(int Clef)
{
    if (Clef<0||Clef>5)
    {
        return FALSE;
    }
    if (Value<0)
    {
        return TRUE;    // rest
    }
    if (Value<ClefRange[Clef][0])
    {
        return FALSE;
    }
    if (Value>ClefRange[Clef][1])
    {
        return FALSE;
    }
    return TRUE;
}

unsigned SNote::MIDINote()
{
    unsigned cNote = 12;
    switch (Name)
    {
    case 'A':
        cNote += 9;
        break;
    case 'B':
        cNote += 11;
        break;
    case 'C':
        cNote += 0;
        break;
    case 'D':
        cNote += 2;
        break;
    case 'E':
        cNote += 4;
        break;
    case 'F':
        cNote += 5;
        break;
    case 'G':
        cNote += 7;
        break;

    default:
        return 0;
    }
    cNote += (unsigned)((Octave - '0')*12);
    switch (Accidental)
    {
    case '=':
        break;
    case '$':
    case '#':
    case '+':
        ++cNote;
        break;
    case '!':
    case '@':
    case '-':
        --cNote;
        break;
    }
    return cNote;
}

unsigned long SNote::MIDIDuration()
{
    unsigned long l = 0;
    switch (Duration)   // note duration
    {
    case 'w':
        l = 4 * TICKS_PER_Q;
        break;
    case 'h':
        l = 2 * TICKS_PER_Q;
        break;
    case 'q':
        l = TICKS_PER_Q;
        break;
    case 'i':
        l = TICKS_PER_Q / 2;
        break;
    case 's':
        l = TICKS_PER_Q / 4;
        break;
    case 'z':
        l = TICKS_PER_Q / 8;
        break;

    default:
        return 0;
    }
    switch (Duration2)   // modify duration
    {
    case 'n':
        break;
    case '.':
        l += (l/2L);
        break;
    case 't':
        l -= (l/3L);
        break;
    case 'v':
        l -= (l/5L);
        break;
    }
    return l;
}

// ***************
// CInstrumentMenu
// ***************

class CInstrumentMenu
{
    HMENU hInstrumentMnu;
    HMENU hPercussionMnu;
    HMENU hStringMnu;
    HMENU hBrassMnu;
    HMENU hWoodMnu;
    HMENU hMiscMnu;

public:
    CInstrumentMenu(int instrument=0);
    ~CInstrumentMenu();
    void Check(int instrument);
    HMENU Menu()
    {
        return hInstrumentMnu;
    }
};

CInstrumentMenu::CInstrumentMenu(int /*Instrument*/)
{
    hInstrumentMnu = CreatePopupMenu();
    hPercussionMnu = CreatePopupMenu();
    hStringMnu = CreatePopupMenu();
    hBrassMnu = CreatePopupMenu();
    hWoodMnu = CreatePopupMenu();
    hMiscMnu = CreatePopupMenu();

    AppendMenu(hPercussionMnu,MF_ENABLED|MF_STRING,WM_USER+128,_T(" 0 Acoustic Grand Piano"));
    AppendMenu(hPercussionMnu,MF_ENABLED|MF_STRING,WM_USER+129,_T(" 1 Bright Acoustic Piano"));
    AppendMenu(hPercussionMnu,MF_ENABLED|MF_STRING,WM_USER+130,_T(" 2 Electric Grand Piano"));
    AppendMenu(hPercussionMnu,MF_ENABLED|MF_STRING,WM_USER+131,_T(" 3 Honkytonk Piano"));
    AppendMenu(hPercussionMnu,MF_ENABLED|MF_STRING,WM_USER+132,_T(" 4 Rhodes Piano"));
    AppendMenu(hPercussionMnu,MF_ENABLED|MF_STRING,WM_USER+133,_T(" 5 Chorus Piano"));
    AppendMenu(hPercussionMnu,MF_ENABLED|MF_STRING,WM_USER+135,_T(" 7 Clavinet"));
    AppendMenu(hPercussionMnu,MF_ENABLED|MF_STRING,WM_USER+136,_T(" 8 Celesta"));
    AppendMenu(hPercussionMnu,MF_ENABLED|MF_STRING,WM_USER+137,_T(" 9 Glockenspiel"));
    AppendMenu(hPercussionMnu,MF_ENABLED|MF_STRING,WM_USER+138,_T(" 10 Music Box"));
    AppendMenu(hPercussionMnu,MF_ENABLED|MF_STRING,WM_USER+139,_T(" 11 Vibraphone"));
    AppendMenu(hPercussionMnu,MF_ENABLED|MF_STRING,WM_USER+140,_T(" 12 Marimba"));
    AppendMenu(hPercussionMnu,MF_ENABLED|MF_STRING,WM_USER+141,_T(" 13 Xylophone"));
    AppendMenu(hPercussionMnu,MF_ENABLED|MF_STRING,WM_USER+142,_T(" 14 Tubular Bells"));
    AppendMenu(hPercussionMnu,MF_ENABLED|MF_STRING,WM_USER+143,_T(" 15 Dulcimer"));
    AppendMenu(hPercussionMnu,MF_ENABLED|MF_STRING,WM_USER+175,_T(" 47 Timpani"));
    AppendMenu(hPercussionMnu,MF_ENABLED|MF_STRING,WM_USER+236,_T("108 Kalimba"));
    AppendMenu(hPercussionMnu,MF_ENABLED|MF_STRING,WM_USER+240,_T("112 Tinkle Bell"));
    AppendMenu(hPercussionMnu,MF_ENABLED|MF_STRING,WM_USER+241,_T("113 Agogo"));
    AppendMenu(hPercussionMnu,MF_ENABLED|MF_STRING,WM_USER+242,_T("114 Steel Drums"));
    AppendMenu(hPercussionMnu,MF_ENABLED|MF_STRING,WM_USER+243,_T("115 Wood Block"));
    AppendMenu(hPercussionMnu,MF_ENABLED|MF_STRING,WM_USER+244,_T("116 Taiko Drum"));
    AppendMenu(hPercussionMnu,MF_ENABLED|MF_STRING,WM_USER+245,_T("117 Melodic Tom"));
    AppendMenu(hPercussionMnu,MF_ENABLED|MF_STRING,WM_USER+246,_T("118 Synth Drum"));
    AppendMenu(hPercussionMnu,MF_ENABLED|MF_STRING,WM_USER+247,_T("119 Reverse Cymbal"));

    AppendMenu(hInstrumentMnu,MF_ENABLED|MF_POPUP,(UINT)hPercussionMnu,_T("&Percussion"));

    AppendMenu(hStringMnu,MF_ENABLED|MF_STRING,WM_USER+134,_T(" 6 Harpsichord"));
    AppendMenu(hStringMnu,MF_ENABLED|MF_STRING,WM_USER+152,_T(" 24 Acoustic Guitar (nylon)"));
    AppendMenu(hStringMnu,MF_ENABLED|MF_STRING,WM_USER+153,_T(" 25 Acoustic Guitar (steel)"));
    AppendMenu(hStringMnu,MF_ENABLED|MF_STRING,WM_USER+154,_T(" 26 Electric Guitar (jazz)"));
    AppendMenu(hStringMnu,MF_ENABLED|MF_STRING,WM_USER+155,_T(" 27 Electric Guitar (clean)"));
    AppendMenu(hStringMnu,MF_ENABLED|MF_STRING,WM_USER+156,_T(" 28 Electric Guitar (muted)"));
    AppendMenu(hStringMnu,MF_ENABLED|MF_STRING,WM_USER+157,_T(" 29 Overdriven Guitar"));
    AppendMenu(hStringMnu,MF_ENABLED|MF_STRING,WM_USER+158,_T(" 30 Distortion Guitar"));
    AppendMenu(hStringMnu,MF_ENABLED|MF_STRING,WM_USER+159,_T(" 31 Guitar Harmonics"));
    AppendMenu(hStringMnu,MF_ENABLED|MF_STRING,WM_USER+160,_T(" 32 Acoustic Bass"));
    AppendMenu(hStringMnu,MF_ENABLED|MF_STRING,WM_USER+161,_T(" 33 Electric Bass (finger)"));
    AppendMenu(hStringMnu,MF_ENABLED|MF_STRING,WM_USER+162,_T(" 34 Electric Bass (pick)"));
    AppendMenu(hStringMnu,MF_ENABLED|MF_STRING,WM_USER+163,_T(" 35 Fretless Bass"));
    AppendMenu(hStringMnu,MF_ENABLED|MF_STRING,WM_USER+164,_T(" 36 Slap Bass 1"));
    AppendMenu(hStringMnu,MF_ENABLED|MF_STRING,WM_USER+165,_T(" 37 Slap Bass 2"));
    AppendMenu(hStringMnu,MF_ENABLED|MF_STRING,WM_USER+166,_T(" 38 Synth Bass 1"));
    AppendMenu(hStringMnu,MF_ENABLED|MF_STRING,WM_USER+167,_T(" 39 Synth Bass 2"));
    AppendMenu(hStringMnu,MF_ENABLED|MF_STRING,WM_USER+168,_T(" 40 Violin"));
    AppendMenu(hStringMnu,MF_ENABLED|MF_STRING,WM_USER+169,_T(" 41 Viola"));
    AppendMenu(hStringMnu,MF_ENABLED|MF_STRING,WM_USER+170,_T(" 42 Cello"));
    AppendMenu(hStringMnu,MF_ENABLED|MF_STRING,WM_USER+171,_T(" 43 Contra Bass"));
    AppendMenu(hStringMnu,MF_ENABLED|MF_STRING,WM_USER+172,_T(" 44 Tremolo Strings"));
    AppendMenu(hStringMnu,MF_ENABLED|MF_STRING,WM_USER+173,_T(" 45 Pizzicato Strings"));
    AppendMenu(hStringMnu,MF_ENABLED|MF_STRING,WM_USER+174,_T(" 46 Orchestral Harp"));
    AppendMenu(hStringMnu,MF_ENABLED|MF_STRING,WM_USER+176,_T(" 48 String Ensemble 1"));
    AppendMenu(hStringMnu,MF_ENABLED|MF_STRING,WM_USER+177,_T(" 49 String Ensemble 2"));
    AppendMenu(hStringMnu,MF_ENABLED|MF_STRING,WM_USER+178,_T(" 50 Synth Strings 1"));
    AppendMenu(hStringMnu,MF_ENABLED|MF_STRING,WM_USER+179,_T(" 51 Synth Strings 2"));
    AppendMenu(hStringMnu,MF_ENABLED|MF_STRING,WM_USER+183,_T(" 55 Orchestra Hit"));
    AppendMenu(hStringMnu,MF_ENABLED|MF_STRING,WM_USER+232,_T("104 Sitar"));
    AppendMenu(hStringMnu,MF_ENABLED|MF_STRING,WM_USER+233,_T("105 Banjo"));
    AppendMenu(hStringMnu,MF_ENABLED|MF_STRING,WM_USER+234,_T("106 Shamisen"));
    AppendMenu(hStringMnu,MF_ENABLED|MF_STRING,WM_USER+235,_T("107 Koto"));
    AppendMenu(hStringMnu,MF_ENABLED|MF_STRING,WM_USER+238,_T("110 Fiddle"));
    AppendMenu(hStringMnu,MF_ENABLED|MF_STRING,WM_USER+248,_T("120 Guitar Fret Noise"));
    AppendMenu(hInstrumentMnu,MF_ENABLED|MF_POPUP,(UINT)hStringMnu,_T("&String"));

    AppendMenu(hBrassMnu,MF_ENABLED|MF_STRING,WM_USER+184,_T("56 Trumpet"));
    AppendMenu(hBrassMnu,MF_ENABLED|MF_STRING,WM_USER+185,_T("57 Trombone"));
    AppendMenu(hBrassMnu,MF_ENABLED|MF_STRING,WM_USER+186,_T("58 Tuba"));
    AppendMenu(hBrassMnu,MF_ENABLED|MF_STRING,WM_USER+187,_T("59 Muted Trumpet"));
    AppendMenu(hBrassMnu,MF_ENABLED|MF_STRING,WM_USER+188,_T("60 French Horn"));
    AppendMenu(hBrassMnu,MF_ENABLED|MF_STRING,WM_USER+189,_T("61 Brass Section"));
    AppendMenu(hBrassMnu,MF_ENABLED|MF_STRING,WM_USER+190,_T("62 Synth Brass 1"));
    AppendMenu(hBrassMnu,MF_ENABLED|MF_STRING,WM_USER+191,_T("63 Synth Brass 2"));
    AppendMenu(hInstrumentMnu,MF_ENABLED|MF_POPUP,(UINT)hBrassMnu,_T("&Brass"));

    AppendMenu(hWoodMnu,MF_ENABLED|MF_STRING,WM_USER+144,_T(" 16 Hammond Organ"));
    AppendMenu(hWoodMnu,MF_ENABLED|MF_STRING,WM_USER+145,_T(" 17 Percuss. Organ"));
    AppendMenu(hWoodMnu,MF_ENABLED|MF_STRING,WM_USER+146,_T(" 18 Rock Organ"));
    AppendMenu(hWoodMnu,MF_ENABLED|MF_STRING,WM_USER+147,_T(" 19 Church Organ"));
    AppendMenu(hWoodMnu,MF_ENABLED|MF_STRING,WM_USER+148,_T(" 20 Reed Organ"));
    AppendMenu(hWoodMnu,MF_ENABLED|MF_STRING,WM_USER+149,_T(" 21 Accordion"));
    AppendMenu(hWoodMnu,MF_ENABLED|MF_STRING,WM_USER+150,_T(" 22 Harmonica"));
    AppendMenu(hWoodMnu,MF_ENABLED|MF_STRING,WM_USER+151,_T(" 23 Tango Accordion"));
    AppendMenu(hWoodMnu,MF_ENABLED|MF_STRING,WM_USER+192,_T(" 64 Soprano Sax"));
    AppendMenu(hWoodMnu,MF_ENABLED|MF_STRING,WM_USER+193,_T(" 65 Alto Sax"));
    AppendMenu(hWoodMnu,MF_ENABLED|MF_STRING,WM_USER+194,_T(" 66 Tenor Sax"));
    AppendMenu(hWoodMnu,MF_ENABLED|MF_STRING,WM_USER+195,_T(" 67 Baritone Sax"));
    AppendMenu(hWoodMnu,MF_ENABLED|MF_STRING,WM_USER+196,_T(" 68 Oboe"));
    AppendMenu(hWoodMnu,MF_ENABLED|MF_STRING,WM_USER+197,_T(" 69 English Horn"));
    AppendMenu(hWoodMnu,MF_ENABLED|MF_STRING,WM_USER+198,_T(" 70 Bassoon"));
    AppendMenu(hWoodMnu,MF_ENABLED|MF_STRING,WM_USER+199,_T(" 71 Clarinet"));
    AppendMenu(hWoodMnu,MF_ENABLED|MF_STRING,WM_USER+200,_T(" 72 Piccolo"));
    AppendMenu(hWoodMnu,MF_ENABLED|MF_STRING,WM_USER+201,_T(" 73 Flute"));
    AppendMenu(hWoodMnu,MF_ENABLED|MF_STRING,WM_USER+202,_T(" 74 Recorder"));
    AppendMenu(hWoodMnu,MF_ENABLED|MF_STRING,WM_USER+203,_T(" 75 Pan Flute"));
    AppendMenu(hWoodMnu,MF_ENABLED|MF_STRING,WM_USER+204,_T(" 76 Bottle Blow"));
    AppendMenu(hWoodMnu,MF_ENABLED|MF_STRING,WM_USER+205,_T(" 77 Shaku"));
    AppendMenu(hWoodMnu,MF_ENABLED|MF_STRING,WM_USER+206,_T(" 78 Whistle"));
    AppendMenu(hWoodMnu,MF_ENABLED|MF_STRING,WM_USER+207,_T(" 79 Ocarina"));
    AppendMenu(hWoodMnu,MF_ENABLED|MF_STRING,WM_USER+237,_T("109 Bagpipe"));
    AppendMenu(hWoodMnu,MF_ENABLED|MF_STRING,WM_USER+239,_T("111 Shahnai"));
    AppendMenu(hInstrumentMnu,MF_ENABLED|MF_POPUP,(UINT)hWoodMnu,_T("&Woodwind"));

    AppendMenu(hMiscMnu,MF_ENABLED|MF_STRING,WM_USER+180,_T(" 52 Choir Aahs"));
    AppendMenu(hMiscMnu,MF_ENABLED|MF_STRING,WM_USER+181,_T(" 53 Voice Oohs"));
    AppendMenu(hMiscMnu,MF_ENABLED|MF_STRING,WM_USER+182,_T(" 54 Synth Voice"));
    AppendMenu(hMiscMnu,MF_ENABLED|MF_STRING,WM_USER+208,_T(" 80 Lead 1 (square)"));
    AppendMenu(hMiscMnu,MF_ENABLED|MF_STRING,WM_USER+209,_T(" 81 Lead 2 (sawtooth)"));
    AppendMenu(hMiscMnu,MF_ENABLED|MF_STRING,WM_USER+210,_T(" 82 Lead 3 (calliope lead)"));
    AppendMenu(hMiscMnu,MF_ENABLED|MF_STRING,WM_USER+211,_T(" 83 Lead 4 (chiff lead)"));
    AppendMenu(hMiscMnu,MF_ENABLED|MF_STRING,WM_USER+212,_T(" 84 Lead 5 (charang)"));
    AppendMenu(hMiscMnu,MF_ENABLED|MF_STRING,WM_USER+213,_T(" 85 Lead 6 (voice)"));
    AppendMenu(hMiscMnu,MF_ENABLED|MF_STRING,WM_USER+214,_T(" 86 Lead 7 (fifths)"));
    AppendMenu(hMiscMnu,MF_ENABLED|MF_STRING,WM_USER+215,_T(" 87 Lead 8 (bass + lead)"));
    AppendMenu(hMiscMnu,MF_ENABLED|MF_STRING,WM_USER+216,_T(" 88 Pad 1 (new age)"));
    AppendMenu(hMiscMnu,MF_ENABLED|MF_STRING,WM_USER+217,_T(" 89 Pad 2 (warm)"));
    AppendMenu(hMiscMnu,MF_ENABLED|MF_STRING,WM_USER+218,_T(" 90 Pad 3 (poly synth)"));
    AppendMenu(hMiscMnu,MF_ENABLED|MF_STRING,WM_USER+219,_T(" 91 Pad 4 (choir)"));
    AppendMenu(hMiscMnu,MF_ENABLED|MF_STRING,WM_USER+220,_T(" 92 Pad 5 (bowed)"));
    AppendMenu(hMiscMnu,MF_ENABLED|MF_STRING,WM_USER+221,_T(" 93 Pad 6 (metallic)"));
    AppendMenu(hMiscMnu,MF_ENABLED|MF_STRING,WM_USER+222,_T(" 94 Pad 7 (halo)"));
    AppendMenu(hMiscMnu,MF_ENABLED|MF_STRING,WM_USER+223,_T(" 95 Pad 8 (sweep)"));
    AppendMenu(hMiscMnu,MF_ENABLED|MF_STRING,WM_USER+224,_T(" 96 FX 1 (rain)"));
    AppendMenu(hMiscMnu,MF_ENABLED|MF_STRING,WM_USER+225,_T(" 97 FX 2 (sound track)"));
    AppendMenu(hMiscMnu,MF_ENABLED|MF_STRING,WM_USER+226,_T(" 98 FX 3 (crystal)"));
    AppendMenu(hMiscMnu,MF_ENABLED|MF_STRING,WM_USER+227,_T(" 99 FX 4 (atmosphere)"));
    AppendMenu(hMiscMnu,MF_ENABLED|MF_STRING,WM_USER+228,_T("100 FX 5 (bright)"));
    AppendMenu(hMiscMnu,MF_ENABLED|MF_STRING,WM_USER+229,_T("101 FX 6 (goblins)"));
    AppendMenu(hMiscMnu,MF_ENABLED|MF_STRING,WM_USER+230,_T("102 FX 7 (echoes)"));
    AppendMenu(hMiscMnu,MF_ENABLED|MF_STRING,WM_USER+231,_T("103 FX 8 (sci-fi)"));
    AppendMenu(hMiscMnu,MF_ENABLED|MF_STRING,WM_USER+249,_T("121 Breath Noise"));
    AppendMenu(hMiscMnu,MF_ENABLED|MF_STRING,WM_USER+250,_T("122 Seashore"));
    AppendMenu(hMiscMnu,MF_ENABLED|MF_STRING,WM_USER+251,_T("123 Bird Tweet"));
    AppendMenu(hMiscMnu,MF_ENABLED|MF_STRING,WM_USER+252,_T("124 Telephone Ring"));
    AppendMenu(hMiscMnu,MF_ENABLED|MF_STRING,WM_USER+253,_T("125 Helicopter"));
    AppendMenu(hMiscMnu,MF_ENABLED|MF_STRING,WM_USER+254,_T("126 Applause"));
    AppendMenu(hMiscMnu,MF_ENABLED|MF_STRING,WM_USER+255,_T("127 Gunshot"));

    AppendMenu(hInstrumentMnu,MF_ENABLED|MF_POPUP,(UINT)hMiscMnu,_T("&Miscellaneous"));
}

CInstrumentMenu::~CInstrumentMenu()
{
    DestroyMenu(hMiscMnu);
    DestroyMenu(hWoodMnu);
    DestroyMenu(hBrassMnu);
    DestroyMenu(hStringMnu);
    DestroyMenu(hPercussionMnu);
    DestroyMenu(hInstrumentMnu);
    hMiscMnu=hWoodMnu=hBrassMnu=hStringMnu=hPercussionMnu=hInstrumentMnu=NULL;
}

void CInstrumentMenu::Check(int Instrument)
{
    CheckMenuItem(hInstrumentMnu,WM_USER+128+Instrument,MF_CHECKED);
    if (Instrument<16||Instrument==47||Instrument==108||(Instrument>=112&&Instrument<=119))
    {
        CheckMenuItem(hInstrumentMnu,0,MF_BYPOSITION|MF_CHECKED);    // Percussion
    }
    else if (Instrument>=56&&Instrument<=63)
    {
        CheckMenuItem(hInstrumentMnu,2,MF_BYPOSITION|MF_CHECKED);    // Brass
    }
    else if ((Instrument>=16&&Instrument<=23)||(Instrument>=64&&Instrument<=79)||Instrument==109||Instrument==111)
    {
        CheckMenuItem(hInstrumentMnu,3,MF_BYPOSITION|MF_CHECKED);    // Woodwind
    }
    else if ((Instrument>=52&&Instrument<=54)||(Instrument>=80&&Instrument<=103)||(Instrument>=121&&Instrument<=127))
    {
        CheckMenuItem(hInstrumentMnu,4,MF_BYPOSITION|MF_CHECKED);    // Miscellaneous
    }
    else
    {
        CheckMenuItem(hInstrumentMnu,1,MF_BYPOSITION|MF_CHECKED);    // String
    }
}


// ***************
// CTempoMenu
// ***************

#define WM_TEMPO (WM_MOUSEFIRST-200)

class CTempoMenu
{
    HMENU hTempoMnu;
    HMENU hAdagioMnu;
    HMENU hAndanteMnu;
    HMENU hModeratoMnu;
    HMENU hAllegroMnu;
    HMENU hAllegrettoMnu;
    HMENU hPrestoMnu;

public:
    CTempoMenu(int iTempo=0);
    ~CTempoMenu();
    void Check(int iTempo);
    HMENU Menu()
    {
        return hTempoMnu;
    }
};

CTempoMenu::CTempoMenu(int /*iTempo*/)
{
    hTempoMnu = CreatePopupMenu();
    hAdagioMnu = CreatePopupMenu();
    hAndanteMnu = CreatePopupMenu();
    hModeratoMnu = CreatePopupMenu();
    hAllegroMnu = CreatePopupMenu();
    hAllegrettoMnu = CreatePopupMenu();
    hPrestoMnu = CreatePopupMenu();

    AppendMenu(hAdagioMnu,MF_ENABLED|MF_STRING,WM_TEMPO+52,_T("52"));
    AppendMenu(hAdagioMnu,MF_ENABLED|MF_STRING,WM_TEMPO+56,_T("56"));
    AppendMenu(hAdagioMnu,MF_ENABLED|MF_STRING,WM_TEMPO+60,_T("60"));
    AppendMenu(hAdagioMnu,MF_ENABLED|MF_STRING,WM_TEMPO+64,_T("64"));
    AppendMenu(hAdagioMnu,MF_ENABLED|MF_STRING,WM_TEMPO+68,_T("68"));
    AppendMenu(hAdagioMnu,MF_ENABLED|MF_STRING,WM_TEMPO+72,_T("72"));
    AppendMenu(hTempoMnu,MF_ENABLED|MF_POPUP,(UINT)hAdagioMnu,_T("52-72\t(Adagio)"));

    AppendMenu(hAndanteMnu,MF_ENABLED|MF_STRING,WM_TEMPO+76,_T("76"));
    AppendMenu(hAndanteMnu,MF_ENABLED|MF_STRING,WM_TEMPO+80,_T("80"));
    AppendMenu(hAndanteMnu,MF_ENABLED|MF_STRING,WM_TEMPO+84,_T("84"));
    AppendMenu(hAndanteMnu,MF_ENABLED|MF_STRING,WM_TEMPO+88,_T("88"));
    AppendMenu(hAndanteMnu,MF_ENABLED|MF_STRING,WM_TEMPO+92,_T("92"));
    AppendMenu(hAndanteMnu,MF_ENABLED|MF_STRING,WM_TEMPO+96,_T("96"));
    AppendMenu(hTempoMnu,MF_ENABLED|MF_POPUP,(UINT)hAndanteMnu,_T("76-96\t(Andante)"));

    AppendMenu(hModeratoMnu,MF_ENABLED|MF_STRING,WM_TEMPO+100,_T("100"));
    AppendMenu(hModeratoMnu,MF_ENABLED|MF_STRING,WM_TEMPO+104,_T("104"));
    AppendMenu(hModeratoMnu,MF_ENABLED|MF_STRING,WM_TEMPO+108,_T("108"));
    AppendMenu(hModeratoMnu,MF_ENABLED|MF_STRING,WM_TEMPO+112,_T("112"));
    AppendMenu(hModeratoMnu,MF_ENABLED|MF_STRING,WM_TEMPO+116,_T("116"));
    AppendMenu(hModeratoMnu,MF_ENABLED|MF_STRING,WM_TEMPO+120,_T("120"));
    AppendMenu(hTempoMnu,MF_ENABLED|MF_POPUP,(UINT)hModeratoMnu,_T("100-120\t(Moderato)"));

    AppendMenu(hAllegroMnu,MF_ENABLED|MF_STRING,WM_TEMPO+124,_T("124"));
    AppendMenu(hAllegroMnu,MF_ENABLED|MF_STRING,WM_TEMPO+128,_T("128"));
    AppendMenu(hAllegroMnu,MF_ENABLED|MF_STRING,WM_TEMPO+132,_T("132"));
    AppendMenu(hAllegroMnu,MF_ENABLED|MF_STRING,WM_TEMPO+136,_T("136"));
    AppendMenu(hAllegroMnu,MF_ENABLED|MF_STRING,WM_TEMPO+140,_T("140"));
    AppendMenu(hAllegroMnu,MF_ENABLED|MF_STRING,WM_TEMPO+144,_T("144"));
    AppendMenu(hAllegroMnu,MF_ENABLED|MF_STRING,WM_TEMPO+148,_T("148"));
    AppendMenu(hTempoMnu,MF_ENABLED|MF_POPUP,(UINT)hAllegroMnu,_T("124-148\t(Allegro)"));

    AppendMenu(hAllegrettoMnu,MF_ENABLED|MF_STRING,WM_TEMPO+152,_T("152"));
    AppendMenu(hAllegrettoMnu,MF_ENABLED|MF_STRING,WM_TEMPO+154,_T("156"));
    AppendMenu(hAllegrettoMnu,MF_ENABLED|MF_STRING,WM_TEMPO+160,_T("160"));
    AppendMenu(hAllegrettoMnu,MF_ENABLED|MF_STRING,WM_TEMPO+164,_T("164"));
    AppendMenu(hAllegrettoMnu,MF_ENABLED|MF_STRING,WM_TEMPO+168,_T("168"));
    AppendMenu(hAllegrettoMnu,MF_ENABLED|MF_STRING,WM_TEMPO+172,_T("172"));
    AppendMenu(hTempoMnu,MF_ENABLED|MF_POPUP,(UINT)hAllegrettoMnu,_T("152-172\t(Allegretto)"));

    AppendMenu(hPrestoMnu,MF_ENABLED|MF_STRING,WM_TEMPO+176,_T("176"));
    AppendMenu(hPrestoMnu,MF_ENABLED|MF_STRING,WM_TEMPO+180,_T("180"));
    AppendMenu(hPrestoMnu,MF_ENABLED|MF_STRING,WM_TEMPO+184,_T("184"));
    AppendMenu(hPrestoMnu,MF_ENABLED|MF_STRING,WM_TEMPO+188,_T("188"));
    AppendMenu(hPrestoMnu,MF_ENABLED|MF_STRING,WM_TEMPO+192,_T("192"));
    AppendMenu(hPrestoMnu,MF_ENABLED|MF_STRING,WM_TEMPO+196,_T("196"));
    AppendMenu(hPrestoMnu,MF_ENABLED|MF_STRING,WM_TEMPO+200,_T("200"));
    AppendMenu(hTempoMnu,MF_ENABLED|MF_POPUP,(UINT)hPrestoMnu,_T("176-200\t(Presto)"));
}

CTempoMenu::~CTempoMenu()
{
    DestroyMenu(hAdagioMnu);
    DestroyMenu(hAndanteMnu);
    DestroyMenu(hModeratoMnu);
    DestroyMenu(hAllegroMnu);
    DestroyMenu(hAllegrettoMnu);
    DestroyMenu(hPrestoMnu);
    DestroyMenu(hTempoMnu);

    hAdagioMnu=hAndanteMnu=hModeratoMnu=hAllegroMnu=hAllegrettoMnu=hPrestoMnu=hTempoMnu=NULL;
}

void CTempoMenu::Check(int iTempo)
{
    CheckMenuItem(hTempoMnu,WM_TEMPO+iTempo,MF_CHECKED);

    if (iTempo<76)
    {
        CheckMenuItem(hTempoMnu,0,MF_BYPOSITION|MF_CHECKED);    // Adagio
    }
    else if (iTempo<100)
    {
        CheckMenuItem(hTempoMnu,1,MF_BYPOSITION|MF_CHECKED);    // Andante
    }
    else if (iTempo<124)
    {
        CheckMenuItem(hTempoMnu,2,MF_BYPOSITION|MF_CHECKED);    // Moderato
    }
    else if (iTempo<152)
    {
        CheckMenuItem(hTempoMnu,3,MF_BYPOSITION|MF_CHECKED);    // Alegro
    }
    else if (iTempo<176)
    {
        CheckMenuItem(hTempoMnu,4,MF_BYPOSITION|MF_CHECKED);    // Allegretto
    }
    else
    {
        CheckMenuItem(hTempoMnu,5,MF_BYPOSITION|MF_CHECKED);    // Presto
    }
}


// **************
// SPartWindowData
// **************

SPartWindowData::SPartWindowData(HWND hWnd,int Size) : m_stMThdChunk(NULL)
{
    Active = (TRUE);
    Selecting = (FALSE);
    Control = (FALSE);
    Enabled = (TRUE);
    SelectBegin = 0;
    SelectEnd = 0;
    ViewBegin = 0;
    ViewWidthPixels = 0;
    MelodySize = 0;
    MelodyBufferSize = Size;
    hFont = NULL;
    hMyself = hWnd;
    hPlayButton = NULL;
    hPauseButton = NULL;
    hStopButton = NULL;
    hLoopButton = NULL;
    Clef = 1;
    Instrument = 41; // viola
    Tempo = 120; // quarter notes per minute
    m_sPlay.Tempo = 0; // not playing
    Modified = (FALSE);

    Melody = new pNote[Size];
    register int i;
    for (i=0; i<Size; ++i)
    {
        Melody[i]=NULL;
    }

	for (i=0; i<MUSIQUE_FONT_SIZE; ++i)
    {
        FontCharWidth[i]=0;
    }

    m_stMThdChunk = new SMTHeaderChunk();
    if (m_stMThdChunk==NULL)
    {
        return;
    }

    m_stMTrkHeader = new SChunkHeader();
    if (m_stMTrkHeader==NULL)
    {
        delete m_stMThdChunk;
        m_stMThdChunk=NULL;
        return;
    }

}

SPartWindowData::~SPartWindowData()
{
    Active=Enabled=Selecting=FALSE;
    SelectBegin=SelectEnd=ViewBegin=ViewWidthPixels=Clef=Modified=0;
    if (hFont)
    {
        DeleteObject(hFont);
        hFont = NULL;
    }
    hMyself=NULL;
    if (hPlayButton)
    {
        DestroyWindow(hPlayButton);
        hPlayButton = NULL;
    }
    if (hPauseButton)
    {
        DestroyWindow(hPauseButton);
        hPauseButton = NULL;
    }
    if (hStopButton)
    {
        DestroyWindow(hStopButton);
        hStopButton = NULL;
    }
    if (hLoopButton)
    {
        DestroyWindow(hLoopButton);
        hLoopButton = NULL;
    }
    if (Melody)
    {
        for (unsigned int i=0; i<MelodyBufferSize; i++)
        {
            if (Melody[i])
            {
                delete Melody[i];
                Melody[i]=NULL;
            }
        }
        delete Melody;
        Melody = NULL;
        MelodySize=MelodyBufferSize=0;
    }
    if (m_stMThdChunk)
    {
        delete m_stMThdChunk;
        m_stMThdChunk = NULL;
    }
    if (m_stMTrkHeader)
    {
        delete m_stMTrkHeader;
        m_stMTrkHeader = NULL;
    }

}//SPartWindowData destructor

void SPartWindowData::Grow(int Size)
{
    register unsigned i=0;
    MelodyBufferSize += Size;

    pNote * NewMelody = new pNote[MelodyBufferSize];
    for (; i<MelodySize; ++i)
    {
        NewMelody[i] = new SNote(*Melody[i]);  // copy old notes to new array
        delete Melody[i];
        Melody[i] = NULL;
    }
    delete Melody;
    Melody = NewMelody;

    for (; i<MelodyBufferSize; ++i)   // initialize newly grown area
    {
        Melody[i]=NULL;
    }
}

int SPartWindowData::WhichClef()
{
    if (!MelodySize)
    {
        return (Clef=1);    // NOTHING TO DISPLAY (DEFAULT:TREBLE)
    }

    int ClefHit[5] = {0,0,0,0,0};

    for (unsigned register i=0; i<MelodySize; ++i)
    {
        if (!Melody[i] || Melody[i]->Value < 0)
        {
            continue;    // not a note
        }

        for (register int j=0; j<5; ++j)
            if (Melody[i]->Value >= ClefRange[j][0] && Melody[i]->Value <= ClefRange[j][1])
            {
                ++ClefHit[j];
            }
    }

    // Treble and bass clefs are preferred, so penalize the non-standard clefs
    ClefHit[0] = 3 * ClefHit[0] / 4;
    ClefHit[2] = 3 * ClefHit[2] / 4;
    ClefHit[4] = 3 * ClefHit[4] / 4;

    int NewClef = 0;
    for (register int k=1; k<5; ++k)
    {
        if (ClefHit[k]<ClefHit[NewClef])
        {
            continue;
        }
        if (ClefHit[k]==ClefHit[NewClef] && Clef!=k)
        {
            continue;
        }
        NewClef=k;
    }

    return (Clef=NewClef);
}

void SPartWindowData::SetClef(int NewClef)
{
    Clef = NewClef;
    for (register unsigned i=0; i<MelodySize; ++i)
    {
        Melody[i]->SetClef(NewClef);
    }
    InvalidateRgn(hMyself,NULL,0);
}

WCHAR * SPartWindowData::PartEncode(unsigned Begin,unsigned End)
{
    unsigned X = 0;

    if (End>MelodySize)
    {
        End = MelodySize-1;
    }

    if (!MelodySize || Begin>MelodySize || Begin>End)
    {
        return (WCHAR *)(NULL);
    }

    WCHAR * aString = new WCHAR [((End-Begin)*5) + 1];

    unsigned register j=0;
    for (unsigned register i=Begin; i<End; ++i)
    {
        if (!Melody[i])
        {
            continue;
        }
        // ONSET
        if (Melody[i]->DisplayOnset)
        {
            aString[j++]=Melody[i]->DisplayOnset;
            X += FontCharWidth[Melody[i]->DisplayOnset];
        }
        // ACCIDENTAL
        if (Melody[i]->DisplayAccidental)
        {
            aString[j++]=Melody[i]->DisplayAccidental;
            X += FontCharWidth[Melody[i]->DisplayAccidental];
        }
        // MOD-DURATION
        if (Melody[i]->DisplayModDur)
        {
            aString[j++]=Melody[i]->DisplayModDur;
            X += FontCharWidth[Melody[i]->DisplayModDur];
        }
        // NOTE (NAME/OCTAVE/DURATION)
        if (Melody[i]->DisplayNote)
        {
            aString[j++]=Melody[i]->DisplayNote;
            X += FontCharWidth[Melody[i]->DisplayNote];
        }
        // CONCLUSION
        if (Melody[i]->DisplayConclusion)
        {
            aString[j++]=Melody[i]->DisplayConclusion;
            X += FontCharWidth[Melody[i]->DisplayConclusion];
        }
    }
    aString[j]='\0';
    return aString;
}

int SPartWindowData::wcslenLimited(WCHAR * pWchar, int nPixels)
{
    int nChar = 0;

    for (int nPixelsUsed = 0; pWchar[nChar]; nChar++)
    {
        int nUsed = FontCharWidth[pWchar[nChar]];

        if (nPixelsUsed > nPixels && nUsed)
        {
            break;
        }

        nPixelsUsed += nUsed;
    }

    return nChar;
}

char * SPartWindowData::GetString(unsigned Begin, unsigned End)
{
    if (End>MelodySize)
    {
        End = MelodySize;
    }

    if (!MelodySize || Begin>MelodySize || Begin>End)
    {
        return (char *)(NULL);
    }

    char * aString = NULL;
    UINT aStringSize = ((End-Begin)*7) + 1; // ((1+(End-Begin))*7) + 1;
    UINT aStringExtra = 0;
    if (!Begin && End==MelodySize)
    {
        // copying whole string; add clef and instrument info to output
        aStringExtra = 21;
        aString = new char[aStringSize+aStringExtra];

        strcpy_s(aString,aStringSize+aStringExtra,"(CLEFx)(VXxxx)(Q=xxx)");
        aString[5] = (char)('0'+Clef);
        aString[10] = (char)((Instrument/100) + '0');
        aString[11] = (char)(((Instrument%100)/10) + '0');
        aString[12] = (char)((Instrument%10) + '0');
        aString[17] = (char)((Tempo/100)+'0');
        aString[18] = (char)(((Tempo%100)/10) + '0');
        aString[19] = (char)((Tempo%10) + '0');
    }
    else
    {
        aString = new char[aStringSize];
    }

    register unsigned i;
    for (i=0; i<(End-Begin); ++i)
    {
        if (strchr("ABCDEFGR",Melody[Begin+i]->Name))
        {
            aString[i*7+aStringExtra]=Melody[Begin+i]->Name;
            aString[i*7+1+aStringExtra]=Melody[Begin+i]->Accidental;
            aString[i*7+2+aStringExtra]=Melody[Begin+i]->Octave;
            aString[i*7+3+aStringExtra]=Melody[Begin+i]->Onset;
            aString[i*7+4+aStringExtra]=Melody[Begin+i]->Duration;
            aString[i*7+5+aStringExtra]=Melody[Begin+i]->Duration2;
            aString[i*7+6+aStringExtra]=Melody[Begin+i]->Conclusion;
        }
        else switch (Melody[Begin+i]->Name)
            {
            case 0:
                strcpy_s(&aString[i*7+aStringExtra],aStringSize+aStringExtra-(i*7+aStringExtra),"(FIELD)");
                break;
            case ANNOT_BAR:
                strcpy_s(&aString[i*7+aStringExtra],aStringSize+aStringExtra-(i*7+aStringExtra),"(-BAR-)");
                break;
            case ANNOT_THICK_BAR:
                strcpy_s(&aString[i*7+aStringExtra],aStringSize+aStringExtra-(i*7+aStringExtra),"(=BAR=)");
                break;
            case ANNOT_PHRASE:
                strcpy_s(&aString[i*7+aStringExtra],aStringSize+aStringExtra-(i*7+aStringExtra),"(\'PHR\')");
                break;
            case ANNOT_PHRASE2:
                strcpy_s(&aString[i*7+aStringExtra],aStringSize+aStringExtra-(i*7+aStringExtra),"(\"PHR\")");
                break;
            case ANNOT_SPACE:
                strcpy_s(&aString[i*7+aStringExtra],aStringSize+aStringExtra-(i*7+aStringExtra),"(SPACE)");
                break;
            default:
                strcpy_s(&aString[i*7+aStringExtra],aStringSize+aStringExtra-(i*7+aStringExtra),"(??" "??" "?)");
                break;
            }
    }
    aString[i*7+aStringExtra]='\0';
    return aString;
}

int SPartWindowData::ReplaceString(char * String,unsigned Begin,unsigned End)
{
    int InsertionLength=0;

    if (String)
    {
        while (*String == '(')
        {
            if (!strncmp(String+1,"CLEF",4))   // display clef
            {
                Clef = String[5]-'0';
                String += 7;
            }
            else if (!strncmp(String+1,"VX",2))     // playback voice (instrument)
            {
                Instrument  = (char)(String[3]-'0');
                Instrument *= 10;
                Instrument = (char)(Instrument + (String[4]-'0'));
                Instrument *= 10;
                Instrument = (char)(Instrument + (String[5]-'0'));
                String += 7;
            }
            else if (!strncmp(String+1,"Q=",2))     // playback tempo
            {
                Tempo = String[3]-'0';
                Tempo *= 10;
                Tempo += String[4]-'0';
                Tempo *= 10;
                Tempo += String[5]-'0';
                String += 7;
            }
            else
            {
                break;
            }
        }
        InsertionLength = strlen(String);
        InsertionLength /= 7;
    }

    int ReplacedLength = End - Begin;

    // add space to Melody buffer if necessary
    if (ReplacedLength<InsertionLength &&
            MelodySize+(InsertionLength-ReplacedLength)>MelodyBufferSize)
    {
        Grow(DELTA+InsertionLength-ReplacedLength);
    }

    // copy what you can
    for (register int i=0; i<InsertionLength&&i<ReplacedLength; ++i)
    {
        Melody[Begin+i]->Set((unsigned char *)&String[i*7],Clef);
    }

    int NetLengthChange = InsertionLength - (End - Begin);

    // move over to add remaining notes if any
    if (NetLengthChange>0)
    {
        register int i;
        if (MelodySize)
        {
            // move over
            for (i=MelodySize-1; i>=(int)End; --i)
                if (Melody[i+NetLengthChange])
                {
                    *Melody[i+NetLengthChange]=*Melody[i];
                }
                else
                {
                    Melody[i+NetLengthChange]= new SNote(*Melody[i]);
                }
        }
        // add notes
        for (i=InsertionLength-1; i>=ReplacedLength; --i)
        {
            if (Melody[Begin+i])
            {
                Melody[Begin+i]->Set((unsigned char *)&String[i*7],Clef);
            }
            else
            {
                SNote * np = new SNote((unsigned char *)&String[i*7],Clef);
                Melody[Begin+i] = np;
            }
        }
        MelodySize += NetLengthChange;
    }

    // else move over to delete additional notes if any
    else if (NetLengthChange<0)
    {
        MelodySize += NetLengthChange;
        for (register int i=End+NetLengthChange; i<=(int)MelodySize; ++i)
        {
            if (Melody[i])
            {
                delete Melody[i];
            }
            if (Melody[i-NetLengthChange])
            {
                Melody[i] = new SNote(*Melody[i-NetLengthChange]);
            }
            else
            {
                Melody[i] = NULL;
            }
        }
    }

    if (Clef < 0 || Clef > 4)
    {
        WhichClef();
    }
    InvalidateRgn(hMyself,NULL,0);
    return InsertionLength;
}

int SPartWindowData::Paint()
{
    SetScrollRange(hMyself,SB_HORZ,0,MelodySize-1,1);
    SetScrollPos(hMyself,SB_HORZ,ViewBegin,1);

    // get window dimensions
    RECT r;
    GetClientRect(hMyself,&r);
    // start up
    PAINTSTRUCT ps;
    BeginPaint(hMyself,&ps);
    // create font handle, if needed
    if (!hFont)
    {
        hFont = CreateFont(r.bottom<25?25:r.bottom,0,0,0,FW_DONTCARE,FALSE,FALSE,FALSE,
                           ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS|0x0400,DEFAULT_QUALITY,DEFAULT_PITCH|FF_DONTCARE,"Musique");
        SelectObject(ps.hdc,hFont);
        BOOL bResult = GetCharWidth32W(ps.hdc,0,MUSIQUE_FONT_SIZE-1,FontCharWidth);
        if (!bResult)
        {
            GetCharWidthW(ps.hdc,0,MUSIQUE_FONT_SIZE-1,FontCharWidth);
        }
    }
    else
    {
        SelectObject(ps.hdc,hFont);
    }

    // clear frame
    FillRect(ps.hdc,&ps.rcPaint,(HBRUSH)(COLOR_WINDOW+1));
    r.left+=2;
    r.top+=2;
    r.right-=4;
    r.bottom-=4;

    // Quick draw string
    SetTextColor(ps.hdc,Enabled?DISPLAY_ACTIVE_TEXT:DISPLAY_INACTIVE_TEXT);
    SetBkColor(ps.hdc,DISPLAY_BKG);
    WCHAR * Sequence = PartEncode(ViewBegin,MelodySize);

    if (Sequence)
    {
        int SequenceLength = wcslen(Sequence);
        WCHAR * FullSequence = new WCHAR [SequenceLength+5];
        switch (Clef)
        {
        case 1:
            *FullSequence = DISPLAY_TREBLE;
            break;
        case 2:
            *FullSequence = DISPLAY_TREBLE_8VB;
            break;
        case 3:
            *FullSequence = DISPLAY_BASS;
            break;
        case 0:
            *FullSequence = DISPLAY_TREBLE_8VA;
            break;
        case 4:
            *FullSequence = DISPLAY_BASS_8VB;
            break;
        }
        wcscpy_s(FullSequence+1,SequenceLength+5-1,Sequence);
        FullSequence[1+SequenceLength] = DISPLAY_BAR;
        FullSequence[2+SequenceLength] = DISPLAY_SMALL_SPACE;
        FullSequence[3+SequenceLength] = DISPLAY_THICK_BAR;
        FullSequence[4+SequenceLength] = '\0';
        TextOutW(ps.hdc,r.left,r.top,FullSequence,wcslenLimited(FullSequence, r.right-r.left));

        r.left += FontCharWidth[*FullSequence] - 1;

        delete Sequence;

        if (ViewBegin<SelectMin())
        {
            Sequence = PartEncode(ViewBegin,SelectMin());
            for (register int i=0; Sequence[i]; ++i)
            {
                r.left += FontCharWidth[Sequence[i]];
            }
            delete Sequence;
        }

        if (ViewBegin)
        {
            if (Enabled)
            {
                SetTextColor(ps.hdc,DISPLAY_INACTIVE_TEXT);
            }
            FullSequence[1] = DISPLAY_CONTINUED;
            TextOutW(ps.hdc,2,2,FullSequence,2);
            if (Enabled)
            {
                SetTextColor(ps.hdc,DISPLAY_ACTIVE_TEXT);
            }
        }
        delete FullSequence;
    }
    else     // quick draw empty staff
    {
        WCHAR EmptyStaff[5]
            = { DISPLAY_TREBLE,DISPLAY_BAR,DISPLAY_SMALL_SPACE,DISPLAY_THICK_BAR,'\0'};
        switch (Clef)
        {
        case 3: // BASS
            *EmptyStaff = DISPLAY_BASS;
            break;
        case 0: // TREBLE 8VA
            *EmptyStaff = DISPLAY_TREBLE_8VA;
            break;
        case 2: // TREBLE 8VB
            *EmptyStaff = DISPLAY_TREBLE_8VB;
            break;
        case 4: // BASS 8VB
            *EmptyStaff = DISPLAY_BASS_8VB;
            break;
        }
        TextOutW(ps.hdc,r.left,r.top,EmptyStaff,4);
        r.left += FontCharWidth[*EmptyStaff] - 1;
    }

    // if nothing is selected, draw cursor
    if (SelectBegin==SelectEnd)
    {
        if (SelectBegin>=ViewBegin)
        {
            // Draw cursor
            HPEN CursorPen = ::CreatePen(PS_SOLID,0,DISPLAY_CURSOR);
            HPEN OldPen = (HPEN)SelectObject(ps.hdc,CursorPen);
            Rectangle(ps.hdc,r.left,r.top,r.left+1,r.bottom<25?25:r.bottom);
            SelectObject(ps.hdc,OldPen);
            DeleteObject(CursorPen);
        }
    }
    // otherwise, draw selection
    else
    {
        // Draw selection
        if (ViewBegin<SelectMax())
        {
            SetTextColor(ps.hdc,Enabled?DISPLAY_SELECT_TEXT:DISPLAY_INACTIVE_TEXT);
            SetBkColor(ps.hdc,DISPLAY_SELECT_BKG);
            WCHAR * Sequence = NULL;
            if (ViewBegin>SelectMin())
            {
                Sequence = PartEncode(ViewBegin,SelectMax());
            }
            else
            {
                Sequence = PartEncode(SelectMin(),SelectMax());
            }
            if (Sequence)
            {
                TextOutW(ps.hdc,r.left,r.top,Sequence,wcslenLimited(Sequence, r.right-r.left));
                delete Sequence;
            }
        }

    }

    if (r.bottom>=50)
    {
        SetTextColor(ps.hdc, DISPLAY_ACTIVE_TEXT);
        SetBkColor(ps.hdc,DISPLAY_BKG);

        HFONT hMSSansSerif = CreateFont(11, 0, 0, 0, FW_NORMAL, 0, 0, 0,
                                        ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                        DEFAULT_QUALITY, (VARIABLE_PITCH | FF_ROMAN),"MS Sans Serif");
        HANDLE hOldFont = SelectObject(ps.hdc, hMSSansSerif);
        char szTempo[12];
        strcpy_s(szTempo,_countof(szTempo), "MM = XXX");
        _ltoa_s(Tempo, &szTempo[5], _countof(szTempo)-5, 10);
        TextOut(ps.hdc, 3, 3, szTempo, strlen(szTempo));
        SelectObject(ps.hdc, hOldFont);
        DeleteObject(hMSSansSerif);
    }

    // finish up
    ValidateRect(hMyself,&ps.rcPaint);
    if (hPlayButton)
    {
        InvalidateRgn(hPlayButton,NULL,FALSE);
    }
    if (hPauseButton)
    {
        InvalidateRgn(hPauseButton,NULL,FALSE);
    }
    if (hStopButton)
    {
        InvalidateRgn(hStopButton,NULL,FALSE);
    }
    if (hLoopButton)
    {
        InvalidateRgn(hLoopButton,NULL,FALSE);
    }
    EndPaint(hMyself,&ps);
    return 0;
}

unsigned SPartWindowData::Width(unsigned From, unsigned Through)
{
    if (From>=MelodySize)
    {
        return 0;
    }
    if (Through<MelodySize)
    {
        Through=MelodySize;
    }

    unsigned SectionWidth = 0;
    for (register int i=From; i<=(int)Through; ++i)
    {
        if (Melody[i])
        {
            if (Melody[i]->DisplayOnset)
            {
                SectionWidth += FontCharWidth[Melody[i]->DisplayOnset];
            }
            if (Melody[i]->DisplayAccidental)
            {
                SectionWidth += FontCharWidth[Melody[i]->DisplayAccidental];
            }
            if (Melody[i]->DisplayModDur)
            {
                SectionWidth += FontCharWidth[Melody[i]->DisplayModDur];
            }
            if (Melody[i]->DisplayNote)
            {
                SectionWidth += FontCharWidth[Melody[i]->DisplayNote];
            }
            if (Melody[i]->DisplayConclusion)
            {
                SectionWidth += FontCharWidth[Melody[i]->DisplayConclusion];
            }
        }
    }
    return SectionWidth;
}

unsigned SPartWindowData::Position(unsigned Pixels)
{
    if (!Pixels)
    {
        return 0;
    }
    if (ViewBegin>=MelodySize)
    {
        return MelodySize;
    }
    unsigned Left=0,Right=0;
    switch (Clef)
    {
    case 0:
        Left=Right=FontCharWidth[DISPLAY_TREBLE_8VA];
        break;
    case 1:
        Left=Right=FontCharWidth[DISPLAY_TREBLE];
        break;
    case 2:
        Left=Right=FontCharWidth[DISPLAY_TREBLE_8VB];
        break;
    case 3:
        Left=Right=FontCharWidth[DISPLAY_BASS];
        break;
    case 4:
        Left=Right=FontCharWidth[DISPLAY_BASS_8VB];
        break;
    }
    register int i;
    for (i=ViewBegin; i<(int)MelodySize; ++i)
    {
        if (Melody[i])
        {
            if (Melody[i]->DisplayOnset)
            {
                Right += FontCharWidth[Melody[i]->DisplayOnset];
            }
            if (Melody[i]->DisplayAccidental)
            {
                Right += FontCharWidth[Melody[i]->DisplayAccidental];
            }
            if (Melody[i]->DisplayModDur)
            {
                Right += FontCharWidth[Melody[i]->DisplayModDur];
            }
            if (Melody[i]->DisplayNote)
            {
                Right += FontCharWidth[Melody[i]->DisplayNote];
            }
            if (Melody[i]->DisplayConclusion)
            {
                Right += FontCharWidth[Melody[i]->DisplayConclusion];
            }

            if (Pixels<(Left/2)+(Right/2))
            {
                return i;
            }
        }
        Left = Right;
    }
    return i;
}

void SPartWindowData::AutoScrollRight(void)
{
    unsigned ClefWidth = 0;
    switch (Clef)
    {
    case 0:
        ClefWidth = FontCharWidth[DISPLAY_TREBLE_8VA];
        break;
    case 1:
        ClefWidth = FontCharWidth[DISPLAY_TREBLE];
        break;
    case 2:
        ClefWidth = FontCharWidth[DISPLAY_TREBLE_8VB];
        break;
    case 3:
        ClefWidth = FontCharWidth[DISPLAY_BASS];
        break;
    case 4:
        ClefWidth = FontCharWidth[DISPLAY_BASS_8VB];
        break;
    }
    unsigned Width = 0;
    unsigned BeginHere = SelectEnd;
    if (SelectEnd==MelodySize)
    {
        Width += FontCharWidth[DISPLAY_BAR];
        Width += FontCharWidth[DISPLAY_SMALL_SPACE];
        Width += FontCharWidth[DISPLAY_THICK_BAR];
    }

    while (BeginHere)
    {
        --BeginHere;

        if (Melody[BeginHere]->DisplayOnset)
        {
            Width += FontCharWidth[Melody[BeginHere]->DisplayOnset];
        }
        if (Melody[BeginHere]->DisplayAccidental)
        {
            Width += FontCharWidth[Melody[BeginHere]->DisplayAccidental];
        }
        if (Melody[BeginHere]->DisplayModDur)
        {
            Width += FontCharWidth[Melody[BeginHere]->DisplayModDur];
        }
        if (Melody[BeginHere]->DisplayNote)
        {
            Width += FontCharWidth[Melody[BeginHere]->DisplayNote];
        }
        if (Melody[BeginHere]->DisplayConclusion)
        {
            Width += FontCharWidth[Melody[BeginHere]->DisplayConclusion];
        }

        if (Width+ClefWidth>ViewWidthPixels)
        {
            ++BeginHere;
            break;
        }
    }

    if (ViewBegin<BeginHere)
    {
        ViewBegin = BeginHere;
    }
    InvalidateRgn(hMyself,NULL,0);
}

static char * NoteNum2Name(UBYTE byMIDINumber, char * sMusique, size_t len)
{
    UBYTE bySamaNumber;
    short nOctave;
    short nInterval;
    char sNoteNames[12][3] =
    {"C=", "C#", "D=", "E@", "E=", "F=", "F#", "G=", "G#", "A=", "B@", "B="};
    char pASCII[2] = {'\0','\0'};

    // limit range of input MIDI numbers
    byMIDINumber = (BYTE)((byMIDINumber < 12) ? 12 : byMIDINumber);
    // calculate interval and octave
    bySamaNumber = (BYTE)(byMIDINumber - 12); // shifted slightly from MIDI Numbers
    nOctave = (short)(bySamaNumber / 12);
    nInterval = (short)((int)bySamaNumber - nOctave * 12);
    // construct note name
    strcpy_s(sMusique, len, &sNoteNames[nInterval][0]);
    _ltoa_s(nOctave, pASCII, _countof(pASCII), 10);
    strcat_s(sMusique, len, pASCII);

    return sMusique;
}

static void OutputVarLenNum(ofstream & os, const unsigned long value)
{
    if (value<=0x7F)   // simple, one byte representation
    {
        os.put((unsigned char)value);
        return;
    }
    unsigned char OutVals[4];
    OutVals[3] = (unsigned char)(value & 0x7F);
    OutVals[2] = (unsigned char)((value >>  7) & 0x7F);
    OutVals[1] = (unsigned char)((value >> 14) & 0x7F);
    OutVals[0] = (unsigned char)((value >> 21) & 0x7F);
    // if value > 0x0FFFFFFF it's truncated to 0x0FFFFFFF

    register int i;
    for (i=0; i<3; ++i)
        if (OutVals[i])
        {
            break;
        }
    while (i<3)
    {
        os.put((unsigned char)(OutVals[i++] | 0x80));
    }

    os.put(OutVals[3]);
}

unsigned long InputVarLenNum(ifstream & is)
{
    register unsigned long value;
    register unsigned char c;

#ifdef _DEBUG
    long nTest = is.tellg();
#endif

    c = (BYTE)(is.get());
#ifdef _DEBUG
    nTest = is.tellg();
#endif
    value = c & 0x7F;
    while (c & 0x80)
    {
        value <<= 7;
        is >> c;
        value += (c & 0x7F);
    }

    return value;
}


static DWORD RevByteOrder(const DWORD dwOriginal)
{
    UINT nLength = sizeof(dwOriginal);
    UINT nMaskShift;
    DWORD dwCurrByte;
    DWORD dwReversed = 0;
    DWORD dwMask = 0xFF;
    for (UINT i = 0; i < nLength; i++)
    {
        nMaskShift = i * sizeof(BYTE) * 8; // calculate shift for mask
        dwCurrByte = (dwOriginal & (dwMask << nMaskShift)) >> nMaskShift; // get current byte
        dwCurrByte <<= (nLength - i - 1) * 8; // shift to reversed position
        dwReversed += dwCurrByte; // add to result
    }

    return dwReversed;
}

static WORD RevByteOrder(const WORD wOriginal)
{
    UINT nLength = sizeof(wOriginal);
    UINT nMaskShift;
    WORD wCurrByte;
    WORD wReversed = 0;
    WORD wMask = 0xFF;
    for (UINT i = 0; i < nLength; i++)
    {
        nMaskShift = i * sizeof(BYTE) * 8; // calculate shift for mask
        wCurrByte = (unsigned short)((wOriginal & (wMask << nMaskShift)) >> nMaskShift); // get current byte
        wCurrByte <<= (nLength - i - 1) * 8; // shift to reversed position
        wReversed = (unsigned short)(wReversed + wCurrByte); // add to result
    }

    return wReversed;
}

DWORD SPartWindowData::Ticks2Dur(DWORD dwNoteTicks, DWORD dwTolerance, char * sMusique, size_t len)
{
    double fNoteStdDur[16] = {1/8.0, 1/6.0, 0.25, 1/3.0, 3/8.0, 0.5, 2/3.0, 0.75, 1.0, 4/3.0, 1.5,  2.0, 8/3.0, 3.0,  4.0,  6.0};
    char  sNoteDur[16][3] = {"zn",  "st",  "sn", "it",  "s.",  "in", "qt", "i.", "qn", "ht", "q.", "hn", "wt", "h.", "wn", "w."};
    DWORD dwResult;

    float fNoteQtrNotes = (float)dwNoteTicks / (float)m_stMThdChunk->Division;
    BOOL  bRest = (sMusique[Name] == 'R');

    USHORT  nMinIndex,  // lowest valid index at current step in search
            nMaxIndex,  // highest valid index
            nMidIndex,  // watershed index (in the middle of valid indexes)
            nNoteIndex; // final index decision

    // binary search for note duration
    nMinIndex = 0;
    nMaxIndex = 15;
    if (fNoteQtrNotes > fNoteStdDur[15])
    {
        nNoteIndex = 14;
    }
    else
    {
        while (nMinIndex != nMaxIndex)
        {
            nMidIndex = (unsigned short)((nMaxIndex + nMinIndex) / 2);
            if (fNoteQtrNotes <= fNoteStdDur[nMidIndex])
            {
                nMaxIndex = nMidIndex;
            }
            else
            {
                nMinIndex = (unsigned short)(nMidIndex + 1);
            }
        }
        nNoteIndex = nMinIndex;
        // if rest, may need to shift index by one
        if (bRest)
        {
            if (fNoteQtrNotes < fNoteStdDur[nNoteIndex])
            {
                nNoteIndex--;
            }
        }
    }
    // calc residue for return value
    DWORD dwNoteTicksNominal = (DWORD)(fNoteStdDur[nNoteIndex] * (float)m_stMThdChunk->Division),
          dwNoteTicksMax;
    if (!bRest)
    {
        dwNoteTicksMax = dwNoteTicksNominal;
    }
    else
    {
        dwNoteTicksMax = (DWORD)(fNoteStdDur[nNoteIndex] * (float)m_stMThdChunk->Division) + dwTolerance - 1;
    }
    if (dwNoteTicks <= dwNoteTicksMax)
    {
        dwResult = 0;
    }
    else
    {
        dwResult = dwNoteTicks - dwNoteTicksNominal;
    }
    // compose duration string
    strcat_s(sMusique, len, sNoteDur[nNoteIndex]);
    if ((dwResult) && (!bRest))
    {
        strcat_s(sMusique, len, "_");
    }
    else
    {
        strcat_s(sMusique, len, " ");
    }

    return dwResult;
}

BOOL SPartWindowData::MusicXMLIn(LPCSTR filename)
{
    CoInitialize(NULL);
    ISaAudioDocumentReaderPtr saAudioDocRdr;
    HRESULT createResult = saAudioDocRdr.CreateInstance(__uuidof(SaAudioDocumentReader));
    if (createResult)
    {
        return FALSE;
    }

    _bstr_t szMusique = saAudioDocRdr->ImportMusicXML(filename);
    ReplaceString(szMusique,0,MelodySize);

    saAudioDocRdr->Close();
    saAudioDocRdr->Release();
    saAudioDocRdr = NULL;
    CoUninitialize();

    return true;
}

BOOL SPartWindowData::MusicXMLOut(LPCSTR filename)
{
    CoInitialize(NULL);
    ISaAudioDocumentWriterPtr saAudioDocWriter;
    HRESULT createResult = saAudioDocWriter.CreateInstance(__uuidof(SaAudioDocumentWriter));
    if (createResult)
    {
        return FALSE;
    }

    _bstr_t szMusique = GetString(0,MelodySize);
    saAudioDocWriter->ExportMusicXML(szMusique, filename);
    ReplaceString(szMusique,0,MelodySize);

    // uninitialize com object
    saAudioDocWriter->Close();
    saAudioDocWriter->Release();
    saAudioDocWriter = NULL;
    CoUninitialize();

    return true;
}

BOOL SPartWindowData::MIDIIn(LPCSTR filename)
{
    if (!filename || !*filename)
    {
        return FALSE;
    }

    ifstream infile(filename,ios::binary);
    if (!infile)
    {
        return FALSE;
    }

    // delete contents of staff control
    if (MelodySize)
    {
        ReplaceString("", 0, MelodySize);
    }

    // look for header marker "MThd"
    // read in headers

    infile.read((char *)m_stMThdChunk, sizeof(*m_stMThdChunk));
    infile.read((char *)m_stMTrkHeader, sizeof(*m_stMTrkHeader));

    // check file status and chunk ID
    LPSTR lpstrChunkName = new char[5];
    for (short i = 0; i < 4; i++)
    {
        lpstrChunkName[i] = m_stMThdChunk->Header.Id.Name[i];
    }
    lpstrChunkName[4] = 0;
    if (!infile.good() || strcmp(lpstrChunkName, "MThd"))
    {
        delete infile;
        return FALSE;
    }

    // reverse byte order
    m_stMThdChunk->Header.Size = RevByteOrder(m_stMThdChunk->Header.Size);
    m_stMThdChunk->Format = RevByteOrder(m_stMThdChunk->Format);
    m_stMThdChunk->NTrks = RevByteOrder(m_stMThdChunk->NTrks);
    m_stMThdChunk->Division = RevByteOrder(m_stMThdChunk->Division);

    m_stMTrkHeader->Size = RevByteOrder(m_stMTrkHeader->Size);

    // !!
    // read the body
    // DWORD dwFileBytesLeft = m_stMTrkHeader->Size;
    // UINT nBatchSize = (UINT)dwFileBytesLeft;
    // UINT nBatchIndex;
    char pASCII[33] = "";

    DWORD dwDeltaTime = 0;
    UBYTE byEventCommand = 0;
    UBYTE byEventCommandOld = 0;
    UBYTE byEventTrack = 0;
    UBYTE byEventTrackFirst = 0;
    UBYTE byEventType = 0;
    DWORD dwEventLen = 0;
    BOOL  bFirstNote = FALSE;
    BOOL  bNoteOn = FALSE;
    BOOL  bMusiqueFile = FALSE;
    UBYTE byNoteNum = 0;
    UBYTE byNoteVel = 0;
    DWORD dwNoteDeltaTime = 0;
    DWORD dwNoteTolerance = m_stMThdChunk->Division / 6;
    DWORD dwNoteResidue = 0;
    char sMusique[12] = "";
    //  short nTempo = 120;

    UBYTE bySamaFlag = FALSE;
    byEventCommand = 0;

    // !!
    //while(nBytesRead = infile.read(lpszBuffer, nBatchSize))
    //{ // nBatchSize = ((DWORD)nBufSize < dwFileBytesLeft) ? nBufSize : (UINT)dwFileBytesLeft;
    // nBatchIndex = 0;
    // dwFileBytesLeft -= nBytesRead;

#ifdef _DEBUG
    long nTest = infile.tellg();
#endif
    // translate MIDI messages
    while (infile.good())
    {
        // get delta time
#ifdef _DEBUG
        nTest = infile.tellg();
#endif
        dwDeltaTime = InputVarLenNum(infile);
        //      if (bNoteOn)
        dwNoteDeltaTime += dwDeltaTime;
#ifdef _DEBUG
        nTest = infile.tellg();
#endif
        // get command
        byEventCommandOld = byEventCommand;
        infile >> byEventCommand; //infile.read((char*)&byEventCommand, sizeof(byEventCommand));
#ifdef _DEBUG
        nTest = infile.tellg();
#endif
        if (byEventCommand < 0x80)   // running status
        {
            byEventCommand = byEventCommandOld;
            infile.seekg(-1L, ios::cur);
        }
        // get data
        switch (byEventCommand >> 4)
        {
            // MIDI events
        case 0x8: // Note Off
        case 0x9: // Note On
            byEventTrack = (BYTE)((byEventCommand & 0x0F));
            if (!bFirstNote)
            {
                byEventTrackFirst = byEventTrack;
            }
            infile.read((char *)&byNoteNum, sizeof(byNoteNum));
            infile.read((char *)&byNoteVel, sizeof(byNoteVel));
            if ((byEventTrack == byEventTrackFirst) /*&& !bySamaFlag*/)
            {
                if ((byEventCommand == 0x90) && (byNoteVel))   // Note On message
                {
                    if (dwNoteDeltaTime > dwNoteTolerance)   // there is a rest here
                    {
                        do
                        {
                            strcpy_s(sMusique, _countof(sMusique), "R=3*");
                            dwNoteResidue = Ticks2Dur(dwNoteDeltaTime, dwNoteTolerance, sMusique, _countof(sMusique));
                            dwNoteDeltaTime = dwNoteResidue;
                            if (sMusique[Duration] == 'z')
                            {
                                sMusique[Duration] = 's';
                            }
                            ReplaceString(sMusique,MelodySize,MelodySize);
                        }
                        while (dwNoteResidue);
                    }
                    dwNoteDeltaTime = 0;
                    bNoteOn = TRUE;
                } // if
                else   // Note Off message
                {
                    if (bNoteOn && dwNoteDeltaTime)
                    {
                        if (!bySamaFlag)
                            do
                            {
                                *sMusique = '\0';
                                NoteNum2Name(byNoteNum, sMusique, _countof(sMusique)); // name, accidental, octave
                                strcat_s(sMusique, _countof(sMusique), "*"); // no onset glide
                                dwNoteResidue = Ticks2Dur(dwNoteDeltaTime, dwNoteTolerance, sMusique, _countof(sMusique));
                                dwNoteDeltaTime = dwNoteResidue;
                                ReplaceString(sMusique,MelodySize,MelodySize);
                            }
                            while (dwNoteResidue);
                        else
                        {
                            dwNoteDeltaTime = 0;
                        }
                    }
                    bNoteOn = FALSE;
                }
            } // if
            bySamaFlag = FALSE;
            break;

        case 0xA: // Poly Key Pressure
        case 0xB: // Control Change
            infile.ignore(2); // skip over 2 data bytes
            break;

        case 0xC: // Program Change
        case 0xD: // Channel Pressure
            infile.ignore(1); // skip over 1 data byte
            break;

        case 0xE: // Pitch Bend
            infile.ignore(2); // skip over 2 data bytes
            break;

        case 0xF: // sysex/meta events
            switch (byEventCommand)
            {
            case 0xF0: // sysex event
            case 0xF7: // sysex event
                dwEventLen = InputVarLenNum(infile);
                //!!for (; lpszBuffer[nBatchIndex] & 0x80; nBatchIndex++); // update Batch Index
                //!!nBatchIndex++;
                // for now...
                infile.ignore((int)dwEventLen);
                break;

            case 0xFF: // meta event
#ifdef _DEBUG
                nTest = infile.tellg();
#endif
                infile.read((char *)&byEventType, sizeof(byEventType));
#ifdef _DEBUG
                nTest = infile.tellg();
#endif
                switch (byEventType)
                {
                case 0x2F: // end of track
                    if (MelodySize)   //!!
                    {
                        if (!bMusiqueFile)
                        {
                            SetClef(WhichClef());
                        }
                        InvalidateRgn(hMyself,NULL,0);
                        return 0;
                    }
                    else     // empty track. try next one
                    {
                        infile.ignore(1);
                        infile.read((char *)m_stMTrkHeader, sizeof(SChunkHeader));
                        m_stMTrkHeader->Size = RevByteOrder(m_stMTrkHeader->Size);
                        //!!nBytesRead = pMIDIfile->Read(lpszBuffer, nBatchSize);
                        //!!nBatchIndex = 0;
                    }
                    break;

                case 0x7F: // sequencer-specific
                    dwEventLen = InputVarLenNum(infile);
#ifdef _DEBUG
                    nTest = infile.tellg();
#endif
                    //!!for (; lpszBuffer[nBatchIndex] & 0x80; nBatchIndex++); // update Batch Index
                    //!!nBatchIndex++;
                    infile.read(pASCII, 4);
#ifdef _DEBUG
                    nTest = infile.tellg();
#endif
                    pASCII[4] = NULL;
                    if (strcmp(pASCII, "SAMA") == 0)
                    {
                        if (dwNoteDeltaTime > dwNoteTolerance && !bNoteOn)   // there is a rest here
                        {
                            do
                            {
                                strcpy_s(sMusique, _countof(sMusique), "R=3*");
                                dwNoteResidue = Ticks2Dur(dwNoteDeltaTime, dwNoteTolerance, sMusique, _countof(sMusique));
                                dwNoteDeltaTime = dwNoteResidue;
                                if (sMusique[Duration] == 'z')
                                {
                                    sMusique[Duration] = 's';
                                }
                                ReplaceString(sMusique,MelodySize,MelodySize);
                            }
                            while (dwNoteResidue);
                            dwNoteDeltaTime = 0;
                        }

                        bMusiqueFile=TRUE;
                        infile.read(sMusique, 7);
                        sMusique[7] = NULL;
                        ReplaceString(sMusique,MelodySize,MelodySize);
                        if (sMusique[0] != '(')
                        {
                            bySamaFlag = TRUE;    // set the SAMA flag
                        }
                    }
                    else
                    {
                        infile.ignore((int)dwEventLen - 4);
                    }
                    break;

                default:
                    dwEventLen = InputVarLenNum(infile);
                    infile.ignore((int)dwEventLen);
                    break;
                } // switch (byEventType)
                break;

            default:
                break;
            } // switch (byEventCommand)
            break;

        default:
            break;
        } // switch (byEventCommand >> 4)
    } // while

    if (!bMusiqueFile)
    {
        SetClef(WhichClef());
    }
    InvalidateRgn(hMyself,NULL,0);
    return(0);

}

BOOL SPartWindowData::MIDIOut(LPCSTR filename)
{
    if (!filename || !*filename)
    {
        return FALSE;
    }

    ofstream outfile(filename,ios::out|ios::binary);
    if (outfile)
    {
        int Start=SelectMin(), End=SelectMax();
        if (SelectBegin==SelectEnd)
        {
            Start=0,End=MelodySize;
        }

        outfile.write(
            "MThd" // MIDI file
            "\0\0\0\x06" // six bytes in header
            "\0\0" // format type 0
            "\0\x01" // one MTrk chunk in file
            ,12);
        // How many ticks per quarter note?
        unsigned char c = TICKS_PER_Q>>8;
        outfile.put(c);
        c = TICKS_PER_Q & 0x00FF;
        outfile.put(c);

        // begin MIDI track
        outfile.write("MTrk"
                      "\xAA\xBB\xCC\xDD",8); // unknown size yet -- we'll set this later

        // copyright meta-event
        outfile.write("\0\xFF\x02\x24" "Copyright (c) 2000 SIL International",40);

        // select channel 1
        outfile.write("\0\xFF\x20\x01\0",5);

        // set pitchwheel sensitivity
        outfile.write("\0\xB0\x64\0\0\xB0\x65\0\0\xB0\x06\x04\0\xB0\x26\0",16);

        // specify clef (in a "SAMA" chunk)
        outfile.write("\0\xFF\x7F\x0B" "SAMA" "(CLEF",13);
        outfile.put((unsigned char)('0' + Clef));
        outfile.put(')');

        // set instrument
        outfile.put('\0');
        outfile.put('\xC0');
        outfile.put(Instrument);

        // specify instrument (in a "SAMA" chunk)
        outfile.write("\0\xFF\x7F\x0B" "SAMA" "(VX",11);
        outfile.put((unsigned char)('0' + (Instrument/100)));
        outfile.put((unsigned char)('0' + ((Instrument%100)/10)));
        outfile.put((unsigned char)('0' + (Instrument%10)));
        outfile.put(')');

        // set tempo
        outfile.write("\0\xFF\x51\x03",4);
        unsigned long MicrosecondsPerQuarter = 60000000L/Tempo;
        outfile.put((unsigned char)((MicrosecondsPerQuarter&0xFF0000)>>16));
        outfile.put((unsigned char)((MicrosecondsPerQuarter&0xFF00)>>8));
        outfile.put((unsigned char)(MicrosecondsPerQuarter&0xFF));

        // specify tempo (in a "SAMA" chunk)
        outfile.write("\0\xFF\x7F\x0B" "SAMA" "(Q=",11);
        outfile.put((unsigned char)('0' + (Tempo/100)));
        outfile.put((unsigned char)('0' + ((Tempo%100)/10)));
        outfile.put((unsigned char)('0' + (Tempo%10)));
        outfile.put(')');

        // output each note
        outfile.put('\0'); // timing for first event


        BOOL bTiedNote = FALSE;
        for (register int i=Start; i<End; ++i)
        {
            // in some cases we output a SAMA chunk to help re-input parser
            if (!strchr("=#@",Melody[i]->Accidental)
                    || Melody[i]->Onset != '*'
                    || Melody[i]->Conclusion != ' '
                    || bTiedNote)
            {
                // "SAMA" chunk (proprietary meta-event)
                outfile.write("\xFF\x7F\x0B" "SAMA",7);
                char * aString = GetString(i, i+1); // translate to (xxxxx) format
                outfile.write(aString, 7);
                outfile.put('\0'); // timing for next event
                delete aString;

            }
            // NOTE itself
            if ((Melody[i]->Name>='A'&&Melody[i]->Name<='G')||Melody[i]->Name=='R')
            {
                // NOTE
                unsigned char cNote = (BYTE)(Melody[i]->MIDINote());

                if (!cNote || bTiedNote)   // rest or tied note -- no "note on" needed
                {
                    // here, instead of "Note On", I'm just outputting a text event
                    // that consists of a single space character.  I need to have some
                    // type of event here so we can make time pass.
                    outfile.write("\xFF\x01\x01 ",4);

                    bTiedNote=FALSE;
                }
                else
                {
                    //Pitch bend for halftones
                    switch (Melody[i]->Accidental)
                    {
                    case '=':
                        break;
                    case '+': // need to make it a little less sharp
                    case '!': // need to make it a little flatter
                        //outfile.write("\xE0\x38\0",3);
                        outfile.write("\xE0\0\x38",3);
                        outfile.put('\0'); // timing for next event
                        break;
                    case '-': // need to make it a little less flat
                    case '$': // need to make it a little more sharp
                        //outfile.write("\xE0\x48\0",3);
                        outfile.write("\xE0\0\x48",3);
                        outfile.put('\0'); // timing for next event
                        break;
                    }

                    outfile.put('\x90'); // note on, channel 1
                    outfile.put(cNote); // note to output
                    outfile.put('\x70'); // medium volume
                }

                unsigned long l = Melody[i]->MIDIDuration();

                if (Melody[i]->Conclusion == ' ' && Melody[i]->Name != 'R')
                {
                    // normal, legato note
                    OutputVarLenNum(outfile,l - 5); // duration - 5 = legato

                    outfile.put('\x80'); // note off, channel 1
                    outfile.put(cNote);
                    outfile.put('\0');
                    outfile.put('\x05'); // silence of duration 5
                }
                else     // rest or slurred/tied note
                {
                    OutputVarLenNum(outfile,l);
                    if (Melody[i]->Name != 'R')
                    {
                        BOOL bTieIt = FALSE;
                        if (Melody[i]->Conclusion == '_')
                        {
                            // look to see if next note is same tone
                            for (register int j=1; i+j<End; ++j)
                            {
                                if (Melody[i]->Value == Melody[i+j]->Value)
                                {
                                    bTieIt = TRUE;
                                    break;
                                }
                                else if (Melody[i+j]->Value!=-2)
                                {
                                    break;    // anything else (except annotations), don't tie!
                                }
                            }
                        }
                        if (bTieIt)
                        {
                            bTiedNote = TRUE;
                        }
                        else
                        {
                            outfile.put('\x80'); // note off, channel 1
                            outfile.put(cNote);
                            outfile.put('\0');
                            outfile.put('\0'); // time for next event
                        }
                    }
                }

                if (Melody[i]->Accidental!='=' && !bTiedNote)   // reset pitch bend
                {
                    outfile.write("\xE0\0\x40",3);
                    outfile.put('\0'); // timing for next event
                }
            }
        }

        // end of track
        outfile.write("\xFF\x2F\0",3);
        outfile.flush();

        // write file size at position 18
        long l = static_cast<long>(outfile.tellp()) - 22;
        outfile.seekp(18);
        outfile.put((unsigned char)((l&0xFF000000L)/16777216L));
        outfile.put((unsigned char)((l&0x00FF0000L)/65536L));
        outfile.put((unsigned char)((l&0x0000FF00L)/256L));
        outfile.put((unsigned char)((l&0x000000FFL)));

        outfile.close();
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

BOOL SPartWindowData::RTFOut(LPCSTR filename)
{
    if (!filename || !*filename)
    {
        return FALSE;
    }

    ofstream outfile(filename);
    if (outfile)
    {

        outfile.write(RTFprolog,strlen(RTFprolog));

        char aString[34] = "0";
        switch (Clef)
        {
        case 1:
            _ltoa_s(DISPLAY_TREBLE,aString,_countof(aString),10);
            break;
        case 2:
            _ltoa_s(DISPLAY_TREBLE_8VB,aString,_countof(aString),10);
            break;
        case 3:
            _ltoa_s(DISPLAY_BASS,aString,_countof(aString),10);
            break;
        case 0:
            _ltoa_s(DISPLAY_TREBLE_8VA,aString,_countof(aString),10);
            break;
        case 4:
            _ltoa_s(DISPLAY_BASS_8VB,aString,_countof(aString),10);
            break;
        }
        outfile.write("\\uc0",4);
        outfile.put('\\');
        outfile.put('u');
        outfile.write(aString,strlen(aString));
        WCHAR * wString = PartEncode(0,MelodySize);
        if (wString)
        {
            for (register int i=0; wString[i]; ++i)
            {
                _ltoa_s(wString[i],aString,_countof(aString),10);
                outfile.put('\\');
                outfile.put('u');
                outfile.write(aString,strlen(aString));
            }
            delete wString;
            _ltoa_s(DISPLAY_BAR,aString,_countof(aString),10);
            outfile.put('\\');
            outfile.put('u');
            outfile.write(aString,strlen(aString));
            _ltoa_s(DISPLAY_SMALL_SPACE,aString,_countof(aString),10);
            outfile.put('\\');
            outfile.put('u');
            outfile.write(aString,strlen(aString));
            _ltoa_s(DISPLAY_THICK_BAR,aString,_countof(aString),10);
            outfile.put('\\');
            outfile.put('u');
            outfile.write(aString,strlen(aString));
        }

        outfile.write(RTFepilog,strlen(RTFepilog));

        outfile.close();
        return TRUE;
    }
    return FALSE;
}

BOOL SPartWindowData::TXTIn(LPCSTR filename)
{
    if (!filename || !*filename)
    {
        return FALSE;
    }

    ifstream infile(filename);
    if (!infile)
    {
        return FALSE;
    }

    if (infile.good())
    {
        infile.seekg(0,ios::end);
        streampos FileSize = infile.tellg();
        infile.seekg(0,ios::beg);

        char * Buffer = new char[FileSize]+1;
        if (!Buffer)
        {
            *Buffer = '\0';
        }
        else
        {
            infile.get(Buffer,(int)FileSize+1,'\0');
        }
        infile.close();

        ReplaceString(Buffer,0,MelodySize);
        if (strncmp(Buffer,"(CLEF",5))
        {
            SetClef(WhichClef());
        }
    }

    infile.close();
    return TRUE;

}

BOOL SPartWindowData::TXTOut(LPCSTR filename)
{
    if (!filename || !*filename)
    {
        return FALSE;
    }

    ofstream outfile(filename);
    if (outfile)
    {
        char * pe = GetString(0,MelodySize);
        if (!pe)
        {
            outfile.write("", 0);
        }
        else
        {
            outfile.write(pe,strlen(pe));
        }
        delete pe;

        outfile.close();
        return TRUE;
    }
    return FALSE;
}


BOOL SPartWindowData::NotesUp(HWND hWnd)
{
    if (SelectBegin!=SelectEnd)   // something is selected
    {
        BOOL ClefChangeNeeded = FALSE;
        for (register unsigned i= SelectMin(); i < SelectMax(); ++i)
        {
            if (Control)
            {
                (Melody[i])->UpOctave();
            }
            else
            {
                (Melody[i])->UpStep();
            }
            (Melody[i])->CalcValue(Clef);
            if (!(Melody[i])->InClef(Clef))
            {
                ClefChangeNeeded = TRUE;
            }
        }
        if (ClefChangeNeeded)
        {
            int OldClef = Clef;
            int NewClef = WhichClef();
            if (NewClef != OldClef)
            {
                SetClef(NewClef);
            }
        }
        InvalidateRgn(hWnd,NULL,FALSE);
        Modified = TRUE;
        SendMessage(hMyParent,WM_PARENTNOTIFY,EM_SETMODIFY,1);
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

BOOL SPartWindowData::NotesDown(HWND hWnd)
{
    if (SelectBegin!=SelectEnd)   // something is selected
    {
        BOOL ClefChangeNeeded = FALSE;
        for (register unsigned i= SelectMin(); i < SelectMax(); ++i)
        {
            if (Control)
            {
                (Melody[i])->DownOctave();
            }
            else
            {
                (Melody[i])->DownStep();
            }
            (Melody[i])->CalcValue(Clef);
            if (!(Melody[i])->InClef(Clef))
            {
                ClefChangeNeeded = TRUE;
            }
        }
        if (ClefChangeNeeded)
        {
            int OldClef = Clef;
            int NewClef = WhichClef();
            if (NewClef != OldClef)
            {
                SetClef(NewClef);
            }
        }
        InvalidateRgn(hWnd,NULL,FALSE);
        Modified = TRUE;
        SendMessage(hMyParent,WM_PARENTNOTIFY,EM_SETMODIFY,1);
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

BOOL SPartWindowData::GoHome(HWND hWnd)
{
    ViewBegin = SelectEnd = 0;
    if (!Selecting)
    {
        SelectBegin = 0;
    }
    InvalidateRgn(hWnd,NULL,FALSE);
    return TRUE;
}

struct SPartClassData
{
    BOOL InsertMode;
    HFONT hButtonFont;
    SPartClassData() : InsertMode(TRUE)
    {
        hButtonFont = CreateFont(-24,0,0,0,0,0,0,0,
                                 ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,
                                 DEFAULT_QUALITY,FF_DONTCARE|DEFAULT_PITCH,"Musique");
    };
    ~SPartClassData()
    {
        DeleteObject(hButtonFont);
        InsertMode = 0;
        hButtonFont = 0;
    }
};

static DWORD PlayMidiFile(char * lpszMIDIFileName, HWND hWndNotify);
static DWORD PauseMidiFile(HWND hWndNotify);
static DWORD StopMidiFile(HWND hWndNotify);
static DWORD LoopMidiFile(char * lpszMIDIFileName, HWND hWndNotify);

// ========================================
//      M A I N   P R O C E D U R E
// ========================================

LRESULT CALLBACK PartitureProc(HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
#ifdef SetFocusPrecaution
    if (uMsg != WM_KILLFOCUS)
    {
        SetFocus(hWnd);
    }
#endif
    switch (uMsg)
    {
    case WM_PAINT:
    {
        struct SPartWindowData * pwd = (struct SPartWindowData *)GetWindowLong(hWnd,GWL_USERDATA);
        if (pwd)
        {
            pwd->Paint();

            double dHere = 0;
            unsigned i;
            unsigned begin = pwd->SelectBegin < pwd->SelectEnd ? pwd->SelectBegin : pwd->SelectEnd;
            unsigned end   = pwd->SelectBegin > pwd->SelectEnd ? pwd->SelectBegin : pwd->SelectEnd;

            for (i=0; i < begin; i++)
            {
                dHere +=  pwd->Melody[i]->MIDIDuration();
            }
            pwd->m_sSelectionMS.begin = (dHere - TICKS_PER_Q / 64.) * 60. / (pwd->Tempo * TICKS_PER_Q);
            if (pwd->m_sSelectionMS.begin < 0)
            {
                pwd->m_sSelectionMS.begin = 0;
            }

            for (; i < end; i++)
            {
                dHere +=  pwd->Melody[i]->MIDIDuration();
            }
            pwd->m_sSelectionMS.end = (dHere + TICKS_PER_Q / 64.) * 60. / (pwd->Tempo * TICKS_PER_Q);

            SendMessage(pwd->hMyParent,WM_PARENTNOTIFY,WM_PAINT, (LPARAM) &pwd->m_sSelectionMS);
        }
        break;
    }
    case WM_KEYDOWN:
    {
        struct SPartWindowData * pwd;
        pwd = (struct SPartWindowData *) GetWindowLong(hWnd,GWL_USERDATA);
        struct SPartClassData * pcd;
        pcd = (struct SPartClassData *) GetClassLong(hWnd,0);

        switch (wParam)
        {
        case VK_SHIFT:
            pwd->Selecting = TRUE;
            break;
        case VK_CONTROL:
            pwd->Control = TRUE;
            break;
        }

        if (pwd && pwd->m_sPlay.Tempo)
        {
            MessageBeep(MB_OK);
            return 0;         // Do not allow editing if playing
        }

        switch (wParam)
        {
        case VK_BACK:
            if (pwd->SelectBegin!=pwd->SelectEnd)
            {
                SendMessage(hWnd,WM_CLEAR,0,0);
            }
            else if (pwd->SelectBegin)
            {
                if (pwd->SelectBegin<=pwd->ViewBegin)
                {
                    pwd->ViewBegin = --pwd->SelectBegin;
                }
                else
                {
                    --pwd->SelectBegin;
                }
                SendMessage(hWnd,WM_CLEAR,0,0);
            }
            break;

        case VK_DELETE:
            if (pwd->Selecting)
            {
                SendMessage(hWnd,WM_CUT,0,0);
            }
            else
            {
                SendMessage(hWnd,WM_CLEAR,0,0);
            }
            break;

        case VK_HOME:
            pwd->GoHome(hWnd);
            break;
        case VK_END:
            pwd->SelectEnd = pwd->MelodySize;
            if (!pwd->Selecting)
            {
                pwd->SelectBegin = pwd->MelodySize;
            }
            pwd->AutoScrollRight();
            InvalidateRgn(hWnd,NULL,FALSE);
            break;
        case VK_INSERT:
            if (!pwd->Control)
            {
                pcd->InsertMode = !pcd->InsertMode;
            }
            break;
        case VK_LEFT:
            if (pwd->Control)
            {
                if (pwd->ViewBegin>0)
                {
                    pwd->ViewBegin--;
                    InvalidateRgn(hWnd,NULL,FALSE);
                }
            }
            else
            {
                if (pwd->SelectEnd &&
                        (pwd->Selecting || pwd->SelectBegin==pwd->SelectEnd))
                {
                    --pwd->SelectEnd;
                }
                if (!pwd->Selecting)
                {
                    pwd->SelectBegin = pwd->SelectEnd;
                }

                if (pwd->ViewBegin > pwd->SelectEnd)
                {
                    pwd->ViewBegin = pwd->SelectEnd;
                }
                InvalidateRgn(hWnd,NULL,FALSE);
            }
            break;
        case VK_RIGHT:
            if (pwd->Control)
            {
                if ((pwd->ViewBegin+1)<pwd->MelodySize)
                {
                    pwd->ViewBegin++;
                    InvalidateRgn(hWnd,NULL,FALSE);
                }
            }
            else
            {
                if (pwd->SelectEnd < pwd->MelodySize
                        && (pwd->Selecting || pwd->SelectBegin==pwd->SelectEnd))
                {
                    ++pwd->SelectEnd;
                }
                if (!pwd->Selecting)
                {
                    pwd->SelectBegin=pwd->SelectEnd;
                }
                pwd->AutoScrollRight();
                InvalidateRgn(hWnd,NULL,FALSE);
            }
            break;
        case VK_UP:
            pwd->NotesUp(hWnd);
            return 0;
            break;
        case VK_DOWN:
            pwd->NotesDown(hWnd);
            return 0;
            break;
        case 'C': // somehow these are not getting translated into WM_CHAR messages in SA
        case 'W':
        case 'M':
        case 'L':
        case 'V':
        case 'X':
            if (pwd->Control)
            {
                SendMessage(hWnd,WM_CHAR,wParam-'@',lParam);
            }
            else
            {
                SendMessage(hWnd,WM_CHAR,wParam,lParam);
            }
            break;
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
            if (pwd->Control)
            {
                pwd->SetClef(wParam-'1');
            }
            break;
        case '0':
            if (pwd->Control)
            {
                pwd->SetClef(pwd->WhichClef());
            }
            break;
        }
        break;
    }

    case WM_KEYUP:
    {
        struct SPartWindowData * pwd;
        pwd = (struct SPartWindowData *) GetWindowLong(hWnd, GWL_USERDATA);

        switch (wParam)
        {
        case VK_SHIFT:
            pwd->Selecting = FALSE;
            break;
        case VK_CONTROL:
            pwd->Control = FALSE;
            break;
        }
        break;
    }

    case WM_CHAR:
    {
        struct SPartWindowData * pwd;
        pwd = (struct SPartWindowData *) GetWindowLong(hWnd,GWL_USERDATA);

        if (pwd && pwd->m_sPlay.Tempo)
        {
            MessageBeep(MB_OK);
            return 0;         // Do not allow editing if playing
        }

        switch (wParam)
        {
        case 1: // Ctrl-A = select all
            pwd->SelectBegin=0;
            pwd->SelectEnd=pwd->MelodySize;
            InvalidateRgn(hWnd,NULL,FALSE);
            return 0;
        case 3: // Ctrl-C = copy              MOVED TO WM_KEYUP
            SendMessage(hWnd,WM_COPY,0,0);
            return 0;
        case 4:   // Ctrl-D = view
        {
            char * String;
            if (pwd->SelectBegin==pwd->SelectEnd)
            {
                String = pwd->GetString(0,pwd->MelodySize);
            }
            else
            {
                String = pwd->GetString(pwd->SelectMin(),pwd->SelectMax());
            }
            MessageBox(hWnd,String,"Musique",MB_OK);
            delete String;
            pwd->Control = FALSE;
            return 0;
        }
        case 23:   // Ctrl-W = Write to MIDI file    MOVED TO WM_KEYUP
        {
            SendMessage(hWnd,WM_APP+1,0,0);
            pwd->Control = FALSE;
            return 0;
        }
        case 5:   // Ctrl-E = Read from MIDI file
        {
            SendMessage(hWnd,WM_APP+3,0,0);
            pwd->Control = FALSE;
            return 0;
        }
        case 13:   // Ctrl-M = MIDI instrument           MOVED TO WM_KEYUP
        {
            CInstrumentMenu im;
            im.Check(pwd->Instrument);
            HMENU hInstrumentMnu = im.Menu();
            POINT p;
            GetCursorPos(&p);
            TrackPopupMenu(hInstrumentMnu,TPM_CENTERALIGN,p.x,p.y,0,hWnd,NULL);
            pwd->Control = FALSE;
            break;
        }
        case 12: // Ctrl-L = play
            SendMessage(hWnd,WM_APP+2,1,0);
            return 0;
        case 22: // Ctrl-V = paste              MOVED TO WM_KEYUP
            SendMessage(hWnd,WM_PASTE,0,0);
            return 0;
        case 24: // Ctrl-X = cut                MOVED TO WM_KEYUP
            SendMessage(hWnd,WM_CUT,0,0);
            return 0;
        case 26: // Ctrl-Z = undo
            SendMessage(hWnd,WM_UNDO,0,0);
            return 0;
        case 32: // Space = move on
            pwd->SelectBegin = pwd->SelectEnd = pwd->SelectMax();
            InvalidateRgn(hWnd,NULL,FALSE);
            return 0;
        case 'x':
        case 'X':
            wParam = 32; // pass on a space (= no conclusion)
            break;
        default:
            if (wParam<' ')
            {
                return DefWindowProc(hWnd,uMsg,wParam,lParam);
            }
            break;
        }

        // check validity of keystroke
        if (!strchr("ABCDEFGRabcdefgr0123456whqiszn.tv#@$+=-!*|~?\" \\`/\'_WHQISZNTV",wParam))
        {
            if (wParam != ANNOT_SPACE &&
                    wParam != ANNOT_PHRASE &&
                    wParam != ANNOT_PHRASE2 &&
                    wParam != ANNOT_BAR &&
                    wParam != ANNOT_THICK_BAR)
            {
                MessageBeep(MB_OK);
                break;
            }
        }

        BOOL ClefChangeNeeded = FALSE;

        if (pwd->SelectBegin==pwd->SelectEnd)   // no selection
        {
            if (!pwd->Melody[pwd->SelectBegin])   // add a note to the end
            {
                pwd->Melody[pwd->SelectBegin] = new SNote(pwd->Clef);
                ++pwd->MelodySize;
                ++pwd->SelectEnd;
            }
            else     // insert a note into staff
            {
                char NewNote[8] =  "C=4*qn ";
                switch (pwd->Clef)
                {
                case 0: // TREBLE 8VA
                    NewNote[2]='5';
                    break;
                case 1: // TREBLE
                    NewNote[2]='4';
                    break;
                case 4: // BASS 8VB
                    NewNote[2]='2';
                    break;
                }
                pwd->ReplaceString(NewNote,pwd->SelectBegin,pwd->SelectBegin);
                ++pwd->SelectEnd;
            }
            (pwd->Melody[pwd->SelectBegin])->Set((BYTE)wParam,pwd->Clef);
            if (!(pwd->Melody[pwd->SelectBegin])->InClef(pwd->Clef))
            {
                ClefChangeNeeded = TRUE;
            }
        }
        else
            for (register unsigned i=pwd->SelectMin(); i<pwd->SelectMax(); ++i)
            {
                (pwd->Melody[i])->Set((BYTE)wParam,pwd->Clef);
                if (!(pwd->Melody[i])->InClef(pwd->Clef))
                {
                    ClefChangeNeeded = TRUE;
                }
            }

        if (ClefChangeNeeded)
        {
            int OldClef = pwd->Clef;
            int NewClef = pwd->WhichClef();
            if (NewClef != OldClef)
            {
                pwd->SetClef(NewClef);
            }
        }

        // if we're going off the screen, adjust view
        pwd->AutoScrollRight();

        InvalidateRgn(hWnd,NULL,FALSE);
        pwd->Modified = TRUE;
        SendMessage(pwd->hMyParent,WM_PARENTNOTIFY,EM_SETMODIFY,1);
        return 0;
    }
    case WM_LBUTTONDOWN:
    {
        struct SPartWindowData * pwd;
        pwd = (struct SPartWindowData *) GetWindowLong(hWnd,GWL_USERDATA);

        if (pwd)
        {
            pwd->SelectEnd = pwd->Position(LOWORD(lParam));
            if (!pwd->Selecting)
            {
                pwd->SelectBegin = pwd->SelectEnd;
            }
            InvalidateRgn(hWnd,NULL,FALSE);
            SendMessage(pwd->hMyParent,WM_PARENTNOTIFY,WM_SETFOCUS,lParam);
        }

        return 0;
    }
    case WM_CONTEXTMENU:
    case WM_RBUTTONUP:
    {
        struct SPartWindowData * pwd;
        pwd = (struct SPartWindowData *) GetWindowLong(hWnd,GWL_USERDATA);
        SendMessage(pwd->hMyParent,WM_PARENTNOTIFY,WM_SETFOCUS,lParam);

        if (pwd && pwd->m_sPlay.Tempo)
        {
            MessageBeep(MB_OK);
            return 0;         // Do not allow editing if playing
        }

        if (pwd)
        {
            SNote * note = NULL;
            if (pwd->SelectMax() == pwd->SelectMin() + 1)   // a single note is selected
            {
                note = pwd->Melody[pwd->SelectMin()];
            }

            HMENU hMnu = CreatePopupMenu();
            HMENU hNameMnu = CreatePopupMenu();
            HMENU hOctaveMnu = CreatePopupMenu();
            HMENU hAccidentMnu = CreatePopupMenu();
            HMENU hDurationMnu = CreatePopupMenu();
            HMENU hGlideMnu = CreatePopupMenu();
            HMENU hSymbolMnu = CreatePopupMenu();
            //        HMENU hTempoMnu = CreatePopupMenu();
            HMENU hClefMnu = CreatePopupMenu();
            CInstrumentMenu im;
            im.Check(pwd->Instrument);
            CTempoMenu tm;
            tm.Check(pwd->Tempo);

            AppendMenu(hMnu,MF_ENABLED|MF_STRING,WM_CUT,"Cu&t\tCtrl+X");
            AppendMenu(hMnu,MF_ENABLED|MF_STRING,WM_COPY,"&Copy\tCtrl+C");
            AppendMenu(hMnu,MF_ENABLED|MF_STRING,WM_PASTE,"&Paste\tCtrl+V");
            AppendMenu(hMnu,MF_ENABLED|MF_STRING,WM_CLEAR,"Clear\tDelete");
            AppendMenu(hMnu,MF_SEPARATOR,NULL,NULL);
            AppendMenu(hMnu,MF_ENABLED|MF_STRING,WM_USER+1,"Select &all\tCtrl+A");
            AppendMenu(hMnu,MF_SEPARATOR,NULL,NULL);
            AppendMenu(hMnu,MF_ENABLED|MF_STRING,WM_USER+4,"View &data\tCtrl+D");
            AppendMenu(hMnu,MF_ENABLED|MF_STRING,WM_USER+12,"P&lay selection\tCtrl+L");
            AppendMenu(hMnu,MF_ENABLED|MF_STRING,WM_USER+23,"E&xport to file...\tCtrl+W");
            AppendMenu(hMnu,MF_ENABLED|MF_STRING,WM_USER+5,"&Import from file...\tCtrl+E");
            AppendMenu(hMnu,MF_ENABLED|MF_STRING,WM_APP+10,"Convert");
            AppendMenu(hMnu,MF_SEPARATOR,NULL,NULL);
            AppendMenu(hMnu,MF_ENABLED|MF_POPUP,(UINT)im.Menu(),"&MIDI voice\tCtrl+M");
            AppendMenu(hMnu,MF_ENABLED|MF_POPUP,(UINT)tm.Menu(),"T&empo");

            AppendMenu(hClefMnu,MF_ENABLED|MF_STRING,0,"&Auto\tCtrl+0");
            AppendMenu(hClefMnu,MF_SEPARATOR,NULL,NULL);
            AppendMenu(hClefMnu,MF_ENABLED|MF_STRING,27,"Treble 8&va\tCtrl+1");
            AppendMenu(hClefMnu,MF_ENABLED|MF_STRING,28,"&Treble\tCtrl+2");
            AppendMenu(hClefMnu,MF_ENABLED|MF_STRING,29,"Treble &8vb\tCtrl+3");
            AppendMenu(hClefMnu,MF_ENABLED|MF_STRING,30,"&Bass\tCtrl+4");
            AppendMenu(hClefMnu,MF_ENABLED|MF_STRING,31,"Ba&ss 8vb\tCtrl+5");
            CheckMenuItem(hClefMnu,27+pwd->Clef,MF_BYCOMMAND|MF_CHECKED);
            AppendMenu(hMnu,MF_ENABLED|MF_POPUP,(UINT)hClefMnu,"Clef");
            AppendMenu(hMnu,MF_SEPARATOR,NULL,NULL);

            AppendMenu(hNameMnu,MF_ENABLED|MF_STRING,WM_USER+'A',"&A / La\tA");
            AppendMenu(hNameMnu,MF_ENABLED|MF_STRING,WM_USER+'B',"&B / Si\tB");
            AppendMenu(hNameMnu,MF_ENABLED|MF_STRING,WM_USER+'C',"&C / Do\tC");
            AppendMenu(hNameMnu,MF_ENABLED|MF_STRING,WM_USER+'D',"&D / Re\tD");
            AppendMenu(hNameMnu,MF_ENABLED|MF_STRING,WM_USER+'E',"&E / Mi\tE");
            AppendMenu(hNameMnu,MF_ENABLED|MF_STRING,WM_USER+'F',"&F / Fa\tF");
            AppendMenu(hNameMnu,MF_ENABLED|MF_STRING,WM_USER+'G',"&G / Sol\tG");
            AppendMenu(hNameMnu,MF_SEPARATOR,NULL,NULL);
            AppendMenu(hNameMnu,MF_ENABLED|MF_STRING,WM_USER+'r',"&Rest\tR");
            if (note)
            {
                CheckMenuItem(hNameMnu,WM_USER+(note->Name=='R'?'r':note->Name),MF_BYCOMMAND|MF_CHECKED);
            }
            AppendMenu(hMnu,MF_ENABLED|MF_POPUP,(UINT)hNameMnu,"&Name");

            AppendMenu(hAccidentMnu,MF_ENABLED|MF_STRING,WM_USER+'$',"11/2 sharp\t$");
            AppendMenu(hAccidentMnu,MF_ENABLED|MF_STRING,WM_USER+'#',"Sharp\t#");
            AppendMenu(hAccidentMnu,MF_ENABLED|MF_STRING,WM_USER+'+',"Half sharp\t+");
            AppendMenu(hAccidentMnu,MF_ENABLED|MF_STRING,WM_USER+'=',"(None)\t=");
            AppendMenu(hAccidentMnu,MF_ENABLED|MF_STRING,WM_USER+'-',"Half flat\t-");
            AppendMenu(hAccidentMnu,MF_ENABLED|MF_STRING,WM_USER+'@',"Flat\t@");
            AppendMenu(hAccidentMnu,MF_ENABLED|MF_STRING,WM_USER+'!',"11/2 flat\t!");
            if (note)
            {
                CheckMenuItem(hAccidentMnu,WM_USER+note->Accidental,MF_BYCOMMAND|MF_CHECKED);
            }
            AppendMenu(hMnu,MF_ENABLED|MF_POPUP,(UINT)hAccidentMnu,"Accidental");

            AppendMenu(hDurationMnu,MF_ENABLED|MF_STRING,WM_USER+'w',"Whole note\tW");
            AppendMenu(hDurationMnu,MF_ENABLED|MF_STRING,WM_USER+'h',"Half note\tH");
            AppendMenu(hDurationMnu,MF_ENABLED|MF_STRING,WM_USER+'q',"Quarter note\tQ");
            AppendMenu(hDurationMnu,MF_ENABLED|MF_STRING,WM_USER+'i',"Eighth note\tI");
            AppendMenu(hDurationMnu,MF_ENABLED|MF_STRING,WM_USER+'s',"Sixteenth note\tS");
            AppendMenu(hDurationMnu,MF_ENABLED|MF_STRING,WM_USER+'z',"Grace note\tZ");
            if (note)
            {
                CheckMenuItem(hDurationMnu,WM_USER+note->Duration,MF_BYCOMMAND|MF_CHECKED);
            }
            AppendMenu(hDurationMnu,MF_SEPARATOR,NULL,NULL);
            AppendMenu(hDurationMnu,MF_ENABLED|MF_STRING,WM_USER+'.',"Dotted\t.");
            AppendMenu(hDurationMnu,MF_ENABLED|MF_STRING,WM_USER+'t',"Triplet\tT");
            AppendMenu(hDurationMnu,MF_ENABLED|MF_STRING,WM_USER+'v',"Quintuplet\tV");
            AppendMenu(hDurationMnu,MF_ENABLED|MF_STRING,WM_USER+'n',"(normal)\tN");
            if (note)
            {
                CheckMenuItem(hDurationMnu,WM_USER+note->Duration2,MF_BYCOMMAND|MF_CHECKED);
            }
            AppendMenu(hMnu,MF_ENABLED|MF_POPUP,(UINT)hDurationMnu,"D&uration");

            AppendMenu(hOctaveMnu,MF_ENABLED|MF_STRING,WM_USER+'7',"Octave &7 (Piccolo)\t7");
            AppendMenu(hOctaveMnu,MF_ENABLED|MF_STRING,WM_USER+'6',"Octave &6 (Sopranino)\t6");
            AppendMenu(hOctaveMnu,MF_ENABLED|MF_STRING,WM_USER+'5',"Octave &5 (Soprano)\t5");
            AppendMenu(hOctaveMnu,MF_ENABLED|MF_STRING,WM_USER+'4',"Octave &4 (Alto/Tenor)\t4");
            AppendMenu(hOctaveMnu,MF_ENABLED|MF_STRING,WM_USER+'3',"Octave &3 (Tenor/Bass)\t3");
            AppendMenu(hOctaveMnu,MF_ENABLED|MF_STRING,WM_USER+'2',"Octave &2 (Bass)\t2");
            AppendMenu(hOctaveMnu,MF_ENABLED|MF_STRING,WM_USER+'1',"Octave &1 (Contrabass)\t1");
            AppendMenu(hOctaveMnu,MF_ENABLED|MF_STRING,WM_USER+'0',"Octave &0\t0");
            if (note)
            {
                CheckMenuItem(hOctaveMnu,WM_USER+note->Octave,MF_BYCOMMAND|MF_CHECKED);
            }
            AppendMenu(hMnu,MF_ENABLED|MF_POPUP,(UINT)hOctaveMnu,"&Octave");

            AppendMenu(hGlideMnu,MF_ENABLED|MF_STRING,WM_USER+'`',"Short fall after\t`");
            AppendMenu(hGlideMnu,MF_ENABLED|MF_STRING,WM_USER+'\\',"Long fall after\t\\");
            AppendMenu(hGlideMnu,MF_ENABLED|MF_STRING,WM_USER+'\'',"Short rise after\t\'");
            AppendMenu(hGlideMnu,MF_ENABLED|MF_STRING,WM_USER+'/',"Long rise after\t/");
            AppendMenu(hGlideMnu,MF_ENABLED|MF_STRING,WM_USER+'_',"Tied to next note\t_");
            AppendMenu(hGlideMnu,MF_ENABLED|MF_STRING,WM_USER+'x',"None of the above\tX");
            if (note)
            {
                CheckMenuItem(hGlideMnu,WM_USER+(note->Conclusion==' '?'x':note->Conclusion),MF_BYCOMMAND|MF_CHECKED);
            }
            AppendMenu(hGlideMnu,MF_SEPARATOR,NULL,NULL);
            AppendMenu(hGlideMnu,MF_ENABLED|MF_STRING,WM_USER+'~',"Short fall before\t~");
            AppendMenu(hGlideMnu,MF_ENABLED|MF_STRING,WM_USER+'|',"Long fall before\t|");
            AppendMenu(hGlideMnu,MF_ENABLED|MF_STRING,WM_USER+'\"',"Short rise before\t\"");
            AppendMenu(hGlideMnu,MF_ENABLED|MF_STRING,WM_USER+'?',"Long rise before\t?");
            AppendMenu(hGlideMnu,MF_ENABLED|MF_STRING,WM_USER+'*',"None of the above\t*");
            if (note)
            {
                CheckMenuItem(hGlideMnu,WM_USER+note->Onset,MF_BYCOMMAND|MF_CHECKED);
            }
            AppendMenu(hMnu,MF_ENABLED|MF_POPUP,(UINT)hGlideMnu,"&Glide");
            AppendMenu(hMnu,MF_SEPARATOR,NULL,NULL);

            AppendMenu(hSymbolMnu,MF_ENABLED|MF_STRING,WM_USER+'[',"Bar\t[");
            AppendMenu(hSymbolMnu,MF_ENABLED|MF_STRING,WM_USER+']',"Thick bar\t]");
            AppendMenu(hSymbolMnu,MF_ENABLED|MF_STRING,WM_USER+',',"Subphrase break\t,");
            AppendMenu(hSymbolMnu,MF_ENABLED|MF_STRING,WM_USER+';',"Phrase break\t;");
            AppendMenu(hSymbolMnu,MF_ENABLED|MF_STRING,WM_USER+'&',"Space\t&&");
            AppendMenu(hMnu,MF_ENABLED|MF_POPUP,(UINT)hSymbolMnu,"&Symbol");

            RECT r;
            GetWindowRect(hWnd,&r);

            TrackPopupMenu(hMnu,TPM_RIGHTALIGN,LOWORD(lParam)+r.left,HIWORD(lParam)+r.top,0,hWnd,NULL);
            DestroyMenu(hMnu);
            DestroyMenu(hNameMnu);
            DestroyMenu(hOctaveMnu);
            DestroyMenu(hAccidentMnu);
            DestroyMenu(hSymbolMnu);
        }
        return 0;
    }
    case WM_MOUSEMOVE:
    {
        // is the user draggin the mouse?
        if (!(wParam&MK_LBUTTON))
        {
            return 0;
        }

        struct SPartWindowData * pwd;
        pwd = (struct SPartWindowData *) GetWindowLong(hWnd,GWL_USERDATA);

        if (pwd)
        {
            if (!pwd->Active)
            {
                break;
            }

            unsigned NewSelect = pwd->Position(LOWORD(lParam));
            if (pwd->SelectEnd != NewSelect)
            {
                pwd->SelectEnd = NewSelect;
                InvalidateRgn(hWnd,NULL,FALSE);
            }
        }

        return 0;
    }
    case WM_SETFOCUS:
    {
        if (lParam)
        {
            struct SPartWindowData * pwd;
            pwd = (struct SPartWindowData *) GetWindowLong(hWnd,GWL_USERDATA);
            if (pwd)
            {
                pwd->Active = TRUE;
                SendMessage(pwd->hMyParent,WM_PARENTNOTIFY,WM_SETFOCUS,lParam);
            }
        }
        return 0;
    }
    case WM_KILLFOCUS:
    {
        if (lParam)
        {
            struct SPartWindowData * pwd;
            pwd = (struct SPartWindowData *) GetWindowLong(hWnd,GWL_USERDATA);
            if (pwd)
            {
                pwd->Active = FALSE;
                SendMessage(pwd->hMyParent,WM_PARENTNOTIFY,WM_KILLFOCUS,lParam);
            }
        }
        return 0;
    }
    case WM_ACTIVATE:
    case WM_ACTIVATEAPP:
    {
        struct SPartWindowData * pwd;
        pwd = (struct SPartWindowData *) GetWindowLong(hWnd,GWL_USERDATA);
        if (pwd)
        {
            if (wParam==WA_INACTIVE)
            {
                pwd->Active = FALSE;
            }
            else
            {
                pwd->Active = TRUE;
            }
        }
        return 0;
    }
    case WM_ENABLE:
    {
        struct SPartWindowData * pwd  = (struct SPartWindowData *)GetWindowLong(hWnd,GWL_USERDATA);
        if (pwd)
        {
            pwd->Enabled = (BOOL) wParam;
            InvalidateRgn(hWnd,NULL,FALSE);
            return 0;
        }
        break;
    }
    case WM_SETTEXT:
    {
        struct SPartWindowData * pwd = (struct SPartWindowData *)GetWindowLong(hWnd,GWL_USERDATA);

        if (pwd && pwd->m_sPlay.Tempo)
        {
            MessageBeep(MB_OK);
            return 0;         // Do not allow editing if playing
        }

        if (pwd)
        {
            pwd->ReplaceString((char *)lParam,0,pwd->MelodySize);

            int OldClef = pwd->Clef;
            int NewClef = pwd->WhichClef();
            if (NewClef!=OldClef)
            {
                pwd->SetClef(NewClef);
            }
            pwd->GoHome(hWnd);
            return TRUE;
        }
        return FALSE;
    }
    case WM_GETTEXT:
    {
        if (!wParam)
        {
            return 0;
        }

        struct SPartWindowData * pwd = (struct SPartWindowData *)GetWindowLong(hWnd,GWL_USERDATA);
        if (pwd)
        {
            char * Text = pwd->GetString(0,pwd->MelodySize);
            if (Text!=NULL)
            {
                unsigned TextLen = strlen(Text);
                if (TextLen>wParam)
                {
                    Text[wParam-1]='\0';
                    TextLen=wParam;
                }
                strcpy_s((char *)lParam,wParam,Text);
                delete Text;
                return TextLen;
            }
        }
        *((char *)lParam)='\0';
        return 0;
    }
    case WM_GETTEXTLENGTH:
    {
        struct SPartWindowData * pwd = (struct SPartWindowData *)GetWindowLong(hWnd,GWL_USERDATA);
        return (pwd)? ((pwd->MelodySize + 3) * 7) + 1 : 0; // add 3 SAMAs for (CLEFx) & (VXxxx) & (Q=xxx)
    }
    case WM_CUT:
    {
        struct SPartWindowData * pwd = (struct SPartWindowData *)GetWindowLong(hWnd,GWL_USERDATA);
        if (pwd && pwd->m_sPlay.Tempo)
        {
            MessageBeep(MB_OK);
            return 0;         // Do not allow editing if playing
        }

        if (pwd && pwd->SelectBegin!=pwd->SelectEnd)
        {
            SendMessage(hWnd,WM_COPY,0,0);
            SendMessage(hWnd,WM_CLEAR,0,0);
        }
        else
        {
            MessageBeep(MB_OK);
        }
        return 0;
    }
    case WM_COPY:
    {
        CClipboard Clip(hWnd);
        struct SPartWindowData * pwd = (struct SPartWindowData *)GetWindowLong(hWnd,GWL_USERDATA);
        if (pwd)
        {
            char * RTFText = NULL;
            char * Text = NULL;

            if (pwd->SelectMin()==pwd->SelectMax() ||
                    (!pwd->SelectMin() && pwd->SelectMax()==pwd->MelodySize))
            {
                Text = pwd->GetString(0,pwd->MelodySize);
                WCHAR * RTFTextW = pwd->PartEncode(0,pwd->MelodySize);
                unsigned RTFTextWLen = wcslen(RTFTextW);
                unsigned UniCharLen = 3 + (unsigned) log10((double)MUSIQUE_FONT_SIZE);
                size_t len = 4 + RTFTextWLen * UniCharLen + 4 * UniCharLen + 1;
                RTFText = new char[len];

                int i=0;
                RTFText[i++]='\\';
                RTFText[i++]='u';
                RTFText[i++]='c';
                RTFText[i++]='0';
                RTFText[i++]='\\';
                RTFText[i++]='u';
                switch (pwd->Clef)
                {
                case 1:
                    _ltoa_s(DISPLAY_TREBLE,&RTFText[i],len-i,10);
                    break;
                case 2:
                    _ltoa_s(DISPLAY_TREBLE_8VB,&RTFText[i],len-i,10);
                    break;
                case 3:
                    _ltoa_s(DISPLAY_BASS,&RTFText[i],len-i,10);
                    break;
                case 0:
                    _ltoa_s(DISPLAY_TREBLE_8VA,&RTFText[i],len-i,10);
                    break;
                case 4:
                    _ltoa_s(DISPLAY_BASS_8VB,&RTFText[i],len-i,10);
                    break;
                }
                while (RTFText[i])
                {
                    ++i;
                }
                for (register unsigned j=0; j<RTFTextWLen; ++j)
                {
                    RTFText[i++]='\\';
                    RTFText[i++]='u';
                    _ltoa_s(RTFTextW[j],&RTFText[i],len-i,10);
                    while (RTFText[i])
                    {
                        ++i;
                    }
                }
                RTFText[i++]='\\';
                RTFText[i++]='u';
                _ltoa_s(DISPLAY_BAR,&RTFText[i],len-i,10);
                while (RTFText[i])
                {
                    ++i;
                }
                RTFText[i++]='\\';
                RTFText[i++]='u';
                _ltoa_s(DISPLAY_SMALL_SPACE,&RTFText[i],len-i,10);
                while (RTFText[i])
                {
                    ++i;
                }
                RTFText[i++]='\\';
                RTFText[i++]='u';
                _ltoa_s(DISPLAY_THICK_BAR,&RTFText[i],len-i,10);
                while (RTFText[i])
                {
                    ++i;
                }
                delete RTFTextW;
            }
            else
            {
                WCHAR * RTFTextW = pwd->PartEncode(pwd->SelectMin(),pwd->SelectMax());
                unsigned RTFTextWLen = wcslen(RTFTextW);
                unsigned UniCharLen = 3 + (unsigned) log10((double)MUSIQUE_FONT_SIZE);
                size_t len = (RTFTextWLen*UniCharLen)+(4*UniCharLen)+1;
                RTFText = new char[len];
                register int i=0;
                RTFText[i++]='\\';
                RTFText[i++]='u';
                RTFText[i++]='c';
                RTFText[i++]='0';
                for (register unsigned j=0; j<RTFTextWLen; ++j)
                {
                    RTFText[i++]='\\';
                    RTFText[i++]='u';
                    _ltoa_s(RTFTextW[j],&RTFText[i],len-i,10);
                    while (RTFText[i])
                    {
                        ++i;
                    }
                }
                Text = pwd->GetString(pwd->SelectMin(),pwd->SelectMax());
                delete RTFTextW;
            }
            if (Text)
            {
                if (*RTFText && *Text)
                {
                    Clip.SetTextRTF(RTFText,Text);
                }
                delete [] RTFText;
                delete Text;
            }
        }
        return 0;
    }
    case WM_CLEAR:
    {
        struct SPartWindowData * pwd = (struct SPartWindowData *)GetWindowLong(hWnd,GWL_USERDATA);
        if (pwd && pwd->m_sPlay.Tempo)
        {
            MessageBeep(MB_OK);
            return 0;         // Do not allow editing if playing
        }

        if (pwd)
        {
            unsigned ClearEnd = pwd->SelectMax();
            if (pwd->SelectMin()==ClearEnd && ClearEnd<pwd->MelodySize)
            {
                ++ClearEnd;
            }

            pwd->ReplaceString("",pwd->SelectMin(),ClearEnd);
            pwd->SelectEnd = pwd->SelectBegin = pwd->SelectMin();
            InvalidateRgn(hWnd,NULL,FALSE);
            pwd->Modified = TRUE;
            SendMessage(pwd->hMyParent,WM_PARENTNOTIFY,EM_SETMODIFY,1);
        }
        return 0;
    }
    case WM_PASTE:
    {
        CClipboard Clip(hWnd);
        if (Clip.hasText())
        {
            struct SPartWindowData * pwd = (struct SPartWindowData *)GetWindowLong(hWnd,GWL_USERDATA);
            if (pwd && pwd->m_sPlay.Tempo)
            {
                MessageBeep(MB_OK);
                return 0;         // Do not allow editing if playing
            }

            if (pwd)
            {
                char * String;
                Clip >> String;
                if (String)
                {
                    // minimal validity checking
                    if (strchr("ABCDEFGR(abcdefgr",*String))
                    {
                        int PasteLen = pwd->ReplaceString(String,pwd->SelectMin(),pwd->SelectMax());
                        pwd->SelectBegin = pwd->SelectMin();
                        pwd->SelectEnd = pwd->SelectBegin + PasteLen;
                    }
                    delete String;
                }
                pwd->AutoScrollRight();
                InvalidateRgn(hWnd,NULL,FALSE);
                pwd->Modified = TRUE;
                SendMessage(pwd->hMyParent,WM_PARENTNOTIFY,EM_SETMODIFY,1);
            }
        }

        return 0;
    }
    case WM_DESTROY:
    {
        struct SPartWindowData * pwd = (struct SPartWindowData *) GetWindowLong(hWnd,GWL_USERDATA);
        if (pwd)
        {
            delete pwd;
        }
        return 0;
    }
    case WM_CLOSE:
    {
        DestroyWindow(hWnd);
        return 0;
    }
    case WM_SIZE:
    {
        struct SPartWindowData * pwd = (struct SPartWindowData *) GetWindowLong(hWnd,GWL_USERDATA);
        if (pwd)
        {
            if (pwd->hFont)
            {
                DeleteObject(pwd->hFont);
                pwd->hFont = NULL;
            }
            pwd->ViewWidthPixels = LOWORD(lParam);
            pwd->AutoScrollRight();
            InvalidateRgn(hWnd,NULL,TRUE);
            return 0;
        }
        break;
    }
    case WM_SYSCOMMAND:
    {
        if (wParam==SC_CLOSE)
        {
            DestroyWindow(hWnd);
            return 0;
        }
        if (wParam==SC_KEYMENU)
        {
            struct SPartWindowData * pwd = (struct SPartWindowData *) GetWindowLong(hWnd,GWL_USERDATA);
            SendMessage(pwd->hMyParent,WM_SYSCOMMAND,SC_KEYMENU,lParam);
            return 0;
        }
        break;
    }
    case EM_GETLIMITTEXT:
        return 32000;
    case EM_GETMODIFY:
    {
        struct SPartWindowData * pwd = (struct SPartWindowData *) GetWindowLong(hWnd,GWL_USERDATA);
        if (pwd)
        {
            return pwd->Modified;
        }
        else
        {
            return 0;
        }
    }
    case EM_SETMODIFY:
    {
        struct SPartWindowData * pwd = (struct SPartWindowData *) GetWindowLong(hWnd,GWL_USERDATA);
        if (pwd)
        {
            pwd->Modified = wParam;
        }
        return 0;
    }
    case WM_GETDLGCODE:
    {
        return DLGC_WANTARROWS|DLGC_WANTCHARS;
    }
    case WM_CREATE:
    {
        static SPartClassData * pcd = NULL;
        if (!pcd)
        {
            static SPartClassData autocleanup;
            pcd = &autocleanup;
            SetClassLong(hWnd,0,(LONG)pcd);
        }
        SPartWindowData * pwd = NULL;
        if (!pwd)
        {
            pwd = new SPartWindowData(hWnd,DELTA);
            SetWindowLong(hWnd,GWL_USERDATA,(LONG)pwd);
            LPCREATESTRUCT lpcs = (LPCREATESTRUCT) lParam;
            pwd->ViewWidthPixels = lpcs->cx;
            pwd->hMyParent = lpcs->hwndParent;
            pwd->hPlayButton = CreateWindow("BUTTON","\x91",WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON,
                                            3,3,20,20,hWnd,(HMENU)IDM_PLAY,hInstance,NULL);
            pwd->hPauseButton = CreateWindow("BUTTON","\x92",WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON,
                                             23,3,20,20,hWnd,(HMENU)IDM_PAUSE,hInstance,NULL);
            pwd->hStopButton = CreateWindow("BUTTON","\x93",WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON,
                                            43,3,20,20,hWnd,(HMENU)IDM_STOP,hInstance,NULL);
            pwd->hLoopButton = CreateWindow("BUTTON","\x94",WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON,
                                            63,3,20,20,hWnd,(HMENU)IDM_LOOP,hInstance,NULL);
            SendMessage(pwd->hPlayButton,WM_SETFONT,(WPARAM)pcd->hButtonFont,0);
            SendMessage(pwd->hPauseButton,WM_SETFONT,(WPARAM)pcd->hButtonFont,0);
            SendMessage(pwd->hStopButton,WM_SETFONT,(WPARAM)pcd->hButtonFont,0);
            SendMessage(pwd->hLoopButton,WM_SETFONT,(WPARAM)pcd->hButtonFont,0);

            //TEMPORARY
            EnableWindow(pwd->hPauseButton,FALSE);
            EnableWindow(pwd->hLoopButton,FALSE);
        }

        break;
    }
    case WM_APP + 1:   // write file (filename = (char*)lParam)
    {
        struct SPartWindowData * pwd = (struct SPartWindowData *) GetWindowLong(hWnd,GWL_USERDATA);
        if (pwd)
        {
            char * fn = (char *)malloc(256); // filename
            char * pn = ""; // pathname

            if (lParam)
            {
                strcpy_s(fn,256,(char *)lParam);

                if (wParam!=0)
                {
                    pn = (char *)malloc(256);
                    strcpy_s(pn,256,(char *)lParam);
                    register int i=strlen(pn);
                    for (; i&&pn[i]!='\\'; --i);
                    pn[i+1]='\0';
                    strcpy_s(fn,256,fn+i+1);
                }
            }
            else
            {
                *fn = '\0';
            }

            if (!lParam || wParam!=0)   // choose filename from dialog box
            {
                OPENFILENAME ofn;
                /* Set all structure members to zero. */
                memset(&ofn, 0, sizeof(OPENFILENAME));
                /* Initialize the OPENFILENAME members. */
                ofn.lStructSize = sizeof(OPENFILENAME);
                ofn.hwndOwner = hWnd;
                ofn.lpstrFilter = "MusicXML file (*.xml)\0*.xml\0MIDI file (*.mid)\0*.mid\0Rich Text file (*.rtf)\0*.rtf\0Text file (*.txt)\0*.txt\0\0";
                ofn.lpstrFile= fn;
                ofn.nMaxFile = 256;
                ofn.lpstrFileTitle = NULL;
                ofn.nMaxFileTitle = 0;
                ofn.lpstrInitialDir = pn;
                ofn.lpstrDefExt = ".xml";
                ofn.Flags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;

                //ofn.lpstrTitle = "Music staff : export data to file";
                if (!GetSaveFileName(&ofn))
                {
                    free(fn);
                    return 0;
                }
            }
            int fnlen = strlen(fn);
            if (!fnlen)
            {
                free(fn);
                return 0;
            }
            BOOL Ret = FALSE;
            if (fnlen>4)
            {
                if (!_stricmp(&fn[fnlen-4],".xml"))
                {
                    Ret = pwd->MusicXMLOut(fn);
                }
                else if (!_stricmp(&fn[fnlen-4],".mid"))
                {
                    Ret = pwd->MIDIOut(fn);
                }
                else if (!_stricmp(&fn[fnlen-4],".rtf"))
                {
                    Ret =  pwd->RTFOut(fn);
                }
                else
                {
                    Ret = pwd->TXTOut(fn);
                }
            }
            else
            {
                Ret = pwd->TXTOut(fn);
            }
            if (!Ret)
            {
                free(fn);
                return Ret;
            }
            else if (lParam)
            {
                free(fn);
                return lParam;
            }
            return (LRESULT) fn;
        }
        else
        {
            return 0;
        }
    }
    case WM_APP + 2:   // play selection
    {
        struct SPartWindowData * pwd = (struct SPartWindowData *) GetWindowLong(hWnd,GWL_USERDATA);
        if (pwd)
        {
            char lpszTempFileName[144];
            OFSTRUCT ofTmp;

            // TCJ 7/00 - 32bit conversion
            char lpszTempPath[_MAX_PATH];
            GetTempPath(sizeof(lpszTempPath), lpszTempPath);
            GetTempFileName(lpszTempPath,"SIL",0,lpszTempFileName);
            OpenFile(lpszTempFileName,&ofTmp,OF_DELETE); // Delete automatically created file
            if (strlen(lpszTempFileName)>3)
                strcpy_s(lpszTempFileName + strlen(lpszTempFileName) - 3,
                         _countof(lpszTempFileName)-strlen(lpszTempFileName) + 3,
                         "mid");

            // save to MIDI file
            if (SendMessage(hWnd,WM_APP + 1,0,(LPARAM) lpszTempFileName))
            {
                if (pwd->SelectBegin != pwd->SelectEnd)
                {
                    pwd->m_sPlay.nMin = pwd->SelectMin();
                    pwd->m_sPlay.nMax = pwd->SelectMax();
                }
                else
                {
                    pwd->m_sPlay.nMin = 0;
                    pwd->m_sPlay.nMax = pwd->MelodySize;
                }

                pwd->m_sPlay.ViewBegin = pwd->ViewBegin;
                pwd->m_sPlay.SelectBegin = pwd->SelectBegin;
                pwd->m_sPlay.SelectEnd = pwd->SelectEnd;

                int nResult = 0;
                if (wParam)
                {
                    nResult = PlayMidiFile(lpszTempFileName,hWnd);
                }
                if (!nResult)
                {
                    if (pwd->SelectBegin < pwd->ViewBegin)
                    {
                        pwd->ViewBegin = pwd->SelectBegin;
                    }

                    pwd->m_sPlay.dwStartTime = GetTickCount();

                    pwd->m_sPlay.Tempo = pwd->Tempo;

                    SetTimer( hWnd, ID_TIMER_MIDI, PLAY_TIMER_INTERVAL,NULL);
                    SendMessage(hWnd,WM_TIMER, 0, 0);
                    if (lParam)
                    {
                        SPartSelectionMS & selectionTime = pwd->m_sPlay.selectionTime;
                        double dHere = 0;
                        unsigned i;
                        unsigned begin = pwd->m_sPlay.nMin;
                        unsigned end   = pwd->m_sPlay.nMax;

                        for (i=0; i < begin; i++)
                        {
                            dHere +=  pwd->Melody[i]->MIDIDuration();
                        }
                        selectionTime.begin = (dHere) * 60. / (pwd->Tempo * TICKS_PER_Q);
                        if (selectionTime.begin < 0)
                        {
                            selectionTime.begin = 0;
                        }

                        for (; i < end; i++)
                        {
                            dHere +=  pwd->Melody[i]->MIDIDuration();
                        }
                        selectionTime.end = (dHere) * 60. / (pwd->Tempo * TICKS_PER_Q);
                        SendMessage(pwd->hMyParent,WM_PARENTNOTIFY,WM_APP + 2, (LPARAM) &selectionTime);
                    }
                }

                OpenFile(lpszTempFileName,&ofTmp,OF_DELETE);
                return 1;
            }
        }
        return 0;
    }
    case WM_TIMER:   // Play feedback timer
    {
        struct SPartWindowData * pwd = (struct SPartWindowData *) GetWindowLong(hWnd,GWL_USERDATA);
        if (pwd)
        {
            if (!pwd->m_sPlay.Tempo)
            {
                KillTimer(hWnd, ID_TIMER_MIDI);
                return 0;
            }

            DWORD dwElapsed = GetTickCount() - pwd->m_sPlay.dwStartTime;
            double dElapsedTicks = dwElapsed * (pwd->m_sPlay.Tempo * TICKS_PER_Q/60000.);
            double dPlayError = 100 * (pwd->m_sPlay.Tempo * TICKS_PER_Q/60000.);

            if (pwd->MelodySize < pwd->m_sPlay.nMax)
            {
                pwd->m_sPlay.nMax = pwd->MelodySize;
            }

            double dHere = 0;
            unsigned i=pwd->m_sPlay.nMin;
            for (; i < pwd->m_sPlay.nMax; i++)
            {
                dHere +=  pwd->Melody[i]->MIDIDuration();

                if (dHere > dElapsedTicks - dPlayError)
                {
                    break;
                }
            }
            if (i < pwd->m_sPlay.nMax)
            {
                pwd->SelectBegin = i > pwd->m_sPlay.nMin ? i : pwd->m_sPlay.nMin;
                for (; i < pwd->m_sPlay.nMax; i++)
                {
                    dHere +=  pwd->Melody[i]->MIDIDuration();

                    if (dHere > dElapsedTicks + dPlayError)
                    {
                        break;
                    }
                }
                pwd->SelectEnd = i < pwd->m_sPlay.nMax ? i+1 : pwd->m_sPlay.nMax ;

                unsigned oldViewBegin = pwd->ViewBegin;
                pwd->AutoScrollRight(); // Scoll playing notes into view

                if (pwd->ViewBegin != oldViewBegin)   // Scrolled
                {
                    pwd->ViewBegin = pwd->SelectBegin;    // Scroll a whole page to minimize scrolling
                }

                RedrawWindow(hWnd,NULL,NULL,RDW_INVALIDATE | RDW_UPDATENOW);
            }
            else
            {
                StopMidiFile(hWnd);
            }
        }

        return 0;
    }
    case WM_APP + 3:   // read file
    {
        struct SPartWindowData * pwd = (struct SPartWindowData *) GetWindowLong(hWnd,GWL_USERDATA);
        if (pwd && pwd->m_sPlay.Tempo)
        {
            MessageBeep(MB_OK);
            return 0;         // Do not allow editing if playing
        }

        if (pwd)
        {
            OPENFILENAME ofn;

            char * szFile = (char *)malloc(256);
            if (lParam && *((char *)lParam))
            {
                strcpy_s(szFile,256,(char *)lParam);
            }
            else
            {
                szFile[0] = '\0';
            }

            /* Set all structure members to zero. */
            memset(&ofn, 0, sizeof(OPENFILENAME));

            ofn.lStructSize = sizeof(OPENFILENAME);
            ofn.hwndOwner = hWnd;
            ofn.hInstance = NULL;
            ofn.lpstrFilter = "MusicXML file (*.xml)\0*.xml\0MIDI file (*.mid)\0*.mid\0Text file (*.txt)\0*.txt\0\0";
            ofn.nFilterIndex = 1;
            ofn.lpstrFile= szFile;
            ofn.nMaxFile = 256;
            ofn.lpstrFileTitle = NULL;
            ofn.nMaxFileTitle = 0;
            ofn.lpstrInitialDir = NULL;
            ofn.lpstrDefExt = ".xml";
            ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;

            //ofn.lpstrTitle = "Music staff : import data from file";

            if (!GetOpenFileName(&ofn))
            {
                free(szFile);
                return 0;
            }

            int fnlen = strlen(szFile);
            if (!fnlen)
            {
                return 0;
            }

            BOOL Ret = FALSE;
            if (fnlen>4 && !_stricmp(&szFile[fnlen-4],".xml"))
            {
                Ret = pwd->MusicXMLIn(szFile);
            }
            else if (fnlen>4 && !_stricmp(&szFile[fnlen-4],".mid"))
            {
                Ret = pwd->MIDIIn(szFile);
            }
            else
            {
                Ret = pwd->TXTIn(szFile);
            }
            return Ret;
        }
        else
        {
            return 0;
        }
    }
    case WM_APP + 4:   // pause MIDI
    {
        return PauseMidiFile(hWnd);
    }
    case WM_APP + 5:   // stop MIDI
    {
        return StopMidiFile(hWnd);
    }
    case WM_APP + 6:   // loop play selection
    {
        struct SPartWindowData * pwd = (struct SPartWindowData *) GetWindowLong(hWnd,GWL_USERDATA);
        if (pwd)
        {
            char lpszTempFileName[144];
            OFSTRUCT ofTmp;

            // TCJ 7/00 - 32bit conversion
            char lpszTempPath[_MAX_PATH];
            GetTempPath(sizeof(lpszTempPath), lpszTempPath);
            GetTempFileName(lpszTempPath,"SIL",0,lpszTempFileName);
            OpenFile(lpszTempFileName,&ofTmp,OF_DELETE); // Delete automatically created file
            if (strlen(lpszTempFileName)>3)

                strcpy_s(lpszTempFileName + strlen(lpszTempFileName) - 3,
                         (_countof(lpszTempFileName)- (strlen(lpszTempFileName) - 3)),
                         "mid");

            // save to MIDI file
            if (SendMessage(hWnd,WM_APP + 1,0,(LPARAM) lpszTempFileName))
            {
                LoopMidiFile(lpszTempFileName,hWnd);

                OpenFile(lpszTempFileName,&ofTmp,OF_DELETE);
                return 1;
            }
        }
        return 0;
    }
    case WM_APP + 7:   // hide play buttons
    {
        struct SPartWindowData * pwd = (struct SPartWindowData *) GetWindowLong(hWnd,GWL_USERDATA);
        if (pwd)
        {
            if (pwd->hPlayButton)
            {
                DestroyWindow(pwd->hPlayButton);
                pwd->hPlayButton = NULL;
            }
            if (pwd->hPauseButton)
            {
                DestroyWindow(pwd->hPauseButton);
                pwd->hPauseButton = NULL;
            }
            if (pwd->hStopButton)
            {
                DestroyWindow(pwd->hStopButton);
                pwd->hStopButton = NULL;
            }
            if (pwd->hLoopButton)
            {
                DestroyWindow(pwd->hLoopButton);
                pwd->hLoopButton = NULL;
            }
        }
        return 0;
    }
    case WM_APP + 8:   // choose MIDI voice
    {
        struct SPartWindowData * pwd = (struct SPartWindowData *) GetWindowLong(hWnd,GWL_USERDATA);
        if (pwd && pwd->m_sPlay.Tempo)
        {
            MessageBeep(MB_OK);
            return 0;         // Do not allow editing if playing
        }

        if (pwd)
        {
            CInstrumentMenu im;
            im.Check(pwd->Instrument);
            POINT p;
            GetCursorPos(&p);
            TrackPopupMenu(im.Menu(),TPM_CENTERALIGN,p.x,p.y,0,hWnd,NULL);
        }
        return 0;
    }
    case WM_APP + 9:   // set tempo
    {
        struct SPartWindowData * pwd = (struct SPartWindowData *) GetWindowLong(hWnd,GWL_USERDATA);
        if (pwd && pwd->m_sPlay.Tempo)
        {
            MessageBeep(MB_OK);
            return 0;         // Do not allow editing if playing
        }

        if (pwd)
        {
            if (!wParam)   // choose tempo?
            {
                CTempoMenu tm;
                tm.Check(pwd->Tempo);
                POINT p;
                GetCursorPos(&p);
                TrackPopupMenu(tm.Menu(),TPM_CENTERALIGN,p.x,p.y,0,hWnd,NULL);
                return 0;
            }
            pwd->Tempo = wParam;
            InvalidateRgn(hWnd,NULL,FALSE); // force repaint of tempo label
        }
        return 0;
    }
    case WM_APP + 10:   // convert
    {
        struct SPartWindowData * pwd = (struct SPartWindowData *) GetWindowLong(hWnd,GWL_USERDATA);
        if (pwd && pwd->m_sPlay.Tempo)
        {
            MessageBeep(MB_OK);
            return 0;         // Do not allow editing if playing
        }

        if (pwd)
        {
            SendMessage(pwd->hMyParent,WM_PARENTNOTIFY,IDC_CONVERT,0);
        }
    }
    case WM_APP + 11:   // get tempo
    {
        struct SPartWindowData * pwd = (struct SPartWindowData *) GetWindowLong(hWnd,GWL_USERDATA);
        if (pwd)
        {
            return pwd->Tempo;
        }
        else
        {
            return 0;
        }

    }

    case WM_HSCROLL:
    {
        struct SPartWindowData * pwd = (struct SPartWindowData *) GetWindowLong(hWnd,GWL_USERDATA);
        if (pwd)
        {
            switch (LOWORD(wParam))
            {
            case SB_LEFT:
            case SB_LINELEFT:
                if (pwd->ViewBegin>0)
                {
                    pwd->ViewBegin--;
                    InvalidateRgn(hWnd,NULL,FALSE);
                }
                break;
            case SB_PAGELEFT:
            {
                unsigned j = pwd->SelectEnd;
                pwd->SelectEnd = pwd->ViewBegin;
                if (pwd->SelectEnd)
                {
                    --pwd->SelectEnd;
                }
                pwd->ViewBegin = 0;
                pwd->AutoScrollRight();
                pwd->SelectEnd = j;
                InvalidateRgn(hWnd,NULL,FALSE);
                break;
            }
            case SB_RIGHT:
            case SB_LINERIGHT:
                if ((pwd->ViewBegin+1)<pwd->MelodySize)
                {
                    pwd->ViewBegin++;
                    InvalidateRgn(hWnd,NULL,FALSE);
                }
                break;
            case SB_PAGERIGHT:
                pwd->ViewBegin = pwd->Position(pwd->ViewWidthPixels);
                if (pwd->ViewBegin && pwd->ViewBegin>=pwd->MelodySize)
                {
                    pwd->ViewBegin=pwd->MelodySize-1;
                }
                InvalidateRgn(hWnd,NULL,FALSE);
                break;
            case SB_THUMBTRACK:
            case SB_THUMBPOSITION:
                pwd->ViewBegin=HIWORD(wParam);
                InvalidateRgn(hWnd,NULL,FALSE);
                break;
            }
        }
        return 0;
    }
    case WM_GETMINMAXINFO:
    {
        MINMAXINFO * lpmmi = (MINMAXINFO *) lParam;
        lpmmi->ptMinTrackSize.y=45;
        break;
    }
    case WM_COMMAND:
    {
        SetFocus(hWnd);
        if (LOWORD(wParam)==IDM_PLAY)   // Play button pushed
        {
            return SendMessage(hWnd,WM_APP+2,1,0);
        }
        if (LOWORD(wParam)==IDM_PAUSE)   // Pause button pushed
        {
            return SendMessage(hWnd,WM_APP+4,0,0);
        }
        if (LOWORD(wParam)==IDM_STOP)   // Stop button pushed
        {
            return SendMessage(hWnd,WM_APP+5,0,0);
        }
        if (LOWORD(wParam)==IDM_LOOP)   // Loop button pushed
        {
            return SendMessage(hWnd,WM_APP+6,0,0);
        }
        else if (LOWORD(wParam)<=(WM_TEMPO+200) && LOWORD(wParam)>=WM_TEMPO)
        {
            return SendMessage(hWnd,WM_APP+9,LOWORD(wParam)-WM_TEMPO,0);
        }
        else if (!LOWORD(wParam))     // re-determine clef
        {
            struct SPartWindowData * pwd = (struct SPartWindowData *) GetWindowLong(hWnd,GWL_USERDATA);
            if (pwd)
            {
                pwd->SetClef(pwd->WhichClef());
                return 0;
            }
        }
        else if (LOWORD(wParam) >= 27 && LOWORD(wParam) <= 31)     // set clef
        {
            struct SPartWindowData * pwd = (struct SPartWindowData *) GetWindowLong(hWnd,GWL_USERDATA);
            if (pwd)
            {
                pwd->SetClef(LOWORD(wParam)-27);
                return 0;
            }
        }
        else if (LOWORD(wParam)==WM_APP+10)
        {
            PostMessage(hWnd,WM_APP+10,0,0);
            return 0;
        }
        else if (LOWORD(wParam)>WM_USER)
        {
            if (LOWORD(wParam)<WM_USER+128)   // KEYSTROKE
            {
                LRESULT ret = SendMessage(hWnd,WM_CHAR,LOWORD(wParam)-WM_USER,0L);
                if (LOWORD(wParam)<WM_USER+26)   // some keystrokes are handled by keyup instead
                {
                    struct SPartWindowData * pwd = (struct SPartWindowData *) GetWindowLong(hWnd,GWL_USERDATA);
                    if (pwd)
                    {
                        BOOL OldControl = pwd->Control;
                        pwd->Control = TRUE;
                        ret = SendMessage(hWnd,WM_KEYUP,'A'-1+LOWORD(wParam)-WM_USER,0L);
                        pwd->Control = OldControl;
                    }
                }
                return ret;
            }
            else if (LOWORD(wParam)<WM_USER+256)     // INSTRUMENT
            {
                struct SPartWindowData * pwd = (struct SPartWindowData *) GetWindowLong(hWnd,GWL_USERDATA);
                if (pwd)
                {
                    pwd->Instrument = (char)(LOWORD(wParam) - (WM_USER + 128));
                }
            }
        }
        else
        {
            return SendMessage(hWnd,(UINT)wParam,0,0L);
        }
    }
    }
    return DefWindowProc(hWnd,uMsg,wParam,lParam);
}//PartitureProc

int INIT(HINSTANCE hInst)
{
    hInstance = hInst;
    WNDCLASS wc;
    wc.style = CS_OWNDC/*|CS_GLOBALCLASS*/;
    wc.lpfnWndProc = (WNDPROC) PartitureProc;
    wc.cbClsExtra = 4;
    wc.cbWndExtra = 4;
    wc.hInstance = hInst;
    wc.hIcon = LoadIcon(NULL,IDI_HAND);
    wc.hCursor = LoadCursor(NULL,IDC_IBEAM);
    wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
    wc.lpszMenuName = NULL;
    wc.lpszClassName = "Partiture";

    return (int) RegisterClass(&wc);
}//Init

extern "C" void cdecl Splash(HWND hWnd,HINSTANCE hInst,char * BitmapName)
{
    HBITMAP hbmpMyBitmap, hbmpOld;
    BITMAP bm;

    hbmpMyBitmap = LoadBitmap(hInst, BitmapName);
    GetObject(hbmpMyBitmap, sizeof(BITMAP), &bm);

    RECT r;
    GetClientRect(hWnd,&r);
    int xx = (r.right/2) - (bm.bmWidth/2);
    if (xx<0)
    {
        xx=0;
    }
    int yy = (r.bottom/2) - (bm.bmHeight/2);
    if (yy<0)
    {
        yy=0;
    }

    HWND hSplashWindow = CreateWindow("STATIC",BitmapName,WS_BORDER|WS_CHILD|WS_VISIBLE,
                                      xx,yy,bm.bmWidth,bm.bmHeight,hWnd,NULL,hInst,NULL);
    if (!hSplashWindow)
    {
        return;
    }

    HDC hdc, hdcMemory;

    hdc = GetDC(hSplashWindow);
    hdcMemory = CreateCompatibleDC(hdc);
    hbmpOld = (HBITMAP) SelectObject(hdcMemory, hbmpMyBitmap);

    GetClientRect(hSplashWindow,&r);

    BitBlt(hdc, (r.right-bm.bmWidth)/2, (r.bottom-bm.bmHeight)/2, bm.bmWidth, bm.bmHeight, hdcMemory, 0, 0, SRCCOPY);
    SelectObject(hdcMemory, hbmpOld);
    DeleteObject(hbmpMyBitmap);

    DeleteDC(hdcMemory);
    ReleaseDC(hSplashWindow, hdc);
    Sleep(5000);
    DestroyWindow(hSplashWindow);
}//Splash


// static globals for MIDI commands
static WORD wDeviceID = NULL;
static DWORD dwReturn = NULL;
static MCI_OPEN_PARMS mciOpenParms;
static MCI_PLAY_PARMS mciPlayParms;
static MCI_STATUS_PARMS mciStatusParms;
static MCI_SEQ_SET_PARMS mciSeqSetParms;

static DWORD PlayMidiFile(char * lpszMIDIFileName, HWND hWndNotify)
{
    if (wDeviceID)
    {
        mciSendCommand(wDeviceID, MCI_CLOSE, 0, NULL);
    }

    memset((void *)&mciOpenParms,0,sizeof(MCI_OPEN_PARMS)); // ?????
    mciOpenParms.lpstrElementName = lpszMIDIFileName;

    dwReturn = mciSendCommand(NULL, MCI_OPEN, MCI_OPEN_ELEMENT, (DWORD)(LPVOID) &mciOpenParms);
    if (dwReturn)
    {
        // Failed to open device; don't close it, just return error
        LPSTR lpszErrorDescr = new char[512];
        mciGetErrorString(dwReturn, lpszErrorDescr, 512);
        MessageBox(hWndNotify,lpszErrorDescr,lpszMIDIFileName,MB_OK);
        return(DWORD(-1));
    }
    else
    {
        wDeviceID = (unsigned short)(mciOpenParms.wDeviceID);
    }

    mciOpenParms.lpstrDeviceType = "SMA Sequencer";

    mciSendCommand(wDeviceID, MCI_CLOSE, 0, NULL);

    mciOpenParms.lpstrDeviceType = "sequencer";
    mciOpenParms.lpstrElementName = lpszMIDIFileName;

    dwReturn = mciSendCommand(NULL, MCI_OPEN, MCI_OPEN_TYPE | MCI_OPEN_ELEMENT, (DWORD)(LPVOID) &mciOpenParms);
    if (dwReturn != 0)
    {
        // Failed to open device; don't close it, just return error
        LPSTR lpszErrorDescr = new char[512];
        mciGetErrorString(dwReturn, lpszErrorDescr, 512);
        MessageBox(hWndNotify,lpszErrorDescr,lpszMIDIFileName,MB_OK);
        return (DWORD(-2));
    }

    wDeviceID = (unsigned short)(mciOpenParms.wDeviceID);

    mciStatusParms.dwItem = MCI_SEQ_STATUS_PORT;

    memset((void *)&mciPlayParms,0,sizeof(MCI_PLAY_PARMS)); // ?????

    mciPlayParms.dwCallback = (DWORD)(HWND *) hWndNotify;
    dwReturn = mciSendCommand(wDeviceID, MCI_PLAY, MCI_NOTIFY, (DWORD)(LPVOID) &mciPlayParms);
    if (dwReturn)
    {
        mciSendCommand(wDeviceID, MCI_CLOSE, 0, NULL);
        return(dwReturn);
    }
    return(0);
}//PlayMidiFile

static DWORD PauseMidiFile(HWND /*hWndNotify*/)
{
    return 0;
}

static DWORD StopMidiFile(HWND hWnd)
{
    struct SPartWindowData * pwd = (struct SPartWindowData *) GetWindowLong(hWnd,GWL_USERDATA);
    if (pwd)
    {
        if (pwd->m_sPlay.Tempo)
        {
            KillTimer(hWnd, ID_TIMER_MIDI);

            pwd->m_sPlay.Tempo = 0;

            pwd->ViewBegin = pwd->m_sPlay.ViewBegin;
            pwd->SelectBegin = pwd->m_sPlay.SelectBegin;
            pwd->SelectEnd = pwd->m_sPlay.SelectEnd;

            RedrawWindow(hWnd,NULL,NULL,RDW_INVALIDATE | RDW_UPDATENOW);
        }
    }

    return (mciSendCommand(wDeviceID, MCI_CLOSE, 0, NULL));
}

static DWORD LoopMidiFile(char * /*lpszMIDIFileName*/, HWND /*hWndNotify*/)
{
    return 0;
}
