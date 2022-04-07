---
title: Export Time Table (.SFT)
---

When you export an SFM Time Table, you create a file containing numeric data which may be time-aligned to a Phonetic segment or to a user-selected time frame.

- Here is a description of all the fields on the output file.
  - **\\name** - Exported file name and path (from the [File tab](../information/overview)).
  - **\\date** - Day, month, year, time of export.
  - **\\wav** - Wav file path and name which was exported.
  - **\\calc** - calculations either averaged or mid-point values.
  - **\\table** - table data.
- All the data for a *particular segment* at a *specific time* displays horizontally. Column headings identify each type of data. For example:

|Start|Length|Ref|Etic|Tone|Emic|Ortho|Gloss|Pos|MagdB|Pitch(Hz)|
| :- | :- | :- | :- | :- | :- | :- | :- | :- | :- | :- |
- The following is the explanation for each column heading:
  - **Start** - Start time of each [segment](../../edit/segment) or frame.
  - **Length** - Length in time of this segment or frame.
  - **Ref** - the number of [transcription](../../edit/transcription/overview) windows.
  - **Etic** - the Phonetic transcription.
  - **Tone** - the Tone transcription.
  - **Emic** - the Phonemic transcription.
  - **Ortho** - the Orthographic transcription.
  - **Gloss** -the Gloss transcription.
  - **POS** - *Part of Speech* from Phonology Assistant.
  - **Int(dB)** - from the [Magnitude graph](../../graphs/types/music/magnitude).
  - **Pitch(Hz)** - from the [Auto Pitch graph](../../graphs/types/auto-pitch), in 2 columns.
  - **RawPitch** - from the [Raw Pitch graph](../../graphs/types/raw-pitch).
  - **SmPitch** - from the [Smoothed Pitch graph](../../graphs/types/smoothed-pitch).
  - **Melogram(st)** - Pitch in semitones from [Melogram](../../graphs/types/music/melogram) (during music analysis), in 2 columns.
  - **ZCross** - Crossings from the [Zero Crossing graph](../../graphs/types/zero-crossing).
  - **Fx(Hz)** - the hertz of each [formant](../../graphs/types/formants) (**X** = F1 to F4).

#### **Related Topics**
[SFM Time Table](sfm-time-table)