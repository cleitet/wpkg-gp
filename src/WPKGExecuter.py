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

class WPKGExecuter():
    def __init__(self):
        self.debug = 0
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
                self.wpkg_executable = _winreg.QueryValueEx(key, "WpkgPath")[0]
                #Parameters to wpkg.js
                self.wpkg_parameters = _winreg.QueryValueEx(key, "WpkgParameters")[0]
                try:
                    # "force" = force reboot for GP (install at boot) installs
                    # "ignore" = do not reboot
                    self.wpkg_gprebootpolicy = _winreg.QueryValueEx(key, "GPRebootPolicy")[0]
                except (WindowsError):
                    #set default
                    self._Log(R"Unable to read HKLM\SOFTWARE\Policies\WPKG_GP\GPRebootPolicy, defaulting to 'force'", "info", 2)
                    self.wpkg_gprebootpolicy = "force"
                try:
                    self.wpkg_maxreboots = _winreg.QueryValueEx(key, "WpkgMaxReboots")[0]
                except (WindowsError):
                    #set default
                    self._Log(R"Unable to read HKLM\SOFTWARE\Policies\WPKG_GP\WpkgMaxReboots, defaulting to 10", "info", 2)
                    self.wpkg_maxreboots = 10
                try:
                    self.wpkg_verbosity = _winreg.QueryValueEx(key, "WpkgVerbosity")[0]
                except (WindowsError):
                    #set default
                    self.wpkg_verbosity = 1
        except(WindowsError):
            self._Log(R"Unable to open HKLM\SOFTWARE\Policies\WPKG_GP", "error", 1)
            self.status = "Error while reading WPKG policy registry settings"

    def _ReadRegistryRebootNumber(self):
        with _winreg.OpenKey(_winreg.HKEY_LOCAL_MACHINE, R"SOFTWARE\WPKG-gp", 0, _winreg.KEY_ALL_ACCESS) as key:
            try:
                self._rebootnumber = _winreg.QueryValueEx(key, "RebootNumber")[0]
            except (WindowsError):
                self._rebootnumber = 0
                self._Log(R"Unable to read HKLM\SOFTWARE\WPKG-gp\RebootNumber, defaulting to 0", "info", 2)
                _winreg.SetValueEx(key, "RebootNumber", 0, _winreg.REG_DWORD, 0)

    def _IncrementRegistryRebootNumber(self):
        with _winreg.OpenKey(_winreg.HKEY_LOCAL_MACHINE, R"Software\WPKG-gp", 0, _winreg.KEY_ALL_ACCESS) as key:
            rebootnumber = self._rebootnumber + 1
            self._Log(R"Incrementing reboot number to %s" % rebootnumber, "info", 3)
            _winreg.SetValueEx(key, "RebootNumber", 0, _winreg.REG_DWORD, rebootnumber)

    def _ResetRegistryRebootNumber(self):
        with _winreg.OpenKey(_winreg.HKEY_LOCAL_MACHINE, R"Software\WPKG-gp", 0, _winreg.KEY_ALL_ACCESS) as key:
            rebootnumber = 0
            self._Log(R"Resetting reboot number to 0", "info", 3)
            _winreg.SetValueEx(key, "RebootNumber", 0, _winreg.REG_DWORD, rebootnumber)
    
    def _getExecutable(self):
        return "%s /noreboot %s" % (self.wpkg_executable, self.wpkg_parameters)
    
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
        self._Log(R"Writing '%s' to pipe" % line, "info", 2)
        if self.useWriteFile:
            try:
                win32file.WriteFile(handle, line.encode('utf-8'))
                return 1
            except win32file.error, (n, f, e):
                if n == 232: #The pipe is being closed (in the other end)
                    _Log("A client closed the pipe unexpectedly.", "warning", 1)
                    self.Cancel()
                    return 1
                else:
                    raise
        else:
            handle.write(line + '\n')
            return 1

    #Write to syslog
    def _Log(self, message, type="info", verbosity=1):
        if verbosity <= self.wpkg_verbosity:
            if type == "info":
                servicemanager.LogInfoMsg(message)
            elif type == "warning":
                servicemanager.LogWarningMsg(message)
            elif type == "error":
                servicemanager.LogErrorMsg(message)
            
    
    def Execute(self, handle=sys.stdout, useWriteFile=False):
        self.useWriteFile = useWriteFile
        self.lines = []
        if not self.isrunning:
            executable = self._getExecutable()
            self.isrunning = 1
            self._Log(R"Executing WPKG with the command %s" % executable, "info", 3)
            self.proc = subprocess.Popen(executable, stdout=subprocess.PIPE, universal_newlines=True)
            self._Log(R"Executed WPKG with the command %s" % executable, "info", 3)
            while 1:
                self.nextline = self.proc.stdout.readline()#.decode(sys.stdin.encoding)
                self._Log(R"WPKG command returned: %s" % self.nextline, "info", 3)
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
            if exitcode == 1: #Cscript returned an error
                self._Log(R"WPKG command returned an error: '%s'" % self.lines[-2], "error", 3)
                self._Write(handle, "200 %s" % self.lines[-2])
                return
                
            
            if exitcode == 770560: #WPKG returns this when it requests a reboot
                self._Log(R"WPKG requested a reboot", "info", 1)
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
            self._Log(R"Client requested WPKG to execute, but WPKG is already running", "info", 1)
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
                        
if __name__=='__main__':
    WPKG = WPKGExecuter()
    WPKG.Execute()