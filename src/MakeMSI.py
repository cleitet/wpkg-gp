# MakeMSI.py - A tool for generating MSI file with a INI file for installing WPKG-GP
import msilib, msilib.schema, msilib.sequence, msilib.text, os.path, os, shutil, tempfile
import WpkgConfig
from win32api import GetFileVersionInfo, LOWORD, HIWORD


# We want the installer to use MSI, but not leave a trace in the registry
# http://msdn.microsoft.com/en-us/library/aa367519.aspx

class FileObj(object):
    def __init__(self, path):
        self.name = os.path.basename(path)
        self.path = path

class IniFileObj(FileObj):
    def __init__(self, path):
        FileObj.__init__(self, path)
        self.size = os.path.getsize(path)

class ExeFileObj(IniFileObj):
    def __init__(self, path):
        IniFileObj.__init__(self, path)
        self.get_fileversion()
        
    def get_fileversion(self):
        try:
            info = GetFileVersionInfo (self.path, "\\")
            ms = info['FileVersionMS']
            ls = info['FileVersionLS']
            self.fileversion = (HIWORD(ms), LOWORD(ms), HIWORD(ls), LOWORD(ls))
        except:
            self.fileversion = (0,0,0,0)

    
    def get_fileversion_as_string(self):
        return".".join([str (i) for i in self.fileversion])
        

class WpkgGpMsi(object):
    def __init__(self, platform, exefile, inifile, msifile):
        self.platform = platform
        self.exefile = ExeFileObj(exefile)
        self.inifile = IniFileObj(inifile)
        self.msifile = FileObj(msifile)
        config = WpkgConfig.WpkgConfig()
        self.template = os.path.join(config.install_path, "MSI", "wpkg-gp_%s.msitemplate" % self.platform)
        #self.template = r"Z:\wpkg-devel\wpkg-gp\trunk\src\msi\wpkg-gp_x86.msitemplate"
        self.tempdir = tempfile.mkdtemp()
        self.productcode = msilib.gen_uuid()
        self.productversion = self.exefile.get_fileversion_as_string()
        self.manufacturer = "The Wpkg-GP team"
        
    def generate_MSI(self):
        shutil.copy(self.template, self.msifile.path)
        #1 Add exe and ini file to cab
        filelist = [(self.exefile.path, "ExeFile"), (self.inifile.path, "IniFile")]
        cabfile  = os.path.join(self.tempdir, "files.cab")
        msilib.FCICreate(cabfile, filelist)

        #2 Open the MSI database
        #database = msilib.init_database(self.msifile.path, msilib.schema, self.msifile.name,  self.productcode, self.productversion, self.manufacturer)
        #print self.msifile.path
        #msilib.add_tables(database, msilib.schema)
        database = msilib.OpenDatabase(self.msifile.path, msilib.MSIDBOPEN_DIRECT)

        msilib.add_stream(database, "Wpkg_GP.cab", cabfile)

        # Add information to Media
        # DiskId | LastSequence | DiskPrompt | Cabinet | VolumeLabel | Source
        table = "Media"
        records = [(1, 2, None, "#Wpkg_GP.cab", None, None)]
        msilib.add_data(database, table, records)

        #CAB = msilib.CAB("Wpkg_GP.cab")
        #CAB.append(self.exefile.path, "ExeFile", "ExeFile")
        #CAB.append(self.inifile.path, "IniFile", "IniFile")
        #CAB.commit(database)

        # Add information to File
        # File | Component_ | FileName | FileSize| Version | Language | Attributes | Sequence
        table = "File"
        records = [
            ("ExeFile", "Installer", self.exefile.name, self.exefile.size, None, None, 512, 1),
            ("IniFile", "Installer", self.inifile.name, self.inifile.size, None, None, 512, 2)
            ]
        msilib.add_data(database, table, records)

        
        # Add information to CustomAction
        # Action | Type | Source | Target
        # For Type, see: http://msdn.microsoft.com/en-us/library/aa372048%28v=VS.85%29.aspx
        

        database.Commit()


def main():
    try:
        if sys.argv[1] == "-h":
            usage()
        platform = sys.argv[1]
        exefile = sys.argv[2]
        inifile = sys.argv[3]
        msifile = sys.argv[4]
        if platform != "x64" and platform != "x86":
            usage("Error: Platform must be x64 or x86, you entered %s" % platform)
        
    except IndexError:
        usage("Error: Invalid number of parameters")
    print "Generating %s with the following settings:" % msifile
    print "Platform.: %s" % platform
    print "Installer: %s" % exefile
    print "Ini file.: %s" % inifile
    MSI = WpkgGpMsi(platform, exefile, inifile, msifile)
    MSI.generate_MSI()
        
    
def usage(error=None):
    my_name = os.path.split(sys.argv[0])[1]
    if error != None:
        print
        print error
        print
    print """Usage: %s x64|x32 installerfile inifile msifile
    
Generates a MSI file to be installed with the settings in the INI file
Example: %s x64 c:\path_to\Wpkg-GP-0.12_x86.exe c:\path_to\Wpkg-GP.ini c:\path_to\Wpkg-GP-0.12_x86.msi""" % (my_name, my_name)
    sys.exit(0)

if __name__=='__main__':
    import sys
    main()
        
        