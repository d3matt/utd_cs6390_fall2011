#!/usr/bin/env python

import os, os.path, sys, time
import pexpect
from test_common import *


if __name__ == "__main__":
    if not os.path.exists(".log"):
        os.mkdir(".log")

    AS = {}
    r={}
    for i in range(0, 10):
        print "Spawning PCE %d" % (i)
        AS[i] = spawnwrapper("./PCE %d local.cfg" % (i), ".log/PCE%d.log" % (i) )
        r[i]={}
    time.sleep(1)


    print "Spawning router 0_0 (nets 0, AS1 0)"
    r[0][0] = spawnwrapper("./router 0 0 local.cfg  0 0 0", ".log/rt0_0.log" )
    r[0][0].expect(">>>")

    print "Spawning router 0_1 (nets 0, AS9 1)"
    r[0][1] = spawnwrapper("./router 0 1 local.cfg  9 1 0", ".log/rt0_1.log" )
    r[0][1].expect(">>>")

    print "Spawning router 1_0 (nets 1 2, AS0 1)"
    r[1][0] = spawnwrapper("./router 1 0 local.cfg  0 1 1 2", ".log/rt1_0.log" )
    r[1][0].expect(">>>")

    print "Spawning router 9_0 (nets 32, AS8 1)"
    r[9][0] = spawnwrapper("./router 9 0 local.cfg  8 1 32", ".log/rt9_0.log" )
    r[9][0].expect(">>>")
    print "Spawning router 9_1 (nets 32, AS0 0)"
    r[9][1] = spawnwrapper("./router 9 1 local.cfg  0 0 32", ".log/rt9_1.log" )
    r[9][1].expect(">>>")

    RC = 0
#    print "Spawning router 2 (nets 4 5 6)"
#    rt2=spawnwrapper("./router 0 2 local.cfg  99 99 4 5 6", ".log/rt2.log" )
#    rt2.expect(">>>")

#    print "Spawning router 3 (nets 6 7)"
#    rt3=spawnwrapper("./router 0 3 local.cfg  99 99 6 7", ".log/rt3.log" )
#    rt3.expect(">>>")

#    print "Spawning router 4 (nets 0 7)"
#    rt4=spawnwrapper("./router 0 4 local.cfg  99 99 1 7", ".log/rt4.log" )
#    rt4.expect(">>>")

#    print "Trying RT 0 at router 0"
#    rt0.sendline("RT 0")
#    rt0.expect(">>>")

#    RC |= check_routes(rt0.shell, "0 1")

#    print "Trying RT 7 at router 0"
#    rt0.sendline("RT 7")
#    rt0.expect(">>>")

#    RC |= check_routes(rt0.shell, "0 4")

#    print "Bringing down interface 1 on router 4"
#    rt4.sendline("DN 1")
#    rt4.expect(">>>")

#    print "Trying RT 7 at router 0 (again)"
#    rt0.sendline("RT 7")
#    rt0.expect(">>>")

#    RC |= check_routes(rt0.shell, "0 1 2 3")

    sys.stdout.write("Shutting down ...")
    sys.stdout.flush()
    #setting the reference to None causes immediate garbage collection (and destructor) in python 2.7
    r = None
    time.sleep(1)
    AS = None
    if RC == 0:
        print " PASS"
    else:
        print " FAIL"

    sys.exit(RC)
