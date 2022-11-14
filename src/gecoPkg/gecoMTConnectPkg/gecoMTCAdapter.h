// This may look like C code, but it is really -*- C++ -*-
// ---------------------------------------------------------------- 
//                                                                  
// Header file for class gecoMTCAdapter
//
// (c) Rolf Wuthrich
//     2022 Concordia University
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
// 28.04.2022 Creation                         R. Wuthrich
//
// ---------------------------------------------------------------
/*! \file */

#ifndef gecoMTCAdapter_SEEN_
#define gecoMTCAdapter_SEEN_

#include <tcl.h>
#include "gecoProcess.h"
#include "gecoApp.h"
#include "gecoMTCBaseAdapter.h"

using namespace std;


// -----------------------------------------------------------------------
//
// Tcl interface
//

/**
 * @brief C++ implementation of the Tcl command to create a gecoMTCAdapter object
 * @param clientData pointer to the gecoApp in which the gecoMTCAdapter instance will live
 * @param interp Tcl interpreter in which the Tcl command is executed
 * @param objc number of arguments of the Tcl command
 * @param objv arguments of the the Tcl command
 * \return TCL_OK if the execution of the Tcl command is successful and TCL_ERROR otherwise
 */

int geco_MTCAdapterCmd(ClientData clientData, Tcl_Interp *interp, 
		       int objc,Tcl_Obj *const objv[]);


// -----------------------------------------------------------------------
//
// Class to store linked Tcl variables and MTConnect keys 
// to be sent to the agent
//

class SHDRInsn 
{

  friend class gecoMTCAdapter;

protected:

  Tcl_DString*   TclVar;           // Tcl Variable
  Tcl_DString*   MTCKey;           // MTConnect key
  Tcl_DString*   SHDR;             // SHDR data
  char*          TclVarValue;      // Data stored by the Tcl Variable
  Tcl_DString*   TclVarOldValue;   // Previous data stored by the Tcl Variable
  SHDRInsn*      next;
  Tcl_Interp     *interp;
  

public:

  SHDRInsn(const char* Tcl_Var, const char* MTC_Key, Tcl_Interp *interp);
  ~SHDRInsn();

  SHDRInsn* getNext() {return next;}
  char*     getSHDR(bool forceSend = false);
};

// -----------------------------------------------------------------------
//
// class gecoMTCAdapter : a class for streaming data to a MTConnect Agent
//

/** 
 * @brief A gecoProcess to stream data to an MTConnect agent
 * \author Rolf Wuthrich
 * \date 2022
 *
 * The gecoMTCAdapter class allows to create a gecoProcess able to stream
 * data, in form of Tcl variables defined in the Tcl interpreter of the 
 * gecoApp in which the gecoMTCAdapter instance lives, to a MTConnet agent.
 *
 * The geco_MTCAdapterCmd() is the C++ implementation for the Tcl command to
 * create gecoMTCAdapter objects. This Tcl command is already available in 
 * the Tcl interpreter where the gecoMTConnectPkg is loaded under the name 'mtcadapter'.
 *
 * Associated Tcl command
 * ----------------------
 * Every gecoMTCAdapter is associated to a Tcl command. The associated Tcl command 
 * is created during the construction of an instance of gecoMTCAdapter by its
 * parent class. The Tcl command is created in the Tcl interpreter
 * run by the gecoApp in which gecoMTCAdapter lives.
 *
 * The gecoMTCAdapter class extends the subcommands from gecoObj, gecoProcess
 * and gecoMTCBaseAdapter by the following subcommands
 *
 * Sub-command       | Short description
 * ----------------- | ------------------
 * -linkTclVariable  | links a Tcl variable
 */

class gecoMTCAdapter : public gecoMTCBaseAdapter
{
protected:

  SHDRInsn*  firstSHDRInsn;


public:

  gecoMTCAdapter(const char* adaptName, const char* adaptCmd, int portID, gecoApp* App, bool addIDtoCmd = true) :
    gecoObj(adaptName, adaptCmd, App, addIDtoCmd),
    gecoMTCBaseAdapter(adaptName, adaptCmd, portID, App)
  {
    firstSHDRInsn = NULL;
    addOption("-linkTclVariable", "links a Tcl variable");
    addOption("-unlinkTclVariable", "removes link to a Tcl variable");
    addOption("-listLinkedTclVariables", "list all linked Tcl variables");
  }

  ~gecoMTCAdapter();

  virtual const char* adapterVersion() {return "gecoMTCAdapter v1.0";}

  virtual int cmd(int &i,int objc,Tcl_Obj *const objv[]);
  virtual void handleEvent(gecoEvent* ev);
  virtual Tcl_DString* info(const char* frontStr = "");

  virtual void terminate(gecoEvent* ev);
  virtual void activate(gecoEvent* ev);

  virtual void         initialSHDR();
  virtual Tcl_DString* SHDR(bool forceSend = false);

  void      listLinkedVar();
  SHDRInsn* findLinkedTclVar(const char* TclVar);

  void      addInsn(SHDRInsn* insn);
  void      removeInsn(SHDRInsn* insn);
  SHDRInsn* getFirstSHDRInsn() {return firstSHDRInsn;}

};

#endif /* gecoMTCAdapter_SEEN_ */
