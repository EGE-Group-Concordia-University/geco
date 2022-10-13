// $Id: ECApp.cc 27 2014-03-16 17:58:36Z wuthrich $
// ---------------------------------------------------------------
//
// Definition of the class gecoApp
//
// (c) Rolf Wuthrich
//     2015 - 2020 Concordia University
//
// author:  Rolf Wuthrich
// email:   rolf.wuthrich@concordia.ca
// version: v1 ($Revision: 27 $)
//
// This software is copyright under the BSD license
//  
// ---------------------------------------------------------------
// history:
// ---------------------------------------------------------------
// Date       Modification                     Author
// ---------------------------------------------------------------
// 12.10.2015 Creation                         R. Wuthrich
// 21.11.2020 Added DOxygen documentation      R. Wuthrich
//
// ---------------------------------------------------------------

#define gecoVersion "1.0"

#include <tcl.h>
#include <cstring>
#include <iostream>
#include <tclreadline.h>
#include "gecoApp.h"
#include "gecoEvent.h"
#include "gecoProcess.h"
#include "gecoTcpServer.h"
#include "gecoIOModule.h"
#include "gecoPkgHandle.h"
#include "gecoTrigger.h"
#include "gecoUProc.h"
#include "gecoGraph.h"
#include "gecoFileStream.h"
#include "gecoMemStream.h"
#include "gecoIO.h"
#include "gecoIOSocket.h"
#include "gecoIOTcp.h"
#include "gecoClock.h"
#include "gecoTriangle.h"
#include "gecoSawtooth.h"
#include "gecoStep.h"
#include "gecoPulse.h"
#include "gecoEnd.h"

using namespace std;

// ------------------------------------------------------------
//
// geco license
//

/**
 * @brief geco license
 */
 
static CONST char* geco_license =
"   geCo the generic communication library\n"
"   Copyright (c) 2015-2021, Rolf Wuthrich <rolf.wuthrich@concordia.ca>\n"
"   All rights reserved.\n "
"   \n"
"   Redistribution and use in source and binary forms, with or without\n"
"   modification, are permitted provided that the following conditions\n"
"   are met:\n"
"   \n"
"     * Redistributions of source code must retain the above copyright\n"
"       notice, this list of conditions and the following disclaimer.\n"
"     * Redistributions in binary form must reproduce the above copyright\n"
"       notice, this list of conditions and the following disclaimer in the\n"
"       documentation and/or other materials provided with the distribution.\n"
"     * Neither the name of Rolf Wuthrich nor the names of contributors\n"
"       to this software may be used to endorse or promote products derived\n"
"       from this software without specific prior written permission.\n"
"       \n"
"   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS\n"
"   ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT\n"
"   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR\n"
"   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR\n"
"   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,\n"
"   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,\n"
"   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR\n"
"   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF\n"
"   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING\n"
"   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS\n"
"   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.";


// ------------------------------------------------------------
//
// New Tcl commands defined by gecoApp
//


/**
 * @brief Tcl 'start' command
 */

int geco_StartCmd(ClientData clientData, Tcl_Interp *interp, 
		  int objc,Tcl_Obj *const objv[])
{
  gecoApp* app=(gecoApp *)clientData;

  if (objc!=1)
    {
      Tcl_WrongNumArgs(interp,1,objv,NULL);
      return TCL_ERROR;
    }

  if (app->getEvent()->eventLoopStatus()==1)
    {
      Tcl_AppendResult(interp,
	         "can't start experiment; experiment is already running",NULL);
      return TCL_ERROR;
    }

  // loops over all processes in order to reset them in Waiting status
  gecoProcess* p=app->getFirstGecoProcess();
  while (p!=NULL)
    {
      p->setStatus(Waiting);
      p=p->getNextGecoProcess();
    }

  app->getEvent()->setCmdEvent(CmdStart,NULL);
  app->getEvent()->setEventLoopStatus(1);

  Tcl_Eval(interp, "cons \"+---------------------------------+\"");
  Tcl_Eval(interp, "cons \"|       Starting experiment       |\"");
  Tcl_Eval(interp, "cons \"+---------------------------------+\"");
  Tcl_Eval(interp, "cons \"|  [exec date]   |\"");
  Tcl_Eval(interp, "cons \"+---------------------------------+\"");
  return TCL_OK;
}


