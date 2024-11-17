// This may look like C code, but it is really -*- C++ -*-
// $Id: ECTrigger.h 15 2014-01-03 18:49:04Z wuthrich $
// ---------------------------------------------------------------- 
//
// Header file for class gecoPulse
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
/*! \file */

#ifndef gecoPulse_SEEN_
#define gecoPulse_SEEN_

#include <tcl8.6/tcl.h>
#include <stdio.h>
#include "gecoGenerator.h"

using namespace std;


// -----------------------------------------------------------------------
//
// Tcl interface
//

/**
 * @brief C++ implementation of the Tcl command to create a gecoPulse object
 * @param clientData pointer to the gecoApp in which the gecoPulse will live
 * @param interp Tcl interpreter in which the Tcl command is executed
 * @param objc number of arguments of the Tcl command
 * @param objv arguments of the the Tcl command
 * \return TCL_OK if the execution of the Tcl command is successful and TCL_ERROR otherwise
 */

int geco_PulseCmd(ClientData clientData, Tcl_Interp *interp, 
		  int objc,Tcl_Obj *const objv[]);


// -----------------------------------------------------------------------
//
// class gecoPulse : implements a pulse train
//

/** 
 * @brief A pulse train signal generator
 * \author Rolf Wuthrich
 * \date 2015-2020
 *
 *
 * The gecoPulse class allows to generate a pulse train signal. The
 * pulse train characteristics can be set by the user with the various
 * subcommand from the Tcl command associated to the pulse train.
 *
 * The geco_PulseCmd() is the C++ implementation of the Tcl command to
 * create gecoPulse objects. This Tcl command is already available in 
 * the Tcl interpreter run by an instance of gecoApp under the name 'pulse'.
 *
 * Associated Tcl command
 * ----------------------
 * Every gecoPulse is associated to a Tcl command. The associated Tcl command 
 * is created during the construction of an instance of gecoPulse by its
 * parent class. The Tcl command is created in the Tcl interpreter
 * run by the gecoApp in which gecoPulse lives.
 *
 * The gecoPulse class extends the subcommands from gecoObj, gecoProcess
 * and gecoGenerator by the following subcommands
 *
 * Sub-command       | Short description
 * ----------------- | ------------------
 * -pulseHigh        | returns/sets the high value of the pulse train
 * -pulseLow         | returns/sets the low value of the pulse train
 * -Ton              | returns/sets the on-time of the pulse train
 * -Toff             | returns/sets the off-time of the pulse train
 * -duration         | returns/sets the duration of the pulse train
 */

class gecoPulse : public gecoGenerator
{

protected:

  double high;     /// Pulse high value
  double low;      /// Pulse low value
  double Ton;      /// Pulse-On time in ms
  double Toff;     /// Pulse-Off in ms
  double duration; /// Pulse-train duration in sec

public:

  gecoPulse(gecoApp* App) :
    gecoObj("Pulse train", "pulse", App),
    gecoGenerator("Pulse train", "pulse", App),
    high(0), low(0), Ton(0), Toff(0), duration(0)
  {
    addOption("-pulseHigh",&high,"returns/sets pulse high value (V)");
    addOption("-pulseLow",&low,"returns/sets pulse low value (V)");
    addOption("-Ton",&Ton,"returns/sets pulse on-time (ms)");
    addOption("-Toff",&Toff,"returns/sets pulse off-time (ms)");
    addOption("-duration",&duration,"returns/sets pulse train duration (s)");
  }

  virtual Tcl_DString* info(const char* frontStr = "");
  virtual void handleEvent(gecoEvent* ev);
  virtual double signalFunction(double t);
};

#endif /* gecoPulse_SEEN_ */
