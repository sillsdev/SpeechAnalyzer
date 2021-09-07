# SA - 3.1.1.2 9/7/2021
- Add shortcut keys for Ctrl-Down and Ctrl-Up to select next/previous graph.

# SA - 3.1.1.1 8/16/2021
- Updated language list
- Fix 'Esc' key to cancel progress status bar
- Fix phonemic and phonetic pronunciations in the LIFT export feature.
- Fixed double-quote '"' problem when saving file names.
- Developer Changes:
    - Create parse-langtags tool to generate language list
    - Fix test sample paths for unit tests.
    - Get xerces dependency as nuget package.
    - Updated SA project and dependencies so that it is now built with Microsoft Visual Studio 2019.

# SA - 3.1.0.148 9/1/2018
- Fixed bug where Play and Stop buttons on Start dialog weren't restored after playback completed.
- Removed MBROLA feature.

# SA - 3.1.0.147 8/28/2018
- Fixed bug where the 'Open' and 'Open Startup' dialog boxes were not able to play .mp3 files.

# SA - 3.1.0.146 3/23/2018
- Fixed mini-caption bar rendering issues that occurs when Windows Themes are changed to 'classic' mode. 
- Fixed mini-caption bar rendering issues that occur when window is minimized.  When minimized, the window now uses the 'Normal' format in all cases (e.g. without/without caption, no caption, etc).
- Fixed mini-caption bar rendering issues that occur when window as in use.  Some edges were not properly calculated causing display problems.
- Fixed bug that causes the Gloss Transcription bar to be displayed when performing 'Tools/Automatic Markup/Locate Words...'

# SA - 3.1.0.145 3/5/2018
- Fixed a bug where for 'Tools/Automatic Markup/Locate Words...' that when Gloss English segments are created, they are created without a delimiter ('#').  When you attempt then edit the transcription, the first letter typed becomes the WORD('#') or BOOKMARK ('!') delimiter.
- Fixed a bug where a crash occurs after editing a lone phonetic segment with the transcription dialog editor, and then deleting the segment.  The segment must be the only segment in the document.
- Fixed a bug where the SaServer was not observing the 'start' position for the 'PLAY' batch command.  The value was always treated as zero.
- Updated the Visual C++ Runtime package to version 14.0.24212.

# SA - 3.1.0.144 2/18/2018
- Modified application so that 'Edit Boundaries' is no longer automatically disabled when performing 'split' and 'merge' operations.

# SA - 3.1.0.143 2/9/2018
- Fixed problem where 'Hide' and 'Minimize' commands were not working correctly when batch mode was used.
- Fixed problem where Player dialog does not observe setting in 'Play' dropdown list.

# SA - 3.1.0.142 2/2/2018
- Fixed two syntax problems in the LIFT export feature.

# SA - 3.1.0.141 1/29/2018
- Fixed crash that occurs when using SA with Phonology Assistant.
- Fixed some rendering issues for MiniCaptionBar feature.

# SA - 3.1.0.140 12/29/2017
- Fixed crash that occurs when using 'locate phonetic segments' feature.
- Removed dependencies on .NET 2.0 and 3.5.  Speech Analyser now uses .NET 4.0.

# SA - 3.1.0.139 9/20/2017
- Fixed exception that occurred when export Lift files.
- Modified Lift export operation so that the user may now select the language by it's full name, rather than the 2 character country code, when selecting and assigning segment types to export. 

# SA - 3.1.0.138 9/8/2017
- Updated Microsoft Visual C++ Redistributable to version 14.0.23026
- Fixed phonetic replacement problem.

# SA - 3.1.0.137 7/24/2017
- Updated SA project and dependencies so that it is now built with Microsoft Visual Studio 2015.

# SA - 3.1.0.136 11/12/2015
- Fixed bug in FW Lift export code that was preventing audio files from being imported into FW.
- Modified tempfile naming function so that all temp files are now stored in %temp%/SpeechAnalyzer
- Modified install program name (displayed in 'Programs and Features') so that the word 'version' is now ommitted.

# SA - 3.1.0.135 11/4/2015
- Added 'Sample Files' entry to start mode dialog box.  If user has deleted the samples directory, the entry will default to the last used location (eg. same as 'More Files')
- Added 'Gloss National' to Find/Replace dialog.
- Added 'sa batch.psa' file to installation program.
- Modified resampling function to use less memory during operation.
- Modified 'save/save as' operation so that wave form workbench operations and modification (eg. invert or normalize) are not applied to wave file.
- Fixed 'Page Setup' dialog and Workbench dialog.  Neither dialog was properly loading the bitmaps for the button controls.

# SA - 3.1.0.134 10/15/2015
- Fix for Music Mode playback bug - the message event handler was not properly handling the 'play specific' event for the playback button on the staff window.
- Modified install to check for .NET frameworks 2.0 and 3.5.  If they aren't installed, the user will be notified and the install will stop.
- Modified install to place sample files in users document directory.  Checked SA for conflicts.
- Modified install to update registry entries that PA uses to locate SA. Both 32-bit and 64-bit hives are checked.
- Fixed 'position view' problem.  Any paste operation in the wave form window induces an error in the position view.  The position view was not properly resizing when new data is added to the wave form.  If the full size of the wave form is copy and pasted, it will appear to the user as if nothing happened.

