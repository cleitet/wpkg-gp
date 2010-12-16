@echo off
set VERSION=0.6

cd %~dp0

c:\python26\python.exe setup.py py2exe
"%ProgramFiles%\Windows Installer XML v3\bin\candle.exe" -arch x86 -dReleaseVersion=%VERSION% -out installer\x86\wpkg-gp-%VERSION%_x86.wixobj wpkg-gp.wxs
"%ProgramFiles%\Windows Installer XML v3\bin\candle.exe" -arch x64 -dReleaseVersion=%VERSION% -out installer\x64\wpkg-gp-%VERSION%_x64.wixobj wpkg-gp.wxs

"%ProgramFiles%\Windows Installer XML v3\bin\light.exe" -spdb -ext WixUiExtension installer\x86\wpkg-gp-0.6_x86.wixobj
"%ProgramFiles%\Windows Installer XML v3\bin\light.exe" -spdb -ext WixUiExtension installer\x64\wpkg-gp-0.6_x64.wixobj