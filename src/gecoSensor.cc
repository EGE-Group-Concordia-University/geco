// $Id: ECIOModule.cc 37 2015-01-09 12:35:29Z wuthrich $
// ---------------------------------------------------------------
//
// Definition of the class gecoIOModule
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

#include <tcl.h>
#include <cstring>
#include "gecoSensor.h"
#include "gecoApp.h"

using namespace std;


// ---------------------------------------------------------------
//
// class gecoSensor : abstract class for geCo sensor processes
//


/**
 * @brief Constructor
 * @param sensorName name of the gecoSensor
 * @param sensorCmd Tcl command associated to the gecoSensor
 * @param App gecoApp in which the gecoSensor instance lives 
 *
 * The constructor will create a Tcl command via the call of
 * the constructor of gecoObj. It further defines additional
 * subcommands of gecoSensor.
*/

gecoSensor::gecoSensor(const char* sensorName, const char* sensorCmd, gecoApp* App) :
  gecoObj(sensorName, sensorCmd, App),
  gecoProcess(sensorName, "user", sensorCmd, App)
{
  TclVar = new Tcl_DString;
  Tcl_DStringInit(TclVar);
  
  addOption("-TclVariable", TclVar, "returns/sets linked Tcl variable");
}


/**
 * @brief Destructor
*/

gecoSensor::~gecoSensor()
{
  Tcl_DStringFree(TclVar);
  delete TclVar;
}


/*!
 * @copydoc gecoProcess::cmd 
 *
 * Compared to gecoProcess::cmd, gecoSensor::cmd adds the processing of
 * the new subcommands of gecoSensor.
 */

int gecoSensor::cmd(int &i,int objc,Tcl_Obj *const objv[])
{
  // executes the command options defined in gecoProcess
  int index=gecoProcess::cmd(i,objc,objv);

  return index;
}


/**
 * @copydoc gecoProcess::handleEvent
 *
 * In addition to gecoProcess::handleEvent, gecoSensor::handleEvent implements
 * the call to gecoSensor::readSensor in order to read the sensor signal and copy
 * it's output to the associated Tcl variable.
 */

void gecoSensor::handleEvent(gecoEvent* ev)
{
  gecoProcess::handleEvent(ev);

  if (status!=Active) return;

  // calls the sensor read procedure and fills the Tcl variable
  char str[80];
  sprintf(str,"%f", readSensor());
  Tcl_SetVar(interp, Tcl_DStringValue(TclVar), str, 0);
}


/**
 * @copydoc gecoProcess::info
 *
 * In addition to gecoProcess::info, gecoSensor::info adds the information
 * about the associated Tcl variable.
 */ 

Tcl_DString* gecoSensor::info(const char* frontStr)
{
  gecoProcess::info(frontStr);
  addInfo(frontStr,"Tcl variable = ", Tcl_DStringValue(TclVar));
  return infoStr;
}


/**
 * @brief Reads the sensor signal
 * \return The sensor signal
 *
 * This method has to be implemented by the children of gecoSensor.
 */

double gecoSensor::readSensor()
{
  return 0.0;
}







