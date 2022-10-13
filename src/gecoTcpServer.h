// This may look like C code, but it is really -*- C++ -*-
// $Id: ECTrigger.h 15 2014-01-03 18:49:04Z wuthrich $
// ---------------------------------------------------------------- 
//                                                                  
// Header file for class gecoTcpServer
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
// 09.12.2020 Added doxygen documentation      R. Wuthrich
//
// ---------------------------------------------------------------
/*! \file */

#ifndef gecoTcpServer_SEEN_
#define gecoTcpServer_SEEN_

#include <tcl.h>
#include <stdio.h>
#include "gecoObj.h"
#include "gecoApp.h"

using namespace std;


// -----------------------------------------------------------------------
//
// Tcl interface
//

/**
 * @brief C++ implementation of the Tcl command to create a gecoTcpServer object
 * @param clientData pointer to the gecoApp in which the gecoTcpServer instance will live
 * @param interp Tcl interpreter in which the Tcl command is executed
 * @param objc number of arguments of the Tcl command
 * @param objv arguments of the the Tcl command
 * \return TCL_OK if the execution of the Tcl command is successful and TCL_ERROR otherwise
 */

int geco_TcpServerCmd(ClientData clientData, Tcl_Interp *interp, 
		    int objc,Tcl_Obj *const objv[]);
			
			
			
// -----------------------------------------------------------------------
//
// Class to store linked server commands and and Tcl scripts
//

/** 
 * @brief A class to store TCP server commands and and Tcl scripts
 *
 * The SrvCmd class stores TCP commands and their associated Tcl scripts
 * for the usage of the gecoTcpServer class.
 */

class SrvCmd
{

  friend class gecoTcpServer;
  
private:
  SrvCmd*        next;

protected:

  Tcl_DString*   cmd;
  Tcl_DString*   TclScript;

public:

  SrvCmd(const char* Srv_Cmd, const char* Tcl_Script);
  ~SrvCmd();

  SrvCmd*        getNext()  {return next;}
  Tcl_DString*   getTclScript() {return TclScript;}
};



// -----------------------------------------------------------------------
//
// Class to store connected clients information
//

/** 
 * @brief A class to store connected clients information
 *
 * The ConnectedClient class stores information (client name
 * and Tcl_Channel used ot communicate witht he client) of abort
 * client connected to a gecoTcpServer instance.
 */

class ConnectedClient
{
	
  friend class gecoTcpServer;
  
private:
  ConnectedClient*        next;

protected:

  Tcl_Channel    channel;
  Tcl_DString*   clientName;

public:

  ConnectedClient(Tcl_Channel chan, const char* client);
  ~ConnectedClient();

  ConnectedClient*   getNext()  {return next;}
  Tcl_DString*       getClientName() {return clientName;}
  Tcl_Channel        getChannel() {return channel;}
};



// -----------------------------------------------------------------------
//
// class gecoTcpServer : a class for running a Tcp server on geco
//

