// This may look like C code, but it is really -*- C++ -*-
// $Id: ECIO.h 37 2015-01-09 12:35:29Z wuthrich $
// ---------------------------------------------------------------- 
//                                                                  
// Header file for class gecoClock
//
// (c) Rolf Wuthrich
//     2015-2020 Concordia University
//
// author:  Rolf Wuthrich
// email:   rolf.wuthrich@concordia.ca
// version: v1 ($Revision: 37 $)
//
// This software is copyright under the BSD license
//
// ---------------------------------------------------------------
// history:
// ---------------------------------------------------------------
// Date       Modification                     Author
// ---------------------------------------------------------------
// 25.17.2015 Creation                         R. Wuthrich
// 08.12.2020 Added doxygen documentation      R. Wuthrich
//
// ---------------------------------------------------------------

#ifndef gecoClock_SEEN_
#define gecoClock_SEEN_

#include <tcl8.6/tcl.h>
#include "gecoProcess.h"

using namespace std;


// ---------------------------------------------------------------
//
// class gecoClock : class responsible for updating time
//

/** 
 * @brief Class responsible for updating time in the geco process loop
 * \author Rolf Wuthrich
 * \date 2015-2020
 *
 *
 * The gecoClock class is responsible for the time management of the 
 * geco process loop. It must be the first process in the geco process
 * loop. In particular it saves the tick (the time interval between two
 * runs of the geco process loop). The value of the tick can be set by 
 * the user with the the subcommand '-tick' from the Tcl command 'clk'
 * associated to the gecoClock. This Tcl command is already available in 
 * the Tcl interpreter run by an instance of gecoApp.
 *
 * The tick value is used by geco_eventLoop() to register itself 
 * again continuously for a callback.
 *
 * The gecoClock class keeps as well statistics on the execution
 * of the geco process loop.
 *
 * Associated Tcl command
 * ----------------------
 * The associated Tcl command 'clk' is created during the construction 
 * of an instance of gecoClock by its parent class gecoProcess. 
 * The Tcl command is created in the Tcl interpreter
 * run by the gecoApp in which gecoClock lives and the geco process
 * loop runs.
 *
 * The gecoClock class extends the subcommands from gecoObj and
 * gecoProcess by the following subcommands
 *
 * Sub-command       | Short description
 * ----------------- | ------------------
 * -IOStat           | returns statistics on IO operations
 * -tick             | returns/sets clock tick (ms)
 * -reset            | resets the clock
 *
 */

class gecoClock : public gecoProcess
{

  friend void geco_eventLoop(ClientData clientData);

private:

  bool         running;         // 1 if process loops runs
  bool         holdOn;          // 1 if process loop is on hold
  timeval      holdTime;        // time instant since hold was activated
  double       holdDuration;    // duration (in sec) of hold time done so far 
                                // (except current hold interval if any)
								
  // entries for stats
  double   min;
  double   max;
  double   sum_dt;
  double   sqr_sum_dt;
  int      n;

protected:

  int      tick;                /*!< output update rate [ms] i.e. interval between 
                                     two calls of the geco eventLoop */

public:

  gecoClock(gecoApp* App);
  ~gecoClock();

  virtual int  cmd(int &i,int objc,Tcl_Obj *const objv[]);
  virtual void handleEvent(gecoEvent* ev);
  virtual Tcl_DString* info(const char* frontStr = "");

  void addStatReco(double dt);
  void reset();
  void setTick(int Tick) {tick=Tick;}  /*!< sets the tick value */
  int  getTick() {return tick;}        /*!< returns the tick value */

  void hold();
  void resume();

  void iostat();
};


#endif /* gecoClock_SEEN_ */
