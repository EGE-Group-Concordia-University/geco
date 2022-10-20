# This Makefile builds the examples
#
# GECO library must have been built with make install previousley
#
# (c) Rolf Wuthrich
#     2022 Concordia University
#
# author:    Rolf Wuthrich
# email:     rolf.wuthrich@concordia.ca
#
# This software is copyright under the BSD license
#
# ---------------------------------------------------------------
# history:
# ---------------------------------------------------------------
# Date       Modification                     Author
#----------------------------------------------------------------
# 20.10.2022 Creation                         R. Wuthrich
#----------------------------------------------------------------

# taget to be built
TARGET = demoGecoApp

# complier with options
CC = g++ -I /usr/include/tcl8.6 -I /usr/local/include/geco1.0

#----------------------------------------------------------------
# List of all object files to be included
OBJS  += gecoAppEx.o
OBJS  += gecoObjEx.o

#----------------------------------------------------------------
# Instructions on how to build the demo app

all: $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) -lgeco1.0 -ltcl8.6 -ltk8.6 -lm

clean:
	rm -f *.o 

gecoAppEx.o: gecoAppEx.cc gecoObjEx.h
	$(CC) -c gecoAppEx.cc
    
gecoObjEx.o: gecoObjEx.cc gecoObjEx.h
	$(CC) -c gecoObjEx.cc
