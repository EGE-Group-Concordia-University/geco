// This may look like C code, but it is really -*- C++ -*-
// $Id: ECEvent.h 25 2014-03-16 17:03:44Z wuthrich $
// ---------------------------------------------------------------- 
//                                                                  
// Header file for class gecoEvent
//
// (c) Rolf Wuthrich
//     2015-2016 Concordia University
//
// author:  Rolf Wuthrich
// email:   rolf.wuthrich@concordia.ca
// version: $Revision: 25 $
//
// This software is copyright under the BSD license
//
// ---------------------------------------------------------------
// history:
// ---------------------------------------------------------------
// Date       Modification                     Author
// ---------------------------------------------------------------
// 17.10.2015 Creation                         R. Wuthrich
// 05.12.2020 Added doxygen documentation      R. Wuthrich
//
// ---------------------------------------------------------------

#ifndef gecoEvent_SEEN_
#define gecoEvent_SEEN_

#include <tcl.h>
#include <iostream>

using namespace std;

// -----------------------------------------------------------------------
//
// Event types
//

const int
  NoEvent  = 0,
  CmdEvent = 1;


// -----------------------------------------------------------------------
//
// Predefined commands
//

const int
  NoCmd                 = 0,
  CmdExit               = 1,
  CmdStart              = 2,
  CmdStop               = 3,
  CmdHold               = 4,
  CmdResume             = 5,
  CmdProcessTerminated  = 6,
  CmdProcedureActivated = 7;



// -----------------------------------------------------------------------
//
// class gecoEvent : class for event manipulation and storing
//

class gecoProcess; // forward definition
class gecoApp;     // forward definition

/** 
 * @brief Class for storing and manipulating geco events
 * \author Rolf Wuthrich
 * \date 2015-2020
 *
 *
 * The gecoEvent class stores geco event. Geco events are used
 * by the geco process loop and are passed successively to the
 * gecoProcess::handleEvent method of the gecoProcess inside the loop.
 *
 * A gecoEvent contains the following information
 *
 * Field           | Brief description
 * --------------- | ---------------------------
 * interp          | Tcl interpreter of the gecoApp in which the geco event loop runs
 * app             | gecoApp in which the geco event loop runs
 * eventType       | Type of gecoEvent
 * generator       | gecoProcess which generated the event
 * EvCmd           | Command carried by the event
 * EventLoopStatus | Status of the event loop
 * t               | Time at which the event was generated
 *
 * Event types
 * -----------
 * Currently there exist only two event types: NoEvent and CmdEvent.
 * A CmdEvent corresponds to an event which carnies a command with it.
 *
 * Event commands
 * --------------
 * The following commands are defined by geco:
 *
 * Event command         | Brief description
 * --------------------- | --------------------------
 * NoCmd                 | No command (same as an empty event)
 * CmdExit               | User issued the 'exit' command
 * CmdStart              | User issued the 'sta rt' command
 * CmdStop               | User issued the 'stop' command
 * CmdHold               | User issued the 'hold' command
 * CmdResume             | User issued the 'resume' command
 * CmdProcessTerminated  | A gecoProcess was terminated 
 * CmdProcedureActivated | A gecoProcess was activated
 */

class gecoEvent
{
protected:

  Tcl_Interp*   interp;
  gecoApp*      app;

  int           eventType;
  gecoProcess*  generator;
  int           EvCmd;

  int           EventLoopStatus;
  double        t;

public:

  gecoEvent(gecoApp* App);
  ~gecoEvent();
  
  int      cmd() {return EvCmd;}           /*!< Returns the event command*/
  gecoProcess* eventGenerator();
  void     reset();
  void     setCmdEvent(int Cmd, gecoProcess* EventGenerator);

  void     setEventType(int type)          {eventType=type;}           /*!< Sets the type of the event */
  void     setEventLoopStatus(int status)  {EventLoopStatus=status;}   /*!< Sets the status of the event loop*/
  int      eventLoopStatus()               {return EventLoopStatus;}   /*!< Returns the status of the event loop*/
 
  void     setT(double T)  {t=T;}          /*!< Sets t*/
  double   getT()          {return t;}     /*!< Returns t*/

  gecoApp* getApp()        {return app;}   /*!< Returns the gecoApp in which the geco event loop runs*/
};

#endif /* gecoEvent_SEEN_ */
