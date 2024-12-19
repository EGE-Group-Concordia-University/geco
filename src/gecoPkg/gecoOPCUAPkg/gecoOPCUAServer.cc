// ---------------------------------------------------------------
//
// Definition of the class gecoOPCUAServer
//
// (c) Rolf Wuthrich
//     2024 Concordia University
//
// author:    Rolf Wuthrich
// email:     rolf.wuthrich@concordia.ca
//
// This software is copyright under the BSD license

#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <tcl8.6/tcl.h>
#include <open62541/server.h>
#include <open62541/server_config_default.h>
#include "gecoHelp.h"
#include "gecoApp.h"
#include "gecoProcess.h"
#include "gecoOPCUAServer.h"

using namespace std;

// ------------------------------------------------------------
//
// OPC UA Server thread
//

// Global variables
UA_Boolean running = false;          // flag to control the server's running state
volatile sig_atomic_t stop_flag = 0; // flag to control main loop

void *server_thread(void *arg)
{
  UA_Server *server = (UA_Server *)arg;
  // The server will block as long as running = true
  UA_Server_run(server, &running);
  return NULL;
}

void stopHandler(int signum)
{
  printf("\nReceived signal '%s' (signal number: %d)\n", strsignal(signum), signum);
  stop_flag = 1;
  running = false; // This will stop the server loop in UA_Server_run
}

// ------------------------------------------------------------
//
// Tcl interface
//

// Command to create a new gecoOPCUAServer object
//

int gecoOPCUAServerCmd(ClientData clientData, Tcl_Interp *interp,
                       int objc, Tcl_Obj *const objv[])
{
  Tcl_ResetResult(interp);
  gecoApp *app = (gecoApp *)clientData;
  gecoOPCUAServer *opcuaserver;

  if (objc == 1)
  {
    Tcl_WrongNumArgs(interp, 1, objv, "subcommand ?argument ...?");
    return TCL_ERROR;
  }

  int index;
  static CONST char *cmds[] = {"-help", "-create", NULL};
  static CONST char *help[] = {"create an OPC UA server", NULL};

  if (Tcl_GetIndexFromObj(interp, objv[1], cmds, "subcommand", '0', &index) != TCL_OK)
    return TCL_ERROR;

  int port;

  switch (index)
  {
  case 0: // -help
    if (objc != 2)
    {
      Tcl_WrongNumArgs(interp, 2, objv, NULL);
      return TCL_ERROR;
    }
    gecoHelp(interp, "opcuaserver", "OPC UA server", cmds, help);
    break;

  case 1: // -create
    if ((objc > 4) || (objc < 3))
    {
      Tcl_WrongNumArgs(interp, 2, objv, "port ?cmdName?");
      return TCL_ERROR;
    }

    if (Tcl_GetIntFromObj(interp, objv[2], &port) != TCL_OK)
      return TCL_ERROR;

    if (objc == 3)
      opcuaserver = new gecoOPCUAServer("OPC UA Server", "opcuaserver", port, (gecoApp *)clientData);

    if (objc == 4)
      opcuaserver = new gecoOPCUAServer("OPC UA Server", Tcl_GetString(objv[3]), port, (gecoApp *)clientData, false);

    // adds the gecoProcess to the geco loop
    app->addGecoProcess(opcuaserver);
    Tcl_ResetResult(app->getInterp());
    Tcl_AppendResult(app->getInterp(), opcuaserver->getID(), NULL);
    break;
  }

  return TCL_OK;
}

// ---------------------------------------------------------------
//
// Class LinkedVariable
//

