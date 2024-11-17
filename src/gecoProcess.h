// This may look like C code, but it is really -*- C++ -*-
// $Id: ECProcess.h 25 2014-03-16 17:03:44Z wuthrich $
// ---------------------------------------------------------------- 
//                                                                  
// Header file for class gecoProcess
//
// (c) Rolf Wuthrich
//     2015 - 2020 Concordia University
//
// author:  Rolf Wuthrich
// email:   rolf.wuthrich@concordia.ca
// version: ($Revision: 25 $
//
// This software is copyright under the BSD license
//
// ---------------------------------------------------------------
// history:
// ---------------------------------------------------------------
// Date       Modification                     Author
// ---------------------------------------------------------------
// 17.10.2015 Creation                         R. Wuthrich
// 28.11.2020 Added doxygen documentation      R. Wuthrich
//
// ---------------------------------------------------------------
/*! \file */

#ifndef gecoProcess_SEEN_
#define gecoProcess_SEEN_

#include <sys/time.h>
#include <tcl8.6/tcl.h>
#include "gecoEvent.h"
#include "gecoObj.h"
#include "gecoApp.h"

using namespace std;


// -----------------------------------------------------------------------
//
// Process status types
//

const int
  Waiting    = 0,         // process waiting to be executed
  Active     = 1,         // process under execution
  Hold       = 2,         // process on hold
  Terminated = 3;         // process terminated

const char StatusStr[4][11] = {"Waiting", "Active", "Hold", "Terminated"};


// -------------------------------------------------------------------------
//
// Tcl interface

/**
 * @brief Function to create a new Tcl command for an existing gecoProcess
 * @param clientData pointer to the gecoProcess for which the Tcl command has to be created
 * @param objc number of arguments of the Tcl command
 * @param objv arguments of the the Tcl command
 * \return TCL_OK if the creation of the Tcl command was successful and TCL_ERROR otherwise
 *
 * The function geco_CreateGecoProcessCmd() can be used to setup a new Tcl command
 * available to the user to create interactively gecoProcess objects and upload them to
 * the geco process loop. objv are the arguments with which this new Tcl command was
 * invoked by the user.
 *
 * The function is typically used inside a Tcl_ObjCmdProc() type function defining
 * a Tcl command which allows the user to create interactively gecoProcess objects.
 * Such a function will typically look like
 * \code
 * int mygecoProcessCmd(ClientData clientData, int objc, Tcl_Obj *const objv[])
 * {
 *   mygecoProcess* proc = new mygecoProcess((gecoApp *)clientData);
 *   return geco_CreateGecoProcessCmd(proc, objc, objv);
 * }
 * \endcode
 *
 * The function geco_CreateGecoProcessCmd() will create a new Tcl command for
 * the gecoProcess proc and invoke this command with the parameters objv. It will
 * as well add proc to the geco process loop of the gecoApp in which proc is living. 
 *
 * If objv is the '-help' subcommand, then the help text of the Tcl command 
 * associated to proc will be displayed and afterwards the gecoProcess proc will 
 * be deleted. 
 *
 * Otherwise it will invoke the geco_gecoObjCmd() function with proc and objv
 * in order to execute the Tcl command associated to the gecoProcess proc and handle
 * the various subcommands from proc. In case any error occurs (TCL_ERROR is
 * returned by the geco_gecoObjCmd() function) the gecoProcess proc is deleted.
 * If geco_gecoObjCmd() returns TCL_OK the gecoProcess proc is added to the
 * gecoApp in in which the gecoProcess lives and the result of the Tcl command 
 * is left in the interpreter of the gecoApp.
 *
 * For an example usage refer to the example presented in the description of 
 * the class gecoProcess.
 *
 */

int geco_CreateGecoProcessCmd(gecoProcess* proc, int objc, 
			      Tcl_Obj *const objv[]);

// -----------------------------------------------------------------------
//
// class gecoProcess : abstract class from which all geco processes derive
//

