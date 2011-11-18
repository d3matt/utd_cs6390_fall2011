"""function common to PCE unit tests"""

import pexpect

def check_routes(pspawn, exp):
    """parses the pexpect.spawn object looking for 'Result:'
checks the result against the expected value"""
    retval = 0
    result_found = False
    for line in pspawn.before.splitlines():
        if "Result:" in line:
            result_found = True
            spl = line.split(":")
            if spl[len(spl) - 1].strip() != exp:
                print "FAILED!"
                print "BEFORE: '" + pspawn.before + "'"
                print "AFTER:  '" + pspawn.after  + "'"
                retval = 1
            else:
                print "SUCCESS!   " + line.strip()
            break
    if not result_found:
        print "'Result:' not found!"
        print "BEFORE: '" + pspawn.before + "'"
        print "AFTER:  '" + pspawn.after  + "'"
    return retval

class spawnwrapper(object):
    """convenience class to get good logs of interactions"""
    def __init__(self, script, logfile):
        self.shell = pexpect.spawn(script, logfile=open(logfile, 'w') )
    def __del__(self):
        self.shell.terminate()
        self.shell.read()
    def sendline(self, line):
        return self.shell.sendline(line)
    def expect(self, exp):
        return self.shell.expect(exp)

