// $Id: ECTrigger.cc 15 2014-01-03 18:49:04Z wuthrich $
// ---------------------------------------------------------------
//
// Definition of the class gecoTriangle
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
// 02.02.2016 Creation                         R. Wuthrich
// 03.12.2020 Added doxygen documentation      R. Wuthrich
//
// ---------------------------------------------------------------

#include <tcl.h>
#include <cmath>
#include "gecoApp.h"
#include "gecoTriangle.h"

using namespace std;


// -------------------------------------------------------------------------
//
// Tcl interface
//


// Command to create a new triangle object
//

int geco_TriangleCmd(ClientData clientData, Tcl_Interp *interp, 
		     int objc,Tcl_Obj *const objv[])
{
  gecoTriangle* proc  = new gecoTriangle((gecoApp *)clientData);
  return geco_CreateGecoProcessCmd(proc,objc,objv);
}

// -------------------------------------------------------------------------


// -------------------------------------------------------------------------
//
// class gecoTriangle : implements a triangular signal
//


/**
 * @copydoc gecoGenerator::handleEvent
 *
 * In addition to gecoGenerator::handleEvent, gecoTriangle::handleEvent implements
 * the termination of the gecoTriangle once it was active longer than duration.
 */

void gecoTriangle::handleEvent(gecoEvent* ev)
{
  gecoGenerator::handleEvent(ev);

  if (status!=Active) return;

  if (timeSinceActivated()>=duration) terminate(ev);
}


/**
 * @copydoc gecoGenerator::info
 *
 * In addition to gecoGenerator::info, gecoTriangle::info adds the information
 * about the triangle parameters.
 */

Tcl_DString* gecoTriangle::info(const char* frontStr)
{
  gecoGenerator::info(frontStr);
  addInfo(frontStr, "period (s) = ", period);
  addInfo(frontStr, "Ulow = ", Ulow);
  addInfo(frontStr, "Uhigh = ", Uhigh);
  addInfo(frontStr, "duration of signal (s) = ", duration);
  return infoStr;
}


/**
 * @brief Computes the triangle train signal
 * @param t time since the geco event loop got started
 * \return The computed signal
 */

double gecoTriangle::signalFunction(double t)
{
  t = t-to();
  double scanRate = abs(Uhigh-Ulow)/period;
  int n = floor(t/period);
  double dt = t-2*floor(t/period/2)*period;
  if (n % 2 == 0)
    return Ulow+sign(Uhigh-Ulow)*scanRate*dt;
  else
    return Uhigh-sign(Uhigh-Ulow)*scanRate*(dt-period);
}
