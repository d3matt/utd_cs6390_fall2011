#!/usr/bin/env python
import subprocess
import time
import pexpect

print "Spawning echoserv"
p=subprocess.Popen("./echoserv 12543", shell=True, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
print "Spawning Message_test"
m=subprocess.Popen("./Message_test", shell=True, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)

time.sleep(2)

print "Showing test results"
print p.stdout.read()
print m.stdout.read()
p.kill()
