// $Id: ECIO.cc 37 2015-01-09 12:35:29Z wuthrich $
// ---------------------------------------------------------------
//
// Definition of the class gecoIO
//
// (c) Rolf Wuthrich
//     2015 - 2020 Concordia University
//
// author:  Rolf Wuthrich
// email:   rolf.wuthrich@concordia.ca
// version: v1 ($Revision: 37 $)
//
// This software is copyright under the BSD license
//  
// ---------------------------------------------------------------
// history:
// ---------------------------------------------------------------
// Date       Modification                     Author
// ---------------------------------------------------------------
// 17.10.2015 Creation                         R. Wuthrich
// 08.12.2020 Added doxygen documentation      R. Wuthrich
//
// ---------------------------------------------------------------

#include <tcl.h>
#include <cstring>
#include "gecoEvent.h"
#include "gecoIO.h"
#include "gecoIOModule.h"

using namespace std;


// ------------------------------------------------------------
//
// Tcl interface
//


// Command to create a new io object
//

int geco_IOCmd(ClientData clientData, Tcl_Interp *interp, 
		    int objc,Tcl_Obj *const objv[])
{
  gecoIO* proc  = new gecoIO((gecoApp *)clientData);
  return geco_CreateGecoProcessCmd(proc, objc, objv);
}


// ---------------------------------------------------------------
//
// class gecoIO : class to link/unlink IOModules to geCo
//


/**
 * @brief Constructor
 * @param App gecoApp in which the gecoProcess lives 
 *
 * The constructor will create a Tcl command via the call of
 * the constructor of gecoObj. It further defines additional
 * subcommands.
*/

gecoIO::gecoIO(gecoApp* App) :
  gecoObj("IO Operations", "io", App),
  gecoProcess("IO Operations", "user", "io", App),
  firstGecoIOModule(NULL)
{
  activateOnStart=1;
  addOption("-linkModule", "links an IO-module");
  addOption("-unlinkModule", "unlinks an IO-module");
  addOption("-listLinkModules", "list linked IO-modules");
  addOption("-update", "updates all IO operations");
}


/**
 * @brief Destructor
*/

gecoIO::~gecoIO()
{
  linkedGecoIOModules* b1=firstGecoIOModule;
  linkedGecoIOModules* b2=firstGecoIOModule;
  while (b1!=NULL)
    {
      b2=b1;
      b1=b1->nextModule;
      delete b2;
    }
}


/*! 
 * @copydoc gecoProcess::cmd 
 *
 * Compared to gecoProcess::cmd, gecoIO::cmd adds the processing of
 * the new subcommands of gecoIO.
 */

int gecoIO::cmd(int &i, int objc,Tcl_Obj *const objv[])
{
  // first executes the command options defined in gecoProcess
  int index=gecoProcess::cmd(i,objc,objv);

  char str[80];
  gecoIOModule* iomodule;

  if (index==getOptionIndex("-linkModule"))
    {
      if (objc!=3)
	{
	  Tcl_WrongNumArgs(interp, 2, objv, "IOModule");
	  return -1;
	}
      iomodule=getGecoApp()->findGecoIOModule(Tcl_GetString(objv[2]));
      if (iomodule==NULL)
	{
	  snprintf(str, 80, "\"%s\" is not a valid module or not open",
		   Tcl_GetString(objv[2]));
	  Tcl_AppendResult(interp, str, NULL);
	  return -1;
	}
      if (addGecoIOModule(iomodule)==TCL_ERROR) return -1;
      i = objc;
    }

  if (index==getOptionIndex("-unlinkModule"))
    {
      if (objc!=3)
	{
	  Tcl_WrongNumArgs(interp, 2, objv, "IOModule");
	  return -1;
	}
      if (removeGecoIOModule(Tcl_GetString(objv[2]))==TCL_ERROR) return -1;
      i = objc;
    }

  if (index==getOptionIndex("-listLinkModules"))
    {
     if (objc!=2)
	{
	  Tcl_WrongNumArgs(interp, 2, objv, NULL);
	  return -1;
	}
     Tcl_DString* s;
     s=new Tcl_DString;
     Tcl_DStringInit(s);
     listGecoIOModules(s);
     Tcl_AppendResult(interp, Tcl_DStringValue(s), NULL);
     Tcl_DStringFree(s);
     delete s;
     i=objc;
    }

  if (index==getOptionIndex("-update"))
    {
      if (objc!=3)
	{
	  Tcl_WrongNumArgs(interp, 2, objv, "Tcl_Variable");
	  return -1;
	}

      linkedGecoIOModules* m=firstGecoIOModule;
      int ret;
      while (m!=NULL)
	{
	  ret=m->module->update(Tcl_GetString(objv[2]));
	  if (ret<0)
	    {
	      m->module->IOerror();
	      return -1;
	    }
	  if (ret==1) 
	    {
	      Tcl_PrintDouble(interp, ret, str);
	      Tcl_AppendResult(interp, str, NULL);
	      i = objc;
	      return index;
	    }
	  m=m->nextModule;
	}
      Tcl_AppendResult(interp, "The Tcl variable \"", Tcl_GetString(objv[2]),
           "\" is not linked to any IO operation on any linked IO-module", NULL);
      return -1;
    }

  return index;
}


