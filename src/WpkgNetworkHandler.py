import logging
import WpkgConfig
import re
import win32wnet, win32netcon
import time

class NullHandler(logging.Handler):
    def emit(self, record):
        pass

class WpkgNetworkHandler(object):
    def __init__(self):
        self.config = WpkgConfig.WpkgConfig()
        wpkg_command = self.config.get("WpkgCommand")
        self.get_network_share(wpkg_command)
        self.update_credentials()
        self.connected = False
        
    def update_credentials(self):
        self.network_username = self.config.get("WpkgNetworkUsername")
        self.network_password = self.config.get("WpkgNetworkPassword")

    def get_network_share(self, command_string):
        #Extracting \\servername_or_ip_or_whatever\\sharename
        logger.debug("Trying to extract share name from %s" % command_string)
        result = re.search(r'(\\\\[^\\]+\\[^\\]+)\\.*', command_string)
        if result != None:
            self.network_share = result.group(1)
            logger.debug("Extracted share: '%s'" % self.network_share)
        else:
            logger.info("The command %s did not contain a share name" % command_string)
            self.network_share = None
            
    def connect_to_network_share(self):
        if self.connected == True:
            return
        if self.network_share == None:
            logger.info("Wpkg is not on the network, will not connect to a share")
            self.connected = False
            return
        #Disconnect from the network if already connected
        self.disconnect_from_network_share()

        if self.network_username == None:
            logger.info("No network user configured, any network activity will be done by process user.")
            self.connected = False
            return #do not make any special connection
        if self.network_password == None:
            logger.info("User name %s specified, but no password set. Any network activity will be done by process user" % self.network_username)
            self.connected = False
            return
        else:
            i = 1
            tries = 6
            while self.connected != True and i >= tries:
                i = i+1
                try:
                    logger.debug("Trying to connect to share. %n of %n" % (i, tries))
                    win32wnet.WNetAddConnection2(win32netcon.RESOURCETYPE_DISK, None, self.network_share, None, self.network_username, self.network_password, 0)
                    logger.info("Successfully connected to %s as %s" % (self.network_share, self.network_username))
                    self.connected = True
                except win32wnet.error, (n, f, e):
                    self.connected = False
                    if n == 1326: #Logon failure
                        logger.info("Could not log on the network with the username: %s\n The error was: %s Continuing to log on to share as service user" % (network_username, e))
                        break
                    elif n == 1219: # Multiple connections from same user
                        logger.info("Tried to connect to share '%s', but a connection already exists." % self.network_share)
                        self.connected = True
                        break
                    elif n == 53 or n == 1231: # Network path not found | Network location cannot be reached
                        # This can indicate that the network path was wrong, or that the network is not available yet
                        logger.info("An issue occured when conencting to '%s', the error code is %n and the error string is '%s'" % (self.network_share, n, e))
                        time.sleep(5) # Sleep 5 seconds
                    else:
                        raise

    def disconnect_from_network_share(self):
        if self.connected == False:
            return
        try:
            logger.info("Trying to disconnect from the network share %s" % self.network_share)
            win32wnet.WNetCancelConnection2(self.network_share, 1, True)
            logger.info("Successfully disconnected from the network")
        except win32wnet.error, (n, f, e):
            if n == 2250: #This network connection does not exist
                logger.info("Was not connected to the network") 
            else:
                raise

if __name__ == '__main__':
    import sys
    logger = logging.getLogger("WpkgNetworkHandler")
    handler = logger.addHandler(logging.StreamHandler(sys.stdout))
    logger.setLevel(logging.DEBUG)
    network_handler = WpkgNetworkHandler()
    print "Network share: %s" % network_handler.network_share
    print "Username.....: %s" % network_handler.network_username
    print "Password.....: %s" % network_handler.network_password
    network_handler.connect_to_network_share()
    network_handler.disconnect_from_network_share()
    
else:
    h = NullHandler()
    logger = logging.getLogger("WpkgService")
    logger.addHandler(h)