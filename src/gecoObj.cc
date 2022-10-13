// $Id: ECObj.cc 39 2015-01-09 13:09:42Z wuthrich $
// ---------------------------------------------------------------
//
// Definition of the class gecoObj
//
// (c) Rolf Wuthrich
//     2015-2020 Concordia University
//
// author:  Rolf Wuthrich
// email:   rolf.wuthrich@concordia.ca
// version: v1 ($Revision: 39 $)
//
// This software is copyright under the BSD license
//  
// ---------------------------------------------------------------
// history:
// ---------------------------------------------------------------
// Date       Modification                     Author
// ---------------------------------------------------------------
// 10.10.2015 Creation                         R. Wuthrich
// 23.11.2020 Added Doxygen documentation      R. Wuthrich
// 25.11.2020 Added call to geco_gecoObjCmd    R. Wuthrich
//            in constructor
// ---------------------------------------------------------------

#include <cstdlib>
#include <cstring>
#include <tcl.h>
#include "gecoObj.h"

#include <iostream>

using namespace std;

int gecoObj::IDcounter = 0;


// -------------------------------------------------------------------------
//
// Tcl interface


// Tcl_ObjCmdProc implementing the Tcl command associated to an existing gecoObj object
//

int geco_gecoObjCmd(ClientData clientData, Tcl_Interp *interp, 
	            int objc, Tcl_Obj *const objv[])
{
  gecoObj* gecoobj=(gecoObj *)clientData;

  if (objc==1)
    {
      Tcl_AppendResult(interp, gecoobj->getID(), NULL);
      return TCL_OK;
    }

  int i = 1;
  while (i<objc)
    if (gecoobj->cmd(i, objc, objv)==-1) return TCL_ERROR;

  return TCL_OK;
}


// -----------------------------------------------------------------------
//
// class gecoObj : abstract class from which all geco objects derive
//


/**
 * @brief Constructor
 * @param objName Name of the gecoObj
 * @param objCmd Tcl command associated to the gecoObj
 * @param App gecoApp in which the gecoObj lives
 * @param addIDtoCmd If set to true will add the gecoObj ID behind the objCmd 
 *        to create the Tcl command. If set to false, uses objCmd as Tcl command name 
 *
 * The constructor will as well create the associated Tcl command in the Tcl interpreter 
 * run by App.
 */

gecoObj::gecoObj(const char* objName, const char* objCmd, gecoApp* App, bool addIDtoCmd) :
  app(App),
  name(objName),
  cmdOpts(NULL)
{
  interp = App->getInterp();
  IDcounter++;
  sprintf(objID, "%d", IDcounter);
  TclCmd  = new Tcl_DString;
  infoStr = new Tcl_DString;
  comment = new Tcl_DString;
  Tcl_DStringInit(TclCmd);
  if (objCmd!=NULL)
    {
      Tcl_DStringAppend(TclCmd, objCmd, -1);
      if (addIDtoCmd) Tcl_DStringAppend(TclCmd, objID, -1);
    }
  Tcl_DStringInit(infoStr);
  Tcl_DStringInit(comment);
  Tcl_DStringAppend(comment, "", -1);

  addOption("-help", "displays this message");
  addOption("-name", "returns name of process");
  addOption("-info", "returns info");
  addOption("-comment", comment, "returns/sets a comment");
  
  // creates associated Tcl command
  Tcl_CreateObjCommand(interp,
		       Tcl_DStringValue(TclCmd),   // Tcl command
		       geco_gecoObjCmd,            // C++ implementation of Tcl command
		       (ClientData) this,          // stores link to gecoObj
		       (Tcl_CmdDeleteProc *) NULL);
}


/**
 * @brief Destructor
 * The destructor removes the Tcl command created in the constructor
 */

gecoObj::~gecoObj()
{
  Tcl_DeleteCommand(interp, Tcl_DStringValue(TclCmd));
  Tcl_DStringFree(TclCmd);
  Tcl_DStringFree(infoStr);
  delete TclCmd;
  delete infoStr;
  int i = 0;
  while (cmdOpts[i]!=NULL)
    {
      Tcl_DStringFree(assVars[i].help);
      delete assVars[i].help;
      delete cmdOpts[i];
      i++;
    }
}


// ---- ADDOPTION : adds an option to the option table
//