/**
 * @copydoc gecoProcess::handleEvent
 *
 * In addition to gecoProcess::handleEvent, gecoIO::handleEvent implements
 * the calling of each linked gecoIOModule and requesting them to
 * execute the defined IO operations.
 */

void gecoIO::handleEvent(gecoEvent* ev)
{
  gecoProcess::handleEvent(ev);

  // if not running nothing more to do
  if (ev->eventLoopStatus()==0) return;

  // calls the IO module(s) 
  linkedGecoIOModules* b = firstGecoIOModule;
  while (b!=NULL)
    {
      if (b->module->doInstr()<0)
	{
	  b->module->IOerror();
	  return;
	}
      b=b->nextModule;
    }
}


/**
 * @copydoc gecoProcess::info
 *
 * In addition to gecoProcess::info, gecoGraph::info adds the information
 * about the linked gecoIOModule. (NOT YET IMPLEMENTED)
 */ 

Tcl_DString* gecoIO::info(const char* frontStr)
{
  gecoProcess::info(frontStr);
  //Tcl_DStringAppend(infoStr, "\nLinked IO-modules:\n", -1);
  Tcl_DStringAppend(infoStr, "\n", -1);
  listGecoIOModules(infoStr);
  return infoStr;
}


/**
 * @copydoc gecoProcess::activate
 *
 * In addition to gecoProcess::activate, gecoIO::activate 
 * will loop over all linked gecoIOModule and activate them
 * by calling their gecoIOModule::activate method.
 */

void gecoIO::activate(gecoEvent* ev)
{
  gecoProcess::activate(ev);

  linkedGecoIOModules* m=firstGecoIOModule;

  // checks if connected to any IO module
  if (m==NULL)
    Tcl_Eval(interp,"cons \"WARNING no IO module connected.\"");

  // calls the activate function of each module
  while (m!=NULL)
    {
      m->module->activate(ev);
      m = m->nextModule;
    }
}


/**
 * @brief Adds a gecoIOModule to the linked gecoIOModule
 * @param newModule gecoIOModule to be added
 * \return TCL_OK in case of success and TCL_ERROR otherwise
 *
 * If newModule was already inked will return TCL_ERROR and leave an error 
 * message in the Tcl interpreter run by the gecoApp in which the 
 * instance of gecoIO lives
 */

int gecoIO::addGecoIOModule(gecoIOModule* newModule)
{
  linkedGecoIOModules* m=firstGecoIOModule;

  while (m!=NULL)
    {
      if (m->module==newModule)
	{
	  Tcl_AppendResult(interp,
	     "module \"", newModule->getTclCmd(), "\" already added", NULL);
	  return TCL_ERROR;
	}
      if (m->nextModule==NULL) break;
      m = m->nextModule;
    } 

  linkedGecoIOModules* newM;
  newM = new linkedGecoIOModules();
  newM->module=newModule;
  newM->nextModule=NULL;
  newModule->setLinkedToGeco();
  if (m!=NULL) m->nextModule=newM; else firstGecoIOModule=newM;

  return TCL_OK;
}


/**
 * @brief Removes a gecoIOModule from the linked gecoIOModule
 * @param TclCmdOfModule name of gecoIOModule to be removed
 * \return TCL_OK in case of success and TCL_ERROR otherwise
 *
 * If TclCmdOfModule was never inked will return TCL_ERROR and leave an error 
 * message in the Tcl interpreter run by the gecoApp in which the 
 * instance of gecoIO lives
 */

int gecoIO::removeGecoIOModule(char* TclCmdOfModule)
{
  // finds the module to be removed
  linkedGecoIOModules* m=firstGecoIOModule;
  linkedGecoIOModules* prev=NULL;
  while (m!=NULL)
    {
      if (strcmp(TclCmdOfModule, m->module->getTclCmd())==0) break;
      prev = m;
      m = m->nextModule;
    }

  if (m==NULL)     // module is not linked
    {
      char str[100];
      sprintf(str, "module '%s' is not connected", TclCmdOfModule);
      Tcl_AppendResult(interp, str, NULL);
      return TCL_ERROR;
    }
	
  // removes the module
  if (prev==NULL)
    firstGecoIOModule = m->nextModule;
  else
    prev->nextModule = m->nextModule;
  m->module->setUnlinkedToGeco();
  delete m;
  return TCL_OK;
}


/**
 * @brief Lists the linked gecoIOModule
 * @param str Tcl_DString that will be filled with the list of linked gecoIOModul
 */

void gecoIO::listGecoIOModules(Tcl_DString* str)
{
  char s[100];
  linkedGecoIOModules* m = firstGecoIOModule;
  Tcl_DStringAppend(str, "TCL CMD         MODULE\n", -1);
  while (m!=NULL)
    {
      sprintf(s, "%-15s %-20s\n", m->module->getTclCmd(),
	      m->module->getGecoObjName());
      Tcl_DStringAppend(str, s, -1);
      m=m->nextModule;
    }
}
