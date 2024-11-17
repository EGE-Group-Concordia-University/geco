// This may look like C code, but it is really -*- C++ -*-
// $Id: ECIO.h 37 2015-01-09 12:35:29Z wuthrich $
// ---------------------------------------------------------------- 
//                                                                  
// Header file for class gecoIO
//
// (c) Rolf Wuthrich
//     2015 - 2020 Concordia University
//
// author:  Rolf Wuthrich
// email:   rolf.wuthrich@concordia.ca
// version: v2 ($Revision: 37 $)
//
// This software is copyright under the BSD license
//
// ---------------------------------------------------------------
// history:
// ---------------------------------------------------------------
// Date       Modification                     Author
// ---------------------------------------------------------------
// 10.17.2015 Creation                         R. Wuthrich
// 08.12.2020 Added doxygen documentation      R. Wuthrich
//
// ---------------------------------------------------------------
/*! \file */

#ifndef gecoIO_SEEN_
#define gecoIO_SEEN_

#include <tcl8.6/tcl.h>
#include "gecoProcess.h"
#include "gecoIOModule.h"

using namespace std;

// -----------------------------------------------------------------------
//
// Tcl interface
//

/**
 * @brief C++ implementation of the Tcl command to create a gecoIO object
 * @param clientData pointer to the gecoApp in which the gecoIO will live
 * @param interp Tcl interpreter in which the Tcl command is executed
 * @param objc number of arguments of the Tcl command
 * @param objv arguments of the the Tcl command
 * \return TCL_OK if the execution of the Tcl command is successful and TCL_ERROR otherwise
 */

int geco_IOCmd(ClientData clientData, Tcl_Interp *interp, 
	       int objc,Tcl_Obj *const objv[]);

// -----------------------------------------------------------------------
//
// Structure for linked gecoIOModules list
//

struct linkedGecoIOModules {
  gecoIOModule*         module;
  linkedGecoIOModules*  nextModule;
};


// ---------------------------------------------------------------
//
// class gecoIO : class responsible for IO operations
//

/** 
 * @brief A gecoProcess to link a list of gecoIOModule to the geco process loop
 * \author Rolf Wuthrich
 * \date 2015-2020
 *
 * The gecoIO class allows to create a gecoProcess able to link
 * a list of gecoIOModule. Each gecoIOModule will execute the defined
 * input-output operations as long as the gecoIO process is active.
 *
 * The geco_IOCmd() is the C++ implementation for the Tcl command to
 * create gecoIO objects. This Tcl command is already available in 
 * the Tcl interpreter run by an instance of gecoApp under the name 'io'.
 *
 * Associated Tcl command
 * ----------------------
 * Every gecoIO is associated to a Tcl command. The associated Tcl command 
 * is created during the construction of an instance of gecoIO by its
 * parent class. The Tcl command is created in the Tcl interpreter
 * run by the gecoApp in which gecoIO lives.
 *
 * The gecoIO class extends the subcommands from gecoObj and gecoProcess
 * by the following subcommands
 *
 * Sub-command       | Short description
 * ----------------- | ------------------
 * -linkModule       | links an IO-module
 * -unlinkModule     | unlinks an IO-module
 * -listLinkModules  | list linked IO-modules
 * -update           | updates all IO operations
 */

class gecoIO : public gecoProcess
{

protected:

  linkedGecoIOModules* firstGecoIOModule;
  
public:

  gecoIO(gecoApp* App);
  ~gecoIO();

  virtual int  cmd(int &i,int objc,Tcl_Obj *const objv[]);
  virtual void handleEvent(gecoEvent* ev);
  virtual Tcl_DString* info(const char* frontStr = "");

  virtual void activate(gecoEvent* ev);

  int  addGecoIOModule(gecoIOModule* newModule);
  int  removeGecoIOModule(char* TclCmdOfModule);
  void listGecoIOModules(Tcl_DString* str);
};


#endif /* gecoIO_SEEN_ */
