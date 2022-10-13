// This may look like C code, but it is really -*- C++ -*-
// $Id: ECTrigger.h 15 2014-01-03 18:49:04Z wuthrich $
// ---------------------------------------------------------------- 
//
// Header file for class gecoTriangle
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
/*! \file */

#ifndef gecoTriangle_SEEN_
#define gecoTriangle_SEEN_

#include <tcl.h>
#include <stdio.h>
#include "gecoGenerator.h"

using namespace std;


// -----------------------------------------------------------------------
//
// Tcl interface
//

/**
 * @brief C++ implementation of the Tcl command to create a gecoTriangle object
 * @param clientData pointer to the gecoApp in which the gecoTriangle will live
 * @param interp Tcl interpreter in which the Tcl command is executed
 * @param objc number of arguments of the Tcl command
 * @param objv arguments of the the Tcl command
 * \return TCL_OK if the execution of the Tcl command is successful and TCL_ERROR otherwise
 */

int geco_TriangleCmd(ClientData clientData, Tcl_Interp *interp, 
		     int objc,Tcl_Obj *const objv[]);


// -----------------------------------------------------------------------
//
// class gecoTriangle : implements a triangular signal
//

/** 
 * @brief A triangle signal generator
 * \author Rolf Wuthrich
 * \date 2015-2020
 *
 *
 * The gecoTriangle class allows to generate a triangle signal. The
 * triangle characteristics can be set by the user with the various
 * subcommand from the Tcl command associated to the sawtooth train.
 *
 * The geco_TriangleCmd() is the C++ implementation for the Tcl command to
 * create gecoTriangle objects. This Tcl command is already available in 
 * the Tcl interpreter run by an instance of gecoApp under the name 'triangle'.
 *
 * Associated Tcl command
 * ----------------------
 * Every gecoTriangle is associated to a Tcl command. The associated Tcl command 
 * is created during the construction of an instance of gecoTriangle by its
 * parent class. The Tcl command is created in the Tcl interpreter
 * run by the gecoApp in which gecoTriangle lives.
 *
 * The gecoTriangle class extends the subcommands from gecoObj, gecoProcess
 * and gecoGenerator by the following subcommands
 *
 * Sub-command       | Short description
 * ----------------- | ------------------
 * -UHigh            | returns/sets the high value of the triangle
 * -ULow             | returns/sets the low value of the triangle
 * -period           | returns/sets the period of the triangle
 * -duration         | returns/sets the duration of the triangle train
 */

class gecoTriangle : public gecoGenerator
{

protected:

  double  period, Ulow, Uhigh, duration;

public:

  gecoTriangle(gecoApp* App) :
    gecoObj("Triangle signal","triangle",App),
    gecoGenerator("Triangle signal","triangle",App),
    period(0.0), Ulow(0.0), Uhigh(0.0)
  {
    addOption("-period", &period, "returns/sets period of signal (s)");
    addOption("-Ulow", &Ulow, "returns/sets Ulow");
    addOption("-Uhigh", &Uhigh, "returns/sets Uhigh");
    addOption("-duration", &duration, "returns/sets duration of signal (s)");
  }

  virtual Tcl_DString* info(const char* frontStr = "");
  virtual void handleEvent(gecoEvent* ev);
  virtual double signalFunction(double t);
};

#endif /* gecoTriangle_SEEN_ */
