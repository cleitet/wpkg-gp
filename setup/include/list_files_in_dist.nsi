!include LogicLib.nsh
!include FileFunc.nsh
OutFile list_files_in_dist.exe
CrcCheck off # CRC check generates random errors
SilentInstall Silent
var /GLOBAL Platform
var /GLOBAL DIST_DIR
var /GLOBAL F_TMP
var /GLOBAL F_FILE
var /GLOBAL F_MOVE
var /GLOBAL F_DELETE

Section
  ${GetParameters} $R0
  ${GetOptions} $R0 "/Platform" $Platform
  ${If} $Platform == ""
    StrCpy $Platform "x86"
  ${EndIF}
  
  StrCpy $DIST_DIR "dist-$Platform"
  StrCpy $F_TMP    "setup\include\list_files_in_dist.tmp"
  StrCpy $F_FILE   "setup\include\file-$PLATFORM.nsi"
  StrCpy $F_MOVE   "setup\include\move-$PLATFORM.nsi"
  StrCpy $F_DELETE "setup\include\delete-$PLATFORM.nsi"

  
  nsExec::ExecToStack "cmd /c dir /B $DIST_DIR > $F_TMP" $0
  DetailPrint $0

  FileOpen $0 $F_TMP r
  FileOpen $1 $F_FILE w
  FileOpen $2 $F_MOVE w
  FileOpen $3 $F_DELETE w
  
  FileWrite $1 "Function F_File$\r$\n"
  FileWrite $2 "Function F_Move$\r$\n"
  FileWrite $3 "Function un.F_Delete$\r$\n"
  
  FileRead $0 $4
  ${DoUntil} ${Errors}
    FileRead $0 $4
    ${If} $4 == ""
      ${Break}
    ${EndIf}
    StrCpy $5 $4 -2
    FileWrite $1 "  !insertmacro InstallLib DLL NOTSHARED REBOOT_NOTPROTECTED dist-$$"
    FileWrite $1 "{PLATFORM}\$5 $$INSTDIR\$5 $$INSTDIR$\r$\n"

    FileWrite $2 "  Rename /REBOOTOK $$INSTDIR\New\$5 $$INSTDIR$\r$\n"
    FileWrite $3 "  Delete $$INSTDIR\$4"
  ${LoopUntil} 1 = 0
  
  FileWrite $1 "FunctionEnd"
  FileWrite $2 "FunctionEnd"
  FileWrite $3 "FunctionEnd"
  
  FileClose $0
  FileClose $1
  FileClose $2
#  Delete $F_TMP
SectionEnd