// This may look like C code, but it is really -*- C++ -*-
// $Id: ECTrigger.h 15 2014-01-03 18:49:04Z wuthrich $
// ---------------------------------------------------------------- 
//
// Header file for class gecoSawtooth
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

#ifndef gecoSawtooth_SEEN_
#define gecoSawtooth_SEEN_

#include <tcl.h>
#include <stdio.h>
#include "gecoGenerator.h"

using namespace std;


// -----------------------------------------------------------------------
//
// Tcl interface
//

/**
 * @brief C++ implementation of the Tcl command to create a gecoSawtooth object
 * @param clientData pointer to the gecoApp in which the gecoPulse will live
 * @param interp Tcl interpreter in which the Tcl command is executed
 * @param objc number of arguments of the Tcl command
 * @param objv arguments of the the Tcl command
 * \return TCL_OK if the execution of the Tcl command is successful and TCL_ERROR otherwise
 */

int geco_SawtoothCmd(ClientData clientData, Tcl_Interp *interp, 
		     int objc,Tcl_Obj *const objv[]);


// -----------------------------------------------------------------------
//
// class gecoSawtooth : implements a sawtooth signal
//

/** 
 * @brief A sawtooth train signal generator
 * \author Rolf Wuthrich
 * \date 2015-2020
 *
 *
 * The gecoSawtooth class allows to generate a sawtooth train signal. The
 * sawtooth train characteristics can be set by the user with the various
 * subcommand from the Tcl command associated to the sawtooth train.
 *
 * The geco_SawtoothCmd() is the C++ implementation of the Tcl command to
 * create gecoSawtooth objects. This Tcl command is already available in 
 * the Tcl interpreter run by an instance of gecoApp under the name 'sawtooth'.
 *
 * Associated Tcl command
 * ----------------------
 * Every gecoSawtooth is associated to a Tcl command. The associated Tcl command 
 * is created during the construction of an instance of gecoSawtooth by its
 * parent class. The Tcl command is created in the Tcl interpreter
 * run by the gecoApp in which gecoSawtooth lives.
 *
 * The gecoSawtooth class extends the subcommands from gecoObj, gecoProcess
 * and gecoGenerator by the following subcommands
 *
 * Sub-command       | Short description
 * ----------------- | ------------------
 * -UHigh            | returns/sets the high value of the sawtooth train
 * -ULow             | returns/sets the low value of the sawtooth train
 * -period           | returns/sets the period of the sawtooth train
 * -duration         | returns/sets the duration of the sawtooth train
 */

class gecoSawtooth : public gecoGenerator
{

protected:

  double  period, Ulow, Uhigh, duration;

public:

  gecoSawtooth(gecoApp* App) :
    gecoObj("Sawtooth signal","sawtooth",App),
    gecoGenerator("Sawtooth signal","sawtooth",App),
    period(0.0), Ulow(0.0), Uhigh(0.0)
  {
    addOption("-period", &period, "returns/sets period of signal (s)");
    addOption("-Ulow", &Ulow, "returns/sets Ulow (V)");
    addOption("-Uhigh", &Uhigh, "returns/sets Uhigh (V)");
    addOption("-duration", &duration, "returns/sets duration of signal (s)");
  }

  virtual Tcl_DString* info(const char* frontStr = "");
  virtual void handleEvent(gecoEvent* ev);
  virtual double signalFunction(double t);
};

#endif /* gecoSawtooth_SEEN_ */
