// $Id: ECIOTcp.cc 37 2015-01-09 12:35:29Z wuthrich $
// ---------------------------------------------------------------
//
// Definition of the class gecoIOTcp
//
// (c) Rolf Wuthrich
//     2021 Concordia University
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
// 26.01.2021 Creation                         R. Wuthrich
// 29.05.2021 Major revision                   R. Wuthrich
// ---------------------------------------------------------------

#include "gecoIOSocket.h"
#include "gecoApp.h"
#include "gecoIO.h"
#include "gecoHelp.h"
#include <tcl.h>
#include <cstring>

using namespace std;


// ------------------------------------------------------------
//
// New Tcl command
//


// ---- Tcl iosocket command
//

int geco_IOSocketCmd(ClientData clientData, Tcl_Interp *interp, 
	          int objc,Tcl_Obj *const objv[])
{
  Tcl_ResetResult(interp);
  gecoApp*       app = (gecoApp *)clientData;
  gecoIOSocket*  sock;

  if (objc==1)
    {
      Tcl_WrongNumArgs(interp, 1, objv, "subcommand ?argument ...?");
      return TCL_ERROR;
    }

  int index;
  static CONST char* cmds[] = {"-help", "-open", NULL};
  static CONST char* help[] = {"creates a Tcl socket IO-module", NULL};

  if (Tcl_GetIndexFromObj(interp, objv[1], cmds, "subcommand", '0', &index)!=TCL_OK)
    return TCL_ERROR;

  switch (index)
    {

    case 0: // -help
      if (objc!=2)
	{
	  Tcl_WrongNumArgs(interp, 2, objv, NULL);
	  return TCL_ERROR;
	}
      gecoHelp(interp, "iosocket", "Tcl socket IO-module", cmds, help);
      break;

    case 1: // -open
      if (objc!=4)
	{
	  Tcl_WrongNumArgs(interp, 2, objv, "TclSocket cmdName");
	  return TCL_ERROR;
	}
	
	int modePtr;
	Tcl_Channel chan = Tcl_GetChannel(interp, Tcl_GetString(objv[2]), &modePtr);
	
	if (!chan) {
	  Tcl_AppendResult(interp, "\nTcl socket does not exist\n", NULL);
	  return TCL_ERROR;
	}
	
	/*if (modePtr != (TCL_WRITABLE) ) {
	  Tcl_AppendResult(interp, "Tcl socket is not a read/write channel\n", NULL);
      return TCL_ERROR;
	}*/
	
	if (Tcl_GetCommandInfo(interp, Tcl_GetString(objv[3]), NULL))
	  {
	    Tcl_AppendResult(interp, "Module already open or with identical name\n", NULL);
	    return TCL_ERROR;
	  }
	
	sock = new gecoIOSocket(chan, Tcl_GetString(objv[3]), app);

      break;

    }

  return TCL_OK;
}


// ---------------------------------------------------------------
//
// class SocketInsn
//


// ---- CONSTRUCTOR
//

SocketInsn::SocketInsn(const char* Tcl_Var, const char* insn, 
		 Tcl_Channel chanID, int Type, bool handshake) :
  IOModuleInsn(Tcl_Var, Type)
{
  SocketInstr = new Tcl_DString;
  Tcl_DStringInit(SocketInstr);
  Tcl_DStringAppend(SocketInstr, insn, -1);
  
  TclSocketInstr = new Tcl_DString;
  Tcl_DStringInit(TclSocketInstr);
  Tcl_DStringAppend(TclSocketInstr, "puts ", -1);
  Tcl_DStringAppend(TclSocketInstr, Tcl_GetChannelName(chanID), -1);
  Tcl_DStringAppend(TclSocketInstr," ", -1);
  Tcl_DStringAppend(TclSocketInstr, insn, -1);
  Tcl_DStringAppend(TclSocketInstr, "; flush ", -1);
  Tcl_DStringAppend(TclSocketInstr, Tcl_GetChannelName(chanID), -1);
  Tcl_DStringAppend(TclSocketInstr, "; ", -1);
  
  // if a handshake will be issued by socket needs to read the handshake:
  if (handshake)
  {
    Tcl_DStringAppend(TclSocketInstr, "gets ", -1);
    Tcl_DStringAppend(TclSocketInstr, Tcl_GetChannelName(chanID), -1);
    Tcl_DStringAppend(TclSocketInstr, "; flush ", -1);
    Tcl_DStringAppend(TclSocketInstr, Tcl_GetChannelName(chanID), -1);
    Tcl_DStringAppend(TclSocketInstr, "; ", -1);
  }
  
  if (Type==TclVarRead)
  {
    Tcl_DStringAppend(TclSocketInstr, "set ", -1);
    Tcl_DStringAppend(TclSocketInstr, Tcl_Var, -1);
    Tcl_DStringAppend(TclSocketInstr, " [gets ", -1);
    Tcl_DStringAppend(TclSocketInstr, Tcl_GetChannelName(chanID), -1);
    Tcl_DStringAppend(TclSocketInstr, "]; flush ", -1);
    Tcl_DStringAppend(TclSocketInstr, Tcl_GetChannelName(chanID), -1);
    Tcl_DStringAppend(TclSocketInstr, "; ", -1);
  }
  
  PostProcScript = new Tcl_DString;
  Tcl_DStringInit(PostProcScript);
}


