---
title: Status Bar
---

The status bar displays information about the [active graph](../graphs/active-graph) at the bottom of a Speech Analyzer window. Hertz is a unit of frequency. One *hertz* equals one cycle per second. A *semitone* is a unit of frequency indicating the number of half-steps between a given frequency and a reference frequency. *Pitch* is the perceived fundamental frequency of a sound. While the actual fundamental frequency can be precisely determined through physical measurement, it may differ from the perceived pitch because of overtones, or partials, in the sound

*As the active graph changes, the status bar configuration also changes*.

- To turn the status bar on and off, do the following:
  - On the **Tools** menu, click **Options**, and then click the [View tab](options/view-tab).
  - In the **Mainframe** area, select or clear the **Statusbar** check box.
- Depending on the active graph, you may see any of the following information:

|**Status Bar icon**|**Means**|
| :- | :- |
|![](../../../images/025.png)|Displays the time interval in (seconds, samples, or bytes) from the *beginning* of the file to the begin cursor. 0:1.4391 means 0 minutes and 1.4391 seconds|
|![](../../../images/026.png)|Displays the time interval *between* the [begin and end cursor](../graphs/begin-end-cursors) in (seconds, samples, or bytes). 0:1.0540 means 1.0540 seconds|
|![](../../../images/027.png)|Displays the *pitch* in [semitones](../graphs/parameters/pitch-tab) (st). For the [Melogram](../graphs/types/music/melogram) and Tonal Weighting Chart, the closest musical note displays in parentheses.|
|![](../../../images/028.png)|Displays the *pitch* in hertz (Hz). For the Tonal Weighting Chart, the percentage of sound heard at this frequency displays in parentheses.|
|![](../../../images/029.png)|<p>Displays the [relative intensity](../graphs/parameters/intensity-tab) in decibels (dB) at the begin cursor.</p><p>The next status bar pane to the right displays the [linear magnitude](../graphs/parameters/intensity-tab) in percent.</p>|
|L|Displays the height or *amplitude* at the begin cursor. The percentage shows the height compared to full scale. The value in parentheses show equivalent the numeric value.|
|R|Displays the height or *amplitude* at the end cursor. The percentage shows the height compared to full scale. The value in parentheses show equivalent the numeric value|
|F1|Displays the *frequency* of [formant](../graphs/types/formants) 1 in hertz (Hz).|
|F2|Displays the *frequency* of formant 2 in hertz (Hz).|
|F3|Displays the *frequency* of formant 3 in hertz (Hz).|
|F4|Displays the *frequency* of formant 4 in hertz (Hz).|
|[Tonal Weighting Chart](../graphs/types/music/tonal-weighting-chart) Pane 1|The pitch of the *0* line in semitones (st) and hertz (Hz).|
|Tonal Weighting Chart Pane 2|The difference in pitch in semitones (st) and hertz (Hz) between the private cursor and the zero line.|
|[Spectrogram](../graphs/types/spectrogram) & [Snapshot](../graphs/types/snapshot-spectrogram) Pane 3|Displays the frequency in hertz (Hz) at the location of the (hand) mouse pointer.|
|Spectrogram & Snapshot Pane 4|Displays the intensity in decibels (dB) at the location of the (hand) mouse pointer.|
|[Change](../graphs/types/change) Pane 3|The degree of change (*X* axis) at the begin cursor.|
|Change Pane 4|The degree of change (*X* axis) at the end cursor.|
|[Zero Crossing](../graphs/types/zero-crossing) Pane 3|The number of zero crossings (*X* axis) at the begin cursor.|
|Zero Crossing Pane 4|The number of zero crossings (*X* axis) at the end cursor.|
|[Spectrum](../graphs/types/spectrum) Pane 1|Displays the frequency in hertz (Hz) at the private cursor.|
|Cep|Displays the value of the Cepstral plot in the [Spectrum graph](../graphs/types/spectrum) at the private cursor.|
|Raw|Displays the value of the Raw plot in the Spectrum graph at the private cursor.|
|LPC|Displays the value of the LPC plot in the Spectrum graph at the private cursor.|

#### ![](../../../images/001.png) **Note**
- To switch the beginning time interval to seconds, samples, or bytes, on the **Tools** menu, click **Options**, and then select the [View tab](options/view-tab).

#### **Related Topics**
[Tools overview](overview)
