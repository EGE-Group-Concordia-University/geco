// This may look like C code, but it is really -*- C++ -*-
// $Id: ECIOBoard.h 37 2015-01-09 12:35:29Z wuthrich $
// ---------------------------------------------------------------- 
//                                                                  
// Header file for the gecoDemoPkg
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
// 05.02.2016 Creation                         R. Wuthrich
// ---------------------------------------------------------------

#ifndef GECODEMOPKG_SEEN_
#define GECODEMOPKG_SEEN_

#include <tcl.h>
#include "geco.h"

using namespace std;


// -----------------------------------------------------------------------
//
// Tcl interface
//

extern "C" {

  int Gecodemopkg_Init(Tcl_Interp *interp, gecoApp* app, 
		       gecoPkgHandle* pkgHandle);
  int Gecodemopkg_Unload(Tcl_Interp *interp);

} /* extern "C" */


// -----------------------------------------------------------------------
//
// TODO : implement here the routines of your package
//


#endif /* GECODEMOPKG_SEEN_ */
