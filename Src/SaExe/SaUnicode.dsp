# Microsoft Developer Studio Project File - Name="SaUnicode" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=SaUnicode - Win32 URelease
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "SaUnicode.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "SaUnicode.mak" CFG="SaUnicode - Win32 URelease"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "SaUnicode - Win32 UDebug" (based on "Win32 (x86) Application")
!MESSAGE "SaUnicode - Win32 URelease" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 1
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "SaUnicode - Win32 UDebug"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "UDebug"
# PROP BASE Intermediate_Dir "UDebug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\Output\Debug"
# PROP Intermediate_Dir "UDebug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W4 /Gm /GX /ZI /Od /I "." /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "WINDOWS_PLATFORM" /D "_AFXDLL" /D WINVER=0x400 /D _WIN32_IE=0x0400 /FR /YX"stdafx.h" /FD /c
# ADD CPP /nologo /MDd /W4 /Gm /GX /Zi /Od /I "." /I "..\lib" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "WINDOWS_PLATFORM" /D "_AFXDLL" /D WINVER=0x400 /D _WIN32_IE=0x0400 /D "_UNICODE" /FR /YX"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "AFX_TARG_NEU" /d "_AFXDLL" /d "AFX_RESOURCE_DLL"
# ADD RSC /l 0x409 /d "_DEBUG" /d "AFX_TARG_NEU" /d "_AFXDLL" /d "AFX_RESOURCE_DLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 mfc42d.lib mfco42d.lib mfcs42d.lib msvcrtd.lib comdlg32.lib user32.lib gdi32.lib kernel32.lib advapi32.lib oldnames.lib winmm.lib version.lib imm32.lib ole32.lib oleaut32.lib uuid.lib /nologo /stack:0x10000 /subsystem:windows /incremental:no /map /debug /machine:IX86 /out:"Debug/SA2.exe"
# SUBTRACT BASE LINK32 /profile /nodefaultlib
# ADD LINK32 mfc42ud.lib mfco42ud.lib mfcs42ud.lib msvcrtd.lib msvcprtd.lib comdlg32.lib user32.lib gdi32.lib kernel32.lib advapi32.lib oldnames.lib winmm.lib version.lib imm32.lib ole32.lib oleaut32.lib uuid.lib comsupp.lib HtmlHelp\htmlhelp.lib shell32.lib /nologo /stack:0x100000 /entry:"wWinMainCRTStartup" /subsystem:windows /map /debug /machine:IX86 /nodefaultlib /out:"..\..\Output\Debug\SA.exe"
# SUBTRACT LINK32 /profile /incremental:no

!ELSEIF  "$(CFG)" == "SaUnicode - Win32 URelease"

# PROP BASE Use_MFC 2
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "URelease"
# PROP BASE Intermediate_Dir "URelease"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\Output\Release"
# PROP Intermediate_Dir "URelease"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W4 /Gm /GX /Zi /Od /I "." /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "WINDOWS_PLATFORM" /D "_AFXDLL" /D WINVER=0x400 /D _WIN32_IE=0x0400 /D "_UNICODE" /FR /YX"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W4 /Gm /Gi /GX /Zi /Ob2 /I "." /I "..\lib" /D "_WINDOWS" /D "WINDOWS_PLATFORM" /D "_AFXDLL" /D WINVER=0x400 /D _WIN32_IE=0x0400 /D "_UNICODE" /D "WIN32" /D "NDEBUG" /FR /YX"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "AFX_TARG_NEU" /d "_AFXDLL" /d "AFX_RESOURCE_DLL"
# ADD RSC /l 0x409 /d "AFX_TARG_NEU" /d "_AFXDLL" /d "AFX_RESOURCE_DLL" /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 mfc42ud.lib mfco42ud.lib mfcs42ud.lib msvcrtd.lib msvcprtd.lib comdlg32.lib user32.lib gdi32.lib kernel32.lib advapi32.lib oldnames.lib winmm.lib version.lib imm32.lib ole32.lib oleaut32.lib uuid.lib /nologo /stack:0x10000 /entry:"wWinMainCRTStartup" /subsystem:windows /map /debug /machine:IX86 /nodefaultlib /out:"UDebug/SA2.exe"
# SUBTRACT BASE LINK32 /profile /incremental:no
# ADD LINK32 mfc42u.lib mfcs42u.lib msvcrt.lib msvcprt.lib comdlg32.lib user32.lib gdi32.lib kernel32.lib advapi32.lib oldnames.lib winmm.lib version.lib imm32.lib ole32.lib oleaut32.lib uuid.lib comsupp.lib HtmlHelp\htmlhelp.lib shell32.lib /nologo /stack:0x100000 /entry:"wWinMainCRTStartup" /subsystem:windows /map /debug /machine:IX86 /nodefaultlib /out:"..\..\Output\Release\SA.exe" /pdbtype:sept
# SUBTRACT LINK32 /profile /incremental:no

