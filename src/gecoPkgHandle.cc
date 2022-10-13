// $Id: ECHelp.cc 39 2015-01-09 13:09:42Z wuthrich $
// ---------------------------------------------------------------
//
// Class gecoPkgHandle
//
// (c) Rolf Wuthrich
//     2015-2016 Concordia University
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
// 14.11.2015 Creation                         R. Wuthrich
// ---------------------------------------------------------------

#include <dlfcn.h>
#include <tcl.h>
#include "gecoPkgHandle.h"
#include "gecoApp.h"
#include <iostream>

using namespace std;

// ------------------------------------------------------------
//
// Tcl Interface
//


// ---- Auxiliary function to compute package name
//      Uses same convention as Tcl load/unload functions
//

int getPkgName(Tcl_Obj* obj, Tcl_DString* pkgName)
{
  Tcl_Obj*     splitPtr;
  Tcl_Obj*     pkgGuessPtr;
  int          pElements;
  char*        pkgGuess;
  
  Tcl_DStringInit(pkgName);

  splitPtr = Tcl_FSSplitPath(obj, &pElements);
  Tcl_ListObjIndex(NULL, splitPtr, pElements-1, &pkgGuessPtr);
  pkgGuess = Tcl_GetString(pkgGuessPtr);
  if ((pkgGuess[0] == 'l') && (pkgGuess[1] == 'i') && (pkgGuess[2] == 'b'))
    pkgGuess += 3;

  char* p;
  Tcl_UniChar ch;
  int offset;
  for (p=pkgGuess; *p!=0; p+=offset) 
    {
      offset = Tcl_UtfToUniChar(p, &ch);
      if ((ch>0x100) || 
  	  !(isalpha((unsigned char)(ch)) || ((unsigned char)(ch) == '_'))) 
  	break;
    }
  
  if (p == pkgGuess) 
    {
      Tcl_DecrRefCount(splitPtr);
      return TCL_ERROR;
    }
  Tcl_DStringAppend(pkgName,pkgGuess,p-pkgGuess);
  Tcl_DecrRefCount(splitPtr);

  // Fix the capitalization in the package name so that the first
  // character is in caps (or title case) but the others are all
  // lower-case.

  Tcl_DStringSetLength(pkgName,Tcl_UtfToTitle(Tcl_DStringValue(pkgName)));
  return TCL_OK;
}


// ---- Tcl loadGecoPkg command
//

int geco_LoadGecoPkgCmd(ClientData clientData, Tcl_Interp* interp, 
			int objc, Tcl_Obj* const objv[])
{
  gecoApp* app=(gecoApp *)clientData;

  if (objc!=2)
    {
      Tcl_WrongNumArgs(interp,1,objv,"fileName");
      return TCL_ERROR;
    }

  // determines the initialization procedure name
  Tcl_DString* pkgName = new Tcl_DString; 
  Tcl_DStringInit(pkgName);

  if (getPkgName(objv[1],pkgName)==TCL_ERROR)
    {
      Tcl_AppendResult(interp,"couldn't figure out package name for ",
   	 	              Tcl_GetString(objv[1]), NULL);
      return TCL_ERROR;
    }

  // checks if already loaded
  if (app->findGecoPkgHandle(Tcl_DStringValue(pkgName))!=NULL)
    {
      Tcl_AppendResult(interp,"geCo package \"",Tcl_GetString(objv[1]),
		       "\" is already loaded",NULL);
      return TCL_ERROR;
    }

  // loads the shared library
  void* handle;
  handle = dlopen(Tcl_GetString(objv[1]), RTLD_LAZY);
  if (!handle) 
    {
      Tcl_AppendResult(interp,dlerror(),NULL);
      return TCL_ERROR;
    }

  // loads the handle in the handle history
  gecoPkgHandle* h = new gecoPkgHandle(handle,
				       Tcl_DStringValue(pkgName),
				       Tcl_GetString(objv[1]),interp);
  app->addGecoPkgHandle(h);

  // Computes initialization function name based on the package name
  Tcl_DStringAppend(pkgName,"_Init",5);

  // loads the initialization procedure of the package
  gecoPkgInitProc* proc = 
    (gecoPkgInitProc *) dlsym(handle,Tcl_DStringValue(pkgName));

  if (proc==NULL) 
    {
      Tcl_AppendResult(interp, "could not initialize the geCo package", NULL);
      dlclose(handle);
      app->removeGecoPkgHandle(h);
      delete h;
      return TCL_ERROR;
    }

  (proc)(interp,app,h);

  Tcl_DStringFree(pkgName);
  delete pkgName;

  return TCL_OK;
}


// ---- Tcl unloadGecoPkg command
//

