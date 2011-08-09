#Build Switches
# /DVERSION=[Version]
# /DPLATFORM=[x86|x64]

# Switches:
# /INI [IniFile] Use settings from INI file
# /S Silent
# /NetworkUsername Username - Use this username (Overrides INI settings)
# /NetworkPassword Username - Use this password (Overrides INI settings)
# /WpkgCommand Path_to_wpkg.js [/extra_switches ...] - Use this path to wpkg.js (Overrides INI settings)
# /DisableViaLGP

# Exit codes:
# 0: OK
# 1: Cancel
# 2: Not administrator
# -1: Error

!define GUID "{b48b962c-f288-4e6c-9c7c-dd30c73effb3}"
!define PRODUCT_NAME "Wpkg-GP ${VERSION} (${PLATFORM})"
!define PRODUCT_SHORTNAME "Wpkg-GP"
!define PRODUCT_DESCRIPTION "A tool for automatic software deployment"
!define PRODUCT_VERSION "${VERSION}"
!define PRODUCT_PUBLISHER "The Wpkg-GP team"
!define PRODUCT_WEB_SITE "http://wpkg-gp.googlecode.com"
!define PRODUCT_COPYRIGHT "© The Wpkg-GP team"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${GUID}"
!define PRODUCT_UNINST_ROOT_KEY "HKLM"

!define GPE_GUID "{A9B8D792-F454-11DE-BA92-FDCF56D89593}"
!define GPE_REG_KEY "SOFTWARE\Microsoft\Windows NT\CurrentVersion\Winlogon\GPExtensions\${GPE_GUID}"

!addplugindir "setup\plugin"

!include MUI2.nsh
!include InstallOptions.nsh
!include x64.nsh
!include LogicLib.nsh
!include FileFunc.nsh
!include Library.nsh
!include Sections.nsh


!include "setup\include\StrContains.nsi"
!include "setup\include\Registry.nsh"
!include "setup\include\IsUserAdmin.nsi"
!include "setup\include\servicelib.nsi"

Var /GLOBAL CURRENTPLATFORM
Var /GLOBAL NetworkUsername
Var /GLOBAL NetworkPassword
Var /GLOBAL WpkgCommand
Var /GLOBAL EnableViaLGP
Var /GLOBAL INI
Var /GLOBAL Features

# Dynamic file lists
!system "setup\include\list_files_in_dist.exe /Platform ${PLATFORM}"
!include "setup\include\file-${PLATFORM}.nsi"
!include "setup\include\delete-${PLATFORM}.nsi"


!define MUI_UNICON "setup\wpkg-gp.ico"
!define MUI_FINISHPAGE_NOAUTOCLOSE
!define MUI_UNFINISHPAGE_NOAUTOCLOSE
!insertmacro MUI_PAGE_LICENSE "setup\License.rtf"
!insertmacro MUI_PAGE_COMPONENTS
Page custom GroupPolicySettingsPage GroupPolicySettingsPageCallback
Page custom WpkgSettingsPage WpkgSettingsPageCallback
Page custom NetworkUserPage NetworkUserPageCallback
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH

!insertmacro MUI_LANGUAGE "English"

#
# General Attributes
#
CrcCheck off # CRC check generates random errors
Icon "setup\wpkg-gp.ico"
!if ${PLATFORM} == "x64"
  InstallDir "$PROGRAMFILES64\Wpkg-GP\"
!else
  InstallDir "$PROGRAMFILES\Wpkg-GP\"
!endif
OutFile "Wpkg-GP-${VERSION}_${PLATFORM}.exe"
XpStyle On
RequestExecutionLevel admin
Name "${PRODUCT_NAME}"

VIAddVersionKey "ProductName" "${PRODUCT_NAME}"
VIAddVersionKey "Comments" "${PRODUCT_DESCRIPTION}"
VIAddVersionKey "CompanyName" "${PRODUCT_PUBLISHER}"
VIAddVersionKey "LegalCopyright" "${PRODUCT_COPYRIGHT}"
VIAddVersionKey "FileDescription" "Installer"
VIAddVersionKey "FileVersion" "${VERSION}"
VIProductVersion "${VERSION}.0.0"

