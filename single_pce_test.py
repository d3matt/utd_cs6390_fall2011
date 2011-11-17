#!/usr/bin/env python

import sys, time
import pexpect


print "Spawning PCE 0"
PCE=pexpect.spawn("./PCE 0 local.cfg")
time.sleep(1)

print "Spawning router 0"
rt0=pexpect.spawn("./router.exe 0 0 local.cfg  99 99 1 2 3")

print "Spawning router 1"
rt1=pexpect.spawn("./router.exe 0 1 local.cfg  99 99 0 1 4 5 6")

rt0.expect(">>>")
rt0.sendline("RT 0")
rt0.expect(">>>")

RC = 0

for line in rt0.before.splitlines():
    if "Result:" in line:
        sp = line.split(":")
        if sp[len(sp) - 1].strip() != "0 1":
            print "FAILED!"
            print "BEFORE: '" + rt0.before + "'"
            print "AFTER:  '" + rt0.after  + "'"
            RC = 1

print "Shutting down"
rt0.terminate
rt1.terminate
PCE.terminate

sys.exit(RC)
