# $Id: Makefile 15 2014-01-03 18:49:04Z wuthrich $
#
# This is the geCo library Makefile
#
# (c) Rolf Wuthrich
#     2015-2016 Concordia University
#
# author:    Rolf Wuthrich
# email:     rolf.wuthrich@concordia.ca
# $Revision: 15 $
#
# This software is copyright under the BSD license
#
# ---------------------------------------------------------
# history:
# ---------------------------------------------------------
# Date       Modification                     Author
#----------------------------------------------------------
# 12.10.2015 Creation                         R. Wuthrich
#----------------------------------------------------------

# geco library version
libVer = 1.0

# taget to be built
TARGET = libgeco$(libVer).so

# complier with options
CC = gcc -fPIC -I /usr/include/tcl8.6

# --------------------------------------------------------------
# List of all object files to be included into the geco library
OBJS  += gecoApp.o 
OBJS  += gecoHelp.o 
OBJS  += gecoObj.o
OBJS  += gecoEvent.o
OBJS  += gecoProcess.o
OBJS  += gecoTrigger.o 
OBJS  += gecoIOModule.o
OBJS  += gecoPkgHandle.o
OBJS  += gecoTcpServer.o
OBJS  += gecoIOSocket.o 
OBJS  += gecoIOTcp.o 
OBJS  += gecoIO.o 
OBJS  += gecoUProc.o 
OBJS  += gecoGraph.o 
OBJS  += gecoClock.o 
OBJS  += gecoFileStream.o
OBJS  += gecoMemStream.o
OBJS  += gecoSensor.o
OBJS  += gecoGenerator.o
OBJS  += gecoTriangle.o
OBJS  += gecoSawtooth.o
OBJS  += gecoStep.o
OBJS  += gecoPulse.o
OBJS  += gecoEnd.o

# --------------------------------------------------------------
# Instructions on how to build the geco library 

all: $(TARGET)

clean:
	rm -f *.o 
	rm -f *.so

install: $(TARGET)
	cp $(TARGET) /usr/local/lib/
	chmod 0755 /usr/local/lib/$(TARGET)
	tclsh geco_install.tcl
	cp *.h /usr/local/include/geco$(libVer)
	ldconfig
	
documentation: 
	doxygen Doxyfile
	cp -r html/* /var/www/html/ege/geco/

$(TARGET): $(OBJS)
	gcc $(OBJS) -shared -o $(TARGET) -lc

gecoApp.o: gecoApp.cc gecoApp.h gecoEvent.h gecoProcess.h gecoIO.h
	$(CC) -c gecoApp.cc

gecoHelp.o: gecoHelp.cc gecoHelp.h
	$(CC) -c gecoHelp.cc

gecoObj.o: gecoObj.cc gecoObj.h
	$(CC) -c gecoObj.cc

gecoEvent.o: gecoEvent.cc gecoEvent.h
	$(CC) -c gecoEvent.cc

gecoProcess.o: gecoProcess.cc gecoProcess.h gecoApp.h gecoEvent.h
	$(CC) -c gecoProcess.cc

gecoClock.o: gecoClock.cc gecoClock.h gecoEvent.h
	$(CC) -c gecoClock.cc

gecoIO.o: gecoIO.cc gecoIO.h gecoIOModule.h gecoEvent.h
	$(CC) -c gecoIO.cc

gecoIOModule.o: gecoIOModule.cc gecoIOModule.h gecoObj.h gecoEvent.h
	$(CC) -c gecoIOModule.cc
	
gecoIOSocket.o: gecoIOSocket.cc gecoIOSocket.h gecoApp.h gecoIO.h gecoHelp.h
	$(CC) -c gecoIOSocket.cc
	
gecoIOTcp.o: gecoIOTcp.cc gecoIOTcp.h gecoApp.h gecoIO.h gecoHelp.h
	$(CC) -c gecoIOTcp.cc
	
gecoTcpServer.o: gecoTcpServer.cc gecoTcpServer.h
	$(CC) -c gecoTcpServer.cc

gecoPkgHandle.o: gecoPkgHandle.cc gecoPkgHandle.h gecoApp.h gecoObj.h
	$(CC) -c gecoPkgHandle.cc

gecoTrigger.o: gecoTrigger.cc gecoTrigger.h gecoProcess.h gecoEvent.h
	$(CC) -c gecoTrigger.cc

gecoUProc.o: gecoUProc.cc gecoUProc.h gecoProcess.h 
	$(CC) -c gecoUProc.cc

gecoGraph.o: gecoGraph.cc gecoGraph.h gecoProcess.h 
	$(CC) -c gecoGraph.cc

gecoFileStream.o: gecoFileStream.cc gecoFileStream.h gecoProcess.h 
	$(CC) -c gecoFileStream.cc

gecoMemStream.o: gecoMemStream.cc gecoMemStream.h gecoProcess.h 
	$(CC) -c gecoMemStream.cc
	
gecoSensor.o: gecoSensor.cc gecoSensor.h gecoProcess.h gecoEvent.h
	$(CC) -c gecoSensor.cc	

gecoGenerator.o: gecoGenerator.cc gecoGenerator.h gecoProcess.h gecoEvent.h
	$(CC) -c gecoGenerator.cc

gecoTriangle.o: gecoTriangle.cc gecoTriangle.h gecoGenerator.h
	$(CC) -c gecoTriangle.cc

gecoSawtooth.o: gecoSawtooth.cc gecoSawtooth.h gecoGenerator.h
	$(CC) -c gecoSawtooth.cc

gecoStep.o: gecoStep.cc gecoStep.h gecoGenerator.h
	$(CC) -c gecoStep.cc

gecoPulse.o: gecoPulse.cc gecoPulse.h gecoGenerator.h
	$(CC) -c gecoPulse.cc

gecoEnd.o: gecoEnd.cc gecoEnd.h 
	$(CC) -c gecoEnd.cc

