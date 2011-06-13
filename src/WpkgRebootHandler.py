import _winreg
import logging
import WpkgConfig
import reboot
import thread

STATUS_OK = 1
STATUS_PENDING = 2
STATUS_REBOOTING = 3
STATUS_ERROR = 4

class NullHandler(logging.Handler):
    def emit(self, record):
        pass

class WpkgRebootError(Exception):
    def __init__(self, value):
        self.value = value
    def __str__(self):
        return repr(self.value)

class WpkgRebootHandler(object):
    def __init__(self):
        config = WpkgConfig.WpkgConfig()
        self.get_reboot_number()
        self.maximum_number_of_reboots = config.get("WpkgMaxReboots")
        self.reboot_policy = config.get("WpkgRebootPolicy")
        self.status = STATUS_OK

    def reboot(self):
        logger.info(R"Wpkg-GP requested a reboot")
        # Check if we are past maximum number of reboots
        self.increment_reboot_number()
        print self.reboot_number, self.maximum_number_of_reboots
        if self.reboot_number >= self.maximum_number_of_reboots:
            self.status = STATUS_ERROR
            logger.info("Current number of reboots is %i, and maximum number of reboots is %i. Will not reboot" % (self.reboot_number, self.maximum_number_of_reboots))
            self.reset_reboot_number()
            return "Wpkg-GP requested a reboot, but we have rebooted too many times in a row. Continuing."
        elif self.reboot_policy == "force":
            logger.info("Rebooting now.")
            # This will only work from service, as main thread will exit before the thread has finished when
            # running the main() loop from the module, thus killing it.
            thread.start_new_thread(reboot.RebootServer, ("Wpkg-GP software installation requested a reboot.", 0, 1))
            self.status = STATUS_REBOOTING
            return "102 Installation requested a reboot. Rebooting now."
        elif self.reboot_policy == "ignore":
            logger.info("Reboot policy is set to 'ignore'. Reboot is pending")
            self.status = STATUS_PENDING
            return "103 Installation requires a reboot, but policy set to ignore reboots. Continuing."

    def cancel(self):
        reboot.AbortReboot()
        self.status = STATUS_OK
        
    def increment_reboot_number(self):
        with _winreg.OpenKey(_winreg.HKEY_LOCAL_MACHINE, R"Software\WPKG-gp", 0, _winreg.KEY_ALL_ACCESS) as key:
            self.reboot_number = self.reboot_number + 1
            logger.info(R"Incrementing reboot number to %s" % self.reboot_number)
            _winreg.SetValueEx(key, "RebootNumber", 0, _winreg.REG_DWORD, self.reboot_number)

    def get_reboot_number(self):
        with _winreg.OpenKey(_winreg.HKEY_LOCAL_MACHINE, R"SOFTWARE\WPKG-gp", 0, _winreg.KEY_ALL_ACCESS) as key:
            try:
                self.reboot_number = _winreg.QueryValueEx(key, "RebootNumber")[0]
            except (WindowsError):
                self.reboot_number = 0
                logger.info(R"Unable to read HKLM\SOFTWARE\WPKG-gp\RebootNumber, defaulting to 0")
                _winreg.SetValueEx(key, "RebootNumber", 0, _winreg.REG_DWORD, 0)

    def reset_reboot_number(self):
        with _winreg.OpenKey(_winreg.HKEY_LOCAL_MACHINE, R"Software\WPKG-gp", 0, _winreg.KEY_ALL_ACCESS) as key:
            self.reboot_number = 0
            logger.info(R"Resetting reboot number to 0")
            _winreg.SetValueEx(key, "RebootNumber", 0, _winreg.REG_DWORD, self.reboot_number)

if __name__ == '__main__':
    import sys
    logger = logging.getLogger("WpkgRebootHandler")
    handler = logger.addHandler(logging.StreamHandler(sys.stdout))
    logger.setLevel(logging.DEBUG)
    reboothandler = WpkgRebootHandler()
    reboothandler.reboot()
else:
    h = NullHandler()
    logger = logging.getLogger("WpkgRebootHandler")
    logger.addHandler(h)
    
    