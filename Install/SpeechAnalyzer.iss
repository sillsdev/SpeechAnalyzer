; Script generated by the Inno Setup Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

#define MyAppName "Speech Analyzer MSEA"
#define MyAppVersion "3.1.0.134"
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
DefaultDirName={pf}\SIL\{#MyAppName}
DefaultGroupName={#MyAppName}
OutputDir=C:\Working\SIL\MSEA\Install\Output
OutputBaseFilename=SpeechAnalyzerMSEA
Compression=lzma
SolidCompression=yes
SetupLogging=yes
PrivilegesRequired=admin
UsePreviousAppDir=no

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
Source: "C:\Working\SIL\MSEA\Output\Release\SA_DEU.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Working\SIL\MSEA\Output\Release\SilEncConverters40.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Working\SIL\MSEA\Output\Release\SAUtils.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Working\SIL\MSEA\Output\Release\yeti.mmedia.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Working\SIL\MSEA\Output\Release\yeti.wmfsdk.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Working\SIL\MSEA\Output\Release\zGraph.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Working\SIL\MSEA\Src\Release Notes.txt"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Working\SIL\MSEA\DistFiles\usp10.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Working\SIL\MSEA\DistFiles\Microsoft\vcredist_x86.exe"; DestDir: "{app}\components"; Flags: ignoreversion; Check: VCRedistCheck()
Source: "C:\Working\SIL\MSEA\DistFiles\Bmp2png.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Working\SIL\MSEA\DistFiles\xerces\xerces-c_3_1.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Working\SIL\MSEA\DistFiles\xerces\notice"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Working\SIL\MSEA\DistFiles\xerces\license"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Working\SIL\MSEA\DistFiles\What's New.pdf"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Working\SIL\MSEA\DistFiles\Speech_Analyzer_Help.chm"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Working\SIL\MSEA\DistFiles\Using_Help.chm"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Working\SIL\MSEA\DistFiles\uriparser\uriparser_copying"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\Working\SIL\MSEA\DistFiles\iso639.txt"; DestDir: "{app}"; Flags: ignoreversion
;samples
Source: "C:\Working\SIL\MSEA\DistFiles\Samples\*"; DestDir: "{userdocs}\Speech Analyzer\Samples"; Flags: ignoreversion
Source: "C:\Working\SIL\MSEA\DistFiles\Samples\Music\*"; DestDir: "{userdocs}\Speech Analyzer\Samples\Music"; Flags: ignoreversion
;training
Source: "C:\Working\SIL\MSEA\DistFiles\Training\*.pdf"; DestDir: "{app}\Training"; Flags: ignoreversion
;fonts
Source: "C:\Working\SIL\MSEA\DistFiles\Fonts\DoulosSILR.ttf"; DestDir: "{fonts}"; FontInstall: "Doulos SIL"; Flags: onlyifdoesntexist uninsneveruninstall
Source: "C:\Working\SIL\MSEA\Lib\Fonts\Musique\MusiqueU.ttf"; DestDir: "{fonts}"; FontInstall: "Musique"; Flags: onlyifdoesntexist uninsneveruninstall
Source: "C:\Working\SIL\MSEA\Lib\Fonts\Musique\Musique Unicode.ttf"; DestDir: "{fonts}"; FontInstall: "Musique Unicode"; Flags: onlyifdoesntexist uninsneveruninstall
;dtds for musicxml
Source: "C:\Working\SIL\MSEA\DistFiles\MusicXML\dtds\*" ; DestDir: "{app}\dtds"; Flags: ignoreversion

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{commondesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon
Name: "{userappdata}\Microsoft\Internet Explorer\Quick Launch\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: quicklaunchicon

;.NET 3.5 = .NET 2.0 + .NET 3.5
;Due to the way .NET 3.5 is "just" an add-on to .NET 3.0 which is "just" an add-on to .NET 2.0, 
; the tools in the .NET 2.0 directory are still the ones to use.  
; The CLR version number is the same for all three of these frameworks (2.0.50727).
[Run]
Filename: "{app}\components\vcredist_x86.exe"; Parameters: "/q"; WorkingDir: "{app}\components"; Flags: waituntilterminated skipifdoesntexist; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; StatusMsg: "Installing Microsoft Visual C++ 2010 SP1 Redistributable"
Filename: "{win}\Microsoft.NET\Framework\v2.0.50727\regasm.exe"; Parameters: "SAUtils.dll /tlb:SAUtils.tlb /codebase"; WorkingDir: "{app}"; Flags: waituntilterminated runhidden; Description: "Registering SpeechToolsUtils"
Filename: "{win}\Microsoft.NET\Framework\v2.0.50727\regasm.exe"; Parameters: "yeti.mmedia.dll /tlb:yeti.mmedia.tlb /codebase"; WorkingDir: "{app}"; Flags: waituntilterminated runhidden; Description: "Registering yeti.mmedia"
Filename: "{win}\Microsoft.NET\Framework\v2.0.50727\regasm.exe"; Parameters: "yeti.wmfsdk.dll /tlb:yeti.wmfsdk.tlb /codebase"; WorkingDir: "{app}"; Flags: waituntilterminated runhidden; Description: "Registering yeti.wmfsdk"

[UninstallRun]
Filename: "{win}\Microsoft.NET\Framework\v2.0.50727\regasm.exe"; Parameters: "SAUtils.dll /unregister"; WorkingDir: "{app}"; Flags: waituntilterminated runhidden
Filename: "{win}\Microsoft.NET\Framework\v2.0.50727\regasm.exe"; Parameters: "yeti.mmedia.dll /unregister"; WorkingDir: "{app}"; Flags: waituntilterminated runhidden
Filename: "{win}\Microsoft.NET\Framework\v2.0.50727\regasm.exe"; Parameters: "yeti.wmfsdk.dll /unregister"; WorkingDir: "{app}"; Flags: waituntilterminated runhidden

[Dirs]
Name: "{app}\components"
Name: "{app}\Samples"
Name: "{app}\Samples\Music"
Name: "{app}\Training"

[InstallDelete]
Type: files; Name: "{app}\SA MSEA Release Notes.txt"
Type: files; Name: "{app}\SA MSEA Roadmap.txt"
Type: files; Name: "{app}\Roadmap.txt"

[UninstallDelete]
Type: files; Name: "{app}\SAUtils.tlb"
Type: files; Name: "{app}\yeti.mmedia.tlb"
Type: files; Name: "{app}\yeti.wmfsdk.tlb"

[Registry]
Root: HKLM64; Subkey: "Software\SIL\Speech Analyzer"; ValueType: string; ValueName: "Location"; ValueData: "{app}\SA.exe"; Check: IsWin64 
Root: HKLM32; Subkey: "Software\SIL\Speech Analyzer"; ValueType: string; ValueName: "Location"; ValueData: "{app}\SA.exe"

[Code]
function VCRedistCheck(): Boolean;
begin
  Result := (FileExists(ExpandConstant('{win}\WinSxS\x86_Microsoft.VC90.MFC_1fc8b3b9a1e18e3b_9.0.30729.1_x-ww_405b0943\mfc90.dll'))=false);
end;

/////////////////////////////////////////////////////////////////////
function GetMSEAUninstallString(): String;
var
  sUnInstPath: String;
  sUnInstallString: String;
begin
  sUnInstPath := ExpandConstant('Software\Microsoft\Windows\CurrentVersion\Uninstall\{#emit SetupSetting("AppId")}_is1');
  sUnInstallString := '';
  if not RegQueryStringValue(HKLM, sUnInstPath, 'UninstallString', sUnInstallString) then
    RegQueryStringValue(HKCU, sUnInstPath, 'UninstallString', sUnInstallString);
  Result := sUnInstallString;
  Log('uninstallstring for msea:');
  Log(sUnInstallString);
end;

/////////////////////////////////////////////////////////////////////
function Has31UninstallString(): Boolean;
var
  sUnInstPath: String;
  sUnInstallString: String;
begin
  // old 3.1 product id
  sUnInstPath := 'Software\Microsoft\Windows\CurrentVersion\Uninstall\{D99E9365-BB4F-4430-875C-BD5516EE92DA}';
  sUnInstallString := '';
  if not RegQueryStringValue(HKLM, sUnInstPath, 'UninstallString', sUnInstallString) then
    RegQueryStringValue(HKCU, sUnInstPath, 'UninstallString', sUnInstallString);
  Log('uninstallstring for 31:');
  Log(sUnInstallString);
  if (sUnInstallString <> '') then
    begin
      Result := TRUE
      Log('result=true')
    end
  else
    begin
      Result := FALSE;
      Log('result=false')
    end;
end;

/////////////////////////////////////////////////////////////////////
function UnInstallMSEA(): Boolean;
var
  sUnInstallString: String;
  iResultCode: Integer;
begin

  // default return value
  Result := FALSE;

  // get the uninstall string of the old app
  sUnInstallString := GetMSEAUninstallString();
  if sUnInstallString <> '' then begin
    Log('uninstalling msea');
    sUnInstallString := RemoveQuotes(sUnInstallString);
    if Exec(sUnInstallString, '/SILENT /NORESTART /SUPPRESSMSGBOXES','', SW_SHOW, ewWaitUntilTerminated, iResultCode) then
      begin
        Log('uninstall succeeded');
        Result := TRUE;
      end
    else
      begin
        Log('unistall exec failed');
        Result := FALSE;
      end;
  end else
    Result := FALSE;
end;

/////////////////////////////////////////////////////////////////////
function UnInstall_3_1(): Boolean;
var
  iResultCode: Integer;
begin

  // default return value
  Result := FALSE;

  // get the uninstall string of the old app
  if Has31UninstallString() then begin
    Log('uninstalling 3.1');
    if Exec('MsiExec.exe', '/x {D99E9365-BB4F-4430-875C-BD5516EE92DA} /quiet /passive /norestart','', SW_SHOW, ewWaitUntilTerminated, iResultCode) then
      begin
        Log('uninstall succeeded');
        Result := TRUE;
      end
    else
      begin
        Log('unistall exec failed');
        Result := FALSE;
      end;
  end else
    Result := FALSE;

end;

/////////////////////////////////////////////////////////////////////
procedure CurStepChanged(CurStep: TSetupStep);
begin
  if (CurStep=ssInstall) then
  begin
    UnInstallMSEA();
    UnInstall_3_1();
  end;
end;