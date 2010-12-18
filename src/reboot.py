# RebootServer.py
import win32security
import win32api
import sys, traceback
import time
from ntsecuritycon import *
import servicemanager

def AdjustPrivilege(priv, enable = 1):
     # Get the process token.
     flags = TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY
     htoken = win32security.OpenProcessToken(win32api.GetCurrentProcess(), flags)
     # Get the ID for the system shutdown privilege.
     id = win32security.LookupPrivilegeValue(None, priv)
     # Now obtain the privilege for this process.
     # Create a list of the privileges to be added.
     if enable:
         newPrivileges = [(id, SE_PRIVILEGE_ENABLED)]
     else:
         newPrivileges = [(id, 0)]
     # and make the adjustment.
     win32security.AdjustTokenPrivileges(htoken, 0, newPrivileges)

def RebootServer(message="Server Rebooting", timeout=30, bForce=0, bReboot=1):
    # Reboots server, retrying every 5 seconds for 5 minutes if system is not ready
     AdjustPrivilege(SE_SHUTDOWN_NAME)
     counter = 0
     servicemanager.LogInfoMsg("Trying to reboot from service")
     while counter <= 60:
         try:
             win32api.InitiateSystemShutdown(None, message, timeout, bForce, bReboot)
             break
         except win32api.error, (e, f, d):
             #servicemanager.LogInfoMsg(traceback.format_exc())
             servicemanager.LogInfoMsg("Reboot blocked. Reason is %s block. Retry counter is %i" % (d, counter))
             if e==21: # The device is not ready
                 time.sleep(5)
                 counter += 1
             else:
                 break
     AdjustPrivilege(SE_SHUTDOWN_NAME, 0)

def AbortReboot():
     AdjustPrivilege(SE_SHUTDOWN_NAME)
     try:
         win32api.AbortSystemShutdown(None)
     finally:
         # Now we remove the privilege we just added.
         AdjustPrivilege(SE_SHUTDOWN_NAME, 0)

if __name__=='__main__':
         message = "This server is pretending to reboot\r\n"
         message = message + "The shutdown will stop in 10 seconds"
         RebootServer(message)
         print "Sleeping for 10 seconds"
         time.sleep(10)
         print "Aborting shutdown"
         AbortReboot()