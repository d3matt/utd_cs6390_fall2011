CXX=g++
LD=g++
CFLAGS=-Wall -g
LDFLAGS=

ifeq ($(HIDE),)
HIDE:= @
endif


default: main serialize

.cpp.o:
	@ mkdir -p .depend
	@ echo CXX $@
	$(HIDE) $(CXX) $(CFLAGS) -c -o $@ -MMD -MF $(@:%.o=.depend/%.d) $(firstword $^)


main: main.o
	@ echo LD $@
	$(HIDE) $(LD) -o $@ $^

serialize: serialize.o
	@ echo LD $@
	$(HIDE) $(LD) -lboost_serialization -o $@ $^

clean:
	rm -f *.o main serialize file.dat

distclean: clean
	rm -rf .depend
dist:
	rm -rf project project.zip
	mkdir -p project
	cp *.cpp *.h README Makefile project
	zip project.zip project/*


-include .depend/*.d
