---
title: Import
---

You can import transcriptions or music notations. The **Import** command appears on the [File](overview) menu only if an existing file is open in Speech Analyzer. You *cannot import* an [SFM time table](export/sfm-time-table).

### **Import Standard Format (.sfm)**
1. On the **File** menu, point to **Import**, and then click **Standard Format (.SFM)**.

    The **Open** dialog box appears.

1. If needed, in the **Look in** list, browse to a *different folder* to import.
1. Click the file in the list that contains the information to import.
1. Click **Open**.

    Speech Analyzer imports the data from the specified file.

1. The **Result** dialog box appears showing the imported data with standard format markers.
1. Click **Open**.

### **Import Staff Notes (.xml, .mid, .txt)**
To import Staff Notes, you must display the active file on the [Staff](../graphs/types/music/staff).

1. Do one of the following:
   1. On the **File** menu, point to **Import** and then click **Staff Notes**.
   1. Right-click the **Staff**, and then click **Import from file**.
   1. On the **Staff**, click **Import**.

    The **Open** dialog box appears.

1. If needed, in the **Look in** list, browse to a *different folder* to import.
1. In the **Files of type** box, do one of the following:
   1. To import from a MusicXML, select **MusicXML file (\*.xml)**.

        **MusicXML** is an open, XML-based music notation file format. This is a richer format than MIDI. Many music notation packages, like Sibelius and Finale, support this interchange format.

    1. To import from a MIDI file, select **MIDI file (\*.mid)**.

        **MIDI** stands for Musical Instrument Digital Interface. It is an electronic standard used for the transmission of digitally encoded music.

    1. To import from a text file, select **Text file (\*.txt)**.

      The files of that type appear in the current folder.

1. Click the file in the list that contains the information to import.
1. Click **Open**.

    Speech Analyzer imports the data to the **Staff**.

#### ![](../../../images/001.png) **Note**
- You can only import .sfm or text files that have been previously [exported](export/overview) by Speech Analyzer.

    When importing files, Speech Analyzer ignores the Reference, Tone, and Phrase Level transcriptions.

- Speech Analyzer imports only the *top line* of MIDI or MusicXML files that are polyphonic or multi-track.

#### **Related Topics**
[File overview](overview)