!define ProductKeyVer0.5 {A70E99ED-87C6-4142-88A7-8491459494A2}
!define ProductKeyVer0.6 1f97d86c-2968-4def-9419-9d9cb349c339
!define ProductKeyVer0.7 f4fefaeb-9305-42dd-a6a6-dcdf0664a642
!define ProductKeyVer0.8 aedc7b28-3b50-411b-b218-c4f2264a5adc
!define ProductKeyVer0.9 aa05d116-5a76-4bae-bd5f-220a3ba59abd
!define ProductKeyVer0.10 aa05d116-5a76-4bae-bd5f-220a3ba59abd

!macro CheckForMsi GUID
  ClearErrors
  ReadRegStr $0 HKLM SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\{${GUID}} "DisplayVersion"
  ${IfNot} ${Errors}
    MessageBox MB_OK "Version $0 is installed from MSI. Will try to uninstall." /SD IDOK
    ${If} ${Silent}
      ExecWait "Msiexec /qn /x{${GUID}}"
    ${Else}
      ExecWait "Msiexec /passive /x{${GUID}}"
    ${EndIf}
  ${EndIf}
!macroend

Function Usage
  push $0
  StrCpy $0 "Switches:$\r$\n"
  StrCpy $0 "$0/INI [IniFile] Use settings from INI file$\r$\n"
  StrCpy $0 "$0/S - Silent$\r$\n"
  StrCpy $0 "$0/NetworkUsername [Username] - Use this username$\r$\n"
  StrCpy $0 "$0/NetworkPassword [Password] - Use this password$\r$\n"
  StrCpy $0 "$0/WpkgCommand [Path_to_wpkg.js] - Use this path for wpkg.js$\r$\n"
  StrCpy $0 "$0     Examples:$\r$\n"
  StrCpy $0 "$0     /WpkgCommand \\srv01\install\wpkg.js$\r$\n"
  StrCpy $0 '$0     /WpkgCommand "\\srv01\path with space\wpkg.js"$\r$\n'
  StrCpy $0 '$0     /WpkgCommand "\\srv01\path_to_cmd_file_with_additional_parameters.cmd"$\r$\n'
  StrCpy $0 "$0/DisableViaLGP Do not run Wpkg-GP via local group policies$\r$\n"
  StrCpy $0 "$0/Features Client|Adm|MSITool - Install only these features$\r$\n"
  StrCpy $0 "$0     You may select multiple features"
  MessageBox MB_OK $0
  pop $0
FunctionEnd