!ENDIF 

# Begin Target

# Name "SaUnicode - Win32 UDebug"
# Name "SaUnicode - Win32 URelease"
# Begin Group "Source Files"

# PROP Default_Filter ""
# Begin Group "Processes"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Process\Butterworth.cpp
# End Source File
# Begin Source File

SOURCE=.\Process\FormantTracker.cpp
# End Source File
# Begin Source File

SOURCE=.\Dsp\Lpc.cpp
# End Source File
# Begin Source File

SOURCE=.\Process\sa_p_3dPitch.cpp
# End Source File
# Begin Source File

SOURCE=.\Process\SA_P_CHA.CPP
# End Source File
# Begin Source File

SOURCE=.\Process\sa_p_cor.cpp
# End Source File
# Begin Source File

SOURCE=.\Process\SA_P_CPI.CPP
# End Source File
# Begin Source File

SOURCE=.\Process\sa_p_doc.cpp
# End Source File
# Begin Source File

SOURCE=.\Process\SA_P_DUR.CPP
# End Source File
# Begin Source File

SOURCE=.\Process\SA_P_FMT.CPP
# End Source File
# Begin Source File

SOURCE=.\Process\SA_P_FRA.CPP
# End Source File
# Begin Source File

SOURCE=.\Process\Sa_p_glo.cpp
# End Source File
# Begin Source File

SOURCE=.\Process\Sa_p_gra.cpp
# End Source File
# Begin Source File

SOURCE=.\Process\sa_p_InstantaneousPower.cpp
# End Source File
# Begin Source File

SOURCE=.\Process\Sa_p_lou.cpp
# End Source File
# Begin Source File

SOURCE=.\Process\sa_p_mel.cpp
# End Source File
# Begin Source File

SOURCE=.\Process\Sa_p_pit.cpp
# End Source File
# Begin Source File

SOURCE=.\Process\sa_p_poa.cpp
# End Source File
# Begin Source File

SOURCE=.\Process\SA_P_RAT.CPP
# End Source File
# Begin Source File

SOURCE=.\Process\Sa_p_raw.cpp
# End Source File
# Begin Source File

SOURCE=.\Process\sa_p_sfmt.cpp
# End Source File
# Begin Source File

SOURCE=.\Process\Sa_p_spg.cpp
# End Source File
# Begin Source File

SOURCE=.\Process\SA_P_SPI.CPP
# End Source File
# Begin Source File

SOURCE=.\Process\Sa_p_spu.cpp
# End Source File
# Begin Source File

SOURCE=.\Process\SA_P_TWC.CPP
# End Source File
# Begin Source File

SOURCE=.\Process\sa_p_wavelet.cpp
# End Source File
# Begin Source File

SOURCE=.\Process\SA_P_ZCR.CPP
# End Source File
# Begin Source File

SOURCE=.\Process\Sa_proc.cpp
# End Source File
# Begin Source File

SOURCE=.\Process\SA_W_ADJ.CPP
# End Source File
# Begin Source File

SOURCE=.\Process\SA_W_EQU.CPP
# End Source File
# Begin Source File

SOURCE=.\Process\SA_W_PAS.CPP
# End Source File
# Begin Source File

SOURCE=.\Process\SA_W_REV.CPP
# End Source File
# Begin Source File

SOURCE=.\Dsp\Wavewarp.cpp
# End Source File
# End Group
# Begin Group "Graphs"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Clip.cpp
# End Source File
# Begin Source File