/**
 * @brief Tcl 'stop' command
 */

int geco_StopCmd(ClientData clientData, Tcl_Interp *interp, 
		 int objc,Tcl_Obj *const objv[])
{
  gecoApp* app=(gecoApp *)clientData;

  if (objc!=1)
    {
      Tcl_WrongNumArgs(interp, 1, objv, NULL);
      return TCL_ERROR;
    }

  app->getEvent()->setEventLoopStatus(0);
  app->getEvent()->setCmdEvent(CmdStop, NULL);
  return TCL_OK;
}


/**
 * @brief Tcl 'hold' command
 */

int geco_HoldCmd(ClientData clientData, Tcl_Interp *interp, 
		   int objc,Tcl_Obj *const objv[])
{
  gecoApp* app=(gecoApp *)clientData;

  if (objc!=1)
    {
      Tcl_WrongNumArgs(interp, 1, objv, NULL);
      return TCL_ERROR;
    }

  app->getEvent()->setCmdEvent(CmdHold, NULL);
  return TCL_OK;
}


/**
 * @brief Tcl 'resume' command
 */

int geco_ResumeCmd(ClientData clientData, Tcl_Interp *interp, 
		   int objc,Tcl_Obj *const objv[])
{
  gecoApp* app=(gecoApp *)clientData;

  if (objc!=1)
    {
      Tcl_WrongNumArgs(interp, 1, objv, NULL);
      return TCL_ERROR;
    }

  app->getEvent()->setCmdEvent(CmdResume, NULL);
  return TCL_OK;
}


/**
 * @brief Tcl 'ps' command
 */

int geco_PsCmd(ClientData clientData, Tcl_Interp *interp, 
	       int objc,Tcl_Obj *const objv[])
{
  gecoApp*     app=(gecoApp *)clientData;
  gecoProcess* p=app->getFirstGecoProcess();

  char str[100];
  char user_name[15];

  if (objc==1)
    {
      Tcl_AppendResult(interp,
		       "OWNER     PID  PROCESS NAME         ",
                       "CMD          STATUS      COMMENT\n",
		       NULL);
      while (p!=NULL)
	{
	  sprintf(str,"::geco::%s", p->getProcessOwner());
	  if (Tcl_GetVar(interp, str, 0)!=NULL)
	    strncpy(user_name, Tcl_GetVar(interp, str, 0), 9);
	  else
	    strncpy(user_name, p->getProcessOwner(), 9);
	  user_name[9]='\0';

	  sprintf(str,"%-9s %-3s  %-20s %-12s %-12s",
		  user_name,
		  p->getID(),
		  p->getGecoObjName(),
		  p->getTclCmd(),
		  p->getStatusStr());
	  Tcl_AppendResult(interp, str, NULL);
	  strncpy(user_name,Tcl_DStringValue(p->getComment()), 15);
	  if (strlen(Tcl_DStringValue(p->getComment()))<=15)
	    sprintf(str,"%-15s\n", user_name);
	  else
	    sprintf(str,"%-15s...\n", user_name);
	  Tcl_AppendResult(interp, str, NULL);
	  p=p->getNextGecoProcess();
	}
      return TCL_OK;
    }

  int index;
  static CONST char* cmds[] = {"-list", "-cmd", "-move", NULL};
  if (Tcl_GetIndexFromObj(interp, objv[1], cmds, "subcommand", '0', &index)!=TCL_OK)
    return TCL_ERROR;

  switch (index)
    {

    case 0: // -list
      if (objc>2)
	{
	  Tcl_WrongNumArgs(interp, 2, objv, "");
	  return TCL_ERROR;
	}
      if (p!=NULL) p=p->getNextGecoProcess();
      while (p!=NULL)
	{
	  sprintf(str,"%s ",p->getTclCmd());
	  Tcl_AppendResult(interp, str, NULL);
	  p=p->getNextGecoProcess();
	}
      break;

    case 1: //-cmd
      if (objc!=3)
	{
	  Tcl_WrongNumArgs(interp, 2, objv, "PID");
	  return TCL_ERROR;
	}
      p=app->findGecoProcess(Tcl_GetString(objv[2]));
      if (p==NULL)
	{
	  Tcl_AppendResult(interp, "invalid process ID", NULL);
	  return TCL_ERROR;
	}
      sprintf(str,"%s",p->getTclCmd());
      Tcl_AppendResult(interp, str, NULL);
      break;

    case 2: //-move
      if (objc!=5)
      	{
      	  Tcl_WrongNumArgs(interp, 2, objv, "PID after PID");
      	  return TCL_ERROR;
      	}
      
      if (app->moveGecoProcess(Tcl_GetString(objv[2]), Tcl_GetString(objv[4]))==-1)
	{
	  Tcl_AppendResult(interp, "invalid process IDs", NULL);
	  return TCL_ERROR;
	}
      break;

    }

  return TCL_OK;
}


