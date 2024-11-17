// This may look like C code, but it is really -*- C++ -*-
// $Id: ECTrigger.h 15 2014-01-03 18:49:04Z wuthrich $
// ---------------------------------------------------------------- 
//
// Header file for class gecoGenerator
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
// 01.02.2016 Creation                         R. Wuthrich
// 03.12.2020 Added doxygen documentation      R. Wuthrich
//
// ---------------------------------------------------------------

#ifndef gecoGenerator_SEEN_
#define gecoGenerator_SEEN_

#include <tcl8.6/tcl.h>
#include <stdio.h>
#include "gecoProcess.h"

using namespace std;


// -----------------------------------------------------------------------
//
// class gecoGenerator : abstract class to generate signals
//

/** 
 * @brief Abstract class to create a gecoProcess able to generate signals
 * \author Rolf Wuthrich
 * \date 2015-2020
 *
 *
 * The gecoGenerator class is an abstract class to create a gecoProcess able
 * to generate signals. Compared to gecoProcess, from which gecoGenerator
 * derives, it add specialized methods for the generation of a signal.
 *
 * The method gecoGenerator::signalFunction can be overloaded in order to 
 * implement the signal to be generated. This function is called
 * by gecoGenerator::handleEvent at each pass of the geco process loop.
 * The return value of gecoGenerator::signalFunction is then stored into a
 * Tcl variable which is associated to the gecoGenerator object.
 *
 * Associated Tcl command
 * ----------------------
 * Every gecoGenerator is associated to a Tcl command. The associated Tcl command 
 * is created during the construction of an instance of gecoGenerator by its
 * parent class gecoProcess. The Tcl command is created in the Tcl interpreter
 * run by the gecoApp in which gecoGenerator lives.
 *
 * The gecoGenerator class extends the subcommands from gecoObj and
 * gecoProcess by the following subcommands
 *
 * Sub-command       | Short description
 * ----------------- | ------------------
 * -outputVariable   | returns/sets Tcl output variable
 *
 * Associated Tcl variable
 * -----------------------
 * Every gecoGenerator is associated to a Tcl variable. This variable holds 
 * the value of the signal generated as the the gecoGenerator is called by
 * the geco process loop. The Tcl variable can be set by the user with the Tcl
 * subcommand '-outputVariable'. The gecoGenerator class updates this variable
 * in the gecoGenerator::handleEvent method using gecoGenerator::signalFunction, which
 * has to be implemented by the children of gecoGenerator.
 *
 * Example
 * -------
 * To implement a child of gecoGenerator the method gecoGenerator::signalFunction must 
 * be implemented. In the present example we implement a square signal, which outputs
 * the square of the time t since the geco process loop got started.
 *
 * The new class will be gecoSquare. The function method writes
 * \snippet gecoGeneratorEx.cc function
 * Note that 't' is the time since the geco loop got started. If one wants
 * to use the time since the gecoProcess got activated, one should correct with
 * a call to timeSinceActivated().
 *
 * In this example we further want to add the subcommand '-duration' which will
 * allow the user to chose how long the signal lasts. For this we add a protected 
 * variable duration to our class which we set to 0.0 in the constructor. We
 * add as well the subcommand '-duration' with the addOption() method:
 * \snippet gecoGeneratorEx.cc Constructor
 *
 * To stop the gecoSquare process after the time duration, we need to overload
 * the gecoGenerator::handleEvent method:
 * \snippet gecoGeneratorEx.cc handleEvent
 * Once the time since the activation of the gecoSquare process exceeds duration,
 * the process calls the gecoProcess::terminate method to terminate it.
 *
 * As reference we give here the full class definition
 * \snippet gecoGeneratorEx.cc Class definition
 *
 * Finally we create a Tcl command that allows the user to create gecoSquare processes.
 * Using the geco_CreateGecoProcessCmd() simplifies this task greatly:
 * \snippet gecoGeneratorEx.cc Tcl interface
 *
 * The following code snippet gives an example how the new gecoSquare class and Tcl command
 * can be attached to a gecoApp:
 * \snippet gecoGeneratorEx.cc main
 */

class gecoGenerator : public gecoProcess
{

protected:

  Tcl_DString* outputVar;

public:

  gecoGenerator(const char* procName, const char* procCmd, gecoApp* App);
  ~gecoGenerator();

  virtual void handleEvent(gecoEvent* ev);
  virtual Tcl_DString* info(const char* frontStr = "");

  virtual double signalFunction(double t);

  int     sign(double x);
};

#endif /* gecoGenerator_SEEN_ */
