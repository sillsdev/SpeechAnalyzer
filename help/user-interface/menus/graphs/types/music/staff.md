---
title:  Staff
---

On the **Staff**, you can transcribe Western music notation corresponding to rhythm and pitch patterns. Speech Analyzer has MIDI capabilities, but it is *not* designed as a music publishing tool.

Music notation is a standardized system of writing for music, involving notes, clefs, sharps, flats, and other symbols. Speech Analyzer supports quarter-tone accidentals.

To [transcribe notes to the Staff](transcribe-notes), you can do either of the following:

- [Type the music notation](notation).
- Right-click in the **Staff**, and then select a note on the [Staff shortcut menu](staff-shortcut-menu).

At the left of the **Staff**, you can do any of the following:

|**To**|**Click**|
| :- | :- |
|Play the notes.|![](../../../../../images/012.png)|
|Play the notes and also the waveform.|![](../../../../../images/012-1.png)|
|Play only the waveform.|![](../../../../../images/012-2.png)|
|Stop the playback.|![](../../../../../images/012-3.png)|
|Select an instrument for playback.|**Voice**|
|Select a tempo for playback.|**Tempo**|
|Import notes from a MusicXML, MIDI, RTF, or text file.|**Import**|
|Export notes to a MusicXML, MIDI, RTF, or text file.|**Export**|
|Convert pitch levels in the [Melogram](melogram) to a rough, initial notation. It erases any notes on the staff before the conversion.|**Convert**|
#### ![](../../../../../images/001.png) **Note**
- Music transcription data is stored in a file with a .music.xml extension in the same folder as the audio file.
- On the [status bar](../../../tools/status-bar), the two panes display:
  - The beginning time interval.
  - The time between the [begin and end cursors](../../begin-end-cursors).
- **Convert** only works for *monophonic* recording, that is, only one singer or one instrument and no accompaniment.
- **Convert** does not create half sharps, half flats, 1 1/2 sharps and 1 1/2 flats.
- When the beginning of the data is not visible, the [clef](clefs) becomes gray.

#### **Related Topics**
[Music Graph Types overview](overview)
