# $Id: Makefile 15 2014-01-03 18:49:04Z wuthrich $
#
# This is the gecoDemoPkg Makefile
#
# (c) Rolf Wuthrich
#     2016 Concordia University
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
# 05.02.2016 Creation                         R. Wuthrich
#----------------------------------------------------------

# module version
modVer = 1.0

# taget to be built
TARGET = libgecoPiPkg$(modVer).so

# complier with options
CC = gcc -fPIC -I /usr/include/tcl8.6 -I /usr/local/include/geco1.0

# --------------------------------------------------------------
# List of all object files to be included into the geco library
OBJS  += gecoPiPkg.o
OBJS  += gecoPi.o
OBJS  += gecoPiGPIO.o
OBJS  += gecoDS18B20.o

# --------------------------------------------------------------
# Instructions on how to build the geco library 

all: $(TARGET)

clean:
	rm -f *.o 
	rm -f *.so

install: $(TARGET)
	cp $(TARGET) /usr/local/share/geco/pkg/
	ldconfig

$(TARGET): $(OBJS)
	gcc $(OBJS) -shared -o $(TARGET) -lc -lgeco1.0 -ltcl8.6

gecoPiPkg.o: gecoPiPkg.cc gecoPiPkg.h
	$(CC) -c gecoPiPkg.cc

gecoPi.o: gecoPi.cc gecoPi.h gecoPiPkg.h
	$(CC) -c gecoPi.cc

gecoPiGPIO.o: gecoPiGPIO.cc gecoPiGPIO.h gecoPiPkg.h
	$(CC) -c gecoPiGPIO.cc
	
gecoDS18B20.o: gecoDS18B20.cc gecoDS18B20.h gecoPiPkg.h
	$(CC) -c gecoDS18B20.cc

