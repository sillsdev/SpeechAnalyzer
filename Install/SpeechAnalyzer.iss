; Script generated by the Inno Setup Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

#define MyAppName "Speech Analyzer MSEA Beta"
#define MyAppVersion "3.1.0.95"
#define MyAppPublisher "SIL International, Inc."
#define MyAppURL "http://www.speechanalyzer.sil.org/"
#define MyAppExeName "SA.exe"

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{E4ECCF52-56A6-47C9-9B4D-11B8C5CDBF29}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
;AppVerName={#MyAppName} {#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={pf}\{#MyAppName}
DefaultGroupName={#MyAppName}
OutputDir=C:\Working\SIL\MSEA\Install\Output
OutputBaseFilename=SpeechAnalyzerMSEABeta
Compression=lzma
SolidCompression=yes

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"
Name: "quicklaunchicon"; Description: "{cm:CreateQuickLaunchIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked; OnlyBelowVersion: 0,6.1

[Files]
Source: "C:\Working\SIL\MSEA\Output\Release\SA.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Working\SIL\MSEA\Output\Release\SA.exe"; DestDir: "{app}"; DestName: "SAServer.exe"; Flags: ignoreversion
Source: "C:\Working\SIL\MSEA\Output\Release\ECInterfaces.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Working\SIL\MSEA\Output\Release\mbrola.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Working\SIL\MSEA\Output\Release\SA_DSP.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Working\SIL\MSEA\Output\Release\SA_ENU.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Working\SIL\MSEA\Output\Release\SilEncConverters40.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Working\SIL\MSEA\Output\Release\SilUtils.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Working\SIL\MSEA\Output\Release\SpeechToolsUtils.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Working\SIL\MSEA\Output\Release\ST_Audio.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Working\SIL\MSEA\Output\Release\yeti.mmedia.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Working\SIL\MSEA\Output\Release\yeti.wmfsdk.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Working\SIL\MSEA\Output\Release\zGraph.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Working\SIL\MSEA\Src\SA MSEA Release Notes.txt"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Working\SIL\MSEA\DistFiles\usp10.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Working\SIL\MSEA\DistFiles\Microsoft\vcredist_x86.exe"; DestDir: "{app}\components"; Flags: ignoreversion; Check: VCRedistCheck()
Source: "C:\Working\SIL\MSEA\DistFiles\Bmp2png.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Working\SIL\MSEA\DistFiles\xerces\xerces-c_3_1.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Working\SIL\MSEA\DistFiles\xerces\notice"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Working\SIL\MSEA\DistFiles\xerces\license"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Working\SIL\MSEA\DistFiles\What's New.pdf"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Working\SIL\MSEA\DistFiles\Speech_Analyzer_Help.chm"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Working\SIL\MSEA\DistFiles\Using_Help.chm"; DestDir: "{app}"; Flags: ignoreversion
;samples
Source: "C:\Working\SIL\MSEA\DistFiles\Samples\*"; DestDir: "{app}\Samples"; Flags: ignoreversion
Source: "C:\Working\SIL\MSEA\DistFiles\Samples\Music\*"; DestDir: "{app}\Samples\Music"; Flags: ignoreversion
;training
Source: "C:\Working\SIL\MSEA\DistFiles\Training\*.pdf"; DestDir: "{app}\Training"; Flags: ignoreversion

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{commondesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon
Name: "{userappdata}\Microsoft\Internet Explorer\Quick Launch\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: quicklaunchicon

[Run]
Filename: "{app}\components\vcredist_x86.exe"; Parameters: "/q"; WorkingDir: "{app}\components"; Flags: waituntilterminated skipifdoesntexist; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; StatusMsg: "Installing Microsoft Visual C++ 2008 Redistributable"
Filename: "{win}\Microsoft.NET\Framework\v2.0.50727\regasm.exe"; Parameters: "SpeechToolsUtils.dll /tlb:SpeechToolsUtils.tlb /codebase"; WorkingDir: "{app}"; Flags: waituntilterminated runhidden; Description: "Registering SpeechToolsUtils"
Filename: "{win}\Microsoft.NET\Framework\v2.0.50727\regasm.exe"; Parameters: "ST_Audio.dll /tlb:ST_Audio.tlb /codebase"; WorkingDir: "{app}"; Flags: waituntilterminated runhidden; Description: "Registering ST_Audio"
Filename: "{win}\Microsoft.NET\Framework\v2.0.50727\regasm.exe"; Parameters: "yeti.mmedia.dll /tlb:yeti.mmedia.tlb /codebase"; WorkingDir: "{app}"; Flags: waituntilterminated runhidden; Description: "Registering yeti.mmedia"
Filename: "{win}\Microsoft.NET\Framework\v2.0.50727\regasm.exe"; Parameters: "yeti.wmfsdk.dll /tlb:yeti.wmfsdk.tlb /codebase"; WorkingDir: "{app}"; Flags: waituntilterminated runhidden; Description: "Registering yeti.wmfsdk"

[UninstallRun]
Filename: "{win}\Microsoft.NET\Framework\v2.0.50727\regasm.exe"; Parameters: "SpeechToolsUtils.dll /unregister"; WorkingDir: "{app}"; Flags: waituntilterminated runhidden
Filename: "{win}\Microsoft.NET\Framework\v2.0.50727\regasm.exe"; Parameters: "ST_Audio.dll /unregister"; WorkingDir: "{app}"; Flags: waituntilterminated runhidden
Filename: "{win}\Microsoft.NET\Framework\v2.0.50727\regasm.exe"; Parameters: "yeti.mmedia.dll /unregister"; WorkingDir: "{app}"; Flags: waituntilterminated runhidden
Filename: "{win}\Microsoft.NET\Framework\v2.0.50727\regasm.exe"; Parameters: "yeti.wmfsdk.dll /unregister"; WorkingDir: "{app}"; Flags: waituntilterminated runhidden

[Dirs]
Name: "{app}\components"
Name: "{app}\Samples"
Name: "{app}\Samples\Music"
Name: "{app}\Training"

[UninstallDelete]
Type: files; Name: "{app}\SpeechToolsUtils.tlb"
Type: files; Name: "{app}\ST_Audio.tlb"
Type: files; Name: "{app}\yeti.mmedia.tlb"
Type: files; Name: "{app}\yeti.wmfsdk.tlb"

[Code]
function VCRedistCheck(): Boolean;
begin
  Result := (FileExists(ExpandConstant('{win}\WinSxS\x86_Microsoft.VC90.MFC_1fc8b3b9a1e18e3b_9.0.30729.1_x-ww_405b0943\mfc90.dll'))=false);
end;