LinkedVariable::LinkedVariable(const char *Tcl_Var, const char *browse_Name, const char *display_Name, Tcl_Interp *Interp)
{
  interp = Interp;

  TclVar = new Tcl_DString;
  Tcl_DStringInit(TclVar);
  Tcl_DStringAppend(TclVar, Tcl_Var, -1);
  TclVarValue = Tcl_Alloc(10);
  Tcl_LinkVar(interp, Tcl_Var, (char *)&TclVarValue, TCL_LINK_STRING);

  browseName = new Tcl_DString;
  Tcl_DStringInit(browseName);
  Tcl_DStringAppend(browseName, browse_Name, -1);

  displayName = new Tcl_DString;
  Tcl_DStringInit(displayName);
  Tcl_DStringAppend(displayName, display_Name, -1);

  next = NULL;

  // OPC UA attributes
  ua_attr = UA_VariableAttributes_default;
  ua_attr.displayName = UA_LOCALIZEDTEXT(const_cast<char *>("en-US"), Tcl_DStringValue(displayName));
  ua_attr.dataType = UA_TYPES[UA_TYPES_FLOAT].typeId;
  ua_attr.accessLevel = UA_ACCESSLEVELMASK_READ;
}

LinkedVariable::~LinkedVariable()
{
  Tcl_UnlinkVar(interp, Tcl_DStringValue(TclVar));
  Tcl_DStringFree(TclVar);
  delete TclVar;
  Tcl_DStringFree(browseName);
  delete browseName;
  Tcl_DStringFree(displayName);
  delete displayName;
}

// -----------------------------------------------------------------------
//
// Class to store linked OPC UA server commands
//

/**
 * @brief Constructor
 * @param Srv_Cmd command to which the OPC UA server has to react
 * @param geco_app Geco App in which the command will have to run
 */

OPCUACmd::OPCUACmd(const char *Srv_Cmd, gecoApp *geco_app)
{
  cmd = new Tcl_DString;
  Tcl_DStringInit(cmd);
  Tcl_DStringAppend(cmd, Srv_Cmd, -1);

  app = geco_app;

  next = NULL;
}

/**
 * @brief Destructor
 */

OPCUACmd::~OPCUACmd()
{
  Tcl_DStringFree(cmd);
  delete cmd;
}

// -------------------------------------------------------------------------

// ------------------------------------------------------------
//
// Tcl Script Call Back Method
//

static UA_StatusCode
tcl_script_MethodCallback(UA_Server *server,
                          const UA_NodeId *sessionId, void *sessionHandle,
                          const UA_NodeId *methodId, void *methodContext,
                          const UA_NodeId *objectId, void *objectContext,
                          size_t inputSize, const UA_Variant *input,
                          size_t outputSize, UA_Variant *output)
{

  // Validate the number of inputs
  if (inputSize != 1 || !UA_Variant_hasScalarType(&input[0], &UA_TYPES[UA_TYPES_STRING]))
  {
    return UA_STATUSCODE_BADINVALIDARGUMENT;
  }

  // Retrieve the command and application context
  auto *cmd = static_cast<OPCUACmd *>(methodContext);
  gecoApp *app = cmd->getGecoApp();
  // const char *baseCmd = cmd->getCmd();

  Tcl_DString *tcl_cmd = new Tcl_DString;
  Tcl_DStringInit(tcl_cmd);
  Tcl_DStringAppend(tcl_cmd, cmd->getCmd(), -1);
  Tcl_DStringAppend(tcl_cmd, " ", -1);
  Tcl_DStringAppend(tcl_cmd, static_cast<const char *>(input[0].data), input[0].arrayLength);

  // Run Tcl script in the gecoApp interpreter
  Tcl_ResetResult(app->getInterp());
  Tcl_Eval(app->getInterp(), Tcl_DStringValue(tcl_cmd));

  // Set the output as the Tcl script result
  const char *tclResult = Tcl_GetStringResult(app->getInterp());
  UA_String returnValue = UA_STRING(const_cast<char *>(tclResult));
  UA_Variant_setScalarCopy(output, &returnValue, &UA_TYPES[UA_TYPES_STRING]);

  // Reset Tcl result
  Tcl_ResetResult(app->getInterp());

  // Clean up
  Tcl_DStringFree(tcl_cmd);
  delete tcl_cmd;

  return UA_STATUSCODE_GOOD;
}

// -------------------------------------------------------------------------

// ---------------------------------------------------------------
//
// class gecoOPCUAServer
//

// ---- CONSTRUCTOR
//

