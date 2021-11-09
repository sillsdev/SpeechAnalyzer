---
title: Music tab, Graph Parameters
---

The **Music** tab appears on the [Graph Parameters](overview) dialog box, if you select a [Melogram](../types/music/melogram).

To change the **Music** tab parameters, do the following

1. Select an [active](../active-graph) [graph type](../types/overview).
1. Do one of the following:
   1. On the [Graphs](../overview) menu, click **Parameters**.
   1. Right-click in a plot area, and then click **Parameters**.
1. Click the **Music** tab.
1. In the **Display** area or **Calculation** area, in the **Range** list, select either **Auto** or **Manual**.

**Manual** activates the **Upper Boundary** and **Lower Boundary** lists. Select the boundary ranges by typing or selecting numbers using the up or down arrows.

1. To include only the portion of audio data between the cursors in the Tonal Weighting Chart, select the **Dynamic Calculation** check box.
1. To save the settings and *not close* the dialog box, click **Apply**.
1. To save the settings and *close* the dialog box, click **OK**.

#### ![](../../../../images/001.png) **Note**
- If the actual fundamental frequency of the audio data is below the **Lower Boundary** setting, the Melogram does not display a frequency value.

If the actual fundamental frequency is above the **Upper Boundary** setting, the Melogram might display a sub-harmonic frequency value. For example, if the actual frequency is 1200 Hz, the Melogram might display a frequency of 600 Hz (1/2) or 400 Hz (1/3), depending on the setting.

#### **Related Topics**
[Graph Parameters overview](overview)
