// This may look like C code, but it is really -*- C++ -*-
// $Id: ECObj.h 25 2014-03-16 17:03:44Z wuthrich $
// ---------------------------------------------------------------- 
//                                                                  
// Header file for class gecoObj
//
// (c) Rolf Wuthrich
//     2015-2022 Concordia University
//
// author:  Rolf Wuthrich
// email:   rolf.wuthrich@concordia.ca
// version: $Revision: 25 $
//
// This software is copyright under the BSD license
//
// ---------------------------------------------------------------
// history:
// ---------------------------------------------------------------
// Date       Modification                     Author
// ---------------------------------------------------------------
// 17.10.2015 Creation                         R. Wuthrich
// 23.11.2020 Added doxygen documentation      R. Wuthrich
// 27.04.2022 Added addInfo for Tcl_DString    R. Wuthrich
//
// ---------------------------------------------------------------
/*! \file */

#ifndef gecoObj_SEEN_
#define gecoObj_SEEN_

#include <tcl.h>
#include <vector>
#include "gecoApp.h"


using namespace std;


// -------------------------------------------------------------------------
//
// Tcl interface

/**
 * @brief C++ implementation of the Tcl command to manipulate an existing gecoObj object
 * @param clientData pointer to the gecoObj to which the Tcl command is associated
 * @param interp Tcl interpreter in which the Tcl command is executed
 * @param objc number of arguments of the Tcl command
 * @param objv arguments of the the Tcl command
 * \return TCL_OK if the execution of the Tcl command is successful and TCL_ERROR otherwise
 */

int geco_gecoObjCmd(ClientData clientData, Tcl_Interp *interp, 
	 	    int objc,Tcl_Obj *const objv[]);


// -------------------------------------------------------------------------
//
// Structure needed to maintain list of options with associated variables

struct AssocVar
{
  ClientData     varPtr;
  int            varType;
  Tcl_DString*   help;
};

const int
  geco_NO_VAR   = 0,
  geco_INT      = 1,
  geco_DOUBLE   = 2,
  geco_DSTRING  = 3,
  geco_BOOL     = 4;


// -----------------------------------------------------------------------
//
// class gecoObj : abstract class from which all geCo objects derive
//

/** 
 * @brief Abstract class from which all geco objects derive
 * \author Rolf Wuthrich
 * \date 2015-2020
 *
 *
 * The gecoObj class is the base class from which all geco objects derive.
 * The class implements an interface to the associate Tcl command defined in
 * the Tcl interpreter of the gecoApp in which the gecoObj lives.
 *
 * It implements as well a mechanism to add sub-commands (options) to the
 * main Tcl command and the geco help system.
 *
 * At its creation, the gecoObj class will attributed a unique ID to the instance and
 * create a Tcl command in the Tcl interpreter run by the gecoApp in which the gecoObj
 * lives.
 *
 * Associated Tcl command
 * ----------------------
 * Every gecoObj is associated to a Tcl command. The associated Tcl command 
 * is created during the construction of an instance of gecoObj. The Tcl command
 * is created in the Tcl interpreter run by the gecoApp in which the gecoObj
 * lives.
 *
 * The function geco_gecoObjCmd() implements the geco help system and the 
 * parsing of subcommands. Subcommands can be conveniently added to children 
 * of a gecoObj with the method gecoObj::addOption. The gecoObj class 
 * implements the following subcommands
 *
 * Sub-command | Short description
 * ----------- | ------------------
 * -help       | displays help information for each defined sub-command
 * -name       | returns the name of the gecoObj
 * -info       | returns information relevant to the gecoObj
 * -comment    | returns/sets a comment 
 *
 * The function geco_gecoObjCmd() loops over all arguments of the Tcl command in order
 * to execute the various subcommands by calling iteratively the gecoObj::cmd method.
 *
 * If the Tcl command is called without any subcommand, then geco_gecoObjCmd()
 * will leave the ID of the gecoObj in the Tcl interpreter as return value.
 *
 * The geco help system
 * --------------------
 * Tcl commands defined by the geco library have the sub-command '-help'. This help system is managed
 * by the class gecoObj. Every sub-command defined via the gecoObj::addOption method will automatically
 * be registered in the help system and no further action is required.
 *
 * The '-info' sub-command
 * ----------------------- 
 * The gecoObj class implements the '-info' subcommand. The geco::info method
 * is called by gecoObj::cmd when the '-info' subcommand was passed by the user on 
 * invoking the associated Tcl command to the gecoObj.
 *
 * By default the '-info' subcommand displays the name of the gecoObj. 
 * Children can, by extending the gecoObj::info method to add additional 
 * relevant information.
 * 
 * Extending the subcommand set
 * ----------------------------
 * Additional subcommands of the Tcl command associated tot he gecoObj can be implemented by 
 * children with the method gecoObj::addOption. The method will as well define the help text, 
 * displayed by the '-help' subcommand. The gecoClass uses the mechanism of the Tcl C API to
 * handle subcommands. For this the class stores all subcommands in an internal array. 
 *
 * Subcommands are processed by the gecoObj::cmd method. Children must overload this method in
 * order to have processed their own subcommands. For this they need to first call the 
 * geco::cmd method from the gecoObj class which will return the index of the subcommand to process.
 * To match the index with a subcommand, the method gecoObj::getOptionIndex can be used.
 * 
 * The simplest version is geco::addOption(const char* opt, const char* help). This will define a 
 * new subcommand opt. 
 *
 * The other versions, such as gecoObj::addOption(const char* opt, double* dblPtr, const char* help),
 * allow to have subcommands of the form '-subcmd ?val' where val is an optional value (a floating number
 * in the present example). When the user uses the subcommand as '-subcmd 12.3', the number 12.3 will 
 * automatically be stored in the location pointed to by dblPtr. If used in the version '-subcmd'
 * then the value stored at the location pointed to by dblPtr will be returned to the user.
 *
 * Example
 * -------
 * The following code is an example for creating a new class based on gecoObj that
 * has two subcommands added ('-x' and 'squareX') in the the corresponding Tcl command.
 * The associated Tcl command is named 'gecoObjEx'.
 * The '-info' subcommand was extended by displaying the content of the variable x.
 *
 * The definition of the new class and the C-function for implementing the Tcl command are
 * as follows. Two methods need to be overloaded. The method cmd in order to 
 * to implement the new '-squareX' subcommand and the info method to implement the display
 * of the variable x.
 *
 * \include gecoObjEx.h
 * The implementation of the new class and the C-function implementing the Tcl command are
 * as follows. Note that only the subcommand '-squareX' needs to be implemented.
 * The subcommand '-x' is already handled by the class gecoObj.
 * \include gecoObjEx.cc
 *
 * To use the new class and add its Tcl command to the Tcl interpreter run by the gecoApp,
 * one creates an instance of the class:
 * \code
 * // Creates an instance of the new class
 * // app is pointing to the gecoApp in which the new gecoObj is added
 * gecoObjExCmd = new gecoObjEx(app); 
 * \endcode
 * The creation of an instance of gecoObjEx will create a Tcl command in the gecoApp 
 * passed as argument in the constructor (the pointer app in our example).
 */
 

