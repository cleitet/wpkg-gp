@echo off
SET VERSION=0.9
SET WIX="%ProgramFiles(x86)%\Windows Installer XML v3\bin"
SET PYTHON32="c:\python26 (x86)\python.exe"
SET PYTHON64="c:\python26 (x64)\python.exe"

cd %~dp0
IF [%1]==[] GOTO error
IF [%2]==[] GOTO error

IF "%1"=="all" (
  set ARCH=x86 x64
) ELSE (
  set ARCH=%1
)
IF "%2"=="all" (
  CALL :build clean
  CALL :build make_py
  CALL :build make_installer
) ELSE (
  CALL :build %2
)
GOTO :eof

:: function build(Target)
:build
  SETLOCAL
  SET TARGET=%1
  for %%k in (%ARCH%) do (
    echo ###################################
    echo running %TARGET% for %%k
    CALL :%TARGET% %%k
    echo done
    echo ###################################
  )
  ENDLOCAL
  GOTO :eof

:clean
  SETLOCAL
  SET ARCH=%1
  rd /s /q dist-%ARCH%
  rd /s /q build-%ARCH%
  del /s /q installer\%ARCH%\*.wixobj
  ENDLOCAL
  GOTO :eof

:make_py
  SETLOCAL
  SET ARCH=%1
  "c:\python26 (%ARCH%)\python.exe" setup.py py2exe
  move dist dist-%ARCH%
  move build build-%ARCH%
  ENDLOCAL
  GOTO :eof

:make_installer
  SETLOCAL
  SET ARCH=%1
  %WIX%\candle.exe -ext WixUtilExtension -arch %ARCH% -dReleaseVersion=%VERSION% -out installer\%ARCH%\ wpkg-gp.wxs installer\*.wxs
  %WIX%\light.exe -spdb -ext WixUiExtension -ext WixUtilExtension -out wpkg-gp-%VERSION%_%ARCH%.msi installer\%ARCH%\*.wixobj
  ENDLOCAL
  GOTO :eof
  
:error
  echo "Usage: %~nx0 [x86|x64|all] [clean|make_py|make_installer|all]"