# SA - 3.1.0.133 9/24/2015
- Fix for MusicXML DTD load delay.
- Fix for music mode 'hang'.  I believe this was caused by the use of autosave when trying to save the musicxml file.  It was taking a very long time to complete (because of #1) and autosave may not complete before a new one is requested.  To the user, the app appears continually busy.
- The saveas layout has been modified per request.
- The missing music xml fonts and DTD files were added to the install.
- Wording was changed for the Export menu item for Fieldworks operations.
- SA is now maximized if the user right-clicks on the minimized icon and select 'Speech Analyzer'.  NOTE: This behaviour is somewhat deviated from 'normal' behaviour because SA is a single instance app.  Normally, the right-click sequence should cause a new (second) instance of the app to be launched.  If the user insteads to maximize the app, all they need to do is left-click on the minimized icon.
- Modified install so that it no longer uses the previous install path.

# SA - 3.1.0.132 8/18/2015
- Fix for LIFT exports.  Export dialog allows user to specify location of generated files.  Audio files are now stored in a media subdirectory.
- Modified text on tools/options/colors tab.
- Fixed SaveAs GPF.  Code used for reading wave file information used a buffer that was too small for long filenames.
- Fixed segment boundaries default.  Segment boundaries are now stored on a per-graph basis in the .psa file.
- Fixed left-paren and ampersand character problem in transcription editor.
- Modifed install.  Old versions are now uninstalled before the new version is installed.
- Added German resource dll (SA_DEU.DLL).  Interface is not fully translated.

# SA - 3.1.0.131 8/9/2015
- Fixed bug where autosave was not able to handle .wav files that were marked as 'readonly'
- Abbreviated wording on tools/options/color tab.
- Fixed bug in 'splitfile' feature where directory creation fails if an intermediate directory does not exist.

# SA - 3.1.0.130 8/8/2015
-Fix for copy/paste bug.

# SA - 3.1.0.129 8/7/2015
- Disabled 'SaveAs' wave resampling function if user didn't change the sampling rate.
- Fixed 'SaveAs' problem for readonly files.  If the user is saving to a new file, the read-only attribute is cleared if it is set.  If saving to the same file, the user is warned and the operation is terminated.
- Fixed GPF that occurs when saving a recording.
- Modified shortcut keys for Shift-Tab and Alt-Tab.  Alt-Tab has been removed from usage.  Shift-Tab now selects the previous segment.
- Fixed AltZ phrase list bug.
- Modified wording is SaveAs dialog box.

# SA - 3.1.0.128 8/4/2015
- Removed message that occurs when application is converting a incoming wave file to 16-bit format.  Text is now displayed on the status bar during the conversion.
- Modified the 'SaveAs' dialog so that the user now has the choice to downsample the file to 22khz or 44khz, dependent on the current wave file sample rate.

# SA - 3.1.0.127 7/26/2015
- Install bug fixes.
- Changed application install name from 'Speech Analyzer MSEA Beta' to 'Speech Analyzer MSEA'.
- Branched audio-sync project.
- Removed audio-sync code dependencies and resources.
- Implemented color options for reference, gloss nat, and taskbar pages.
- Added 'Pastel' color set and made it the default for SA.

# SA - 3.1.0.126 7/1/2015
- First build after AS release.

# SA - 3.1.0.125 4/8/2015
- Fixed bug where gloss and glossnat segments weren't identified during SAB import function.
- Added code to save .SFT timing data along with other .WAV and .SAXML data.
- Removed 'Open Startup' menu choice for AS.

# SA - 3.1.0.124 4/7/2015
- Removed Autosave from menu for AudioSync
- Added support so that move left/move right/mege and split can be done after selecting any segment and then performing a right mouse click to access the popup menu.  This is for AudioSync only.
- Modified basic toolbar for AudioSync.
- Transcription boundaries are enabled by default for AudioSync.
- Segment boundaries are disabled by default for AudioSync.
- Modified AudioSync import process so that it now can automatically segment the project if it is not already.  If empty segments exist, of if segment data exist, AudioSync will ask the user what they want to do.
- Renamed install application to to 'AudioSync'.
- Fixed some self-inflicted program crashes when attempting to use a reference to the application object instead of a pointer.  Occurs during 'saveas' operation.  Introduced in 3.1.0.123
- Fixed bug where .SAB backup file wasn't created when import the SAB data file.
- Disabled single-click used to enter in-place editor.
- Disabled the ability to 'move data left' on a cell that contains data.  This prevents data loss.
- Fixed problem where file status was STILL not updated to 'modified' after performing independent 'move left', 'move right' operations.

# SA - 3.1.0.123 4/6/2015
- Fixed bug where checkpoint was not created when import SAB files.
- Fixed bug where first letter of gloss was dropped during SAB import process.
- Added feature to generate CV data
- Fixed bug where screen was not refreshed after SAB import process.

# SA - 3.1.0.122 4/3/2015
- Fixed issue where message box is displayed to tell user they may overwrite existing reference numbers, but all the reference segments are blank.  This occurs during auto-add-reference.
- Started reducing menus for SAB release.
- Added 'Import SAB' feature.  Automatically imports data and assigns reference and gloss values to segments.
- Fixed bug that occurs during MoveLeft - Phonetic marker is not added and Gloss word marker is also missing.
- Modified default character for most segments.  Code was insert space, rather than blank entry.

# SA - 3.1.0.121 4/2/2015
- Fixed problem where file status was not updated to 'modified' after performing 'move left', 'move right', merge or split.
- Fixed problem were extra segments were left when performing 'move left' in normal mode.
- Added feature for SAB to retrieve fields from original SAB import file when using merge/move features.

# SA - 3.1.0.120 3/25/2015
- Fixed bug that was calculating from gloss duration value for gloss segments when import ELAN files.
- Fixed problems with ELAN import feature in that SA wasn't using correct tier name selected in ELAN import dialog box.
- Fixed problems with ELAN import feature where SA was not correctly processing tiers when adding dependent segment types.  SA allows you to now use any tier type to import ELAN data into a SA segment.  If the SA segment type is a dependent segment type (PHONEMIC,ORTHO,REF or GLOSS_NAT), it's parent type will be automatically created if it is not selected in the import process.
- Fixed multirecord import problem that causes extra data to be added to some segments when copying segments.
- Fixed playback bug that occurs when zooming in and playing a segment for the first time after SA is opened.

# SA - 3.1.0.119 3/2/2015
- Fixed 'File/SaveAs' bug that occurs when user selects a 'Save Area' choice of something other than 'Entire File'.
- Fixed bug in display of file length shown on status bar.  Length was not calculate properly for files with more than one channel.

# SA - 3.1.0.118 2/16/2015
- Fixed crash that occurs when using tools-options dialog box before any document is opened.  Crash occurs when existing dialog.
- Fixed crash that occurs when loading user document 96khz-1 minute/10second.  Divide by zero was causing crash.
- Fixed errors that occur when using Invert and Normalize functions on stereo data.
- Modified application to use and deploy VS2010 Microsoft Visual C++ Runtime libraries.

# SA - 3.1.0.117 2/12/2015
- Fixed autosave timer problem.  Timer was not starting until document menu was initialized.
- Modified application so that settings directory is automatically created if it does not exist.

# SA - 3.1.0.116 2/10/2015
- Changed 'what's new' help link to point to release notes document.
- Fixed rendering of /gl on 'import word list' view of 'align transcription data' dialog box. removed '#' character.
- Fixed bug where /ge was prepended with '#' during 'align transcription data' function.
- Modified autosave feature so that when a file is closed, it's autosave files are also removed.
- Modified autosave feature so that it doesn't assume wave file extension is '.wav'.

# SA - 3.1.0.115 2/9/2014
- Increased display time of splash dialog by 0.5 seconds.
- Fixed wordlist data alignment problem for gloss-english. 
- Added validation for case where autosave files in .info file may be missing.
- Fixed autosave problem that prevents autosave files from being saved when temp-recordings have been made.
- Removed 'requireAdministrator' attribute from manifest.  Application now runs as 'user'.

# SA - 3.1.0.114 2/9/2015
- Fixed bug in playback - selecting playback of 'word' when phrase is selected in which start of selected segment preceeds 'word' segment start causes wrong audio portion to be played.
- Fixed selection bug that occurs when playing 'word' from the audio player.
- Fixed segment 'delete' bugs.  Code was allowing dependent segments to be deleted.  Code was modified so that the content is only deleted.
- Fixed segment 'delete' bugs.  When deleting the last phonetic segment holding a gloss segment, the gloss segments were resized to zero and not deleted.

# SA - 3.1.0.113 2/6/2015
- Added Alt-M command and toolbar icon
- Fixed fieldworks export bugs - tone and gloss national were not exported.

# SA - 3.1.0.112 2/5/2015
- Modified code so that missing segments for phonemic, orthographic and tone are added if phonetic segment is present.
- Modified code so that administrator privileges are required to run SA.  This is required for updating registry for PA.
- Fixed bug where transcription boundary state from property file is not properly displayed when opening an document. 

# SA - 3.1.0.111 2/4/2015
- Fixing text-change bugs.

# SA - 3.1.0.110 2/2/2015
- Modified code so that phonetic, phonemic and orthographic segments use an array of text strings for data storage, rather than using a single string buffer.

# SA - 3.1.0.109 1/24/2015
- Fixed function that test file read-only attribute.  Function was not working correctly.  This also caused the autosave feature to fail.

# SA - 3.1.0.108 1/22/2015
- Modified splash screen so that it is displayed sooner, and for a shorter amount of time.  Display time is 1.5 seconds
- Fixed 'improper argument' bug.  Application was setting non-existent reference segment as 'selected'.
- Fixed file save/open gloss-nat bug.  Gloss-nat segments were being adjusted on startup when they should not be.
- Fixed Ctrl1-4, AltZ, AltX bug - Previous modification had altered offset and duration values are returned for segments that don't exist.  Previous code returned zero - new code was causing out-of-bounds exception which prevented function from completing.

# SA - 3.1.0.107 12/292014
- Modified wording on Option Dialog
- Fixed COM problem between older versions of SA and 3.1.0.105.  Renamed SpeechToolsUtils.dll to SAUtils.dll and gave it it's own GUIDs and interface names.
- Fixed Shift-Ctrl-H exception that occurs when merging first phonetic segment.

# SA - 3.1.0.106 12/26/2014
- Fixed bug where last segment markup is deleted when moving data left.
- Fixed segment alignment problem that occurs when splitting segments

# SA - 3.1.0.105 12/22/2014
- Modified splash screen so that it displays and closes quicker.  It is now on it's own thread.
- Merged the yeti*.dlls, ST_Audio.dll, SILTools.dll, SILUtils.dll into SpeechToolsUtils.dll
- Improved SAB features - All features are operational.
The following hot keys are implemented:
CTRL-G 		- Split annotations
SHIFT-G 	- Move annotations left
CTRL-SHIFT-G 	- Split and move annotations left
CTRL-H 		- Merge annotations
SHIFT-H 	- Move annotations right
CTRL-SHIFT-H 	- Move annotations right and merge

# SA - 3.1.0.104 11/13/2014
- Improved SAB features - split and merge segments

# SA - 3.1.0.103 11/10/2014
- Updated application icons
- Added SAB features - split and merge segments

# SA - 3.1.0.102 10/22/2014
- Modified several dialogs to change 'Gloss' to 'Gloss English' and 'Gloss Nat.' to 'Gloss National'.
- Modified SFM export dialog combo list box to use the term 'Rows' instead of 'Default'
- Added a SFM Marker help dialog box.
- Fixed problem where transcription alignment importing was not functioning for gloss national transcription fields when using reference numbers for importing.

# SA - 3.1.0.101 10/20/2014
- Fixed two bugs related to refreshing annotations.
- Fixed 'gloss national' bugs in 'export SFM'

# SA - 3.1.0.100 10/14/2014
- Removed POS data type.
- Added Gloss Nat. transcription field.
- Upgraded all C# dlls to use .NET 3.5

# SA - 3.1.0.99 10/2/2014
- Fixed bug in SFM export - column and multi-record mode - where adjoining segments that ended and started on the same data sample cause concatenation of the data.
- For SFM Export dialog - added 'Select Export Type...' to dropdown box list.
- Fixed Add-Ref bug.  There were several bugs existing in this feature (eg. Insert text wasn't save, Inserting at end causes text to wrap.  Inserting text with existing text injected errors in the new and old references numbers.  
At this time, adding reference numbers will overwrite existing numbers.
- Enabled split file support for stereo files.

# SA - 3.1.0.98 9/24/2014
- Fixed 'Show Boundaries' bug.  Annotation windows that were hidden were not being updated with change-in-state of 'show-boundaries' flag.
- Ref Bug#1 - Fixed problem where Reference fields were not added when new gloss segments were added.  NOTE: Reference segments are not automatically added if the entire reference annotation line is empty.
- Ref Bug#2 - Fixed problem where selecting graphs with mouse was not deselecting high-lighted reference fields.

# SA - 3.1.0.97 9/15/2014
- Fixed missing-gloss bug.  When blank gloss field is saved to an SAXML file, it is not recreated when the file is reopened in SA.

# SA - 3.1.0.96 9/10/2014
- Added FLEX-Lift export feature.
- Fixed gloss selection bug.  Software was allowing gloss field to be edited before it was tagged as a word or bookmark.

# SA - 3.1.0.95 7/27/2014
- Updated 'show boundaries' toolbar bitmap and tooltip text.

# SA - 3.1.0.94 7/27/2014
- Tweaked "Insert Silence" feature to place stop cursor at beginning of wave data following the silence section.
- Converted popup menu for displaying/hiding transcription bars into a popup dialog.
- Modified operation of "graphs/view/segment boundaries" so that 'this graph', 'no graphs' and 'waveform only' follow what is displayed.
- Added 'view boundaries' to transcription popup menu and moved location of 'show transcription editor'.
- sorted 'file/export' menu to alphabetical order.

# SA - 3.1.0.93 7/24/2014
- Added capability to draw transcription boundaries

# SA - 3.1.0.92 7/15/2014
- Fixed bug that prevents user from exporting FW-SFM files if 'other' destination directory is selected.
- Changed enabling and disabling of playback toolbar buttons so that the button that initiated the playback
is the only one enabled during a 'pause' of the record.
- Enabled auto-scrolling for all playback modes except 'playback window'

# SA - 3.1.0.91 4/8/2014
- Install fixes for help files.

# SA - 3.1.0.90 4/7/2014
- Added Training and Samples subdirectories to install.
- Updated text on first page of ELAN import dialog sequence.

# SA - 3.1.0.89 3/31/2014
- Added rudimentary ELAN support.

# SA - 3.1.0.88 3/16/2014
- After using Alt-Z or Alt-X, the software now automatically selects the PL1 segment that was just created.
- Updated the player and the tool bar icons to show the new 'Play End Cursor->File End' feature addted in 3.1.0.87
- Fixed a GPF that was occuring with the new large word lists.  The error occurred because the CObjectIStream class used a buffer with a fix size of 32,000 bytes, regardless of the incoming file size.
- The 'Locate Word' feature now automatically clears the Ref, Phonemic, Tone and Ortho segments before processing
- Import file now allows .txt files as a default file type.
- Fixed a 'Locate Words' bug that was causing a crash in some files.  The crash is caused with the detected word to be created has a duration that is the exact same size of the segment being created, causing the segment end to be at the same location as the next segment start.  This causes the next segment calculation to fail.  The issue was resolved by 'pushing' the next segment ahead, if it's start overlaps the previous segments stop location.

# SA - 3.1.0.87 3/6/2014
- Added feature to automatic scroll wave form during playback - specifically when playing between the end cursor and the end of the file.
- Modified menu wording.
- Converted SA_ENU.dll to a 'resource only' DLL.

# SA - 3.1.0.86 2/24/2014
- Added Alt-X feature to be used for quickly marking up stories in a wave file.

# SA - 3.1.0.85 2/19/2014
- Added capability to save stereo files as mono audio files via the 'SaveAs' dialog box.  The user has the option to select the left or right channel for saving.

# SA - 3.1.0.84 1/20/2014
- Fixed segment rendering problem that occurs when stereo audio files are used.  The bug was introduced in 3.1.083.
- Implemented partial change for stereo-to-mono conversion.  The 'SaveAs' dialog has been modified.  The conversion checks are in place, but the conversion function has been disabled.
- Refactored the code that references the SaParm structure into a class.  The code was modified to reduce the usage of the structure as much as possible.

# SA - 3.1.0.83 1/8/2014
- Added code to check for segments that are beyond the end of the audio data when the segments are loaded on a file-open.  A message is displayed if bad segments are found.  The bad segments are ignored.

# SA - 3.1.0.82 1/7/2014
- Fixed bug that was preventing automatic loading of transcription data after the user clicks 'next' on the first dialog of the Align-Transcription-Data feature.
- Modified code to better support importing of SFM tags that aren't lower case.

# SA - 3.1.0.81 1/6/2014
- Added support for UTF16 (BE&LE) that are encountered during the import process.  UTF32 files are disallowed.  UTF16 files will cause a warning message to be displayed but the contents will be converted and imported.
- Removed dependency on sfSettings.DLL.  The code has been imported in the main SA project.
- Updated application copyright dates.
- Fixed build problem. C# projects (speechtoolsutils.dll, yeti.mmedia.dll, yeti.wmfsdk.dll and st_audio.dll) were not getting harvested.  The binaries in the install were outdated from 9/2013.

# SA - 3.1.0.80 11/27/2013 (Stereo Branch)
- Added menu item for 'Auto Add' feature
- Added checks, messages and conversion functions for UTF16 BE&LE files.  The transcription alignment feature still does not support UTF16 files properly, but most other import functions now provide checks and conversions for UTF16.  The code will not currently properly identify UTF-32.  If the unicode BOM is not provided at the beginning of the file, and the data contains characters > 0x7f, the file will be treated as UTF-8 without warning.
- Fixed the crash that prevents the display of the Duration graph type.
- Fixed 'SaveAs' menuitem problem.  SaveAs was disabled for stereo files for no apparent reason.

# SA - 3.1.0.79 11/17/2013 (Stereo Branch)
- Fixed find/replace bug that occurs when user changes 'find' value on initial selection.  The subsequent search will fail to select a value if the 'find' value is a partial match within the currently selected string.
- Fixed bug which prevented SA from finding the current FieldWorks project directory during an export.  The code now recursively searches both the 32 and 64-bit registry hives for the 'ProjectsDir' value.  FieldWorks names it's registry keys according to version. The new code enumerates over this key.  If SA is unable to find the key, it will attempt to use either the "ProgramData" default FW path on MS Vista and above, or the "Application Data" path for XP.  If that fails, a warning will be posted and the user will be allowed to select an alternate path.
- Fixed a bug that prevented the 'Import SFM' feature from importing reference numbers in a SFM file.

# SA - 3.1.0.78 11/03/2013 (Stereo Branch)
- Fixes for interface with IPA Help.  
1a) Modified the message:
	'No resources found, exiting.' 
	to read 
	'Speech Analyzer was unable to locate or use SA_ENU.DLL or SA_LOC.DLL.\nIt will now exit.'
	
1c) There was a buffer length problem that was preventing the code from correctly parsing parameters received in the batch file.  This resulted in the wrong graph types being displayed.
1d) NOTE!!!! IPA Help needs to be run with 'Windows XP SP3' compatibility mode under Windows 7 and later.  If it is not,
a message will be displayed when Speech Analyzer is unable to located the batch file.  
The message is:
	The file:
	'C:\Program Files (x86)\Speech Tools\IPA-Help\IPA-Help.lst'
	was specified on the command line, but it does not exist.
	If you are using IPA Help on Windows 7 or later, please use the compatibility mode for Windows XP.
	Speech Analyzer will now exit.