// ---- DESTRUCTOR
//

SocketInsn::~SocketInsn()
{
  Tcl_DStringFree(SocketInstr);
  Tcl_DStringFree(TclSocketInstr);
  Tcl_DStringFree(PostProcScript);
  delete SocketInstr;
  delete TclSocketInstr;
  delete PostProcScript;
}


// ---------------------------------------------------------------
//
// class gecoIOSocket
//


// ---- CONSTRUCTOR
//

gecoIOSocket::gecoIOSocket(const char* modName, const char* moduleCmd, gecoApp* App) :
  gecoIOModule(modName, moduleCmd, App)
{
  handshake = false;
  transDelay = 0;
  
  addOption("-handshake", &handshake, "sets (ON/OFF) if socket replies with a handshake or not");
  addOption("-transmitDelay", &transDelay, "returns/sets transmission delay between sending/receiving data (ms)");
  addOption("-linkTclVariable", "links a Tcl variable");
  addOption("-postProcessScript", "set/returns post processing script on Tcl variable");
  addOption("-query", "sends a query to the socket and returns the answers");
  addOption("-write", "writes a command to the socket");
  addOption("-getSocketID", "returns Tcl socket ID");
}


gecoIOSocket::gecoIOSocket(Tcl_Channel socket, const char* moduleCmd, gecoApp* App) :
  gecoIOModule("Tcl socket IO-module", moduleCmd, App)
{
  handshake = false;
  transDelay = 0;
  
  addOption("-handshake", &handshake, "sets (ON/OFF) if socket replies with a handshake or not");
  addOption("-transmitDelay", &transDelay, "returns/sets transmission delay between sending/receiving data (ms)");
  addOption("-linkTclVariable", "links a numerical Tcl variable");
  addOption("-postProcessScript", "set/returns post processing script on numerical Tcl variable");
  addOption("-query", "sends a query to the socket and returns the answers");
  addOption("-write", "writes a command to the socket");
  addOption("-getSocketID", "returns Tcl socket ID");
  
  chanID = socket;
}


// ---- DESTRUCTOR
//

gecoIOSocket::~gecoIOSocket()
{
  if (chanID) Tcl_UnregisterChannel(interp, chanID);
}


/*!
 * @copydoc gecoObj::cmd 
 *
 * Compared to gecoObj::cmd, gecoIOSocket::cmd adds the processing of
 * the new subcommands of gecoIOSocket.
 */

