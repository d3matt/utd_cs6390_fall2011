CC=gcc
CXX=g++
LD=$(CXX)
CFLAGS=-Wall -g -fexceptions -O2 -fno-guess-branch-probability
#uncomment to enable debug messages
#CFLAGS+=-DPROJ_DEBUG
LDFLAGS=-lpthread

ifeq ($(HIDE),)
HIDE:= @
endif

ifeq ($(shell uname -m),x86_64)
LIBDIR:=lib64
else
LIBDIR:=lib
endif

BOOSTFLAGS=
ifeq ($(shell uname -s),Linux)
BOOSTFLAGS=-Wl,--start-group /usr/$(LIBDIR)/libboost_serialization.a -Wl,--end-group
endif

ifeq ($(shell uname -s),CYGWIN_NT-5.1)
BOOSTFLAGS=--start-group /usr/$(LIBDIR)/libboost_*.a --end-group
CFLAGS += -D__CYGWIN__
endif

BINLIST=echoserv Message_test router PCE

COMMON_OBJECTS=utils.o PCEconfig.o Socket.o usage.o Message.o

default: $(BINLIST)

.cpp.o:
	@ mkdir -p .depend
	@ echo CXX $@
	$(HIDE) $(CXX) $(CFLAGS) -c -o $@ -MMD -MF $(@:%.o=.depend/%.d) $(firstword $^)

.c.o:
	@ mkdir -p .depend
	@ echo CC $@
	$(HIDE) $(CC) $(CFLAGS) -c -o $@ -MMD -MF $(@:%.o=.depend/%.d) $(firstword $^)


main: main.o
	@ echo LD $@
	$(HIDE) $(LD) $(LDFLAGS) -o $@ $^

echoserv: echoserv.o
	@ echo LD $@
	$(HIDE) $(LD) $(LDFLAGS) -o $@ $^

Message_test: Message_test.o $(COMMON_OBJECTS)
	@ echo LD $@
	$(HIDE) $(LD) $(LDFLAGS) -o $@ $^ $(BOOSTFLAGS)

router: router.o $(COMMON_OBJECTS)
	@ echo LD $@
	$(HIDE) $(LD) $(LDFLAGS) -o $@ $^ $(BOOSTFLAGS)

PCE: PCE.o $(COMMON_OBJECTS)
	@ echo LD $@
	$(HIDE) $(LD) $(LDFLAGS) -o $@ $^ $(BOOSTFLAGS)

clean:
	rm -f *.map *.o $(BINLIST) core.*

distclean: clean
	rm -rf .depend
	rm -rf project

dist:
	rm -rf project project.zip
	mkdir -p project
	cp *.cpp *.h README Makefile project
	zip project.zip project/*

message_unit_test: echoserv Message_test
	python message_test.py

single_pce_test: PCE router
	python single_pce_test.py

tests: message_unit_test single_pce_test


-include .depend/*.d
