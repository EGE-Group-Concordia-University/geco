// ---------------------------------------------------------------
//
// Definition of the class gecoMTCAdapter
//
// (c) Rolf Wuthrich
//     2022 Concordia University
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
// 28.04.2022 Creation                         R. Wuthrich
//
// ---------------------------------------------------------------

#include <tcl.h>
#include <cstring>
#include "gecoHelp.h"
#include "gecoApp.h"
#include "gecoMTCAdapter.h"


using namespace std;


// -------------------------------------------------------------------------
//
// Tcl interface
//

// Command to create a new gecoMTCAdapter object
//

int geco_MTCAdapterCmd(ClientData clientData, Tcl_Interp *interp, 
		       int objc,Tcl_Obj *const objv[])
{
  Tcl_ResetResult(interp);
  gecoApp*        app=(gecoApp *)clientData;
  gecoMTCAdapter* adap;

  if (objc==1)
    {
      Tcl_WrongNumArgs(interp, 1, objv, "subcommand ?argument ...?");
      return TCL_ERROR;
    }

  int index;
  static CONST char* cmds[] = {"-help", "-open", NULL};
  static CONST char* help[] = {"opens a MTConnect adapter", NULL};

  if (Tcl_GetIndexFromObj(interp, objv[1], cmds, "subcommand", '0', &index)!=TCL_OK)
    return TCL_ERROR;

  int port;

  switch (index)
    {

    case 0: // -help
      if (objc!=2)
	{
	  Tcl_WrongNumArgs(interp, 2, objv, NULL);
	  return TCL_ERROR;
	}
      gecoHelp(interp, "mtcadapter", "MTConnect adapter", cmds, help);
      break;

    case 1: // -open
      if ((objc>4)||(objc<3))
	{
	  Tcl_WrongNumArgs(interp, 2, objv, "port ?cmdName?");
	  return TCL_ERROR;
	}

      if (Tcl_GetIntFromObj(interp,objv[2], &port)!=TCL_OK) return TCL_ERROR;

      if (objc==3)
        adap = new gecoMTCAdapter("MTConnect Adapter", "mtcadapter", port, (gecoApp *)clientData);
      
      if (objc==4)
	adap = new gecoMTCAdapter("MTConnect Adapter", Tcl_GetString(objv[3]), port, (gecoApp *)clientData, false);
      
      // adds the gecoProcess to the geco loop
      app->addGecoProcess(adap);
      Tcl_ResetResult(app->getInterp());
      Tcl_AppendResult(app->getInterp(), adap->getID(), NULL);
      break;

    }
  
  return TCL_OK;
}


// -------------------------------------------------------------------------



// -----------------------------------------------------------------------
//
// Class to store linked Tcl variables and MTConnect keys
//

// ---- CONSTRUCTOR
//

SHDRInsn::SHDRInsn(const char* Tcl_Var, const char* MTC_Key, Tcl_Interp *Interp)
{
  interp = Interp;
  TclVar = new Tcl_DString;
  Tcl_DStringInit(TclVar);
  Tcl_DStringAppend(TclVar, Tcl_Var, -1);
  TclVarValue = Tcl_Alloc(10);
  Tcl_LinkVar(interp, Tcl_Var, (char *)&TclVarValue, TCL_LINK_STRING);
  TclVarOldValue = new Tcl_DString;
  Tcl_DStringInit(TclVarOldValue);
  Tcl_DStringAppend(TclVarOldValue, " ", -1);
  
  MTCKey = new Tcl_DString;
  Tcl_DStringInit(MTCKey);
  Tcl_DStringAppend(MTCKey, MTC_Key, -1);

  SHDR = new Tcl_DString;
  Tcl_DStringInit(SHDR);
  
  next = NULL;
}


// ---- DESTRUCTOR
//

SHDRInsn::~SHDRInsn()
{
  Tcl_UnlinkVar(interp, Tcl_DStringValue(TclVar));
  Tcl_DStringFree(TclVar);
  delete TclVar;
  Tcl_DStringFree(MTCKey);
  delete MTCKey;
  Tcl_DStringFree(SHDR);
  delete SHDR;
  Tcl_Free(TclVarValue);
  Tcl_DStringFree(TclVarOldValue);
  delete TclVarOldValue;
}


char* SHDRInsn::getSHDR(bool forceSend)
{
  Tcl_DStringFree(SHDR);
  if ((strcmp(Tcl_DStringValue(TclVarOldValue), TclVarValue)!=0) || (forceSend))
    {
      Tcl_DStringAppend(SHDR, "|", -1);
      Tcl_DStringAppend(SHDR, Tcl_DStringValue(MTCKey), -1);
      Tcl_DStringAppend(SHDR, "|", -1);
      Tcl_DStringAppend(SHDR, TclVarValue, -1);
      Tcl_DStringFree(TclVarOldValue);
      Tcl_DStringAppend(TclVarOldValue, TclVarValue, -1);
    }
  return Tcl_DStringValue(SHDR);
}

// -------------------------------------------------------------------------



// -------------------------------------------------------------------------
//
// class gecoMTCAdapter: a class to stream data to a MTConnect agent
//

/**
 * @brief Destructor
*/

gecoMTCAdapter::~gecoMTCAdapter()
{
  SHDRInsn* p = getFirstSHDRInsn();
  SHDRInsn* q = getFirstSHDRInsn();
  while (p)
    {
      p = p->getNext();
      delete q;
      q = p;
    }
}


/*!
 * @copydoc gecoMTCBaseAdapter::cmd 
 *
 * Compared to gecoMTCBaseAdapter::cmd, gecoMTCAdapter::cmd adds the processing of
 * the new subcommands of gecoMTCAdapter.
 */

