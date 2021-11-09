---
title: Data tab, File Information
---

On the [File Information](overview) dialog box, the **Data** tab displays technical information about the active file.

|**Information type**|**Description**|
| :- | :- |
|**Number of Samples**|This is the total number of digitized samples taken from the sound recording. A *sample* is a value taken from an audio signal at a precise point in time. The sequence of these samples graphed over time is the waveform (or raw waveform).|
|**Length**|This is the total time of the recording in seconds.|
|**Sampling Frequency**|<p>The sampling frequency is the number of samples taken from an audio signal each second. It is also sometimes referred to as the sampling rate.</p><p>Higher sampling frequency often results in better quality recordings, at the cost of larger audio files</p>|
|**Bandwidth**|<p>The bandwidth is the difference between the highest and lowest frequencies a recording contains. This is usually half the sampling frequency, unless a filtering or resampling operation has been applied. The higher the bandwidth, the better recording quality. See also **Storage Format** and **Quantization Size** which affect the quality.</p><p>For files that may have been converted from CECIL (.utt) files to Windows (.wav) files, the bandwidth can never be greater than half the original **Sampling Frequency**.</p>|
|**Highpass Filtered**|<p>This indicates whether or not the 70 Hz highpass filter was applied during recording. The filter is selected in the [Recorder Settings](../recorder-settings) dialog box. </p><p>A *highpass filter* is a filter that passes high frequencies well, but attenuates (or reduces) frequencies lower than the cutoff frequency. This helps eliminate low-frequency hum or tape recorder rumble sounds.</p>|
|**Storage Format**|This value shows the precision of the recorded data.|
|**Quantization Size**|<p>This is the original number of bits used to encode the amplitude of the sound recording at each digital sample point. A greater number of bits yields greater accuracy and quality. Files that were originally a CECIL (.utt) file will have an 8-bit value. Even though [AudioCon](../../tools/audio-converter) is able to convert them to a Windows (.wav) format and 16 bits, *the quality will still be no better than the original 8-bit recording*.</p><p>The *Quantization parameter* was formerly known as *Effective Sample Size*.</p>|
|**Number of Phones**|This is the count of [Phonetic Segments](../../edit/transcription/guidelines). This includes blank Phonetic [Segments](../../edit/segment) created by adding [Syllable Breaks](../../edit/transcription/add-syllable-segment) or other symbols.|
|**Number of Words**|This is a count of the Gloss Segments (that is, the number of **#** [Word Break](../../edit/transcription/add-gloss-word-segment) symbols).|

#### ![](../../../../images/001.png) **Note**
- There is more information about **Sampling Frequency**, **Highpass Filtering**, and **Storage Format** in the [Recorder Settings](../recorder-settings).
- If a sound file was originally captured by a system with a fewer number of bits, even though it is stored using a more accurate system, it can never be better than the original file. For example, if a CECIL (.utt) file was originally recorded using 8 bits, then converted with [AudioCon](../../tools/audio-converter) at 16 bits to the current .wav file, the quality can never be better than the original 8-bit sound, which is the **Quantization**.

#### **Related Topics**
[Information overview](overview)