#!/usr/bin/env python
import sys, time
import pexpect

print "Spawning echoserv"
p=pexpect.spawn("./echoserv 12543")
print "Spawning Message_test"
m=pexpect.spawn("./Message_test")

time.sleep(2)

print "Showing test results"
print m.read()

if m.isalive():
    m.wait()

print "RC: %d" % (m.exitstatus)

print "Showing echo serv results"
p.terminate()
print p.read()

sys.exit(m.exitstatus)
