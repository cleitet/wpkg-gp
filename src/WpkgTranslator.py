import gettext
import _winreg
import os

class WpkgTranslator(object):
    def __init__(self):
        with _winreg.OpenKey(_winreg.HKEY_LOCAL_MACHINE, R"SOFTWARE\Wpkg-GP", 0, _winreg.KEY_READ) as key:
            self.install_path = _winreg.QueryValueEx(key, "InstallPath")[0]
            self.locale_path = os.path.join(self.install_path, "locale")

    def install(self):
        gettext.install('wpkg-gp', self.locale_path, unicode=True)
