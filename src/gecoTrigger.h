// This may look like C code, but it is really -*- C++ -*-
// $Id: ECTrigger.h 15 2014-01-03 18:49:04Z wuthrich $
// ---------------------------------------------------------------- 
//                                                                  
// Header file for class gecoTrigger
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
// 17.10.2015 Creation                         R. Wuthrich
// 09.12.2020 Added doxygen documentation      R. Wuthrich
//
// ---------------------------------------------------------------
/*! \file */

#ifndef gecoTrigger_SEEN_
#define gecoTrigger_SEEN_

#include <tcl8.6/tcl.h>
#include <stdio.h>
#include "gecoProcess.h"

using namespace std;


// ------------------------------------------------------------------------
//
// Trigger type
//

const int
  Trigger_single = 1,
  Trigger_always = 0;


// -----------------------------------------------------------------------
//
// Tcl interface
//

/**
 * @brief C++ implementation of the Tcl command to create a gecoTrigger object
 * @param clientData pointer to the gecoApp in which the gecoTrigger instance will live
 * @param interp Tcl interpreter in which the Tcl command is executed
 * @param objc number of arguments of the Tcl command
 * @param objv arguments of the the Tcl command
 * \return TCL_OK if the execution of the Tcl command is successful and TCL_ERROR otherwise
 */

int geco_TriggerCmd(ClientData clientData, Tcl_Interp *interp, 
		    int objc,Tcl_Obj *const objv[]);


// -----------------------------------------------------------------------
//
// class gecoTrigger : a class implementing triggers
//

/** 
 * @brief A gecoProcess to create a trigger
 * \author Rolf Wuthrich
 * \date 2015-2020
 *
 * The gecoTrigger class allows to create a gecoProcess able to
 * create a trigger. A trigger can monitor specific events or conditions
 * (e.g. a Tcl variable reaches a defined threshold) and initiate
 * an action (execution of a Tcl script) if the condition is met.
 *
 * The geco_TriggerCmd() is the C++ implementation for the Tcl command to
 * create gecoTrigger objects. This Tcl command is already available in 
 * the Tcl interpreter run by an instance of gecoApp under the name 'trigger'.
 *
 * Associated Tcl command
 * ----------------------
 * Every gecoTrigger is associated to a Tcl command. The associated Tcl command 
 * is created during the construction of an instance of gecoTrigger by its
 * parent class. The Tcl command is created in the Tcl interpreter
 * run by the gecoApp in which gecoTrigger lives.
 *
 * The gecoTrigger class extends the subcommands from gecoObj and gecoProcess
 * by the following subcommands
 *
 * Sub-command       | Short description
 * ----------------- | ------------------
 * -triggerScript    | returns/sets trigger condition
 * -action           | returns/sets action
 * -single           | trigger is removed after release
 * -always           | trigger is permanent
 *
 * The trigger condition to monitor is defined by the user with the '-triggerScript'
 * subcommand. It is a Tcl script which has to return '0' or 'STOP' when the trigger
 * condition is met. 
 *
 * The return of '0' or 'STOP' by the Tcl script passed to '-triggerScript' will trigger the
 * execution of the Tcl script defined via the '-action' subcommand.
 */

class gecoTrigger : public gecoProcess
{
private:

  int  trigger_type;

protected:

  Tcl_DString* triggerScript;
  Tcl_DString* actionScript;

public:

  gecoTrigger(gecoApp* App) :
    gecoObj("Trigger", "trig", App),
    gecoProcess("Trigger", "user", "trig", App),
    trigger_type(Trigger_single)
  {
    triggerScript = new Tcl_DString;
    Tcl_DStringInit(triggerScript);
    Tcl_DStringAppend(triggerScript, "1", -1);
    actionScript = new Tcl_DString;
    Tcl_DStringInit(actionScript);
    Tcl_DStringAppend(actionScript, "", -1);
    verbose=0;
    addOption("-triggerScript", triggerScript, "returns/sets trigger condition");
    addOption("-action", actionScript, "returns/sets action");
    addOption("-single", "trigger is removed after release");
    addOption("-always", "trigger is permanent");
  }

  ~gecoTrigger();

  virtual int  cmd(int &i,int objc,Tcl_Obj *const objv[]);
  virtual void handleEvent(gecoEvent* ev);
  virtual Tcl_DString* info(const char* frontStr = "");

  void          setTriggerScript(Tcl_DString* TriggerScript);
  Tcl_DString*  getTriggerScript() {return triggerScript;}
};

#endif /* gecoTrigger_SEEN_ */
