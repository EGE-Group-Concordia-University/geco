# $Id: Makefile 15 2014-01-03 18:49:04Z wuthrich $
#
# This is the gecoMtConnectPkg Makefile
#
# (c) Rolf Wuthrich
#     2022 Concordia University
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
# 27.04.2022 Creation                         R. Wuthrich
#----------------------------------------------------------

# module version
modVer = 1.0

# taget to be built
TARGET = libgecoMtcPkg$(modVer).so

# complier with options
CC = gcc -fPIC -I /usr/include/tcl8.6 -I /usr/local/include/geco1.0

# --------------------------------------------------------------
# List of all object files to be included into the geco library
OBJS  += gecoMtcPkg.o
OBJS  += gecoMTCBaseAdapter.o
OBJS  += gecoMTCAdapter.o
OBJS  += gecoMTCGrblAdapter.o

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
	gcc $(OBJS) -shared -o $(TARGET) -lc -ltcl8.6 -lgeco1.0

gecoMtcPkg.o: gecoMtcPkg.cc gecoMtcPkg.h
	$(CC) -c gecoMtcPkg.cc

gecoMTCBaseAdapter.o: gecoMTCBaseAdapter.cc gecoMTCBaseAdapter.h
	$(CC) -c gecoMTCBaseAdapter.cc

gecoMTCAdapter.o: gecoMTCAdapter.cc gecoMTCAdapter.h gecoMTCBaseAdapter.h
	$(CC) -c gecoMTCAdapter.cc

gecoMTCGrblAdapter.o: gecoMTCGrblAdapter.cc gecoMTCGrblAdapter.h gecoMTCBaseAdapter.h
	$(CC) -c gecoMTCGrblAdapter.cc
