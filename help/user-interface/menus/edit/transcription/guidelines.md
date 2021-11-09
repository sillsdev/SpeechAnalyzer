---
title: Transcription guidelines
---

A *transcription* is a text that is time-aligned with the audio data. An *annotation* is an explanatory comment added to help understand the wave file. Annotations can be found on the [Comments](../../file/information/comments-tab) and [Source](../../file/information/source-tab) tabs of the **File Information** dialog box. The transcription categories, **Reference**, **Phonetic**, **Phonemic**, **Tone**, **Orthographic**, and **Gloss**, appear as bars above the graph. **Phrase Level** transcription is used for music analysis in several hierarchical levels, **L1** being the highest, **L4** the lowest.

You can edit transcription characters using [Direct Edit](../direct-edit) or [Transcription Editor](editor).

![](../../../../images/007.png)

The following are some basic guideline for transcriptions:

- Before you transcribe, you may need to [resize the phonetic segment](../segment-resizing), because some segments take their width from the **Phonetic** segment.
- To display a transcription bar, do one of the following:
  - On the [Graphs](../../graphs/overview) menu, point to **Transcription Bars**, point to a transcription type, and then select a graph category (such as **All Graphs**).

    A transcription bar appears above *every* graph category selected.

- Right-click in any graph display area, point to **Transcription Bars** on the [shortcut menu](../../graphs/shortcut), and then select a transcription type (such as **Gloss**).

A transcription bar appears above the specific graph in which you right-clicked.

### **Notes for transcription types**
- **Phonetic** - transcribe segments with [IPA Characters](../character-chart).
  - [Add phonetic segments](add-phonetic-segment) before you complete any other transcription, because this determines the width of other segments. You can also [resize the segment](../segment-resizing).
- **Phonemic** - transcribe *after* phonetic segmentation of the recording.
- **Tone** - transcribe *after* phonetic segmentation of the recording.
  - To ensure the **Tone** transcription bar *is always above the other transcription bars*, click **Tone Above** on the [View tab](../../tools/options/view-tab) of the [Options dialog box](../../tools/options/overview).
- **Orthographic** - transcribe *after* phonetic segmentation of the recording.
- **Gloss** - complete phonetic segmentation of the recording *is not required* before transcription.
  - Presently, Gloss segments can only begin where a **Phonetic** segment is marked. *It can be empty*.
  - You can still insert [word breaks](word-break) or [bookmarks](add-gloss-bookmark) even if the gloss is unknown.
- **Reference** - enter field translation notebook word numbers. Transcribe only after a **Gloss** segment is added. If a segment is added to **Gloss**, it is automatically added to **Reference**.

#### ![](../../../../images/001.png) **Note**
- Once a new **Phonetic** segment is added, new **Phonemic**, **Tone**, and **Orthographic** segments are *automatically* added, with the same width.
- To enter a field translation notebook word number for each word segment of the recording:

    On the [File](../../file/overview) menu, click [Information](../../file/information/overview). Click the [Source tab](../../file/information/source-tab). In the **Notebook** area, type in the **Reference** text box, and then click **OK**.

#### **Related Topics**
[Transcription Editor](editor)