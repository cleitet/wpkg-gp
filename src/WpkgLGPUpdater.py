"""
WpkgLGPUpdater.py - WPKG-GP Local Group Policy Configuration tool

This module adds and removes GUID's from gpt.ini. This is necessary in order for Windows to execute the
WPKG-GP Group Policy extension.
"""
# Documentation on LGP and GPE:
# - http://technet.microsoft.com/en-us/library/cc784268%28WS.10%29.aspx
# - http://blogs.technet.com/b/askperf/archive/2007/06/05/the-basics-of-group-policies.aspx
# - http://grouppolicy.editme.com/ClientSideProcessing
# - http://technet.microsoft.com/en-us/library/cc938768.aspx
# - http://technet.microsoft.com/en-us/library/cc938750.aspx
# - http://technet.microsoft.com/en-us/library/cc978247.aspx
# - http://technet.microsoft.com/en-us/library/cc779745%28WS.10%29.aspx
# - http://msdn.microsoft.com/en-us/library/Aa374407

import win32api
import os.path
import ConfigParser
import sys, os
import WpkgConfig
import _winreg
import logging



WPKGGPGUID = '{A9B8D792-F454-11DE-BA92-FDCF56D89593}'
MMCEXTENSIONW61 = '{D02B1F72-3407-48AE-BA88-E8213C6761F1}' # I have no idea why they are different (different version of MMC?)
MMCEXTENSIONW51 = '{0F6B957D-509E-11D1-A7CC-0000F87571E3}' # The Windows 5.1 (XP) worked on 6.1 (Windows 7) So i'll use it :)

class NullHandler(logging.Handler):
    def emit(self, record):
        pass

class WpkgLocalGPConfigurator:
    def __init__(self):
        grouppolicydir = os.path.join(win32api.GetSystemDirectory(), "GroupPolicy")
        if not os.path.exists(grouppolicydir):
            os.mkdir(grouppolicydir)
        inifile = os.path.join(grouppolicydir, "gpt.ini")
        self._inifile = inifile
        self.config = ConfigParser.SafeConfigParser()
        logger.debug("LGP: Opening %s" % inifile)
        try:
            self.config.read(self._inifile)
        except ConfigParser.Error: #file does not exist
            self.config.add_section('General')
        if not self.config.has_section('General'):
            self.config.add_section('General')
    def isInLocalPolicies(self):
        try:
            extensions = self.config.get('General', 'gPCMACHINEExtensionNames')
        except ConfigParser.NoOptionError: #Specified option not found
            return 0
        if WPKGGPGUID in extensions:
            return 1
        else:
            return 0
    
    def updateVersion(self):
        try:
            version = self.config.get('General', 'Version')
        except ConfigParser.NoOptionError:
            version = 1
        #Documentation :http://technet.microsoft.com/en-us/library/cc978247.aspx
        version = int(version) + 1 #Computer policy is least significant bits
        self.config.set('General', 'Version', str(version))
        
    def addToLocalPolicies(self):
        if self.isInLocalPolicies():
            return #Already is in local policies
        self.updateVersion()
        try:
            extensions = self.config.get('General', 'gPCMACHINEExtensionNames')
        except ConfigParser.NoOptionError:
            extensions = ""
        extensions = "%s[%s%s]" % (extensions, WPKGGPGUID, MMCEXTENSIONW51)
        self.config.set('General', 'gPCMACHINEExtensionNames', extensions)
        with open(self._inifile, 'w') as configfile:
            self.config.write(configfile)
        self.fixNewLines()
            
    def removeFromLocalPolicies(self):
        if not self.isInLocalPolicies():
            return #Is not in local policies
        self.updateVersion()
        # Extensions should be readable, if you want to remove it
        extensions = self.config.get('General', 'gPCMACHINEExtensionNames')
        extensionlist = extensions.split("[")
        extensions = ""
        for line in extensionlist:
            line = line[:-1] #Removing "]" on end of line
            if WPKGGPGUID in line:
                continue # Skip this line
            elif line != "":
                extensions = "%s[%s]" % (extensions, line)
        if extensions == "[]": #Was the only extension
            self.config.remove_option('General', 'gPCMACHINEExtensionNames')
        else:
            self.config.set('General', 'gPCMACHINEExtensionNames', extensions)
        with open(self._inifile, 'w') as configfile:
            self.config.write(configfile)
        self.fixNewLines()
    def fixNewLines(self):
        file = open(self._inifile)
        lines = file.readlines()
        file.close()
        file = open(self._inifile, "w")
        for line in lines:
            file.write(line)
        file.close()
    def update(self):
        try:
            with _winreg.OpenKey(_winreg.HKEY_LOCAL_MACHINE, R"SOFTWARE\Wpkg-GP", 0, _winreg.KEY_READ) as key:
                current_setting = _winreg.QueryValueEx(key, "EnableViaLGP")[0]
        except WindowsError:
            current_setting = None
        config = WpkgConfig.WpkgConfig()
        EnableViaLGP = config.get("EnableViaLGP")
        if EnableViaLGP != current_setting:
            if EnableViaLGP == 1:
                logger.debug("Adding to LGP")
                self.addToLocalPolicies()
            else:
                logger.debug("Removing from LGP")
                self.removeFromLocalPolicies()

            with _winreg.OpenKey(_winreg.HKEY_LOCAL_MACHINE, R"Software\WPKG-gp", 0, _winreg.KEY_ALL_ACCESS) as key:
                _winreg.SetValueEx(key, "EnableViaLGP", 0, _winreg.REG_DWORD, EnableViaLGP)
                
            

def usage():
    my_name = os.path.split(sys.argv[0])[1]
    print("Usage: %s add|remove|update Add or removes WPKG-GP GPE from local Group Policies" % my_name)
def main():
    try:
        if sys.argv[1] == "add":
            wpkggp = WpkgLocalGPConfigurator()
            wpkggp.addToLocalPolicies()
            return
        elif sys.argv[1] == "remove":
            wpkggp = WpkgLocalGPConfigurator()
            wpkggp.removeFromLocalPolicies()
            return
        elif sys.argv[1] == "update":
            wpkggp = WpkgLocalGPConfigurator()
            wpkggp.update()
            return
        else:
            usage()
            return
    except IndexError:
        usage()
        return


if __name__=='__main__':
    formatter = logging.Formatter("%(asctime)s - %(name)s - %(levelname)s - %(message)s")                        
    h = logging.StreamHandler(sys.stdout)
    h.setFormatter(formatter)
    logger = logging.getLogger("WpkgLGPUpdater")
    logger.addHandler(h)
    logger.setLevel(logging.DEBUG)
    main()
else:
    h = NullHandler()
    logger = logging.getLogger("WpkgService")
    logger.addHandler(h)