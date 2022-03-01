---
title: Equalize Length
---

This function can be used when you want to apply a *specific* cursor separation to a recording (the *source* file) and then apply the *same separation* to a second recording (the *target* file). By equalizing the length in time of the [begin and end cursor](begin-end-cursors) separations in two different files, you can then better visually compare acoustic features in each file.

1. [Open](../file/open) two different .wav files.
1. On the [Window](../window) menu, select **Tile Horizontal**.
1. On the target file, place the cursors around a selected region.
1. On the source file, place the cursors around a selected region for comparison with the target file.
1. On the **Graphs** menu, click **Equalize Length**.

The **Equalize Length** dialog box appears, identifying the source document.

1. **Select a target view** (file) in the list, and then click **OK**.

The end cursor in the target moves to the *same separation* as in the source file.

#### ![](../../../images/001.png) **Note**
- To verify the resulting cursor separation, look at the second status bar pane in each file.
- If the target file separation would move the end cursor past the end of the file, a Speech Analyzer dialog box appears with the message **This would move the cursor too far to the right**.
- To switch graphs, *without disturbing the cursor positions*, click on the title bar of a graph.

#### **Related Topics**
[Graphs overview](overview)
