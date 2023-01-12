# 
# Copyright (c) 2013 No Face Press, LLC
# License http://opensource.org/licenses/mit-license.php MIT License
#

#This makefile is used to test the other Makefiles


PROG = WebTaskManager
SRC = server.c

TOP = ./include/libhttp
CIVETWEB_LIB = libcivetweb.a

CFLAGS = -I$(TOP)/include $(COPT)
LIBS = -lpthread

include $(TOP)/resources/Makefile.in-os

ifeq ($(TARGET_OS),LINUX) 
	LIBS += -ldl
endif

all: $(PROG)
	make clean

$(PROG): $(CIVETWEB_LIB) $(SRC)
	$(CC) -o $@ $(CFLAGS) $(LDFLAGS) $(SRC) $(CIVETWEB_LIB) $(LIBS)
	gcc -o runCommand runCommand.c
	g++ -o limitCpuLoad limitCpuLoad.cpp
	gcc -o updateProcess updateProcess.c
	make run

$(CIVETWEB_LIB):
	$(MAKE) -C $(TOP) clean lib
	cp $(TOP)/$(CIVETWEB_LIB) .
	
run:
	./WebTaskManager

help:
	xdg-open ./docs/documentation.html
	clear

help-de:
	xdg-open ./docs/documentation.de.html
	clear

clean:
	rm -f $(CIVETWEB_LIB) $(PROG)
	rm -rf WebTaskManager
	rm -rf libcivetweb.a
	rm -rf runCommand
	rm -rf limitCpuLoad
	rm -rf updateProcess

.PHONY: all clean