/** 
 * @brief Abstract class from which all geco processes derive
 * \author Rolf Wuthrich
 * \date 2015-2020
 *
 *
 * The gecoProcess class is the abstract class from which all geco process derive.
 * The class implements an interface to the associate Tcl command defined in
 * the Tcl interpreter of the gecoApp in which the gecoProcess lives.
 *
 * The gecoProcess class is a derived class of gecoObj.
 *
 * Geco processes are inserted to the geco process loop at its creation by
 * the user.
 *
 * Geco process loop
 * -----------------
 * The geco process loop is an ordered list of gecoProcess. They 
 * are executed sequentially with a frequency controlled by the 
 * gecoClock process. To start the execution of the loop the Tcl 
 * command 'start' has to be used. The Tcl command 'stop' will stop it
 * and the commands 'hold' and 'resume' pause and resumes its execution.
 *
 * A gecoProcess in the event loop will have one the following status
 *
 * geco-Process status | Description
 * ------------------- | ----------------------------------------
 * Waiting             | process waiting to be activated
 * Active              | process is active (under execution)
 * Hold                | process is on hold
 * Terminated          | process is terminated and can no longer be activated
 *
 * When the Tcl command 'start' is invoked every process set to be
 * 'active on start' (with the '-activateOnStart' subcommand) will 
 * be activated. The first gecoProcess after the gecoClock process
 * will be activated too. 
 * Geco then loops over all gecoProcess in the geco process loop. For
 * each gecoProcess the method gecoProcess::handleEvent is called. 
 * Each geco process in the 'active' status can then execute the
 * specific actions to the gecoProcess (e.g. compute the value of a variable).
 * 
 * When a gecoProcess no longer wants to be in the 'active' status,
 * the method gecoProcess::terminate must be called. This will signal
 * to the geco event loop that the gecoProcess is terminated and that the
 * next following process in 'waiting' status must be activated.
 *
 * Activating a gecoProcess
 * ------------------------
 * Whenever a gecoProcess gets activated, a Tcl variable ID containing
 * the process ID is defined in the Tcl interpreter of the gecoApp in
 * which the gecoProcess lives. Then the pre-process script (as
 * defined with the '-preprocess' subcommand) is invoked and the finally
 * the gecoProcess status is set to 'active'.
 *
 * A gecoProcess can be activated either by
 *  * a call to the gecoProcess::activate method
 *  * a call to the Tcl command 'activate' defined by the gecoApp
 *  * the geco process loop when the presiding gecoProcess in
 *    the geco process loop is terminated and the status of the
 *    gecoProcess is 'waiting'.
 *
 * Terminating a gecoProcess
 * -------------------------
 * Whenever a gecoProcess gets terminated, a Tcl variable ID containing
 * the process ID is defined in the Tcl interpreter of the gecoApp in
 * which the gecoProcess lives. Then the post-process script (as
 * defined with the '-postprocess' subcommand) is invoked and the finally
 * the gecoProcess status is set to 'terminated'.
 *
 * A gecoProcess can be terminated either by
 *  * a call to the gecoProcess::terminate method
 *  * a call to the Tcl command 'terminate' defined by the gecoApp
 *
 * Associated Tcl command
 * ----------------------
 * Every gecoProcess is associated to a Tcl command. The associated Tcl command 
 * is created during the construction of an instance of gecoProcess by its
 * parent class gecoObj. The Tcl command is created in the Tcl interpreter
 * run by the gecoApp in which the gecoProcess lives.
 *
 * The gecoProcess class extends the subcommands from gecoObj by 
 * the following subcommands
 *
 * Sub-command       | Short description
 * ----------------- | ------------------
 * -verbose          | turns on/off messaging to console
 * -activateOnStart  | turns on/off activation on start command
 * -preprocess       | returns/sets pre-process script
 * -postprocess      | returns/sets post-process script  
 * -status           | returns the process status
 *
 * This Tcl command can be used to alter the gecoProcess, by for example
 * updating parameters (e.g the pre-pocess script).
 *
 * In general a gecoProcess should be associated to a further Tcl 
 * command to allow a user of the gecoApp to create a gecoProcess object.
 * The function 
 * geco_CreateGecoProcessCmd(gecoProcess* proc, int objc, Tcl_Obj *const objv[])
 * can be used to help to develop the C++ implementation of this Tcl command.
 * This function takes care of registering the gecoProcess to the geco loop
 * of the gecoApp in which the gecoProcess lives.
 * It avoids as well the creation of the new gecoProcess object in the geco
 * process loop in case the user only invokes the '-help' subcommand.
 * See the example section for an illustration of how to use this function.
 *
 * Example
 * -------
 * The following example code illustrated how a new gecoProcess can be created
 * as a child of gecoProcess.
 *
 * The example creates a new gecoProcess, called gecoNetTraff, which will
 * monitor the network in and out traffic based on the entries in  
 * sysfs-class-net-statistics.
 *
 * In Linux under /sys/class/net/<ifac>/statistics can be found two files
 * rx_bytes and tx_bytes which contain the total inward and outward network 
 * traffic on the network interface <ifac>.
 * A complete documentation can be found under
 * https://www.kernel.org/doc/Documentation/ABI/testing/sysfs-class-net-statistics
 *
 * The new gecoProcess gecoNetTraff will read the entries of these files in ordered
 * to determine the current transmission rates.
 * The gecoNetTraff class is a very basic example. It does for example no error
 * check if the requested network interface to monitor is valid or not.
 *
 * To create a new gecoProcess typically the constructor, destructor and the
 * gecoProcess::handleEvent methods need to be implemented. The present example
 * will as well illustrate how the gecoProcess::activate and
 * gecoProcess::terminate methods can be used.
 *
 * The constructor of gecoNetTraff is as follows
 * \snippet gecoProcessEx.cc Constructor
 *
 * The constructor requires as parameters a pointer (App) to the gecoApp in which the
 * gecoNetTraff process will live.
 *
 * The constructor calls the gecoProcess and gecoObj constructors which has as effect
 * to create a gecoObj of name 'Network traffic', with an associated
 * Tcl command 'netTraff<ID>', where gecoObj will add the unique <ID> to the
 * end of the command. It further tells geco that this is a gecoProcess created by
 * the user. 
 *
 * The constructor initialize the Tcl_DString netDevice. It further defines 
 * a subcommand '-networkInterface' for the associated Tcl command of
 * the gecoProcess. This subcommand allows the user to check or change the
 * network interface to be monitored. 
 * If more complex subcommands are defined (not simple assignment/display
 * of variables), the gecoProcess::cmd needs be reimplemented (see the documentation 
 * of gecoObj for an example).
 *
 * The destructor of gecoNetTraff is as follows
 * \snippet gecoProcessEx.cc Destructor
 *
 * It frees the Tcl_DString netDevice.
 *
 * Next we implement the gecoProcess::handleEvent method for gecoNetTraff:
 * \snippet gecoProcessEx.cc handleEvent
 *
 * The method first calls gecoProcess::handleEvent. It then checks 
 * if the gecoProcess is active or not. If not active nothing more needs to be done.
 *
 * The core of the method computes the inward and outward network traffic rates
 * in kB/sec. For this it reads the current amount of traffic with the two
 * auxiliary functions gecoNetTraff::RXtraffic() and gecoNetTraff::TXtraffic()
 * \snippet gecoProcessEx.cc auxiliary functions
 * It then compares it with the amount red in the previous pass of the geco event loop.
 * The previous amount is stored in RXbytes and TXbytes. The time interval
 * spent since the previous call to handleEvent is computed by taking the difference 
 * between prevT (time of the previous call) and ev->getT(), which is the time
 * of the present call tp handleEvent.
 *
 * It then stores the computed values in the two Tcl variables 'RXrate' and
 * 'TXrate' in the Tcl interpreter run by the gecoApp in which gecoNetTraff lives.
 *
 * The methods ends by updating the variables RXbytes, TXbytes and prevT.
 *
 * In order the computed values in handleEvent make sense as well for the very first
 * round of the geco-process loop, the variables RXbytes, TXbytes and prevT must be
 * correctly initialized at the moment of the activation of the gecoNetTraff object.
 * Further the two files rx_bytes and tx_bytes from /sys/class/net/<ifac>/statistics
 * have to be opened.
 * This can be done in the gecoProcess::activate method which is called at the moment
 * gecoNetTraff gets activated:
 * \snippet gecoProcessEx.cc activate
 *
 * Once the gecoNetTraff gets terminated the two files rx_bytes and tx_bytes, 
 * needs to be closed. This can be implemented with the gecoProcess::terminate method::
 * \snippet gecoProcessEx.cc terminate
 *
 * For reference, the complete class definition is
 * \snippet gecoProcessEx.cc Class definition
 * We added as well a setter and getter function for the protected variable netDevice.
 *
 * The following code shows how the new gecoNetTraff class can be used.
 * In the present example, the network interface 'eno1' is monitored. 
 * Besides creating a gecoNetTraff object, a gecoEnd process is created in order
 * to form a complete geco-process loop. The clock tick is further set to 1 sec.
 * \snippet gecoProcessEx.cc main
 * When running the gecoApp, the user can change the network interface to be
 * monitored with the subcommand '-networkInterface'. Note that all other
 * subcommands from a gecoProcess (e.g. '-help') are available too.
 *
 * One can extend this code by defining a Tcl command to allow the user to
 * create a new gecoNetTraff object interactively. The function
 * geco_CreateGecoProcessCmd() can be used to help with this task. The following 
 * code shows how it can be implemented:
 * \snippet gecoProcessEx.cc Tcl interface
 * The function netTraffCmd is the C++ implementation of the Tcl command
 * available to the user to create interactively gecoNetTraff objects.
 * It must respect the type of a Tcl_ObjCmdProc according the C API of Tcl.
 * It starts by creating a gecoNetTraff object and then calls the function
 * geco_CreateGecoProcessCmd() which takes care of defining a Tcl command for the 
 * created gecoNetTraff object and adding it to the gecoApp in which the object lives.
 * Note that clientData of netTraffCmd() is a pointer to the gecoApp in which the 
 * object lives.
 *
 * To make available to the user the new Tcl command implemented by netTraffCmd
 * one has to use the Tcl_CreateObjCommand() function from the C API of Tcl. 
 * Adding the following line to the main function of our example will achieve this:
 * \code
 * Tcl_CreateObjCommand(app->getInterp(), "netTraff", netTraffCmd, (ClientData) app, (Tcl_CmdDeleteProc *) NULL);
 * \endcode
 * The user can now use the Tcl command 'netTraff' to create new gecoNetTraff obejcts
 * and add them to the geco process loop.
 *
 */


