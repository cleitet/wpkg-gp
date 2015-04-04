This is the documentation on how to build the dll components in Visual C++ 2010 Express.

Note that it is not often necessary to build these, as most of the work is done in the python scripts, and the compiled files are included in the source code releases in SVN.

Visual C++ 2010 Express needs som tweaking in order to work correctly. But most of the tweaks are done in the project files already.

# What you need #
  * Visual Studio C++ 2010 Express
  * Windows 7 SDK for AMD64 builds

# Installing Visual Studio C++ 2010 Express #
This is necessary in order to build the wpkg-gp-test.exe and wpkg-gp.dll files.

# Installing Windows 7 SDK #
The Windows 7.1 SDK contains cross compiler for AMD64 platform, and must be installed after Visual Studio C++ 2010 Express.
  * Download and install the Windows 7.1 SDK

# Tweaks necessary to create the project in Visual C++ Express 2010 #
These changes are already made, so you would not need to do them. However, they are provided here as a reference

## Adding 64-bit support ##
It is necessary to do this manually:
  * Solution -> Properties -> Configuration Properties -> Configuration Manager
  * Active Solution Platform -> New
  * Platform: x64
  * Copy Settings from: Win32

## Setting stdcall calling convention for WPKG-gp.dll ##
It seems that Group Policy Extentions need to have stdcall calling convention set.
  * On all projects:
  * Properties
  * Configuration -> All Configurations
  * Platform -> All platforms
  * C/C++ -> Advanced -> Calling Convention -> **stdcall (/GZ)**

## Using Windows7.1 SDK ##
  * On all projects:
  * Properties
  * Configuration -> All Configurations
  * Platform -> All platforms
  * General -> Platform Toolset: **Windows7.1SDK**

## Using .def file for exporting entry point function in WPKG-gp.dll ##
You have to use a .def file to export the function name without any "decoration".
  * Added a .def file as a Resource File
  * WPKG-gp -> Properties -> Linker -> Input -> Module Definition File -> **$(InputDir)\Export.def**

## Change output dirs ##
  * On all projects:
  * Properties
  * Configuration -> All Configurations
  * Platform -> All platforms
  * General -> Output directory: **$(SolutionDir)$(Platform)\$(Configuration)\**
  * General -> Intermediate directory: **$(Platform)\$(Configuration)\**