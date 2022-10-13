// $Id: ECIOModule.cc 37 2015-01-09 12:35:29Z wuthrich $
// ---------------------------------------------------------------
//
// Definition of the class gecoIOModule
//
// (c) Rolf Wuthrich
//     2015-2021 Concordia University
//
// author:    Rolf Wuthrich
// email:     rolf.wuthrich@concordia.ca
// version:   v2 ($Revision: 37 $)
//
// This software is copyright under the BSD license
//  
// ---------------------------------------------------------------
// history:
// ---------------------------------------------------------------
// Date       Modification                     Author
// ---------------------------------------------------------------
// 17.10.2015 Creation                         R. Wuthrich
// 10.11.2020 General update                   R. Wuthrich
// 30.01.2020 Added doxygen documentation      R. Wuthrich
// 29.05.2021 Major revision                   R. Wuthrich
// ---------------------------------------------------------------

#include <tcl.h>
#include <cstring>
#include "gecoIOModule.h"
#include "gecoApp.h"

using namespace std;


// -----------------------------------------------------------------------
//
// Class to store linked Tcl variables 
//

// ---- CONSTRUCTOR
//

IOModuleInsn::IOModuleInsn(const char* Tcl_Var, int tclVarType, int ChanID)
{
  TclVar = new Tcl_DString;
  Tcl_DStringInit(TclVar);
  Tcl_DStringAppend(TclVar, Tcl_Var, -1);
  TclVarType = tclVarType;
  chanID = ChanID;
  next = NULL;
}


// ---- DESTRUCTOR
//

IOModuleInsn::~IOModuleInsn()
{
  Tcl_DStringFree(TclVar);
  delete TclVar;
}


// ---------------------------------------------------------------
//
// class gecoIOModule : abstract class for geco IO-Modules 
//

/**
 * @brief Constructor
 * @param moduleName Name of the geco IO-module
 * @param moduleCmd  Tcl command of the geco IO-module
 * @param App gecoApp in which the gecoIOModule lives 
 *
 * The constructor will create a Tcl command via the call of
 * the constructor of gecoObj. It further defines additional
 * subcommands and a Tcl name-space having the same name as moduleCmd.
*/

gecoIOModule::gecoIOModule(const char* moduleName, const char* moduleCmd,
			   gecoApp* App) :
  gecoObj(moduleName, moduleCmd, App, false)
{
  setUnlinkedToGeco();
  firstIOModuleInsn = NULL;
  nextGecoIOModule = NULL;
  App->addGecoIOModule(this);

  // creates associated Tcl Namespace
  TclNamespace=Tcl_CreateNamespace(App->getInterp(), moduleCmd, NULL, NULL);

  // define Tcl subcommands
  addOption("-listIOoperations", "list scheduled IO operations");
  addOption("-doIOoperations", "executes all scheduled IO operations");
  addOption("-update", "executes the IO operation associated to a Tcl variable");
  addOption("-unlinkTclVariable", "unlinks a Tcl variable");
  addOption("-close", "closes the io-module");
}


/**
 * @brief Destructor
 *
 * Will remove the Tcl name-space and all linked IOModuleInsn
*/

gecoIOModule::~gecoIOModule()
{
  // removes associated Tcl Namespace (Tcl command is removed by gecoObj)
  Tcl_DeleteNamespace(TclNamespace);

  app->removeGecoIOModule(this);
  IOModuleInsn* p = firstIOModuleInsn;
  while (firstIOModuleInsn)
    {
      p=firstIOModuleInsn;
      firstIOModuleInsn = firstIOModuleInsn->next;
      delete p;
    }
}


/*!
 * @copydoc gecoObj::cmd 
 *
 * Compared to gecoObj::cmd, gecoIOModule::cmd adds the processing of
 * the new subcommands of gecoIOModule.
 */

