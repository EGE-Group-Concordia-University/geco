// This may look like C code, but it is really -*- C++ -*-
// $Id: ECIOBoard.h 37 2015-01-09 12:35:29Z wuthrich $
// ---------------------------------------------------------------- 
//                                                                  
// Header file for the gecoMtcPkg
//
// (c) Rolf Wuthrich
//     2022 Concordia University
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
// 27.04.2022 Creation                         R. Wuthrich
// ---------------------------------------------------------------

#ifndef GECOMTCPKG_SEEN_
#define GECOMTCPKG_SEEN_

#include <tcl.h>
#include "geco.h"

using namespace std;


// -----------------------------------------------------------------------
//
// Tcl interface
//

extern "C" {

  int Gecomtcpkg_Init(Tcl_Interp *interp, gecoApp* app, 
		      gecoPkgHandle* pkgHandle);
  int Gecomtcpkg_Unload(Tcl_Interp *interp);

} /* extern "C" */


// -----------------------------------------------------------------------
//
// TODO : implement here the routines of your package
//


#endif /* GECODEMOPKG_SEEN_ */
