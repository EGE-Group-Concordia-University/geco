// $Id: ECTrigger.cc 15 2014-01-03 18:49:04Z wuthrich $
// ---------------------------------------------------------------
//
// Definition of the class gecoUProc
//
// (c) Rolf Wuthrich
//     2015 - 2020 Concordia University
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
// 23.10.2015 Creation                         R. Wuthrich
// 09.12.2020 Added doxygen documentation      R. Wuthrich
//
// ---------------------------------------------------------------

#include <tcl.h>
#include <cstring>
#include "gecoUProc.h"
#include "gecoApp.h"

using namespace std;


// -------------------------------------------------------------------------
//
// Tcl interface
//


// Command to create a new uproc object
//

int geco_UProcCmd(ClientData clientData, Tcl_Interp *interp, 
		  int objc,Tcl_Obj *const objv[])
{
  gecoUProc* proc  = new gecoUProc((gecoApp *)clientData);
  return geco_CreateGecoProcessCmd(proc,objc,objv);
}

// -------------------------------------------------------------------------


// -------------------------------------------------------------------------
//
// class gecoUProc : a class for User Processes
//


/**
 * @brief Destructor
*/

gecoUProc::~gecoUProc()
{
  Tcl_DStringFree(userScript);
  delete userScript;
}


/**
 * @copydoc gecoProcess::handleEvent
 *
 * In addition to gecoProcess::handleEvent, gecoUProc::handleEvent implements
 * the call to the user defined Tcl script.
 */

void gecoUProc::handleEvent(gecoEvent* ev)
{
  gecoProcess::handleEvent(ev);

  // If the user process is Active, evaluates the user script.
  if (status==Active) Tcl_Eval(interp,Tcl_DStringValue(userScript)); 

  // Necessary as otherwise the result of the UserScript would
  // remain in the interpreter
  Tcl_ResetResult(interp);
}


/**
 * @copydoc gecoProcess::info
 *
 * In addition to gecoProcess::info, gecoUProc::info adds the information
 * about the user defined Tcl script.
 */

Tcl_DString* gecoUProc::info(const char* frontStr)
{
  gecoProcess::info(frontStr);
  addInfo(frontStr, "\nProcess:\n\t", Tcl_DStringValue(userScript));  
  return infoStr;
}
