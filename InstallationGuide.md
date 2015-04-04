## Table of contents ##


## Introduction ##
The process is quite easy. If you ever have used WPKG and, it should be pretty straight forward.

## What is needed? ##
  * Wpkg-GP-{version}.exe
  * A working [WPKG](http://wpkg.org) setup. wpkg-gp is tested with both version 1.1.2 and 1.2.
    * Wpkg-GP runs by default as the _SYSTEM_ user on the local computer, however it can connect to the network as another user of your choice.
    * Windows XP or newer

## How to install ##
  * If you use Group Policies, turn off asynchronous application of group policies at boot up (See below for Howto). This is done automatically by the installer, but may be overridden by your Group Policies if you are in a domain environment.
  * If you use Group Policies on _Windows >= Vista_ Turn on verbose startup and shutdown messages (See below for Howto). This is done automatically by the installer, but may be overridden by your Group Policies if you are in a domain environment.
  * Install all wpkg-gp-versionnumber`_`[x86|x64].exe on the computers, see below for information on how to automate this.
  * In the "Path to Wpkg.js" dialog, you provide the path to wpkg.js. These settings are read from your current settings. Be sure to use an UNC path if it is located on the network, as no drives are mapped when GPE's are run. If the path to wpkg.js contains spaces, or an environment variable contain you need to enclose it in "'s. E.g.: "%SOFTWARE%\wpkg.js" /parameters
> > ![http://wpkg-gp.googlecode.com/svn/wiki/images/setup-wpkg-path.png](http://wpkg-gp.googlecode.com/svn/wiki/images/setup-wpkg-path.png)
  * In the "Group Policy Settings" dialog, you can enable Wpkg-GP via Local Group Policies. This will make Wpkg-GP run in a non-domain environment as well. This will normally be what you want in most circumstances, even if you are in a domain environment, as it requires minimal post setup configuration. See [Wpkg-GP via Domain Group Policies](Configuring.md) if you want to configure Wpkg-GP via Domain Group Policies instead.
> > ![http://wpkg-gp.googlecode.com/svn/wiki/images/setup-gp-settings.png](http://wpkg-gp.googlecode.com/svn/wiki/images/setup-gp-settings.png)
  * In the "Network user name and password" dialog, you can either leave it as it is, to make the WPKG-GP service connect to the network as the _NT AUTHORITY\SYSTEM_ user (the computer account) to execute wpkg.js and the different installers, or add the credentials of a network user here. For more information on how this works, see the NetworkUser wiki page.
> > ![http://wpkg-gp.googlecode.com/svn/wiki/images/setup-network-user.png](http://wpkg-gp.googlecode.com/svn/wiki/images/setup-network-user.png)

  * _If you want to configure WPKG-GP via domain policies_: Install the Administrative template feature only on your domain controllers.

## Unattended/Silent installation ##
WPKG-GP supports the following switches:
| Switch | Function                             | Example |
|:-------|:-------------------------------------|:--------|
| /S     | Silent                               | /S      |
| /INI   | Use settings from specified INI file | /INI path\_to\_ini\_file.ini |
| /NetworkUsername | Use this username (Overrides INI settings) | /NetworkUsername CONTOSO\WpkgInstallUser |
| /NetworkPassword | Use this password (Overrides INI settings) | /NetworkPassword Pa$$w0rd |
| /WpkgCommand | Provide Path to wpkg.js | /WpkgCommand \\Server1\Wpkg\wpkg.js If the path to wpkg.js contains spaces, or an environment variable contain you need to enclose it in "'s. E.g.: "%SOFTWARE%\wpkg.js" /parameters|
| /DisableViaLGP| Disable Wpkg-GP via Local Group Policies | /DisableViaLGP|
| /Features Client|Adm|MSITool | Install only these features | /Features Client |

**Example with an INI file:** `wpkg-gp-versionnumber_[x86|x64].exe /S /INI \\path\t\wpkg-gp.ini `
 Please note that the wpkg-gp.ini cannot have an encrypted password when you deploy wpkg-gp.

**Example with provided settings:** `wpkg-gp-versionnumber_[x86|x64].exe /S /NetworkUsername CONTOSO\WpkgInstallUser /NetworkPassword Pa$$w0rd /WpkgCommand \\server\Wpkg\Wpkg.js `

**Example with everything configured via Actuve Directory Group policy settings:** `wpkg-gp-versionnumber_[x86|x64].exe /S /DisableViaLGP`

## Install with Wpkg ##
Starting with version 0.11 you may now upgrade Wpkg-GP from Wpkg-GP.

The recommended packages.xml entry for Wpkg is:
```xml

<package id="wpkg-gp" name="Wpkg-GP" revision="1">
<check type="uninstall" condition="versiongreaterorequal" path="Wpkg-GP 0.15 (x86)" value="0.15"/>

<install cmd="\\path\to\wpkg-gp-0.NN_x64.exe /S /INI \\path\to\wpkg-gp.ini">

<exit code="3010" reboot="delayed" />



Unknown end tag for &lt;/install&gt;



<upgrade cmd="\\path\to\wpkg-gp-0.NN_x64.exe /S /INI \\path\to\wpkg-gp.ini" >

<exit code="3010" reboot="delayed" />



Unknown end tag for &lt;/upgrade&gt;





Unknown end tag for &lt;/package&gt;


```

If you also want to be able to remove wpkg-gp and disable wpkg client see [here](https://code.google.com/p/wpkg-gp/issues/detail?id=94) for a more sophisticated version.

## Install via AD ##
A tool to generate a MSI file that can be deployed via AD is included `MakeMSI.exe`. Please note that this tool is only supported for GPO deployment. For all other installations, please use the exe installer.

```
Usage: makemsi x64|x32 installerfile inifile msifile
    
Generates a MSI file to be installed with the settings in the INI file
Example: makemsi x64 c:\path_to\Wpkg-GP-0.12_x86.exe c:\path_to\Wpkg-GP.ini c:\path_to\Wpkg-GP-0.12_x86.msi
```

## Install via Group Policy scripts ##
This is a simple method to make sure wpkg-gp is installed
  * Paste the following to a file named install\_wpkg\_gp.cmd
```
@ECHO OFF
cscript \\path\to\wpkg.js /install:wpkg-gp
```
  * Configure a package for Wpkg-GP named wpkg-gp in your packages.xml (for example by using the above package as a basis.
  * In your Group Policy Management Editor, open the target Group Policy
  * Navigate to Computer Configuration/Policies/Windows Settings/Scripts
  * Open Startup
  * Add the UNC path to wpkg-gp.cmd and click OK
  * **NOTE:** Make sure the computer account has access to the share where wpkg.js and Wpkg-GP installer resides on.

## How to configure ##
Starting with version 0.11 no Group Policy configuration is necessary if you enable it via Local Group Policies on installation. The settings provided are written to the wpkg-gp.ini file in the installation dir on installation and used. The settings can be changed by editing wpkg-gp.ini, or reinstalling Wpkg-GP.

## Configure via Wpkg-GP.ini ##
Starting with version 0.11, Wpkg-GP is configured via an ini-file residing in Wpkg-GP's installation directory.

Settings set via Group Policies overrides settings set in Wpkg-GP.ini, unless you specify it otherwise.

You can find an example of the default Wpkg-GP.ini in the [source code](http://code.google.com/p/wpkg-gp/source/browse/trunk/Wpkg-GP.ini) that you may use as a basis for your own ini-file.

## Configure via Group Policies ##
You may still configure it via Group Policies if necessary.
If you do not enable Wpkg-GP via Local Group Policies on installation, it will not be activated unless the corresponding administrative template is configured.

This can either be done on via Local Group Policies (LGP) per computer, or via a Domain Group Policy.
  * Copy admx files to be find [here](https://code.google.com/p/wpkg-gp/source/browse/#svn%2Ftrunk%2Fsrc%2Fadmx) to %systemroot%\PolicyDefinitions.
  * Open the group policy editor (gpedit.msc for LGP).
  * Navigate to "Computer Configuration -> Administrative Templates" -> "WPKG-GP"
    * **WPKG file path:** Path to "\\path\to\wpkg.js" or a batch file to start wpkg.js. Be sure to use an UNC path if it is located on the network, as no drives are mapped when GPE's are run. Remember: wpkg-gp is only tested with WPKG 1.1.2.
      * _Example:_ \\filesrv01\WPKG\wpkg.js or "cscript \\path\wpkg.js /myextraflags"
    * **WPKG Verbosity:** A number between 0 and 3. The greater the number, the more "chatty" messages will appear in your event log.
      * _Example:_ 1
> > > > ![http://wpkg-gp.googlecode.com/svn/wiki/images/gpedit-xp-wpkg-adm.png](http://wpkg-gp.googlecode.com/svn/wiki/images/gpedit-xp-wpkg-adm.png)
    * **Maximum number of consecutive reboots**

**NOTE:** The ADMX file is updated with each release, so make sure you use the latest for your Group Policies.

## How to disable asynchronous execution of GP ##
_Starting with WPKG-GP 0.10 this is done by the installer_. However if you are using Group Policies, you may have overridden this in your Group Policy settings.

Client versions of Windows is by default configured to execute Group Policies asynchronously. This means that it will try to execute WPKG-GP in the background while the user is logging in. WPKG-GP is configured to deny that, and that results in GP executing the Group policies in the foreground on next reboot. To avoid having to restart twice, you can disable asynchronous execution of Group Polices. It is normally considered a "best practice" to do this on computers connected to a domain, as a lot of the GPO's will not be applied correctly without network connection.

Apply a GPO with the setting Always wait for the network at computer startup and logon to the computer. This setting is located under **Computer Configuration\Administrative Templates\System\Logon** in the Group Policy Object Editor. For this setting to take effect, Group Policy must be refreshed or the computer restarted. To ensure a consistent experience in a mixed desktop environment, consider enabling this setting for all your Windows computers.

**Note:** Some computers (or combination of computers/network cards), use long time at boot to configure the network, or even report back that the network is up although it is not yet. Some users have reported this with Lenovo laptops and HP computers. Raising the GpNetworkStartTimeoutPolicyValue setting might help on this issue. See http://support.microsoft.com/kb/2421599 for more information about this setting.

Windows XP doesn't have system setting available under Administrative Templates in gpedit.msc by default. You can enable it by right clicking on "Administrative Templates" -> Add/Remove Templates -> Add -> system.adm -> Open -> Close

## How to enable verbose startup and shutdown messages ##
_Starting with WPKG-GP 0.10 this is done by the installer_. However if you are using Group Policies, you may have overridden this in your Group Policy settings.

Windows >= Vista does not show any kind of startup messages, other than "Please wait..." and "Starting Windows". To show the messages from WPKG-gp (and other status messages on startup and shutdown, you must enable verbose startup and shutdown messages in either your Group Policy, or in the registry. Information on this is available here:
http://support.microsoft.com/kb/325376 Even though the article is about Windows Server 2003, the information applies to all versions of Windows.