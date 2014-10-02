@echo on

SET PYTHON64DIR=c:\python27 (x64)
SET PYTHON64=%PYTHON64DIR%\python.exe

cd %~dp0
"%PYTHON64%" "%PYTHON64DIR%\Tools\i18n\pygettext.py" -d wpkg-gp src\WpkgExecuter.py src\WpkgOutputParser.py src\WpkgRebootHandler.py

move wpkg-gp.pot locale

