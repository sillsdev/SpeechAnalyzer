---
title: Export Extensible Markup Language
---

Speech Analyzer can export data in Extensible Markup Language (XML) so that you can open the data in other programs.

Depending on the check boxes that you selected in the [Export Speech Format](sfm-xml) dialog box, the exported file can contain some or all of the following elements.

[Transcriptions](../../edit/transcription/guidelines) appear on transcription bars above the plot area of [graphs](../../graphs/overview).

Other data appear on the tabs of the [File Information](../information/overview) dialog box.

### **FileInfo**

|**Element**|**Data**|**Tab**|
| :- | :- | :- |
|Comments|Description of the audio file|[Comments](../information/comments-tab)|
|AudioFile|Audio file path and name|[File](../information/overview)|
|CreationTime|Audio file created (day, month, year, time)|File|
|LastEdit|Audio file last edited (day, month, year, time)|File|
|FileSize|Audio file size (bytes)|File|
|OrigFormat|Original audio file format<br>**wave file (WAV)<br>utterance (UTT)** recorded using CECIL or WinCecil, and then converted to wave file|File|

### **LinguisticInfo**

|**Element**|**Data**|**Tab**|
| :- | :- | :- |
|PhoneticText|Phonetic transcription|[Comments](../information/comments-tab)|
|Tone|Tone transcription| |
|PhonemicText|Phonemic transcription| |
|Orthographic|Orthographic transcription| |
|Gloss|Gloss transcription| |
|PartOfSpeech|Part of speech<br>not used in Speech Analyzer 3.0| |
|Reference|Reference transcription<br>for example, word number| |
|PhraseLevel1|Phrase Level 1 transcription| |
|PhraseLevel2|Phrase Level 2 transcription| |
|PhraseLevel3|Phrase Level 3 transcription| |
|PhraseLevel4|Phrase Level 4 transcription| |
|FreeTranslation|Free translation|[Comments](../information/comments-tab)|
|NumPhones|Number of phones in the transcription|[Data](../information/data-tab)|
|NumWords|Number of words in the transcription|Data|

### **DataInfo**

|**Element**|**Data**|**Tab**|
| :- | :- | :- |
|NumSamples|Number of samples|[Data](../information/data-tab)|
|Length|Length of the audio data (seconds)|Data|
|SamplingFreq|Sampling frequency (hertz)|Data|
|Bandwidth|Bandwidth of the original recording (hertz)<br>\*Bandwidth of the audio data can be less than the original bandwidth if the audio data was processed by AudioCon|Data\*|
|HighPassFiltered|Highpass filtered during recording (**Yes** or **No**)|Data|
|StorageFormat|Storage format of the file (bits)|Data|
|QuantizSize|Sample size of the original recording (bits)<br>\*Sample size of the audio data can be less than the original sample size if the audio data was processed by AudioCon|Data\*|

### **SourceInfo**

|**Element**|**Data**|**Tab**|
| :- | :- | :- |
|Language|Language name|[Source](../information/source-tab)|
|Dialect|Dialect of the language|Source|
|Family|Linguistic family of the language|Source|
|Country|Country of the language|Source|
|Region|Region of the language|Source|
|EthnologueID|Ethnologue code of the language|Source|
|SpeakerName|Name of the language speaker|Source|
|Gender|Gender of the language speaker|Source|
|NotebookReference|Reference to the researcher's data<br>for example, page and line numbers|Source|
|Transcriber|Researcher who transcribed the data|Source|

#### **Related Topics**
[Standard Format or Extensible Markup](sfm-xml)

[Speech Analyzer XML](../saxml)