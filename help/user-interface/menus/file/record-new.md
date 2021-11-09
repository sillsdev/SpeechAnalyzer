---
title: Record New
---

In Speech Analyzer, you can record [audio input](audio-input-devices) from a microphone or an external recording device.

1. Do one of the following:
   1. On the [File](overview) menu, click **Record New**.
   1. On the [toolbar](../../toolbar/toolbar), click **Record** ![](../../../images/050.png).
   1. Press the [shortcut key](../../shortcuts/menu) **Ctrl+R**.

    The **Recorder** dialog box appears.

1. Do any of the following:
   1. To adjust audio input volume, use the **Rec Vol** slider or arrows, or click [Input](recording-control).

      To control distortion, avoid red **Record Level** bars while recording.

    1. To change recorder settings, click [Settings](recorder-settings).
1. To record, click **Rec**.

    The **Cur** box displays the running elapsed time of the recording.

1. If you need to stop recording *temporarily*, click **Pause**. Click **Pause** again to continue recording.
1. When you have finished recording, click **Stop**.

    The **End** box displays the duration of the recording in minutes and seconds.

1. To verify the quality of the recording, click **Play**.

    To adjust the playback volume, use the **Play Vol** slider or arrows.

1. If the recording is unsatisfactory, repeat steps 3 through 6.
1. When you are satisfied with the recording quality, click **Done**.

    Until you click **Done**, each new recording replaces the previous recording.

    The file window displays the new recording in one or more graphs.

1. If the [Raw Waveform graph](../graphs/types/raw-waveform) displays clipping at the top and bottom of the waveform, [close](close) the file, and then repeat steps 1 through 8.
1. To save the file, on the **File** menu, click [Save](save).

#### ![](../../../images/001.png) **Note**
- Recordings with waveform inversion, low signal level, or sound card offset problems can be corrected using the [Adjust Waveform](../tools/adjust-waveform) command on the **Tools** menu.
- Although recordings can be up to 4 GB, recordings of 2 or 3 minutes are suggested. A file recorded at 22 kHz [sampling rate](recorder-settings) with 16 bit samples will require about 2.6 MB per minute recorded.
- *If you need to use the slow Playback function*, keep recordings under ten seconds.

#### **Related Topics**
[File overview](overview)