int geco_UnloadGecoPkgCmd(ClientData clientData, Tcl_Interp* interp, 
			  int objc,Tcl_Obj* const objv[])
{
  gecoApp* app=(gecoApp *)clientData;

  if (objc!=2)
    {
      Tcl_WrongNumArgs(interp,1,objv,"fileName");
      return TCL_ERROR;
    }

  if (app->getEvent()->eventLoopStatus()==1)
    {
      Tcl_AppendResult(interp,
	        "can't unload a geco package while experiment is running",NULL);
      return TCL_ERROR;
    }

  Tcl_DString* pkgName = new Tcl_DString;

  if (getPkgName(objv[1],pkgName)==TCL_ERROR)
    {
      Tcl_AppendResult(interp,"couldn't figure out package name for ",
   	 	              Tcl_GetString(objv[1]), NULL);
      return TCL_ERROR;
    }

  gecoPkgHandle* h;
  h = app->findGecoPkgHandle(Tcl_DStringValue(pkgName));
  if (h==NULL)
    {
      Tcl_AppendResult(interp,"geCo package \"",Tcl_GetString(objv[1]), 
		       "\" was never loaded",NULL);
      return TCL_ERROR;
    }

  // checks if any registered TclCmds are still loaded
  Tcl_DString* cmd=h->activeTclCmd();
  if (cmd!=NULL)
    {
      Tcl_AppendResult(interp, "cannot unload geCo package \"",
		       Tcl_GetString(objv[1]), 
		       "\" : remove first \"",
		       Tcl_DStringValue(cmd),
                       "\"",NULL);
      return TCL_ERROR;
    }

  app->removeGecoPkgHandle(h);
  delete h;

  return TCL_OK;
}


// -----------------------------------------------------------------------
//
// class gecoPkgHandle : store handle of loaded geCo packages
//


// ---- CONSTRUCTOR
//

gecoPkgHandle::gecoPkgHandle(void* Handle, char* PackageName, char* LibFile,
			     Tcl_Interp* Interp)
{
  interp=Interp;
  handle=Handle;
  pkgName = new Tcl_DString;
  libFile = new Tcl_DString;
  Tcl_DStringInit(pkgName);
  Tcl_DStringInit(libFile);
  Tcl_DStringAppend(pkgName,PackageName,-1);
  Tcl_DStringAppend(libFile,LibFile,-1);
  next=NULL;
}


// ---- DESTRUCTOR
//

gecoPkgHandle::~gecoPkgHandle()
{
  // removes the list of registered TclCmds
  for (int i=0; i<registeredTclCmds.size(); i++)
    {
      Tcl_DStringFree(registeredTclCmds[i]);
      delete registeredTclCmds[i];
    }

  // removes the list of registered gecoObj
  unloadRegisteredGecoObj();

  // Computes unload function name based on the package name
  Tcl_DStringAppend(pkgName,"_Unload",7);

  gecoPkgUnloadProc* proc = 
    (gecoPkgUnloadProc *) dlsym(handle,Tcl_DStringValue(pkgName));

  if (proc!=NULL) (proc)(interp);

  Tcl_DStringFree(pkgName);
  Tcl_DStringFree(libFile);
  delete pkgName;
  delete libFile;
  if (handle) dlclose(handle);
}


// ---- REGISTERTCLCMD : registers a Tcl cmd defined by the geCo package
//                       geCo will check before unloading the package 
//                       if the registered command is actif or not.
//                       If the command is actif the package 
//                       will not be unloaded.
//

void gecoPkgHandle::registerTclCmd(const char* TclCmd)
{
  Tcl_DString* str = new Tcl_DString;
  Tcl_DStringInit(str);
  Tcl_DStringAppend(str,TclCmd,-1);
  registeredTclCmds.push_back(str);
}


// ---- REGISTERGRCOOBJ : registers a gecoObj defined by the geCo package
//                        geCo will, while unloading the package, 
//                        delete the gecoObj
//                        Deleting will be done only if no registered 
//                        Tcl cmd is still active

void gecoPkgHandle::registerGecoObj(gecoObj* obj)
{
  registeredGecoObj.push_back(obj);
}


// ---- ACTIVETCLCMD : returns the first registered TclCmd 
//                     which is still loaded.
//                     Returns NULL if none is loaded.
//

Tcl_DString* gecoPkgHandle::activeTclCmd()
{
  Tcl_CmdInfo info;
  for (int i=0; i<registeredTclCmds.size(); i++)
    if (Tcl_GetCommandInfo(interp,Tcl_DStringValue(registeredTclCmds[i]),&info))
      return registeredTclCmds[i];
  return NULL;
}


// ---- UNLOADREGISTEREDTCLCMDS : forces to remove the registred TclCmds
//                                Is called by geCo at exit
//

void gecoPkgHandle::unloadRegisteredTclCmds()
{
 Tcl_CmdInfo info;
  for (int i=0; i<registeredTclCmds.size(); i++)
    if (Tcl_GetCommandInfo(interp,Tcl_DStringValue(registeredTclCmds[i]),&info))
      Tcl_DeleteCommand(interp,Tcl_DStringValue(registeredTclCmds[i])); 
}


// ---- UNLOADREGISTEREDGECOOBJ : removes all registered gecoObj
//                                Is called by geco while unloading the pkg
//

void gecoPkgHandle::unloadRegisteredGecoObj()
{
  for (int i=0; i<registeredGecoObj.size(); i++)
    delete registeredGecoObj[i];
}
