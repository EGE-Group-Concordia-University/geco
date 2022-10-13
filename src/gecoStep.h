// This may look like C code, but it is really -*- C++ -*-
// $Id: ECTrigger.h 15 2014-01-03 18:49:04Z wuthrich $
// ---------------------------------------------------------------- 
//
// Header file for class gecoStep
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
// 03.02.2016 Creation                         R. Wuthrich
// 03.12.2020 Added doxygen documentation      R. Wuthrich
//
// ---------------------------------------------------------------
/*! \file */

#ifndef gecoStep_SEEN_
#define gecoStep_SEEN_

#include <tcl.h>
#include <stdio.h>
#include "gecoGenerator.h"

using namespace std;


// -----------------------------------------------------------------------
//
// Tcl interface
//

/**
 * @brief C++ implementation of the Tcl command to create a gecoStep object
 * @param clientData pointer to the gecoApp in which the gecoStep will live
 * @param interp Tcl interpreter in which the Tcl command is executed
 * @param objc number of arguments of the Tcl command
 * @param objv arguments of the the Tcl command
 * \return TCL_OK if the execution of the Tcl command is successful and TCL_ERROR otherwise
 */

int geco_StepCmd(ClientData clientData, Tcl_Interp *interp, 
		 int objc,Tcl_Obj *const objv[]);


// -----------------------------------------------------------------------
//
// class gecoStep : implements a step signal
//

/** 
 * @brief A step signal generator
 * \author Rolf Wuthrich
 * \date 2015-2020
 *
 *
 * The gecoStep class allows to generate a step signal. The
 * step characteristics can be set by the user with the various
 * subcommand from the Tcl command associated to the step.
 *
 * The geco_StepCmd() is the C++ implementation of the Tcl command to
 * create gecoStep objects. This Tcl command is already available in 
 * the Tcl interpreter run by an instance of gecoApp under the name 'step'.
 *
 * Associated Tcl command
 * ----------------------
 * Every gecoStep is associated to a Tcl command. The associated Tcl command 
 * is created during the construction of an instance of gecoStep by its
 * parent class. The Tcl command is created in the Tcl interpreter
 * run by the gecoApp in which gecoStep lives.
 *
 * The gecoStep class extends the subcommands from gecoObj, gecoProcess
 * and gecoGenerator by the following subcommands
 *
 * Sub-command       | Short description
 * ----------------- | ------------------
 * -stepTime         | returns/sets the step time (s)
 * -E1               | returns/sets the first step value (before step)
 * -E2               | returns/sets the second step value (after step)
 * -duration         | returns/sets the duration of the step (s)
 */

class gecoStep : public gecoGenerator
{

protected:

  double  stepTime, E1, E2, duration;

public:

  gecoStep(gecoApp* App) :
    gecoObj("Step signal","step",App),
    gecoGenerator("Step signal","step",App),
    stepTime(0.0), E1(0.0), E2(0.0)
  {
    addOption("-stepTime", &stepTime, "returns/sets the step-time (s)");
    addOption("-E1", &E1, "returns/sets first step value (before step)");
    addOption("-E2", &E2, "returns/sets second step value (after step)");
    addOption("-duration", &duration, "returns/sets duration of signal (s)");
  }

  virtual Tcl_DString* info(const char* frontStr = "");
  virtual void handleEvent(gecoEvent* ev);
  virtual double signalFunction(double t);
};

#endif /* gecoStep_SEEN_ */