int gecoIOModule::cmd(int &i, int objc, Tcl_Obj *const objv[])
{
  // first executes the command options defined in gecoObj
  int index = gecoObj::cmd(i, objc, objv);

  int n;
  char str[TCL_DOUBLE_SPACE];

  if (index==getOptionIndex("-listIOoperations"))
    {
      listInstr();
      i++;
    }

  if (index==getOptionIndex("-doIOoperations"))
    {
      n=doInstr();
      if (n<0)
	{
	  IOerror();
	  return -1;
	}
      Tcl_PrintDouble(interp, n, str);
      Tcl_AppendResult(interp, str, NULL);
      i++;
    }

  if (index==getOptionIndex("-update"))
    {
      if (i+1>=objc)
	{
	  Tcl_WrongNumArgs(interp, i+1, objv, "TclVar");
	  return -1;
	}
      n=update(Tcl_GetString(objv[i+1]));
      if (n<0)
	{
	  IOerror();
	  return -1;
	}
      if (n==0)
	{
	  Tcl_AppendResult(interp, "The Tcl variable \"",
			   Tcl_GetString(objv[i+1]),
		"\" is not linked to any IO operation on this module", NULL);
	  return -1;
	}
      Tcl_PrintDouble(interp, n, str);
      Tcl_AppendResult(interp, str, NULL);
      i = i+2;
    }

  if (index==getOptionIndex("-unlinkTclVariable"))
    {
      if (i+1>=objc)
      	{
      	  Tcl_WrongNumArgs(interp, i+1, objv,"Tcl_Variable");
      	  return -1;
      	}
      IOModuleInsn* p=findLinkedTclVariable(Tcl_GetString(objv[i+1]));
      if (p==NULL)
	{
	  Tcl_AppendResult(interp, "variable \"", Tcl_GetString(objv[i+1]),
			          "\" is not linked to any instruction", NULL);
	  return -1;
	}
      removeInsn(p);
      i = i+2;
    }

  if (index==getOptionIndex("-close"))
    {
      if (linkedToGeco())
	{
	  Tcl_AppendResult(interp, "The module is linked to the geco process loop. First unlink the module with the \"io\" command.", NULL);
	  return -1;
	}
      i = objc;
      delete this;
    }

  return index;
}


/**
 * @copydoc gecoObj::activate
 */

void gecoIOModule::activate(gecoEvent* ev)
{
}


/**
 * @copydoc gecoObj::info
 */

Tcl_DString* gecoIOModule::info(const char* frontStr)
{
  gecoObj::info(frontStr);
  return infoStr;
}


/**
 * @brief Adds an instruction to the instruction list
 * @param insn IOModuleInsn to be added to the list 
*/

int gecoIOModule::addInsn(IOModuleInsn* insn)
{
  // adds the instruction
  IOModuleInsn* p = getFirstInsn();
  if (p)
    {
      while (p->getNext())
	  p = p->getNext();    
      p->next = insn;
    }
  else
    firstIOModuleInsn = insn;
  insn->next = NULL;
  
  return TCL_OK;
}


/**
 * @brief Removes an instruction from the instruction list
 * @param insn IOModuleInsn to be removed from the list 
*/

void gecoIOModule::removeInsn(IOModuleInsn* insn)
{
  if (insn==getFirstInsn())
    {
      firstIOModuleInsn = insn->getNext();
      delete insn;
      return;
    }

  IOModuleInsn* p = getFirstInsn();
  while(p)
    {
      if (p->getNext()==insn) break;
      p=p->getNext();
    }
  p->next = insn->next;
  delete insn;
}


/**
 * @brief Lists the IO instruction list
 *
 * Must be defined by child
*/

void gecoIOModule::listInstr()
{
}


/**
 * @brief Executes the IO instruction list
 *
 * Must be defined by child
 *
 * Must return the number of successful operations done
*/

int gecoIOModule::doInstr()
{
  return 0;
}


/**
 * @brief Executes the IO instruction associated to a Tcl variable
 * @param Tcl_Var Tcl variable associated to the IO instruction one wants to execute
 *
 * Must be defined by child
 *
 * Must return :
 * * 1 if successful
 * * 0 if the Tcl variable is not linked
 * * -1 if an error occurred during the IO operation
*/

int gecoIOModule::update(const char* Tcl_Var)
{
  return -1;
}


/**
 * @brief To be called whenever an IO error occurs
 *
 * Must be defined by child if special action is required in case of an IO error
 */
 
void gecoIOModule::IOerror()
{
}


/**
 * @brief Finds the IO instruction associated to a Tcl variable
 * @param Tcl_Var Tcl variable associated to the IO instruction one wants to find
 *
*/

IOModuleInsn* gecoIOModule::findLinkedTclVariable(const char* TclVar)
{
  IOModuleInsn* p = getFirstInsn();
  while(p)
    {
      if (strcmp(Tcl_DStringValue(p->TclVar), TclVar)==0) break;
      p = p->getNext();
    }
  return p;
}
