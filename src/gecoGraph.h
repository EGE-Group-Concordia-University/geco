// This may look like C code, but it is really -*- C++ -*-
// $Id: ECTrigger.h 15 2014-01-03 18:49:04Z wuthrich $
// ---------------------------------------------------------------- 
//                                                                  
// Header file for class gecoGraph
//
// (c) Rolf Wuthrich
//     2015 - 2020 Concordia University
//
// author:  Rolf Wuthrich
// email:   rolf.wuthrich@concordia.ca
// version: v2 ($Revision: 15 $)
//
// This software is copyright under the BSD license
//
// ---------------------------------------------------------------
// history:
// ---------------------------------------------------------------
// Date       Modification                     Author
// ---------------------------------------------------------------
// 24.10.2015 Creation                         R. Wuthrich
// 09.12.2020 Added doxygen documentation      R. Wuthrich
//
// ---------------------------------------------------------------
/*! \file */

#ifndef gecoGraph_SEEN_
#define gecoGraph_SEEN_

#include <tcl.h>
#include <stdio.h>
#include <fstream>
#include <limits.h>	      // for PATH_MAX 
#include "gecoProcess.h"

using namespace std;


// -----------------------------------------------------------------------
//
// Tcl interface
//

/**
 * @brief C++ implementation of the Tcl command to create a gecoGraph object
 * @param clientData pointer to the gecoApp in which the gecoGraph will live
 * @param interp Tcl interpreter in which the Tcl command is executed
 * @param objc number of arguments of the Tcl command
 * @param objv arguments of the the Tcl command
 * \return TCL_OK if the execution of the Tcl command is successful and TCL_ERROR otherwise
 */

int geco_GraphCmd(ClientData clientData, Tcl_Interp *interp, 
		  int objc,Tcl_Obj *const objv[]);


// -----------------------------------------------------------------------
//
// class gecoGraph : a class for plotting data
//

/** 
 * @brief A gecoProcess to plotting data
 * \author Rolf Wuthrich
 * \date 2015-2020
 *
 * The gecoGraph class allows to create a gecoProcess able to interact
 * with gnuplot in order to graph data, in form of Tcl variables defined 
 * in the Tcl interpreter of the gecoApp in which the gecoGraph instance
 * lives.
 *
 * To produce the graph, gecoGraph creates a pipe to gnuplot. Consequently
 * gnuplot must be installed on the system running geco and graphical
 * display must be possible. Data is exchanged via a temporary file in the
 * home folder of the user.
 *
 * The geco_GraphCmd() is the C++ implementation for the Tcl command to
 * create gecoGraph objects. This Tcl command is already available in 
 * the Tcl interpreter run by an instance of gecoApp under the name 'graph'.
 *
 * Associated Tcl command
 * ----------------------
 * Every gecoGraph is associated to a Tcl command. The associated Tcl command 
 * is created during the construction of an instance of gecoGraph by its
 * parent class. The Tcl command is created in the Tcl interpreter
 * run by the gecoApp in which gecoGraph lives.
 *
 * The gecoGraph class extends the subcommands from gecoObj and gecoProcess
 * by the following subcommands
 *
 * Sub-command       | Short description
 * ----------------- | ------------------
 * -stop             | stops to send data to the graph
 * -reset            | resets the graph
 * -dtRecord         | returns/sets time interval between two recordings (s)
 * -dtUpdate         | returns/sets time interval between two graph updates (s)
 * -x                | sets/returns the data for the X-coordinate for plotting
 * -y                | sets/returns the data for the Y-coordinate for plotting
 * -style            | sets/returns the style for plotting
 *
 * To choose the data to be plotted the '-x' and '-y' have to be used.
 * The user can pass any Tcl expression that can be evaluated by the Tcl command 'expr'
 * which output will be the x, respectively the y, coordinate of the data point.
 * For example '-x $Var' will use the content of the Tcl variable $Var
 * as x-coordinate. The Tcl command
 * \code
 * graph -x {$t} -y {$t*$t}
 * \endcode
 * will add a graph process to the geco process loop plotting t versus the square
 * of t where t is the time since the start of the geco process loop.
 *
 * The subcommand '-style' can be used to set the plot-style of the graph
 * using the syntax from gnuplot (see man pages f gnuplot). 
 * The default value is 'with lines notitle'.
 */

class gecoGraph : public gecoProcess
{
private:

  char         fname[PATH_MAX]; // full file name of the temporary graph file
  Tcl_DString* plotString;
  double       saveTime;
  double       graphTime;

protected:

  int          graphFile;

  double       dtRecord;        // time interval between two recording
  double       dtUpdate;        // time interval between two graph update
  Tcl_DString* xCoord;          // x coordinate
  Tcl_DString* yCoord;          // y coordinate
  Tcl_DString* plotStyle;       // plot style

public:

  gecoGraph(gecoApp* App);
  ~gecoGraph();

  virtual int  cmd(int &i,int objc,Tcl_Obj *const objv[]);
  virtual void handleEvent(gecoEvent* ev);
  virtual Tcl_DString* info(const char* frontStr = "");

  virtual void activate(gecoEvent* ev);
};

#endif /* gecoGraph_SEEN_ */