Function .onInit
  # Check if is administrator
  !insertmacro IsUserAdmin $0
  
  ${If} $0 != 1
    MessageBox MB_OK|MB_ICONEXCLAMATION  "You need to be an administrator to install this product" /SD IDOK
    SetErrorLevel 2
    Abort "You need to be an administrator to install this product"
  ${EndIf}
  
  # Check which platform we are running on
  ${If} ${RunningX64}
    StrCpy $CURRENTPLATFORM "x64"
  ${Else}
    StrCpy $CURRENTPLATFORM "x86"
  ${EndIf}
  
  ${If} $CURRENTPLATFORM != ${PLATFORM}
    MessageBox MB_OK|MB_ICONEXCLAMATION "This package is for ${PLATFORM} only, you are running it on $CURRENTPLATFORM" /SD IDOK
    SetErrorLevel 3
    Abort "This package is for ${PLATFORM} only, you are running it on $CURRENTPLATFORM"
  ${EndIf}

  ${If} $CURRENTPLATFORM == "x64"
    SetRegView 64
  ${EndIf}
  
  ${GetParameters} $R0
  
  # Check if /? flag is set
  ClearErrors
  ${GetOptions} $R0 "/?"    $R1
  ${IfNot} ${Errors}
    call Usage
    Abort
  ${EndIf}
  
  # Get InstallDir
  ClearErrors
  ReadRegStr $0 HKLM "SOFTWARE\Wpkg-GP" "InstallPath"
  ${IfNot} ${Errors}
    Strcpy $INSTDIR $0
  ${EndIf}

  # Check if <= 0.10 (msi install)
  !insertmacro CheckForMsi ${ProductKeyVer0.5}
  !insertmacro CheckForMsi ${ProductKeyVer0.6}
  !insertmacro CheckForMsi ${ProductKeyVer0.7}
  !insertmacro CheckForMsi ${ProductKeyVer0.8}
  !insertmacro CheckForMsi ${ProductKeyVer0.9}
  !insertmacro CheckForMsi ${ProductKeyVer0.10}


  # Check if already is installed
  Var /GLOBAL IsUpgrade
  Var /GLOBAL PreviousVersion
  ClearErrors
  ReadRegStr $PreviousVersion HKLM "${PRODUCT_UNINST_KEY}" "DisplayVersion"
  ${If} $PreviousVersion == ""
    StrCpy $IsUpgrade 0
  ${Else}
    StrCpy $IsUpgrade 1
  ${EndIf}

  # Check if parameters provided are OK
  ${GetOptions} $R0 "/NetworkUsername" $NetworkUsername
  ${GetOptions} $R0 "/NetworkPassword" $NetworkPassword
  ${GetOptions} $R0 "/WpkgCommand"     $WpkgCommand
  ${GetOptions} $R0 "/INI"             $INI
  ${GetOptions} $R0 "/Features"        $Features

  Call GetFeatures
  
  # Check if EnableViaLGP flag is set
  ClearErrors
  ${GetOptions} $R0 "/DisableViaLGP"    $R1
  ${IfNot} ${Errors}
    StrCpy $EnableViaLGP 0
  ${EndIf}
  
  
  # Check for INI file to be able to extract the file
  # It seems that ReadINIStr does not work correctly when reading a file without absolute path
  ClearErrors
  ReadINIStr $WpkgCommand $INI "WpkgConfig" "WpkgCommand"
  ${If} ${Errors}
  ${AndIf} $INI != ""
    #Maybe the INI file is in the Current Directory
    System::Call "kernel32::GetCurrentDirectory(i ${NSIS_MAX_STRLEN}, t .r0)"
    StrCpy $INI "$0\$INI"
    ${IfNot} ${FileExists} $INI
      Abort "Could not open ini file: $INI"
    ${EndIF}
  ${EndIf}
  
   # Read INI settings from provided INI file
  ${If} $INI != ""
    ${If} $WpkgCommand == ""
      ReadINIStr $WpkgCommand $INI "WpkgConfig" "WpkgCommand"
    ${EndIf}
    ${If} $EnableViaLGP == ""
      ReadINIStr $EnableViaLGP $INI "WpkgConfig" "EnableViaLGP"
    ${EndIf}
    ${If} $NetworkUsername == ""
      ReadINIStr $NetworkUsername $INI "WpkgConfig" "WpkgNetworkUsername"
    ${EndIf}
  ${ElseIf} $IsUpgrade == 1
    ${If} $WpkgCommand == ""
      ReadINIStr $WpkgCommand $INSTDIR\Wpkg-GP.ini "WpkgConfig" "WpkgCommand"
    ${EndIf}
    ${If} $EnableViaLGP == ""
      ReadINIStr $EnableViaLGP $INSTDIR\Wpkg-GP.ini "WpkgConfig" "EnableViaLGP"
    ${EndIf}
    ${If} $NetworkUsername == ""
      ReadINIStr $NetworkUsername $INSTDIR\Wpkg-GP.ini "WpkgConfig" "WpkgNetworkUsername"
    ${EndIf}
  ${EndIf}
  
  ${If} $EnableViaLGP == ""
    StrCpy $EnableViaLGP 1
  ${EndIF}

  !insertmacro INSTALLOPTIONS_EXTRACT_AS "setup\WpkgSettings.ini" "WpkgSettings.ini"
  !insertmacro INSTALLOPTIONS_EXTRACT_AS "setup\GroupPolicySettings.ini" "GroupPolicySettings.ini"
  !insertmacro INSTALLOPTIONS_EXTRACT_AS "setup\NetworkUserSettings.ini" "NetworkUserSettings.ini"
FunctionEnd

