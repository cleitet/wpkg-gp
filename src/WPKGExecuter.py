"""WPKGExecuter.py
Class for executing WPKG
"""
import subprocess
import sys
import win32file
import re
import _winreg


class WPKGExecuter():
    def __init__(self):
        self.debug = 0
        self.proc = 0
        self.status = "OK"
        self._ReadRegistrySettings()
        
        self.nextline = ""   #Read line
        self._Reset()

    def getStatus(self):
        return self.status
	
    def _ReadRegistrySettings(self):
        try:
            with _winreg.OpenKey(_winreg.HKEY_LOCAL_MACHINE, R"SOFTWARE\Policies\WPKG_GP") as key:
                self.wpkg_executable = _winreg.QueryValueEx(key, "WpkgPath")[0]
                self.wpkg_parameters = _winreg.QueryValueEx(key, "WpkgParameters")[0]
        except(WindowsError):
            self.status = "Error when reading WPKG policy registry settings"
        
    def _Reset(self):
        self.isrunning = 0
        self._formattedline = ""
        self._pkgnum = 0
        self._pkgtot = 0
        self._operation = "Initializing WPKG"
        self._package_id = ""
        self._package_name = ""
    
    def _getExecutable(self):
        return self.wpkg_executable + " " + self.wpkg_parameters
    
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
        if re.match("Remove:", line):
            self._operation = "removing"
            self._package_name, self._pkgnum, self._pkgtot = re.search("'(.*)' \(([0-9]+)/([0-9]+)\)$", line).group(1, 2, 3)
        elif re.match("Install:", line):
            self._operation = "installing"
            self._package_name, self._pkgnum, self._pkgtot = re.search("'(.*)' \(([0-9]+)/([0-9]+)\)$", line).group(1, 2, 3)
        elif re.match("Performing operation", line):
            operation, self._package_name, self._package_id = re.search(
                "\((\w+)\) on '(.+)' \((.*)\)$", line).group(1,2,3)
            if operation == "upgrade":
                self._operation == "upgrading"
        else: #Skip line
            return False
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
        formattedline = "100 WPKG is %s %s (%s%%)" % (self._operation, self._package_name, self._GetPercent())
        if formattedline == self._formattedline:
            return False
        else:
            self._formattedline = formattedline
            return self._formattedline
    
    def Execute(self, handle=sys.stdout, useWriteFile=False):
        if not self.isrunning:
            executable = self._getExecutable()
            self.isrunning = 1
            self.proc = subprocess.Popen(executable, stdout=subprocess.PIPE, universal_newlines=True)
            while 1:
                self.nextline = self.proc.stdout.readline()
                if not self.nextline: #If it is the last line an empty line is returned from readline()
                    #We are finished for now
                    self._Reset()
                    break
                self._Parse()
                #self._MakeFormattedLine() returns false if we are to skip line
                if not self._MakeFormattedLine():
                    continue
                if useWriteFile:
                    #win32file.WriteFile(handle, self.nextline.encode('utf-8')) #For debugging
                    win32file.WriteFile(handle, self._formattedline.encode('utf-8'))
                else:
                    handle.write(self._formattedline)
        else:
            msg = "201 WPKG is already running"
            if useWriteFile:
                win32file.WriteFile(handle, msg.encode('utf-8'))
            else:
                handle.write(msg)
    def Cancel(self, handle=sys.stdout, useWriteFile=False):
        if self.isrunning:
            self.proc.kill()
            msg = "101 WPKG process was killed"
        else:
            msg = "202 WPKG process is not running"
        if useWriteFile:
            win32file.WriteFile(handle, msg.encode('utf-8'))
        else:
            handle.write(msg)
                        
if __name__=='__main__':
    WPKG = WPKGExecuter()
    print (WPKG.getStatus())
    WPKG.Execute()