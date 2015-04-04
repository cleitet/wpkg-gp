# Prerequisites #
WPKG-GP is written mainly in Python starting with version 0.6. The Python scripts are converted to windows executables with py2exe. If you intend to build for both x86 and x64, you will need a x64 buildhost, and to have the python packages for both x86 and x64 installed in parallell.

  * Python 2.7
  * Py2exe for python 2.7
  * win32python for python 2.7
  * NSIS 2.46 with logging patch
    * Install NSIS 2.46 and copy in the contents of  the logging patch available at: http://nsis.sourceforge.net/Special_Builds
  * Visual C++ Express 2010 (If building dll)
  * Windows SDK 7.1 (If building dll)

We have embedded the dll file in the SVN repository, as it is not likely to change too often, and you should be able to participate in the project without knowing C++ or having to install Visual C++ Express or Windows SDK 7.1. You should be able to understand Python though :)

## Note about 64 bit support ##
Starting with 0.9 we build all the python binaries for the x64 platform. The build scripts per default expect to find 32 bit python in `c:\python 2.7(x86)` and 64 bit python in `c:\python 2.7(x64)`. You can change the paths in make\_installer.cmd.

# Building WPKG-GP #
To build new MSI files for WPKG-GP you only need to run make\_installer.cmd. It has hard coded paths to python and NSIS, so make sure all the necessary components are installed.

The syntax is `make_installer.cmd [x86|x64|all] [clean|make_py|make_installer|all]`

# Debugging WPKG-GP #
You can run WPKG-GP python scripts without converting them to exe files to make development faster.

WpkgServer.py is the service. In order to debug it, you need to:
  * Uninstall WPKG-GP as the service names will crash
  * Run "WpkgServer.py install" to register the wpkg service
  * Run "WpkgServer.py debug" to start the service in debug mode.
  * Press ctrl+c to end the service
  * Run "WpkgServer.py remove" to unregister the service, so you can reinstall the compiled client.

WPKGExecuter.py is primarily used as a module for WpkgServer.py, but can be executed standalone for debugging without having to launch the service.

WpkgPipeClient.py is the pipe client connecting to WpkgServer. It takes the first argument and sends it to the pipe server, and prints the output it receives, removing the first 4 characters which are status message numbers. You can also run WpkgPipeClient.py -s hostname [Command](Command.md) to execute a command on a remote computer.

# WPKG-GP Pipe Server commands #
The specs for the commands the Pipe Server accepts are:
  * **Execute** - Starts wpkg.js
  * **Cancel** - Ends an ongoing execution of wpkg.js