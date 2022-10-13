// $Id: ECTrigger.cc 15 2014-01-03 18:49:04Z wuthrich $
// ---------------------------------------------------------------
//
// Definition of the class gecoTcpServer
//
// (c) Rolf Wuthrich
//     2020 Concordia University
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
// 31.10.2020 Creation                         R. Wuthrich
//
// ---------------------------------------------------------------

#include <tcl.h>
#include <cstring>
#include "gecoHelp.h"
#include "gecoTcpServer.h"
#include "gecoApp.h"

using namespace std;


// ------------------------------------------------------------
//
// New Tcl command
//


// ---- Tcl tcpserver command
//

int geco_TcpServerCmd(ClientData clientData, Tcl_Interp *interp, 
	          int objc,Tcl_Obj *const objv[])
{
  Tcl_ResetResult(interp);
  gecoApp*       app=(gecoApp *)clientData;
  gecoTcpServer* srv;

  if (objc==1)
    {
      Tcl_WrongNumArgs(interp, 1, objv, "subcommand ?argument ...?");
      return TCL_ERROR;
    }

  int index;
  static CONST char* cmds[] = {"-help", "-open", NULL};
  static CONST char* help[] = {"opens a TCP server", NULL};

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
      gecoHelp(interp,"tcpserver", "TCP server", cmds, help);
      break;

    case 1: // -open
      if ((objc>4)||(objc<3))
	{
	  Tcl_WrongNumArgs(interp, 2, objv, "port ?cmdName?");
	  return TCL_ERROR;
	}

      if (Tcl_GetIntFromObj(interp,objv[2],&port)!=TCL_OK) return TCL_ERROR;

	  if (objc==3)
        srv = new gecoTcpServer(port, "localTcpServer", (gecoApp *)clientData);
	  if (objc==4)
	    srv = new gecoTcpServer(port, Tcl_GetString(objv[3]), (gecoApp *)clientData);

      if (!(srv->getTclChannel())) 
	{
	  Tcl_AppendResult(interp," \"",Tcl_GetString(objv[2]),"\"",NULL);
	  delete srv;
	  return TCL_ERROR;
	}

      break;

    }

  return TCL_OK;
}


// -------------------------------------------------------------------------
//
// Callback procedure called when Tcl_Channel to the client is closed
//

void ClientTclChannelClosed(ClientData clientData)
{
  ClientConnection *conn  = (ClientConnection *)clientData;
  gecoApp*         app    = conn->app;
  Tcl_Channel      chan   = conn->clientChan;
  gecoTcpServer*   srv    = conn->srv;
  ConnectedClient* client = srv->findClient(chan);
  
  if (srv->getVerbose())
    cout << "Tcl Channel to client " << Tcl_DStringValue(client->getClientName()) << " closed\n";
  
  srv->removeClient(chan);
}


// -------------------------------------------------------------------------
//
// Callback procedure called when the server gets data from the client
//

void ClientTrsm(ClientData clientData, int mask)
{
  ClientConnection *conn = (ClientConnection *)clientData;
  gecoApp*       app  = conn->app;
  Tcl_Channel    chan = conn->clientChan;
  gecoTcpServer* srv  = conn->srv;
  
  // checks if server still exists
  if (app->gecoServerExist(srv)==0) {
    Tcl_UnregisterChannel(srv->getTclInterp(), chan);
    return;
  }
  
  // checks if connection was closed by client
  if (Tcl_Eof(chan)) {
    if (srv->getVerbose()) cout << "Communication closed by client\n";
    Tcl_UnregisterChannel(srv->getTclInterp(), chan);  // will trigger ClientTclChannelClosed 
    return;
  }
  
  Tcl_DString* str  = new Tcl_DString;
  Tcl_DStringInit(str);
  Tcl_Gets(chan, str);
  
  SrvCmd* p=srv->findServerCmd(Tcl_DStringValue(str));
  if (srv->getVerbose()) {
    cout << "Tcp server received a command from client : " << Tcl_DStringValue(str) << "\n";
    if (p) cout << "Found an associated Tcl script : " << Tcl_DStringValue(p->getTclScript()) << "\n";
  }
  
  // evaluates the associated TclScript and sends back to client its result
  if (p) {
    Tcl_ResetResult(app->getInterp());
    Tcl_Eval(app->getInterp(), Tcl_DStringValue(p->getTclScript()));
    Tcl_WriteChars(chan, Tcl_GetStringResult(app->getInterp()), -1);
    Tcl_WriteChars(chan, "\n", -1);
    Tcl_Flush(chan);
    Tcl_ResetResult(app->getInterp());
  }
  
  Tcl_DStringFree(str);
  delete str;
  
  // reschedule file handler 
  int chanID;
  Tcl_GetChannelHandle(chan, TCL_READABLE, (ClientData*)&chanID);
  Tcl_CreateFileHandler(chanID, TCL_READABLE, ClientTrsm, clientData);
}


