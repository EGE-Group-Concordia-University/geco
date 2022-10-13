// $Id: ECIOBoard.cc 37 2015-01-09 12:35:29Z wuthrich $
// ---------------------------------------------------------------
//
// gecoPiPkg : geco package for the raspberry pi
//
// (c) Rolf Wuthrich
//     2016 - 2020 Concordia University
//
// author:    Rolf Wuthrich
// email:     rolf.wuthrich@concordia.ca
// version:   v1 ($Revision: 37 $)
//
// This software is copyright under the BSD license
//  
// ---------------------------------------------------------------
// history:
// ---------------------------------------------------------------
// Date       Modification                     Author
// ---------------------------------------------------------------
// 07.02.2016 Creation                         R. Wuthrich
// 10.11.2020 General update                   R. Wuthrich
// ---------------------------------------------------------------

#include <tcl.h>
#include "gecoPiPkg.h"
#include "gecoPi.h"
#include "gecoDS18B20.h"
#include "gecoPiGPIO.h"
#include "geco.h"

using namespace std;

#define gecoPiPkgVersion "1.0"

// ------------------------------------------------------------
//
// license
//

static CONST char* gecoPiPkg_license =
"   gecoPiPkg : geCo package for the raspberry pi\n"
"   Copyright (c) 2016-2020, Rolf Wuthrich <rolf.wuthrich@concordia.ca>\n"
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
	
  // ---- local variables needed by the package

  Tcl_Namespace* gecoPiPkgNsPtr;
  gecoPkgHandle* gecoPiPkgHandle;
  gecoPi*        gecoPiCmd;
  gecoPiGPIObus* GPIObus;
  

  
  // ---- geCo package initialization procedure
  //

  int Gecopipkg_Init(Tcl_Interp *interp, gecoApp* app, 
		     gecoPkgHandle* pkgHandle)
  {

    // Tcl namespace of the package
    gecoPiPkgNsPtr=Tcl_CreateNamespace(interp, "piPkg", NULL, NULL);

    // stores the package-handle in local variable for 
    // further use within the package
    gecoPiPkgHandle=pkgHandle;

    // makes the license and version public
    Tcl_LinkVar(interp, "::piPkg::license",
    		(char *)&gecoPiPkg_license, 
    		TCL_LINK_STRING | TCL_LINK_READ_ONLY);
    Tcl_SetVar(interp, "::piPkg::version", gecoPiPkgVersion, 0);

    // pi GPIO bus
    GPIObus = new gecoPiGPIObus(app);

    // exports Tcl commands
    gecoPiCmd     = new gecoPi(app);
    Tcl_CreateObjCommand(interp, "pi", geco_gecoObjCmd,
			 static_cast<gecoObj*>(gecoPiCmd), (Tcl_CmdDeleteProc *) NULL);
    gecoPiPkgHandle->registerGecoObj(static_cast<gecoObj*>(gecoPiCmd));

    Tcl_CreateObjCommand(interp, "piGPIO", gecoPiGPIOCmd, 
			 (ClientData) GPIObus, (Tcl_CmdDeleteProc *) NULL);
	
    Tcl_CreateObjCommand(interp, "DS18B20", gecoPiDS18B20Cmd, 
			 (ClientData) app, (Tcl_CmdDeleteProc *) NULL);
	//gecoPiPkgHandle->registerGecoObj(static_cast<gecoObj*>(gecoPiDS18B20Cmd));

    // greetings
    Tcl_AppendResult(interp,
    		     "+---------------------------------------------------+\n",
    		     "|          geCo raspberry pi package                |\n",
    		     "|                  Version 1.1                      |\n",
    		     "|          last modified November 2020              |\n",
    		     "+---------------------------------------------------+\n",
    		     "| Copyright (C) 2016 - 2020                         |\n",
    		     "| Rolf Wuthrich, Concordia University, Canada       |\n",
    		     "| This is free software                             |\n",
    		     "| Type 'puts $::piPkg::license' for more details    |\n",
		     "+---------------------------------------------------+\n",
	             "| New commands imported: pi, piGPIO, DS18B20        |\n",
    		     "+---------------------------------------------------+",
    		     NULL);

    // needed to tell geCo all is ok
    return TCL_OK;
  }


  // ---- geCo package unload procedure
  //

  int Gecopipkg_Unload(Tcl_Interp *interp)
  {
    // removes package namespace
    Tcl_DeleteNamespace(gecoPiPkgNsPtr);

    // removes GPIO bus;
    delete GPIObus;

    // Message to user
    Tcl_AppendResult(interp, "\n\tUnloaded gecoPiPkg\n", NULL);

    // needed to tell geCo all is ok
    return TCL_OK;
  }

} /* extern "C" */

// ------------------------------------------------------------


