// This may look like C code, but it is really -*- C++ -*-
// $Id: ECTrigger.h 15 2014-01-03 18:49:04Z wuthrich $
// ---------------------------------------------------------------- 
//                                                                  
// Header file for class gecoEnd
//
// (c) Rolf Wuthrich
//     2016 - 2020 Concordia University
//
// author:  Rolf Wuthrich
// email:   rolf.wuthrich@concordia.ca
// version: v1 ($Revision: 15 $)
//
// This software is copyright under the BSD license
//
// ---------------------------------------------------------------
// history:
// ---------------------------------------------------------------
// Date       Modification                     Author
// ---------------------------------------------------------------
// 11.02.2016 Creation                         R. Wuthrich
// 08.12.2020 Added doxygen documentation      R. Wuthrich
//
// ---------------------------------------------------------------
/*! \file */

#ifndef gecoEnd_SEEN_
#define gecoEnd_SEEN_

#include <tcl8.6/tcl.h>
#include "gecoProcess.h"
#include "gecoApp.h"

using namespace std;


// -----------------------------------------------------------------------
//
// Tcl interface
//

/**
 * @brief C++ implementation of the Tcl command to create a gecoEnd object
 * @param clientData pointer to the gecoApp in which the gecoEnd will live
 * @param interp Tcl interpreter in which the Tcl command is executed
 * @param objc number of arguments of the Tcl command
 * @param objv arguments of the the Tcl command
 * \return TCL_OK if the execution of the Tcl command is successful and TCL_ERROR otherwise
 */

int geco_EndCmd(ClientData clientData, Tcl_Interp *interp, 
		int objc,Tcl_Obj *const objv[]);


// -----------------------------------------------------------------------
//
// class gecoEnd : a class for ending the geco event loop
//

/** 
 * @brief A gecoProcess to end the geco loop
 * \author Rolf Wuthrich
 * \date 2015-2020
 *
 * The gecoEnd class is a gecoProcess to end the geco loop.
 * Each geco loop must have as last gecoProcess a gecoEnd process.
 * The gecoEnd process will emit the Tcl 'stop' command as soon as it gets
 * activated and in this way end the geco process loop.
 *
 * The geco_EndCmd() is the C++ implementation of the Tcl command
 * to create gecoEnd objects. This Tcl command is already available in 
 * the Tcl interpreter run by an instance of gecoApp under the name 'end'.
 */

class gecoEnd : public gecoProcess
{


public:

  gecoEnd(gecoApp* App) :
    gecoObj("End geco loop", "end", App),
    gecoProcess("End geco loop", "user", "end", App)
  {}

  virtual void handleEvent(gecoEvent* ev);
};

#endif /* gecoEnd_SEEN_ */