// -------------------------------------------------------------------------


// -------------------------------------------------------------------------
//
// Callback procedure called when Tcl_Channel of a Tcp server is closed
//

void TcpServerChannelClosed(ClientData clientData)
{
  gecoTcpServer *srv = (gecoTcpServer *)clientData;
  delete srv;
}


// -------------------------------------------------------------------------
//
// Callback procedure called if a connection from a client happens
//

void TcpAcceptProc(ClientData clientData, Tcl_Channel channel, char *hostName, int port)
{  
  gecoTcpServer *srv = (gecoTcpServer *)clientData;
  Tcl_RegisterChannel(srv->getTclInterp(), channel);
  
  if (srv->getVerbose())
    cout << "Got a connection from " << hostName << " on port "
	 << port << " with Tcl socket : " << Tcl_GetChannelName(channel) << "\n";
  
  if (srv->getNbrClients()==srv->getMaxConnections()) {
    Tcl_WriteChars(channel, "Maximum number of connections reached - Connection refused\n", -1);
    Tcl_UnregisterChannel(srv->getTclInterp(), channel);
    return;
  }
  
  ConnectedClient* client = new ConnectedClient(channel, hostName);
  srv->addClient(client);
	  
  // Ensure that each "puts" by the server results in a network transmission
  Tcl_SetChannelOption(srv->getTclInterp(), channel, "-buffering", "line");

  // gets Tcl channel ID
  int chanID;
  Tcl_GetChannelHandle(channel, TCL_READABLE, (ClientData*)&chanID);
  
  ClientConnection* connection = new ClientConnection();
  connection->srv = srv;
  connection->clientChan = channel;
  connection->app = srv->getGecoApp();

  // Set up a callback for when the client disconnects
  Tcl_CreateCloseHandler(channel, ClientTclChannelClosed, (ClientData)connection);
  
  // Set up a callback for when the client sends data
  Tcl_CreateFileHandler(chanID, TCL_READABLE, ClientTrsm, (ClientData)connection);
}

// -------------------------------------------------------------------------



// -----------------------------------------------------------------------
//
// Class to store linked server commands and and Tcl scripts
//

/**
 * @brief Constructor
 * @param Srv_Cmd command to which the Tcp server has to react
 * @param Tcl_Script Tcl script associated to the Srv_Cmd 
*/

SrvCmd::SrvCmd(const char* Srv_Cmd, const char* Tcl_Script)
{
  cmd = new Tcl_DString;
  Tcl_DStringInit(cmd);
  Tcl_DStringAppend(cmd, Srv_Cmd, -1);
  
  TclScript = new Tcl_DString;
  Tcl_DStringInit(TclScript);
  Tcl_DStringAppend(TclScript, Tcl_Script, -1);
  
  next=NULL;
}


/**
 * @brief Destructor
*/

SrvCmd::~SrvCmd()
{
  Tcl_DStringFree(cmd);
  delete cmd;
  Tcl_DStringFree(TclScript);
  delete TclScript;
}

// -------------------------------------------------------------------------



// -----------------------------------------------------------------------
//
// Class to store connected clients information
//

/**
 * @brief Constructor
 * @param chan Tcl channel over which the connected client communicates with the Tcp server
 * @param client client IP connected to the Tcp server
*/

ConnectedClient::ConnectedClient(Tcl_Channel chan, const char* client)
{
  clientName = new Tcl_DString;
  Tcl_DStringInit(clientName);
  Tcl_DStringAppend(clientName, client, -1);
  
  channel=chan;
  
  next=NULL;
}


/**
 * @brief Destructor
*/

ConnectedClient::~ConnectedClient()
{
  Tcl_DStringFree(clientName);
  delete clientName;
}

// -------------------------------------------------------------------------



// -------------------------------------------------------------------------
//
// class gecoTcpServer : a class for running a Tcp server on geco
//


/**
 * @brief Constructor
 * @param portID port to which the gecoTcpServer is listening
 * @param SrvCmd Tcl command associated to the gecoTcpServer
 * @param App gecoApp in which the gecoTcpServer lives 
 *
 * The constructor will create a Tcl command via the call of
 * the constructor of gecoObj. It further defines additional
 * subcommands.
 *
 * Opens a Tcp server listening on port portID. Registers in the
 * Tcl interpreter run by the gecoApp in which the gecoTcpServer
 * instance lives the Tcl_Channel of the Tcp server.
 *
 * Setup call back procedures to handle connections from clients
 * and closing of the Tcp_Channel of the gecoTcpServer.
*/

