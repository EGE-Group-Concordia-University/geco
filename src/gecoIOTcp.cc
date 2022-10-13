// $Id: ECIOTcp.cc 37 2015-01-09 12:35:29Z wuthrich $
// ---------------------------------------------------------------
//
// Definition of the class gecoIOTcp
//
// (c) Rolf Wuthrich
//     2015-2016 Concordia University
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
// 09.11.2015 Creation                         R. Wuthrich
// ---------------------------------------------------------------

#include "gecoIOTcp.h"
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


// ---- Tcl iotcp command
//

int geco_IOTcpCmd(ClientData clientData, Tcl_Interp *interp, 
	          int objc,Tcl_Obj *const objv[])
{
  Tcl_ResetResult(interp);
  gecoApp*     app = (gecoApp *)clientData;
  gecoIOTcp*   tcp;

  if (objc==1)
    {
      Tcl_WrongNumArgs(interp, 1, objv, "subcommand ?argument ...?");
      return TCL_ERROR;
    }

  int index;
  static CONST char* cmds[] = {"-help", "-open", NULL};
  static CONST char* help[] = {"opens a TCP/IP connection", NULL};

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
      gecoHelp(interp, "iotcp", "TCP/IP interface", cmds, help);
      break;

    case 1: // -open
      if ((objc>5)||(objc<4))
	{
	  Tcl_WrongNumArgs(interp, 2, objv, "host port ?cmdName?");
	  return TCL_ERROR;
	}

      if (Tcl_GetIntFromObj(interp, objv[3], &port)!=TCL_OK) return -1;

      if (objc==4)
	tcp = new gecoIOTcp(Tcl_GetString(objv[2]), Tcl_GetString(objv[2]),
			    port, app);
      if (objc==5)
	tcp = new gecoIOTcp(Tcl_GetString(objv[4]), Tcl_GetString(objv[2]),
			    port, app);

    if (!(tcp->getTclChannel())) 
	{
	  Tcl_AppendResult(interp, " \"", Tcl_GetString(objv[2]), "\"", NULL);
	  delete tcp;
	  return TCL_ERROR;
	}

      break;

    }

  return TCL_OK;
}


// ---------------------------------------------------------------
//
// class gecoIOTcp
//


// ---- CONSTRUCTOR
//

gecoIOTcp::gecoIOTcp(const char* moduleCmd, const char* Host, int portID, 
		     gecoApp* App) :
  gecoIOSocket("TCP/IP IO-module", moduleCmd, App)
{
  port = portID;
  host  = new Tcl_DString;
  Tcl_DStringInit(host);
  Tcl_DStringAppend(host, Host, -1);

  chanID = Tcl_OpenTcpClient(App->getInterp(), portID, Host, NULL, 0, 0);
  if (chanID) Tcl_RegisterChannel(App->getInterp(), chanID);
}


// ---- DESTRUCTOR
//

gecoIOTcp::~gecoIOTcp()
{
  Tcl_DStringFree(host);
  delete host;
}


/*!
 * @copydoc gecoObj::cmd 
 *
 * Compared to gecoObj::cmd, gecoIOTcp::cmd adds the processing of
 * the new subcommands of gecoIOTcp.
 */

int gecoIOTcp::cmd(int &i, int objc,Tcl_Obj *const objv[])
{
  // first executes the command options defined in gecoIOSocket
  int index=gecoIOSocket::cmd(i, objc, objv);

  return index;
}


/**
 * @copydoc gecoObj::info
 */            

Tcl_DString* gecoIOTcp::info(const char* frontStr)
{
  gecoIOSocket::info(frontStr);

  Tcl_DStringAppend(infoStr, "\nhost : ", -1);
  Tcl_DStringAppend(infoStr, Tcl_DStringValue(host), -1);
  addInfo(frontStr, "port : ", port);
  return infoStr;
}
