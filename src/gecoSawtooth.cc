// $Id: ECTrigger.cc 15 2014-01-03 18:49:04Z wuthrich $
// ---------------------------------------------------------------
//
// Definition of the class gecoSawtooth
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
// 02.02.2016 Creation                         R. Wuthrich
// 03.12.2020 Added doxygen documentation      R. Wuthrich
//
// ---------------------------------------------------------------

#include <tcl.h>
#include <cmath>
#include "gecoApp.h"
#include "gecoSawtooth.h"

using namespace std;


// -------------------------------------------------------------------------
//
// Tcl interface
//


// Command to create a new sawtooth object
//

int geco_SawtoothCmd(ClientData clientData, Tcl_Interp *interp, 
		     int objc,Tcl_Obj *const objv[])
{
  gecoSawtooth* proc  = new gecoSawtooth((gecoApp *)clientData);
  return geco_CreateGecoProcessCmd(proc,objc,objv);
}

// -------------------------------------------------------------------------


// -------------------------------------------------------------------------
//
// class gecoSawtooth : implements a sawtooth signal
//


/**
 * @copydoc gecoGenerator::handleEvent
 *
 * In addition to gecoGenerator::handleEvent, gecoSawtooth::handleEvent implements
 * the termination of the gecoSawtooth once it was active longer than duration.
 */

void gecoSawtooth::handleEvent(gecoEvent* ev)
{
  gecoGenerator::handleEvent(ev);

  if (status!=Active) return;

  if (timeSinceActivated()>=duration) terminate(ev);
}



/**
 * @copydoc gecoGenerator::info
 *
 * In addition to gecoGenerator::info, gecoSawtooth::info adds the information
 * about the sawtooth train parameters.
 */

Tcl_DString* gecoSawtooth::info(const char* frontStr)
{
  gecoGenerator::info(frontStr);
  addInfo(frontStr, "Period (s) = ", period);
  addInfo(frontStr, "Ulow (V) = ", Ulow);
  addInfo(frontStr, "Uhigh (V) = ", Uhigh);
  addInfo(frontStr, "Duration (s) = ", duration);
  return infoStr;
}


/**
 * @brief Computes the sawtooth train signal
 * @param t time since the geco event loop got started
 * \return The computed signal
 */

double gecoSawtooth::signalFunction(double t)
{
  t = t-to();
  double scanRate = abs(Uhigh-Ulow)/period;
  return Ulow+sign(Uhigh-Ulow)*scanRate*(t-floor(t/period)*period);
}
