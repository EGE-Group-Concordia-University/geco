#include <tcl.h>
#include "gecoObjEx.h"

using namespace std;

// -----------------------------------------------------------------------
//
// class gecoObjEx : Class to demonstrate the creation of a new gecoObj
//


// ---- CONSTRUCTOR
//

gecoObjEx::gecoObjEx(gecoApp* App) : 
  gecoObj("GecoObjEx", "gecoObjEx", App)
{
  x=0.0;
  addOption("-x", &x, "set/returns the value of x");
  addOption("-squareX", "returns the square of x");
}


// ---- DESTRUCTOR
//

gecoObjEx::~gecoObjEx()
{
}


// ---- CMD : process a potential command option
//            returns the index from the options table

int gecoObjEx::cmd(int &i, int objc,Tcl_Obj *const objv[])
{
  // Executes the command options defined in gecoObj
  int index=gecoObj::cmd(i,objc,objv);
    
  // Checks if the 'squareX' subcommand was used
  if (index==getOptionIndex("-squareX"))
    {
      char str[10];
      Tcl_ResetResult(interp);
      sprintf(str, "%f", x*x);
      Tcl_AppendResult(interp, str, NULL);
      i++;   // needed in order the next subcommand will be processed
    }
    
  // returns index
  return index;
}


// ---- INFO : returns a Tcl_DString containing relevant info about content
//

Tcl_DString* gecoObjEx::info(const char* frontStr)
{
  gecoObj::info(frontStr);
  addInfo(frontStr,"x = ", x);
  return infoStr;
}