int gecoIOSocket::cmd(int &i, int objc,Tcl_Obj *const objv[])
{
  // first executes the command options defined in gecoIOModule
  int index=gecoIOModule::cmd(i, objc, objv);

  if (index==getOptionIndex("-getSocketID"))
    {
      Tcl_AppendResult(interp, Tcl_GetChannelName(getTclChannel()), NULL);
      i++;
    }

  if (index==getOptionIndex("-linkTclVariable"))
    {
      if (i+3>=objc)
      	{
      	  Tcl_WrongNumArgs(interp, i+1, objv, "Tcl_Variable Type Instruction");
      	  return -1;
      	}
      int type;
      if (strcmp(Tcl_GetString(objv[i+2]), "read")==0) type=TclVarRead;
	else
	  if (strcmp(Tcl_GetString(objv[i+2]), "write")==0) type=TclVarWrite;
	    else
	      {
		Tcl_AppendResult(interp, "wrong variable type \"",
				 Tcl_GetString(objv[i+2]),
				 "\": must be \"read\" or \"write\"",NULL);
		return -1;
	      }

      // creates a new entry and links it
      SocketInsn* isn = new SocketInsn(Tcl_GetString(objv[i+1]),
				 Tcl_GetString(objv[i+3]),
				 getTclChannel(), type, handshake);
      if (addInsn(isn)==TCL_ERROR) 
	{
	  delete isn;
	  return -1;
	}
      i = i+4;
    }

  if (index==getOptionIndex("-postProcessScript"))
    {
      if (i+1>=objc)
      	{
      	  Tcl_WrongNumArgs(interp, i+1, objv, "Tcl_Variable ?Post_Process_Script?");
      	  return -1;
      	}
      SocketInsn* p = findLinkedTclVariable(Tcl_GetString(objv[i+1]));
      if (p==NULL)
      	{
	  Tcl_AppendResult(interp, "variable \"", Tcl_GetString(objv[i+1]),
		       "\" is not linked to any instruction",NULL);
      	  return -1;
      	}

      if ((i+3<=objc)&&(Tcl_StringMatch(Tcl_GetString(objv[i+2]), "-*")==0))
	{
	  Tcl_DStringFree(p->PostProcScript);
	  Tcl_DStringAppend(p->PostProcScript, Tcl_GetString(objv[i+2]), -1);
	  i=i+3;
	}
      else
	{
	  Tcl_AppendResult(interp, Tcl_DStringValue(p->PostProcScript), NULL);
	  i = i+2;
	}
    }
	
   if (index==getOptionIndex("-query"))
    {
      if (objc!=3)
      	{
      	  Tcl_WrongNumArgs(interp,i+1, objv, "queryCmd");
      	  return -1;
      	}
	  query(Tcl_GetString(objv[i+1]));
	  i = i+2;
	}
	
   if (index==getOptionIndex("-write"))
    {
      if (objc!=3)
      	{
      	  Tcl_WrongNumArgs(interp,i+1, objv, "cmdTowrite");
      	  return -1;
      	}
	  write(Tcl_GetString(objv[i+1]));
	  i = i+2;
	}

  return index;
}


// ---- LISTINSTR : lists the IO instruction list
//

void gecoIOSocket::listInstr()
{
  char str[100];
  Tcl_AppendResult(interp, "NUM  OPERATION  TCL VARIABLE  INSTRUCTION       POST PROCESS\n", NULL);
  SocketInsn* p=getFirstInsn();
  int i = 1;

  while (p)
    {
      if (p->getVarType()==TclVarRead)
      	sprintf(str, "%-4d read       %-13s %-17s %s\n", i,
                Tcl_DStringValue(p->TclVar),
				Tcl_DStringValue(p->SocketInstr),
                Tcl_DStringValue(p->PostProcScript));	
      else
      	sprintf(str, "%-4d write      %-13s %-17s %s\n", i,
                Tcl_DStringValue(p->TclVar),
				Tcl_DStringValue(p->SocketInstr),
                Tcl_DStringValue(p->PostProcScript));
      Tcl_AppendResult(interp, str, NULL);
      i++;
      p=p->getNext();
    }

}


// ---- DOINSTR : executes the IO instruction list
//
//      returns number of successful instructions done
//

int gecoIOSocket::doInstr()
{
  SocketInsn* p=getFirstInsn();
  int i=0;
  while (p)
    {
      i++;
      Tcl_Eval(interp, Tcl_DStringValue(p->TclSocketInstr));
      p=p->getNext();
    }

  p=getFirstInsn();
  while (p)
    {
      Tcl_Eval(interp, Tcl_DStringValue(p->PostProcScript));
      Tcl_ResetResult(interp);
      p=p->getNext(); 
    }
  return i;
}


// ---- update : executes the IO instruction associated to the Tcl_Var
//
//      return  1 if successful
//              0 if Tcl_Var is not linked
//             -1 if an error occurred
//

int gecoIOSocket::update(const char* Tcl_Var)
{
  SocketInsn* p = findLinkedTclVariable(Tcl_Var);
  if (p==NULL) return 0;

  Tcl_Eval(interp, Tcl_DStringValue(p->TclSocketInstr));
  Tcl_Flush(chanID);
  Tcl_Eval(interp, Tcl_DStringValue(p->PostProcScript));
  Tcl_ResetResult(interp);
  return 1;
}


/**
 * @brief Sends a query to the socket and reads answer
 * @param queryInsn query to be written to the socket
*/

