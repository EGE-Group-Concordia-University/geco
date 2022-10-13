// $Id: ECTrigger.cc 15 2014-01-03 18:49:04Z wuthrich $
// ---------------------------------------------------------------
//
// Definition of the class gecoGenerator
//
// (c) Rolf Wuthrich
//     2016 - 2020 Concordia University
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
// 01.02.2015 Creation                         R. Wuthrich
// 03.12.2020 Added doxygen documentation      R. Wuthrich
//
// ---------------------------------------------------------------

#include <tcl.h>
#include "gecoGenerator.h"

using namespace std;


// -------------------------------------------------------------------------
//
// class gecoGenerator : abstract class to generate signals
//


/**
 * @brief Constructor
 * @param procName name of the gecoGenerator
 * @param procCmd Tcl command associated to the gecoGenerator
 * @param App gecoApp in which the gecoGenerator instance lives 
 *
 * The constructor will create a Tcl command via the call of
 * the constructor of gecoObj. It further defines additional
 * subcommands of gecoGenerator.
*/

gecoGenerator::gecoGenerator(const char* procName, const char* procCmd, 
			     gecoApp* App) :
  gecoObj(procName, procCmd, App),
  gecoProcess(procName, "user", procCmd, App)
{
  outputVar = new Tcl_DString;
  Tcl_DStringInit(outputVar);
  addOption("-outputVariable", outputVar, "returns/sets Tcl output variable");
}


/**
 * @brief Destructor
*/

gecoGenerator::~gecoGenerator()
{
  Tcl_DStringFree(outputVar);
  delete outputVar;
}


/**
 * @copydoc gecoProcess::handleEvent
 *
 * In addition to gecoProcess::handleEvent, gecoGenerator::handleEvent implements
 * the call to gecoGenerator::signalFunction in order to compute the signal and copy
 * it's output to the associated Tcl variable.
 */

void gecoGenerator::handleEvent(gecoEvent* ev)
{
  gecoProcess::handleEvent(ev);

  if (status!=Active) return;

  // calls the signalFunction method in order to compute the output
  char str[80];
  sprintf(str, "%f", signalFunction(ev->getT()));
  Tcl_SetVar(interp, Tcl_DStringValue(outputVar), str, 0);
}


/**
 * @copydoc gecoProcess::info
 *
 * In addition to gecoProcess::info, gecoGenerator::info adds the information
 * about the associated Tcl variable.
 */

Tcl_DString* gecoGenerator::info(const char* frontStr)
{
  gecoProcess::info(frontStr);
  addInfo(frontStr,"Output variable = ", Tcl_DStringValue(outputVar));
  return infoStr;
}


/**
 * @brief Computes the signal
 * @param t time since the geco event loop got started
 * \return The computed signal
 *
 * This method has to be implemented by the children of gecoGenerator.
 */

double gecoGenerator::signalFunction(double t)
{
  return 0.0;
}


/**
 * @brief The sign function
 * @param x a number
 * \return +1 if x is positive or null; -1 otherwise
 * 
 * This function is available to be used in children of gecoGenerator as C++ does not
 * provide a sing function.
 */

int gecoGenerator::sign(double x)
{
  if (x>=0)
    return 1;
  else 
    return -1;
}
