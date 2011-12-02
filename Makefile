CC=gcc
CXX=g++
LD=$(CXX)
CFLAGS=-Wall -g -fexceptions -O2 -fno-guess-branch-probability
#uncomment to enable extra debug messages
#CFLAGS+=-DPROJ_DEBUG
LDFLAGS=-lpthread

#used for unit tests
export PYTHONPATH := $(PWD)/pexpect-2.3

#pretty printing of CXX and LD
ifeq ($(HIDE),)
HIDE:= @
endif

BINLIST=echoserv Message_test router PCE

COMMON_OBJECTS=PCEconfig.o Socket.o usage.o Message.o

default: $(BINLIST)

# default compile rule
.cpp.o:
	@ mkdir -p .depend
	@ echo CXX $@
	$(HIDE) $(CXX) $(CFLAGS) -c -o $@ -MMD -MF $(@:%.o=.depend/%.d) $(firstword $^)

#include generated dependency files
-include .depend/*.d

echoserv: echoserv.o
	@ echo LD $@
	$(HIDE) $(LD) $(LDFLAGS) -o $@ $^

Message_test: Message_test.o $(COMMON_OBJECTS)
	@ echo LD $@
	$(HIDE) $(LD) $(LDFLAGS) -o $@ $^

router: router.o $(COMMON_OBJECTS)
	@ echo LD $@
	$(HIDE) $(LD) $(LDFLAGS) -o $@ $^

PCE: PCE.o $(COMMON_OBJECTS)
	@ echo LD $@
	$(HIDE) $(LD) $(LDFLAGS) -o $@ $^

clean:
	rm -f *.map *.o $(BINLIST) core.* *.exe.stackdump *.pyc

distclean: clean
	rm -rf .depend
	rm -rf mjs010200 mjs010200.zip
	rm -rf pexpect-2.3
	rm -rf .log

dist:
	rm -rf mjs010200 mjs010200.zip
	mkdir -p mjs010200
	cp *.cpp *.h *.py README Makefile pexpect-2.3.tar.gz local.cfg mjs010200/
	zip mjs010200.zip mjs010200/*

# from here down is unit test rules
message_unit_test: echoserv Message_test pexpect-2.3/.mkdir
	$(HIDE) mkdir -p .log
	$(HIDE) echo -n "Running message_test.py..."
	$(HIDE) python message_test.py > .log/message_test.log
	$(HIDE) echo "PASS"

single_pce_test: PCE router pexpect-2.3/.mkdir
	$(HIDE) mkdir -p .log
	$(HIDE) echo -n "Running single_pce_test.py..."
	$(HIDE) python single_pce_test.py > .log/single_pce_test.log
	$(HIDE) echo "PASS"

multi_pce_test: single_pce_test
	$(HIDE) mkdir -p .log
	$(HIDE) echo -n "Running multi_pce_test.py..."
	$(HIDE) python multi_pce_test.py > .log/multi_pce_test.log
	$(HIDE) echo "PASS"

star_pce_test: multi_pce_test
	$(HIDE) mkdir -p .log
	$(HIDE) echo -n "Running star_pce_test.py..."
	$(HIDE) python star_pce_test.py > .log/star_pce_test.log
	$(HIDE) echo "PASS"

pexpect-2.3/.mkdir: pexpect-2.3.tar.gz
	tar -xzf pexpect-2.3.tar.gz
	touch pexpect-2.3/.mkdir

tests: message_unit_test single_pce_test multi_pce_test star_pce_test
