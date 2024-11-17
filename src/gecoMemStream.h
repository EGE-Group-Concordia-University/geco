// This may look like C code, but it is really -*- C++ -*-
// $Id: ECTrigger.h 15 2014-01-03 18:49:04Z wuthrich $
// ---------------------------------------------------------------- 
//                                                                  
// Header file for class gecoMemStream
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
/*! \file */

#ifndef gecoMemStream_SEEN_
#define gecoMemStream_SEEN_

#include <tcl8.6/tcl.h>
#include "gecoProcess.h"
#include "gecoApp.h"

using namespace std;


// -----------------------------------------------------------------------
//
// Tcl interface
//

/**
 * @brief C++ implementation of the Tcl command to create a gecoMemStream object
 * @param clientData pointer to the gecoApp in which the gecoEnd will live
 * @param interp Tcl interpreter in which the Tcl command is executed
 * @param objc number of arguments of the Tcl command
 * @param objv arguments of the the Tcl command
 * \return TCL_OK if the execution of the Tcl command is successful and TCL_ERROR otherwise
 */

int geco_MemStreamCmd(ClientData clientData, Tcl_Interp *interp, 
		      int objc,Tcl_Obj *const objv[]);


// -------------------------------------------------------------------------
//
// Structure for data

struct dataRec
{
  Tcl_DString*   data;
  dataRec*       next;
};


// -----------------------------------------------------------------------
//
// class gecoMemStream : a class to stream data to the memory
//

/** 
 * @brief A gecoProcess to stream data to the memory
 * \author Rolf Wuthrich
 * \date 2015-2020
 *
 * The gecoMemStream class allows to create a gecoProcess able to stream
 * data, in form of Tcl variables defined in the Tcl interpreter of the 
 * gecoApp in which the gecoMemStream instance lives, to a file.
 *
 * The geco_MemStreamCmd() is the C++ implementation for the Tcl command to
 * create gecoMemStream objects. This Tcl command is already available in 
 * the Tcl interpreter run by an instance of gecoApp under the name 'memstream'.
 *
 * Associated Tcl command
 * ----------------------
 * Every gecoMemStream is associated to a Tcl command. The associated Tcl command 
 * is created during the construction of an instance of gecoMemStream by its
 * parent class. The Tcl command is created in the Tcl interpreter
 * run by the gecoApp in which gecoMemStream lives.
 *
 * The gecoMemStream class extends the subcommands from gecoObj and gecoProcess
 * by the following subcommands
 *
 * Sub-command       | Short description
 * ----------------- | ------------------
 * -dtRecord         | returns/sets time interval between two recordings (s) 
 * -data             | returns/sets data to stream to file
 * -cut              | returns/sets filter on data to be saved
 * -autosave         | saves automatically recored data to disk when gecoMemStream is terminated
 * -save             | saves recored data to a file
 */

class gecoMemStream : public gecoProcess
{
private:

  Tcl_DString*           dataStr;
  double                 saveTime;
  int                    autosaveCounter;

protected:

  dataRec*       firstData;
  dataRec*       lastData;

  bool           autoSave;

  Tcl_DString*   data;            // data to be recorder
  Tcl_DString*   cut;             // cut fiter

  double         dtRecord;          

public:

  gecoMemStream(gecoApp* App) :
    gecoObj("Stream to memory", "memstream", App),
    gecoProcess("Stream to memory", "user", "memstream", App),
    autosaveCounter(0),
    saveTime(0.0),
    dtRecord(0.1),
    autoSave(false)
  {
    activateOnStart=1;
    firstData=new dataRec;
    firstData->data=NULL;
    firstData->next=NULL;
    lastData=firstData;

    data     = new Tcl_DString;
    cut      = new Tcl_DString;
    dataStr  = new Tcl_DString;
    Tcl_DStringInit(data);
    Tcl_DStringInit(cut);
    Tcl_DStringInit(dataStr);

    addOption("-dtRecord", &dtRecord,
	      "returns/sets time interval between two recordings (s)");
    addOption("-cut", cut, "returns/sets filter on data to be recorded");
    addOption("-data", data, "returns/sets data to record");
    addOption("-autosave", &autoSave, "saves automatically recored data to disk");
    addOption("-save", "saves recored data to a file");
  }

  ~gecoMemStream();

  virtual int  cmd(int &i,int objc,Tcl_Obj *const objv[]);
  virtual void handleEvent(gecoEvent* ev);
  virtual Tcl_DString* info(const char* frontStr = "");

  virtual void activate(gecoEvent* ev);
  virtual void terminate(gecoEvent* ev);

  int          saveData(Tcl_DString* fileName);
  void         resetData();
};

#endif /* gecoMemStream_SEEN_ */
