CC=gcc
CXX=g++
LD=g++
CFLAGS=-Wall -g -fexceptions -O2 -fno-guess-branch-probability
LDFLAGS=-Wl,-Map,$@.map 

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
endif

BINLIST=echoserv Message_test router

COMMON_OBJECTS=utils.o PCEconfig.o RouterStatus.o usage.o

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

Message_test: RREQ.o RREP.o Message_test.o Socket.o
	@ echo LD $@
	$(HIDE) $(LD) $(LDFLAGS) -o $@ $^ 

router: router.o $(COMMON_OBJECTS)
	@ echo LD $@
	$(HIDE) $(LD) $(LDFLAGS) -o $@ $^ $(BOOSTFLAGS)

clean:
	rm -f *.map *.o $(BINLIST)

distclean: clean
	rm -rf .depend
	rm -rf project

dist:
	rm -rf project project.zip
	mkdir -p project
	cp *.cpp *.h README Makefile project
	zip project.zip project/*


-include .depend/*.d
