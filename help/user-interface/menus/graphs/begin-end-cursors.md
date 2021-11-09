---
title: Begin and end cursors
---

Speech Analyzer provides a begin cursor and an end cursor to mark portions of audio data in time-based graphs (for example, Raw Waveform and Auto Pitch). The *begin cursor* is a vertical green line. It marks a point in time and the beginning of a portion. The *end cursor* is a vertical red line. It marks the end of a portion.

When an audio file opens, the begin cursor is at the left edge and the end cursor is at the right edge of the plot area. Cursors might be difficult to see at the edge of the graph.

If you display an audio file in multiple graphs, moving a cursor in one graph moves it to identical positions in other time-based graphs.

Another cursor, a *private cursor* is the blue cursor on the [Spectrum](types/spectrum) and [Tonal Weighting Chart](types/music/melogram) which can be moved independently of the begin and end cursors.

- By default the begin cursor is green, and the end cursor is red. To change the colors:
  - On the **Tools** menu, click **Options**, and click the [Colors tab](../tools/options/colors-tab).
  - In the **Graph** list, select either **Begin Cursor** or **End Cursor**.
  - Click ![](../../../images/078.png) to display the [Color](../tools/options/color-palette) dialog box, and pick a *new* color for the selected cursor.
- By default, the begin and end cursors snap to the nearest [fragment](fragment) (pitch period). When you drag either cursor and release the left mouse button, the cursor may jump as it snaps to the nearest fragment boundary.
- If you click a [segment](../edit/segment) in a [transcription bar](../edit/transcription/guidelines), the begin and end cursors snap to the [segment boundaries](../edit/segment-boundaries).
- To *move the begin cursor* to a new location, do one of the following:
  - Click the left-mouse button in the plot area.
  - Hold down **Ctrl**, and click the left-mouse button.
- To *move the end cursor* to a new location, hold down **Shift**, and click the left-mouse button.
- To move *either* the begin or end cursor to a new location, do the following:
  - Move the mouse pointer near the begin or end cursor. The mouse pointer changes to a hand.
  - Hold down the left mouse button (the cursor changes to a double-edge arrow) and drag either the begin or end cursor.

The begin and end cursors can:

- Be moved using [shortcut keys](../../shortcuts/cursor).
- Mark [coarticulation](../edit/segment-resizing), by placing the begin cursor inside the preceding phonetic segment.
- Set [cursor alignment](cursor-alignment) (or align) to **Sample**, **Zero Crossing** or **Fragment**.
- Precisely locate a character in the [transcription bars](../edit/transcription/guidelines).
- Select [playback](../playback/overview) of selected portions of the recording.
- [Select a waveform](../edit/select-waveform) portion and use [zoom](zoom) functions.

#### ![](../../../images/001.png) **Note**
- The [status bar](../tools/status-bar) at the lower right of the window displays information on the begin, end, and private cursor positions, and the time of a fragment.

#### **Related Topics**
[Fragment](fragment)

[Cursor example](cursor-example)
