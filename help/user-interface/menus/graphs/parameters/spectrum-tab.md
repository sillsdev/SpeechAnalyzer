---
title: Spectrum tab, Graph Parameters
---

The **Spectrum** tab appears on the [Graph Parameters](overview) dialog box, if you select a [Spectrum graph](../types/spectrum).

To change the **Spectrum** tab parameters, do the following:

1. Select an [active](../active-graph) [graph type](../types/overview).
1. Do one of the following:
   * On the [Graphs](../overview) menu, click **Parameters**.
   * Right-click in a plot area, and then click **Parameters**.
1. Click the **Spectrum** tab.
1. In the **Power Scale** area, type or use the arrows to set the **Upper Boundary** and **Lower Boundary** levels displayed on the **Spectrum** graph.
1. In the **Frequency Range** area, select **Full Scale**, **Half Scale**, **Third Scale**, or **Quarter Scale**.

(**Full Scale** displays the spectrum from 0 to half the sampling rate.)

1. In the **Calculations** area, select one of the following:
   * Select **Perform LPC Smoothing**.

The [Peak Sharpening Factor](peak-sharpening) list activates. Type or select numbers using the arrows.

1. Select **Perform Cepstral Smoothing**.

The [Smoothing Level](cepstral-smoothing) list activates. Type or select numbers using the arrows.

1. In the **Formant Display** area, do any of the following:
   * To display formant frequencies in hertz (Hz), select the **Frequency** check box.

The **Bandwidth** check box activates. Select this box to display formant bandwidths in hertz (Hz).

1. To display peak values in decibels (dB), select the **Power** check box.
1. To save the settings and *not close* the dialog box, click **Apply**.
1. To save the settings and *close* the dialog box, click **OK**.

#### ![](../../../../images/001.png) **Note**
- To view the bandwidth for the file, on the [File](../../file/overview) menu, click **Information**, and then click the [Data tab](../../file/information/data-tab).
- Formant estimates in a **Spectrum** graph are calculated using an LPC model whose order is based on a sampling frequency (Fs / 1000 + 2).

#### **Related Topics**
[Graph Parameters overview](overview)
