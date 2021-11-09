---
title: Open
---

1. To open an existing recording, do one of the following:
   1. On the **File** menu, click **Open**.
   1. On the [toolbar](../../toolbar/toolbar), click **Open** ![](../../../images/051.png).
   1. Press the [shortcut key](../../shortcuts/overview) **Ctrl+O**.

    The **Open** dialog appears.

1. In the **Look in** box, select the desired folder.
1. In the **Files of type** box, select the desired file type.

    The dialog box displays the available files of that type in the current folder.

1. Select the file you wish to open from the list.
1. To hear the recorded sound, click **Play**.
1. To open the file, click **Open**.
1. If a file needs to have segment boundaries reset, a message displays saying they are improperly set and asking if you wish to readjust. Click **Yes**.
1. If the selected file contains *transcription data from a previous version* of Speech Analyzer, use the [Transcription Encoding Converters](transcription-encoding-converters) dialog box that appears to select mapping converters.

    Speech Analyzer opens the selected file in a new window.

#### ![](../../../images/000.png) **Important**
- Transcription and annotation data is stored in a file with a [.saxml](saxml) extension in the same folder as the audio file.
- Music transcription data on the [Staff](../graphs/types/music/staff) is stored in a file with a .music.xml extension in the same folder as the audio file.
- If you open a read-only audio file, the title bar displays **(Read-only)** after the file name.
- Only .wav files stored in PCM format with mono recording in 8-bit or 16-bit sample size can be opened by Speech Analyzer. If you attempt to open .wav files in other formats, the program notifies you of the problem, and does *not* open the file.
- Speech Analyzer *cannot* open CECIL utterance files (with the .utt extension), but you can use the [AudioCon program](../tools/audio-converter) to convert them to .wav file format.
- All segmented files readjust when opened to set phonetic [segment boundaries](../edit/segment-boundaries) to the nearest [fragment](../graphs/fragment) (pitch period).

#### ![](../../../images/001.png) **Tip**
- You can open more than one file in Speech Analyzer. You can display a file in one or more [graphs](../graphs/overview).
- You can [register Speech Analyzer to open Wave Sound files](register) when you double-click them in Windows Explorer.

#### **Related Topics**
[File overview](overview)