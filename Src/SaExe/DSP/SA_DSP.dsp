# Microsoft Developer Studio Project File - Name="SA_DSP" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=SA_DSP - Win32 URelease
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "SA_DSP.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "SA_DSP.mak" CFG="SA_DSP - Win32 URelease"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "SA_DSP - Win32 UDebug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "SA_DSP - Win32 URelease" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 1
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "SA_DSP - Win32 UDebug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "SA_DSP___Win32_UDebug"
# PROP BASE Intermediate_Dir "SA_DSP___Win32_UDebug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\..\Output\Debug"
# PROP Intermediate_Dir "UDebug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /Gi /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SA_DSP_EXPORTS" /YX"stddsp.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /Gi /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SA_DSP_EXPORTS" /YX"stddsp.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 MSVCRTD.LIB Kernel32.lib MSVCPRTD.lib /nologo /dll /map /debug /machine:I386 /nodefaultlib /pdbtype:sept
# ADD LINK32 MSVCRTD.LIB Kernel32.lib MSVCPRTD.lib /nologo /dll /map /debug /machine:I386 /nodefaultlib /pdbtype:sept

!ELSEIF  "$(CFG)" == "SA_DSP - Win32 URelease"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "SA_DSP___Win32_URelease"
# PROP BASE Intermediate_Dir "SA_DSP___Win32_URelease"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\..\Output\Release"
# PROP Intermediate_Dir "URelease"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /Gi /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SA_DSP_EXPORTS" /YX"stddsp.h" /FD /GZ /c
# ADD CPP /nologo /MD /W4 /Gm /Gi /GX /Zi /Ob2 /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SA_DSP_EXPORTS" /D "WIN32" /D "NDEBUG" /FR /YX"stddsp.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 MSVCRTD.LIB Kernel32.lib MSVCPRTD.lib /nologo /dll /map /debug /machine:I386 /nodefaultlib /pdbtype:sept
# ADD LINK32 MSVCRT.LIB Kernel32.lib MSVCPRT.lib /nologo /dll /map /debug /machine:I386 /nodefaultlib /pdbtype:sept

!ENDIF 

# Begin Target

# Name "SA_DSP - Win32 UDebug"
# Name "SA_DSP - Win32 URelease"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\AcousticTube.cpp
# End Source File
# Begin Source File

SOURCE=.\cspksrc.cpp
# End Source File
# Begin Source File

SOURCE=.\curvefit.cpp
# End Source File
# Begin Source File

SOURCE=.\dspwins.cpp
# End Source File
# Begin Source File

SOURCE=.\epksrc.cpp
# End Source File
# Begin Source File

SOURCE=.\fft.cpp
# End Source File
# Begin Source File

SOURCE=.\formants.cpp
# End Source File
# Begin Source File

SOURCE=.\Fragment.cpp
# End Source File
# Begin Source File

SOURCE=.\grappl.cpp
# End Source File
# Begin Source File

SOURCE=.\histgram.cpp
# End Source File
# Begin Source File

SOURCE=.\HPFLTR70.CPP
# End Source File
# Begin Source File

SOURCE=.\mathx.cpp
# End Source File
# Begin Source File

SOURCE=.\peakpick.cpp
# End Source File
# Begin Source File

SOURCE=.\qpksrc.cpp
# End Source File
# Begin Source File

SOURCE=.\Roots.cpp
# End Source File
# Begin Source File

SOURCE=.\SA_DSP.def
# End Source File
# Begin Source File

SOURCE=.\Spectgrm.cpp
# End Source File
# Begin Source File

SOURCE=.\Spectrum.cpp
# End Source File
# Begin Source File

SOURCE=.\template.cpp
# End Source File
# Begin Source File

SOURCE=.\ZTransform.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\AcousticTube.h
# End Source File
# Begin Source File

SOURCE=.\complex.h
# End Source File
# Begin Source File

SOURCE=.\cspksrc.h
# End Source File
# Begin Source File

SOURCE=.\curvefit.h
# End Source File
# Begin Source File

SOURCE=.\dspTypes.h
# End Source File
# Begin Source File

SOURCE=.\Dspwins.h
# End Source File
# Begin Source File

SOURCE=.\epksrc.h
# End Source File
# Begin Source File

SOURCE=.\Errcodes.h
# End Source File
# Begin Source File

SOURCE=.\Error.h
# End Source File
# Begin Source File

SOURCE=.\fft.h
# End Source File
# Begin Source File

SOURCE=.\Formants.h
# End Source File
# Begin Source File

SOURCE=.\Fragment.h
# End Source File
# Begin Source File

SOURCE=.\Grappl.h
# End Source File
# Begin Source File

SOURCE=.\histgram.h
# End Source File
# Begin Source File

SOURCE=.\Hpfltr70.h
# End Source File
# Begin Source File

SOURCE=.\Mathx.h
# End Source File
# Begin Source File

SOURCE=.\Modules.h
# End Source File
# Begin Source File

SOURCE=.\peakpick.h
# End Source File
# Begin Source File

SOURCE=.\pksrc.h
# End Source File
# Begin Source File

SOURCE=.\qpksrc.h
# End Source File
# Begin Source File

SOURCE=.\Roots.h
# End Source File
# Begin Source File

SOURCE=.\Scale.h
# End Source File
# Begin Source File

SOURCE=.\Signal.h
# End Source File
# Begin Source File

SOURCE=.\Spectgrm.h
# End Source File
# Begin Source File

SOURCE=.\Spectrum.h
# End Source File
# Begin Source File

SOURCE=.\Stddsp.h
# End Source File
# Begin Source File

SOURCE=.\template.h
# End Source File
# Begin Source File

SOURCE=.\tpair.h
# End Source File
# Begin Source File

SOURCE=.\tpair.hpp
# End Source File
# Begin Source File

SOURCE=.\tpq.h
# End Source File
# Begin Source File

SOURCE=.\tpq.hpp
# End Source File
# Begin Source File

SOURCE=.\tvector.h
# End Source File
# Begin Source File

SOURCE=.\tvector.hpp
# End Source File
# Begin Source File

SOURCE=.\ZTransform.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