SOURCE=.\Partiture.cpp
# End Source File
# Begin Source File

SOURCE=.\PickOver.cpp
# End Source File
# Begin Source File

SOURCE=.\SA_Annot.cpp
# End Source File
# Begin Source File

SOURCE=.\SA_Cursr.cpp
# End Source File
# Begin Source File

SOURCE=.\sa_g_3dPitch.cpp
# End Source File
# Begin Source File

SOURCE=.\SA_G_Cha.cpp
# End Source File
# Begin Source File

SOURCE=.\SA_G_CPi.cpp
# End Source File
# Begin Source File

SOURCE=.\Sa_G_Dur.cpp
# End Source File
# Begin Source File

SOURCE=.\Sa_g_glo.cpp
# End Source File
# Begin Source File

SOURCE=.\Sa_g_gra.cpp
# End Source File
# Begin Source File

SOURCE=.\Sa_g_lou.cpp
# End Source File
# Begin Source File

SOURCE=.\Sa_g_mag.cpp
# End Source File
# Begin Source File

SOURCE=.\sa_g_mel.cpp
# End Source File
# Begin Source File

SOURCE=.\Sa_g_pit.cpp
# End Source File
# Begin Source File

SOURCE=.\Sa_g_poa.cpp
# End Source File
# Begin Source File

SOURCE=.\Sa_g_rat.cpp
# End Source File
# Begin Source File

SOURCE=.\Sa_g_raw.cpp
# End Source File
# Begin Source File

SOURCE=.\sa_g_rec.cpp
# End Source File
# Begin Source File

SOURCE=.\Sa_g_spe.cpp
# End Source File
# Begin Source File

SOURCE=.\Sa_g_spg.cpp
# ADD CPP /O2
# End Source File
# Begin Source File

SOURCE=.\Sa_g_spi.cpp
# End Source File
# Begin Source File

SOURCE=.\Sa_g_spu.cpp
# End Source File
# Begin Source File

SOURCE=.\SA_G_Stf.cpp
# End Source File
# Begin Source File

SOURCE=.\Sa_g_twc.cpp
# End Source File
# Begin Source File

SOURCE=.\sa_g_wavelet.cpp
# End Source File
# Begin Source File

SOURCE=.\Sa_g_zcr.cpp
# End Source File
# Begin Source File

SOURCE=.\Sa_grap2.cpp
# End Source File
# Begin Source File

SOURCE=.\Sa_graph.cpp
# End Source File
# Begin Source File

SOURCE=.\Sa_gz3d.cpp
# End Source File
# Begin Source File

SOURCE=.\Sa_gzpol.cpp
# End Source File
# Begin Source File

SOURCE=.\Sa_mplot.cpp
# End Source File
# Begin Source File

SOURCE=.\Sa_plot.cpp
# End Source File
# End Group
# Begin Group "Workbench"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Sa_w_dlg.cpp
# End Source File
# Begin Source File

SOURCE=.\Sa_w_doc.cpp
# End Source File
# Begin Source File

SOURCE=.\Sa_wbch.cpp
# End Source File
# End Group
# Begin Group "Settings"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Colors.cpp
# End Source File
# Begin Source File

SOURCE=.\Grid.cpp
# End Source File
# Begin Source File

SOURCE=.\Param.cpp
# End Source File
# End Group
# Begin Group "IPA Chart"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Chrchart\Ch_dlg.cpp
# End Source File
# Begin Source File

SOURCE=.\Chrchart\Ch_dlwnd.cpp
# End Source File
# Begin Source File

SOURCE=.\Chrchart\Ch_table.cpp
# End Source File
# End Group
# Begin Group "Dialogs"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\advancedParameters.cpp
# End Source File
# Begin Source File

SOURCE=.\autoRecorder.cpp
# End Source File
# Begin Source File

SOURCE=.\DlgAudio.cpp
# End Source File
# Begin Source File

SOURCE=.\DlgVowelFormants.cpp
# End Source File
# Begin Source File

SOURCE=.\FileInformation.cpp
# End Source File
# Begin Source File

SOURCE=.\FileOpen.cpp
# End Source File
# Begin Source File

