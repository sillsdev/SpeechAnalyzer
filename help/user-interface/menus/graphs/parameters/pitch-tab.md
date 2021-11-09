---
title: Pitch tab, Graph Parameters
---

The **Pitch** tab appears on the [Graph Parameters](overview) dialog box, if you select the [Raw Pitch](../types/raw-pitch), [Custom Pitch](../types/custom-pitch), [Smoothed Pitch](../types/smoothed-pitch), or [Auto Pitch](../types/auto-pitch) graph types.

To change the **Pitch** tab parameters, do the following:

1. Select an [active](../active-graph) [graph type](../types/overview).
1. Do one of the following:
   1. On the [Graphs](../overview) menu, click **Parameters**.
   1. Right-click in a plot area, and then click **Parameters**.
1. Click the **Pitch** tab.
1. In the **Display** area, do any of the following:
   1. In the **Scaling** list, select **Linear**, **Logarithmic**, or **Semitones**. A semitone represents a half step on a musical scale.
   1. In the **Range list**, select **Auto** or **Manual**.

**Auto** automatically scales the graph legend to match the lowest to highest frequency present.

**Manual** activates the **Upper Boundary** and **Lower Boundary** boxes. You determine the maximum and minimum pitch values by the numbers you type or select using the up or down arrows.

**Upper Boundary** is the upper boundary of frequencies (*Y*-axis). **Lower Boundary** is the lower boundary of frequencies (*Y*-axis).

1. In the **Manual Settings** area, you can change any of the pitch contour calculation numbers, helping to eliminate pitch outliers and inaccurate voicing assessments:
   1. The *first three parameters* apply to the Raw Pitch, Custom Pitch, and Smoothed Pitch graphs.

**Min Frequency** sets the lower bound for pitch calculations.

**Max Frequency** sets the upper bound for pitch calculations.

**Voicing Threshold** sets the *required* percentage of full-scale magnitude for a portion of the recording to be able to consider the area *voiced*.

1. The *second three parameters* apply only to the Custom Pitch and Smoothed Pitch graphs.

**Max. Change** sets the maximum percentage change between adjacent frequency estimates, so the tone contour is continuous between them.

**Min Group** sets the *minimum number of consecutive pitch points* that display in the [Custom Pitch](../types/custom-pitch) and [Smoothed Pitch](../types/smoothed-pitch) graphs.

**Max Interpolation** sets the maximum number of consecutive missing frequency estimates that can be interpolated before final smoothing and display in the [Smoothed Pitch](../types/smoothed-pitch) graph (given that the estimates on either side are within a range specified by **Max Change**.)

1. To restore the last saved **Manual Settings**, click **File Defaults**.
1. To restore the Speech Analyzer **Manual Settings**, click **App Defaults**.
1. To save the settings and *not close* the dialog box, click **Apply**.
1. To save the settings and *close* the dialog box, click **OK**.

#### ![](../../../../images/001.png) **Note**
- The **Upper Boundary** and **Lower Boundary** numbers do not affect the pitch contour calculation, only the individual pitch graph display.
- **Manual Settings** allows you to change how the pitch algorithm operates on the recording. If you change the **Manual Settings**, they will change the pitch algorithm *of that particular file*.
- The **Manual Settings** area is *not available* with the [Auto Pitch graph](../types/auto-pitch).
- If the actual fundamental frequency of the audio data is below the **Min Frequency** setting, the Pitch graphs do not display a frequency value.

If the actual fundamental frequency is above the **Max Frequency** setting, the Pitch graphs might display a sub-harmonic frequency value. For example, if the actual frequency is 1200 Hz, the graphs might display a frequency of 600 Hz (1/2) or 400 Hz (1/3), depending on the setting.

- You can compare [CECIL terminology with that of Speech Analyzer](../cecil).

#### **Related Topics**
[Graph Parameters overview](overview)