class gecoProcess : public virtual gecoObj
{


protected:

  timeval      activationTime;          /*!< time at which the gecoProcess got activated */
  double       t_o;                     /*!< time since the geco process loop runs at */
                                        /*!< activation of the gecoProcess */

  const char*  owner;                   /*!< gecoProcess owner */
  int          status;                  /*!< gecoProcess status */
  bool         verbose;                 /*!< gecoProcess verbose mode */
  bool         activateOnStart;         /*!< gecoProcess activate-on-start mode */

  Tcl_DString* preProcessScript;        /*!< gecoProcess Tcl preProcessScript */
  Tcl_DString* postProcessScript;       /*!< gecoProcess Tcl postProcessScript */

  gecoProcess* nextGecoProc;            /*!< next gecoProcess in the geco process loop*/

public:

  gecoProcess(const char* procName, const char* procOwner,
  	      const char* procCmd, gecoApp* App);
  ~gecoProcess();

  virtual int  cmd(int &i, int objc, Tcl_Obj *const objv[]);
  virtual void handleEvent(gecoEvent* ev);
  virtual Tcl_DString* info(const char* frontStr = "");

  virtual void terminate(gecoEvent* ev);
  virtual void activate(gecoEvent* ev);
  double       timeSinceActivated();
  double       to() {return t_o;}                                  /*!< Returns t_o */

  void         setPreProcessScript(Tcl_DString* PreProcessScript);
  void         setPostProcessScript(Tcl_DString* PostProcessScript);
  Tcl_DString* getPreProcessScript()  {return preProcessScript;}   /*!< Returns the preProcessScript */
  Tcl_DString* getPostProcessScript() {return postProcessScript;}  /*!< Returns the postProcessScript */

  const char*  getProcessOwner() {return owner;}                   /*!< Returns the gecoProcess owner */
  int          getStatus() {return status;}                        /*!< Returns the gecoProcess status */
  virtual void setStatus(int gecoProcessStatus) {status=gecoProcessStatus;} /*!< Sets the gecoProcess status */
  const char*  getStatusStr() {return StatusStr[status];}          /*!< Returns the gecoProcess status */

  void         setNextGecoProcess(gecoProcess* NextGecoProc) 
                       {nextGecoProc=NextGecoProc;}                /*!< Sets the next gecoProcess in the process loop*/
  gecoProcess* getNextGecoProcess() {return nextGecoProc;}         /*!< Returns the next gecoProcess from the process loop */
  bool         getVerbose() {return verbose;}                      /*!< Returns value of verbose */
};

#endif /* gecoProcess_SEEN_ */
