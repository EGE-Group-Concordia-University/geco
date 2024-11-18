# This is the gecoOPCUAPkg Makefile
#
# (c) Rolf Wuthrich
#     2024 Concordia University
#
# author:    Rolf Wuthrich
# email:     rolf.wuthrich@concordia.ca
#
# This software is copyright under the BSD license

# module version
modVer = 1.0

# Target to be built
TARGET = libgecoOPCUAPkg$(modVer).so

# Compiler with options
CC = gcc -fPIC -I /usr/include/tcl8.6 -I /usr/local/include/geco1.0

# --------------------------------------------------------------
# List of all object files to be included into the geco library
OBJS  += gecoOPCUAPkg.o gecoOPCUAServer.o

# --------------------------------------------------------------
# Instructions on how to build the geco OPC UA package 

# Default to STATIC linking of open62541 library
OPEN62541_LINK_TYPE ?= STATIC

# Define linking flags based on the type
ifeq ($(OPEN62541_LINK_TYPE), SHARED)
    LIBS = -lc -lgeco1.0 -ltcl8.6
else ifeq ($(OPEN62541_LINK_TYPE), STATIC)
    LIBS = -lc -lgeco1.0 -ltcl8.6 -L./ -lopen62541
else
    $(error Unknown OPEN62541 '$(OPEN62541_LINK_TYPE)', should be 'SHARED' or 'STATIC')
endif

all: $(TARGET)

clean:
	rm -f *.o 
	rm -f *.so

install: $(TARGET)
	cp $(TARGET) /usr/local/share/geco/pkg/
	ldconfig

$(TARGET): $(OBJS)
	gcc $(OBJS) -shared -o $(TARGET) $(LIBS)

gecoOPCUAPkg.o: gecoOPCUAPkg.cc gecoOPCUAPkg.h
	$(CC) -c gecoOPCUAPkg.cc

gecoOPCUAServer.o: gecoOPCUAServer.cc gecoOPCUAServer.h
	$(CC) -c gecoOPCUAServer.cc

.PHONY: clean install