/**
 * @brief Tcl 'lsiomod' command
 */

int geco_lsiomodCmd(ClientData clientData, Tcl_Interp *interp, 
		    int objc,Tcl_Obj *const objv[])
{
  gecoApp*      app=(gecoApp *)clientData;
  gecoIOModule* m=app->getFirstGecoIOModule();

  char str[100];

  Tcl_AppendResult(interp, "MODID  MODULE NAME         CMD\n", NULL);
  while (m!=NULL)
    {
      sprintf(str, "%-6s %-18s  %-20s\n",
		  m->getID(),
		  m->getGecoObjName(),
		  m->getTclCmd());
      Tcl_AppendResult(interp, str, NULL);
      m=m->getNextGecoIOModule();
    }
  return TCL_OK;
}


/**
 * @brief Tcl 'lsgecopkg' command
 */

int geco_lsgecopkgCmd(ClientData clientData, Tcl_Interp *interp, 
		      int objc,Tcl_Obj *const objv[])
{
  gecoApp*       app = (gecoApp *)clientData;
  gecoPkgHandle* p   = app->getFirstGecoPkgHandle();

  char str[100];

  Tcl_AppendResult(interp, "GECO PACKAGE NAME    LIBRARY FILE\n", NULL);
  while (p!=NULL)
    {
      sprintf(str, "%-20s %-50s\n", p->getPackageName(), p->getLibFile());
      Tcl_AppendResult(interp, str, NULL);
      p=p->getNextGecoPkgHandle();
    }
  return TCL_OK;
}


/**
 * @brief Tcl 'lstcpserver' command
 */

int geco_lstcpserverCmd(ClientData clientData, Tcl_Interp *interp, 
		      int objc,Tcl_Obj *const objv[])
{
  gecoApp*       app = (gecoApp *)clientData;
  gecoTcpServer* p   = app->getFirstGecoTcpServer();

  char str[100];

  Tcl_AppendResult(interp, "PORT     NBR CONNECTED CLIENTS  TCL COMMAND\n", NULL);
  while (p!=NULL)
    {
      sprintf(str,"%-8d %-22d %s\n", p->getPort(), p->getNbrClients(), p->getTclCmd());
      Tcl_AppendResult(interp, str, NULL);
      p=p->getNextGecoTcpServer();
    }
  return TCL_OK;
}


/**
 * @brief Tcl 'terminate' command
 */

int geco_TerminateCmd(ClientData clientData, Tcl_Interp *interp, 
		      int objc,Tcl_Obj *const objv[])
{
  gecoApp* app=(gecoApp *)clientData;

  if (objc!=2)
    {
      Tcl_WrongNumArgs(interp, 1, objv, "processID");
      return TCL_ERROR;
    }

  gecoProcess* p=app->findGecoProcess(Tcl_GetString(objv[1]));
  if (p==NULL)
    {
      Tcl_AppendResult(interp, "invalid process ID", NULL);
      return TCL_ERROR;
    }
  if (p->getStatus()!=Active)
    {
      Tcl_AppendResult(interp, "process not active; can't be terminated", NULL);
      return TCL_ERROR;
    }
  p->terminate(app->getEvent());
  //app->getEvent()->setCmdEvent(CmdProcessTerminated, NULL);
  return TCL_OK;
}


