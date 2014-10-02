import gettext
import _winreg
import logging
import os
import locale

class NullHandler(logging.Handler):
    def emit(self, record):
        pass

class WpkgTranslator(object):
    def __init__(self):
        with _winreg.OpenKey(_winreg.HKEY_LOCAL_MACHINE, R"SOFTWARE\Wpkg-GP", 0, _winreg.KEY_READ) as key:
            self.install_path = _winreg.QueryValueEx(key, "InstallPath")[0]
            self.locale_path = os.path.join(self.install_path, "locale")

    def _build_localename(self, localetuple):
        """ Builds a locale code from the given tuple (language code,
            encoding).
            No aliasing or normalizing takes place.
        """
        language, encoding = localetuple
        if language is None:
            language = 'C'
        if encoding is None:
            return language
        else:
            return language + '.' + encoding

    def install(self):
        if os.getenv('LANG') is None:
            lang = self._build_localename(locale.getdefaultlocale())
            os.environ['LANG'] = lang
        logger.debug("Loading locale %s in path %s" % (os.environ['LANG'], self.locale_path))
        gettext.install('wpkg-gp', self.locale_path)

if __name__=='__main__':
    import sys
    formatter = logging.Formatter("%(asctime)s - %(name)s - %(levelname)s - %(message)s")
    h = logging.StreamHandler(sys.stdout)
    h.setFormatter(formatter)
    logger = logging.getLogger("WpkgTranslator")
    logger.addHandler(h)
    logger.setLevel(logging.DEBUG)
    WPKG = WpkgTranslator()
    WPKG.install()
    logger.debug(_("Initializing Wpkg-GP software installation"))
else:
    h = NullHandler()
    logger = logging.getLogger("WpkgService")
    logger.addHandler(h)