gecoTcpServer::gecoTcpServer(int portID, const char* SrvCmd, gecoApp* App) :
    gecoObj("Tcp Server", SrvCmd, App, false)
{
  // overwrites the Tcl cmd generated by gecoObj if a cmd is provided by user
  //if (SrvCmd) {
  //  Tcl_DStringInit(TclCmd);
  //  Tcl_DStringAppend(TclCmd, SrvCmd, -1);
  //}
	
  port=portID;
  firstSrvCmd=NULL;
  nextGecoTcpServer=NULL;
  firstClient=NULL;
  verbose=false;
  maxConnections=1;
  nbrClients=0;

  chanID = Tcl_OpenTcpServer(App->getInterp(), portID, NULL, TcpAcceptProc, (ClientData)this);
  if (!chanID) return;
  Tcl_RegisterChannel(App->getInterp(), chanID);
  Tcl_CreateCloseHandler(chanID, TcpServerChannelClosed, (ClientData)this);
  App->addGecoTcpServer(this);
  
  // define Tcl subcommands
  addOption("-verbose", &verbose, "returns/turns on/off verbose mode");
  addOption("-maxConnections", &maxConnections, "returns/sets the maximal number of allowed client connections");
  addOption("-getPort", "returns port on which server is listening");
  addOption("-getSocketID", "returns Tcl socket ID");
  addOption("-addServerCommand", "defines a new server command");
  addOption("-removeServerCommand", "removes a defined server command");
  addOption("-listServerCommands", "list defined server commands");
  addOption("-listClients", "list defined server commands");
  addOption("-close", "closes the Tcp server");
  
  char str[TCL_DOUBLE_SPACE];
  sprintf(str, "%i", port);
  Tcl_AppendResult(interp, "\nNew Tcl server started on port ", str, "\n", NULL);
  Tcl_AppendResult(interp, "Tcl command: ",getTclCmd(), "\n", NULL);
}


/**
 * @brief Destructor
 *
 * Will as well free memory from all SrvCmd defined in the gecoTcpServer
 * and close all connections to connected clients.
*/

gecoTcpServer::~gecoTcpServer()
{
  // Tcl_UnregisterChannel(interp,chanID); is done by callback
  char str[TCL_DOUBLE_SPACE];
  sprintf(str, "%i", port);
  cout << "Shutting down Tcp server on port " << str << "\n";
  
  SrvCmd* p=firstSrvCmd;
  while (firstSrvCmd)
    {
      p=firstSrvCmd;
      firstSrvCmd=firstSrvCmd->next;
      delete p;
    }

  // closes all connected clients (quite tricky due to callback)	
  while (firstClient)
    Tcl_UnregisterChannel(interp, firstClient->getChannel());

  app->removeGecoTcpServer(this);
}


/*! 
 * @copydoc gecoObj::cmd
 *
 * Compared to gecoObj::cmd, gecoTcpServer::cmd adds the processing of
 * the new subcommands of gecoTcpServer.
 */

int gecoTcpServer::cmd(int &i, int objc,Tcl_Obj *const objv[])
{
  // first executes the command options defined in gecoObj
  int index=gecoObj::cmd(i,objc,objv);
  
  if (index==getOptionIndex("-getPort"))
    {
      char str[TCL_DOUBLE_SPACE];
      sprintf(str, "%i", port);
      Tcl_AppendResult(interp, str, NULL);
      i++;
    }

  if (index==getOptionIndex("-getSocketID"))
    {
      Tcl_AppendResult(interp, Tcl_GetChannelName(getTclChannel()), NULL);
      i++;
    }
	
  if (index==getOptionIndex("-addServerCommand"))
    {
      if (i+2>=objc)
      	{
      	  Tcl_WrongNumArgs(interp,i+1,objv,"Server_Command Tcl_Script");
      	  return -1;
      	}
		
      if (findServerCmd(Tcl_GetString(objv[i+1])))
	{
	  Tcl_AppendResult(interp,"command \"",Tcl_GetString(objv[i+1]),
			   "\" already defined. No new command was defined.",NULL);
	  return -1;
	}

      // creates a new entry and links it
      SrvCmd* cmd = new SrvCmd(Tcl_GetString(objv[i+1]),
				               Tcl_GetString(objv[i+2]));
      if (addSrvCmd(cmd)==TCL_ERROR) 
	  {
	    delete cmd;
	    return -1;
	  }
      i=i+3;
    }
	
  if (index==getOptionIndex("-removeServerCommand"))
    {
      if (i+1>=objc)
      	{
      	  Tcl_WrongNumArgs(interp,i+1,objv,"Server_Command");
      	  return -1;
      	}
      SrvCmd* p=findServerCmd(Tcl_GetString(objv[i+1]));
      if (p==NULL)
	{
	  Tcl_AppendResult(interp,"command \"",Tcl_GetString(objv[i+1]),
			   "\" is not defined. No command was removed.",NULL);
	  return -1;
	}
      removeSrvCmd(p);
      i=i+2;
    }
	
  if (index==getOptionIndex("-listServerCommands"))
    {
      listSrvCmds();
      i++;
    }
	
  if (index==getOptionIndex("-listClients"))
    {
      listClients();
      i++;
    }	
	
  if (index==getOptionIndex("-close"))
    {
      i=objc; // otherwise: crash
      Tcl_UnregisterChannel(interp,chanID);
    }

  return index;
}


