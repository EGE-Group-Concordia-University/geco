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
// class gecoOPCUAServer
//

class gecoOPCUAServer : public gecoProcess
{

protected:
    int port; /*!< port of OPC UA server */
    Tcl_DString *opcua_namespace;
    Tcl_DString *browseName;
    Tcl_DString *displayName;

    UA_Server *server;    // OPC UA server
    UA_NodeId topNodeId;  // Top node in server
    UA_UInt16 ns_idx;     // Server namesapce index
    pthread_t thread;
    LinkedVariable *firstLinkedVariable;

public:
    gecoOPCUAServer(const char *serverName, const char *serverCmd, int portID, gecoApp *App);
    virtual ~gecoOPCUAServer();

    virtual int cmd(int &i, int objc, Tcl_Obj *const objv[]);
    virtual void handleEvent(gecoEvent* ev);
    virtual Tcl_DString *info(const char *frontStr = "");

    virtual void terminate(gecoEvent *ev);
    virtual void activate(gecoEvent *ev);

    LinkedVariable *getFirstLinkedVariable() { return firstLinkedVariable; }
    void addLinkedVariable(LinkedVariable *var);
    void removeLinkedVariable(LinkedVariable *var);
    LinkedVariable *findLinkedVariable(const char *TclVar);
    void listLinkedVar();

    void addVariableNode(LinkedVariable *var);
};

#endif /* gecoOPCUAServer_SEEN_ */
