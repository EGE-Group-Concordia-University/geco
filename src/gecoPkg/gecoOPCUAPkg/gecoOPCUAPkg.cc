// ---------------------------------------------------------------
//
// gecoOPCUAPkg : OPC UA geCo-package
//
// (c) Rolf Wuthrich
//     2024 Concordia University
//
// author:    Rolf Wuthrich
// email:     rolf.wuthrich@concordia.ca
// version:   v1
//
// This software is copyright under the BSD license
//  
// ---------------------------------------------------------------
// history:
// ---------------------------------------------------------------
// Date       Modification                     Author
// ---------------------------------------------------------------
// 17.11.2024 Creation                         R. Wuthrich
// ---------------------------------------------------------------


#include <tcl8.6/tcl.h>
#include "gecoOPCUAPkg.h"
#include "gecoOPCUAServer.h"
#include "geco.h"

using namespace std;

#define gecoOPCUAPkgVersion "1.0"

// ------------------------------------------------------------
//
// license
//

static CONST char* gecoOPCUAPkg_license =
"   gecoOPCUAPkg OPC UA geCo-package\n"
"   Copyright (c) 2024, Rolf Wuthrich <rolf.wuthrich@concordia.ca>\n"
"   All rights reserved.\n "
"   \n"
"   Redistribution and use in source and binary forms, with or without\n"
"   modification, are permitted provided that the following conditions\n"
"   are met:\n"
"   \n"
"     * Redistributions of source code must retain the above copyright\n"
"       notice, this list of conditions and the following disclaimer.\n"
"     * Redistributions in binary form must reproduce the above copyright\n"
"       notice, this list of conditions and the following disclaimer in the\n"
"       documentation and/or other materials provided with the distribution.\n"
"     * Neither the name of Rolf Wuthrich nor the names of contributors\n"
"       to this software may be used to endorse or promote products derived\n"
"       from this software without specific prior written permission.\n"
"       \n"
"   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS\n"
"   ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT\n"
"   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR\n"
"   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR\n"
"   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,\n"
"   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,\n"
"   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR\n"
"   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF\n"
"   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING\n"
"   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS\n"
"   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.";


// ------------------------------------------------------------
//
// Initialization and unloading procedures
//

extern "C" {

  Tcl_Namespace* gecoOPCUAPkgNsPtr;
  gecoPkgHandle* gecoOPCUAPkgHandle;

  // ---- geCo package initialization procedure
  //
  //      Convention for name to be respected: <Name>_Init
  //      with <Name> First letter in caps and rest in lower-case
  //      and must match the library name 
  //      Same convention as in Tcl load/unload procedures
  //

  int Gecoopcuapkg_Init(Tcl_Interp *interp, gecoApp* app, 
		       gecoPkgHandle* pkgHandle)
  {

    // creates a Tcl namespace for the package
    gecoOPCUAPkgNsPtr=Tcl_CreateNamespace(interp,"opcuaPkg", NULL, NULL);

    // stores the package-handle in a local variable for 
    // further use within the package
    gecoOPCUAPkgHandle=pkgHandle;

    // makes the liscence and version public
    Tcl_LinkVar(interp,"::opcuaPkg::license",
    		(char *)&gecoOPCUAPkg_license, 
    		TCL_LINK_STRING | TCL_LINK_READ_ONLY);
    Tcl_SetVar(interp,"::opcuaPkg::version",gecoOPCUAPkgVersion,0);

    // exports Tcl commands
    Tcl_CreateObjCommand(interp, "opcua_server", gecoOPCUAServerCmd,
                         (ClientData) app, (Tcl_CmdDeleteProc *) NULL);

    // greetings
    Tcl_AppendResult(interp,
    		     "+---------------------------------------------------+\n",
    		     "|                  geCo OPC UA                      |\n",
    		     "|                  Version 1.0                      |\n",
    		     "|          last modified November 2024              |\n",
    		     "+---------------------------------------------------+\n",
    		     "| Copyright (C) 2024                                |\n",
    		     "| Rolf Wuthrich, Concordia University, Canada       |\n",
    		     "| This is free software                             |\n",
    		     "| Type 'puts $::opcuaPkg::license' for more details |\n",
    		     "+---------------------------------------------------+",
    		     NULL);

    // needed to tell geCo all is ok
    return TCL_OK;
  }


  // ---- geCo package unload procedure
  //
  //      convention for name: as in initialization procedure
  //

  int Gecoopcuapkg_Unload(Tcl_Interp *interp)
  {
    // removes package namespace
    Tcl_DeleteNamespace(gecoOPCUAPkgNsPtr);

    Tcl_AppendResult(interp,"\n\tUnloaded gecoOPCUAPkg\n",NULL); 

    // needed to tell geCo all is ok
    return TCL_OK;
  }

} /* extern "C" */


// -----------------------------------------------------------------------
//
// TODO : implement here the routines of your package
//

