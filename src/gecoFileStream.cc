// $Id: ECTrigger.cc 15 2014-01-03 18:49:04Z wuthrich $
// ---------------------------------------------------------------
//
// Definition of the class gecoFileStream
//
// (c) Rolf Wuthrich
//     2015 - 2020 Concordia University
//
// author:  Rolf Wuthrich
// email:   rolf.wuthrich@concordia.ca
// version: v1 ($Revision: 15 $)
//
// This software is copyright under the BSD license
//  
// ---------------------------------------------------------------
// history:
// ---------------------------------------------------------------
// Date       Modification                     Author
// ---------------------------------------------------------------
// 23.10.2015 Creation                         R. Wuthrich
// 08.12.2020 Added doxygen documentation      R. Wuthrich
//
// ---------------------------------------------------------------

#include <tcl.h>
#include <cstring>
#include "gecoFileStream.h"
#include "gecoApp.h"

using namespace std;


// -------------------------------------------------------------------------
//
// Tcl interface
//

// Command to create a new gecoFileStream object
//

int geco_FileStreamCmd(ClientData clientData, Tcl_Interp *interp, 
		       int objc,Tcl_Obj *const objv[])
{
  gecoFileStream* proc  = new gecoFileStream((gecoApp *)clientData);
  return geco_CreateGecoProcessCmd(proc, objc, objv);
}

// -------------------------------------------------------------------------


// -------------------------------------------------------------------------
//
// class gecoFileStream: a class to stream data to a file
//

/**
 * @brief Destructor
*/

gecoFileStream::~gecoFileStream()
{
  Tcl_DStringFree(fileName);
  Tcl_DStringFree(data);
  Tcl_DStringFree(dataStr);
  Tcl_DStringFree(cut);
  delete fileName;
  delete data;
  delete dataStr;
  delete cut;
}


/**
 * @copydoc gecoProcess::handleEvent
 *
 * In addition to gecoProcess::handleEvent, gecoFileStream::handleEvent implements
 * the streaming of the data to the file and filtering with cut condition.
 */

void gecoFileStream::handleEvent(gecoEvent* ev)
{
  gecoProcess::handleEvent(ev);

  // determines cut condition of set
  int b=1;
  if (Tcl_DStringLength(cut)) Tcl_ExprBoolean(interp,Tcl_DStringValue(cut),&b);

  // If Active, records data
  if ((status==Active) && ((ev->getT()-saveTime)>=dtRecord) && (b))
    {
      saveTime=ev->getT();
      Tcl_DStringFree(dataStr);
      Tcl_DStringAppend(dataStr, "format \"", -1);
      Tcl_DStringAppend(dataStr, Tcl_DStringValue(data), -1);
      Tcl_DStringAppend(dataStr, "\"", -1);
      Tcl_Eval(interp, Tcl_DStringValue(dataStr)); 
      dataFile << Tcl_GetStringResult(interp) << "\n";
      dataFile.flush();
      Tcl_ResetResult(interp);
    }
}


/**
 * @copydoc gecoProcess::info
 *
 * In addition to gecoProcess::info, gecoFileStream::info adds the information
 * about the data and file to stream to.
 */

Tcl_DString* gecoFileStream::info(const char* frontStr)
{
  gecoProcess::info(frontStr);
  addInfo(frontStr, "File:                 ", Tcl_DStringValue(fileName));   
  addInfo(frontStr, "Header:               ", Tcl_DStringValue(header));  
  addInfo(frontStr, "Data to stream:       ", Tcl_DStringValue(data));  
  addInfo(frontStr, "Record interval (s):  ", dtRecord);  
  return infoStr;
}


/**
 * @copydoc gecoProcess::terminate
 *
 * In addition to gecoProcess::terminate, gecoFileStream::terminate 
 * closes the file to stream to.
 */

void gecoFileStream::terminate(gecoEvent* ev)
{
  gecoProcess::terminate(ev);
  dataFile.flush();
  dataFile.close();
}


/**
 * @copydoc gecoProcess::activate
 *
 * In addition to gecoProcess::activate, gecoFileStream::activate 
 * opens for writing the file to stream to.
 */

void gecoFileStream::activate(gecoEvent* ev)
{
  gecoProcess::activate(ev);
  dataFile.open(Tcl_DStringValue(fileName), ios::trunc);
  
  Tcl_DStringFree(dataStr);
  Tcl_DStringAppend(dataStr, "format \"", -1);
  Tcl_DStringAppend(dataStr, Tcl_DStringValue(header), -1);
  Tcl_DStringAppend(dataStr, "\"", -1);
  Tcl_Eval(interp, Tcl_DStringValue(dataStr)); 
  dataFile << Tcl_GetStringResult(interp) << "\n";
  
  saveTime=ev->getT();
}