int gecoObj::addOpt(const char* opt)
{
  int i=0;

  if (cmdOpts!=NULL)
    while (cmdOpts[i]!=NULL)
      i++;

  cmdOpts = (char **)realloc(cmdOpts,(i+2) * sizeof *cmdOpts);
  cmdOpts[i] = (char *)malloc(strlen(opt) + 1);
  strcpy(cmdOpts[i], opt);
  cmdOpts[i+1] = NULL;
  return i;
}


/**
 * @brief Adds a subcommand of the type '-subcmd' to the Tcl command of the gecoObj
 * @param opt Tcl subcommand of the gecoObj
 * @param help Short help text describing the subcommand 
 * \return index of the new subcommand
 */
 
int gecoObj::addOption(const char* opt, const char* help)
{
  int i=addOpt(opt);
  AssocVar av;
  av.varPtr = NULL;
  av.varType = geco_NO_VAR;
  av.help = new Tcl_DString;
  Tcl_DStringInit(av.help);
  Tcl_DStringAppend(av.help, help, -1);
  assVars.push_back(av);
  return i;
}


/**
 * @brief Adds a subcommand of the type '-subcmd ?val' to the Tcl command of the gecoObj
 * @param opt Tcl subcommand of the gecoObj
 * @param dblPtr pointer to a double type variable which holds the value of val
 * @param help Short help text describing the subcommand 
 * \return index of the new subcommand
 */
 
int gecoObj::addOption(const char* opt, double* dblPtr, const char* help)
{
  int i = addOpt(opt);
  AssocVar av;
  av.varPtr = dblPtr;
  av.varType = geco_DOUBLE;
  av.help = new Tcl_DString;
  Tcl_DStringInit(av.help);
  Tcl_DStringAppend(av.help, help, -1);
  assVars.push_back(av);
  return i;
}


/**
 * @brief Adds a subcommand of the type '-subcmd ?val' to the Tcl command of the gecoObj
 * @param opt Tcl subcommand of the gecoObj
 * @param intPtr pointer to a int type variable which holds the value of val
 * @param help Short help text describing the subcommand 
 * \return index of the new subcommand
 */
 
int gecoObj::addOption(const char* opt, int* intPtr, const char* help)
{
  int i = addOpt(opt);
  AssocVar av;
  av.varPtr = intPtr;
  av.varType = geco_INT;
  av.help = new Tcl_DString;
  Tcl_DStringInit(av.help);
  Tcl_DStringAppend(av.help, help, -1);
  assVars.push_back(av);
  return i;
}


/**
 * @brief Adds a subcommand of the type '-subcmd ?val' to the Tcl command of the gecoObj
 * @param opt Tcl subcommand of the gecoObj
 * @param DStrPtr pointer to a Tcl_DString type variable which holds the value of val
 * @param help Short help text describing the subcommand
 * \return index of the new subcommand
 */

int gecoObj::addOption(const char* opt, Tcl_DString* DStrPtr, const char* help)
{
  int i = addOpt(opt);
  AssocVar av;
  av.varPtr = DStrPtr;
  av.varType = geco_DSTRING;
  av.help = new Tcl_DString;
  Tcl_DStringInit(av.help);
  Tcl_DStringAppend(av.help, help, -1);
  assVars.push_back(av);
  return i;
}


/**
 * @brief Adds a subcommand of the type '-subcmd ?val' to the Tcl command of the gecoObj
 * @param opt Tcl subcommand of the gecoObj
 * @param boolPtr pointer to a bool type variable which holds the value of val
 * @param help Short help text describing the subcommand
 * \return index of the new subcommand
 */

int gecoObj::addOption(const char* opt, bool* boolPtr, const char* help)
{
  int i = addOpt(opt);
  AssocVar av;
  av.varPtr = boolPtr;
  av.varType = geco_BOOL;
  av.help = new Tcl_DString;
  Tcl_DStringInit(av.help);
  Tcl_DStringAppend(av.help, help, -1);
  assVars.push_back(av);
  return i;
}


/**
 * @brief Returns the index number of a subcommand registered with gecoObj::addOpt
 * @param opt Tcl subcommand registered with gecoObj::addOpt
 * \return Index number of a subcommand opt
 */

int gecoObj::getOptionIndex(const char* opt)
{
  int i;
  Tcl_GetIndexFromObj(NULL, Tcl_NewStringObj(opt,-1), optsTbl(), "", 0, &i);
  return i;
}


/**
 * @brief Process an argument of the Tcl command and checks if it matches to a subcommand
 * @param i number of the argument
 * @param objc total number of arguments
 * @param objv array of all arguments
 * \return index of the subcommand matched by the argument number i and -1 if no match was found
 *
 * Has to be extended in children to handle subcommands of children
 */

