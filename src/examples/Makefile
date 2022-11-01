# This is the geco library examples Makefile
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


# complier with options
CC = g++ -I /usr/include/tcl8.6 -I /usr/local/include/geco1.0

# --------------------------------------------------------------
# Instructions on how to build the geco library examples

clean:
	rm -f *.o 

gecoObjEx.o: gecoObjEx.cc gecoObjEx.h
	$(CC) -c gecoObjEx.cc

gecoAppEx: gecoObjEx.o gecoAppEx.cc
	$(CC) -c gecoAppEx.cc
	$(CC) gecoObjEx.o gecoAppEx.o -o gecoAppEx -lgeco1.0 -ltcl8.6 -ltk8.6 -lm

gecoNetTraff: gecoProcessEx.cc
	$(CC) -c gecoProcessEx.cc
	$(CC) gecoProcessEx.o -o gecoNetTraff -lgeco1.0 -ltcl8.6 -ltk8.6 -lm

gecoGeneratorEx: gecoGeneratorEx.cc
	$(CC) -c gecoGeneratorEx.cc
	$(CC) gecoGeneratorEx.o -o gecoGeneratorEx -lgeco1.0 -ltcl8.6 -ltk8.6 -lm
