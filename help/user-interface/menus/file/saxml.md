---
title: Speech Analyzer XML
---

Speech Analyzer 3.0 stores transcription and annotation data in a file with a .saxml extension in the same folder as the audio file.

Extensible Markup Language (XML) is a simple, very flexible text format.

[Transcriptions](../edit/transcription/guidelines) appear on transcription bars above the plot area of [graphs](../graphs/overview).

Annotations appear on the tabs of the [File Information](information/overview) dialog box.

Manual settings appear on the [Pitch tab](../graphs/parameters/pitch-tab) of the **Graph Parameters** dialog box.

The **SaAudioDocument** element has **DocVersion** and **MD5HashCode** attributes.

The **SaAudioDocument** element contains any of the following elements.

|**Element**|**Data**|**Tab**|
| :- | :- | :- |
|SpeakerName|Name of the language speaker|[Source](information/source-tab)|
|SpeakerGender|Gender of the language speaker|Source|
|EthnologueId|Ethnologue code of the language|Source|
|Region|Region of the language|Source|
|Country|Country of the language|Source|
|Family|Linguistic family of the language|Source|
|LanguageName|Language name|Source|
|Dialect|Dialect of the language|Source|
|NoteBookReference|Reference to the researcher's data<br>for example, page and line numbers|Source|
|FreeFormTranslation|Free translation|[Comments](information/comments-tab)|
|Transcriber|Researcher who transcribed the data|Source|
|SADescription|Description of the audio file|Comments|
|SAFlags|For internal use by Speech Analyzer<br>2: Sample size changed by AudioCon<br>4: Samping frequency changed by AudioCon<br>32: Highpass filtered during recording| |
|RecordFileFormat|Original audio file format<br>0: utterance (UTT) recorded using CECIL or WinCecil, and then converted to wave file<br>1: wave file (WAV)<br>2: Macintosh<br>3: TIMIT<br>4: other|File|
|RecordTimeStamp|For internal use by Speech Analyzer| |
|RecordBandWidth|Bandwidth of the original recording (hertz)<br>Half the sampling frequency, reduced by 70 if highpass filtered by Speech Analyzer| |
|RecordSampleSize|Sample size of the original recording (bits)| |
|SignalMax|Maximum raw sample value in the audio data| |
|SignalMin|Minimum raw sample value in the audio data| |
|SignalBandWidth|Bandwidth of the audio data (hertz)<br>Can be less than the original bandwidth if the audio data was processed by AudioCon|[Data](information/data-tab)|
|SignalEffSampSize|Sample size of the audio data (bits)<br>Can be less than the original sample size if the audio data was processed by AudioCon|Data|
|CalcFreqLow|Lower bound for pitch calculations (hertz)|[Pitch](../graphs/parameters/pitch-tab)|
|CalcFreqHigh|Upper bound for pitch calculations (hertz)|Pitch|
|CalcVoicingThd|Threshold to consider the audio data voiced|Pitch|
|CalcPercntChng|Maximum change between adjacent points for the line to be continuous (percent)|Pitch|
|CalcGrpSize|Minimum number of adjacent points that can be displayed|Pitch|
|CalcIntrpGap|Maximum number of adjacent missing points that can be interpolated|Pitch|

### **Segments**
If segments have been identified in Speech Analyzer, the **Segments** element contains **SegmentData** elements.

**SegmentData** elements have the following attributes:

- **IsBookmark**: **true** if the segment has a [bookmark](../edit/transcription/add-gloss-bookmark), otherwise **false**.
- **Offset**: time at the beginning of the segment (seconds).
- **Duration**: time difference from the end to the beginning of the segment (seconds).
- **MarkDuration**: length of the [gloss segment](../edit/transcription/add-gloss-word-segment) that begins at this segment (seconds), otherwise **0**.

If [transcriptions](../edit/transcription/overview) have been entered in Speech Analyzer, **SegmentData** elements contain any of the following elements.

|**Element**|**Data**|**Tab**|
| :- | :- | :- |
|Phonetic|Phonetic transcription|[Comments](information/comments-tab)|
|Phonemic|Phonemic transcription| |
|Tone|Tone transcription| |
|Orthographic|Orthographic transcription| |
|Gloss|Gloss transcription| |
|Reference|Reference transcription<br>for example, word number| |

### **MusicSegments**
If [phrase level](../edit/transcription/add-phrase-level) segments have been identified in Speech Analyzer, the **MusicSegments** element contains **MusicSegmentData** elements.

**MusicSegmentData** elements have the following attributes:

- **PhraseLevel**: **1**, **2**, **3**, or **4**.
- **Offset**: time at the beginning of the segment (seconds).
- **Duration**: time difference from the end to the beginning of the segment (seconds).

**MusicSegmentData** elements contain an **Annotation** element. It contains the text in the transcription bar.

#### **Related Topics**
[Open](open)

[Save](save)

[Transcription Encoding Converters](transcription-encoding-converters)

[Transcriptions no longer appear](../../../troubleshooting/transcriptions-no-longer-appear)

[Export Extensible Markup Language](export/xml)