Section "Wpkg-GP Client" Client

  # Remove old install.log
  Delete "$INSTDIR\install.log"
  LogSet on
  
  # Initial checks
  ${If} $WpkgCommand == ""
  ${AndIf} $EnableViaLGP == 1
    Abort "Cannot have a empty WpkgCommand when not configured via Domain Group Policies."
  ${EndIf}
  
  ${If} $IsUpgrade == 1
    DetailPrint "Installed version of ${PRODUCT_NAME} is: $PreviousVersion"
  ${Else}
    DetailPrint "This is a new install"
  ${EndIf}

# Does not work on service upgrading itself
#  ${If} $IsUpgrade == 1
#    DetailPrint "Attempting to stop WpkgServer"
#    nsExec::ExecToStack "net stop WpkgServer"
#    LogText "Command Returned:"
#    ClearErrors
#    Pop $0
#    ${DoUntil} ${Errors}
#      LogText "$0"
#      Pop $0
#    ${LoopUntil} 0 = 1
#  ${EndIf}
  
  # Install files
  SetOutPath $INSTDIR
  Delete /REBOOTOK $INSTDIR\MPR.dll #Causes issues on Vista

  # Loging is too verbose now
  LogSet off
  Call F_File
  # Install Wpkg-GP.dll GPE
  !insertmacro InstallLib DLL NOTSHARED REBOOT_NOTPROTECTED src\gpe\${PLATFORM}\Release\WPKG-gp.dll $INSTDIR\Wpkg-GP.dll $INSTDIR
  !insertmacro InstallLib DLL NOTSHARED REBOOT_NOTPROTECTED src\gpe\${PLATFORM}\Release\WPKG-GP-Test.exe $INSTDIR\Wpkg-GP-Test.exe $INSTDIR
  LogSet on

  CreateDirectory $INSTDIR\Microsoft.VC90.CRT
  SetOutPath $INSTDIR\Microsoft.VC90.CRT
  File redist\VC90\Microsoft.VC90.CRT-${PLATFORM}\*.* 
  SetOutPath $INSTDIR

  # Install Redist components
  DetailPrint "Installing Redist components"
  ClearErrors
  ${If} ${PLATFORM} == "x86"
    ReadRegStr $0 HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\{196BB40D-1578-3D01-B289-BEFC77A11A1E}" DisplayVersion
  ${Else}
    ReadRegStr $0 HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\{DA5E371C-6333-3D8A-93A4-6FD5B20BCC6E}" DisplayVersion
  ${EndIf}
  DetailPrint "Installed version of Redist components is: $0"
  
  ${If} ${Errors} # Not installed
    DetailPrint "Redist components are not installed, installing"
    SetOutPath $INSTDIR\VC++2010Redist
    File  redist\VC100\vcredist_${PLATFORM}.exe

    ${If} ${Silent}
      LogText "Executing silent install"
      ExecWait '"$OUTDIR\vcredist_${PLATFORM}.exe" /q'
    ${Else}
      LogText "Executing verbose install"
      ExecWait '"$OUTDIR\vcredist_${PLATFORM}.exe" /passive'
    ${EndIf}
    #Clear error
    ClearErrors
  ${Else}
    DetailPrint "Redist components are already installed"
  ${EndIf}

  SetOutPath "$INSTDIR"
  ${If} $IsUpgrade == 0
    ${If} $INI == ""
      DetailPrint "This is a new install, and no /INI provided. Will use default Wpkg-GP.ini file"
      File "Wpkg-GP.ini"
    ${Else}
      DetailPrint "This is a new install, and /INI is $INI. Will use the provided file."
      CopyFiles /SILENT $INI $INSTDIR\Wpkg-gp.ini
      File /oname=Wpkg-GP_Default.ini "Wpkg-GP.ini"
    ${EndIf}
  ${Else}
    ${If} $INI == ""
      DetailPrint "This is a upgrade, and no /INI is provided, checking for an existing Wpkg-GP.ini file."
      ${If} ${FileExists} $INSTDIR\Wpkg-GP.ini
        DetailPrint "File exists, will keep it"
        File /oname=Wpkg-GP_Default.ini "Wpkg-GP.ini"
      ${Else}
        DetailPrint "No INI file found, will use the default one."
        File "Wpkg-GP.ini"
      ${EndIf}
    ${Else}
      DetailPrint "This is a upgrade, and /INI is $INI, will use this file."
      Rename $INSTDIR\Wpkg-GP.ini $INSTDIR\Wpkg-GP_Old.ini
      CopyFiles /SILENT $INI $INSTDIR\Wpkg-gp.ini
      File /oname=Default_Wpkg-GP.ini "Wpkg-GP.ini"
    ${EndIf}
  ${EndIf}
  
  #Updating .ini file
  DetailPrint "Updating Wpkg-GP.ini with settings provided through the installer interface."
  WriteINIStr "$INSTDIR\Wpkg-GP.ini" "WpkgConfig" "EnableViaLGP" $EnableViaLGP
  ${If} $WpkgCommand != ""
    WriteINIStr "$INSTDIR\Wpkg-GP.ini" "WpkgConfig" "WpkgCommand" $WpkgCommand
  ${EndIf}
  ${If} $NetworkPassword != ""
    WriteINIStr "$INSTDIR\Wpkg-GP.ini" "WpkgConfig" "WpkgNetworkUsername" $NetworkUsername
    DetailPrint "Disabling logging to not show password"
    LogSet off
    WriteINIStr "$INSTDIR\Wpkg-GP.ini" "WpkgConfig" "WpkgNetworkPassword" "clear:$NetworkPassword"
    LogSet on
  ${EndIf}
  
  # Register components
  DetailPrint "Registering components"
  WriteRegStr HKLM "Software\Wpkg-GP" "InstallPath" "$INSTDIR"
  
  #WPKG-GP
  WriteRegStr HKLM "${GPE_REG_KEY}" "" "Wpkg-GP"
  WriteRegExpandStr  HKLM "${GPE_REG_KEY}" "DllName" "$INSTDIR\Wpkg-GP.dll"
  WriteRegDWORD HKLM "${GPE_REG_KEY}" "NoBackgroundPolicy" 1
  WriteRegDWORD HKLM "${GPE_REG_KEY}" "NoUserPolicy" 1
  WriteRegStr HKLM "${GPE_REG_KEY}" "ProcessGroupPolicy" "ProcessGroupPolicy"

  #Logging
  WriteRegStr HKLM "System\CurrentControlSet\Services\EventLog\Application\WPKG-gp-GPE" "EventMessageFile" "$INSTDIR\WPKG-gp.dll"
  WriteRegDWORD HKLM "System\CurrentControlSet\Services\EventLog\Application\WPKG-gp-GPE" "TypesSupported" 7

  # Install service
  DetailPrint "Installing service"
  LogSet off
  !insertmacro SERVICE "create" "WpkgServer" "path=$INSTDIR\WpkgServer.exe;autostart=1;interact=0;depend=LanmanWorkstation;display=WPKG Control Service;description=Controller service for userspace WPKG management applications. (http://wpkg-gp.googlecode.com/);"

  # Attempt to start service
  LogSet on
  DetailPrint "Starting service"
  nsExec::ExecToStack "net start WpkgServer"
  LogText "Command Returned:"
  ClearErrors
  ${DoUntil} ${Errors}
    Pop $0
    LogText "$0"
  ${LoopUntil} 0 = 1
  
  DetailPrint "Enabling verbose bootup"
  WriteRegDWORD HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Policies\System" "verbosestatus" 1
  WriteRegDWORD HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Policies\System" "DisableStatusMessages" 0
  WriteRegDWORD HKLM "SOFTWARE\Microsoft\Windows NT\CurrentVersion\Winlogon" "SyncForegroundPolicy" 1
  
  DetailPrint "Registering uninstallation options in add/remove programs"
  WriteRegStr HKLM ${PRODUCT_UNINST_KEY} "DisplayName" "${PRODUCT_NAME}"
  WriteRegStr HKLM ${PRODUCT_UNINST_KEY} "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegStr HKLM ${PRODUCT_UNINST_KEY} "InstallLocation" "$INSTDIR"
  #WriteRegStr HKLM PRODUCT_UNINST_KEY "DisplayIcon"
  WriteRegStr HKLM ${PRODUCT_UNINST_KEY} "Publisher" "${PRODUCT_PUBLISHER}"
  WriteRegStr HKLM ${PRODUCT_UNINST_KEY} "URLInfoAbout" "${PRODUCT_WEB_SITE}"
  WriteRegStr HKLM ${PRODUCT_UNINST_KEY} "DisplayVersion" "${PRODUCT_VERSION}"
  WriteRegStr HKLM ${PRODUCT_UNINST_KEY} "NoModify" 1
  WriteRegStr HKLM ${PRODUCT_UNINST_KEY} "NoRepair" 1
  
  WriteUninstaller $INSTDIR\uninstall.exe
  
  ${If} ${RebootFlag}
  ${AndIf} ${Silent}
    LogText "Reboot flag set, setting exit code to 3010."
    SetErrorLevel 3010
  ${EndIf}
  