SOURCE=.\FnKeys.cpp
# End Source File
# Begin Source File

SOURCE=.\graphsMagnify.cpp
# End Source File
# Begin Source File

SOURCE=.\graphsParameters.cpp
# End Source File
# Begin Source File

SOURCE=.\graphsTypes.cpp
# End Source File
# Begin Source File

SOURCE=.\playerRecorder.cpp
# End Source File
# Begin Source File

SOURCE=.\PrintOpt.cpp
# End Source File
# Begin Source File

SOURCE=.\Result.cpp
# End Source File
# Begin Source File

SOURCE=.\SA_DlWnd.cpp
# End Source File
# Begin Source File

SOURCE=.\SA_Edit.cpp
# End Source File
# Begin Source File

SOURCE=.\SA_Exprt.cpp
# End Source File
# Begin Source File

SOURCE=.\SA_Find.cpp
# End Source File
# Begin Source File

SOURCE=.\Sa_start.cpp
# End Source File
# Begin Source File

SOURCE=.\saveAsOptions.cpp
# End Source File
# Begin Source File

SOURCE=.\Targview.cpp
# End Source File
# Begin Source File

SOURCE=.\toolsOptions.cpp
# End Source File
# Begin Source File

SOURCE=.\waveformGenerator.cpp
# End Source File
# End Group
# Begin Group "ActiveX"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ActiveX\FlexEditGrid.cpp
# End Source File
# Begin Source File

SOURCE=.\ActiveX\font.cpp
# End Source File
# Begin Source File

SOURCE=.\ActiveX\mshflexgrid.cpp
# End Source File
# Begin Source File

SOURCE=.\ActiveX\picture.cpp
# End Source File
# Begin Source File

SOURCE=.\ActiveX\recordset.cpp
# End Source File
# End Group
# Begin Group "Misc"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;hpj;bat"
# Begin Source File

SOURCE=.\CSaString.cpp
# End Source File
# Begin Source File

SOURCE=.\MainFrm.cpp
# End Source File
# Begin Source File

SOURCE=.\mixer.cpp
# End Source File
# Begin Source File

SOURCE=.\Riff.cpp
# End Source File
# Begin Source File

SOURCE=.\Sa.cpp
# End Source File
# Begin Source File

SOURCE=.\SA.rc
# End Source File
# Begin Source File

SOURCE=.\SA_AnSel.cpp
# End Source File
# Begin Source File

SOURCE=.\SA_Asert.cpp
# End Source File
# Begin Source File

SOURCE=.\SA_CDib.cpp
# End Source File
# Begin Source File

SOURCE=.\Sa_Doc.cpp
# End Source File
# Begin Source File

SOURCE=.\SA_DPlot.cpp
# End Source File
# Begin Source File

SOURCE=.\sa_ipa.cpp
# End Source File
# Begin Source File

SOURCE=.\Sa_menu.cpp
# End Source File
# Begin Source File

SOURCE=.\Sa_minic.cpp
# End Source File
# Begin Source File

SOURCE=.\Sa_sbar.cpp
# End Source File
# Begin Source File

SOURCE=.\Sa_segm.cpp
# End Source File
# Begin Source File

SOURCE=.\SA_View.cpp
# End Source File
# Begin Source File

SOURCE=.\SA_View2.cpp
# End Source File
# Begin Source File

SOURCE=.\Sa_wave.cpp
# End Source File
# Begin Source File

SOURCE=.\SelfTest.cpp
# End Source File
# Begin Source File

SOURCE=.\Stdafx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\stpwatch.cpp
# End Source File
# Begin Source File

SOURCE=.\TaskBar.cpp
# End Source File
# Begin Source File

SOURCE=.\Undoredo.cpp
# End Source File
# Begin Source File

SOURCE=.\UnicodeString.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\Synthesis\DlgKlattAll.cpp
# End Source File
# Begin Source File

SOURCE=.\Synthesis\DlgMbrola.cpp
# End Source File
# Begin Source File

SOURCE=.\Synthesis\DlgSynthesis.cpp
# End Source File
# Begin Source File

