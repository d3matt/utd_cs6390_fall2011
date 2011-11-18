#!/usr/bin/env python

import os, os.path, sys, time
import pexpect
from test_common import *


if __name__ == "__main__":
    if not os.path.exists(".log"):
        os.mkdir(".log")
    print "Spawning PCE 0"
    PCE=spawnwrapper("./PCE 0 local.cfg", ".log/PCE.log")
    time.sleep(1)

    print "Spawning router 0 (nets 1 2 3)"
    rt0=spawnwrapper("./router 0 0 local.cfg  99 99 1 2 3", ".log/rt0.log" )
    rt0.expect(">>>")

    print "Spawning router 1 (nets 0 1 4)"
    rt1=spawnwrapper("./router 0 1 local.cfg  99 99 0 1 4", ".log/rt1.log" )
    rt1.expect(">>>")

    print "Spawning router 2 (nets 4 5 6)"
    rt2=spawnwrapper("./router 0 2 local.cfg  99 99 4 5 6", ".log/rt2.log" )
    rt2.expect(">>>")

    print "Spawning router 3 (nets 6 7)"
    rt3=spawnwrapper("./router 0 3 local.cfg  99 99 6 7", ".log/rt3.log" )
    rt3.expect(">>>")

    print "Spawning router 4 (nets 0 7)"
    rt4=spawnwrapper("./router 0 4 local.cfg  99 99 1 7", ".log/rt4.log" )
    rt4.expect(">>>")

    print "Trying RT 0 at router 0"
    rt0.sendline("RT 0")
    rt0.expect(">>>")

    RC = 0
    RC |= check_routes(rt0.shell, "0 1")

    print "Trying RT 7 at router 0"
    rt0.sendline("RT 7")
    rt0.expect(">>>")

    RC |= check_routes(rt0.shell, "0 4")

    print "Bringing down interface 1 on router 4"
    rt4.sendline("DN 1")
    rt4.expect(">>>")

    print "Trying RT 7 at router 0 (again)"
    rt0.sendline("RT 7")
    rt0.expect(">>>")

    RC |= check_routes(rt0.shell, "0 1 2 3")

    sys.stdout.write("Shutting down ...")
    sys.stdout.flush()
    #setting the reference to None causes immediate garbage collection (and destructor) in python 2.7
    rt0 = None
    rt1 = None
    rt2 = None
    rt3 = None
    rt4 = None
    PCE = None
    print " done"

    sys.exit(RC)