SectionEnd

Section "Wpkg-GP Administrative Template for Group Policies" ADM
  SetOutPath $WINDIR\INF
  File src\Wpkg-GP.adm
SectionEnd

Section "Wpkg-GP MSI tool" MSITool
  SetOutPath $INSTDIR
  File dist-${PLATFORM}\MakeMSI.exe
  SetOutPath $INSTDIR\MSI
  File src\MSI\wpkg-gp_x64.msitemplate
  File src\MSI\wpkg-gp_x86.msitemplate
SectionEnd

Function WpkgSettingsPage
  ${IfNot} ${SectionIsSelected} ${Client}
    Abort
  ${Endif}
  !insertmacro MUI_HEADER_TEXT "Path to Wpkg.js" "Please enter the path to your wpkg.js"
  !insertmacro INSTALLOPTIONS_WRITE "WpkgSettings.ini" "Field 3" "State" $WpkgCommand
  !insertmacro INSTALLOPTIONS_DISPLAY "WpkgSettings.ini"
FunctionEnd

Function WpkgSettingsPageCallback
  !insertmacro INSTALLOPTIONS_READ $WpkgCommand "WpkgSettings.ini" "Field 3" "State"
  ${If} $WpkgCommand == ""
  ${AndIf} $EnableViaLGP == 1
    SetCtlColors $WpkgCommand 0x000000 0xFF0000
    MessageBox MB_OK "Wpkg path cannot be empty when you are using Local Group Policies" /SD IDOK
    Abort
  ${Else}
    SetCtlColors $WpkgCommand 0x000000 0xFFFFFF
  ${EndIf}

