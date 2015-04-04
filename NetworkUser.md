

## Background ##
WPKG-GP is divided into two parts, the WpkgService running as a service and serving as a "broker" for wpkg.js. and a client, either wpkg-gp.dll, the Group Policy Extension invoked at boot time, or any other client.

The WpkgService is normally running as the _NT AUTHORITY\SYSTEM_ user. This user is a local administrator, meaning it has the necessary rights to install software on the computer. However, unless you specifically allows it to connect to your network shares, e.g. by allowing the _machine_ account access, it has no rights on your network.

Giving the machine account access to the network may not be what you want, as this will allow _all users_ on the computer access to your software share. By configuring a Network User for WPKG-GP, you can connect to a network share as a user on the network and thus restrict access to the network share to this user only.

Please note that connections to shares as another user is done on a per-share basis. This means that Wpkg-GP will only use the provided credentials to connect to _the share that wpkg.js resides on_ and no other shares. This means that if wpkg.js resides on \\fileserver\apps\wpkg\wpkg.js and some of the software wpkg installs resides on \\anotherfileserver\software\setup.exe, you _will not connect to \\anotherfileserver\software as the provided user_ but as the user the Wpkg-GP service runs as (normally _NT AUTHORITY\SYSTEM_). If you want this to work, you need to run the Wpkg-GP service as a user with local administrator rights _and_ access to all the shares in question.

This can be done several ways, however a returning question is how to save the password for the user securely. If you provide the password as a part of the Group Policy, it will be saved in cleartext in the registry under HKLM\Software\Policies\WPKG\_GP\.

Wpkg-GP uses a Windows function to encrypt the password, and saving it to the registry that allows decryption to be done _only by the same user that encrypted it_. This means that if a user reads it from the registry, it has to be able to impersonate the _NT AUTHORITY\SYSTEM_ user in order to decrypt it.

We have added a routine for configuring the user name and password at install time. In addition, you can use the WpkgPipeClient.exe utility to set it and reset it at any time.

## How to set the user name and password ##

### Installation ###
Since the password has to be decrypted as the user the WpkgService runs as, it has to be encrypted by the same user. Therefore the password is set by the WpkgService itself. The installer includes a screen where you can configure the user name and password.

![http://wpkg-gp.googlecode.com/svn/wiki/images/setup-network-user.png](http://wpkg-gp.googlecode.com/svn/wiki/images/setup-network-user.png)

These settings can be provided as parameters to the silent installation:
`wpkg-gp-0.11_x86.exe /S /NetworkUsername CONTOSO\wpkguser /NetworkPassword Pa$$w0rd`

They can also be set directly in an ini-file, that is provided to the installer:
`wpkg-gp-0.11_x86.exe /S /INI \\path\to\wpkg-gp.ini`

The ini-file will then contain a section like:
```
WpkgNetworkUsername = CONTOSO\InstallUser
WpkgNetworkPassword = clear:P@$$w0rd
```

The password will be automatically encrypted in the INI file installed when the service is started the first time. So if you are reusing a current ini-file, make sure to change the password to cleartext first.

### After installation ###
You can rerun the installer after the installation, and change the settings.

The silent options of above will of course also work on a relaunch of the installer.

Or you can edit your local wpkg-gp.ini file directly and restart the Wpkg-GP service afterwards.