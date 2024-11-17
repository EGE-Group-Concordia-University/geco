// $Id: ECHelp.cc 39 2015-01-09 13:09:42Z wuthrich $
// ---------------------------------------------------------------
//
// Header file for the class gecoPkgHandle
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

#ifndef gecoPkgHandle_SEEN_
#define gecoPkgHandle_SEEN_

#include <dlfcn.h>
#include <tcl8.6/tcl.h>
#include <vector>
#include "gecoApp.h"
#include "gecoObj.h"

using namespace std;


// -------------------------------------------------------------------
//
// Tcl Interface
//

int geco_LoadGecoPkgCmd(ClientData clientData, Tcl_Interp *interp, 
			int objc,Tcl_Obj *const objv[]);
int geco_UnloadGecoPkgCmd(ClientData clientData, Tcl_Interp *interp, 
			  int objc,Tcl_Obj *const objv[]);

// -------------------------------------------------------------------
//
// class gecoPkgHandle : stores information about loaded geCo package
//


class gecoPkgHandle
{

 private:

  vector<Tcl_DString*>  registeredTclCmds;
  vector<gecoObj*>      registeredGecoObj;

 protected:

  void*                 handle;
  Tcl_DString*          pkgName;
  Tcl_DString*          libFile;
  gecoPkgHandle*        next;
  Tcl_Interp*           interp;

 public:

  gecoPkgHandle(void* Handle, char* PackageName, char* LibFile, 
		Tcl_Interp* Interp);
  ~gecoPkgHandle();

  char*             getPackageName() {return Tcl_DStringValue(pkgName);}
  char*             getLibFile()     {return Tcl_DStringValue(libFile);}

  void              registerTclCmd(const char* TclCmd);
  void              registerGecoObj(gecoObj* obj);
  Tcl_DString*      activeTclCmd();
  void              unloadRegisteredTclCmds();
  void              unloadRegisteredGecoObj();

  gecoPkgHandle*    getNextGecoPkgHandle() {return next;}
  void              setNextGecoPkgHandle(gecoPkgHandle* handle) {next=handle;}
  
};



// Type defintion for geCo package initialization procedures
//

typedef int gecoPkgInitProc(Tcl_Interp* interp, gecoApp* app, 
			    gecoPkgHandle* pkgHandle);
typedef int gecoPkgUnloadProc(Tcl_Interp* interp);


#endif /* gecoPkgHandle_SEEN_ */
