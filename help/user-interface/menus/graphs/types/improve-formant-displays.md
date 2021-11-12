---
title: Improve formant displays
---

### **Formant Charts**
If the [formant](formants) results appear to incorrect, do the following:

1. Examine the begin cursor position in the [Raw Waveform graph](raw-waveform).
   * Make sure that the cursor is not in a region of coarticulation, unless that is what you intended to analyze.

*Coarticulation* is the influence successive speech sounds exert on each other, especially on the boundary between sounds.

1. Make sure that the fragments near the cursor are very similar.
1. If needed, move the begin cursor to a different area of the phonetic [segment](../../edit/segment).

### **Spectrum graph**
If F1 and F2 are indistinct in the LPC plot line, do the following:

1. On the **Graph Parameters** dialog box, click the [Spectrum tab](../parameters/spectrum-tab).
1. In the **Calculations** area, increase the [Peak Sharpening Factor](../parameters/peak-sharpening) to 3, and then click **Apply**.
1. Examine the **Spectrum** graph to see if the formant peaks become distinct.
1. If more peaks do not appear, increase the **Peak Sharpening Factor** to 5, and then click **Apply**.

#### **Related Topics**
[Formant Charts](formant-charts)

[Spectrum graph](spectrum)
