// $Id: ECTrigger.cc 15 2014-01-03 18:49:04Z wuthrich $
// ---------------------------------------------------------------
//
// Definition of the class gecoEnd
//
// (c) Rolf Wuthrich
//     2016 Concordia University
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

#include <tcl.h>
#include "gecoEnd.h"
#include "gecoApp.h"

using namespace std;


// -------------------------------------------------------------------------
//
// Tcl interface
//


// Command to create a new end object
//

int geco_EndCmd(ClientData clientData, Tcl_Interp *interp, 
		int objc,Tcl_Obj *const objv[])
{
  gecoEnd* proc = new gecoEnd((gecoApp *)clientData);
  return geco_CreateGecoProcessCmd(proc, objc, objv);
}

// -------------------------------------------------------------------------


// -------------------------------------------------------------------------
//
// class gecoEnd : a class to end the geco loop
//


/**
 * @copydoc gecoProcess::handleEvent
 *
 * In addition to gecoProcess::handleEvent, gecoEnd::handleEvent implements
 * the termination of the geco process loop by emitting the Tcl 'stop'
 * command as soon as the gecoEnd process gets activated.
 */

void gecoEnd::handleEvent(gecoEvent* ev)
{
  gecoProcess::handleEvent(ev);

  if (status!=Active) return;

  Tcl_Eval(interp, "stop"); 
}

