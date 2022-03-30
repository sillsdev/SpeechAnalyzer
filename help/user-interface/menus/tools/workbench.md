---
title: Workbench
---

On the [Tools](overview) menu, click **Workbench** to configure signal-processing tasks such as **echo**, **reverb**, and **equation** filtering. You can configure up to *four* processes. You can feed the output of one process into another.

The filtering can apply to the *whole recording* or only data between the [begin and end cursors](../graphs/begin-end-cursors).

To configure a process, do the following:

1. To select a process to configure, do one of the following:
   * Double-click the **Wave File** ![](../../../images/024.png) icon of the process.
   * On the **Properties** menu, select a process.

      A dialog box appears with **Process** (number) as the title.

1. For each of the three filters you use, do the following:
   * Select a [filter selection](workbench-filter-selections) from the list, other than **Plain**.

      You may not use the *same* filter selection *twice* in a process.

      The **Properties** button activates.

    * If you selected **Highpass**, **Lowpass**, or **Bandpass**, click **Properties**, and then do the following:

        Enter or type the **Upper** and **Lower** numbers in the **Cutoff Frequency** area.

        Enter or type the **Filter Order** number.

        Select or clear the **Forward & Reverse Passes** check box.

    * If you selected **Reverb** or **Echo**, click **Properties**, and then do the following:

        Enter or type the **Delay** numbers (in milliseconds).

        Enter or type the **Gain** numbers (in decibels).

    * If you selected **Equation**, click **Properties** and do the following:

        Type or enter an equation in **Enter Equation f(x)**.

1. To close any **Process** dialog box, *and apply*, click **OK**.

    Icons with each filter name appear next to the **Wave File** icon.

1. Leave **Workbench** without closing it, and select a file from the [Window](../window) menu, or on the [File](../file/overview) menu from the list of most recent files

    Workbench *must be active* to use one of the filter processes.

1. Click the [Waveform tab](../graphs/parameters/waveform-tab) of the **Graph Parameters** dialog box, and, in the **Data Source** area, select a filter process.

#### **Related Topics**
[Tools overview](overview)
