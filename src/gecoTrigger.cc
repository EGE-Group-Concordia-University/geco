// $Id: ECTrigger.cc 15 2014-01-03 18:49:04Z wuthrich $
// ---------------------------------------------------------------
//
// Definition of the class gecoTrigger
//
// (c) Rolf Wuthrich
//     2015-2016 Concordia University
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

#include <tcl.h>
#include <cstring>
#include "gecoTrigger.h"

using namespace std;


// -------------------------------------------------------------------------
//
// Tcl interface
//

// Command to create a new trigger object
//

int geco_TriggerCmd(ClientData clientData, Tcl_Interp *interp, 
		    int objc,Tcl_Obj *const objv[])
{
  gecoTrigger* trig = new gecoTrigger((gecoApp *)clientData);
  return geco_CreateGecoProcessCmd(trig,objc,objv);
}

// -------------------------------------------------------------------------


// -------------------------------------------------------------------------
//
// class gecoTrigger : a class implementing triggers
//


/**
 * @brief Destructor
*/

gecoTrigger::~gecoTrigger()
{
  Tcl_DStringFree(triggerScript);
  Tcl_DStringFree(actionScript);
  delete triggerScript;
  delete actionScript;
}


/*!
 * @copydoc gecoProcess::cmd 
 *
 * Compared to gecoProcess::cmd, gecoTrigger::cmd adds the processing of
 * the new subcommands of gecoTrigger.
 */

int gecoTrigger::cmd(int &i, int objc,Tcl_Obj *const objv[])
{
  // first executes the command options defined in gecoProcess
  int index=gecoProcess::cmd(i,objc,objv);

  if (index==getOptionIndex("-always"))
    {
      trigger_type = Trigger_always;
      i++;
    }

  if (index==getOptionIndex("-single"))
    {
      trigger_type = Trigger_single;
      i++;
    }

  return index;
}


/**
 * @copydoc gecoProcess::handleEvent
 *
 * In addition to gecoProcess::handleEvent, gecoTrigger::handleEvent implements
 * the monitoring of the trigger condition and the execution of the action in 
 * case the trigger condition is met
 */

void gecoTrigger::handleEvent(gecoEvent* ev)
{
  // in order to activate next process
  if ((status==Waiting)&&
      ((ev->cmd()==CmdStart)||(ev->cmd()==CmdProcessTerminated)))
    {
      activate(ev);
      ev->setCmdEvent(CmdProcessTerminated, this);
    }

  gecoProcess::handleEvent(ev);

  // If the trigger is Active, evaluates the trigger script.
  // Terminates the trigger process if the script returns 0 or STOP

  if (status==Active)
    {
      Tcl_Eval(interp,Tcl_DStringValue(triggerScript)); 
      if ((strcmp(Tcl_GetStringResult(interp),"0")==0)||
          (strcmp(Tcl_GetStringResult(interp),"STOP")==0))
	{
	  if (verbose==1)
	    {
	      char str[80];
	      sprintf(str, "cons \"  ==> trig%s released\"", objID); 
	      Tcl_Eval(interp,str);
	    }
	  Tcl_Eval(interp, Tcl_DStringValue(actionScript));
	  
	  // saves event
	  int evCmd = ev->cmd();
	  gecoProcess* proc = ev->eventGenerator();
	  if (trigger_type==Trigger_single) terminate(ev);
	  ev->setCmdEvent(evCmd, proc); // restores event
	}
    }

  // Necessary as otherwise the result of the TriggerScript would
  // remain in the interpreter
  Tcl_ResetResult(interp);
}


/**
 * @copydoc gecoProcess::info
 *
 * In addition to gecoProcess::info, gecoTrigger::info adds the information
 * about the trigger condition and action.
 */ 

Tcl_DString* gecoTrigger::info(const char* frontStr)
{
  gecoProcess::info(frontStr);
  addInfo(frontStr, "  ", Tcl_DStringValue(triggerScript));
  Tcl_DStringAppend(infoStr, "\nAction:", -1);
  addInfo(frontStr, "  ", Tcl_DStringValue(actionScript));  
  if (trigger_type==Trigger_always)
    addInfo(frontStr, "Trigger type : always", "");
  else
    addInfo(frontStr, "Trigger type : single", "");
  return infoStr;
}


/**
 * @brief Sets the trigger condition
 * @param TriggerScript Tcl script which has to retrun '0' or 'STOP' in case the trigger condition is met
 */

void gecoTrigger::setTriggerScript(Tcl_DString* TriggerScript)
{
  Tcl_DStringFree(triggerScript);
  Tcl_DStringAppend(triggerScript,Tcl_DStringValue(TriggerScript),-1);
}


