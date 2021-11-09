---
title: Export Standard Format (.SFM)
---

Speech Analyzer can export data in Standard Format so that you can open the data in other programs.

Standard Format is a format for text files that contain language and culture data.

A field is a unit of data in a Standard Format file that consists of a marker and data.

Depending on the check boxes that you selected in the [Export Speech Format](sfm-xml) dialog box, the exported file can contain some or all of the following fields.

[Transcriptions](../../edit/transcription/guidelines) appear on transcription bars above the plot area of [graphs](../../graphs/overview).

Other data appear on the tabs of the [File Information](../information/overview) dialog box.

|**Marker**|**Data**|**Tab**|
| :- | :- | :- |
|\name|Standard Format file path and name| |
|\date|Export time (day, month, year, time)| |
|\wav|Audio file path and name|File|
|\ref|Reference transcription<br>for example, word number| |
|\ph|Phonetic transcription|[Comments](../information/comments-tab)|
|\tn|Tone transcription| |
|\pm|Phonemic transcription| |
|\or|Orthographic transcription| |
|\gl|Gloss transcription| |
|\pos|Part of speech<br>not used in Speech Analyzer 3.0| |
|\phr1|Phrase Level 1 transcription| |
|\phr2|Phrase Level 2 transcription| |
|\phr3|Phrase Level 3 transcription| |
|\phr4|Phrase Level 4 transcription| |
|\ft|Free translation|[Comments](../information/comments-tab)|
|\np|Number of phones in the transcription|[Data](../information/data-tab)|
|\nw|Number of words in the transcription|Data|
|\ct|Audio file created (day, month, year, time)|[File](../information/overview)|
|\le|Audio file last edited (day, month, year, time)|File|
|\size|Audio file size (bytes)|File|
|\of|Original audio file format<br>**wave file (WAV)<br>utterance (UTT)** recorded using CECIL or WinCecil, and then converted to wave file|File|
|\samp|Number of samples|[Data](../information/data-tab)|
|\len|Length of the audio data (seconds)|Data|
|\freq|Sampling frequency (hertz)|Data|
|\bw|Bandwidth of the original recording (hertz)<br>\*Bandwidth of the audio data can be less than the original bandwidth if the audio data was processed by AudioCon|Data\*|
|\hpf|Highpass filtered during recording (**Yes** or **No**)|Data|
|\bits|Storage format of the file (bits)|Data|
|\qsize|Sample size of the original recording (bits)<br>\*Sample size of the audio data can be less than the original sample size if the audio data was processed by AudioCon|Data\*|
|\ln|Language name|[Source](../information/source-tab)|
|\dlct|Dialect of the language|Source|
|\fam|Linguistic family of the language|Source|
|\id|Ethnologue code of the language|Source|
|\cnt|Country of the language|Source|
|\reg|Region of the language|Source|
|\spkr|Name of the language speaker|Source|
|\gen|Gender of the language speaker|Source|
|\nbr|Reference to the researcher's data<br>for example, page and line numbers|Source|
|\tr|Researcher who transcribed the data|Source|
|\desc|Description of the audio file|[Comments](../information/comments-tab)|

#### **Related Topics**
[Standard Format or Extensible Markup](sfm-xml)