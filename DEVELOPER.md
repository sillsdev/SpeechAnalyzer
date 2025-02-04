# Building Speech Analyzer
This document details the build process for Speech Analyzer.
Speech Analyzer is a 32-bit Windows application.  Use (x86) installers where appropriate.

# Software Requirements
Building Speech Analyzer requires the following software to be installed:
1. Speech Analyzer.
1. Microsoft Visual Studio 2022
    1. Workloads
        * .NET desktop development
        * Desktop development with C++
    1. Individual components
        * .NET 6.0 Runtime
        * .NET Framework 4.8 SDK
        * .NET Framework 4.8 targeting pack
        * Windows 10 SDK
        * C++ MFC for latest v143 build tools (x86 & x64)
1. .NET Framework 4.6.1 Developer Pack
1. InnoSetup 5.5.3 'unicode'.  You will need to install both isetup-5.5.3-unicode.exe and ispack-5.5.3-unicode.exe. 
1. MSHFlexGrid. Found in the repository as MSHFLXGD.OCX, you will need to register it.
Innosetup is found at http://files.jrsoftware.org/is/5/.  Accept all defaults when installing the application(s).

# Register MSHFlexGrid
1. Move MSHFLXGD.OCX to C:\Windows\SysWOW64
2. Open Command Prompt as Administrator
3. Type command REGSVR32 C:\Windows\SysWOW64\MSHFLXGD.OCX

# Prerequisites
1. Speech Analyzer.  Install Speech Analyzer to create application directories and install application dependencies that are needed by Speech Analyzer.  Data samples will also be installed.
1. Obtain the source code from GitHub.  The checkout will take a while.
```bash
git clone https://github.com/sillsdev/SpeechAnalyzer
```

## Install Fonts
The following fonts need to be installed before using the music features:
* [Lib\Fonts\Musique\Musique Unicode.ttf](https://github.com/sillsdev/SpeechAnalyzer/raw/master/Lib/Fonts/Musique/Musique%20Unicode.ttf)
* [Lib\Fonts\Musique\MusiqueU.ttf](https://github.com/sillsdev/SpeechAnalyzer/raw/master/Lib/Fonts/Musique/MusiqueU.ttf)

# Build Speech Analyzer
To build Speech Analyzer, do the following:
1. Open Visual Studio 2022 as Administrator and select and open the solution file : *SpeechAnalyzer\SpeechAnalyzer.sln.*
(Administrator rights needed to get SAUtils to register its library)
1. On the menu bar, select the *Debug* or *Release* configuration.  If you want to build a installer executable later on, you will need to select the *Release* configuration.
1. In the solution explorer, right click on the *SA* project and select the *Set as Startup Project* menu item.
1. Use *Ctrl-Alt-F7* or select *Build/Rebuild Solution* from the menu to build the project.
1. Wait for the build to complete.  You will see the following in the *Build Output* window then the project is done compiling:  *Rebuild All: 15 succeeded, 0 failed, 0 skipped*
1. Depending on the build configuration you selected, *Debug* or *Release*, Visual Studio will create either a *Debug* or *Release* directory at the root of the project. (e.g. SpeechAnalyzer/Debug*).
1. Copy DistFiles\iso639.txt into the *Debug* or *Release* directory. You will only need to do this once. 
Refer to [README.md](DevUtils\parse-langtags\README.md) if you need to re-generate iso639.txt.
1. Start SpeechAnalyzer by right-clicking on the *SA* project and selecting *Debug/Start new instance*.

# Installer Creation
1. Double click on the *SpeechAnalyzer\Install\SpeechAnalyzer.iss* to open it with InnoSetup/Studio.
1. In InnoSetup, select *Build\Compile* or use *Ctrl-F9*.
1. After the compile is completed, the installer executable will be stored in the *SpeechAnalyzer/Install/Output* subdirectory.  DO NOT commit this directory when committing the branch.

# Updating version tags
1. In preparation for a new release, the following version labels need to be update.
- Install\SpeechAnalyzer.iss : change the *MyAppVersion* tag
- Src\Lang\SA_ENU.RC : change both instances in FILEVERSION
- Src\SA.rc : change both instances in FILEVERSION
- Src\SA\Sa_Doc.cpp : change Lift reference.
- Src\Lang\SA_DEU.rc : change both instances in FILEVERSION

# Maintaining Speech Analyzer Help
Help files are maintained as Markdown files in the `help\` folder. To build the .chm help file, the following programs need to be installed:
* Pandoc

    Via Chocolatey from PowerShell:
    ```
    choco install pandoc
    ```

* HTML Help Workshop


To compile the chm file, run the following script from the `help/` folder
```
./build.sh
```

# Release Process
The following steps detail how to create a Speech Analyzer release and publish a signed installer
1. Tag the commit you want to release with a version tag e.g. v2022-12-05
2. GitHub actions will build a signed installer for the commit with a tag starting with `v`.  It will create a draft release for you with the installer attached.
6. Modify the release description to your liking and then publish the release when ready.
