CC=gcc
CXX=g++
LD=g++
CFLAGS=-Wall -g
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

BINLIST=main serialize echoserv RREQ_test router

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

serialize: serialize.o Socket.o
	@ echo LD $@
	$(HIDE) $(LD) $(LDFLAGS) -o $@ $^ $(BOOSTFLAGS)

echoserv: echoserv.o
	@ echo LD $@
	$(HIDE) $(LD) $(LDFLAGS) -o $@ $^

RREQ_test: RREQ.o RREQ_test.o Socket.o
	@ echo LD $@
	$(HIDE) $(LD) $(LDFLAGS) -o $@ $^ $(BOOSTFLAGS)

router: router.o utils.o PCEconfig.o
	@ echo LD $@
	$(HIDE) $(LD) $(LDFLAGS) -o $@ $^

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
