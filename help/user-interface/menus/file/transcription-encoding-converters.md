---
title: Transcription Encoding Converters
---

When you [open](open) a file that contains [transcription](../edit/transcription/guidelines) data from a *previous version* of Speech Analyzer, the **Transcription Encoding Converters** dialog box appears.

For each transcription type, the **Legacy Font** column displays the font used in a previous version of Speech Analyzer.

To change the encoding converter for a transcription type, do the following:

1. In the **Transcription** column, select a transcription type (for example, **Phonetic**).

An arrow ![](../../../images/004.png) appears at the left of a selected transcription type, and a down arrow ![](../../../images/005-1.png) appears in the **Converter** column.

1. Click the down arrow in the **Converter** column.
1. In the list, select **none**, an encoding converter, or **Browse**.
   1. If you select **Browse**, the **Find Converter Mapping File** dialog box appears.
   1. Find the converter mapping file using the **Look in** folder list, and click **Open**.

The converter mapping file name appears in the **Converter** list.

1. To change the encoding converter for another transcription type, repeat steps 1, 2, and 3.

To see all the transcription types, you might need to scroll down.

1. When you have selected the encoding converters you want, click **OK**.

Speech Analyzer converts the transcription data to Unicode using the selected converters and stores the converted data in a file with a [.saxml](saxml) extension in the same folder as the wave file.

It backs up the wave file with a .sa2.wav extension in a SA2 Wav Files folder, removes the transcription data from the wave file, and then opens the file.

If you click **Cancel**, Speech Analyzer removes the transcription data from the wave file, but does not create a .saxml file.

#### **Related Topics**
[Start Mode](../../start-mode)