class gecoObj
{
private:

  static int       IDcounter;
  vector<AssocVar> assVars;
  int              addOpt(const char* opt);

protected:

  char             objID[8];    /*!<  unique ID of the gecoObj */
  const char*      name;        /*!<  name of the gecoObj  */
  Tcl_DString*     comment;     /*!<  comment associated to the gecoObj  */

  Tcl_DString*     TclCmd;      /*!<  Tcl command associated to the gecoObj  */
  char**           cmdOpts;     /*!<  table with all subcommands of the gecoObj  */
  Tcl_Interp*      interp;      /*!<  Tcl interpreter in which gecoObj lives  */
  gecoApp*         app;         /*!<  gecoApp in which gecoObj lives  */

  Tcl_DString*     infoStr;     /*!<  used ot build the info of the gecoObj in the gecoObj::info method  */

public:

  gecoObj(const char* objName, const char* objCmd, gecoApp* App, bool addIDtoCmd = true);
  virtual ~gecoObj();

  int           addOption(const char* opt, const char* help);
  int           addOption(const char* opt, double* dblPtr, const char* help);
  int           addOption(const char* opt, int* intPtr, const char* help);
  int           addOption(const char* opt, Tcl_DString* DStrPtr, const char* help);
  int           addOption(const char* opt, bool* boolPtr, const char* help);
  int           getOptionIndex(const char* opt);
  const char**  optsTbl()  {return (const char **)cmdOpts;}  /*!<  Returns the subcommands table */
  
  virtual int   cmd(int &i, int objc, Tcl_Obj *const objv[]);

  virtual Tcl_DString* info(const char* frontStr = "");
  void          addInfo(const char* frontStr, const char* text, const char* var);
  void          addInfo(const char* frontStr, const char* text, const Tcl_DString* var);
  void          addInfo(const char* frontStr, const char* text, int var);
  void          addInfo(const char* frontStr, const char* text, double var);

  const char*   getID()          {return objID;}    /*!<  Returns the unique ID of the gecoObj */
  const char*   getGecoObjName() {return name;}     /*!<  Returns the name gecoObj */
  Tcl_DString*  getComment()     {return comment;}  /*!<  Returns the comment associated to the gecoObj */
  const char*   getTclCmd()      {return Tcl_DStringValue(TclCmd);} /*!<  Returns the associated Tcl command */
  gecoApp*      getGecoApp()     {return app;}      /*!<  Returns the gecoApp in which the gecoObj lives */
  Tcl_Interp*   getTclInterp()   {return interp;}   /*!<  Returns the Tcl interpreter in which the 
                                                          associated Tcl command is defined */
};

#endif /* gecoObj_SEEN_ */
