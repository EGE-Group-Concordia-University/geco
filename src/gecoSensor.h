// This may look like C code, but it is really -*- C++ -*-
// $Id: ECIOModule.h 37 2015-01-09 12:35:29Z wuthrich $
// ---------------------------------------------------------------- 
//                                                                  
// Header file for the class gecoSensor
//
// (c) Rolf Wuthrich
//     2020 Concordia University
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
// 06.11.2020 Creation                         R. Wuthrich
// 08.12.2020 Added doxygen documentation      R. Wuthrich
//
// ---------------------------------------------------------------

#ifndef gecoSensor_SEEN_
#define gecoSensor_SEEN_

#include <tcl8.6/tcl.h>
#include "gecoProcess.h"
#include "gecoEvent.h"

using namespace std;



// -----------------------------------------------------------------------
//
// class gecoSensor : abstract class for geCo sensor processes
//

/** 
 * @brief Abstract class to create a gecoProcess able to implements reading of sensors
 * \author Rolf Wuthrich
 * \date 2020
 *
 *
 * The gecoSensor class is an abstract class to create a gecoProcess reading
 * signals from sensor. Compared to gecoProcess, from which gecoSensor
 * derives, it add specialized methods for the reading of sensor signal.
 *
 * The method gecoSensor::readSensor can be overloaded in order to 
 * implement the sensor signal reading. This function is called
 * by gecoSensor::handleEvent at each pass of the geco process loop.
 * The return value of gecoSensor::readSensor is then stored into a
 * Tcl variable which is associated to the gecoSensor object.
 *
 * Associated Tcl command
 * ----------------------
 * Every gecoSensor is associated to a Tcl command. The associated Tcl command 
 * is created during the construction of an instance of gecoSensor by its
 * parent class gecoProcess. The Tcl command is created in the Tcl interpreter
 * run by the gecoApp in which the instance of gecoSensor lives.
 *
 * The gecoSensor class extends the subcommands from gecoObj and
 * gecoProcess by the following subcommands
 *
 * Sub-command       | Short description
 * ----------------- | ------------------
 * -TclVariable      | returns/sets linked Tcl variable
 *
 * Associated Tcl variable
 * -----------------------
 * Every gecoSensor is associated to a Tcl variable. This variable holds 
 * the value of the sensor signal that is acquired as gecoSensor is called by
 * the geco process loop. The Tcl variable can be set by the user with the Tcl
 * subcommand '-TclVariable'. The gecoSensor updates this variable
 * in the gecoSensor::handleEvent method using the gecoSensor::readSensor method,
 * which has to be implemented by the children of gecoSensor.
 */

class gecoSensor : public gecoProcess
{

protected:

  Tcl_DString*   TclVar;             // Linked Tcl variable

public:

  gecoSensor(const char* sensorName, const char* sensorCmd, gecoApp* App);
  virtual ~gecoSensor();

  virtual int  cmd(int &i,int objc,Tcl_Obj *const objv[]);
  virtual void handleEvent(gecoEvent* ev);
  virtual Tcl_DString* info(const char* frontStr = "");

  Tcl_DString*   getTclVar() {return TclVar;}

  virtual double readSensor();

};


#endif /* gecoSensor_SEEN_ */
