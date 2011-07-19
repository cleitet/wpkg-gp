@echo off
SET VERSION=0.12
IF "%PROCESSOR_ARCHITECTURE%"=="x86" (
  SET NSIS="%ProgramFiles%\NSIS\makensis.exe"
  SET PYTHON32="c:\python26\python.exe"
) ELSE (
  SET NSIS="%ProgramFiles(x86)%\NSIS\makensis.exe"
  SET PYTHON32="c:\python27 (x86)\python.exe"
  SET PYTHON64="c:\python27 (x64)\python.exe"
)

cd %~dp0
IF [%1]==[] GOTO error
IF [%2]==[] GOTO error

IF "%1"=="all" (
  IF "%PROCESSOR_ARCHITECTURE%"=="x86" (
    set ARCH=x86
  ) ELSE (
    set ARCH=x86 x64
  )
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
  ENDLOCAL
  GOTO :eof

:make_py
  SETLOCAL
  SET ARCH=%1
  if "%ARCH%"=="x86" (
    %PYTHON32% setup.py py2exe
  ) ELSE (
    %PYTHON64% setup.py py2exe
  )
  move dist dist-%ARCH%
  move build build-%ARCH%
  ENDLOCAL
  GOTO :eof

:make_installer
  SETLOCAL
  SET ARCH=%1
  %NSIS% /DPLATFORM=%ARCH% -DVERSION=%VERSION% %~dp0\Wpkg-GP.nsi
  ENDLOCAL
  GOTO :eof
  
:error
  echo "Usage: %~nx0 [x86|x64|all] [clean|make_py|make_installer|all]"