# Makefile for DLX simulator
#
#

MACHINE = -DDEBUG #-DSOLARIS
#CFLAGS = $(MACHINE) -O3 -g -DDEBUG
#CXXFLAGS = $(MACHINE) -O3 -g -DDEBUG
CFLAGS = $(MACHINE) -O3 -g
CXXFLAGS = $(MACHINE) -O3 -g

INCS = $(wildcard *.h)
SRCS = $(wildcard *.cc)
OBJS = $(addsuffix .o, $(basename $(wildcard *.cc)))

dlxsim: $(OBJS) Makefile.depend
	g++ -O3 -g $(OBJS) -o dlxsim

Makefile.depend: depend

depend: $(SRCS) $(INCS)
	g++ -MM $(SRCS) > Makefile.depend

clean:
	/bin/rm dlxsim $(OBJS) Makefile.depend

include Makefile.depend
