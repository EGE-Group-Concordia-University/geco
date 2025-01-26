// ----------------------------------------------------------------
//
// Header file for the class gecoOPCUAServer
//
// (c) Rolf Wuthrich
//     2024 Concordia University
//
// author:    Rolf Wuthrich
// email:     rolf.wuthrich@concordia.ca
//
// This software is copyright under the BSD license

#ifndef gecoOPCUAServer_SEEN_
#define gecoOPCUAServer_SEEN_

#include <tcl8.6/tcl.h>
#include <open62541/server.h>
#include "gecoProcess.h"
#include "gecoApp.h"

using namespace std;

// -----------------------------------------------------------------------
//
// Tcl interface
//

int gecoOPCUAServerCmd(ClientData clientData, Tcl_Interp *interp,
                       int objc, Tcl_Obj *const objv[]);

// -----------------------------------------------------------------------
//
// Class to store linked Tcl variables with the OPC UA server
//

class LinkedVariable
{

    friend class gecoOPCUAServer;

protected:
    Tcl_DString *TclVar;
    char *TclVarValue;
    Tcl_DString *browseName;
    Tcl_DString *displayName;
    LinkedVariable *next;
    Tcl_Interp *interp;
    UA_VariableAttributes ua_attr;
    UA_NodeId varNodeId;

public:
    LinkedVariable(const char *Tcl_Var, const char *browseName, const char *displayName, Tcl_Interp *interp);
    ~LinkedVariable();

    LinkedVariable *getNext() { return next; }
};

// -----------------------------------------------------------------------
//
// Class to store linked server commands
//

/**
 * @brief A class to store OPC UA server commands
 *
 * The OPCUACmd class stores commands
 * for the usage of the gecoOPCUAServer class.
 */

class OPCUACmd
{

    friend class gecoOPCUAServer;

private:
    OPCUACmd *next;

protected:
    Tcl_DString *cmd;
    Tcl_DString *description;
    gecoApp *app;

public:
    OPCUACmd(const char *Srv_Cmd, const char *cmd_description, gecoApp *geco_app);
    ~OPCUACmd();

    OPCUACmd *getNext() { return next; }
    char *getCmd() { return Tcl_DStringValue(cmd); }
    gecoApp *getGecoApp() { return app; }
};

// -----------------------------------------------------------------------
//
// class gecoOPCUAServer
//

class gecoOPCUAServer : public gecoProcess
{

protected:
    int port; /*!< port of OPC UA server */
    Tcl_DString *opcua_namespace;
    Tcl_DString *browseName;
    Tcl_DString *displayName;

    UA_Server *server;   // OPC UA server
    UA_NodeId topNodeId; // Top node in server
    UA_UInt16 ns_idx;    // Server namesapce index
    pthread_t thread;
    LinkedVariable *firstLinkedVariable;
    OPCUACmd *firstOPCUACmd; // Pointer to list of server commands

public:
    gecoOPCUAServer(const char *serverName, const char *serverCmd, int portID, gecoApp *App, bool addID = true);
    virtual ~gecoOPCUAServer();

    virtual int cmd(int &i, int objc, Tcl_Obj *const objv[]);
    virtual void handleEvent(gecoEvent *ev);
    virtual Tcl_DString *info(const char *frontStr = "");

    virtual void terminate(gecoEvent *ev);
    virtual void activate(gecoEvent *ev);

    LinkedVariable *getFirstLinkedVariable() { return firstLinkedVariable; }
    void addLinkedVariable(LinkedVariable *var);
    void removeLinkedVariable(LinkedVariable *var);
    LinkedVariable *findLinkedVariable(const char *TclVar);
    void listLinkedVar();
    void addVariableNode(LinkedVariable *var);

    int addSrvCmd(OPCUACmd *cmd);
    void removeSrvCmd(OPCUACmd *cmd);
    OPCUACmd *findServerCmd(const char *cmd);
    void listSrvCmds();
    void addMethodNode(OPCUACmd *cmd);
};

#endif /* gecoOPCUAServer_SEEN_ */
