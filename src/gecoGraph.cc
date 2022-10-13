// $Id: ECTrigger.cc 15 2014-01-03 18:49:04Z wuthrich $
// ---------------------------------------------------------------
//
// Definition of the class gecoGraph
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
// 24.10.2015 Creation                         R. Wuthrich
// 09.12.2020 Added doxygen documentation      R. Wuthrich
//
// ---------------------------------------------------------------

#include <tcl.h>
#include <cstring>
#include "gecoGraph.h"

#include <stdlib.h>
#include <unistd.h>

using namespace std;

#define TEMPLATE "/tmp/gecoGraphXXXXXX"    // Temporary file name template 


// -------------------------------------------------------------------------
//
// Tcl interface
//


// Command to create a new graph object
//

int geco_GraphCmd(ClientData clientData, Tcl_Interp *interp, 
		    int objc,Tcl_Obj *const objv[])
{
  gecoGraph* proc  = new gecoGraph((gecoApp *)clientData);
  return geco_CreateGecoProcessCmd(proc,objc,objv);
}

// -------------------------------------------------------------------------


// -------------------------------------------------------------------------
//
// class gecoGraph : a class for inloop graphing of data
//


/**
 * @brief Constructor
 * @param App gecoApp in which the gecoProcess lives 
 *
 * The constructor will create a Tcl command via the call of
 * the constructor of gecoObj. It further defines additional
 * subcommands.
*/

gecoGraph::gecoGraph(gecoApp* App) :
    gecoObj("Graph","graph",App),
    gecoProcess("Graph","user","graph",App), 
    saveTime(0.0),
    graphTime(0.0)
{
  activateOnStart=1;
  // open temporary file for graph data
  strcpy(fname, TEMPLATE);
  graphFile = mkstemp(fname); 

  // variable for recording and update
  dtRecord=0.1;
  dtUpdate=0.5;

  // variables for coordinates and plot style
  xCoord     = new Tcl_DString;
  yCoord     = new Tcl_DString;
  plotStyle  = new Tcl_DString;
  plotString = new Tcl_DString;
  Tcl_DStringInit(xCoord);
  Tcl_DStringInit(yCoord);
  Tcl_DStringInit(plotStyle);
  Tcl_DStringInit(plotString);
  Tcl_DStringAppend(xCoord, "$t", -1);
  Tcl_DStringAppend(plotStyle, "with lines notitle", -1);

  addOption("-stop", "stops to send data to the graph");
  addOption("-reset", "resets the graph");
  addOption("-dtRecord", &dtRecord,
	    "returns/sets time interval between two recording (s)");
  addOption("-dtUpdate", &dtUpdate,
	    "returns/sets time interval between two graph updates (s)");
  addOption("-x", xCoord, "sets/returns the X-coordinate for plotting");
  addOption("-y", yCoord, "sets/returns the Y-coordinate for plotting");
  addOption("-style", plotStyle, "sets/returns the style for plotting");
}


/**
 * @brief Destructor
*/

gecoGraph::~gecoGraph()
{
  close(graphFile);
  unlink(fname); 
  Tcl_DStringFree(xCoord);
  Tcl_DStringFree(yCoord);
  Tcl_DStringFree(plotStyle);
  Tcl_DStringFree(plotString);
  delete xCoord;
  delete yCoord;
  delete plotStyle;
  delete plotString;
}


/*!
 * @copydoc gecoProcess::cmd 
 *
 * Compared to gecoProcess::cmd, gecoGraph::cmd adds the processing of
 * the new subcommands of gecoGraph.
 */

int gecoGraph::cmd(int &i, int objc,Tcl_Obj *const objv[])
{
  // first executes the command options defined in gecoProcess
  int index=gecoProcess::cmd(i,objc,objv);

  if (index==getOptionIndex("-reset"))
    {
      close(graphFile);
      unlink(fname); 
      strcpy(fname, TEMPLATE);
      graphFile = mkstemp(fname); 
      i++;
    }

  return index;
}


/**
 * @copydoc gecoProcess::handleEvent
 *
 * In addition to gecoProcess::handleEvent, gecoGraph::handleEvent implements
 * the sending of data to gnuplot.
 *
 * For this gecoGraph::handleEvent does at a frequency controlled
 * by dtRecord the calculation of the x- and y-coordinates using the
 * Tcl command 'expr' applied to the information stored in 'x' and 'y'
 * followed by storing it to the temporary file. 
 *
 * With a frequency controlled by 'dtUpdate' gecoGraph::handleEvent
 * asks gnuplot to plot the content of the temporary file.
 */

void gecoGraph::handleEvent(gecoEvent* ev)
{
  gecoProcess::handleEvent(ev);

  if (status!=Active) return;

  if (ev->getT()-saveTime>=dtRecord)
	{
	  saveTime=ev->getT();
	  Tcl_DStringFree(plotString);
	  Tcl_DStringAppend(plotString, "expr {", -1);
	  Tcl_DStringAppend(plotString, Tcl_DStringValue(xCoord), -1);
	  Tcl_DStringAppend(plotString, "}", -1);
	  Tcl_Eval(interp,Tcl_DStringValue(plotString));
	  write(graphFile, Tcl_GetStringResult(interp),
		          strlen(Tcl_GetStringResult(interp)));
	  write(graphFile, "\t", 1);
	  Tcl_DStringFree(plotString);
	  Tcl_DStringAppend(plotString, "expr {", -1);
	  Tcl_DStringAppend(plotString, Tcl_DStringValue(yCoord),-1);
	  Tcl_DStringAppend(plotString, "}", -1);
	  Tcl_Eval(interp,Tcl_DStringValue(plotString));
	  write(graphFile,Tcl_GetStringResult(interp),
		          strlen(Tcl_GetStringResult(interp)));
	  write(graphFile, "\n", 1);
	  Tcl_ResetResult(interp);
	}

  if (ev->getT()-graphTime>=dtUpdate)
    {
      graphTime=ev->getT();
      fsync(graphFile);
      Tcl_DStringFree(plotString);
      Tcl_DStringAppend(plotString, "plotGnuplot ", -1);
      Tcl_DStringAppend(plotString, fname, -1);
      Tcl_DStringAppend(plotString, " ", -1);
      Tcl_DStringAppend(plotString, Tcl_DStringValue(plotStyle), -1);
      Tcl_Eval(interp, Tcl_DStringValue(plotString));
      Tcl_ResetResult(interp);
    }

}


/**
 * @copydoc gecoProcess::info
 *
 * In addition to gecoProcess::info, gecoGraph::info adds the information
 * about the data to plot.
 */ 

Tcl_DString* gecoGraph::info(const char* frontStr)
{
  gecoProcess::info(frontStr);
  addInfo(frontStr, "X-coordinate: ", Tcl_DStringValue(xCoord));
  addInfo(frontStr, "Y-coordinate: ", Tcl_DStringValue(yCoord));
  addInfo(frontStr, "Plot style:   ", Tcl_DStringValue(plotStyle));
  return infoStr;
}


/**
 * @copydoc gecoProcess::activate
 *
 * In addition to gecoProcess::activate, gecoGraph::activate 
 * creates and opens for writing a temporary file used to send data to gnuplot.
 */

void gecoGraph::activate(gecoEvent* ev)
{
  gecoProcess::activate(ev);

  close(graphFile);
  unlink(fname); 
  strcpy(fname, TEMPLATE);
  graphFile = mkstemp(fname); 

  saveTime=ev->getT();
  graphTime=ev->getT();
}
