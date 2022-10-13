// $Id: ECEvent.cc 15 2014-01-03 18:49:04Z wuthrich $
// ---------------------------------------------------------------
//
// Definition of the class ECEvent
//
// (c) Rolf Wuthrich
//     2015-2016 Concordia University
//
// author:  Rolf Wuthrich
// email:   rolf.wuthrich@concordia.ca
// version: $Revision: 15 $
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

#include "gecoEvent.h"
#include "gecoApp.h"

using namespace std;


// -----------------------------------------------------------------------
//
// class gecoEvent : class for event manipulation and storing
//


/**
 * @brief Constructor
 * @param App gecoApp in which the geco process loop runs
 *
 * The constructor will export the read-only Tcl variable 't' to the 
 * the Tcl interpreter Interp and set it to 0.0. 
 * This variable is updated by the geco process loop.
 * It sets as well the EventLoopStatus to 0 (i.e. not running).
*/

gecoEvent::gecoEvent(gecoApp* App) :
    app(App),
    eventType(NoEvent), 
    generator(NULL),
    t(0.0),
    EventLoopStatus(0)
{
  interp = App->getInterp();
  Tcl_LinkVar(interp, "t", (char *)&t, TCL_LINK_DOUBLE|TCL_LINK_READ_ONLY);
}


/**
 * @brief Destructor
*/

gecoEvent::~gecoEvent()
{
  Tcl_UnlinkVar(interp, "t");
}


/**
 * @brief Returns the gecoProcess which generated the event
 * \return gecoProcess which generated the event
 * Will return 0 if the eventType is not a CmdEvent.
*/

gecoProcess* gecoEvent::eventGenerator()
{
  if (eventType==CmdEvent)
    return generator;
  else
    return 0;
}


/**
 * @brief Resets the event
 */

void gecoEvent::reset()
{
  eventType = NoEvent;
  generator = NULL;
  EvCmd = NoCmd;
}


/**
 * @brief Sets an event command
 * @param Cmd event command to set
 * @param EventGenerator gecoProcess which sets the command
 */

void gecoEvent::setCmdEvent(int Cmd, gecoProcess* EventGenerator)
{
  eventType = CmdEvent;
  EvCmd = Cmd;
  generator = EventGenerator;
}