Without the compatibility mode set, the file will be put in:
C:\Users\yournamegoeshere\AppData\Local\VirtualStore\Program Files (x86)\Speech Tools\IPA-Help\ipa-help.lst 
by IPA Help.

# SA - 3.1.0.77 10/28/2013 (Stereo Branch)
- Fixed find/replace bug that occurs when phonetic/phonemic/orthographic segments contain more than one character.
- Fixed cut/paste bug that occurs when pasting in phonetic/phonemic/orthographic segments that contain more than one character per segment.
- Fixed screen capture bug that occurred because bmp2png.exe was missing from install, and because the input file path to bmp2png.exe was not valid.
- Added 'space' accelerator to 'pause' menu item.

# SA - 3.1.0.76 10/21/2013 (Stereo Branch)
- Modified text on 'Insert Silence' dialog
- Added 'What's New' menu item to Help menu.
- Rearranged Tools menu.
- Added ellipsis to menu items that needed them.
- Renamed 'Select Waveform' menu items.
- Changed 'Find...' key sequence to Ctrl-F (from F3)
- Corrected text on startup menu.
- Added 'Layout' and 'Types' to 'Graphs' main menu.
- Removed extra space in export-sfm data files.

# SA - 3.1.0.75 - 10/9/2013 (stereo branch)
- Added capability to export columnar data in the Export SFM dialog box.

