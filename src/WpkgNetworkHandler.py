import logging
import WpkgConfig
import re
import win32wnet, win32netcon, winerror
import socket
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
            
    def test_host_connect(self):
        host = self.config.get("TestConnectionHost")
        port = self.config.get("TestConnectionPort")
        tries = self.config.get("TestConnectionTries")
        sleep = self.config.get("TestConnectionSleepBeforeRetry")
        timeout = 2

        for i in range(tries):
            try:
                logger.debug("Testing connection with host '%s' port '%s' (%i/%i)" % (host,port,i+1,tries))
                tcptest = socket.create_connection((host,port),timeout)
                logger.debug("Testing connection: successful")
                tcptest.shutdown(socket.SHUT_RDWR)
                tcptest.close()
                return True
            except socket.error as msg:
                logger.debug("Testing connection failed: %s" % (msg))
            time.sleep(sleep)

        return False

    def connect_to_network_share(self):
        if self.connected == True:
            logger.debug("Is already connected to the network")
            return
        if self.network_username == None:
            logger.info("No username provided, credentials used will be that of the Wpkg-GP service.")
            self.connected = False
            return
        
        if self.network_share == None:
            logger.info("Wpkg is not on the network, will not connect to a share")
            self.connected = False
            return
        # cleaning up any stale connections
        self.disconnect_from_network_share()

        if self.config.get("TestConnectionHost") != None and not self.test_host_connect():
            logger.info("Test-Host did not respond. Not connecting to the network share")
            return

        sleep = self.config.get("ConnectionSleepBeforeRetry")
        tries = self.config.get("ConnectionTries")
        i = 0
        while self.connected != True and i < tries:
            i = i+1
            try:
                logger.debug("Trying to connect to share. %s of %s" % (i, tries))
                win32wnet.WNetAddConnection2(win32netcon.RESOURCETYPE_DISK, None, self.network_share, None, self.network_username, self.network_password, 0)
                logger.info("Successfully connected to %s as %s" % (self.network_share, self.network_username))
                self.connected = True
            except win32wnet.error, (n, f, e):
                self.connected = False
                if n == 1326: #Logon failure
                    if self.network_username != None:
                        logger.info("Could not log on the network with the username: %s\n The error was: %s Continuing to try to log on to share as service user" % (network_username, e))
                        self.network_username = None
                        self.network_password = None
                    else:
                        logger.info("Could not log on to the network with Wpkg-GP service account")
                        break
                elif n == winerror.ERROR_SESSION_CREDENTIAL_CONFLICT: # 1219: Multiple connections from same user
                    logger.info("Tried to connect to share '%s', but a connection already exists. Will disconnect, and retry." % self.network_share)
                    self.connected = True
                    self.disconnect_from_network_share()
                    pass
                elif n == winerror.ERROR_BAD_NETPATH or n == winerror.ERROR_NETWORK_UNREACHABLE: # 53_ Network path not found | 1231Network location cannot be reached
                    # This can indicate that the network path was wrong, or that the network is not available yet
                    logger.info("An issue occured when connecting to '%s', the error code is %i and the error string is '%s'" % (self.network_share, n, e))
                    time.sleep(sleep)
                else:
                    raise

    def disconnect_from_network_share(self):
        if self.connected == False:
            return
        try:
            logger.info("Trying to disconnect from the network share %s" % self.network_share)
            win32wnet.WNetCancelConnection2(self.network_share, 1, True)
            logger.info("Successfully disconnected from the network")
            self.connected = False
        except win32wnet.error, (n, f, e):
            if n == winerror.ERROR_NOT_CONNECTED: #2250: This network connection does not exist
                logger.info("Was already disconnected from network") 
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