FunctionEnd

Function GroupPolicySettingsPage
  ${IfNot} ${SectionIsSelected} ${Client}
    Abort
  ${Endif}
  !insertmacro MUI_HEADER_TEXT "Group Policy settings" ""
  !insertmacro INSTALLOPTIONS_WRITE "GroupPolicySettings.ini" "Field 3" "State" $EnableViaLGP
  !insertmacro INSTALLOPTIONS_DISPLAY "GroupPolicySettings.ini"
FunctionEnd
Function GroupPolicySettingsPageCallback
  !insertmacro INSTALLOPTIONS_READ $EnableViaLGP "GroupPolicySettings.ini" "Field 3" "State"
FunctionEnd

Function NetworkUserPage
  ${IfNot} ${SectionIsSelected} ${Client}
    Abort
  ${Endif}
  !insertmacro MUI_HEADER_TEXT "Network username and password" "Please enter a username and password for the network"
  !insertmacro INSTALLOPTIONS_WRITE "NetworkUserSettings.ini" "Field 4" "State" $NetworkUsername
  !insertmacro INSTALLOPTIONS_WRITE "NetworkUserSettings.ini" "Field 5" "State" $NetworkPassword
  !insertmacro INSTALLOPTIONS_DISPLAY "NetworkUserSettings.ini"
FunctionEnd
Function NetworkUserPageCallback
  !insertmacro INSTALLOPTIONS_READ $NetworkUsername "NetworkUserSettings.ini" "Field 4" "State"
  !insertmacro INSTALLOPTIONS_READ $NetworkPassword "NetworkUserSettings.ini" "Field 5" "State"
FunctionEnd

