# Microsoft Developer Studio Project File - Name="SA_ENU" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=SA_ENU - Win32 URelease
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "SA_ENU.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "SA_ENU.mak" CFG="SA_ENU - Win32 URelease"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "SA_ENU - Win32 UDebug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "SA_ENU - Win32 URelease" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 1
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "SA_ENU - Win32 UDebug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "SA_ENU___Win32_UDebug"
# PROP BASE Intermediate_Dir "SA_ENU___Win32_UDebug"
# PROP BASE Ignore_Export_Lib 1
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\..\Output\Debug"
# PROP Intermediate_Dir "UDebug"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\\" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_AFXEXT" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\\" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_AFXEXT" /D "_UNICODE" /Yu"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /dll /debug /machine:I386 /out:"Debug/SA_ENUd.dll" /pdbtype:sept
# ADD LINK32 /nologo /subsystem:windows /dll /debug /machine:I386 /out:"..\..\..\Output\Debug/SA_ENUd.dll" /pdbtype:sept

!ELSEIF  "$(CFG)" == "SA_ENU - Win32 URelease"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "SA_ENU___Win32_URelease"
# PROP BASE Intermediate_Dir "SA_ENU___Win32_URelease"
# PROP BASE Ignore_Export_Lib 1
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\..\Output\Release"
# PROP Intermediate_Dir "URelease"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\\" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_AFXEXT" /D "_UNICODE" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MD /W4 /Gm /Gi /GX /Zi /Ob2 /I "..\\" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_AFXEXT" /D "_UNICODE" /D "WIN32" /D "NDEBUG" /FR /Yu"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_AFXDLL" /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /dll /debug /machine:I386 /out:"UDebug/SA_ENUd.dll" /pdbtype:sept
# ADD LINK32 /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "SA_ENU - Win32 UDebug"
# Name "SA_ENU - Win32 URelease"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\SA.rc
# End Source File
# Begin Source File

SOURCE=.\SA_ENU.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=..\RES\1.ICO
# End Source File
# Begin Source File

SOURCE=..\RES\2A.ICO
# End Source File
# Begin Source File

SOURCE=..\RES\2B.ICO
# End Source File
# Begin Source File

SOURCE=..\Res\2c.ico
# End Source File
# Begin Source File

SOURCE=..\RES\3A.ICO
# End Source File
# Begin Source File

SOURCE=..\RES\3B.ICO
# End Source File
# Begin Source File

SOURCE=..\Res\3c.ico
# End Source File
# Begin Source File

SOURCE=..\RES\4A.ICO
# End Source File
# Begin Source File

SOURCE=..\RES\4B.ICO
# End Source File
# Begin Source File

SOURCE=..\Res\4c.ico
# End Source File
# Begin Source File

SOURCE=..\RES\5.ICO
# End Source File
# Begin Source File

SOURCE=..\RES\6A.ICO
# End Source File
# Begin Source File

SOURCE=..\RES\6B.ICO
# End Source File
# Begin Source File

SOURCE=..\Res\advanced.bmp
# End Source File
# Begin Source File

SOURCE=..\RES\BPU.BMP
# End Source File
# Begin Source File

SOURCE=..\Res\down.ico
# End Source File
# Begin Source File

SOURCE=..\RES\ECHOU.BMP
# End Source File
# Begin Source File

SOURCE=..\RES\EQU.BMP
# End Source File
# Begin Source File

SOURCE=..\RES\FACE.BMP
# End Source File
# Begin Source File

SOURCE=..\RES\FINGERN.CUR
# End Source File
# Begin Source File

SOURCE=..\RES\FINGERNW.CUR
# End Source File
# Begin Source File

SOURCE=..\RES\HANDN.CUR
# End Source File
# Begin Source File

SOURCE=..\RES\HPU.BMP
# End Source File
# Begin Source File

SOURCE=..\Res\idr_sa_a.ico
# End Source File
# Begin Source File

SOURCE=..\RES\idr_synthesis_vocal_tract.ico
# End Source File
# Begin Source File

SOURCE=..\RES\JAW.BMP
# End Source File
# Begin Source File

SOURCE=..\RES\KNOBH.BMP
# End Source File
# Begin Source File

SOURCE=..\RES\KNOBV.BMP
# End Source File
# Begin Source File

SOURCE=..\RES\LANDU.BMP
# End Source File
# Begin Source File

SOURCE=..\RES\LED.BMP
# End Source File
# Begin Source File

SOURCE=..\RES\LPU.BMP
# End Source File
# Begin Source File

SOURCE=..\Res\MedFingerN.cur
# End Source File
# Begin Source File

SOURCE=..\Res\MedFingerNW.cur
# End Source File
# Begin Source File

SOURCE=..\Res\MedHandN.cur
# End Source File
# Begin Source File

SOURCE=..\Res\next.ico
# End Source File
# Begin Source File

SOURCE=..\RES\PORTU.BMP
# End Source File
# Begin Source File

SOURCE=..\Res\prev.ico
# End Source File
# Begin Source File

SOURCE=..\RES\RAWU.BMP
# End Source File
# Begin Source File

SOURCE=..\RES\RESULTU.BMP
# End Source File
# Begin Source File

SOURCE=..\RES\REVU.BMP
# End Source File
# Begin Source File

SOURCE=..\RES\SA.ICO
# End Source File
# Begin Source File

SOURCE=..\RES\SA_DOC.ICO
# End Source File
# Begin Source File

SOURCE=..\RES\SA_WB.ICO
# End Source File
# Begin Source File

SOURCE=..\RES\SAS.ICO
# End Source File
# Begin Source File

SOURCE=..\RES\SIL.BMP
# End Source File
# Begin Source File

SOURCE="..\Res\Splash-2.bmp"
# End Source File
# Begin Source File

SOURCE=..\Res\taskbar.bmp
# End Source File
# Begin Source File

SOURCE=..\RES\TONGUE.BMP
# End Source File
# Begin Source File

SOURCE=..\RES\TOOLBAR.BMP
# End Source File
# Begin Source File

SOURCE=..\Res\up.ico
# End Source File
# End Group
# End Target
# End Project
