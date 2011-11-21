#!/usr/bin/env python

import os, os.path, sys, time
import pexpect
from test_common import *


if __name__ == "__main__":
    if not os.path.exists(".log"):
        os.mkdir(".log")
    print "Spawning PCE 0"
    PCE0=spawnwrapper("./PCE 0 local.cfg", ".log/PCE0.log")

    print "Spawning PCE 1"
    PCE1=spawnwrapper("./PCE 1 local.cfg", ".log/PCE1.log")

    print "Spawning PCE 2"
    PCE2=spawnwrapper("./PCE 2 local.cfg", ".log/PCE2.log")

    time.sleep(1)

    print "Spawning router 0-0 (nets 0 1 2 )"
    rt00=spawnwrapper("./router 0 0 local.cfg 99 99 0 1 2", ".log/rt00.log" )
    rt00.expect(">>>")

    print "Spawning router 0-1 (nets 0 1 4)"
    rt01=spawnwrapper("./router 0 1 local.cfg  99 99 2 3 4", ".log/rt01.log" )
    rt01.expect(">>>")

    print "Spawning router 0-2 (nets 4 5 6)"
    rt02=spawnwrapper("./router 0 2 local.cfg  99 99 4 5 6", ".log/rt02.log" )
    rt02.expect(">>>")

    print "Spawning router 0-3 (nets 6 7, PCE 1)"
    rt03=spawnwrapper("./router 0 3 local.cfg  1 0 6 7", ".log/rt03.log" )
    rt03.expect(">>>")

    print "Spawning router 1-0 (nets 10 11 12, PCE 0)"
    rt10=spawnwrapper("./router 1 0 local.cfg  0 3 10 11 12", ".log/rt10.log" )
    rt10.expect(">>>")

    print "Spawning router 1-1 (nets 12 13 14)"
    rt11=spawnwrapper("./router 1 1 local.cfg  99 99 12 13 14", ".log/rt11.log" )
    rt11.expect(">>>")

    print "Spawning router 1-2 (nets 14 15 16)"
    rt12=spawnwrapper("./router 1 2 local.cfg  99 99 14 15 16", ".log/rt12.log" )
    rt12.expect(">>>")

    print "Spawning router 1-3 (nets 16 17 18, PCE 2)"
    rt13=spawnwrapper("./router 1 3 local.cfg  2 0 16 17 18", ".log/rt13.log" )
    rt13.expect(">>>")

    print "Spawning router 2-0 (nets 20 21 22, PCE 0)"
    rt20=spawnwrapper("./router 2 0 local.cfg  1 3 20 21 22", ".log/rt20.log" )
    rt20.expect(">>>")

    print "Spawning router 2-1 (nets 22 23 24)"
    rt21=spawnwrapper("./router 2 1 local.cfg  99 99 22 23 24", ".log/rt21.log" )
    rt21.expect(">>>")

    print "Spawning router 2-2 (nets 24 25 26)"
    rt22=spawnwrapper("./router 2 2 local.cfg  99 99 24 25 26", ".log/rt22.log" )
    rt22.expect(">>>")

    print "Spawning router 2-3 (nets 26 27 28, PCE 2)"
    rt23=spawnwrapper("./router 2 3 local.cfg  99 99 26 27 28", ".log/rt23.log" )
    rt23.expect(">>>")

    print "Trying RT 10 at router 3"
    rt03.sendline("RT 10")
    rt03.expect(">>>")

    RC = 0
    RC |= check_routes(rt03.shell, "0")

    print "Trying RT 18 at router 3"
    rt03.sendline("RT 18")
    rt03.expect(">>>")

    RC = 0
    RC |= check_routes(rt03.shell, "0 1 2 3")

    print "Trying RT 18 at router 0"
    rt00.sendline("RT 18")
    rt00.expect(">>>")

    RC = 0
    RC |= check_routes(rt00.shell, "1 2 3 0 1 2 3")
    
    print "Trying RT 28 at router 0"
    rt00.sendline("RT 28")
    rt00.expect(">>>")

    RC = 0
    RC |= check_routes(rt00.shell, "1 2 3 0 1 2 3 0 1 2 3")

    print "Taking down NET 16 at router 1-3"
    rt13.sendline("DN 16")
    rt13.expect(">>>")

    print "Trying RT 18 at router 0 after link DOWN"
    rt00.sendline("RT 18")
    rt00.expect(">>>")

    RC = 0
    RC |= check_no_route(rt00.shell)

    sys.stdout.write("Shutting down ...")
    sys.stdout.flush()
    #setting the reference to None causes immediate garbage collection (and destructor) in python 2.7
    rt00 = None
    rt01 = None
    rt02 = None
    rt03 = None
    rt10 = None
    rt11 = None
    rt12 = None
    rt13 = None
    rt20 = None
    rt21 = None
    rt22 = None
    rt23 = None
    PCE0 = None
    PCE1 = None
    PCE2 = None
    if RC == 0:
        print( " PASS")
    else:
        print( " FAIL")

    sys.exit(RC)