# SA - 3.1.0.74 - 9/29/2013 (stereo branch)
- Updated install application to include Microsoft VC++ 2008 Redistributables.
- Fixed install application so that the C# dlls are registered at install time.

# SA - 3.1.0.73 - 9/19/2013 (stereo branch)
- Renamed links for help documents from *.doc to *.pdf.
- Corrected scrolling problem in Find dialog box feature.  The code was not properly setting the position parameter in the windows message, so all scrolling updates were effectively disabled.

# SA - 3.1.0.72 - 9/17/2013 (stereo branch)
- Update waveform selection feature so that the start/stop cursors are hidden during selection and redrawn when left mouse button is released.  The start and stop cursors are placed at the edges of the highlight region when the selection is completed.
- Modified code to avoid GPF when transcription data is imported but fields are missing.
- Fixed crash that occurs during a PasteAsNew when more than one Phrase Level transcription is copied (more than one for one level).
- Modified text on InsertSilence dialog box.

# SA - 3.1.0.71 - 9/12/2013 (stereo branch)
- Fixed GPF that occurs during transcription alignment if a field is selected that does not exist in the data.  The software will now insert a '?' for fields that are missing in the data.
- Fixed copy/paste alignment issues.  There were several problems.
  a) The software wasn't calculating the end of the source file correctly, causing an overflow in the math of the length of the data to be copied.  In some cases, no data was copied, yet the system still updated the length of the new file to what the length should be.  this caused display problems.
  b) The software wasn't handling the offset and duration for stereo files correctly when adjusting the annotation segments.
  c) The system wasn't calculating the new position of the stop cursor after the paste operation for a stereo file.
  d) When adjusting the annotations, the software was improperly modifying the old-last segment if the new segment was pasted to the right and outside of it's boundary.