int gecoObj::cmd(int &i, int objc, Tcl_Obj *const objv[])
{
  char st[100];
  int index = -1;
  int j = 0;
  Tcl_GetIndexFromObj(interp, objv[i], optsTbl(), "option", 0, &index);
  if (index==-1) return -1;

  // handles the options -help, -info, -name and -nRecords
  switch (index)
    {

    case 0: // help
      Tcl_AppendResult(interp, "\n",Tcl_GetString(objv[0]), " - ", name, NULL);
      Tcl_AppendResult(interp, "\n\nValid options are:\n\n", NULL);
      while (cmdOpts[j]!=NULL)
	{
	  sprintf(st, "%-20.20s %-58.58s\n", cmdOpts[j],
		  Tcl_DStringValue(assVars[j].help));
	  Tcl_AppendResult(interp, st, NULL);
	  j++;
	}
      Tcl_AppendResult(interp, "\nFor more details see man page.", NULL);
      i++;
      break;

    case 1: // name
      Tcl_AppendResult(interp, name, NULL);
      i++;
      break;

    case 2: // info
      Tcl_DStringResult(interp, info());
      if (strlen(Tcl_DStringValue(getComment()))>0)
	Tcl_AppendResult(interp, "\ncomment : ", Tcl_DStringValue(getComment()),NULL);
      i++;
      break;

    }

  // handles the options with associated variables
  char str[TCL_DOUBLE_SPACE];
  double* pd;
  int* pi;
  Tcl_DString* pstr;
  bool* pb;

  switch (assVars[index].varType)
    {

    case geco_DOUBLE:
      pd = (double*)assVars[index].varPtr;
      if (i+1 <= objc-1)
	{
	  if (Tcl_GetDoubleFromObj(interp,objv[i+1], pd)!=TCL_OK) return -1;
	  i++;
	}
      else
	{
	  Tcl_PrintDouble(interp, *pd, str);
	  Tcl_AppendResult(interp, str, NULL);
	}
      i++;
      break;

    case geco_INT:
      pi = (int*)assVars[index].varPtr;
      if (i+1 <= objc-1)
	{
	  if (Tcl_GetIntFromObj(interp, objv[i+1], pi)!=TCL_OK) return -1;
	  i++;
	}
      else
	{
	  sprintf(str, "%d", *pi);
	  Tcl_AppendResult(interp, str, NULL);
	}
      i++;
      break;

    case geco_DSTRING:
      pstr = (Tcl_DString *)assVars[index].varPtr;
      if ((i+1 <= objc-1)&&(Tcl_StringMatch(Tcl_GetString(objv[i+1]), "-*")==0))
	{
	  Tcl_DStringFree(pstr);
	  Tcl_DStringAppend(pstr, Tcl_GetString(objv[i+1]), -1);
	  i=i+2;
	}
      else
	{
	  Tcl_AppendResult(interp, Tcl_DStringValue(pstr), NULL);
	  i++;
	}
      break;

    case geco_BOOL:
      pb = (bool *)assVars[index].varPtr;
      if (i+1 <= objc-1)
	{
	  if (Tcl_StringMatch(Tcl_GetString(objv[i+1]), "on")==1)
	    *pb=1;
	  else 
	    if (Tcl_StringMatch(Tcl_GetString(objv[i+1]), "off")==1) 
	      *pb=0;
	    else 
	      {
		Tcl_AppendResult(interp,"invalid option \"",
				 Tcl_GetString(objv[i+1]),
				 "\": must be \"on\" or \"off\"", NULL);
		return -1;
	      }
	  i++;
	}
      else
	{
	  if (*pb) 
	    Tcl_AppendResult(interp,"on", NULL);
	  else
	    Tcl_AppendResult(interp,"off", NULL);
	}
      i++;
      break;

    }

  return index;
}


