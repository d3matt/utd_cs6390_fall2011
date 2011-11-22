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

    print "Spawning PCE 3"
    PCE3=spawnwrapper("./PCE 3 local.cfg", ".log/PCE3.log")

    time.sleep(1)

    print "Spawning router 0-0 (nets 0 1, AS 3 )"
    rt00=spawnwrapper("./router 0 0 local.cfg 3 0 0 1", ".log/rt00.log" )
    rt00.expect(">>>")

    print "Spawning router 0-1 (nets 0 2, AS 1)"
    rt01=spawnwrapper("./router 0 1 local.cfg 1 0 0 2", ".log/rt01.log" )
    rt01.expect(">>>")

    print "Spawning router 0-2 (nets 1 2, AS 2)"
    rt02=spawnwrapper("./router 0 2 local.cfg 2 0 1 2", ".log/rt02.log" )
    rt02.expect(">>>")

    print "Spawning router 1-0 (nets 10 11, AS 0)"
    rt10=spawnwrapper("./router 1 0 local.cfg  0 1 10 11", ".log/rt10.log" )
    rt10.expect(">>>")

    print "Spawning router 1-1 (nets 10 12, AS 3)"
    rt11=spawnwrapper("./router 1 1 local.cfg  3 1 10 12", ".log/rt11.log" )
    rt11.expect(">>>")

    print "Spawning router 1-2 (nets 11 12 )"
    rt12=spawnwrapper("./router 1 2 local.cfg  99 99 11 12", ".log/rt12.log" )
    rt12.expect(">>>")

    print "Spawning router 2-0 (nets 20 21, AS 0)"
    rt20=spawnwrapper("./router 2 0 local.cfg  0 2 20 21", ".log/rt20.log" )
    rt20.expect(">>>")

    print "Spawning router 2-1 (nets 20 22, AS 3)"
    rt21=spawnwrapper("./router 2 1 local.cfg  3 2 20 22", ".log/rt21.log" )
    rt21.expect(">>>")

    print "Spawning router 2-2 (nets 21 22)"
    rt22=spawnwrapper("./router 2 2 local.cfg  99 99 21 22", ".log/rt22.log" )
    rt22.expect(">>>")

    print "Spawning router 3-0 (nets 30 31, AS 0)"
    rt30=spawnwrapper("./router 3 0 local.cfg  0 0 30 31", ".log/rt30.log" )
    rt30.expect(">>>")

    print "Spawning router 3-1 (nets 30 32, AS 1)"
    rt31=spawnwrapper("./router 3 1 local.cfg 1 1 30 32", ".log/rt21.log" )
    rt31.expect(">>>")

    print "Spawning router 3-2 (nets 31 32, AS 2)"
    rt32=spawnwrapper("./router 3 2 local.cfg  2 1 31 32", ".log/rt22.log" )
    rt32.expect(">>>")


    print "Trying RT 22 at router 0 1"
    rt01.sendline("RT 22")
    rt01.expect(">>>")

    RC = 0
    RC |= check_routes(rt01.shell, "2 0 1")

    print "Trying RT 22 at router 1 2"
    rt12.sendline("RT 22")
    rt12.expect(">>>")

    RC |= check_routes(rt12.shell, "1 1 2 1")
    
    print "Taking down NET 22 at router 2-1"
    rt21.sendline("DN 22")
    rt21.expect(">>>")

    print "Trying RT 22 at router 1 2"
    rt12.sendline("RT 22")
    rt12.expect(">>>")

    RC |= check_routes(rt12.shell, "1 1 2 1 0 2")

    print "Bringing NET 22 at router 2-1 UP"
    rt21.sendline("UP 22")
    rt21.expect(">>>")

    #print "Taking down NET 30 at router 3-1"
    #rt31.sendline("DN 30")
    #rt31.expect(">>>")

    #print "Taking down NET 32 at router 3-1"
    #rt31.sendline("DN 32")
    #rt31.expect(">>>")

    #print "Trying RT 22 at router 1 2"
    #rt12.sendline("RT 22")
    #rt12.expect(">>>")

    #RC |= check_routes(rt12.shell, "0 1 2 0 2")

    sys.stdout.write("Shutting down ...")
    sys.stdout.flush()
    #setting the reference to None causes immediate garbage collection (and destructor) in python 2.7
    rt00 = None
    rt01 = None
    rt02 = None
    rt10 = None
    rt11 = None
    rt12 = None
    rt20 = None
    rt21 = None
    rt22 = None
    rt30 = None
    rt31 = None
    rt32 = None
    PCE0 = None
    PCE1 = None
    PCE2 = None
    PCE3 = None
    if RC == 0:
        print( " PASS")
    else:
        print( " FAIL")

    sys.exit(RC)