# SA - 3.1.0.70 - 9/9/2013 (stereo branch)
- Corrected menu/resource problem which was preventing InsertSilence feature from operating correctly.
- Added code to disable ALT+Z when it overlaps an existing phonetic, PL1 or PL2 segment.
- Corrected code the prevented ALT+Z from executing when the new PL2 segment to be added is just touching an existing PL2 segment.

# SA - 3.1.0.69 - 9/8/2013 (stereo branch)
- Modified Tools/Options dialog so that the audio portion is available when no view has been displayed yet.
- Modified OnEditAddAutoPhraseL2 to handle cases where the start/cursors are located between two other PL2 segments, or before or after all other segments.

# SA - 3.1.0.68 - 9/4/2013 (stereo branch)
- Fixed transcription mis-alignment bug.  For stereo files only, this bug occurred when the data was written to the .saxml file.  The gloss, reference and POS transcriptions were not written with the correct offset and length.  This also caused improper segment indexing when trying to determine the segment number of the transcription.  Gloss, reference and POS transcription lengths are written in the 'MarkDuration' field of the .saxml.

# SA - 3.1.0.67 - 9/3/2013 (stereo branch)
- Fixed transcription mis-alignment bug.  Caused by SpeechToolUtils dll because it wasn't able to read a Broadcast Wave Format meta tag.
- Fixed reference numbering problem that occurs when importing reference numbers start at a gloss position beyond the first gloss.  Because the reference indexes were empty at the start of the operation, the synchronization between the reference segments and gloss segments was broke.  Added code to insert empty reference segments for the skipped positions if one doesn't exist already.

# SA - 3.1.0.66 - 9/2/2013 (stereo branch)
- Fixed buffer overrun bug in CObjectIStream
- Fixed resource collision problem (autosave off)

# SA - 3.1.0.65 - 8/21/2013 (stereo branch)
- Added stop buttons for 'Play' and 'Original' buttons on record-overlay dialog box.
- Fixed problem with gaps in playback of recording when using record-overlay dialog box.

# SA - 3.1.0.64 - 8/14/2013 (stereo branch)
- Added instruction to copy Sa.exe to SaServer.exe during installation compile.
- Fixed state bug where recording and playing can occur at the same time when using 'between cursors'.  The state machine now blocks until playback is completed.
- Added the word 'transcription' to the 'Edit/Transcription/Copy Phonetic to Phonemic' menu item.
- Added 'Insert Silence' feature
- Made the record-overlay feature dialog modeless.
- Added stop button to record-overlay feature dialog.

# SA - 3.1.0.62 - 7/22/2013 (stereo branch)
- Fixed problems with positioning and truncating of overlay data that occurs when using auto recorder.
- Added 'Entire File' menu choice for SaServer
- Renamed 'Edit/Segment Boundaries' menu items to 'Edit/Edit Segment Boundaries'
- Renamed 'Edit/Segment Size' menu items to 'Edit/Edit Segment Size'
- Lowered frequency limit for female voice for formant charts.

# SA - 3.1.0.61 - 7/16/2013 (stereo branch)
- Reversed mouse wheel/zoom operation.
- Disabled action associated with 'Select Waveform' menu item.
- Modified AutoRecorder so that when 'between cursors' is selected, the recorder limits the record time to the length of the start/stop cursor size.

# SA - 3.1.0.60 (stereo branch)
- Updated 'formant tracks' options so that Pitch/F1-F4 checkboxes are tracked 'through' the changing of the 'show formant tracks' checkbox
- Added Stop button to Transcription Editor dialog box.
- Added shortcut to 'Select Waveform' menu item
- Added seven new fields to the status bar.  The file information now displayed is extension, bitrate, length (time), size (KB/MB/GB), frequency, sample size and stereo/mono.
- Tied mouse wheel movements to 'zoom'
- Updated 'Record Overlay' dialog so that it now can reference the start stop cursors for playback and recording.

# SA - 3.1.0.59 (stereo branch)
- For Formant chart,
 (a) modified application so that the title and 'nearest vowel' labels are no longer resized when graph is resized.
 (b) fixed bug which was causing 'nearest vowel' to move horizontally when graph was resized vertically.
 (c) modified rendering so that 'nearest vowel' label and vowel are drawn on same baseline.
- For 'Graph Parameters'/Spectogram dialog box,
 (a) modified application so that F4 is not enabled by default on initial program start.

# SA - 3.1.0.58 (stereo branch)
- Adjusted F1 & F2 ranges for male voice.
Old Range:
F1 200-800
F2 800-2600
F3 1690-3010

New Range:
F1 200-1000
F2 500-2500
F3 1690-3010

# SA - 3.1.0.57 - 5/8/2013 (stereo branch)
- Fixed bug that was prevent opening of .WMA files (VARIANT_BOOL was not properly converted to c++ bool).
- Fixed cut/'paste as new' bug that occurred when editing stereo files.

# SA - 3.1.0.56 - 5/6/2013 (stereo branch)
- Fixed self-test feature.  Some tests may still exhibit problems.
- Fixed workbench bug.  Code was not buffering/reading wave data correctly when IIR filter was used.

