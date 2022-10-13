// This may look like C code, but it is really -*- C++ -*-
// $Id: ECIOModule.h 37 2015-01-09 12:35:29Z wuthrich $
// ---------------------------------------------------------------- 
//                                                                  
// Header file for the class gecoIOModule
//
// (c) Rolf Wuthrich
//     2015 - 2021 Concordia University
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
// 17.10.2015 Creation                         R. Wuthrich
// 10.11.2020 General update                   R. Wuthrich
// 12.12.2020 Added doxygen documentation      R. Wuthrich
// 29.05.2021 Major revision                   R. Wuthrich
// ---------------------------------------------------------------

#ifndef gecoIOModule_SEEN_
#define gecoIOModule_SEEN_

#include <tcl.h>
#include "gecoObj.h"
#include "gecoEvent.h"

using namespace std;


const int
  TclVarRead  = 0,
  TclVarWrite = 1;


// -----------------------------------------------------------------------
//
// Class to store link between Tcl variables and IO instructions 
//

/** 
 * @brief A class to store link between Tcl variables and IO instructions
 *
 * The IOModuleInsn class stores Tcl variables that are linked to
 * input/output operations in a gecoIOModule.
 */

class IOModuleInsn
{

  friend class gecoIOModule;

protected:

  Tcl_DString*   TclVar;         // name of associated Tcl variable
  int            TclVarType;     // either TclVarRead or TclVarWrite
  int            chanID;         // ID of the channel used to communicate (not all modules need this)
  IOModuleInsn*  next;

public:

  IOModuleInsn(const char* Tcl_Var, int tclVarType, int ChanID = 0);
  virtual ~IOModuleInsn();

  IOModuleInsn* getNext()       {return next;}
  int           getVarType()    {return TclVarType;}
  Tcl_DString*  getTclVar()     {return TclVar;}
  int           getChanID()     {return chanID;}

};


// -----------------------------------------------------------------------
//
// class gecoIOModule : abstract class for geCo IO-Modules 
//

/** 
 * @brief Abstract class from which all geco IO-modules derive
 * \author Rolf Wuthrich
 * \date 2015-2021
 *
 *
 * The gecoIOModule class is the abstract class from which all geco IO-modules derive.
 * The class implements an interface to the associated Tcl command defined in
 * the Tcl interpreter of the gecoApp in which the gecoProcess lives.
 *
 * The gecoIOModule class is a derived class of gecoObj.
 *
 * Geco IO-modules are inserted into a list maintained by the gecoApp
 * during the construction of an instance of gecoIOModule.
 *
 * Geco IO-modules
 * ---------------
 * A geco IO-module is responsible for performing input/output operations
 * and store the transfered data in a Tcl variable. Children of gecoIOModule
 * have to implement how the IO operations have to be done (e.g. over 
 * an analogue IO card, a Tcp connection, ...).
 * 
 * A gecoIOModule serves as an hardware abstraction layer (HAL). It can
 * model a wide variety of hardware IO operations such as performed
 * by IO cards, GPIO buses or Tcp sockets.
 *
 * The gecoApp in which the gecoIOModule was created, maintains a list
 * of created IO-modules. When gecoApp is destroyed, it will take care
 * of destroying all IO-Modules in this list. 
 *
 * A geco IO-module can be linked to the geco process loop with a gecoIO process.
 * Several gecoIOModule can be linked to a same gecoIO process. 
 *
 * IO-module instructions
 * ----------------------
 * Each gecoIOModule maintains a list of IOModuleInsn. A IOModuleInsn is
 * a scheduled input/output operation which will have to be performed
 * by the gecoIOModule. 
 *
 * The IOModuleInsn can be executed interactively by the user of the gecoApp
 * using one of the subcommands '-doIOoperations' or '-update'. 
 * When linked to a gecoIO process, the operations will be executed
 * by the geco process loop whenever the gecoIO process is executed.
 *
 * A IOModuleInsn is always linked to a Tcl variable. It can be either
 * an input our an output operation. In case of an input operation,
 * the data red by the gecoIOModule will be copied into the Tcl variable.
 * In case of an output operation, the content of the Tcl variable 
 * will be written to the output by the gecoIOModule.
 *
 * Associated Tcl command
 * ----------------------
 * Every gecoIOModule is associated to a Tcl command. The associated Tcl command 
 * is created during the construction of an instance of gecoIOModule by its
 * parent class. The Tcl command is created in the Tcl interpreter
 * run by the gecoApp in which gecoIOModule lives.
 *
 * The gecoIOModule class extends the subcommands from gecoObj
 * by the following subcommands
 *
 * Sub-command        | Short description
 * ------------------ | -----------------------------------------------------
 * -listIOoperations  | list scheduled IO operations
 * -doIOoperations    | executes all scheduled IO operations
 * -update            | executes the IO operaton associated to a Tcl variable
 * -unlinkTclVariable | unlinks a Tcl variable
 * -close             | closes the io-module
 */

class gecoIOModule : public gecoObj
{

protected:

  Tcl_Namespace* TclNamespace;       // associated Tcl namespace

  IOModuleInsn*  firstIOModuleInsn;  // Pointer to list of IOMOdule instructions
  gecoIOModule*  nextGecoIOModule;   // Pointer to next gecoIOModule

  int linkedToGecoStatus;            // 1 if linked to geCo process loop

public:

  gecoIOModule(const char* moduleName, const char* moduleCmd, gecoApp* App);
  virtual ~gecoIOModule();

  virtual int   cmd(int &i,int objc,Tcl_Obj *const objv[]);
  virtual void  activate(gecoEvent* ev);

  virtual Tcl_DString* info(const char* frontStr = "");

  IOModuleInsn* getFirstInsn() {return firstIOModuleInsn;}
  int           addInsn(IOModuleInsn* insn);
  void          removeInsn(IOModuleInsn* insn);

  virtual void  listInstr();
  virtual int   doInstr();
  virtual int   update(const char* Tcl_Var);
  virtual void  IOerror();

  IOModuleInsn* findLinkedTclVariable(const char* TclVar);

  void setLinkedToGeco()    {linkedToGecoStatus=1;}
  void setUnlinkedToGeco()  {linkedToGecoStatus=0;}
  int  linkedToGeco()       {return linkedToGecoStatus;}

  gecoIOModule* getNextGecoIOModule() {return nextGecoIOModule;}
  void          setNextGecoIOModule(gecoIOModule* mod) {nextGecoIOModule=mod;}
};


#endif /* gecoIOModule_SEEN_ */
