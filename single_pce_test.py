#!/usr/bin/env python

import sys, time
import pexpect


print "Spawning PCE 0"
PCE=pexpect.spawn("./PCE 0 local.cfg")
time.sleep(1)

print "Spawning router 0 (nets 1 2 3)"
rt0=pexpect.spawn("./router 0 0 local.cfg  99 99 1 2 3")

print "Spawning router 1 (nets 0 1 4)"
rt1=pexpect.spawn("./router 0 1 local.cfg  99 99 0 1 4")

print "Spawning router 2 (nets 4 5 6)"
rt2=pexpect.spawn("./router 0 2 local.cfg  99 99 4 5 6")

print "Trying RT 0 at router 0"
rt0.expect(">>>")
rt0.sendline("RT 0")
rt0.expect(">>>")

RC = 0

resultFound=False
for line in rt0.before.splitlines():
    if "Result:" in line:
        resultFound=True
        sp = line.split(":")
        if sp[len(sp) - 1].strip() != "0 1":
            print "FAILED!"
            print "BEFORE: '" + rt0.before + "'"
            print "AFTER:  '" + rt0.after  + "'"
            RC = 1
        else:
            print "SUCCESS!"
        break
if not resultFound:
    print "Result: not found!"
    print "BEFORE: '" + rt0.before + "'"
    print "AFTER:  '" + rt0.after  + "'"

print "Trying RT 6 at router 0"
rt0.sendline("RT 6")
rt0.expect(">>>")

resultFound=False
for line in rt0.before.splitlines():
    if "Result:" in line:
        resultFound=True
        sp = line.split(":")
        if sp[len(sp) - 1].strip() != "0 1 2":
            print "Routes wrong!"
            print "BEFORE: '" + rt0.before + "'"
            print "AFTER:  '" + rt0.after  + "'"
            RC = 1
        else:
            print "SUCCESS!"
        break
if not resultFound:
    print "Result: not found!"
    print "BEFORE: '" + rt0.before + "'"
    print "AFTER:  '" + rt0.after  + "'"

print "Shutting down"
rt0.terminate
rt1.terminate
rt2.terminate
PCE.terminate

sys.exit(RC)
