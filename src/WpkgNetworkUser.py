# -*- coding: utf-8 -*-
"""WpkgNetworkUser.py

Class for saving and encrypting the username
and password Wpkg Should connect to the network share
and/or execute as.

Uses win32crypt.CryptProtectData() and
win32crypt.CryptUnprotectData() which is only possible
to decrypt by the same user encrypting the data. Thus
this functions needs to be run as the same user (the
user the service executes as) each time. If the service
user changes, the passwords have to be reencrypted.
"""
import _winreg
import win32crypt

def set_network_user(username, password):
    """
    Encrypts the password and stores it in the registry.

    The username and password is stored in the registry using
    win32crypt.CryptProtectData(). It is stored at
    HKLM\Software\WPKG-gp\NetworkUsername and NetworkUserEncryptedPassword
    """
    encrypted_password = win32crypt.CryptProtectData(password, "NetworkUserPassword", None, None, None, 1)
    
    with (_winreg.OpenKey(_winreg.HKEY_LOCAL_MACHINE, R"Software\WPKG-gp", 0, _winreg.KEY_ALL_ACCESS)) as key:
        _winreg.SetValueEx(key, "NetworkUserName", 0, _winreg.REG_SZ, username)
        _winreg.SetValueEx(key, "NetworkUserEncryptedPassword", 0, _winreg.REG_BINARY, encrypted_password)

def get_network_user():
    """
    Returns the username and the decrypted password from the registry as a tuple
    The username and passwords are empty if the corresponding registry keys are
    empty
    """
    with (_winreg.OpenKey(_winreg.HKEY_LOCAL_MACHINE, R"Software\WPKG-gp")) as key:
        try:
            username = _winreg.QueryValueEx(key, "NetworkUserName")[0]
            encrypted_password = _winreg.QueryValueEx(key, "NetworkUserEncryptedPassword")[0]
            password = win32crypt.CryptUnprotectData(encrypted_password, None, None, None, 1)[1]
        except WindowsError as e:
            username = ""
            password = ""
    return (username, password)
    
def set_execute_user(username, password):
    """
    Encrypts the password and stores it in the registry.

    The username and password is stored in the registry using
    win32crypt.CryptProtectData(). It is stored at
    HKLM\Software\WPKG-gp\ExecuteUserName and ExecuteUserEncryptedPassword
    """
    encrypted_password = win32crypt.CryptProtectData(password, "ExecuteUserPassword", None, None, None, 1)
    
    with (_winreg.OpenKey(_winreg.HKEY_LOCAL_MACHINE, R"Software\WPKG-gp", 0, _winreg.KEY_ALL_ACCESS)) as key:
        _winreg.SetValueEx(key, "ExecuteUserName", 0, _winreg.REG_SZ, username)
        _winreg.SetValueEx(key, "ExecuteUserEncryptedPassword", 0, _winreg.REG_BINARY, encrypted_password)

def get_execute_user():
    """
    Returns the username and the decrypted password from the registry as a tuple
    The username and passwords are empty if the corresponding registry keys are
    empty
    """
    with (_winreg.OpenKey(_winreg.HKEY_LOCAL_MACHINE, R"Software\WPKG-gp")) as key:
        try:
            username = _winreg.QueryValueEx(key, "NetworkUserName")[0]
            encrypted_password = _winreg.QueryValueEx(key, "NetworkUserEncryptedPassword")[0]
            password = win32crypt.CryptUnprotectData(encrypted_password, None, None, None, 1)[1]
        except (WindowsError):
            username = ""
            password = ""
    return (username, password)