/**
 * @brief Tcl 'remove' command
 */

int geco_RemoveCmd(ClientData clientData, Tcl_Interp *interp,
		   int objc,Tcl_Obj *const objv[])
{
  gecoApp* app=(gecoApp *)clientData;

  if (objc!=2)
    {
      Tcl_WrongNumArgs(interp, 1, objv, "processID");
      return TCL_ERROR;
    }

  gecoProcess* p=app->findGecoProcess(Tcl_GetString(objv[1]));

  if (p==NULL)
    {
      Tcl_AppendResult(interp, "invalid process ID", NULL);
      return TCL_ERROR;
    }

  if (app->getEvent()->eventLoopStatus()==1)
    {
      Tcl_AppendResult(interp,
		 "can't remove a process while an experiment is running", NULL);
      return TCL_ERROR; 
    }

  if (strcmp(p->getProcessOwner(),"system")==0)
    {
      Tcl_AppendResult(interp, "can't remove a system process", NULL);
      return TCL_ERROR;     
    }
      
  app->removeGecoProcess(p);
  return TCL_OK;
}


/**
 * @brief Dispatches the geCo events
 * @param clientData a pointer to the instance of gecoApp
 *
 * A Tcl_TimerProc used to schedule the geCo event loop inside 
 * the Tcl event loop.
 *
 * The Tcl_TimerProc register itself again continuously with the function
 * Tcl_CreateTimerHandler() from the Tcl C API.
 * The time interval used for registration is the tick value
 * of the gecoClock, which must be the first gecoProcess in the 
 * geco process loop.
*/

void geco_eventLoop(ClientData clientData)
{
  gecoApp* app = (gecoApp *)clientData;
  gecoProcess* p = app->getFirstGecoProcess();
  while (p!=NULL)
    {
      p->handleEvent(app->event);
      p = p->getNextGecoProcess();
    }
  app->event->reset();
  gecoClock* clk = (gecoClock *)app->getFirstGecoProcess();
  Tcl_CreateTimerHandler(clk->tick, geco_eventLoop, clientData);
}


/**
 * @brief A Tcl_ExitProc called on exit of Tcl
 * @param clientData a pointer to the instance of gecoApp
 *
 * Performs required cleanup operations when exiting the gecoApp.
 *
 * Any cleanup operations needed to be done after deletion
 * of an instance of a child of gecoAPP needs to done in a separate
 * Tcl_ExitProc to be setup by Tcl_CreateExitHandler
*/

void geco_exitHandler(ClientData clientData)
{
  gecoApp* app=(gecoApp *)clientData;

  // loops over all gecopPocesses in order to delete them
  gecoProcess* p1=app->getFirstGecoProcess();
  gecoProcess* p2;
  while (p1!=NULL)
    {
      p2=p1->getNextGecoProcess();
      delete p1;
      p1=p2;
    }

  // loops over all gecoIOModules in order to delete them
  gecoIOModule* m1=app->getFirstGecoIOModule();
  gecoIOModule* m2;
  while (m1!=NULL)
    {
      m2=m1->getNextGecoIOModule();
      delete m1;
      m1=m2;
    }

  // loops over all gecoPkgHandles in order to delete them
  gecoPkgHandle* h1=app->getFirstGecoPkgHandle();
  gecoPkgHandle* h2;
  while (h1!=NULL)
    {
      h2=h1->getNextGecoPkgHandle();
      h1->unloadRegisteredTclCmds();
      delete h1;
      h1=h2;
    }
	
  // loops over all gecoTcpServer in order to delete them
  gecoTcpServer* s1=app->getFirstGecoTcpServer();
  gecoTcpServer* s2;
  while (s1!=NULL)
    {
      s2=s1->getNextGecoTcpServer();
      delete s1;
      s1=s2;
    }

  delete app->getEvent();

  cout <<"bye\n";
}


// ----------------------------------------------------------------------
//
// class GECOAPP : An application based on geco library
//

