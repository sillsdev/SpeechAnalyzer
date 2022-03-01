---
title: Transcriptions no longer appear
---

Speech Analyzer stores [transcription](../user-interface/menus/edit/transcription/overview) data in a file with a [.saxml](../user-interface/menus/file/saxml) extension in the same folder as the wave file.

If the transcriptions no longer appear for a wave file that you previously transcribed, any of the following may have happened:

- The wave file was renamed or moved.

    Rename the .saxml file so its name corresponds to the name of the wave file.

    Move the .saxml file into the same folder as the wave file.

- The .saxml for the wave file was renamed or moved.

    Rename the .saxml file so its name corresponds to the name of the wave file.

    Move the .saxml file into the same folder as the wave file.

- You opened a copy of the wave file.

    Make a copy of original .saxml file, and then rename it, or move it, or both.

- The .saxml for the wave file was deleted.

    If possible, restore the .saxml file.

    If the file was originally transcribed in an earlier version of Speech Analyzer and converted to Unicode in this version, Speech Analyzer backs up the wave file with a .sa2.wav extension in a SA2 Wav Files folder. Delete the wave file, copy the backup of the wave file, remove the .sa2 part from the filename, and then open the file in Speech Analyzer. You can convert the original transcriptions to Unicode again. Only changes made since the previous Unicode conversion are lost.

#### **Related Topics**
[Troubleshooting overview](overview)
