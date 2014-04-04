import re

class WpkgOutputParser(object):
    def __init__(self):
        self.reset()
        
    def reset(self):
        self.operation = "Initializing Wpkg-GP"
        self.package_name = ""
        self.pkgnum = 0
        self.pkgtot = 0
        self.updated = True
        self.started = False
        
    def parse_line(self, line_to_parse):
        #Remove all strings not showing "YYYY-MM-DD hh:mm:ss, STATUS  : "
        if not re.search("[0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}:[0-9]{2}, STATUS  : ", line_to_parse):
            self.updated = False
            return
        
        previous_operation = self.operation
        previous_package_name = self.package_name
        previous_pkgnum = self.pkgnum
        
        #Remove "STATUS"-part:
        line = ":".join(line_to_parse.split(":")[3:])[1:]
        
        #Checking current operation:
        if re.match("^Remove: Checking status", line):
            #No action is being performed, only updating internal percentage counter, but do not generate output
            self.operation = "removing"
            self.package_name, self.pkgnum, self.pkgtot = re.search("('.*') \(([0-9]+)/([0-9]+)\)$", line).group(1, 2, 3)
        elif re.match("^Remove: Removing package", line):
            self.operation = "removing"
            self.package_name = re.search("('.*')", line).group(1)
        elif re.match("^Install:", line):
            #No action is being performed, only updating internal percentage counter, but do not generate output
            self.operation = "verifying"
            self.package_name, self.pkgnum, self.pkgtot = re.search("('.*') \(([0-9]+)/([0-9]+)\)$", line).group(1, 2, 3)
        elif re.match("^Performing operation", line):
            #Operation is actually being performed
            operation, self.package_name = re.search(
                "^Performing operation \((.+)\) on ('.+')", line).group(1,2)
            #The description of the operation is misleading on this message, except for upgrades
            if operation == "upgrade":
                self.operation = "upgrading"
            elif operation == "install":
                self.operation = "installing"
        if self.pkgnum == previous_pkgnum and self.package_name == previous_package_name and self.operation == previous_operation:
            self.updated = False
        else:
            self.updated = True

    def get_formatted_line(self):
        if self.updated == True:
            return "Wpkg-GP is %s %s (%s/%s)" % (self.operation, self.package_name, self.pkgnum, self.pkgtot)
        else:
            return False

def main():
    example = """
2011-05-07 10:41:30, STATUS  : Starting software synchronization
2011-05-07 10:41:30, STATUS  : Number of packages to be removed: 1
2011-05-07 10:41:30, STATUS  : Remove: Checking status of 'Removeme' (1/1)
2011-05-07 10:41:30, STATUS  : Performing operation (install) on 'Removeme' (removeme)
2011-05-07 10:41:30, STATUS  : Remove: Removing package 'Removeme' (1/1)
2011-05-07 10:41:30, STATUS  : Install: Verifying package 'test' (1/2)
2011-05-07 10:41:30, STATUS  : Performing operation (install) on 'test' (rebootnow)
2011-05-07 10:41:30, STATUS  : Install: Verifying package 'donothing' (2/2)
2011-05-07 10:41:30, STATUS  : Performing operation (upgrade) on 'donothing' (donothing)
2011-05-07 10:41:30, STATUS  : Finished software synchronization"""
    parser = WpkgOutputParser()
    for line in example.split("\n"):
        parser.parse_line(line)
        if parser.updated == True:
            print parser.get_formatted_line()
        

if __name__=='__main__':
    main()