/**
 * @brief Constructor
 * @param argc number of arguments from main function
 * @param argv arguments from main function
 *
 * The constructor will create a Tcl interpreter and initialize Tcl. 
 * It creates the ::geco namespace and loads global variables from geco
 * to tis Tcl interpreter. It further loads the geco command set and 
 * loads a gecoClock as first gecoProcess in the geco process loop.
 * It loads as well the system wide configuration file of geco: 
 * /usr/local/etc/geco/geco.gecorc.tcl
 * and the Tcl programmed part of geco
 * /usr/local/share/geco/gecolib.tcl.
*/

gecoApp::gecoApp(int argc, char **argv)
{
  Tcl_FindExecutable(argv[0]);

  interp = Tcl_CreateInterp();
  Tcl_Preserve((ClientData) interp);
  Tcl_SetVar(interp, "tcl_interactive", "1", TCL_GLOBAL_ONLY);

  Tcl_Init(interp);
  Tk_SafeInit(interp);

  registerNewCmd();
  registerGlobalVars();

  event              = new gecoEvent(this);
  gecoClock* clk     = new gecoClock(this);;
  firstGecoProcess   = clk;
  firstGecoIOModule  = NULL;
  firstGecoPkgHandle = NULL;
  firstGecoTcpServer = NULL;

  Tcl_EvalFile(interp, "//usr//local//share//geco//gecolib.tcl");
  Tcl_EvalFile(interp, "//usr//local//etc//geco//geco.gecorc.tcl");
  //if (argc==2) Tcl_EvalFile(interp, argv[1]);
  Tcl_CreateExitHandler(geco_exitHandler, this);
  Tcl_CreateTimerHandler(clk->getTick(), geco_eventLoop, this);
}


/**
 * @brief Destructor
 *
 * Note: the destructor is actually never called.
 * Cleanup operations must be done in geco_exitHandler
*/

gecoApp::~gecoApp()
{
}


/**
 * @brief Registers new Tcl commands specific to gecoApp
 * Called by the constructor gecoAPP::gecoApp
*/

