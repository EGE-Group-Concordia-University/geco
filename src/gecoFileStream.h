// This may look like C code, but it is really -*- C++ -*-
// $Id: ECTrigger.h 15 2014-01-03 18:49:04Z wuthrich $
// ---------------------------------------------------------------- 
//                                                                  
// Header file for class gecoFileStream
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
// 25.10.2015 Creation                         R. Wuthrich
// 08.12.2020 Added doxygen documentation      R. Wuthrich
//
// ---------------------------------------------------------------
/*! \file */

#ifndef gecoFileStream_SEEN_
#define gecoFileStream_SEEN_

#include <tcl.h>
#include <fstream>
#include "gecoProcess.h"
#include "gecoApp.h"

using namespace std;


// -----------------------------------------------------------------------
//
// Tcl interface
//

/**
 * @brief C++ implementation of the Tcl command to create a gecoFileStream object
 * @param clientData pointer to the gecoApp in which the gecoFileStream instance will live
 * @param interp Tcl interpreter in which the Tcl command is executed
 * @param objc number of arguments of the Tcl command
 * @param objv arguments of the the Tcl command
 * \return TCL_OK if the execution of the Tcl command is successful and TCL_ERROR otherwise
 */

int geco_FileStreamCmd(ClientData clientData, Tcl_Interp *interp, 
		       int objc,Tcl_Obj *const objv[]);


// -----------------------------------------------------------------------
//
// class gecoFileStream : a class for streaming data to a file
//

/** 
 * @brief A gecoProcess to stream data to a file
 * \author Rolf Wuthrich
 * \date 2015-2020
 *
 * The gecoFileStream class allows to create a gecoProcess able to stream
 * data, in form of Tcl variables defined in the Tcl interpreter of the 
 * gecoApp in which the gecoFileStream instance lives, to a file.
 *
 * The geco_FileStreamCmd() is the C++ implementation for the Tcl command to
 * create gecoFileStream objects. This Tcl command is already available in 
 * the Tcl interpreter run by an instance of gecoApp under the name 'filestream'.
 *
 * Associated Tcl command
 * ----------------------
 * Every gecoFileStream is associated to a Tcl command. The associated Tcl command 
 * is created during the construction of an instance of gecoFileStream by its
 * parent class. The Tcl command is created in the Tcl interpreter
 * run by the gecoApp in which gecoFileStream lives.
 *
 * The gecoFileStream class extends the subcommands from gecoObj and gecoProcess
 * by the following subcommands
 *
 * Sub-command       | Short description
 * ----------------- | ------------------
 * -file             | returns/sets file name to stream data to
 * -data             | returns/sets data to stream to file
 * -cut              | returns/sets filter on data to be saved
 * -dtRecord         | returns/sets time interval between two recordings (s)
 */

class gecoFileStream : public gecoProcess
{
private:

  Tcl_DString*   dataStr;
  double         saveTime;

protected:

  Tcl_DString*   fileName;        // file name
  Tcl_DString*   header;          // header of data file
  Tcl_DString*   data;            // data to be streamed
  Tcl_DString*   cut;             // cut filter
  ofstream       dataFile;
  double         dtRecord;          

public:

  gecoFileStream(gecoApp* App) :
    gecoObj("Stream To File", "filestream", App),
    gecoProcess("Stream To File", "user", "filestream", App),
    saveTime(0.0),
    dtRecord(0.1)
  {
    activateOnStart=1;
    fileName = new Tcl_DString;
    header    = new Tcl_DString;
    data     = new Tcl_DString;
    cut      = new Tcl_DString;
    dataStr  = new Tcl_DString;
    Tcl_DStringInit(fileName);
    Tcl_DStringInit(header);
    Tcl_DStringInit(data);
    Tcl_DStringInit(cut);
    Tcl_DStringInit(dataStr);

    addOption("-file", fileName, "returns/sets file name to stream data to");
    addOption("-header", header, "returns/sets header of the file");
    addOption("-data", data, "returns/sets data to stream to file");
    addOption("-cut", cut, "returns/sets filter on data to be saved");
    addOption("-dtRecord", &dtRecord, "returns/sets time interval between two recordings (s)");
  }

  ~gecoFileStream();

  virtual void handleEvent(gecoEvent* ev);
  virtual Tcl_DString* info(const char* frontStr = "");

  virtual void terminate(gecoEvent* ev);
  virtual void activate(gecoEvent* ev);
};

#endif /* gecoFileStream_SEEN_ */