gecoOPCUAServer::gecoOPCUAServer(const char *serverName, const char *serverCmd, int portID, gecoApp *App, bool addID) : gecoObj(serverName, serverCmd, App, addID),
                                                                                                                        gecoProcess(serverName, "user", serverCmd, App)
{
  firstLinkedVariable = NULL;
  firstOPCUACmd = NULL;
  port = portID;
  opcua_namespace = new Tcl_DString;
  Tcl_DStringInit(opcua_namespace);
  Tcl_DStringAppend(opcua_namespace, "geco_opcua", -1);
  browseName = new Tcl_DString;
  Tcl_DStringInit(browseName);
  displayName = new Tcl_DString;
  Tcl_DStringInit(displayName);

  addOption("-linkTclVariable", "links a Tcl variable");
  addOption("-unlinkTclVariable", "removes link to a Tcl variable");
  addOption("-listLinkedTclVariables", "list all linked Tcl variables");
  addOption("-addServerCommand", "defines a new server command");
  addOption("-removeServerCommand", "removes a defined server command");
  addOption("-listServerCommands", "list defined server commands");
  addOption("-namespace", opcua_namespace, "returns/sets OPC UA namespace");
  addOption("-browseName", browseName, "returns/sets OPC UA BrowsName");
  addOption("-displayName", displayName, "returns/sets OPC UA DisplayName");

  // Register signal handler for SIGINT (Ctrl-C) and SIGTERM
  signal(SIGINT, stopHandler);
  signal(SIGTERM, stopHandler);
}

// ---- DESTRUCTOR
//

gecoOPCUAServer::~gecoOPCUAServer()
{
  cout << "Removing OPC UA server\n";
  if (running)
  {
    running = false;
    pthread_join(thread, NULL);
    UA_Server_delete(server);
  }

  Tcl_DStringFree(opcua_namespace);
  delete opcua_namespace;
  Tcl_DStringFree(browseName);
  delete browseName;
  Tcl_DStringFree(displayName);
  delete displayName;

  LinkedVariable *p = getFirstLinkedVariable();
  LinkedVariable *q = getFirstLinkedVariable();
  while (p)
  {
    p = p->getNext();
    delete q;
    q = p;
  }

  OPCUACmd *srv_cmd = firstOPCUACmd;
  while (firstOPCUACmd)
  {
    srv_cmd = firstOPCUACmd;
    firstOPCUACmd = firstOPCUACmd->next;
    delete srv_cmd;
  }
}

// ---- CMD : process a potential command option (# i of objv[])
//            searches the options table
//            if a match is found processes the command option and
//            returns the index from the options table
//            if no match returns -1

int gecoOPCUAServer::cmd(int &i, int objc, Tcl_Obj *const objv[])
{
  // executes the command options defined in gecoProcess
  int index = gecoProcess::cmd(i, objc, objv);

  if (index == getOptionIndex("-linkTclVariable"))
  {
    if (i + 3 >= objc)
    {
      Tcl_WrongNumArgs(interp, i + 1, objv, "Tcl_Variable BrowseName DisplayName");
      return -1;
    }
    LinkedVariable *var = new LinkedVariable(Tcl_GetString(objv[i + 1]), Tcl_GetString(objv[i + 2]), Tcl_GetString(objv[i + 3]), getTclInterp());
    addLinkedVariable(var);
    i = i + 4;
  }

  if (index == getOptionIndex("-unlinkTclVariable"))
  {
    if (i + 1 >= objc)
    {
      Tcl_WrongNumArgs(interp, i + 1, objv, "Tcl_Variable");
      return -1;
    }
    LinkedVariable *p = findLinkedVariable(Tcl_GetString(objv[i + 1]));
    if (p == NULL)
    {
      Tcl_AppendResult(interp, "variable \"", Tcl_GetString(objv[i + 1]),
                       "\" is not linked to the OPC UA server", NULL);
      return -1;
    }
    removeLinkedVariable(p);
    i = i + 2;
  }

  if (index == getOptionIndex("-listLinkedTclVariables"))
  {
    listLinkedVar();
    i++;
  }

  if (index == getOptionIndex("-addServerCommand"))
  {
    if (i + 1 >= objc)
    {
      Tcl_WrongNumArgs(interp, i + 1, objv, "Server_Command");
      return -1;
    }

    if (findServerCmd(Tcl_GetString(objv[i + 1])))
    {
      Tcl_AppendResult(interp, "command \"", Tcl_GetString(objv[i + 1]),
                       "\" already defined. No new command was added.", NULL);
      return -1;
    }

    // creates a new entry and links it
    OPCUACmd *cmd = new OPCUACmd(Tcl_GetString(objv[i + 1]), app);
    if (addSrvCmd(cmd) == TCL_ERROR)
    {
      delete cmd;
      return -1;
    }
    i = i + 2;
  }

  if (index == getOptionIndex("-removeServerCommand"))
  {
    if (i + 1 >= objc)
    {
      Tcl_WrongNumArgs(interp, i + 1, objv, "Server_Command");
      return -1;
    }
    OPCUACmd *p = findServerCmd(Tcl_GetString(objv[i + 1]));
    if (p == NULL)
    {
      Tcl_AppendResult(interp, "command \"", Tcl_GetString(objv[i + 1]),
                       "\" is not defined. No command was removed.", NULL);
      return -1;
    }
    removeSrvCmd(p);
    i = i + 2;
  }

  if (index == getOptionIndex("-listServerCommands"))
  {
    listSrvCmds();
    i++;
  }

  return index;
}