int gecoMTCAdapter::cmd(int &i,int objc,Tcl_Obj *const objv[])
{
  // executes the command options defined in gecoMTCBaseAdapter
  int index=gecoMTCBaseAdapter::cmd(i,objc,objv);

  if (index==getOptionIndex("-linkTclVariable"))
    {
      if (i+2>=objc)
      	{
      	  Tcl_WrongNumArgs(interp, i+1, objv, "Tcl_Variable MTConnect_Key");
      	  return -1;
      	}
      SHDRInsn* isn = new SHDRInsn(Tcl_GetString(objv[i+1]), Tcl_GetString(objv[i+2]), getTclInterp());
      addInsn(isn);
      i = i+3;
    }

  if (index==getOptionIndex("-unlinkTclVariable"))
    {
      if (i+1>=objc)
      	{
      	  Tcl_WrongNumArgs(interp, i+1, objv,"Tcl_Variable");
      	  return -1;
      	}
      SHDRInsn* p=findLinkedTclVar(Tcl_GetString(objv[i+1]));
      if (p==NULL)
	{
	  Tcl_AppendResult(interp, "variable \"", Tcl_GetString(objv[i+1]),
			          "\" is not linked to any MTConnect key", NULL);
	  return -1;
	}
      removeInsn(p);
      i = i+2;
    }

  if (index==getOptionIndex("-listLinkedTclVariables"))
    {
      listLinkedVar();
      i++;
    }


  return index;
}


/**
 * @copydoc gecoMTCBaseAdapter::handleEvent
 *
 */

void gecoMTCAdapter::handleEvent(gecoEvent* ev)
{
  gecoMTCBaseAdapter::handleEvent(ev);
}


/**
 * @copydoc gecoMTCBaseAdapter::info
 *
 * In addition to gecoMTCBaseAdapter::info, gecoMTCAdapter::info adds the information
 * about the data and file to stream to.
 */

Tcl_DString* gecoMTCAdapter::info(const char* frontStr)
{
  gecoMTCBaseAdapter::info(frontStr);

  return infoStr;
}


/**
 * @copydoc gecoMTCBaseAdapter::terminate
 */

void gecoMTCAdapter::terminate(gecoEvent* ev)
{
  gecoMTCBaseAdapter::terminate(ev);
}


/**
 * @copydoc gecoMTCBaseAdapter::activate
 *
 * In addition to gecoProcess::activate, gecoMTCBaseAdapter::activate 
 * sends result of SHDR(true) to a connected agent
 */

void gecoMTCAdapter::activate(gecoEvent* ev)
{
  gecoMTCBaseAdapter::activate(ev);
}



/**
 * @copydoc gecoMTCBaseAdapter::initialSHDR
 */

void gecoMTCAdapter::initialSHDR()
{
  gecoMTCBaseAdapter::initialSHDR();
}



/**
 * @brief Defines the SHDR data to be sent to agent in each loop of the gecoEventLoop
 * @param forceSend if true sends SDHR data even if values did not change since last sending
 * \return SHDR data to be sent to agent
 */

Tcl_DString* gecoMTCAdapter::SHDR(bool forceSend)
{
  Tcl_DStringFree(SHDR_Str);
  SHDRInsn* p = getFirstSHDRInsn();
  while (p)
    {
      Tcl_DStringAppend(SHDR_Str, p->getSHDR(forceSend), -1);
      p = p->getNext();
    }
  return SHDR_Str;
}


/**
 * @copydoc gecoMTCBaseAdapter::addAgent
 */

void gecoMTCAdapter::addAgent(Tcl_Channel chan, const char* hostName)
{
  gecoMTCBaseAdapter::addAgent(chan, hostName);
}


/**
 * @brief Lists all linked Tcl variables
 */

void gecoMTCAdapter::listLinkedVar()
{
  char str[100];
  Tcl_AppendResult(interp, "TCL VARIABLE   MTCONNECT KEY\n", NULL);

  SHDRInsn* p = getFirstSHDRInsn();

  while (p)
    {
      sprintf(str, "%-14s %s\n", 
	      Tcl_DStringValue(p->TclVar),
	      Tcl_DStringValue(p->MTCKey));
      Tcl_AppendResult(interp, str, NULL);
      p = p->getNext();
    }
}


/**
 * @brief Finds the SHDR instruction associated to a Tcl variable
 * @param TclVar Tcl variable associated to the SHDR instruction one wants to find
*/

SHDRInsn* gecoMTCAdapter::findLinkedTclVar(const char* TclVar)
{
  SHDRInsn* p = getFirstSHDRInsn();
  while (p)
    {
      if (strcmp(Tcl_DStringValue(p->TclVar), TclVar)==0) break;
      p = p->getNext();
    }
  return p;
}


/**
 * @brief Adds an instruction to the instruction list
 * @param insn SHDR instruction to be added to the list 
*/

void gecoMTCAdapter::addInsn(SHDRInsn* insn)
{
  SHDRInsn* p = getFirstSHDRInsn();
  if (p)
    {
      while (p->getNext())
	p = p->getNext();
      p->next = insn;
    }
  else
    firstSHDRInsn = insn;
  insn->next = NULL;
}


/**
 * @brief Removes a SHDR instruction from the list
 * @param insn SHDRInsn to be removed from the list 
*/

void gecoMTCAdapter::removeInsn(SHDRInsn* insn)
{
  if (insn==getFirstSHDRInsn())
    {
      firstSHDRInsn = insn->getNext();
      delete insn;
      return;
    }

  SHDRInsn* p = getFirstSHDRInsn();
  while(p)
    {
      if (p->getNext()==insn) break;
      p=p->getNext();
    }
  p->next = insn->next;
  delete insn;
}