SOURCE=.\Synthesis\DlgVocalTract.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter ""
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\RES\1.ICO
# End Source File
# Begin Source File

SOURCE=.\RES\2A.ICO
# End Source File
# Begin Source File

SOURCE=.\RES\2B.ICO
# End Source File
# Begin Source File

SOURCE=.\Res\2c.ico
# End Source File
# Begin Source File

SOURCE=.\RES\3A.ICO
# End Source File
# Begin Source File

SOURCE=.\RES\3B.ICO
# End Source File
# Begin Source File

SOURCE=.\Res\3c.ico
# End Source File
# Begin Source File

SOURCE=.\RES\4A.ICO
# End Source File
# Begin Source File

SOURCE=.\RES\4B.ICO
# End Source File
# Begin Source File

SOURCE=.\Res\4c.ico
# End Source File
# Begin Source File

SOURCE=.\RES\5.ICO
# End Source File
# Begin Source File

SOURCE=.\RES\6A.ICO
# End Source File
# Begin Source File

SOURCE=.\RES\6B.ICO
# End Source File
# Begin Source File

SOURCE=.\Res\advanced.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\BPU.BMP
# End Source File
# Begin Source File

SOURCE=.\Res\down.ico
# End Source File
# Begin Source File

SOURCE=.\RES\ECHOU.BMP
# End Source File
# Begin Source File

SOURCE=.\RES\EQU.BMP
# End Source File
# Begin Source File

SOURCE=.\RES\FACE.BMP
# End Source File
# Begin Source File

SOURCE=.\RES\FINGERN.CUR
# End Source File
# Begin Source File

SOURCE=.\RES\FINGERNW.CUR
# End Source File
# Begin Source File

SOURCE=.\Res\hand_up.cur
# End Source File
# Begin Source File

SOURCE=.\RES\HANDN.CUR
# End Source File
# Begin Source File

SOURCE=.\RES\HPU.BMP
# End Source File
# Begin Source File

SOURCE=.\Res\idr_sa_a.ico
# End Source File
# Begin Source File

SOURCE=.\RES\idr_synthesis_vocal_tract.ico
# End Source File
# Begin Source File

SOURCE=.\RES\JAW.BMP
# End Source File
# Begin Source File

SOURCE=.\RES\KNOBH.BMP
# End Source File
# Begin Source File

SOURCE=.\RES\KNOBV.BMP
# End Source File
# Begin Source File

SOURCE=.\RES\LANDU.BMP
# End Source File
# Begin Source File

SOURCE=.\RES\LED.BMP
# End Source File
# Begin Source File

SOURCE=.\RES\LPU.BMP
# End Source File
# Begin Source File

SOURCE=.\MainFrm.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\MCAP_CL.BMP
# End Source File
# Begin Source File

SOURCE=.\RES\MCAP_MAX.BMP
# End Source File
# Begin Source File

SOURCE=.\RES\MCAP_MIN.BMP
# End Source File
# Begin Source File

SOURCE=.\RES\MCAP_RES.BMP
# End Source File
# Begin Source File

SOURCE=.\RES\MCAP_SYS.BMP
# End Source File
# Begin Source File

SOURCE=.\Res\MedFingerN.cur
# End Source File
# Begin Source File

SOURCE=.\Res\MedFingerNW.cur
# End Source File
# Begin Source File

SOURCE=.\Res\MedHandN.cur
# End Source File
# Begin Source File

SOURCE=.\Res\next.ico
# End Source File
# Begin Source File

SOURCE=.\RES\PORTU.BMP
# End Source File
# Begin Source File

SOURCE=.\Res\prev.ico
# End Source File
# Begin Source File

SOURCE=.\RES\RAWU.BMP
# End Source File
# Begin Source File

SOURCE=.\RES\RESULTU.BMP
# End Source File
# Begin Source File

SOURCE=.\RES\REVU.BMP
# End Source File
# Begin Source File

SOURCE=.\RES\SA.ICO
# End Source File
# Begin Source File

SOURCE=.\res\sa.rc2
# End Source File
# Begin Source File

SOURCE=.\RES\SA_DOC.ICO
# End Source File
# Begin Source File

SOURCE=.\RES\SA_WB.ICO
# End Source File
# Begin Source File

