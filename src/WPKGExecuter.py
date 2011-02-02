# -*- coding: utf-8 -*-
"""WPKGExecuter.py
Class for executing WPKG
"""
import subprocess
import sys
import win32file
import re
import _winreg
import reboot
import servicemanager
import thread
import os
import WpkgNetworkUser
import win32wnet, win32netcon
import logging
import shlex

class NullHandler(logging.Handler):
    def emit(self, record):
        pass


class WPKGExecuter():
    def __init__(self):
        self.proc = 0
        self.lines = []
        self.status = "OK"
        self.wpkg_verbosity = 1
        self._Reset()
        self._ReadRegistryPolicySettings()
        self._ReadRegistryRebootNumber()
        self.nextline = ""   #Read line
        self.executedfrom = "gpe" #default

    def _Reset(self):
        self.isrunning = 0
        self._formattedline = ""
        self._pkgnum = 0
        self._pkgtot = 0
        self._operation = "Initializing WPKG"
        self._package_id = ""
        self._package_name = ""
        
    def setExecutedFrom(self, executedfrom):
        self.executedfrom = executedfrom #Either "gpe" or "client"

    def getStatus(self):
        return self.status

    def _ReadRegistryPolicySettings(self):
        try:
            with _winreg.OpenKey(_winreg.HKEY_LOCAL_MACHINE, R"SOFTWARE\Policies\WPKG_GP") as key:
                #Path to wpkg.js
                executable = _winreg.QueryValueEx(key, "WpkgPath")[0]
                self.wpkg_execute_command = self._parseExecuteCommand(executable)
                try:
                    # "force" = force reboot for GP (install at boot) installs
                    # "ignore" = do not reboot
                    self.wpkg_gprebootpolicy = _winreg.QueryValueEx(key, "GPRebootPolicy")[0]
                except (WindowsError):
                    #set default
                    logger.info(R"Unable to read HKLM\SOFTWARE\Policies\WPKG_GP\GPRebootPolicy, defaulting to 'force'")
                    self.wpkg_gprebootpolicy = "force"
                try:
                    self.wpkg_maxreboots = _winreg.QueryValueEx(key, "WpkgMaxReboots")[0]
                except (WindowsError):
                    #set default
                    logger.info(R"Unable to read HKLM\SOFTWARE\Policies\WPKG_GP\WpkgMaxReboots, defaulting to 10")
                    self.wpkg_maxreboots = 10
                try:
                    self.wpkg_verbosity = _winreg.QueryValueEx(key, "WpkgVerbosity")[0]
                except (WindowsError):
                    #set default
                    self.wpkg_verbosity = 1
        except WindowsError as e:
            logger.error(R"Unable to open HKLM\SOFTWARE\Policies\WPKG_GP. Maybe you don't have the WPKG-GP Group Policy Applied? Error was: %s" % e)
            self.status = "Error while opening WPKG policy registry settings. Maybe you don't have the WPKG-GP Group Policy Applied."

    def _ReadRegistryRebootNumber(self):
        with _winreg.OpenKey(_winreg.HKEY_LOCAL_MACHINE, R"SOFTWARE\WPKG-gp", 0, _winreg.KEY_ALL_ACCESS) as key:
            try:
                self._rebootnumber = _winreg.QueryValueEx(key, "RebootNumber")[0]
            except (WindowsError):
                self._rebootnumber = 0
                logger.info(R"Unable to read HKLM\SOFTWARE\WPKG-gp\RebootNumber, defaulting to 0")
                _winreg.SetValueEx(key, "RebootNumber", 0, _winreg.REG_DWORD, 0)

    def _IncrementRegistryRebootNumber(self):
        with _winreg.OpenKey(_winreg.HKEY_LOCAL_MACHINE, R"Software\WPKG-gp", 0, _winreg.KEY_ALL_ACCESS) as key:
            rebootnumber = self._rebootnumber + 1
            logger.info(R"Incrementing reboot number to %s" % rebootnumber)
            _winreg.SetValueEx(key, "RebootNumber", 0, _winreg.REG_DWORD, rebootnumber)

    def _ResetRegistryRebootNumber(self):
        with _winreg.OpenKey(_winreg.HKEY_LOCAL_MACHINE, R"Software\WPKG-gp", 0, _winreg.KEY_ALL_ACCESS) as key:
            rebootnumber = 0
            logger.info(R"Resetting reboot number to 0")
            _winreg.SetValueEx(key, "RebootNumber", 0, _winreg.REG_DWORD, rebootnumber)
    
    def _parseExecuteCommand(self, commandstring=False):
        if commandstring == False:
            commandstring = self.wpkg_execute_command
        commandstring = os.path.expandvars(commandstring) #Expanding variables
        #check if starts with cscript and contains /noReboot /synchronize and/or /sendStatus is in command. If not, add it.
        #if not, ignore it
        commandlist = commandstring.split(" ")
        is_js_script = False
        flags = []
        if commandlist[0][:-3] == ".js" or commandlist[0].lower() == "cscript":
                is_js_script = True
        if is_js_script == True:
            if commandlist[0].lower() != "cscript":
                logger.debug("WpkgCommand is a js file but is missing 'cscript', adding")
                commandlist.insert(0, "cscript")
            if not "/noReboot" in commandlist:
                logger.debug("WpkgCommand is a js but is missing /noReboot, adding")
                commandlist.append("/noReboot")
            if not "/synchronize" in commandlist:
                logger.debug("WpkgCommand is a js but is missing /synchronize, adding")
                commandlist.append("/synchronize")
            if not "/sendStatus" in commandlist:
                logger.debug("WpkgCommand is a js but is missing /sendStatus, adding")
                commandlist.append("/sendStatus")
            if not "/nonotify" in commandlist:
                logger.debug("WpkgCommand is a js but is missing /nonotify, adding")
                commandlist.append("/nonotify")
        self.wpkg_execute_command = " ".join(commandlist)
        return self.wpkg_execute_command
    
    def _Parse(self):
        #Remove all strings not showing "YYYY-MM-DD hh:mm:ss, STATUS  : "
        if not re.search("[0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}:[0-9]{2}, STATUS  : ", self.nextline):
            return False
        
        #Remove "STATUS"-part:
        line = ":".join(self.nextline.split(":")[3:])[1:]
        #Example output:
        # Starting software synchronization
        # Number of packages to be removed: 1
        # Remove: Checking status of 'Removeme' (1/1)
        # Performing operation (install) on 'Removeme' (removeme)
        # Remove: Removing package 'Removeme' (1/1)
        # Install: Verifying package 'test' (1/2)
        # Performing operation (install) on 'test' (rebootnow)
        # Install: Verifying package 'donothing' (2/2)
        # Performing operation (upgrade) on 'donothing' (donothing)
        # Finished software synchronization
        
        #Checking current operation:
        if re.match("^Remove: Checking status", line):
            #No action is being performed, only updating internal percentage counter, but do not generate output
            self._operation = "removing"
            self._pkgnum, self._pkgtot = re.search("'.*' \(([0-9]+)/([0-9]+)\)$", line).group(1, 2)
            return False
        elif re.match("^Install:", line):
            #No action is being performed, only updating internal percentage counter, but do not generate output
            self._operation = "installing"
            self._pkgnum, self._pkgtot = re.search("'.*' \(([0-9]+)/([0-9]+)\)$", line).group(1, 2)
            return False
        elif re.match("^Performing operation", line):
            #Operation is actually being performed
            operation, self._package_name, self._package_id = re.search(
                "^Performing operation \((.+)\) on '(.+)' \((.*)\)$", line).group(1,2,3)
            #The description of the operation is misleading on this message, except for upgrades
            if operation == "upgrade":
                self._operation = "upgrading"
            return True
    
    def _GetPercent(self):
        #This is quasi-progress, as some packages might be quick, and some slow
        if self._pkgnum and self._pkgtot:
            percent = "%i" % (int(self._pkgnum) * 100 / int(self._pkgtot))
            #Let it stick at 99% for last pkg :)
            if percent == "100":
                return "99"
            else:
                return percent
        else:
            return "0"
    
    def _MakeFormattedLine(self):
        if not self._Parse():
            return False
        formattedline = "100 %s %s (%s%%)" % (self._operation.capitalize(), self._package_name, self._GetPercent())
        if formattedline == self._formattedline:
            return False
        else:
            self._formattedline = formattedline
            return self._formattedline
        
    def _Write(self, handle, line):
        logger.info(R"Writing '%s' to pipe" % line)
        if self.useWriteFile:
            try:
                win32file.WriteFile(handle, line.encode('utf-8'))
                return 1
            except win32file.error, (n, f, e):
                if n == 232: #The pipe is being closed (in the other end)
                    logger.warning("A client closed the pipe unexpectedly.")
                    self.Cancel()
                    return 1
                else:
                    raise
        else:
            handle.write(line + '\n')
            return 1

    def _GetNetworkShare(self):
        #Extracting \\servername_or_ip_or_whatever\\sharename
        result = re.search(r'(\\\\[^\\]+\\[^\\]+)\\.*', self.wpkg_execute_command)
        if result != None:
            self.network_share = result.group(1)
        else:
            self.network_share = False
            
        return self.network_share
            
    def _ConnectToNetworkShare(self):
        #Disconnect from the network if already connected
        self._DisconnectFromNetworkShare()
        
        username, password = WpkgNetworkUser.get_network_user()
        if username == "":
            logger.info("No network user configured, continuing to connect as service user")
            return #Connect as the same user as the network
        else:
            try:
                win32wnet.WNetAddConnection2(win32netcon.RESOURCETYPE_DISK, None, self.network_share, None, username, password, 0)
                logger.info("Successfully connected to %s as %s" % (self.network_share, username))
            except win32wnet.error, (n, f, e):
                if n == 1326: #Logon failure
                    logger.error("Could not log on the network with the username: %s\n The error was: %s Continuing to log on to share as service user" % (username, e))
                else:
                    raise

    def _DisconnectFromNetworkShare(self):
        try:
            logger.info("Trying to disconnect from the network share %s" % self.network_share)
            win32wnet.WNetCancelConnection2(self.network_share, 1, True)
            logger.info("Successfully disconnected from the network")
        except win32wnet.error, (n, f, e):
            if n == 2250: #This network connection does not exist
                logger.info("Was not connected to the network") 
            else:
                raise
                
    def Execute(self, handle=sys.stdout, useWriteFile=False):
        self.useWriteFile = useWriteFile
        self.lines = []
        if not self.isrunning:
            execute_command = self._parseExecuteCommand()
            self.isrunning = 1
            logger.info(R"Executing WPKG with the command %s" % execute_command)
            
            #Open the network share as another user, if necessary
            if self._GetNetworkShare() != False:
                self._ConnectToNetworkShare()
            
            self.proc = subprocess.Popen(execute_command, stdout=subprocess.PIPE, universal_newlines=True)

            logger.info(R"Executed WPKG with the command %s" % execute_command)
            while 1:
                self.nextline = self.proc.stdout.readline()#.decode(sys.stdin.encoding)
                logger.info(R"WPKG command returned: %s" % self.nextline)
                self.lines.append(self.nextline)
                if not self.nextline: #If it is the last line an empty line is returned from readline()
                    #We are finished for now
                    self._Reset()
                    break
                #self._MakeFormattedLine() returns false if we are to skip line
                if not self._MakeFormattedLine():
                    continue
                if not self._Write(handle, self._formattedline):
                    break
                
            exitcode = self.proc.wait()
            #Closing handle to share
            if self.network_share != False:
                self._DisconnectFromNetworkShare()
                
            if exitcode == 1: #Cscript returned an error
                logger.error(R"WPKG command returned an error: '%s'" % self.lines[-2])
                self._Write(handle, "200 %s" % self.lines[-2])
                return
                
            
            if exitcode == 770560: #WPKG returns this when it requests a reboot
                logger.info(R"WPKG requested a reboot")
                # Check how many reboots in a row
                if self._rebootnumber >= self.wpkg_maxreboots:
                    self._Write(handle, "104 Installation requires a reboot, but the limit on maximum consecutive reboots (%i) is reached, so continuing." % self.wpkg_maxreboots)
                    return

                self._IncrementRegistryRebootNumber()
                # Handle reboot
                # Executed from a gpe:
                if self.executedfrom == "gpe":
                    if self.wpkg_gprebootpolicy == "force":
                        self._Write(handle, "102 Installation requires a reboot. Rebooting now.")
                        self.status = "Reboot pending"
                        # Execute reboot in a separate thread in order to be able to return
                        # Winlogon on Windows XP/2003 blocks reboots when executing Group Policies
                        # so we have to wait for wpkg-gp (and other group policies) to finish
                        # code for handling delaying of reboots in reboot module.
                        thread.start_new_thread(reboot.RebootServer, ("Installation requires a reboot", 0, 1))
                    elif self.wpkg_rebootpolicy == "ignore":
                        self._Write(handle, "103 Installation requires a reboot, but policy set to ignore reboots.")
                        self.status = "Reboot pending"
                elif self.executedfrom == "client":
                    self._Write(handle, "103 Installation requires reboot")
                    self.status = "Reboot pending"
            else: #Not requiring a reboot, so continuing
                self._ResetRegistryRebootNumber()
        else:
            logger.info(R"Client requested WPKG to execute, but WPKG is already running")
            msg = "201 WPKG is already running"
            self._Write(handle, msg)

    def Cancel(self, handle=sys.stdout):
        if self.isrunning:
            self.proc.kill()
            msg = "101 Cancel called, WPKG process was killed"
        else:
            msg = "202 Cancel called, WPKG process was not running"
        try:
            self._Write(handle, msg)
        except TypeError: #Maybe pipe is closed now
            pass

    def SetNetworkUser(self, username, password):
        WpkgNetworkUser.set_network_user(username, password)
    
    def SetExecuteUser(self, username, password):
        WpkgNetworkUser.set_execute_user(username, password)



if __name__=='__main__':
    import sys
    formatter = logging.Formatter("%(asctime)s - %(name)s - %(levelname)s - %(message)s")                        
    h = logging.StreamHandler(sys.stdout)
    h.setFormatter(formatter)
    logger = logging.getLogger("WpkgServiceExecuter")
    logger.addHandler(h)
    logger.setLevel(logging.DEBUG)
    WPKG = WPKGExecuter()
    WPKG.Execute()
else:
    h = NullHandler()
    logger = logging.getLogger("WpkgExecuter")
    logger.addHandler(h)