/**
 * @copydoc gecoProcess::handleEvent
 */

void gecoOPCUAServer::handleEvent(gecoEvent *ev)
{
  gecoProcess::handleEvent(ev);

  if (status != Active)
    return;

  // update all linked Tcl variables on the server
  LinkedVariable *var = getFirstLinkedVariable();
  while (var)
  {
    float val = stof(var->TclVarValue);
    UA_Variant_setScalar(&(var->ua_attr.value), &val, &UA_TYPES[UA_TYPES_FLOAT]);
    UA_Server_writeValue(server, var->varNodeId, var->ua_attr.value);
    var = var->getNext();
  }
}

// ---- INFO : returns a Tcl_DString containing relevant info
//

Tcl_DString *gecoOPCUAServer::info(const char *frontStr)
{
  gecoProcess::info(frontStr);
  return infoStr;
}

/**
 * @copydoc gecoProcess::terminate
 */

void gecoOPCUAServer::terminate(gecoEvent *ev)
{
  // Graceful shutdown
  printf("Stopping OPC UA server ...\n");

  // Initiate the shutdown process of the server
  running = false;

  // Stop the server thread
  pthread_join(thread, NULL);

  // Clean up server resources
  UA_Server_delete(server);

  gecoProcess::terminate(ev);
}

/**
 * @copydoc gecoProcess::activate
 *
 * In addition to gecoProcess::activate, gecoOPCUAServer::activate
 * will build the OPC UA server and start it
 */

void gecoOPCUAServer::activate(gecoEvent *ev)
{
  gecoProcess::activate(ev);

  // Initialize the OPC UA server
  server = UA_Server_new();
  UA_ServerConfig *config = UA_Server_getConfig(server);
  UA_ServerConfig_setMinimal(config, port, NULL);

  // Add custom namespace
  ns_idx = UA_Server_addNamespace(server, Tcl_DStringValue(opcua_namespace));

  // Add top node (Object)
  UA_ObjectAttributes nodeAttr = UA_ObjectAttributes_default;
  nodeAttr.displayName = UA_LOCALIZEDTEXT(const_cast<char *>("en-US"), Tcl_DStringValue(displayName));
  UA_Server_addObjectNode(
      server,
      UA_NODEID_NULL,                                         // Let the server assign a NodeId
      UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER),           // Parent: Objects Folder
      UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),               // Reference type
      UA_QUALIFIEDNAME(ns_idx, Tcl_DStringValue(browseName)), // Browse name (for client)
      UA_NODEID_NUMERIC(0, UA_NS0ID_BASEOBJECTTYPE),          // Type definition
      nodeAttr,
      NULL,
      &topNodeId // Output parameter for the NodeId
  );

  // Add variable nodes
  LinkedVariable *var = firstLinkedVariable;
  while (var)
  {
    addVariableNode(var);
    var = var->getNext();
  }

  // Add method nodes
  OPCUACmd *cmd = firstOPCUACmd;
  while (cmd)
  {
    addMethodNode(cmd);
    cmd = cmd->getNext();
  }

  // Create and start the server thread
  running = true;
  pthread_create(&thread, NULL, server_thread, server);
}

