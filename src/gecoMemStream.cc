// $Id: ECTrigger.cc 15 2014-01-03 18:49:04Z wuthrich $
// ---------------------------------------------------------------
//
// Definition of the class gecoMemStream
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
// 31.10.2015 Creation                         R. Wuthrich
// 08.12.2020 Added doxygen documentation      R. Wuthrich
//
// ---------------------------------------------------------------

#include <tcl.h>
#include <fstream>
#include <cstring>
#include "gecoMemStream.h"
#include "gecoApp.h"

using namespace std;


// -------------------------------------------------------------------------
//
// Tcl interface
//

// Command to create a new recorder object
//

int geco_MemStreamCmd(ClientData clientData, Tcl_Interp *interp, 
		     int objc,Tcl_Obj *const objv[])
{
  gecoMemStream* proc  = new gecoMemStream((gecoApp *)clientData);
  return geco_CreateGecoProcessCmd(proc, objc, objv);
}

// -------------------------------------------------------------------------


// -------------------------------------------------------------------------
//
// class gecoMemStream : a class to stream data to memory
//

/**
 * @brief Destructor
*/

gecoMemStream::~gecoMemStream()
{
  Tcl_DStringFree(data);
  Tcl_DStringFree(cut);
  Tcl_DStringFree(dataStr);
  delete data;
  delete cut;
  delete dataStr;
  resetData();
  delete firstData;
}


/*! 
 * @copydoc gecoProcess::cmd
 *
 * Compared to gecoProcess::cmd, gecoMemStream::cmd adds the processing of
 * the new subcommands of gecoMemStream.
 */

int gecoMemStream::cmd(int &i, int objc,Tcl_Obj *const objv[])
{
  // first executes the command options defined in gecoProcess
  int index=gecoProcess::cmd(i,objc,objv);

  if (index==getOptionIndex("-save"))
    {
      if (objc!=3)
	{
	  Tcl_WrongNumArgs(interp, i+1, objv, "filename");
	  return -1;
	}

      Tcl_DString* fileName;
      fileName = new Tcl_DString;
      Tcl_DStringInit(fileName);
      Tcl_DStringAppend(fileName, Tcl_GetString(objv[i+1]), -1);

      if (saveData(fileName)==TCL_ERROR)
        {
          Tcl_ResetResult(interp);
          Tcl_AppendResult(interp, "could not save file ",
			   Tcl_DStringValue(fileName), NULL);
	  Tcl_DStringFree(fileName);
	  delete fileName;
          return -1;
        }

      Tcl_DStringFree(fileName);
      delete fileName;

      i=i+2;
    }

  return index;

}


/**
 * @copydoc gecoProcess::handleEvent
 *
 * In addition to gecoProcess::handleEvent, gecoMemStream::handleEvent implements
 * the streaming of the data to the memory.
 */

void gecoMemStream::handleEvent(gecoEvent* ev)
{
  gecoProcess::handleEvent(ev);

  // If Active, records data
  if ((status==Active)&&((ev->getT()-saveTime)>=dtRecord))
    {
      saveTime=ev->getT();
      Tcl_DStringFree(dataStr);
      Tcl_DStringAppend(dataStr, "format \"", -1);
      Tcl_DStringAppend(dataStr, Tcl_DStringValue(data), -1);
      Tcl_DStringAppend(dataStr, "\"", -1);
      Tcl_Eval(interp, Tcl_DStringValue(dataStr)); 
      dataRec* rec;
      rec = new dataRec;
      rec->data = new Tcl_DString;
      Tcl_DStringInit(rec->data);
      Tcl_DStringAppend(rec->data, Tcl_GetStringResult(interp), -1);
      rec->next=NULL;
      lastData->next=rec;
      lastData=rec;
      Tcl_ResetResult(interp);
    }
}


/**
 * @copydoc gecoProcess::info
 *
 * In addition to gecoProcess::info, gecoMemStream::info adds the information
 * about the data to stream.
 */

Tcl_DString* gecoMemStream::info(const char* frontStr)
{
  gecoProcess::info(frontStr);
  addInfo(frontStr, "Data to stream:       ", Tcl_DStringValue(data));  
  addInfo(frontStr, "Record interval (s):  ", dtRecord);  
  return infoStr;
}


/**
 * @copydoc gecoProcess::activate
 *
 * In addition to gecoProcess::activate, gecoMemStream::activate 
 * setup the memory structure to stream the data to.
 */

void gecoMemStream::activate(gecoEvent* ev)
{
  gecoProcess::activate(ev);
  saveTime=ev->getT();
  resetData();
}


/**
 * @copydoc gecoProcess::terminate
 *
 * In addition to gecoProcess::terminate, gecoMemStream::terminate 
 * will save the data from the memory to disk if -autosave is true
 */

void gecoMemStream::terminate(gecoEvent* ev)
{
  gecoProcess::terminate(ev);

  if (autoSave==false) return;

  // computes file name
  autosaveCounter++;
  char ID[8];
  sprintf(ID, "%d", autosaveCounter);
  Tcl_DString* fileName;
  fileName = new Tcl_DString;
  Tcl_DStringInit(fileName);
  Tcl_DStringAppend(fileName, getTclCmd(), -1);
  Tcl_DStringAppend(fileName, "_", -1);
  Tcl_DStringAppend(fileName, ID, -1);
  Tcl_DStringAppend(fileName, ".txt", -1);
  char str[80];

  if (saveData(fileName)==TCL_ERROR)
    sprintf(str, "cons \"    ERROR : could not save %s\"",
	    Tcl_DStringValue(fileName));
  else
    if (verbose) 
      sprintf(str, "cons \"    saved %s\"", Tcl_DStringValue(fileName));

  Tcl_Eval(interp, str);
  Tcl_DStringFree(fileName);
  delete fileName;
}


/**
 * @brief saves all recorded data
 * @param fileName name of the file to save to
 * \return TCL_ERROR if the saving fails otherwise TCL_OK
*/

int gecoMemStream::saveData(Tcl_DString* fileName)
{
  ofstream dataFile;
  dataFile.open(Tcl_DStringValue(fileName),ios::trunc);
  if (dataFile.fail()) return TCL_ERROR;
  dataRec* rec=firstData->next;
  while(rec!=NULL)
    {
      dataFile << Tcl_DStringValue(rec->data) << "\n";
      rec=rec->next;
    }
  dataFile.close();
  return TCL_OK;
}


/**
 * @brief deletes all recorded data except first one
 */

void gecoMemStream::resetData()
{
  dataRec* it = firstData->next;
  dataRec* next;
  while (it!=NULL)
    {
      next = it->next;
      Tcl_DStringFree(it->data);
      delete it->data;
      delete it;
      it=next;
    }
  firstData->next = NULL;
  lastData = firstData;
}
