// This may look like C code, but it is really -*- C++ -*-
// $Id: ECHelp.h 39 2015-01-09 13:09:42Z wuthrich $
// ---------------------------------------------------------------- 
//                                                                  
// Header file for geCo help function
//
// (c) Rolf Wuthrich
//     2015-2016 Concordia University
//
// author:  Rolf Wuthrich
// email:   rolf.wuthrich@concordia.ca
// version: v1 ($Revision: 39 $)
//
// This software is copyright under the BSD license
//
// ---------------------------------------------------------------
// history:
// ---------------------------------------------------------------
// Date       Modification                     Author
// ---------------------------------------------------------------
// 09.11.2015 Creation                         R. Wuthrich
// ---------------------------------------------------------------

#ifndef gecoHelp_SEEN_
#define gecoHelp_SEEN_

#include <tcl8.6/tcl.h>

using namespace std;

// -----------------------------------------------------------------------
//
// class gecoHelp : help function
//

void gecoHelp(Tcl_Interp* interp, 
	      const char* cmdName, 
	      const char* cmdInfo, 
	      const char** cmds, 
	      const char** help);

#endif /* gecoHelp_SEEN_ */
