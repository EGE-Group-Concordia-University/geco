// This may look like C code, but it is really -*- C++ -*-
// ---------------------------------------------------------------- 
//                                                                  
// Header file for the gecoOPCUAPkg
//
// (c) Rolf Wuthrich
//     2024 Concordia University
//
// author:    Rolf Wuthrich
// email:     rolf.wuthrich@concordia.ca
// version:   v1
//
// This software is copyright under the BSD license
//
// ---------------------------------------------------------------
// history:
// ---------------------------------------------------------------
// Date       Modification                     Author
// ---------------------------------------------------------------
// 17.11.2024 Creation                         R. Wuthrich
// ---------------------------------------------------------------

#ifndef GECOOPCUAPKG_SEEN_
#define GECOOPCUAPKG_SEEN_

#include <tcl8.6/tcl.h>
#include "geco.h"

using namespace std;


// -----------------------------------------------------------------------
//
// Tcl interface
//

extern "C" {

  int Gecoopcuapkg_Init(Tcl_Interp *interp, gecoApp* app, 
		       gecoPkgHandle* pkgHandle);
  int Gecoopcuapkg_Unload(Tcl_Interp *interp);

} /* extern "C" */


// -----------------------------------------------------------------------
//
// TODO : implement here the routines of your package
//


#endif /* GECOOPCUAPKG_SEEN_ */
