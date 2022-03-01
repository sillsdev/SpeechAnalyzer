---
title: Fragment
---

A *fragment* is the smallest significant unit of audio information in a sound.

In voiced sounds this is the pitch period. Fragment boundaries are located at the beginning of large positive pulses, usually pitch pulses. Similarly, in unvoiced sounds, fragment boundaries are often located at the beginning of major acoustic events like bursts.

To cause the [begin and end cursors](begin-end-cursors) to snap to the nearest fragment, do the following:

1. On the [Tools](../tools/overview) menu, click **Options**.
1. In the [Cursor Alignment](cursor-alignment) area, select **Fragment**.
1. Click **OK**.
1. In the [Raw Waveform graph](types/raw-waveform), position the begin cursor to select a particular fragment.
1. Move the end cursor as close as possible to the begin cursor *without moving* the begin cursor.
1. Use [zoom](zoom) until you believe you have selected *only* a single fragment.
1. To ensure you have selected a *single* fragment, do the following:
   * With the begin cursor at the beginning of the desired fragment, drag the end cursor as close as possible to the begin cursor without moving the begin cursor, and release the mouse button.
   * If the end cursor snaps to the right, it is at the true end of the fragment.

#### **Related Topics**
[Graphs overview](overview)
