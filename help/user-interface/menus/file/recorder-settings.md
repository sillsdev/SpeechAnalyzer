---
title: Recorder Settings
---

When you click **Settings** in the [Recorder](record-new) dialog box, the **Recording Settings** dialog box appears. A quantization size of 16 bits and a sampling frequency of 22 kHz give excellent results for recording speech. A sampling frequency of 44.1 KHz is good for music.

To adjust the recorder settings, do any of the following:

1. In the **Sampling Rate** area, select a rate for subsequent recordings.

    The **Sampling Rate** determines the sampling speed of the incoming audio.

1. In the **Data** area, select a precision and stereo-mono attribute for subsequent recordings.

    Precision determines the number of bits used to encode each audio recording sample.

1. To reduce low-frequency hum in your recordings (frequencies below 70 Hz), select **70 Hz** [Highpass filter](../tools/workbench-filter-selections).
1. To retain file source information for subsequent recordings, select **Add File Information**.
1. To reset the default, click **Default**.

    The default values are a **Sampling Rate** of 22,050Hz and **Data** of 16 bit mono.

1. To add file information for the recording, click **File Info**, and select the [Source tab](information/source-tab) and [Comments tab](information/comments-tab).
1. Click **OK** to save settings, or **Cancel** to exit *without saving the settings*.

#### ![](../../../images/001.png) **Note**
- If you select **Add File Information**, the information you entered in **File Info** is saved in the **Source** tab and **Comments** tab on the [File Information](information/overview) dialog box. 
- To view **Sampling Rate** precision and file information, on the **File Information** dialog box, click the [Data tab](information/data-tab).
- Speech Analyzer *performs better* with higher sampling rates and greater precision. However, higher sampling rates and greater precision also uses *more hard disk space* for the recording.

#### **Related Topics**
[Record New](record-new)