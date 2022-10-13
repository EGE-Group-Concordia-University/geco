// This may look like C code, but it is really -*- C++ -*-
// $Id: ECIOBoard.h 37 2015-01-09 12:35:29Z wuthrich $
// ---------------------------------------------------------------- 
//                                                                  
// Header file for the gecoPiPkg
//
// (c) Rolf Wuthrich
//     2016 Concordia University
//
// author:    Rolf Wuthrich
// email:     rolf.wuthrich@concordia.ca
// version:   v1 ($Revision: 37 $)
//
// This software is copyright under the BSD license
//
// ---------------------------------------------------------------
// history:
// ---------------------------------------------------------------
// Date       Modification                     Author
// ---------------------------------------------------------------
// 07.02.2016 Creation                         R. Wuthrich
// ---------------------------------------------------------------

#ifndef GECOPIPKG_SEEN_
#define GECOPIPKG_SEEN_

#include <tcl.h>
#include "geco.h"

using namespace std;


// -----------------------------------------------------------------------
//
// Tcl interface
//

extern "C" {

  int Gecopipkg_Init(Tcl_Interp *interp, gecoApp* app, 
		     gecoPkgHandle* pkgHandle);
  int Gecopipkg_Unload(Tcl_Interp *interp);

} /* extern "C" */


#endif /* GECOPIPKG_SEEN_ */
