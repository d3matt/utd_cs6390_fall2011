#!/usr/bin/env python
import sys, time
import pexpect

print "Spawning echoserv"
p=pexpect.spawn("./echoserv 12543")
print "Spawning Message_test"
m=pexpect.spawn("./Message_test 12543")

print "Showing test results"
print m.read()

if m.isalive():
    m.wait()

if m.exitstatus is None:
    RC=1
else:
    RC=m.exitstatus

print "RC: %d" % (RC)

print "Showing echo serv results"
p.terminate()
print p.read()

sys.exit(RC)
