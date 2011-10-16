CC=gcc
CXX=g++
LD=g++
CFLAGS=-Wall -g
LDFLAGS=

ifeq ($(HIDE),)
HIDE:= @
endif


default: main serialize echoserv

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
	$(HIDE) $(LD) -o $@ $^

serialize: serialize.o Socket.o
	@ echo LD $@
	$(HIDE) $(LD) -lboost_serialization -o $@ $^

echoserv: echoserv.o
	@ echo LD $@
	$(HIDE) $(LD) -o $@ $^

clean:
	rm -f *.o main serialize echoserv

distclean: clean
	rm -rf .depend
	rm -rf project
dist:
	rm -rf project project.zip
	mkdir -p project
	cp *.cpp *.h README Makefile project
	zip project.zip project/*


-include .depend/*.d
