wpkg-gp is a group policy extension executing wpkg.js, parsing it's output and presenting the parsed output to the user before allowing him to log in.

## Group Policy Extension ##
When Windows starts it runs Group Policies synchronously (in the foreground). Before login, it processes Computer GPO's, and after login it processes User GPO's. As long as the computer is on, it checks for updates to the GPO's and processes them asynchronously (in the background). The default interval for this is 90 minutes, but this can be adjusted.

GPO settings are normally just changes to the registry. But some changes needs to update files or make applications aware of them. To be able to do this, you have so called Group Policy Extensions, which are being called by the GPO framework to perform the necessary actions.

A Group Policy Extension (GPE) is really .dll plugins to the Group Policy framework which is integrated into Microsoft Windows. If a Group Policy setting (for example an administrative template) is configured to use a specified GPE, the corresponding dll is called.

This is what we want. We want to have a GPE execute wpkg.js at bootup. The wpkg-gp GPE is configured to only be run at boot up (synchronously, in the foreground), and it is configured to only be run for when the computer GPO's are being processed (before logon).

## What happens? ##
The GPE (wpkg-gp.dll) is executed by winlogon.exe, and is given a function that allows it to change the status messages presented to the user before login.

wpkg-gp.dll checks if the WpkgService has started yet, and if not, it waits for it to finish starting. Then it connects to the WpkgService via a named pipe and instructs it to start to execute wpkg.js. The settings for how wpkg.js should be started etc. are already read from the registry by the WpkgService. The service returns the parsed output from wpkg.js to wpkg-gp.dll which shows it to the user.

## What is wpkg-gp-test.exe ##
This file is installed with wpkg-gp and does the same as wpkg-gp.dll and is used for debugging. As it's necessary to reboot Windows to execute wpkg-gp.dll, we have made this exe file to make debugging easier. It will execute WpkgPipeClient.exe and prompt the output. If you provide the "-debug" flag, it will be pretty verbose, which is good for debugging.

## How is parsing being done? ##
As for now, we run wpkg.js with the /sendStatus flag, and parses it, hopefully it will be consistent in showing which package is being installed/upgraded/removed and some progress information.

## How does WpkgPipeClient and WpkgServer interact? ##
WpkgPipeClient connects to WpkgServer via named pipes. This means that WpkgServer can execute as another user than WpkgPipeClient, making installation, reboots etc happen as a privileged user.