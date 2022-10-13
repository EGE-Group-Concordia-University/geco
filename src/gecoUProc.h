// This may look like C code, but it is really -*- C++ -*-
// $Id: ECTrigger.h 15 2014-01-03 18:49:04Z wuthrich $
// ---------------------------------------------------------------- 
//                                                                  
// Header file for class gecoUProc
//
// (c) Rolf Wuthrich
//     2015 - 2020 Concordia University
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
// 23.10.2015 Creation                         R. Wuthrich
// 09.12.2020 Added doxygen documentation      R. Wuthrich
//
// ---------------------------------------------------------------

#ifndef gecoUProc_SEEN_
#define gecoUProc_SEEN_

#include <tcl.h>
#include <stdio.h>
#include "gecoProcess.h"
#include "gecoApp.h"

using namespace std;


// -----------------------------------------------------------------------
//
// Tcl interface
//

/**
 * @brief C++ implementation of the Tcl command to create a gecoUProc object
 * @param clientData pointer to the gecoApp in which the gecoUProc instance will live
 * @param interp Tcl interpreter in which the Tcl command is executed
 * @param objc number of arguments of the Tcl command
 * @param objv arguments of the the Tcl command
 * \return TCL_OK if the execution of the Tcl command is successful and TCL_ERROR otherwise
 */

int geco_UProcCmd(ClientData clientData, Tcl_Interp *interp, 
		    int objc,Tcl_Obj *const objv[]);


// -----------------------------------------------------------------------
//
// class gecoUProc : a class for User Processes
//

/** 
 * @brief A gecoProcess for a user defined process
 * \author Rolf Wuthrich
 * \date 2015-2020
 *
 * The gecoUProc class allows to create a gecoProcess for a
 * user defined process. The process is implemented as a Tcl script
 * which is called and executed at each loop of the geco process loop
 * as long as the gecoUProc is active. This Tcl script is defined
 * with the subcommand '-userProcess' by the user.
 *
 * The geco_UProcCmd() is the C++ implementation for the Tcl command to
 * create gecoUProc objects. This Tcl command is already available in 
 * the Tcl interpreter run by an instance of gecoApp under the name 'uproc'.
 *
 * Associated Tcl command
 * ----------------------
 * Every gecoUProc is associated to a Tcl command. The associated Tcl command 
 * is created during the construction of an instance of gecoUProc by its
 * parent class. The Tcl command is created in the Tcl interpreter
 * run by the gecoApp in which gecoUProc lives.
 *
 * The gecoUProc class extends the subcommands from gecoObj and gecoProcess
 * by the following subcommands
 *
 * Sub-command       | Short description
 * ----------------- | ------------------
 * -userProcess      | returns/sets user process
 */

class gecoUProc : public gecoProcess
{

protected:

  Tcl_DString* userScript;

public:

  gecoUProc(gecoApp* App) :
    gecoObj("User Process", "uproc", App),
    gecoProcess("User Process", "user", "uproc", App)
  {
    userScript = new Tcl_DString;
    Tcl_DStringInit(userScript);
    addOption("-userProcess", userScript, "returns/sets user process");
  }

  ~gecoUProc();

  virtual void handleEvent(gecoEvent* ev);
  virtual Tcl_DString* info(const char* frontStr = "");
};

#endif /* gecoUProc_SEEN_ */
