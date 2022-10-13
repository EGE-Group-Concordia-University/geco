// $Id: ECTrigger.cc 15 2014-01-03 18:49:04Z wuthrich $
// ---------------------------------------------------------------
//
// Definition of the class gecoPulse
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
// 10.02.2016 Creation                         R. Wuthrich
// 03.12.2020 Added doxygen documentation      R. Wuthrich
//
// ---------------------------------------------------------------

#include <tcl.h>
#include <cmath>
#include "gecoApp.h"
#include "gecoPulse.h"

using namespace std;


// -------------------------------------------------------------------------
//
// Tcl interface
//


// Command to create a new pulse object
//

int geco_PulseCmd(ClientData clientData, Tcl_Interp *interp, 
		  int objc,Tcl_Obj *const objv[])
{
  gecoPulse* proc  = new gecoPulse((gecoApp *)clientData);
  return geco_CreateGecoProcessCmd(proc, objc, objv);
}

// -------------------------------------------------------------------------


// -------------------------------------------------------------------------
//
// class gecoPulse : implements a pulse train
//


/**
 * @copydoc gecoGenerator::handleEvent
 *
 * In addition to gecoGenerator::handleEvent, gecoPulse::handleEvent implements
 * the termination of the gecoPulse once it was active longer than duration.
 */

void gecoPulse::handleEvent(gecoEvent* ev)
{
  gecoGenerator::handleEvent(ev);

  if (status!=Active) return;

  if (timeSinceActivated()>=duration) terminate(ev);
}


/**
 * @copydoc gecoGenerator::info
 *
 * In addition to gecoGenerator::info, gecoPulse::info adds the information
 * about the pulse train parameters.
 */

Tcl_DString* gecoPulse::info(const char* frontStr)
{
  gecoGenerator::info(frontStr);
  addInfo(frontStr, "pulse high value (V) = ", high);
  addInfo(frontStr, "pulse low value (V) = ", low);
  addInfo(frontStr, "pulse-on time (ms) = ", Ton);
  addInfo(frontStr, "pulse-off time (ms) = ", Toff);
  addInfo(frontStr, "duration of signal (s) = ", duration);
  return infoStr;
}


/**
 * @brief Computes the pulse train signal
 * @param t time since the geco event loop got started
 * \return The computed signal
 */

double gecoPulse::signalFunction(double t)
{
  t=t-to();

  // computes time since last time pulse switched to ON
  unsigned long int n = floor(1000*t/(Ton+Toff));

  // computes potential to output
  if (1000*t-n*(Ton+Toff)<=Ton) 
    return high;
  else
    return low;
}
