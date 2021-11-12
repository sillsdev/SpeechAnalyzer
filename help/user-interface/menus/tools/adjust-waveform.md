---
title: Adjust Waveform
---

To modify the [Raw Waveform graph](../graphs/types/raw-waveform), do the following:

1. On the [Tools](overview) menu, point to **Adjust Waveform**, and then click one of the following:
   * To *invert* a waveform, click **Invert**.

        The **Raw Waveform** graph is inverted (negative values become positive values, and positive values become negative values).

    * To *normalize* a waveform, click **Normalize**.

        All the sample values in the waveform increase until the highest peak is at 100% (at the top of the graph).

    * To remove sound-card offset in a waveform, click **Zero Offset**. Sound-card offset occurs when the sound card *incorrectly* detects the zero-point of the waveform.

        The **Raw Waveform** graph centers on the zero line of the graph.

    A check mark appears to the left of the command on the menu.

1. To change the waveform back, click a command with a check mark next to it.

    The check mark disappears and the **Invert**, **Normalize**, or **Zero Offset** adjustment disappears.

#### ![](../../../images/000.png) **Important**
- When you **Invert**, **Normalize**, or **Zero Offset**, a **Speech Analyzer** dialog box might appear with the following message:

    **The current document contains segment boundaries that are not properly snapped. Would you like to automatically update all segment boundaries to** [correctly snap alignment](../graphs/cursor-alignment) **(Recommended)?**

    Click **Yes**.

#### ![](../../../images/001.png) **Note**
- When you click **Invert**, **Normalize**, or **Zero Offset**, the adjustment might affect other graphs.

#### **Related Topics**
[Tools overview](overview)