/** 
 * @brief A class to run a Tcp server in geco
 * \author Rolf Wuthrich
 * \date 2020
 *
 *
 * The gecoTcpServer class allows to run a Tcp server in geco.
 *
 * The geco_TcpServerCmd() is the C++ implementation of the Tcl command to
 * open a gecoTcpServer. This Tcl command is already available in 
 * the Tcl interpreter run by an instance of gecoApp under the name 'tcpserver'.
 *
 * Associated Tcl command
 * ----------------------
 * The user of geco can create a gecoTcpServer object via the command 'tcpserver -open'
 * which is implemented by geco_TcpServerCmd(). Besides the '-open' subcommand, 'tcpserver'
 * implements as well the '-help' subcommand.
 *
 * Every gecoTcpServer is associated to a Tcl command. The associated Tcl command 
 * is created during the construction of an instance of gecoTcpServer by its
 * parent class. The Tcl command is created in the Tcl interpreter
 * run by the gecoApp in which gecoTcpServer lives.
 *
 * The gecoTcpServer class extends the subcommands from gecoObj
 * by the following subcommands
 *
 * Sub-command          | Short description
 * -------------------- | ------------------
 * -verbose             | returns/turns on/off verbose mode
 * -maxConnections      | returns/sets the maximal number of allowed client connections
 * -getPort             | returns port on which server is listening
 * -getSocketID         | returns Tcl socket ID
 * -addServerCommand    | defines a new server command
 * -removeServerCommand | removes a defined server command
 * -listServerCommands  | list defined server commands
 * -listClients         | list defined server commands
 * -close               | closes the Tcp server
 *
 * Tcp server commands
 * -------------------
 * Each gecoTcpServer will listen to a port for commands. These commands are
 * stored internally in a list of SrvCmd objects. A command
 * can be added to this list with the gecoTcpServer::addSrvCmd method
 * or interactively by the geco user with the subcommand '-addServerCommand'.
 *
 * To each command a Tcl script is associated. This Tcl script will be
 * executed every time the server receives the command to 
 * which the Tcl script is associated.
 *
 * A server command can be removed with gecoTcpServer::removeServerCommand
 * method or interactively with the '-removeServerCommand' subcommand.
 *
 * Connected clients
 * -----------------
 * Information about clients connected to the gecoTcpServer are stored
 * in a list of ConnectedClient. The maximal number of allowed connections
 * is given by maxConnections which can be set with either the
 * gecoTcpServer::setMaxConnections() method or interactively by
 * the geco user with the '-maxConnections' subcommand.
 *
 * Information on connected clients can be obtained with the
 * '-listServerCommands' subcommand or by looping over the list 
 * of ConnectedClient starting with the first one
 * that can be obtained with the gecoTcpServer::getFirstConnectedClient
 * method.
 */

class gecoTcpServer : public gecoObj
{

private:

  SrvCmd*          firstSrvCmd;         // Pointer to list of server commands
  ConnectedClient* firstClient;         // Pointer to list of connected clients
  gecoTcpServer*   nextGecoTcpServer;   // Pointer to next gecoTcpServer

protected:

  int            port;           // port number where server is listening
  Tcl_Channel    chanID;         // Tcl TCP socket
  bool           verbose;        // if on will output to stdout information on client activities
  int            maxConnections; // maximal number of allowed client connections
  int            nbrClients;     // number of connected clients

public:

  gecoTcpServer(int portID, const char* SrvCmd, gecoApp* App);
  ~gecoTcpServer();
  
  virtual int    cmd(int &i,int objc,Tcl_Obj *const objv[]);

  virtual Tcl_DString* info(const char* frontStr = "");
  
  
  SrvCmd*          getFirstSrvCmd() {return firstSrvCmd;}
  int              addSrvCmd(SrvCmd* cmd);
  void             removeSrvCmd(SrvCmd* cmd);
  SrvCmd*          findServerCmd(const char* cmd);
  void             listSrvCmds();
  
  bool             getVerbose() {return verbose;}
  
  void             setMaxConnections(int MaxConnections) {maxConnections = MaxConnections;}
  int              getMaxConnections() {return maxConnections;}
  ConnectedClient* getFirstConnectedClient() {return firstClient;}
  int              getNbrClients() {return nbrClients;}
  void             addClient(ConnectedClient* client);
  ConnectedClient* findClient(Tcl_Channel chan);
  void             removeClient(Tcl_Channel chan);
  void             listClients();
  
  Tcl_Channel      getTclChannel() {return chanID;}
  int              getPort() {return port;}
  
  gecoTcpServer*   getNextGecoTcpServer() {return nextGecoTcpServer;}
  void             setNextGecoTcpServer(gecoTcpServer* srv) {nextGecoTcpServer=srv;}
  
};



// Needed internally for callback procedures

struct ClientConnection
{
  gecoTcpServer* srv;
  Tcl_Channel    clientChan;
  gecoApp*       app;
};


#endif /* gecoTcpServer_SEEN_ */
