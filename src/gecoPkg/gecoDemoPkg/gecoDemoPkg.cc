// $Id: ECIOBoard.cc 37 2015-01-09 12:35:29Z wuthrich $
// ---------------------------------------------------------------
//
// gecoDemoPkg : a demo geCo-package
//
// (c) Rolf Wuthrich
//     2016 Concordia University
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
// 05.02.2016 Creation                         R. Wuthrich
// ---------------------------------------------------------------


#include <tcl.h>
#include "gecoDemoPkg.h"
#include "geco.h"

using namespace std;

#define gecoDemoPkgVersion "1.0"

// ------------------------------------------------------------
//
// license
//

static CONST char* gecoDemoPkg_license =
"   gecoDemoPkg a demo geCo-package\n"
"   Copyright (c) 2016, Rolf Wuthrich <rolf.wuthrich@concordia.ca>\n"
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

  Tcl_Namespace* gecoDemoPkgNsPtr;
  gecoPkgHandle* gecoDemoPkgHandle;

  // ---- geCo package initialization procedure
  //
  //      Convention for name to be respected: <Name>_Init
  //      with <Name> First letter in caps and rest in lower-case
  //      and must match the library name 
  //      Same convention as in Tcl load/unload procedures
  //

  int Gecodemopkg_Init(Tcl_Interp *interp, gecoApp* app, 
		       gecoPkgHandle* pkgHandle)
  {

    // creates a Tcl namespace for the package
    gecoDemoPkgNsPtr=Tcl_CreateNamespace(interp,"demoPkg", NULL, NULL);

    // stores the package-handle in a local variable for 
    // further use within the package
    gecoDemoPkgHandle=pkgHandle;

    // makes the liscence and version public
    Tcl_LinkVar(interp,"::demoPkg::license",
    		(char *)&gecoDemoPkg_license, 
    		TCL_LINK_STRING | TCL_LINK_READ_ONLY);
    Tcl_SetVar(interp,"::demoPkg::version",gecoDemoPkgVersion,0);

    // greetings
    Tcl_AppendResult(interp,
    		     "+---------------------------------------------------+\n",
    		     "|              geCo demo package                    |\n",
    		     "|                  Version 1.0                      |\n",
    		     "|          last modified February 2016              |\n",
    		     "+---------------------------------------------------+\n",
    		     "| Copyright (C) 2016                                |\n",
    		     "| Rolf Wuthrich, Concordia University, Canada       |\n",
    		     "| This is free software                             |\n",
    		     "| Type 'puts $::demoPkg::license' for more details  |\n",
    		     "+---------------------------------------------------+",
    		     NULL);

    // needed to tell geCo all is ok
    return TCL_OK;
  }


  // ---- geCo package unload procedure
  //
  //      convention for name: as in initialization procedure
  //

  int Gecodemopkg_Unload(Tcl_Interp *interp)
  {
    // removes package namespace
    Tcl_DeleteNamespace(gecoDemoPkgNsPtr);

    Tcl_AppendResult(interp,"\n\tUnloaded gecoDemoPkg\n",NULL); 

    // needed to tell geCo all is ok
    return TCL_OK;
  }

} /* extern "C" */


// -----------------------------------------------------------------------
//
// TODO : implement here the routines of your package
//



