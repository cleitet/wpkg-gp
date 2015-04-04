## Introduction ##

Hints for basic troubleshooting and summary of experiences from other users.

## Why is Wpkg-GP not running on most computers, but not on some? Or: Why are Wpkg-GP sometimes not running? ##
Some computers (or combination of computers/network cards), use long time at boot to configure the network, or even report back that the network is up although it is not yet. Some users have reported this with Lenovo laptops and HP computers. Raising the GpNetworkStartTimeoutPolicyValue? setting might help on this issue. See http://support.microsoft.com/kb/2421599 for more information about this setting.

## This can also be defined via a group policy ##

Policy Location: Computer Configuration > Policies > Admin Templates > System > Group Policy
Setting Name: Startup policy processing wait time