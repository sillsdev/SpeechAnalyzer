# Building Speech Analyzer
This document details the build process for Speech Analyzer.
The source code for Speech Analyzer is stored on GitHub. https://github.com/.
Speech Analyzer is a 32-bit application.  Use (x86) installers where appropriate.

# Software Requirements
Building Speech Analyzer requires the following software to be installed:
1. Speech Analyzer.
1. Microsoft Visual Studio 2019
1. .NET Framework 4.6.1 Developer Pack
1. InnoSetup 5.5.3 'unicode'.  You will need to install both isetup-5.5.3-unicode.exe and ispack-5.5.3-unicode.exe.  Innosetup is found at http://files.jrsoftware.org/is/5/.  Accept all defaults when installing the application(s).

# Prerequisites
1. Speech Analyzer.  Install Speech Analyzer to create application directories and install application dependencies that are needed by Speech Analyzer.  Data samples will also be installed.
1. Obtain the source code from GitHub.  The checkout will take a while.
```bash
git clone https://github.com/sillsdev/SpeechAnalyzer
```

## Build xerces-c
The xerces-c library needs to be built for Speech Analyzer.
1. Download the xerces-c 3.1 source from https://archive.apache.org/dist/xerces/c/3/sources/xerces-c-3.1.4.zip and extract it into a folder `xerces-c\`
1. Set the Windows Environment variable `XERCES_VC10_HOME` to the full path for you have xerces-c (no trailing slash)
1. Open Visual Studio 2019 and select and open the solution file : *xerces-c\projects\Win32\VC14\xerces-all\xerces-all.sln*
1. On the menu bar, rebuild the solution for both *Debug* and *Release* configurations for *Win32*.
1. Create a `lib\` directory in xerces-c\
1. Copy the following files from xerces-c\Build\Win32\VC14\Debug\ into the created lib\ directory:
  * xerces-c_3_1D.dll
  * xerces-c_3D.lib
1. Copy the following files from xerces-c\Build\Win32\VC14\Release\ into the created lib\ directory:
  * xerces-c_3_1.dll
  * xerces-c_3.lib

Make note of these files because you will also copy them later for the Speech Analyzer build.

## Install Fonts
The following fonts need to be installed before using the music features:
* [Lib\Fonts\Musique\Musique Unicode.ttf](https://github.com/sillsdev/SpeechAnalyzer/raw/master/Lib/Fonts/Musique/Musique%20Unicode.ttf)
* [Lib\Fonts\Musique\MusicqueU.ttf](https://github.com/sillsdev/SpeechAnalyzer/raw/master/Lib/Fonts/Musique/MusiqueU.ttf)

# Build Speech Analyzer
To build Speech Analyzer, do the following:
1. Open Visual Studio 2019 as Administrator and select and open the solution file : *SpeechAnalyzer\SpeechAnalyzer.sln.*
(Administrator rights needed to get SAUtils to register its library)
1. On the menu bar, select the *Debug* or *Release* configuration.  If you want to build a installer executable later on, you will need to select the *Release* configuration.
1. In the solution explorer, right click on the *SA* project and select the *Set as Startup Project* menu item.
1. Use *Ctrl-Alt-F7* or select *Build/Rebuild Solution* from the menu to build the project.
1. Wait for the build to complete.  You will see the following in the *Build Output* window then the project is done compiling:  *Rebuild All: 15 succeeded, 0 failed, 0 skipped*
1. Depending on the build configuration you selected, *Debug* or *Release*, Visual Studio will create either a *Debug* or *Release* directory at the root of the project. (e.g. SpeechAnalyzer/Debug*).
1. Copy the Xerces DLL and lib from *xerces-c\lib\* into the *Debug* or *Release* directory.  You will only need to do this once.
1. Copy DistFiles\iso639.txt into the *Debug* or *Release* directory. You will only need to do this once.
1. Start SpeechAnalyzer by right-clicking on the *SA* project and selecting *Debug/Start new instance*.

# Installer Creation
1. Double click on the *SpeechAnalyzer\Install\SpeechAnalyzer.iss* to open it with InnoSetup/Studio.
1. In InnoSetup, select *Project\Compile* or use *Ctrl-F9*.
1. After the compile is completed, the installer executable will be stored in the *SpeechAnalyzer/Install/Output* subdirectory.  DO NOT commit this directory when committing the branch.

# Updating version tags
1. In preparation for a new release, the following version labels need to be update.
- Install\SpeechAnalyzer.iss : change the *MyAppVersion* tag
- Src\Lang\SA_ENU.RC : change both instances in FILEVERSION
- Src\SA.rc : change both instances in FILEVERSION
- Src\SA\Sa_Doc.cpp : change Lift reference.
- Src\Lang\SA_DEU.rc : change both instances in FILEVERSION