/**
 * @brief Returns a Tcl_DString containing relevant info about the gecoObj.
 * @param frontStr String to be added in front of each piece of info. Default value is an empty string.
 * \return Tcl_DString containing relevant info about the gecoObj
 *
 * The '-info' subcommand of the associated Tcl command to the gecoObj uses 
 * the return value of gecoApp::info to copy the information about
 * the gecoObj to the Tcl interpreter run by the gecoApp in which the
 * the gecoObj lives.
 *
 * Internally, gecoObj builds a Tcl_DString infoStr by adding progressively
 * to it all relevant pieces of information of the gecoObj.
 *
 * Has to be extended in children to handle info specific to children. 
 * Calls to the gecoObj::addInfo methods should be used for this purpose.
 * This method will add to the internal infoStr the relevant information.
 *
 * Example of extending the method in a child of gecoObj:
 * \code
 * // Calls the info method of the gecoObj class
 * gecoObj::info(frontStr); 
 *
 * // Adds some new information
 * addInfo(frontStr, "pulse high value (V) = ", high);
 * addInfo(frontStr, "pulse low value (V) = ", low);
 *
 * // returns the infoStr
 * return infoStr;
 * \endcode
 */

Tcl_DString* gecoObj::info(const char* frontStr)
{
  Tcl_DStringFree(infoStr);
  Tcl_DStringAppend(infoStr, frontStr, -1);
  Tcl_DStringAppend(infoStr, name, -1);
  Tcl_DStringAppend(infoStr, ":", -1);
  return infoStr;
}


/**
 * @brief Adds a line to infoStr (for a string variable)
 * @param frontStr String to be added in front of each piece of info. Default value is an empty string.
 * @param text Short text explaining the piece of info
 * @param var String to be displayed in the information line
 * \return Tcl_DString containing relevant info about the gecoObj
 *
 * Calls to this method have to be used in the gecoObj::info method of children 
 * to add relevant information to the '-info' subcommand of the associated Tcl 
 * command to the gecoObj
 */

void gecoObj::addInfo(const char* frontStr, const char* text, const char* var)
{
  Tcl_DStringAppend(infoStr, "\n", -1);
  Tcl_DStringAppend(infoStr, frontStr, -1);
  Tcl_DStringAppend(infoStr, text, -1);
  Tcl_DStringAppend(infoStr, var, -1);
}


/**
 * @brief Adds a line to infoStr (for a string variable)
 * @param frontStr String to be added in front of each piece of info. Default value is an empty string.
 * @param text Short text explaining the piece of info
 * @param var Tcl_DString to be displayed in the information line
 * \return Tcl_DString containing relevant info about the gecoObj
 *
 * Calls to this method have to be used in the gecoObj::info method of children 
 * to add relevant information to the '-info' subcommand of the associated Tcl 
 * command to the gecoObj
 */

void gecoObj::addInfo(const char* frontStr, const char* text, const Tcl_DString* var)
{
  Tcl_DStringAppend(infoStr, "\n", -1);
  Tcl_DStringAppend(infoStr, frontStr, -1);
  Tcl_DStringAppend(infoStr, text, -1);
  Tcl_DStringAppend(infoStr, Tcl_DStringValue(var), -1);
}


/**
 * @brief Adds a line to infoStr (for a int variable)
 * @param frontStr String to be added in front of each piece of info. Default value is an empty string.
 * @param text Short text explaining the piece of info
 * @param var Int variable to be displayed in the information line
 * \return Tcl_DString containing relevant info about the gecoObj
 *
 * Calls to this method have to be used in the gecoObj::info method of children 
 * to add relevant information to the '-info' subcommand of the associated Tcl 
 * command to the gecoObj
 */

void gecoObj::addInfo(const char* frontStr, const char* text, int var)
{
  char str[TCL_DOUBLE_SPACE];
  Tcl_DStringAppend(infoStr, "\n", -1);
  Tcl_DStringAppend(infoStr, frontStr, -1);
  Tcl_DStringAppend(infoStr, text, -1);
  sprintf(str, "%i", var);
  Tcl_DStringAppend(infoStr, str, -1);
}


/**
 * @brief Adds a line to infoStr (for a double variable)
 * @param frontStr String to be added in front of each piece of info. Default value is an empty string.
 * @param text Short text explaining the piece of info
 * @param var Double to be displayed in the information line
 * \return Tcl_DString containing relevant info about the gecoObj
 *
 * Calls to this method have to be used in the gecoObj::info method of children 
 * to add relevant information to the '-info' subcommand of the associated Tcl 
 * command to the gecoObj
 */

void gecoObj::addInfo(const char* frontStr, const char* text, double var)
{
  char str[TCL_DOUBLE_SPACE];
  Tcl_DStringAppend(infoStr, "\n", -1);
  Tcl_DStringAppend(infoStr, frontStr, -1);
  Tcl_DStringAppend(infoStr, text, -1);
  Tcl_PrintDouble(interp, var, str);
  Tcl_DStringAppend(infoStr, str, -1);
}
