// $Id: ECTrigger.cc 15 2014-01-03 18:49:04Z wuthrich $
// ---------------------------------------------------------------
//
// Definition of the class gecoStep
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
// 03.02.2016 Creation                         R. Wuthrich
// 03.12.2020 Added doxygen documentation      R. Wuthrich
//
// ---------------------------------------------------------------

#include <tcl.h>
#include <cmath>
#include "gecoApp.h"
#include "gecoStep.h"

using namespace std;


// -------------------------------------------------------------------------
//
// Tcl interface
//


// Command to create a new step object
//

int geco_StepCmd(ClientData clientData, Tcl_Interp *interp, 
		 int objc,Tcl_Obj *const objv[])
{
  gecoStep* proc  = new gecoStep((gecoApp *)clientData);
  return geco_CreateGecoProcessCmd(proc,objc,objv);
}

// -------------------------------------------------------------------------


// -------------------------------------------------------------------------
//
// class gecoStep : implements a step signal
//


/**
 * @copydoc gecoGenerator::handleEvent
 *
 * In addition to gecoGenerator::handleEvent, gecoStep::handleEvent implements
 * the termination of the gecoStep once it was active longer than duration.
 */

void gecoStep::handleEvent(gecoEvent* ev)
{
  gecoGenerator::handleEvent(ev);

  if (status!=Active) return;

  if (timeSinceActivated()>=duration) terminate(ev);
}


/**
 * @copydoc gecoGenerator::info
 *
 * In addition to gecoGenerator::info, gecoStep::info adds the information
 * about the step parameters.
 */ 

Tcl_DString* gecoStep::info(const char* frontStr)
{
  gecoGenerator::info(frontStr);
  addInfo(frontStr, "Step-time (s) = ", stepTime);
  addInfo(frontStr, "E1 = ", E1);
  addInfo(frontStr, "E2 = ", E2);
  addInfo(frontStr, "Duration (s) = ", duration);
  return infoStr;
}


/**
 * @brief Computes the step signal
 * @param t time since the geco event loop got started
 * \return The computed signal
 */

double gecoStep::signalFunction(double t)
{
  t = t-to();
  if (t<=stepTime)
    return E1;
  else
    return E2;
}
