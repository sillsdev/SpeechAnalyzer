---
title: Segment resizing details
---

An audio file can contain gaps in the segmentation. This may occur if the recording contains periods of silence or background noise. Speech Analyzer does not require [segments](segment) to be touching.

### **To eliminate a segment boundary gap**
1. On the **Graphs** menu, point to [View](../graphs/view), point to **Segment Boundaries**, and then select the graphs.

    Segment boundaries display on the Raw Waveform graph.

1. On the **Edit** menu, select [Segment Boundaries](segment-boundaries) or press ![](../../../images/060.png) on the toolbar.
1. Select a **Phonetic** segment, next to the segment boundary gap.
1. Drag the begin or end cursor toward the segment boundary gap.

   The segment boundary gap disappears.

### **To create a segment boundary gap**
- A Phonetic segment is required before adding Phonemic and Orthographic segments.
- Phonemic and Orthographic segments *automatically* align with Phonetic segments.
- Gloss always starts on a phonetic segment, but may span multiple phonetic segments.
- Phrase Level segment boundaries are independent of all others.

To resize segments whose widths are dependent upon the phonetic segment width:

1. [Select the phonetic segment](select-segment) to place the gap.
1. Place the [begin and end cursors](../graphs/begin-end-cursors) at the border of the phonetic segment to resize.
1. Hold down **Ctrl + Shift,** and then do one of the following:
   * Drag the begin cursor to the right to make a gap to the left of this segment.
   * Drag the end cursor to the left to make a gap to the right of this segment.
1. Insert a Gloss [word break](transcription/add-gloss-word-segment) character where you wish the gap to begin.

An empty segment will span the gap but there will be no text inside.

### **To overlap phonetic sounds (coarticulation)**
*Coarticulation* is the influence successive speech sounds exert on each other, especially on the boundary between sounds. Speech Analyzer allows up to 40% overlap.

1. Click the **Phonetic** segment you wish to coarticulate.
1. Hold down **Ctrl + Shift**, and do one of the following:
   * Drag the end cursor to the right.
   * Drag the begin cursor to the left.

#### ![](../../../images/001.png) **Note**
- Always release the mouse button *before* you release **Ctrl + Shift**.

#### **Related Topics**
[Edit overview](overview)