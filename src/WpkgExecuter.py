# -*- coding: utf-8 -*-
"""WPKGExecuter.py
Class for executing WPKG
"""
import WpkgConfig
import WpkgWriter
import WpkgNetworkHandler
import WpkgOutputParser
import WpkgRebootHandler
import logging
import sys, os, csv, subprocess

class NullHandler(logging.Handler):
    def emit(self, record):
        pass


class WpkgExecuter():
    
    is_running = False
    
    def __init__(self, handle=None):
        config = WpkgConfig.WpkgConfig()
        self.wpkg_command = config.get("WpkgCommand")
        self.writer = WpkgWriter.WpkgWriter(handle)
        self.network_handler = WpkgNetworkHandler.WpkgNetworkHandler()
        self.parser = WpkgOutputParser.WpkgOutputParser()
        self.reboot_handler = WpkgRebootHandler.WpkgRebootHandler()
        self.parse_wpkg_command()
    
    def parse_wpkg_command(self):
        commandstring = self.wpkg_command
        commandstring = os.path.expandvars(commandstring) #Expanding variables

        #check if starts with cscript and contains /noreboot /synchronize
        # and/or /sendStatus is in command. If not, add it.
        
        # Using csv module "hack" to split the commandstring since shlex does not work with unicode strings
        # And we want to be able to parse escaped strings like 'cscript "path to file" /switch'

        commandlist = csv.reader([commandstring], delimiter=" ").next()
        is_js_script = False
        
        if commandlist[0].lower() == "cscript" or commandlist[0][-3:].lower()==".js":
           is_js_script = True
        if is_js_script == True:
            if commandlist[0].lower() != "cscript":
                logger.debug("WpkgCommand is a js file but is missing 'cscript', adding")
                commandlist.insert(0, "cscript")
            if not "/noreboot" in commandlist:
                logger.debug("WpkgCommand is a js but is missing /noreboot, adding")
                commandlist.append("/noreboot")
            if not "/synchronize" in commandlist:
                logger.debug("WpkgCommand is a js but is missing /synchronize, adding")
                commandlist.append("/synchronize")
            if not "/sendStatus" in commandlist:
                logger.debug("WpkgCommand is a js but is missing /sendStatus, adding")
                commandlist.append("/sendStatus")
            if not "/nonotify" in commandlist:
                logger.debug("WpkgCommand is a js but is missing /nonotify, adding")
                commandlist.append("/nonotify")
        self.execute_command = " ".join(commandlist)
                
    def Execute(self, handle=None):
        self.writer = WpkgWriter.WpkgWriter(handle)
        lines = []
        if self.is_running:
            logger.info(R"Client requested WPKG to execute, but WPKG is already running.")
            msg = "201 WPKG is already running"
            self.writer.Write(msg)
            return

        self.isrunning = True
        self.writer.Write("100 Initializing Wpkg-GP software installation")
        logger.info(R"Executing WPKG with the command %s" % self.execute_command)
        
        #Open the network share as another user, if necessary
        self.network_handler.connect_to_network_share()

        # Run WPKG        
        self.proc = subprocess.Popen(self.execute_command, stdout=subprocess.PIPE, universal_newlines=True)
        logger.info(R"Executed WPKG with the command %s" % self.execute_command)

        #Reading lines
        while 1:
            line = self.proc.stdout.readline()#.decode(sys.stdin.encoding)
            if not line: #If it is the last line an empty line is returned from readline()
                #We are finished for now
                self.is_running = False
                break
            logger.debug(R"WPKG command returned: %s" % line)
            lines.append(line)
            self.parser.parse_line(line)
            # If line is updated, print it
            if self.parser.updated:
                self.writer.Write("100 %s" % self.parser.get_formatted_line())
        self.parser.reset()
        
        exitcode = self.proc.wait()
        #Closing handle to share
        self.network_handler.disconnect_from_network_share()
            
        if exitcode == 1: #Cscript returned an error
            logger.error(R"WPKG command returned an error: %s" % lines[-1][0:-1])
            self.writer.Write("200 Wpkg returned an error: %s" % lines[-1][0:-1])
            return
        
        if exitcode == 770560: #WPKG returns this when it requests a reboot
            logger.info(R"WPKG requested a reboot")
            status = self.reboot_handler.reboot()
            self.writer.Write(status)
        else:
            self.reboot_handler.reset_reboot_number()

    def Cancel(self, handle=sys.stdout):
        if self.isrunning:
            self.proc.kill()
            logger.info("Cancel called, WPKG process was killed.")
            msg = "101 Cancel called, WPKG process was killed"
        else:
            logger.info("Cancel called, but WPKG process was not running")
            msg = "202 Cancel called, WPKG process was not running"
        try:
            self.writer.Write(handle, msg)
        except TypeError: #Maybe pipe is closed now
            pass

if __name__=='__main__':
    import sys
    formatter = logging.Formatter("%(asctime)s - %(name)s - %(levelname)s - %(message)s")                        
    h = logging.StreamHandler(sys.stdout)
    h.setFormatter(formatter)
    logger = logging.getLogger("WpkgExecuter")
    logger.addHandler(h)
    logger.setLevel(logging.DEBUG)
    WPKG = WpkgExecuter()
    WPKG.Execute()
else:
    h = NullHandler()
    logger = logging.getLogger("WpkgExecuter")
    logger.addHandler(h)
