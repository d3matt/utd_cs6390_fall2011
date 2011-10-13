CXX=g++
LD=g++
CFLAGS=-Wall -g
LDFLAGS=

ifeq ($(HIDE),)
HIDE:= @
endif


default: main

.cpp.o:
	@ mkdir -p .depend
	@ echo CXX $@
	$(HIDE) $(CXX) $(CFLAGS) -c -o $@ -MMD -MF $(@:%.o=.depend/%.d) $(firstword $^)


main: main.o
	@ echo LD $@
	$(HIDE) $(LD) -o $@ $^

clean:
	rm -f *.o client server

dist:
	rm -rf mjs010200 mjs010200.zip
	mkdir -p mjs010200
	cp *.cpp *.h README Makefile mjs010200
	zip mjs010200.zip mjs010200/*


-include .depend/*.d