SOURCE=.\RES\SAS.ICO
# End Source File
# Begin Source File

SOURCE=.\RES\SIL.BMP
# End Source File
# Begin Source File

SOURCE=".\Res\Splash-2.bmp"
# End Source File
# Begin Source File

SOURCE=.\Res\taskbar.bmp
# End Source File
# Begin Source File

SOURCE=.\RES\TONGUE.BMP
# End Source File
# Begin Source File

SOURCE=.\RES\TOOLBAR.BMP
# End Source File
# Begin Source File

SOURCE=.\Res\up.ico
# End Source File
# End Group
# End Target
# End Project
# Section SA : {47A738F1-7FAF-11D0-B148-00A0C922E820}
# 	2:5:Class:CAdodc
# 	2:10:HeaderFile:activex\adodc.h
# 	2:8:ImplFile:activex\adodc.cpp
# End Section
# Section SA : {0ECD9B64-23AA-11D0-B351-00A0C9055D8E}
# 	2:21:DefaultSinkHeaderFile:activex\mshflexgrid.h
# 	2:16:DefaultSinkClass:CMSHFlexGrid
# End Section
# Section SA : {BEF6E003-A874-101A-8BBA-00AA00300CAB}
# 	2:5:Class:COleFont
# 	2:10:HeaderFile:activex\font.h
# 	2:8:ImplFile:activex\font.cpp
# End Section
# Section SA : {00000504-0000-0010-8000-00AA006D2EA4}
# 	2:5:Class:CProperties
# 	2:10:HeaderFile:activex\properties.h
# 	2:8:ImplFile:activex\properties.cpp
# End Section
# Section Sa : {2B50EB44-7CE7-11D4-B2B4-0050DA6A58A6}
# 	2:5:Class:CActiveX
# 	2:10:HeaderFile:activex1.h
# 	2:8:ImplFile:activex1.cpp
# End Section
# Section SA : {00000569-0000-0010-8000-00AA006D2EA4}
# 	2:5:Class:CField
# 	2:10:HeaderFile:activex\field.h
# 	2:8:ImplFile:activex\field.cpp
# End Section
# Section SA : {00000564-0000-0010-8000-00AA006D2EA4}
# 	2:5:Class:CFields
# 	2:10:HeaderFile:activex\fields.h
# 	2:8:ImplFile:activex\fields.cpp
# End Section
# Section SA : {00000503-0000-0010-8000-00AA006D2EA4}
# 	2:5:Class:CProperty
# 	2:10:HeaderFile:activex\property.h
# 	2:8:ImplFile:activex\property.cpp
# End Section
# Section SA : {7BF80981-BF32-101A-8BBB-00AA00300CAB}
# 	2:5:Class:CPicture
# 	2:10:HeaderFile:activex\picture.h
# 	2:8:ImplFile:activex\picture.cpp
# End Section
# Section SA : {67397AA3-7FB1-11D0-B148-00A0C922E820}
# 	2:21:DefaultSinkHeaderFile:activex\adodc.h
# 	2:16:DefaultSinkClass:CAdodc
# End Section
# Section SA : {0ECD9B62-23AA-11D0-B351-00A0C9055D8E}
# 	2:5:Class:CMSHFlexGrid
# 	2:10:HeaderFile:activex\mshflexgrid.h
# 	2:8:ImplFile:activex\mshflexgrid.cpp
# End Section
# Section SA : {0000050E-0000-0010-8000-00AA006D2EA4}
# 	2:5:Class:CRecordset1
# 	2:10:HeaderFile:activex\recordset.h
# 	2:8:ImplFile:activex\recordset.cpp
# End Section
# Section Sa : {2B50EB46-7CE7-11D4-B2B4-0050DA6A58A6}
# 	2:21:DefaultSinkHeaderFile:activex1.h
# 	2:16:DefaultSinkClass:CActiveX
# End Section
# Section SA : {00000556-0000-0010-8000-00AA006D2EA4}
# 	2:5:Class:C_Recordset
# 	2:10:HeaderFile:activex\_recordset.h
# 	2:8:ImplFile:activex\_recordset.cpp
# End Section
