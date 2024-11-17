# This is the gecoDemoPkg Makefile
#
# (c) Rolf Wuthrich
#     2016-2024 Concordia University
#
# author:    Rolf Wuthrich
# email:     rolf.wuthrich@concordia.ca
#
# This software is copyright under the BSD license
#
# ---------------------------------------------------------
# history:
# ---------------------------------------------------------
# Date       Modification                     Author
#----------------------------------------------------------
# 05.02.2016 Creation                         R. Wuthrich
# 17.11.2024 Fix tcl.h and tk.h imports       R. Wuthrich
#----------------------------------------------------------

# module version
modVer = 1.0

# taget to be built
TARGET = libgecoDemoPkg$(modVer).so

# complier with options
CC = gcc -fPIC -I /usr/include/tcl8.6 -I /usr/local/include/geco1.0

# --------------------------------------------------------------
# List of all object files to be included into the geco library
OBJS  += gecoDemoPkg.o

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

gecoDemoPkg.o: gecoDemoPkg.cc gecoDemoPkg.h
	$(CC) -c gecoDemoPkg.cc