# SA - 3.1.0.55 - 4/22/2013 (stereo branch)
- Added DDX routine to check splitfile prefix and suffix for invalid characters.
- Fixed bug in split file code that was filter filename at wrong time.
- Modified autosave code to disable autosave when .wav files are readonly and transcription has not been modified.
- Fixed find/replace bug where find/replace was skipping first entry if stop/start markers on not set on a word when the search is started.
- Added code to cleanup ~#!tstWrite!#~ test file.
- Removed some debug code that was writing files to c:\working when the formant tracker is invoked (spectrogram).
- Fixed a bug in SpeechToolUtils that makes it look like a transcription file is saved when the following steps are performed.  The file is not actually saved.
  a) mark transcription file as 'readonly' in windows
  b) open SA
  c) modify transcription
  d) perform 'save'
  e) observe warning dialog. press OK.
  f) reselect identical folder in which error occurred. press OK.
  g) observe warning dialog. press OK.
  h) press 'cancel' in folder dialog.
  i) applications returns without indicating success of failure.
- Modified code in SpeechToolUtils that updates (removes old SA chunks) from a wave file.  It now only updates the file if it is not 'readonly'

# SA - 3.1.0.54 - 4/18/2013 (stereo branch)
- Fixed find/replace bugs for all transcription types.
- Fixed bug in transcription editor where arrow buttons where not always updates correctly when user reaches end of transcription line.  There are still outstanding issues related to the phrase-n transcription lines.

# SA - 3.1.0.53 - 4/17/2013 (stereo branch)
- Fixed bug the occurs during 'save as' on newly recorded file.  A space was appearing at the end of the default filename in the 'save as' dialog box.
- Added pause button to toolbars
- Modified playback cursor so that it flashes when paused.
- Disabled four playback buttons when player is paused.

# SA - 3.1.0.52 - 4/15/2013 (stereo branch)
- Implemented capability to move start/stop cursors in edit-segment and edit-boundaries mode using CTRL-ALT and SHIFT-ALT sequences with LEFT and RIGHT arrows.

# SA - 3.1.0.50 - 4/10/2013 (stereo branch)
- Fixed Spectrogram display problem.
Known Problems:
- Doing a cut/paste as new/undo corrupts the original graph.
- At 96khz, the Spectrum graph isn't able to display a graph when compared to selecting the same region in a 22khz wave file.
- At 96khz, the F1vF2 graph isn't able to display a graph when compared to selecting the same region in a 22khz wave file.

# SA - 3.1.0.49 - 4/4/2013 (stereo branch)
- Fixed problem in autopitch graph where graph was double the length from what it should be.
- Fixed crash in spectrogram graph when 96khz file is opened -  Code was truncating data value.
- Fixed crash in autopitch graph when 96khz file was opened - same as #2.
Known Problems:
- Doing a cut/paste as new/undo corrupts the original graph.
- The spectrogram graph is corrupted when processing 96khz files.
- The Formant graphs don't work.

# SA - 3.1.0.48 - 4/2/2013 (stereo branch)
- Merged trunk with stereo branch

# SA - 3.1.0.47 - 3/29/2013
- Doubled performance of transcription writes.
- Added code to disable autosaves during playback.

# SA - 3.1.0.46 - 3/29/2013
- Modified autosave to save a .info file which provides the original source directory to be used for file restoration.
- Modified autosave so that the user is prompted on startup concerning autosave files.  They will be queried for each autosave file as to whether or not to recover the file, and in the case of recordings, they will be asked to supply a directory to save the file into.
- Modified autosave to avoid saving .wav files if the file size hasn't changed.
- Modified autosave to stop deleting autosave files when the user performs a 'save' or 'saveas' operation in SA.

# SA - 3.1.0.45 - 3/28/2013
- Fixed GPF in SFM import feature that occurs if any of the transcription fields (or/pm/ph/gl) are missing.
- Merged SFM tabular support into the AutoRef/TAT code.

# SA - 3.1.0.44 - 3/27/2013
- Fixed AutoSave feature so retrieved files are renamed so that they are deleted when the application is closed.
- Added SFM tabular support to File/Import/SFM menu item.

# SA - 3.1.0.43 - 3/25/2013
- Implemented AutoSave feature.

# SA - 3.1.0.42 - 3/22/2013
- Fixed bugs in 'Find' feature where a single character could not be found in any CTextSegment derived segment.  Class was doing whole word search only.  Modified function to search for substrings within annotation strings.
- Fixed bug with 'Alt-Z' feature that prevents the repeat execution of the feature if the cursors have not been moved, or if either cursor is located within another phonetic segment.
- Modified 'Save' function so that if the original file doesn't exist (ex: user retrieved file from removable drive, drive has been removed before 'Save' menu item was executed).
- Fixed bug where VK_MENU key (ALT) was being used to enable 'edit boundaries' feature.

# SA - 3.1.0.41 - 3/18/2013
- Added Ctrl+Q accelerator for 'Add Phonetic Segment' command.  Left in 'Alt+='
- Remapped Ctrl+Q (segment size) to Ctrl+E
- Fixed menu enable/disable problem for Ctrl+C (copy) menu item.
- Added Alt+2 control sequence to automatically add PL2 segment.
- Relabled and retargeted 'Phrase L2' button on Transcription Editor to 'Phrase L1'
- Add Alt+Z control sequence to automatically add phonetic, gloss, PL1 and PL2 segments
- Fixed moving playback cursor problem that occurs after pause button is placed on player dialog.

# SA - 3.1.0.40 - 3/14/2013
- Fixed bug on spectrogram that was limiting calculation for formant tracks to 4.
- Reordered buttons on all four (xml/table/sfm/fw) export dialogs
- Changed default font size for ref,pl1,pl2,pl3 & pl4 transcriptions lines from 10pt to 12pt
- Fixed bug in SFM and XML exports where a space was not inserted between phrase transcriptions.

--Code refactoring changes--
- Reformatted code base using AStyle.
- Removed activex and chrchart project subirectories

# SA - 3.1.0.39 - 3/13/2013
- Changed SFM tag text in FW Export dialog
- Added result dialog that is displayed after FW export operation.
- Fixed copy/paste bug

# SA - 3.1.0.38 - 3/6/2013
- Fixed bugs on XML/SFM and FW export dialogs.
- Removed PL1 and PL2 SFM output from FW export data.
- Fixed bug in code that associates phrase segments with gloss segments during export of FW data.

# SA - 3.1.0.37 - 3/5/2013
- FieldWorks export feature - added code to export phrase files.
- FieldWorks export feature - modified the way the checkbox interact with the 'all annotations' checkbox.
- FieldWorks export feature - reordered gloss and reference marks in SFM output file.
- FieldWorks export feature - removed double backslash in filename generator.
- Recompiled C# code with hopefully now correct version of SILEncConverters.
- Updated install to include sil website reference.

# SA - 3.1.0.36 - 3/4/2013
All changes are associated with 'FieldWorks Export' feature..
- Implemented backend support for FieldWorks exporting
- Added code to disable 'Export' button if data is incompleted in dialog.
- Added SFM mapping table
- Removed SFMExport 'includes' checkboxes that are no longer used.
- Removed Multi-record and Interlinear checkboxes.
- Removed Browse button associated with FieldWorks base project directory.
- Removed initial selection of project in FieldWorks project list.
- Changed title on dialog

