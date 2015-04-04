![http://wpkg-gp.googlecode.com/svn/wiki/images/bootup-windows-7-small.png](http://wpkg-gp.googlecode.com/svn/wiki/images/bootup-windows-7-small.png)

Many want to run [WPKG](http://wpkg.org) at boot up, before user logon, and want their users to get some status information. This has traditionally been done either by running WPKG as a [Group Policy Script](http://wpkg.org/WPKG_with_Active_Directory), or by running the WPKG project's [WPKG Client](http://wpkg.org/WPKG_Client). The first has several limitations, the second [does not support delayed logon on Windows >= Vista](http://wpkg.org/Screenshots#Windows_Vista) due to [new security features in Windows](http://www.microsoft.com/whdc/system/sysinternals/Session0Changes.mspx).

### How does it work? ###
Wpkg-GP circumvents this by hooking into the Group Policy Extensions (GPE) interface available in Windows (the one saying "Applying Group Policies" at boot up), and executing wpkg.js, providing some feedback to the user via the interfaces made available by GPE. The BehindTheScenes wiki page explains this deeper.

### Great! How do I use it? ###
Visit the [downloads](https://drive.google.com/folderview?id=0B9Eadi-crzpOVEtTM01aYm5YNm8&usp=sharing) page to grab the installer or [source](http://code.google.com/p/wpkg-gp/source/checkout) page to check out the source code. Be sure to read the [Installation Guide](InstallationGuide.md) as well. If you have any questions, feel free to send a mail to the [discussion group](mailto:wpkg-gp@googlegroups.com) or visit the [archives](http://groups.google.com/group/wpkg-gp).

# News #

---


## 2014-05-13 ##
Another bug fix release, wpkg-gp 0.17 is available.

No changes of setup from beta version 0.17b1 except that the internal version number is increased to 0.17.

Changes in detail:
  * Fixed a regression so that clear passwords can now be put in quotes again ([r286](https://code.google.com/p/wpkg-gp/source/detail?r=286), [issue #105](https://code.google.com/p/wpkg-gp/issues/detail?id=#105))
  * Replaced the percent indicator with package number processed of total number of packages ([r288](https://code.google.com/p/wpkg-gp/source/detail?r=288), [issue #96](https://code.google.com/p/wpkg-gp/issues/detail?id=#96))
  * Fixed invalid gpt.ini manipulation which could cause wpkg-gp or other extensions not to run ([r287](https://code.google.com/p/wpkg-gp/source/detail?r=287), [issue #92](https://code.google.com/p/wpkg-gp/issues/detail?id=#92))
  * Replaced the adm group policy files with newer admx files (Thanks to michal.sacharewicz) ([r290](https://code.google.com/p/wpkg-gp/source/detail?r=290), [issue #107](https://code.google.com/p/wpkg-gp/issues/detail?id=#107))

## 2014-02-18 ##
After a long time of silence there is a new release available for
> WPKG-GP version 0.16

This is a bug fix release with following changes:
  * It is now possible to use wpkg commands with spaces ([r281](https://code.google.com/p/wpkg-gp/source/detail?r=281), [issue #82](https://code.google.com/p/wpkg-gp/issues/detail?id=#82)) Remember to use quotes if you want to do so.
  * Group policy settings are now reverted when uninstalling wpkg-gp ([r282](https://code.google.com/p/wpkg-gp/source/detail?r=282), [issue #79](https://code.google.com/p/wpkg-gp/issues/detail?id=#79))
  * Optimized speed when connecting to network share fails ([r280](https://code.google.com/p/wpkg-gp/source/detail?r=280), [issue #83](https://code.google.com/p/wpkg-gp/issues/detail?id=#83))
  * Made number of retries and sleeping time between trying to connect to network share configurable ([r275](https://code.google.com/p/wpkg-gp/source/detail?r=275), [issue #83](https://code.google.com/p/wpkg-gp/issues/detail?id=#83))
  * Added optional tcp connection test to reduce waiting time ([r274](https://code.google.com/p/wpkg-gp/source/detail?r=274), [r276](https://code.google.com/p/wpkg-gp/source/detail?r=276), [issue #83](https://code.google.com/p/wpkg-gp/issues/detail?id=#83))
## 2011-08-15 - Release 0.15 ##
This version has major number of bug fixes and enhancements
  * New features
    * Much less verbose in logs. Now it only logs issues in event log, and uses WPKG.js itself (with /quiet switch) for posting informational messages in event log. ([r230](https://code.google.com/p/wpkg-gp/source/detail?r=230), [issue #57](https://code.google.com/p/wpkg-gp/issues/detail?id=#57))
    * You can now choose which features to install in silent mode with the switch /Features Client|Adm|MSITool, for more information run the installer with the /? switch. ([r236](https://code.google.com/p/wpkg-gp/source/detail?r=236), [issue #54](https://code.google.com/p/wpkg-gp/issues/detail?id=#54))
    * Improved speed ([r256](https://code.google.com/p/wpkg-gp/source/detail?r=256), [issue #74](https://code.google.com/p/wpkg-gp/issues/detail?id=#74))
  * Fixes
    * Issues with User Local Group Policies running in parallel with Wpkg-GP Computer Local Group Policies ([r231](https://code.google.com/p/wpkg-gp/source/detail?r=231), [r250](https://code.google.com/p/wpkg-gp/source/detail?r=250), [r254](https://code.google.com/p/wpkg-gp/source/detail?r=254), [issue #56](https://code.google.com/p/wpkg-gp/issues/detail?id=#56), [issue #65](https://code.google.com/p/wpkg-gp/issues/detail?id=#65))
    * Is now using account service is running as to connect to network if no user name is provided ([r232](https://code.google.com/p/wpkg-gp/source/detail?r=232), [issue #58](https://code.google.com/p/wpkg-gp/issues/detail?id=#58))
    * Progress indicator is now containing white spaces to make the boot screen flicker less. ([r233](https://code.google.com/p/wpkg-gp/source/detail?r=233), [issue #57](https://code.google.com/p/wpkg-gp/issues/detail?id=#57))
    * You now have to install Wpkg-GP if you want to install the MSI tool as they share libraries. ([r234](https://code.google.com/p/wpkg-gp/source/detail?r=234))
    * Will not install MPR.dll which caused issues on 32-bit Vista ([r235](https://code.google.com/p/wpkg-gp/source/detail?r=235))
    * Fixed a string formatting typo that caused issues when errors occurred. ([r239](https://code.google.com/p/wpkg-gp/source/detail?r=239), [issue #42](https://code.google.com/p/wpkg-gp/issues/detail?id=#42))
    * Reboot messages missed a status code, and appeared with the first four characters missing. ([r242](https://code.google.com/p/wpkg-gp/source/detail?r=242))
    * Fixed error when installer was called with /WpkgCommand ([r243](https://code.google.com/p/wpkg-gp/source/detail?r=243), [issue #65](https://code.google.com/p/wpkg-gp/issues/detail?id=#65))
    * Fixed issue with encrypted password and characters either misinterpreted by ini parser library or somewhere else by adding a hash (#) at end of all encrypted passwords, making the ini parser library qoute them. ([r244](https://code.google.com/p/wpkg-gp/source/detail?r=244), [issue #66](https://code.google.com/p/wpkg-gp/issues/detail?id=#66))
    * Fixed buffer overflow issue causing random crashes in wpkg-gp.dll and wpkg-gp-test.exe. ([r247](https://code.google.com/p/wpkg-gp/source/detail?r=247), [issue #63](https://code.google.com/p/wpkg-gp/issues/detail?id=#63))
    * Fixed major issues with !MakeMSI tool ([r248](https://code.google.com/p/wpkg-gp/source/detail?r=248), [issue #64](https://code.google.com/p/wpkg-gp/issues/detail?id=#64))
    * Fixed server crashing when Event Log is full ([r252](https://code.google.com/p/wpkg-gp/source/detail?r=252), [issue #67](https://code.google.com/p/wpkg-gp/issues/detail?id=#67))
    * Fixed server crashing if Group Policy Directory is not found ([r253](https://code.google.com/p/wpkg-gp/source/detail?r=253), [issue #68](https://code.google.com/p/wpkg-gp/issues/detail?id=#68))
    * Is now not logging password in clear text ([r255](https://code.google.com/p/wpkg-gp/source/detail?r=255), [issue #70](https://code.google.com/p/wpkg-gp/issues/detail?id=#70))


## 2011-08-05 - Release 0.14 ##
This release is mainly fixing a critical bug introduced in 0.13, which made it impossible to connect to the network with the provided credentials

  * New features
    * New setting: DisableAtBootUp which disables execution at bootup
      * Technical details: The Wpkg-gp.dll GPE now sends ExecuteFromGPE instead of Execute. If this command is received, the service checks if DisableAtBootUp is set. Sending Execute to the service works as before. ([r225](https://code.google.com/p/wpkg-gp/source/detail?r=225))
  * Fixes
    * Connecting to network user was broken ([issue #52](https://code.google.com/p/wpkg-gp/issues/detail?id=#52), [r222](https://code.google.com/p/wpkg-gp/source/detail?r=222))
    * Some of the output was missing when the activity indicator was showing ([r218](https://code.google.com/p/wpkg-gp/source/detail?r=218))
    * In certain scenarios the last line was not retrieved correctly when Wpkg.js returned an error ([r219](https://code.google.com/p/wpkg-gp/source/detail?r=219), [r221](https://code.google.com/p/wpkg-gp/source/detail?r=221), [r223](https://code.google.com/p/wpkg-gp/source/detail?r=223))
    * Reduced lookups in ini file ([r220](https://code.google.com/p/wpkg-gp/source/detail?r=220))

## 2011-08-03 - Release 0.13 ##
  * New features
    * EnableViaLGP is now on by default, to make it possible to run out of the box. New switch /DisableViaLGP disables it if you want to run the installer silent. ([r213](https://code.google.com/p/wpkg-gp/source/detail?r=213))
    * Installer aborts if you do not provide a path to WpkgCommand and are using Local Group Policies (default). ([r214](https://code.google.com/p/wpkg-gp/source/detail?r=214))
    * Support for environment variables in Wpkg-GP.ini under the [EnvironmentVariables](EnvironmentVariables.md) section [r199](https://code.google.com/p/wpkg-gp/source/detail?r=199)
    * Now we have an "activity indicator" (moving dots) to indicate that the process has not hung while installing packages. ([r210](https://code.google.com/p/wpkg-gp/source/detail?r=210), [issue #51](https://code.google.com/p/wpkg-gp/issues/detail?id=#51))
    * Modularized Wpkg-GP.adm Administrative template to be easier to configure and making it possible to only configure parts of it. This is handy if you want to configure WpkgCommand via Group Policies, but username/password via INI. ([r212](https://code.google.com/p/wpkg-gp/source/detail?r=212))
  * Fixes:
    * Workarounds for scenarios where execution of Wpkg-GP started before the network was fully operable ([r198](https://code.google.com/p/wpkg-gp/source/detail?r=198)).
    * Issues with logrotation ([r200](https://code.google.com/p/wpkg-gp/source/detail?r=200)).
    * Explained how to escape # characters in Wpkg-GP.ini ([r201](https://code.google.com/p/wpkg-gp/source/detail?r=201), [issue #47](https://code.google.com/p/wpkg-gp/issues/detail?id=#47))
    * Issues with the security features from 0.12 ([r204](https://code.google.com/p/wpkg-gp/source/detail?r=204), [r204](https://code.google.com/p/wpkg-gp/source/detail?r=204), [issue #46](https://code.google.com/p/wpkg-gp/issues/detail?id=#46))
    * Issue with ini files provided at setup not named wpkg-gp.ini ([r207](https://code.google.com/p/wpkg-gp/source/detail?r=207) [issue #50](https://code.google.com/p/wpkg-gp/issues/detail?id=#50))
    * Issue with looking up account information on system with other languages
> > > than English, as account names was hardcoded. Is now using well known
> > > SID's instead. ([r208](https://code.google.com/p/wpkg-gp/source/detail?r=208), [issue #49](https://code.google.com/p/wpkg-gp/issues/detail?id=#49))
    * Issue when using relative paths to provide /INI file to installer. ([r211](https://code.google.com/p/wpkg-gp/source/detail?r=211))

## 2011-07-19 - Release 0.12 ##
  * New features
    * Security feature added to limit which users have access to execute Wpkg. See [Wpkg-GP.ini](http://code.google.com/p/wpkg-gp/source/browse/trunk/Wpkg-GP.ini) for explanation.
      * Group Policy administrative template is updated with settings for this feature as well.
    * Added a tool to generate a MSI installer from a installer and INI file ([r187](https://code.google.com/p/wpkg-gp/source/detail?r=187), [r188](https://code.google.com/p/wpkg-gp/source/detail?r=188)) See the correct section of the [Installation guide](http://code.google.com/p/wpkg-gp/wiki/InstallationGuide?ts=1311066459&updated=InstallationGuide#Install_via_AD)
    * Updated to python 2.7
  * Fixes
    * Some logging issues fixed ([r183](https://code.google.com/p/wpkg-gp/source/detail?r=183), [r189](https://code.google.com/p/wpkg-gp/source/detail?r=189))
    * Reintroduced wpkg-gp-test.exe
## 2011-06-24 - Release 0.11 ##
  * New features
    * The installer is switched from WIX to NSIS, which now makes it possible for Wpkg-GP to upgrade itself from itself for future versions. However this will not work with previous versions, so you need to run the Wpkg-GP 0.11 installer from outside Wpkg-GP. The new installer will automatically uninstall any previous versions as well.
    * Switched from being configured by registry settings to an INI-file for easier distribution of the software to multiple computers. You can now provide a preconfigured INI file with the installer with your settings.
    * Modularized the source code. This will not be too visible for the users, but will help the maintainers mantain the code better.
  * Bugfixes:
    * A small number of bugfixes ant typo fixes.
    * Updated ADM file. If upgrading and using AD for configuration, be sure to upgrade this on your Domain Controllers and the Group Policy settings as well.
## 2011-03-15 - Release 0.10 ##
  * Bugfixes
    * Made the "Waiting for service" message at boot a little bit more consise. ([r124](https://code.google.com/p/wpkg-gp/source/detail?r=124))
    * Added dependency on LANMAN service, to make sure named pipes are working before starting service ([r125](https://code.google.com/p/wpkg-gp/source/detail?r=125))
  * New features
    * Preliminary support for computers that are not a part of an Active Directory domain ([r129](https://code.google.com/p/wpkg-gp/source/detail?r=129)) by using Local Group Policies. This functionality has not been tested extensively, so use with caution. We have not tested it on Windows Vista/7 home edition, but we expect it to work there as well, as the LGP libraries are in place, even though the gpedit.msc is missing.
    * Modified installer to make it easier to configure WPKG without using Group Policies ([r131](https://code.google.com/p/wpkg-gp/source/detail?r=131))
    * Improved logging. WPKG-GP is now using log files rather than event log for verbose logs. This will make it easier to track down issues and bugs ([r132](https://code.google.com/p/wpkg-gp/source/detail?r=132), [r133](https://code.google.com/p/wpkg-gp/source/detail?r=133), [r134](https://code.google.com/p/wpkg-gp/source/detail?r=134), [r135](https://code.google.com/p/wpkg-gp/source/detail?r=135), [r139](https://code.google.com/p/wpkg-gp/source/detail?r=139), [r140](https://code.google.com/p/wpkg-gp/source/detail?r=140))
    * It is now possible to build x86 on x86 platform. (x64 platform can build both x86 and x64) ([r136](https://code.google.com/p/wpkg-gp/source/detail?r=136))
    * Parsing of Wpkg Execute string is improved. It now adds "cscript" to beginning of string, if it points to a file ending in ".js". It also adds /noReboot, /synchronize, /sendStatus and /nonotify automatically if the flags are missing. ([r134](https://code.google.com/p/wpkg-gp/source/detail?r=134), [r138](https://code.google.com/p/wpkg-gp/source/detail?r=138)) This means you may give the path to wpkg.js with no prefix or arguments, and it will still work.
    * The installer now configures Group Policies to be applied asynchronously when installing WPKG-GP. NOTE: This will be overridden by your Group Policies setting though, so make sure it is turned on in your Group Policies as well if in an Active Directory environment ([r141](https://code.google.com/p/wpkg-gp/source/detail?r=141)).

All the configurable settings can be configured via the command line when installing WPKG-GP. Please see the [Installation Guide](InstallationGuide.md) for more information.

A client is being developed that will enable you to create a MST (transforms file) for the settings as well.

## 2011-01-10 - Release 0.9 ##
  * Bugfixes
    * Issues with WPKG-GP on 64 bit where GPE did not work ([revision #122](https://code.google.com/p/wpkg-gp/source/detail?r=#122))
    * When waiting for service to start, you had to wait for two minutes ([r118](https://code.google.com/p/wpkg-gp/source/detail?r=118) [issue 22](https://code.google.com/p/wpkg-gp/issues/detail?id=22))
  * New features
    * Native 64-bit support with the help of [abadent](http://code.google.com/u/abadent/) ([issue #19](https://code.google.com/p/wpkg-gp/issues/detail?id=#19), [r110](https://code.google.com/p/wpkg-gp/source/detail?r=110))
    * Debug logging for GPE in place ([r120](https://code.google.com/p/wpkg-gp/source/detail?r=120))

## 2011-01-05 - Release 0.8 ##
  * Some important bugs fixed:
    * "wpkg-gp-test.exe -debug" now works on 64 bit
    * Shell variable expansion now works for the path to wpkg.js, e.g. "%SOFTWAREINSTALL%"\wpkg.js
  * Some new killer features:
    * WPKG-GP now has built-in functionality to connect to the network as another user. This can be set at install time via the installation wizard. For more info, see the updated [installation guide](InstallationGuide.md). This means that you do not have to expose the password to your WPKG network user in the registry anymore. WPKG encrypts the password and saves it using a Windows function that only the user the service is running as can decrypt. For more info, see the [network user](NetworkUser.md) wiki page.
  * A new administrative template which exposes the "Max Reboots" setting introduced in 0.7.

What is pending before we have a stable platform?
  * Native x64 support, not the dual x64/x32 with wow64 we have now.
  * A "kill switch" (it actually exists already) feature for interrupting the WPKG execution, and the possibility to set a maximum execution time for WPKG.
  * Anything else? Send a feature request to the mailing list, or create a new issue.

## 2010-12-30 - Release 0.7 ##
  * Some bug fixes and new features, please test! Pretty please :)
  * Randomly hanging at startup (executing before service have started) is fixed. Hopefully this resolves [issue #15](https://code.google.com/p/wpkg-gp/issues/detail?id=#15) and [issue #16](https://code.google.com/p/wpkg-gp/issues/detail?id=#16)
  * Reboots are now working
  * Maximum number of consecutive reboots is implemented (standard value is 10), resolving [issue #4](https://code.google.com/p/wpkg-gp/issues/detail?id=#4)
  * Extensive logging is now possible
    * New switch to wpkg-gp-client.exe "-debug" gives verbose debug messages
    * Three levels of verbosity possible for the service: 1-3
## 2010-12-16 - Release 0.6 ##
  * We have finally managed to have something new to release. This is a complete rewrite, featuring:
  * Python as the primary language for functionality. This means more rapid development, easier to manage code and hopefully even more people will be able to participate.
  * A split into a service reading settings from registry, executing wpkg, and parsing wpkg's output and a client connecting to the service via named pipes and instructing it to start wpkg.js and reading the parsed output. This gives us a lot of extensibility.
  * As this is a complete rewrite, we need testers, so please test and report issues and wishes!

## 2010-07-30 - Release 0.5 ##
  * Although there has been a while since the last news update, but the project has had a lot of updates
  * Release 0.3 and 0.4 introduced x64 support
  * Release 0.4 migrated the project to VC++ Express 2010 (native x64 support)
  * Release 0.5 fixed an installer issue (trying to install VC++ 2010 Redist components even if only installing the Group Policy templates)
  * We cannot guarantee the stability of 0.5 yet (testers needed especially for the x64 part)

## 2010-03-30 - Release 0.2! ##
  * This fixes [issue 6](http://code.google.com/p/wpkg-gp/issues/detail?id=6)
  * Adm file updated with more documentation
  * MSI file autoupdates itself (replaces previous version)
  * InstallationGuide doc is updated with new screenshot of the adm file.
  * Thanks to peter@s...org and google@d...de for feedback and even a patch. That's our first!

## 2010-02-16 - Initial Release! ##

We are proud to present to you an initial release of both the source code and msi installation file for wpkg-gp. Stay tuned for some documentation that will arrive soon too!

Please note that this is the _initial_ release, so most likely you will run into some issues. Please notify us about these via the [Issue Tracking interface](http://code.google.com/p/wpkg-gp/issues/list)

---