/**
 * @brief Links a Tcl varaible to the OPC UA server
 * @param insn LinkedVariable to be linked to the OPC UA server
 */

void gecoOPCUAServer::addLinkedVariable(LinkedVariable *var)
{
  LinkedVariable *p = getFirstLinkedVariable();
  if (p)
  {
    while (p->getNext())
      p = p->getNext();
    p->next = var;
  }
  else
    firstLinkedVariable = var;
  var->next = NULL;
}

/**
 * @brief Removes a linked Tcl variable from the list
 * @param var LinkedVariable to be removed from the list
 */

void gecoOPCUAServer::removeLinkedVariable(LinkedVariable *var)
{
  if (var == getFirstLinkedVariable())
  {
    firstLinkedVariable = var->getNext();
    delete var;
    return;
  }

  LinkedVariable *p = getFirstLinkedVariable();
  while (p)
  {
    if (p->getNext() == var)
      break;
    p = p->getNext();
  }
  p->next = var->next;
  delete var;
}

/**
 * @brief Finds the LinkedVariable associated to a Tcl variable
 * @param TclVar Tcl variable associated to the LinkedVariable one wants to find
 */

LinkedVariable *gecoOPCUAServer::findLinkedVariable(const char *TclVar)
{
  LinkedVariable *p = getFirstLinkedVariable();
  while (p)
  {
    if (strcmp(Tcl_DStringValue(p->TclVar), TclVar) == 0)
      break;
    p = p->getNext();
  }
  return p;
}

/**
 * @brief Lists all linked Tcl variables
 */

void gecoOPCUAServer::listLinkedVar()
{
  char str[100];
  Tcl_AppendResult(interp, "TCL VARIABLE   BROWSE NAME      DISPLAY NAME\n", NULL);

  LinkedVariable *p = getFirstLinkedVariable();

  while (p)
  {
    sprintf(str, "%-14s %-16s %s\n",
            Tcl_DStringValue(p->TclVar),
            Tcl_DStringValue(p->browseName),
            Tcl_DStringValue(p->displayName));
    Tcl_AppendResult(interp, str, NULL);
    p = p->getNext();
  }
}

/**
 * @brief Adds an OPC UA variable node to the server
 */

void gecoOPCUAServer::addVariableNode(LinkedVariable *var)
{
  cout << "Adding variable node: " << Tcl_DStringValue(var->displayName) << "\n";

  // Add variable under the top node
  UA_Server_addVariableNode(
      server,
      UA_NODEID_NULL,                                              // Let the server assign a NodeId for the variable
      topNodeId,                                                   // Parent: Top node
      UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),                 // Reference type
      UA_QUALIFIEDNAME(ns_idx, Tcl_DStringValue(var->browseName)), // Browse name (for client)
      UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE),         // Type definition
      var->ua_attr,
      NULL,
      &(var->varNodeId) // Output parameter for the NodeId
  );

  // Set the initial value of the linked Tcl variable on the server
  float val = stof(var->TclVarValue);
  UA_Variant_setScalar(&(var->ua_attr.value), &val, &UA_TYPES[UA_TYPES_FLOAT]);
  UA_Server_writeValue(server, var->varNodeId, var->ua_attr.value);
}

/**
 * @brief Adds a OPCUACmd command to the command list
 */

int gecoOPCUAServer::addSrvCmd(OPCUACmd *cmd)
{
  OPCUACmd *p = firstOPCUACmd;
  if (p)
  {
    while (p->getNext())
      p = p->getNext();
    p->next = cmd;
  }
  else
    firstOPCUACmd = cmd;
  cmd->next = NULL;
  return TCL_OK;
}