Function .OnSelChange
  #Section 3 requires Section 1
  ${If} ${SectionIsSelected} ${MSITool}
    ${IfNot} ${SectionIsSelected} ${Client}
      SectionGetText ${Client} $0
      SectionGetText ${MSITool} $1
      MessageBox MB_OK "Section $1 requires Section $0 to be installed as well."
      !insertmacro SelectSection ${Client}
    ${EndIf}
  ${EndIf}
FunctionEnd

Function GetFeatures
 #Check the features:
  #Alternatives: Client Adm MSITool
  ${If} $Features != "" #Then install all features
    # Do not install any features
    !insertmacro UnselectSection ${Client}
    !insertmacro UnselectSection ${Adm}
    !insertmacro UnselectSection ${MSITool}
    ${StrContains} $0 "Client" $Features
    ${StrContains} $1 "Adm" $Features
    ${StrContains} $2 "MSITool" $Features
    ${If} $0 != ""
      !insertmacro SelectSection ${Client}
    ${EndIf}
    ${If} $1 != ""
      !insertmacro SelectSection ${Adm}
    ${EndIf}
    ${If} $2 != ""
      !insertmacro SelectSection ${Client}
      !insertmacro SelectSection ${MSITool}
    ${EndIf}
  ${EndIf}
FunctionEnd

# For Modern UI tooltips
LangString DESC_Client ${LANG_ENGLISH} "Install the Wpkg-GP client files"
LangString DESC_Adm ${LANG_ENGLISH} "Install the Wpkg-GP administrative template file"
LangString DESC_MSITool ${LANG_ENGLISH} "Install a tool for deploying Wpkg-GP via MSI files"

!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
  !insertmacro MUI_DESCRIPTION_TEXT ${Client} $(DESC_Client)
  !insertmacro MUI_DESCRIPTION_TEXT ${Adm} $(DESC_Adm)
  !insertmacro MUI_DESCRIPTION_TEXT ${MSITool} $(DESC_MSITool)
!insertmacro MUI_FUNCTION_DESCRIPTION_END

Function un.onInit
  ${If} ${RunningX64}
    StrCpy $CURRENTPLATFORM "x64"
  ${Else}
    StrCpy $CURRENTPLATFORM "x86"
  ${EndIf}

  ${If} $CURRENTPLATFORM != ${PLATFORM}
    MessageBox MB_OK|MB_ICONEXCLAMATION "This package is for ${PLATFORM} only, you are running it on $CURRENTPLATFORM" /SD IDOK
    SetErrorLevel 3
    Abort "This package is for ${PLATFORM} only, you are running it on $CURRENTPLATFORM"
  ${EndIf}

  ${If} $CURRENTPLATFORM == "x64"
    SetRegView 64
  ${EndIf}
FunctionEnd

section "uninstall"

  #Stop service
  nsExec::ExecToLog "net stop WpkgServer"

  !insertmacro SERVICE "delete" "WpkgServer" ""

  Call un.F_Delete
  Delete $INSTDIR\Wpkg-GP-test.exe
  Delete $INSTDIR\WpkgPipeClient.exe
  Delete $INSTDIR\uninstall.exe
  Delete $INSTDIR\wpkg-gp.dll
  Delete $INSTDIR\Wpkg-GP.INI
  Delete $INSTDIR\Wpkg-GP_Old.INI
  Delete $INSTDIR\Wpkg-GP_Default.INI
  Delete $INSTDIR\install.log
  Delete $WINDIR\INF\Wpkg-GP.adm
  RMDir /r $INSTDIR\Logs
  RMDir /r $INSTDIR\Microsoft.VC90.CRT
  RMDir /r $INSTDIR\New
  RMDir /r $INSTDIR\MSI
  RMDir /r $INSTDIR\VC++2010Redist
  RMDir $INSTDIR

  DeleteRegKey HKLM ${PRODUCT_UNINST_KEY}
  DeleteRegKey HKLM "Software\Wpkg-GP"
  DeleteRegKey HKLM "${GPE_REG_KEY}"
  DeleteRegKey HKLM "System\CurrentControlSet\Services\EventLog\Application\WPKG-gp-GPE"
  
sectionEnd