# SA - 3.1.0.35 - 2/25/2013
All changes apply to SFM multi-record exporting only.
- Sorted record output according to email chain..
- Phrase transcriptions are now written with their associated 'parent' transcriptions.
- '#' was removed from gloss transcription

# SA - 3.1.0.34 - 2/21/2013
- A fix for the notorious 'missing end cursor' bug.

# SA - 3.1.0.33 - 2/19/2013
- Implemented multirecord support working for SFM.
- Changed 'edit segment size' to use CTRL-Q
- Resized edit fields in TAT
- Fixed problem that was preventing user from adding a word when no words exist.

# SA - 3.1.0.32 - 2/15/2013
- Fixed bug that was allowing word to be added beyond last segment.
- Add 'tag' prefix to reference numbers when using 'auto ref'

# SA - 3.1.0.31 - 2/12/2013
- Fixed Tools menu bug.
- Implemented feature to write registry entries for PA
- Fixed memory overwrite bug caused by improper use of scanf.

# SA - 3.1.0.30 - 2/11/2013
- Prototype for FW .SFM export feature.
- Reordered Tools menu.
- Fix for #1 error on Find/Replace problems.  String resource was missing.
- Fix for #2 error on Find/Replace bugs - disabled hightlight.

# SA - 3.1.0.29 - 2/11/2013
- Modified Transcription Editor dialog so that long strings of cause the edit field to scroll vertically.  A scroll bar is now available.
- Fixed '0' segment problem.  Code was carrying Unicode BOM in string, causing string compare to fail.
- Fixed problems are related to replacing existing segment strings.  Code now longer does 'DeleteContents' before apply TAT changes.
- Fixed problem where extra 'undo' was being applied in TAT dialog.

# SA - 3.1.0.28 - 2/4/2013
- Added 'singleton' feature to SA.  The feature is disabled if SA is launched with Batch mode or as a server.
- Added capability to play Phrase List 2 from the transcription editor.
- Fixed undo bugs in TAT.

# SA - 3.1.0.27 - 1/23/2013
- Added 'Preview' functionality to Transcription Alignment dialog box.  No text changes yet.

# SA - 3.1.0.26 - 1/22/2013
- Fix for AutoRef where code was validating 'end' value before it had been loaded from control.
- Modified validation of auto ref 'begin' and 'end' parameters.

NOTE:  The following rules are used:
'begin' - must be >=0 and <= 10000.
'end' - must be >= begin and <= 10000.
The code internally limits the actual iterations to the number of gloss segments on the screen, so if the 'end' number is beyond the actual gloss count, the logic will terminate at the correct point - the last gloss word.

- Modified the way defaults are stored for the autoref dialog.  They are now stored in the registry.
- Widened the TAT dialog box.
- Fixed bugs in TAT that occur when a user tries to reimport the same or a different file on the second page (when using reference numbers).  The edit field would always be blank because the internal data structure was in an uninitialized state to the second import, cause errors.
Also, if the user imports another file, and it successfully loads, it will be set as the default.

# SA - 3.1.0.25 - 1/20/2013
- Fix for bug where TAT was incorrectly inserting text into segment list
- Fixed bug where TAT removes all gloss fields when no ref fields exist.  (added error message)
- Fixed bug where AutoRef corrupts reference fields when inserting a field after one or more empty fields.

# SA - 3.1.0.24 - 1/18/2013
- Split lines in TAT import error message.
- Fixed toolbar alignment problem.
- Modified how errors are propogated in AutoRef import operation.  Fixes problem where errors are returned correctly.

