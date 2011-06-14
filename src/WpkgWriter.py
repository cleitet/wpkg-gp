import sys
import logging
import win32file
import pywintypes

class NullHandler(logging.Handler):
    def emit(self, record):
        pass

class WpkgWriter():
    def __init__(self, handle=None):
        self.handle = handle

    def Write(self, string):
        logger.debug(R"Writing '%s' to pipe" % string)
        if self.handle != None:
            try:
                win32file.WriteFile(self.handle, string.decode('iso8859_15').encode('iso8859_15'))
                return True
            except pywintypes.error, (n, f, e):
                if n == 232 or n == 109: #The pipe is being closed (in the other end)
                    logger.warning("A client closed the pipe unexpectedly.")
                    return 1
                else:
                    raise
        else:
            print string

h = NullHandler()
logger = logging.getLogger("WpkgWriter")
logger.addHandler(h)
    