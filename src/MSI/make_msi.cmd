@echo off
SET WIX="%ProgramFiles(x86)%\Windows Installer XML v3\bin"

IF "%1"=="all" (
  set ARCH=x86 x64
) ELSE (
  set ARCH=%1
)

CALL :build clean
CALL :build make
GOTO :eof

:: function build(Target)
:build
  SETLOCAL
  for %%k in (%ARCH%) do (
    echo ###################################
    echo  Running %1 for %%k
    CALL :%1 %%k
    echo done
    echo ###################################
  )
  ENDLOCAL
  GOTO :eof

:clean
  SETLOCAL
  SET ARCH=%1
  del /s /q %ARCH%\*.wixobj
  del /s wpkg-gp_%ARCH%.msitemplate
  ENDLOCAL
  GOTO :eof


:make
  SETLOCAL
  SET ARCH=%1
  %WIX%\candle.exe -ext WixUtilExtension -arch %ARCH% -out %ARCH%\ wpkg-gp.wxs
  %WIX%\light.exe -spdb -ext WixUiExtension -ext WixUtilExtension -out wpkg-gp_%ARCH%.msitemplate %ARCH%\*.wixobj
  ENDLOCAL
  GOTO :eof
  
:error
  echo "Usage: %~nx0 [x86|x64|all]"