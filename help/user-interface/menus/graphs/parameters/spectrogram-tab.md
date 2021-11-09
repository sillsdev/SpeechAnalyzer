---
title: Spectrogram tab, Graph Parameters
---

The **Spectrogram** tab appears on the [Graph Parameters](overview) dialog box, if you select a [Spectrogram graph](../types/spectrogram). The **Snapshot** tab appears on the **Graph Parameters** dialog box if you select a [Snapshot Spectrogram graph](../types/snapshot-spectrogram). Both tabs are identical.

To change the **Spectrogram** or **Snapshot** tab parameters, do the following:

1. Select an [active](../active-graph) [graph type](../types/overview).
1. Do one of the following:
   1. On the [Graphs](../overview) menu, click **Parameters.**
   1. Right-click in a plot area, and then click **Parameters**.
1. Click the **Spectrogram** or **Snapshot** tab.
1. To set the upper frequency for calculation and display, drag the **Display Frequency** slider.

Since most vowels have a third [formant](../types/formants) (**F3**) around 2500 Hz, the default value is 3000 Hz. Use a value of 7500 in the analysis of consonants

1. To set the lowest and highest value displayed, drag one **Thresholds** slider to the lowest setting, and the other **Thresholds** slider to the highest setting.
1. To change any formant selection, select the **Formants Tracks** check box, and then do any of the following:
   1. To display the pitch track, select the **Pitch** check box.
   1. To display *specific* formant tracks, select the **F1**, **F2**, **F3**, or **F4** check boxes.
   1. To display the formant tracks overlaid on the Spectrogram, select [Overlaid](formant-tracks-overlaid).
   1. To display only the formant tracks, select [Only](formant-tracks-only).
   1. To display a smoother graph line, select the **Smoothed** check box.
   1. To display only the *formant tracks in color*, select the **Color** check box.
1. To display the *Spectrogram in color*, in the **Color mode** area, click **Color**. To display the Spectrogram in black and white, click **Monochrome**.
1. You can adjust the [Spectral Resolution and see examples](spectrogram-tab-examples).
1. To save the settings and *not close* the dialog box, click **Apply**.
1. To save the settings and *close* the dialog box, click **OK.**

#### ![](../../../../images/001.png) **Note**
- All values lower than the lowest **Threshold**, display the same color as the lowest value.
- All values higher than the highest **Threshold**, display the same color as the highest value.
- When you change any **Formant Tracks** settings, the **Apply** button activates.
- Formant tracks in a **Spectrogram** graph are calculated using a bank of 1st order LPC tracking filters based on:

Mustafa, Kamran, and Ian C. Bruce, "Robust Formant Tracking for Continuous Speech with Speaker Variability," *IEEE Transactions on Audio, Speech, and Language Processing*, Vol. 14, No. 2, March 2006.

- Formant estimates in a **Spectrum** graph are calculated using an LPC model whose order is based on a sampling frequency (Fs / 1000 + 2).

#### **Related Topics**
[Graph Parameters overview](overview)