void gecoIOSocket::query(const char* queryInsn)
{
  /*char str[10];
  sprintf(str, "%d", transDelay);
  
  // creates Tcl command to be sent to socket
  Tcl_DString* Tclcmd = new Tcl_DString;
  Tcl_DStringInit(Tclcmd);
  Tcl_DStringAppend(Tclcmd, "puts ", -1);
  Tcl_DStringAppend(Tclcmd, Tcl_GetChannelName(chanID), -1);
  Tcl_DStringAppend(Tclcmd," ", -1);
  Tcl_DStringAppend(Tclcmd, queryInsn, -1);
  
  // if a handshake will be issued by socket needs to read the handshake:
  if (handshake)
  {
    Tcl_DStringAppend(Tclcmd, "; flush ", -1);
    Tcl_DStringAppend(Tclcmd, Tcl_GetChannelName(chanID), -1);
	if (transDelay) 
	{
	  Tcl_DStringAppend(Tclcmd, "; after ", -1);
	  Tcl_DStringAppend(Tclcmd, str, -1);
	  Tcl_DStringAppend(Tclcmd, "; gets ", -1);
      Tcl_DStringAppend(Tclcmd, Tcl_GetChannelName(chanID), -1);
	}
    Tcl_DStringAppend(Tclcmd, "; gets ", -1);
    Tcl_DStringAppend(Tclcmd, Tcl_GetChannelName(chanID), -1);
  }
  
  // sends Tcl command to socket
  Tcl_Eval(interp, Tcl_DStringValue(Tclcmd));
  Tcl_Flush(chanID);
  if (transDelay) 
  {
	Tcl_DStringAppend(Tclcmd, "after ", -1);
	Tcl_DStringAppend(Tclcmd, str, -1);
	Tcl_Eval(interp, Tcl_DStringValue(Tclcmd));
    Tcl_Flush(chanID);
  }*/
  
  // sends query
  write(queryInsn);
  
  // creates Tcl command to read answer from socket
  Tcl_DString* Tclcmd = new Tcl_DString;
  Tcl_DStringInit(Tclcmd);
  Tcl_DStringAppend(Tclcmd, "gets ", -1);
  Tcl_DStringAppend(Tclcmd, Tcl_GetChannelName(chanID), -1);
  
  // reads answer from socket
  Tcl_ResetResult(interp);
  Tcl_Eval(interp, Tcl_DStringValue(Tclcmd));
  Tcl_Flush(chanID);
}


/**
 * @brief Writes an instruction to the socket
 * @param cmdTowrite instruction to be written to the socket
*/

void gecoIOSocket::write(const char* cmdTowrite)
{
  char str[10];
  sprintf(str, "%d", transDelay);
  
  // creates Tcl command to be sent to socket
  Tcl_DString* Tclcmd = new Tcl_DString;
  Tcl_DStringInit(Tclcmd);
  Tcl_DStringAppend(Tclcmd, "puts ", -1);
  Tcl_DStringAppend(Tclcmd, Tcl_GetChannelName(chanID), -1);
  Tcl_DStringAppend(Tclcmd," ", -1);
  Tcl_DStringAppend(Tclcmd, cmdTowrite, -1);
  
  // if a handshake will be issued by socket needs to read the handshake:
  if (handshake)
  {
    Tcl_DStringAppend(Tclcmd, "; flush ", -1);
    Tcl_DStringAppend(Tclcmd, Tcl_GetChannelName(chanID), -1);
	if (transDelay) 
	{
	  Tcl_DStringAppend(Tclcmd, "; after ", -1);
	  Tcl_DStringAppend(Tclcmd, str, -1);
	  Tcl_DStringAppend(Tclcmd, "; gets ", -1);
      Tcl_DStringAppend(Tclcmd, Tcl_GetChannelName(chanID), -1);
	}
    Tcl_DStringAppend(Tclcmd, "; gets ", -1);
    Tcl_DStringAppend(Tclcmd, Tcl_GetChannelName(chanID), -1);
  }
  
  if (transDelay) 
  {
	Tcl_DStringAppend(Tclcmd, "; after ", -1);
	Tcl_DStringAppend(Tclcmd, str, -1);
  }
  
  // sends Tcl command to socket
  Tcl_Eval(interp, Tcl_DStringValue(Tclcmd));
  Tcl_Flush(chanID);

  Tcl_DStringFree(Tclcmd);
  delete Tclcmd;
}


/**
 * @copydoc gecoObj::info
 */             

Tcl_DString* gecoIOSocket::info(const char* frontStr)
{
  gecoIOModule::info(frontStr);

  Tcl_DStringAppend(infoStr, "\nTcl socket : ", -1);
  Tcl_DStringAppend(infoStr, Tcl_GetChannelName(chanID), -1);
  
  return infoStr;
}