void gecoApp::registerNewCmd()
{
  Tcl_CreateObjCommand(interp, "loadGecoPkg", geco_LoadGecoPkgCmd, 
		       (ClientData) this, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateObjCommand(interp, "unloadGecoPkg", geco_UnloadGecoPkgCmd, 
		       (ClientData) this, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateObjCommand(interp, "start", geco_StartCmd, 
		       (ClientData) this, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateObjCommand(interp, "hold",geco_HoldCmd, 
		       (ClientData) this, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateObjCommand(interp, "resume",geco_ResumeCmd, 
		       (ClientData) this, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateObjCommand(interp, "stop", geco_StopCmd, 
		       (ClientData) this, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateObjCommand(interp, "terminate", geco_TerminateCmd, 
                       (ClientData) this, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateObjCommand(interp, "remove", geco_RemoveCmd, 
                       (ClientData) this, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateObjCommand(interp, "ps", geco_PsCmd, 
		       (ClientData) this, (Tcl_CmdDeleteProc *) NULL);
			   
  Tcl_CreateObjCommand(interp, "tcpserver", geco_TcpServerCmd, 
		       (ClientData) this, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateObjCommand(interp, "iosocket", geco_IOSocketCmd, 
		       (ClientData) this, (Tcl_CmdDeleteProc *) NULL);
  
  Tcl_CreateObjCommand(interp, "iotcp", geco_IOTcpCmd, 
		       (ClientData) this, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateObjCommand(interp, "io", geco_IOCmd, 
		       (ClientData) this, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateObjCommand(interp, "lsiomod", geco_lsiomodCmd, 
		       (ClientData) this, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateObjCommand(interp, "lsgecopkg", geco_lsgecopkgCmd, 
		       (ClientData) this, (Tcl_CmdDeleteProc *) NULL);
			   
  Tcl_CreateObjCommand(interp, "lstcpserver", geco_lstcpserverCmd, 
		       (ClientData) this, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateObjCommand(interp, "trigger", geco_TriggerCmd, 
                       (ClientData) this, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateObjCommand(interp, "uproc", geco_UProcCmd, 
                       (ClientData) this, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateObjCommand(interp, "graph", geco_GraphCmd, 
                       (ClientData) this, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateObjCommand(interp, "filestream", geco_FileStreamCmd, 
                       (ClientData) this, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateObjCommand(interp, "memstream", geco_MemStreamCmd, 
                       (ClientData) this, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateObjCommand(interp, "triangle", geco_TriangleCmd, 
                       (ClientData) this, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateObjCommand(interp, "sawtooth", geco_SawtoothCmd, 
                       (ClientData) this, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateObjCommand(interp, "step", geco_StepCmd, 
                       (ClientData) this, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateObjCommand(interp, "pulse", geco_PulseCmd, 
                       (ClientData) this, (Tcl_CmdDeleteProc *) NULL);

  Tcl_CreateObjCommand(interp, "end", geco_EndCmd, 
                       (ClientData) this, (Tcl_CmdDeleteProc *) NULL);
}


/**
 * @brief Registers the global Tcl variables specific to gecoApp
 * Called by the constructor gecoAPP::gecoApp
*/

void gecoApp::registerGlobalVars()
{
  Tcl_CreateNamespace(interp, "geco", NULL, NULL);
  
  Tcl_LinkVar(interp,"::geco::license",
              (char *)&geco_license, TCL_LINK_STRING | TCL_LINK_READ_ONLY);

  Tcl_SetVar(interp,"::geco::version", gecoVersion, 0);
  
  commentStr=Tcl_Alloc(2);
  strcpy(commentStr, "#");
  Tcl_LinkVar(interp, "::geco::commentStr", (char *)&commentStr, TCL_LINK_STRING);

  prompt=Tcl_Alloc(7);
  strcpy(prompt, "geco> ");
  Tcl_LinkVar(interp, "::geco::prompt", (char *)&prompt, TCL_LINK_STRING);
}


/**
 * @brief Adds a new gecoProcess to the process list
 * @param proc gecoProcess to be added
*/

void gecoApp::addGecoProcess(gecoProcess* proc)
{
  gecoProcess* p=firstGecoProcess;

  while (p->getNextGecoProcess()!=NULL)
    p=p->getNextGecoProcess();
      
  p->setNextGecoProcess(proc);
  proc->setNextGecoProcess(NULL);
  Tcl_Eval(interp, "event generate . <<ProcessCreated>>");
}


/**
 * @brief Moves gecoProcess with ID PID1 after the gecoProcess with ID PID2
 * @param PID1 gecoProcess ID of process to be moved
 * @param PID2 gecoProcess ID of process after which PID1 has to be moved
 * \return Returns -1 if any error and 0 if no error
*/

int gecoApp::moveGecoProcess(const char* PID1, const char* PID2)
{
  gecoProcess* p1=findGecoProcess(PID1);
  gecoProcess* p2=findGecoProcess(PID2);

  // check validity of PIDs
  if ((p1==NULL)||(p2==NULL)) return -1;
  if (strcmp(p1->getProcessOwner(),"system")==0) return -1;

  // check if order is already correct
  if (p2->getNextGecoProcess()==p1) return 0;

  // finds process before PID1
  gecoProcess* p=firstGecoProcess;
  while (p->getNextGecoProcess()!=NULL)
    {
      if (p->getNextGecoProcess()==p1) break;
      p=p->getNextGecoProcess();
    }

  p ->setNextGecoProcess(p1->getNextGecoProcess());
  p1->setNextGecoProcess(p2->getNextGecoProcess());
  p2->setNextGecoProcess(p1);
  Tcl_Eval(interp, "event generate . <<ProcessMoved>>");
  return 0;
}


/**
 * @brief Removes a gecoProcess from the geco process loop 
 * @param proc gecoProcess to be removed
 * The process will be removed from the geco process loop, but not be deleted
*/

void gecoApp::removeGecoProcess(gecoProcess* proc)
{
  gecoProcess* p=firstGecoProcess;
  while (p->getNextGecoProcess()!=NULL)
    {
      if (p->getNextGecoProcess()==proc) break;
      p=p->getNextGecoProcess();
    }
  p->setNextGecoProcess(proc->getNextGecoProcess());
  delete proc;
  Tcl_Eval(interp, "event generate . <<ProcessDeleted>>");
}


/**
 * @brief Finds and returns the gecoProcess in the geco process loop with ID PID
 * @param PID gecoProcess ID
 * \return returns a pointer to the gecoProcess of ID PID or NULL if the process was not found
*/

gecoProcess* gecoApp::findGecoProcess(const char* PID)
{
  gecoProcess* p=firstGecoProcess;
  while (p!=NULL)
    {
      if (strcmp(p->getID(),PID)==0) break;
      p=p->getNextGecoProcess();
    }
  return p;
}


/**
 * @brief Runs the geco process loop and implements a simple command line interface
*/

void gecoApp::run()
{
  Tcl_Channel inChannel  = Tcl_GetStdChannel(TCL_STDIN);
  Tcl_Channel outChannel = Tcl_GetStdChannel(TCL_STDOUT);

  Tcl_DString cmdline;
  Tcl_DStringInit(&cmdline);
  Tcl_DString Dstr_prompt;
  Tcl_DStringInit(&Dstr_prompt);
  Tcl_DStringAppend(&Dstr_prompt,prompt, -1);
  Tcl_WriteChars(outChannel, Tcl_DStringValue(&Dstr_prompt), -1);

  Tcl_Flush(outChannel);

  while (1)
    {
      inChannel = Tcl_GetStdChannel(TCL_STDIN);

      Tk_DoOneEvent(TK_DONT_WAIT);
      Tcl_DoOneEvent(TCL_DONT_WAIT);
     
      if (Tcl_Gets(inChannel, &cmdline)<0)
	{
	  if (Tcl_InputBlocked(inChannel)) 
	    {
	      continue;
	    }
	}

      if (Tcl_CommandComplete(Tcl_DStringValue(&cmdline)))
	{
	  Tcl_RecordAndEval(interp,Tcl_DStringValue(&cmdline),0);
	  if (Tcl_GetStringResult(interp)!="")
	    {
	      Tcl_WriteChars(outChannel, Tcl_GetStringResult(interp) , -1);
	      Tcl_WriteChars(outChannel, "\n", 1);
	    }

	  Tcl_DStringInit(&cmdline);
	  Tcl_DStringInit(&Dstr_prompt);
	  Tcl_DStringAppend(&Dstr_prompt,prompt, -1);
	  Tcl_WriteChars(outChannel, Tcl_DStringValue(&Dstr_prompt), -1);
	  Tcl_Flush(outChannel);
	}
      else       // tcl command is not yet complete
	{
	  Tcl_WriteChars(outChannel, "\t", 1);
	  Tcl_Flush(outChannel);
	}
    }

  Tcl_DStringFree(&Dstr_prompt);
  Tcl_DStringFree(&cmdline);
}


/**
 * @brief Runs the geco process loop and implements a command line interface
 *        based on tclreadline
*/

void gecoApp::runCLI()
{
	Tcl_Eval(interp, "package require tclreadline");
	Tcl_Eval(interp, "tclreadline::Loop");
}


/**
 * @brief Adds a new gecoIOModule to the gecoApp
 * @param mod gecoIOModule to be added
*/

void gecoApp::addGecoIOModule(gecoIOModule* mod)
{
  gecoIOModule* m=firstGecoIOModule;

  if (m==NULL) 
    {
      firstGecoIOModule=mod;
      m=mod;
    }

  while (m->getNextGecoIOModule()!=NULL)
    m=m->getNextGecoIOModule();
      
  m->setNextGecoIOModule(mod);
  mod->setNextGecoIOModule(NULL);
}


/**
 * @brief Removes a gecoIOModule from the internal list of the gecoApp
 * @param mod gecoIOModule to be removed fromt he intern_list of the gecoApp
 * The method does not delete the gecoIOModule
*/

void gecoApp::removeGecoIOModule(gecoIOModule* mod)
{
  gecoIOModule* m=firstGecoIOModule;

  if (m==mod)
    {
      firstGecoIOModule=NULL;
      return;
    }

  while (m->getNextGecoIOModule()!=NULL)
    {
      if (m->getNextGecoIOModule()==mod) break;
      m=m->getNextGecoIOModule();
    }
  m->setNextGecoIOModule(m->getNextGecoIOModule());
}


/**
 * @brief Finds and return the gecoIOModule associated to a Tcl command
 * @param cmd Tcl command for which one searches the gecoIOModule
 * \return the gecoIOModule associated to the Tcl command cmd and NULL if no module was found
*/

gecoIOModule* gecoApp::findGecoIOModule(const char* cmd)
{
  gecoIOModule* m=firstGecoIOModule;
  while (m!=NULL)
    {
      if (strcmp(m->getTclCmd(), cmd)==0) break;
      m=m->getNextGecoIOModule();
    }
  return m;
}


/**
 * @brief Adds a new gecoPkgHandle to the internal list of the gecoApp
 * @param handle Handle of the gecoPackage to be added
*/

void gecoApp::addGecoPkgHandle(gecoPkgHandle* handle)
{
  gecoPkgHandle* h=firstGecoPkgHandle;

  if (h==NULL) 
    {
      firstGecoPkgHandle=handle;
      return;
    }

  while (h->getNextGecoPkgHandle()!=NULL)
    h=h->getNextGecoPkgHandle();
      
  h->setNextGecoPkgHandle(handle);
}


/**
 * @brief Removes a gecoPkgHandle from the internal list of the gecoApp
 * @param handle Handle of the gecoPackage to be removed
 * This method does not delete the gecoPkgHandle
*/

void gecoApp::removeGecoPkgHandle(gecoPkgHandle* handle)
{
  gecoPkgHandle* h=firstGecoPkgHandle;
  if (h==handle)
    {
      firstGecoPkgHandle=handle->getNextGecoPkgHandle();
      return;
    }
  while (h->getNextGecoPkgHandle()!=NULL)
    {
      if (h->getNextGecoPkgHandle()==handle) break;
      h=h->getNextGecoPkgHandle();
    }
  h->setNextGecoPkgHandle(handle->getNextGecoPkgHandle());
}


/**
 * @brief Finds a gecoPkgHandle in the internal list of the gecoApp based on its name
 * @param pkgName Name of the gecoPackage one wants to find
 * \return pointer to the gecoPkgHandle of the geco-package with name Name 
 *         or NULL if the package was not found 
*/

gecoPkgHandle* gecoApp::findGecoPkgHandle(const char* pkgName)
{
  gecoPkgHandle* h=firstGecoPkgHandle;
  while (h!=NULL)
    {
      if (strcmp(h->getPackageName(),pkgName)==0) break;
      h=h->getNextGecoPkgHandle();
    }
  return h;
}


/**
 * @brief Adds a gecoTcpServer to the internal list of the gecoApp
 * @param srv gecoTcpServer to be added to the internal list
*/

void gecoApp::addGecoTcpServer(gecoTcpServer* srv)
{
  gecoTcpServer* p=firstGecoTcpServer;

  if (p==NULL) 
    {
      firstGecoTcpServer=srv;
      return;
    }

  while (p->getNextGecoTcpServer()!=NULL)
    p=p->getNextGecoTcpServer();
      
  p->setNextGecoTcpServer(srv);
}


/**
 * @brief Removes a gecoTcpServer from the internal list of the gecoApp
 * @param srv gecoTcpServer to be removed from the internal list
 * The method does not delete the gecoTcpServer
*/

void gecoApp::removeGecoTcpServer(gecoTcpServer* srv)
{
  gecoTcpServer* p=firstGecoTcpServer;
  if (p==srv)
    {
      firstGecoTcpServer=srv->getNextGecoTcpServer();
      return;
    }
  while (p->getNextGecoTcpServer()!=NULL)
    {
      if (p->getNextGecoTcpServer()==srv) break;
      p=p->getNextGecoTcpServer();
    }
  p->setNextGecoTcpServer(srv->getNextGecoTcpServer());
}


/**
 * @brief Checks if a gecoTcpServer is in the internal list of the gecoApp
 * @param srv gecoTcpServer to be checked for in the internal list
 * \return 1 if srv is in the internal list and 0 if srv is not in the internal list 
*/

int gecoApp::gecoServerExist(gecoTcpServer* srv)
{
  gecoTcpServer* p=firstGecoTcpServer;
  while (p)
    {
      if (p==srv) return 1;
      p=p->getNextGecoTcpServer();
    }
  return 0;
}
