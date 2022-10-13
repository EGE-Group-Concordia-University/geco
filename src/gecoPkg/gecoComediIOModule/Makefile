# $Id: Makefile 15 2014-01-03 18:49:04Z wuthrich $
#
# This is the gecoComediIOModule Makefile
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
# 14.11.2015 Creation                         R. Wuthrich
#----------------------------------------------------------

# module version
modVer = 1.0

# taget to be built
TARGET = libcomediIOModule$(modVer).so

# complier with options
CC = gcc -fPIC -I /usr/include/tcl8.5 -I /usr/local/include/geco1.0

# --------------------------------------------------------------
# List of all object files to be included into the geco library
OBJS  += gecoComediIOModule.o

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
	gcc $(OBJS) -shared -o $(TARGET) -lc -lgeco1.0 -ltcl8.5 -lcomedi

gecoComediIOModule.o: gecoComediIOModule.cc gecoComediIOModule.h
	$(CC) -c gecoComediIOModule.cc
