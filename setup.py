from distutils.core import setup
from py2exe import *
from glob import glob
import os

version = os.environ.get("version")
architecture = os.environ.get("arch")
company_name = "The WPKG-GP Team (http://wpkg-gp.googlecode.com)"
copyright = "Copyright 2010 The WPKG-GP team"


class Target:
    def __init__(self, **kw):
        self.__dict__.update(kw)
        self.version = version
        self.company_name = company_name
        self.copyright = copyright

WpkgPipeClient = Target(
    description = "WPKG-GP Command Line Pipe Client",
    name = "WPKG-GP Command Line Pipe Client",
    script = "src\WpkgPipeClient.py"
 )


WpkgServer = Target(
    # used for the versioninfo resource
    description = "WPKG-GP Windows Service",
    name = "WPKG-GP Windows Service",
    # what to build. For a service, the module name (not the
    # filename) must be specified!
    modules = ["WpkgServer"],
    cmdline_style='pywin32'
)

data_files = [("Microsoft.VC90.CRT", glob(r'redist\VC90\Microsoft.VC90.CRT-'+architecture+'\*.*'))]

setup(
    version = version,
    package_dir = {'': 'src'},
    packages = [''],
    data_files=data_files,
    service = [WpkgServer],
    console=[WpkgPipeClient])