/**
 * @copydoc gecoObj::info
 *
 * In addition to gecoObj::info, gecoTcpServer::info adds the information
 * about the port and socket of the Tcp server.
 */

Tcl_DString* gecoTcpServer::info(const char* frontStr)
{
  gecoObj::info(frontStr);
  addInfo(frontStr, "Port:\t", port);  
  addInfo(frontStr, "Socket:\t", Tcl_GetChannelName(getTclChannel()));  
  return infoStr;
}


/**
 * @brief Adds a server command to the command list
*/

int gecoTcpServer::addSrvCmd(SrvCmd* cmd)
{
  SrvCmd* p=firstSrvCmd;
  if (p)
    {
      while (p->getNext())
	p=p->getNext();    
      p->next=cmd;
    }
  else
    firstSrvCmd=cmd;
  cmd->next=NULL;
  return TCL_OK;
}


/**
 * @brief Removes a server command from the command list
 * @param cmd Tcp server command to remove
*/

void gecoTcpServer::removeSrvCmd(SrvCmd* cmd)
{
  if (cmd==firstSrvCmd)
    {
      firstSrvCmd=cmd->getNext();
      delete cmd;
      return;
    }

  SrvCmd* p=firstSrvCmd;
  while(p)
    {
      if (p->getNext()==cmd) break;
      p=p->getNext();
    }
  p->next=cmd->next;
  delete cmd;
}


/**
 * @brief Finds entry related to the server command
 * @param cmd Server command to find
 * \return pointer to the SrvCmd related to the server command
*/

SrvCmd* gecoTcpServer::findServerCmd(const char* cmd)
{
  SrvCmd* p=firstSrvCmd;
  while(p)
    {
      if (strcmp(Tcl_DStringValue(p->cmd), cmd)==0) break;
      p=p->getNext();
    }
  return p;
}


/**
 * @brief List all server command from the command list
*/

void gecoTcpServer::listSrvCmds()
{
  char str[100];
  Tcl_AppendResult(interp,
     "NUM  SERVER COMMAND  TCL SCRIPT\n",NULL);
  SrvCmd* p=firstSrvCmd;
  int i = 1;
  while (p)
    {
      sprintf(str,"%-4d %-15s %s\n",i, 
	      Tcl_DStringValue(p->cmd), 
	      Tcl_DStringValue(p->TclScript));
      Tcl_AppendResult(interp, str, NULL);
      i++;
      p=p->getNext();
    }
}


/**
 * @brief Adds a client to the list of the connected clients
*/

void gecoTcpServer::addClient(ConnectedClient* client)
{
  nbrClients++;
  ConnectedClient* p=firstClient;
  if (p)
    {
      while (p->getNext())
	p=p->getNext();    
      p->next=client;
    }
  else
    firstClient=client;
}


/**
 * @brief Finds a client in the list of the connected clients
 * @param chan Tcl_Channel of the client
 * \return pointer to the ConnectedClient from the list of the connected clients
*/

ConnectedClient* gecoTcpServer::findClient(Tcl_Channel chan)
{
  ConnectedClient* p=firstClient;
  while(p)
    {
      if (p->getChannel()==chan) break;
      p=p->getNext();
    }
  return p;
}


/**
 * @brief Removes a client from the list of the connected clients
 * @param chan Tcl_Channel of the client to remove
*/

void gecoTcpServer::removeClient(Tcl_Channel chan)
{
  nbrClients--;
  ConnectedClient* p=firstClient;
  ConnectedClient* q;
  
  if (chan==firstClient->getChannel())
    {
      firstClient=firstClient->getNext();
      delete p;
      return;
    }

  while(p)
    {
      if (p->getNext()->getChannel()==chan) {
	q=p->getNext();
	break;
      }
      p=p->getNext();
    }
  p->next=q->next;
  delete q;
}


/**
 * @brief Lists information on connected clients
*/

void gecoTcpServer::listClients()
{
  char str[100];
  Tcl_AppendResult(interp,
		   "TCL CHANNEL            CLIENT NAME\n",NULL);
  ConnectedClient* p=firstClient;
  int i = 1;
  while (p)
    {
      sprintf(str,"%-22s %s\n", 
	      Tcl_GetChannelName(p->getChannel()), 
	      Tcl_DStringValue(p->getClientName()));
      Tcl_AppendResult(interp, str, NULL);
      i++;
      p=p->getNext();
    }
}