# SA - 3.1.0.23 - 1/15/2013
- Changed name of 'Reference Data' dialog to 'Reference Numbers' and modified menu item.
- SplitFile - added error message when reference field is empty.
- Modified dialog that informs user they are about to overwrite data so that they can either quit, retry or continue.
- Fixed bug that wasn't calculate reference and gloss segments lengths correctly, which was causing spurious error messages.
- Changed default transcription data folder name from xxx-gloss to xxx-data
- Changed 'Phrase' to 'Phrase Text' in drop list for Phrase filename prefix.
- Moved location of 'Split File' menu item so that it isn't near 'Save'
- Added 'Redo' to toolbar
- Fixed 'phrase level 2' bug (see #4)
- Added error message for 'Auto Reference Numbering'.

# SA - 3.1.0.22 - 1/13/2013
- Updates for 'Split File' feature.  Added controls to allow user to control whether data is overwritten or not.  Changed dialog layout.
- Fixed CTRL-left click bug.

# SA - 3.1.0.21 - 1/7/2013
- Modified 'Split File' dialog box.
- Implemented basic functionality for the Split File feature.

# SA - 3.1.0.20 - 12/20/2012
- Completed 'edit segment size' feature.
- Removed popup-dialog with missing text. (resource problem)
- Adjusted 'edit segment size' icon.

# SA - 3.1.0.19 - 12/18/2012
- Fix for transcription display bug.  Text was not being inserted correctly into transcription segment object with the correct index value.
- Added phrase level shortcut keys.
- Fixed bug where application prompts user to see if they wish to automatically snap segment boundaries.  The message was removed and the operation is always performed.
- Added toolbar button and menu item for 'edit segment size'.
- Updated IPA Chart dialog to prompt user and disable controls if IPA Help is not installed.

# SA - 3.1.0.18 - 12/14/2012
- Modifications to Auto Ref dialog box to implement 4 radio buttons and allow importing of word list file.
- Modifications to Automatic Transcription dialog to allow importing of wordlists.  Added 'Use Reference Data...' checkbox on first page controls dialog sequence and processing.

# SA - 3.1.0.17 - 11/23/2012
- Removed saxml.tmp file left over during split file operations.
- Fixed wrong error message when that occurs when split files with gloss/ref naming convention.
- Modified TAT feature so that '#' marks are maintained when numbering is encountered.  If no '#' marks on found on import data, TAT uses spaces to delimit.  The code also no longer injects '.'s when numbers are found.
- Named various enumeration and struct structures throughout code.
- Updated error message text that is displayed when empty segments are found during the split file export feature.
- Reordered 'Automaric Markup' popup menu.

# SA - 3.0.1.16b - 11/18/2012
- Simplified auto transcription alignment.  Phonetic, Phonemic and Orthographic transcriptions use the space delimiter as the word break both for building the displayed input string and for processing the transcriptions.  Gloss uses the space delimitier as a word break, but also supports incoming '#' characters or single digit number as breaks. 

# SA - 3.0.1.16 - 11/16/2012
- Addressing the following emails:
   11/03/12 - Re Split File Work
   10/31/12 - Transcription Alignment Bug
   10/31/12 - Transcription Alignment Selection
   11/13/12 - Formant Chart Range
   11/12/12 - SA bug in F2-F1 Data Set dialog
   11/02/12 - Formant Chart Properties
   11/11/12 - Re sa with auto-transcription fix
   11/05/12 - The Locate Options
   11/15/12 - re sa with auto transcription fix	
- Modifications to auto transcription alignment.  Phonetic and Phonemic alignment are now generated as one character per segment.
- Format Chart ranges and been changed for men and women to 1000 from (800/900)
- Disable 'Next' button when nothing is checked on first dialog in auto transcription alignment feature.
- Disabled edits in F1-F2 combo box in F2-F1 data set dialog
- Enabled edits of default formant tables
- Removed redundant checks on 'Locate Word/Phrases' features that occur when data already exists
   
# SA - 3.0.1.15 - skipped

# SA - 3.0.1.14 - 11/04/2012
- Fix for bug preventing retention of last field in reference transcription bar.
- Added 'Gloss' checkbox to 'Transcription Alignment' dialog box.
- Added 'Ref' selection in dropdown box in 'Split File' dialog box.
- Enabled 'Browse' button for 'Folder Name' field in 'Split File' dialog box.
- Changed titles in 'Split File' dialog box.
- Added processing for 'Ref' selection in 'Split File' dialog box.
- The 'Gloss Transcription' bug seems to be resolved now with the fix in #1
- Fixed 'file save' bug in 'auto reference numbering' feature/dialog box.
- Modified 'auto reference numbering feature to overwrite existing data.
- Fixed size and position of static text field in 'Transcription Alignment' dialog box.
- Fixed capitalization errors in 'Transcription Alignment' dialog box.
- Removed use of 'wizard' from 'Transcription Alignment' dialog box.

# SA 3.0.1.13 - 10/30/2012
- Added Automatic Reference Data numbering feature.  Dialog is located in Tools->Automatic Markup->Reference Data.

# SA 3.0.1.12- 9/6/2012
- Added dialogs to allow users to select which channel should be imported when opening multi-channel wave files.
- Added 'Audio' tab to 'Tools/Options' dialog.  Audio tab allows users to select whether or not 'advanced audio' dialogs (see #1) should be presented during conversion of non-standard wave files.
- Added code to prevent automatic reopening of converted temporary files.  All non-standard wave and non-wave files are opened as temporary files.
- Fixed out-of-memory exception in SpeechToolsUtils.dll that occurs when opening 'broadcast wave extension' file formats.

# SA 3.0.1.11- 5/3/2012
- Split 'Auto Parse' dialog into 'Auto Parse Phrase' and 'Auto Parse Word' dialog boxes.
- Updated sa3.psa format to store settings for the two new dialog boxes.  New parameter for phrase break parameter is "\phrase_break_ms". Word break default is stored in "\break_ms" property, as before.
- Fixed bug in CSegment class that was indexing beyond the end of the segment offset and durations tables.
- Contains all bug fixes for Speech Analyzer release 3.1.

# SA 3.0.1.10 - 12/8/2011
- Made 'Auto Parse' and 'Auto Segment' dialogs modeless.
- Added 'Gloss' convention to split file dialog box.
- Modified file directory structuring and naming when saving split files.

# SA 3.0.1.9 - 11/30/2011
- For Spectrograph parameters dialog - making F4 checkbox perform as 'F4 and above'
- Made 'Auto Parse' and 'Auto Segment' property pages full fledged dialog boxes.
- Renamed buttons for Auto Parse and Auto Segment dialog boxes
- Added text to 'Auto Parse' dialog box
- Fixed bug regarding loading of defaults for 'Auto Parse' dialog box.
- Fixed 'copy phonetic to phonemic' bug.
- Added 'cursor selection' feature.

# SA 3.0.1.8 - 11/28/2011
- Added deselection process to 'move stop cursor here' feature.
- Renaming 'Split ...' to 'Split File ...'
- Fixed minicap text height issue.
- Fixed minicap rendering issues when minimized.
- Added 'copy phonetic to phonemic' feature

# SA 3.0.1.7 - 11/16/2011
- Fixed 'move cursor here' bug.
- Implemented 'File Split' feature.  Splits a audio file and transcription data based on user selected parameters.
- Renamed some of the 'experimental' features.

# SA 3.0.1.6 - 11/8/2011
- Fixed more CRT_SECURE warnings.
- Modified way AutoParse and AutoSegment work.  Selecting either menu item now displays 'parameters' dialog box.  The Apply button actually works now, as well as Cancel and OK.  Using the Cancel button issues a 'undo' message to reset the graphs to their original state.
- Unhid 'experimental features' menu item

# SA 3.0.1.5 - 11/1/2011
- Quick hack for mini-cap problem.  Code was not drawing background fill correctly, exposing default title bar.
- Fixed more CRT_SECURE warnings.
- FEATURE: When a top level checkbox is unchecked, the subitems are also unchecked for the Export SFM and Export XML dialog boxes.
- Cleaned up project files.
- Reformatted code.
- Fix for Vista compatibility issue in FileOpen/FileSaveAs dialog boxes.

# SA 3.0.1.4 - 10/31/2011
- 'MoveStopCursorHere' feature added to graph popup menu.  This feature allows the user to locate the stop cursor by using the right-click/popup menu on a graph instead of a key-press and a left mouse click.
- Cleanup of CRT_SECURE warnings.  
- Workaround for 'SaveAs' dialog box.  Presently the SaveAs dialog box was crashing when launched by the user because of a Vista/XP compatibility issue.  The offending code was disabled.  The code is now using the WINVER=0x501 define to allow WinXP compatibility.  The problem will be fixed more completely in a later release.
- Made project (.vcproj) files more consistent.

# SA 3.0.1.3 - 10/24/2011
- Converting _stprintf to swprintf_s to remove compiler warnings.
- Fix for horizontal scrolling bug.  White lines were being displayed when horizontally scrolling zoomed-in window.  Disabled scrolling code - forced repaint of window.
- Fix for GPF caused by stack overwrite when creating temporary file name. CreateAuxTempFile was using 'sizeof' function on wide character string.  Probably introduced during Unicode conversion.
	To duplicate:
	Run SA
	Open ChFrench
	Right click in the Auto Pitch graph
	Select Types/ Change

# SA 3.0.1.2 - 10/20/2011
- Fix for F1 Formant issue.  A ULONGLONG value should have been an int.  This was causing data corruption when converted to a double.
- Fixed F2-F1 graph exception
- Fixed TaskBar bug - icons not showing.
Disabled musicxml load during opening of document.
- Fix for gcvt bug
Code was casting char buffer into widechar buffer and corrupt output of gcvt call.
- Fix for Semitone2Name exception.
- Merged VS2005 changes to MSEA branch.
- Made minimal changes required to get project to compile in VS2008.
