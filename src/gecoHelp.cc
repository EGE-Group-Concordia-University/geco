// $Id: ECHelp.cc 39 2015-01-09 13:09:42Z wuthrich $
// ---------------------------------------------------------------
//
// Definition of the geCo help function
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
// 09.11.2015 Creation                         R. Wuthrich
// ---------------------------------------------------------------

#include <tcl.h>
#include "gecoHelp.h"

using namespace std;

// ---- HELP : help text for geCo commands
//

void gecoHelp(Tcl_Interp* interp, const char* cmdName, const char* cmdInfo, const char** cmds, const char** help)
{
  Tcl_AppendResult(interp,"\n",cmdName," - ",cmdInfo,NULL);
  Tcl_AppendResult(interp,"\n\nValid options are:\n\n",NULL);
  Tcl_AppendResult(interp,"-help                displays this message\n",NULL);
  int  j=0;
  char str[100];
  while (help[j]!=NULL)
    {
      sprintf(str,"%-20.20s %-56.56s\n",cmds[j+1],help[j]);
      Tcl_AppendResult(interp,str,NULL);
      j++;
    }
  Tcl_AppendResult(interp,"\nFor more details see man page.",NULL);
}