/**
 * @brief Removes a server command from the command list
 * @param cmd Tcp server command to remove
 */

void gecoOPCUAServer::removeSrvCmd(OPCUACmd *cmd)
{
  if (cmd == firstOPCUACmd)
  {
    firstOPCUACmd = cmd->getNext();
    delete cmd;
    return;
  }

  OPCUACmd *p = firstOPCUACmd;
  while (p)
  {
    if (p->getNext() == cmd)
      break;
    p = p->getNext();
  }
  p->next = cmd->next;
  delete cmd;
}

/**
 * @brief Finds entry related to the server command
 * @param cmd Server command to find
 * \return pointer to the OPCUACmd related to the server command
 */

OPCUACmd *gecoOPCUAServer::findServerCmd(const char *cmd)
{
  OPCUACmd *p = firstOPCUACmd;
  while (p)
  {
    if (strcmp(Tcl_DStringValue(p->cmd), cmd) == 0)
      break;
    p = p->getNext();
  }
  return p;
}

/**
 * @brief List all server commands from the command list
 */

void gecoOPCUAServer::listSrvCmds()
{
  char str[100];
  Tcl_AppendResult(interp,
                   "NUM  SERVER COMMAND\n", NULL);
  OPCUACmd *p = firstOPCUACmd;
  int i = 1;
  while (p)
  {
    sprintf(str, "%-4d %s\n", i,
            Tcl_DStringValue(p->cmd));
    Tcl_AppendResult(interp, str, NULL);
    i++;
    p = p->getNext();
  }
  Tcl_AppendResult(interp,
                   "----------------------------\n",
                   "Run `stop` followed by `start` to load/update commands to the OPC UA server", NULL);
}

/**
 * @brief Adds a method node to the OPC UA server
 */

void gecoOPCUAServer::addMethodNode(OPCUACmd *cmd)
{
  cout << "Adding method node: " << Tcl_DStringValue(cmd->cmd) << "\n";

  // Input argument
  UA_Argument inputArgument;
  UA_Argument_init(&inputArgument);
  inputArgument.description = UA_LOCALIZEDTEXT(const_cast<char *>("en-US"),
                                               const_cast<char *>("Input arguments of Tcl Script"));
  inputArgument.name = UA_STRING(const_cast<char *>("Input"));
  inputArgument.dataType = UA_TYPES[UA_TYPES_STRING].typeId;
  inputArgument.valueRank = UA_VALUERANK_SCALAR;

  // Output argument
  UA_Argument outputArgument;
  UA_Argument_init(&outputArgument);
  outputArgument.description = UA_LOCALIZEDTEXT(const_cast<char *>("en-US"),
                                                const_cast<char *>("Return Value from Tcl Script"));
  outputArgument.name = UA_STRING(const_cast<char *>("ReturnValue"));
  outputArgument.dataType = UA_TYPES[UA_TYPES_STRING].typeId;
  outputArgument.valueRank = UA_VALUERANK_SCALAR;

  // Method attributes
  UA_MethodAttributes methodAttr = UA_MethodAttributes_default;
  methodAttr.description = UA_LOCALIZEDTEXT(const_cast<char *>("en-US"), const_cast<char *>("TclScript"));
  methodAttr.displayName = UA_LOCALIZEDTEXT(const_cast<char *>("en-US"), Tcl_DStringValue(cmd->cmd));
  methodAttr.executable = true;
  methodAttr.userExecutable = true;

  // Add the method node under the top node
  UA_Server_addMethodNode(
      server,
      UA_NODEID_NULL,                              // Let the server assign a NodeId
      topNodeId,                                   // Parent Node: Top node
      UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT), // Reference type
      UA_QUALIFIEDNAME(ns_idx, cmd->getCmd()),     // Qualified name
      methodAttr,                                  // Method attributes
      &tcl_script_MethodCallback,                  // Callback function
      1, &inputArgument,                           // One input argument
      1, &outputArgument,                          // One output argument
      cmd,                                         // Node context
      NULL);
}