@echo off
set VERSION=0.8

cd %~dp0

c:\python26\python.exe setup.py py2exe
del /s /q installer\x86\*.wixobj
del /s /q installer\x64\*.wixobj
"%ProgramFiles%\Windows Installer XML v3\bin\candle.exe" -arch x86 -dReleaseVersion=%VERSION% -out installer\x86\ wpkg-gp.wxs installer\*.wxs
"%ProgramFiles%\Windows Installer XML v3\bin\candle.exe" -arch x64 -dReleaseVersion=%VERSION% -out installer\x64\ wpkg-gp.wxs installer\*.wxs

"%ProgramFiles%\Windows Installer XML v3\bin\light.exe" -spdb -ext WixUiExtension -out wpkg-gp-%VERSION%_x86.msi installer\x86\*.wixobj
"%ProgramFiles%\Windows Installer XML v3\bin\light.exe" -spdb -ext WixUiExtension -out wpkg-gp-%VERSION%_x64.msi installer\x64\*.wixobj