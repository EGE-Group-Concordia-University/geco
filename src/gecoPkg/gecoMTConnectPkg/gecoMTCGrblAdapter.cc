// ---------------------------------------------------------------
//
// Definition of the class gecoMTCAdapter
//
// (c) Rolf Wuthrich
//     2022 Concordia University
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
// 03.05.2022 Creation                         R. Wuthrich
//
// ---------------------------------------------------------------

#include <tcl.h>
#include <cstring>
#include "gecoHelp.h"
#include "gecoApp.h"
#include "gecoMTCGrblAdapter.h"

using namespace std;

// -------------------------------------------------------------------------
//
// Tcl interface
//

// Command to create a new gecoMTCGrblAdapter object
//

int geco_MTCGrblAdapterCmd(ClientData clientData, Tcl_Interp *interp,
                           int objc, Tcl_Obj *const objv[])
{
  Tcl_ResetResult(interp);
  gecoApp *app = (gecoApp *)clientData;
  gecoMTCGrblAdapter *adap;

  if (objc == 1)
  {
    Tcl_WrongNumArgs(interp, 1, objv, "subcommand ?argument ...?");
    return TCL_ERROR;
  }

  int index;
  static CONST char *cmds[] = {"-help", "-open", NULL};
  static CONST char *help[] = {"opens a Grbl MTConnect adapter", NULL};

  if (Tcl_GetIndexFromObj(interp, objv[1], cmds, "subcommand", '0', &index) != TCL_OK)
    return TCL_ERROR;

  int port;

  switch (index)
  {

  case 0: // -help
    if (objc != 2)
    {
      Tcl_WrongNumArgs(interp, 2, objv, NULL);
      return TCL_ERROR;
    }
    gecoHelp(interp, "mtcgrbladapter", "Grbl MTConnect adapter", cmds, help);
    break;

  case 1: // -open
    if ((objc > 4) || (objc < 3))
    {
      Tcl_WrongNumArgs(interp, 2, objv, "port ?cmdName?");
      return TCL_ERROR;
    }

    if (Tcl_GetIntFromObj(interp, objv[2], &port) != TCL_OK)
      return TCL_ERROR;

    if (objc == 3)
      adap = new gecoMTCGrblAdapter(port, "mtcgrbladapter", (gecoApp *)clientData);

    if (objc == 4)
      adap = new gecoMTCGrblAdapter(port, Tcl_GetString(objv[3]), (gecoApp *)clientData, false);

    // adds the gecoProcess to the geco loop
    app->addGecoProcess(adap);
    Tcl_ResetResult(app->getInterp());
    Tcl_AppendResult(app->getInterp(), adap->getID(), NULL);
    break;
  }

  return TCL_OK;
}

// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
//
// Callback procedure called when Tcl_Channel to grbl controller is closed

void GrblChannelClosed(ClientData clientData)
{
  gecoMTCGrblAdapter *adapter = (gecoMTCGrblAdapter *)clientData;
  adapter->removeTclChannel();
}

// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
//
// class gecoMTCGrblAdapter: a class to stream grbl data to a MTConnect agent
//

/**
 * @brief Destructor
 */

gecoMTCGrblAdapter::~gecoMTCGrblAdapter()
{
  Tcl_DStringFree(gcodeFileName);
  delete gcodeFileName;
  Tcl_DStringFree(nextBlock);
  delete nextBlock;
  Tcl_DStringFree(lastErrorCode);
  delete lastErrorCode;
}

/*!
 * @copydoc gecoMTCBaseAdapter::cmd
 *
 * Compared to gecoMTCBaseAdapter::cmd, gecoMTCGrblAdapter::cmd adds the processing of
 * the new subcommands of gecoMTCGrblAdapter.
 */

int gecoMTCGrblAdapter::cmd(int &i, int objc, Tcl_Obj *const objv[])
{
  // executes the command options defined in gecoMTCBaseAdapter
  int index = gecoMTCAdapter::cmd(i, objc, objv);

  if (index == getOptionIndex("-grbl"))
  {
    if ((status == Active) && (objc == 3))
    {
      Tcl_AppendResult(interp, "Cannot change Tcl Socket while active\n", NULL);
      return -1;
    }

    if (objc > 3)
    {
      Tcl_WrongNumArgs(interp, i + 1, objv, "Tcl_Socket_To_Grbl_Controller");
      return -1;
    }

    if (objc == 2)
    {
      if (grblConnected)
        Tcl_AppendResult(interp, Tcl_GetChannelName(getTclChannel()), NULL);
      i++;
    }

    if (objc == 3)
    {
      int modePtr;
      Tcl_Channel chan = Tcl_GetChannel(interp, Tcl_GetString(objv[2]), &modePtr);

      if (!chan)
      {
        Tcl_AppendResult(interp, "\nTcl socket does not exist\n", NULL);
        return -1;
      }

      setTclChannel(chan);
      i = i + 2;
    }
  }

  if (index == getOptionIndex("-homing-cycle"))
  {
    sendGcode("$H");
    sendData("|mode|MANUAL_DATA_INPUT|block|$H");
    sendData("|msg|homing cycle|homing cycle started");
    sendData("|grbl|FAULT|Connection|-1|HIGH|grbl controller disconnected");
    sendData("|avail|UNAVAILABLE");

    // waits until homing-cycle is completed
    Tcl_DString *status = new Tcl_DString;
    ;
    Tcl_DStringInit(status);
    Tcl_Gets(grblChan, status);
    while (strcmp(Tcl_DStringValue(status), "") == 0)
    {
      Tcl_WriteChars(grblChan, "?", -1);
      Tcl_Flush(grblChan);
      Tcl_Gets(grblChan, status);
    }
    Tcl_DStringFree(status);
    delete status;
    sendData("|msg|homing cycle|homing cycle completed");
    sendData("|avail|AVAILABLE");
    sendData("|grbl|NORMAL|Connection|||grbl controller connected");

    i++;
  }

  if (index == getOptionIndex("-cycle-start"))
  {
    startCycle();
    i++;
  }

  if (index == getOptionIndex("-sendgcode"))
  {
    if (objc != 3)
    {
      Tcl_WrongNumArgs(interp, i + 1, objv, "gcode");
      return -1;
    }

    // sends message to agent
    sendData("|mode|MANUAL_DATA_INPUT|block|", false);
    sendData(Tcl_GetString(objv[2]));
    // sends g-code to grbl controller
    sendGcode(Tcl_GetString(objv[2]));
    i = i + 2;
  }

  if (index == getOptionIndex("-lastError"))
  {
    Tcl_AppendResult(interp, Tcl_DStringValue(lastErrorCode), NULL);
    i++;
  }

  return index;
}

/**
 * @copydoc gecoMTCAdapter::handleEvent
 *
 * In addition to gecoMTCAdapter::handleEvent, gecoMTCGrblAdapter::handleEvent implements
 * the reading of the grbl controller status and axis positions
 */

void gecoMTCGrblAdapter::handleEvent(gecoEvent *ev)
{
  gecoMTCAdapter::handleEvent(ev);

  if (status != Active)
    return;

  // Gets status of grbl controller and closes connection if controller does not answer
  if (!grblConnected)
  {
    removeTclChannel();
    return;
  }

  if (cycle)
  {
    if ((grblBf == grblBfsize - 1) || (grblBf == grblBfsize))
    {
      // message to agent
      sendData("|block|", false);
      sendData(Tcl_DStringValue(nextBlock), false);
      lineNbr++;
      sendData("|line|", false);
      sendData(to_string(lineNbr).c_str());

      // reads next block into grbl buffer
      string block;
      Tcl_DStringFree(nextBlock);
      if (getline(gcodeFile, block))
      {
        Tcl_DStringAppend(nextBlock, block.c_str(), -1);
        if (sendGcode(block.c_str()) != 0)
          gcodeFile.close();
      }
      else
      {
        sendData("|execution|PROGRAM_COMPLETED|");
        gcodeFile.close();
        cycle = false;
      }
    }
  }
}

/**
 * @copydoc gecoMTCBaseAdapter::info
 *
 * In addition to gecoMTCAdapter::info, gecoMTCGrblAdapter::info adds the information
 * about the data specific to gecoMTCGblrAdapter
 */

Tcl_DString *gecoMTCGrblAdapter::info(const char *frontStr)
{
  gecoMTCAdapter::info(frontStr);

  if (grblConnected)
    addInfo(frontStr, "GRBL Tcl Socket:         ", Tcl_GetChannelName(getTclChannel()));
  addInfo(frontStr, "G-Code file:             ", gcodeFileName);

  return infoStr;
}

/**
 * @copydoc gecoMTCAdapter::initialSHDR
 */

void gecoMTCGrblAdapter::initialSHDR()
{
  gecoMTCAdapter::initialSHDR();
  sendData("|firmeware|GRBL ", false);
  sendData(getGrblVersion().c_str());
}

/**
 * @brief Defines the SHDR data to be sent to agent in each loop of the gecoEventLoop
 * @param forceSend if true sends SDHR data even if values did not change since last sending
 * \return SHDR data to be sent to agent
 */

Tcl_DString *gecoMTCGrblAdapter::SHDR(bool forceSend)
{
  if (!grblChan)
    return gecoMTCAdapter::SHDR(forceSend);

  // handles possible alarm state
  handleGrblResponse();

  Tcl_DString *status = new Tcl_DString;
  ;
  Tcl_DStringInit(status);

  Tcl_WriteChars(grblChan, "?", -1);
  Tcl_Flush(grblChan);

  Tcl_Gets(grblChan, status);
  if (strcmp(Tcl_DStringValue(status), "") == 0)
  {
    if (n_fail == 3)
    {
      if (grblConnected == true)
      {
        // grbl controller got disconnected
        sendData("|grbl|FAULT|Connection|-1|HIGH|grbl controller disconnected");
        sendData("|avail|UNAVAILABLE");
      }
      grblConnected = false;
    }
    else
      n_fail++;
  }
  else
  {
    if (grblConnected == false)
    {
      // grbl controller got connected or reconnected
      sendData("|avail|AVAILABLE");
      sendData("|grbl|NORMAL|Connection|||grbl controller connected");
      forceSend = true;
    }
    grblConnected = true;
    n_fail = 0;
    parseGrblStatus(Tcl_DStringValue(status));
  }

  Tcl_DStringFree(status);
  delete status;

  return gecoMTCAdapter::SHDR(forceSend);
}

/**
 * @copydoc gecoMTCAdapter::addAgent
 */

void gecoMTCGrblAdapter::addAgent(Tcl_Channel chan, const char *hostName)
{
  gecoMTCAdapter::addAgent(chan, hostName);

  if ((grblConnected) && (status == Active))
  {
    sendData("|grbl|NORMAL|Connection|||grbl controller connected");
  }
}

/**
 * @copydoc gecoMTCAdapter::terminate
 *
 * In addition to gecoAdapter::terminate, gecoMTCGrblAdapter::terminate
 * closes the g-code file.
 */

void gecoMTCGrblAdapter::terminate(gecoEvent *ev)
{
  gecoMTCAdapter::terminate(ev);

  // closes g-code file
  if (gcodeFile.is_open())
  {
    gcodeFile.close();
    cycle = false;
  }

  // sends message to agent
  sendData("|grbl|FAULT|Connection|-1|HIGH|grbl controller disconnected");
}

/**
 * @copydoc gecoMTCAdapter::activate
 *
 * In addition to gecoMTCAdapter::activate, gecoMTCGrblAdapter::activate
 * opens for reading the g-code file.
 */

void gecoMTCGrblAdapter::activate(gecoEvent *ev)
{
  gecoMTCAdapter::activate(ev);
  if (grblChan)
  {
    sendData("|avail|AVAILABLE");
    sendData("|grbl|NORMAL|Connection|||grbl controller connected");
  }
  else
  {
    sendData("|grbl|FAULT|Connection|-1|HIGH|grbl controller disconnected");
    sendData("|avail|UNAVAILABLE");
  }
}

/**
 * @brief Handles response sent by Grbl
 * \return returns error code or 0 if no error/alarm
 *
 * If response is 'Ok' nothing is done
 * If response is '[MSG:some message]' sends the message to agent
 * If response is 'error:X' sends error code to agent
 * If response if 'ALARM:X' sends alarm code to agent
 */

int gecoMTCGrblAdapter::handleGrblResponse()
{
  int ret = 0;

  // creates Tcl command to read answer from controller
  Tcl_DString *Tcl_Cmd = new Tcl_DString;
  Tcl_DStringInit(Tcl_Cmd);
  Tcl_DStringAppend(Tcl_Cmd, "read ", -1);
  Tcl_DStringAppend(Tcl_Cmd, Tcl_GetChannelName(grblChan), -1);

  // reads answer from socket
  Tcl_ResetResult(interp);
  Tcl_Eval(interp, Tcl_DStringValue(Tcl_Cmd));
  Tcl_Flush(grblChan);
  Tcl_DStringFree(Tcl_Cmd);
  delete Tcl_Cmd;

  // this is a status response and should not be handeld here
  if (strstr(Tcl_GetStringResult(getTclInterp()), "<"))
    return 0;

  if (strstr(Tcl_GetStringResult(getTclInterp()), "[MSG:"))
  {
    string msg = Tcl_GetStringResult(getTclInterp());
    msg.erase(0, 5);
    msg.erase(msg.find(']'), msg.size());
    sendData("|msg|", false);
    sendData(msg.c_str(), false);
    sendData("|", false);
    sendData(msg.c_str());
    return 0;
  }

  if (strstr(Tcl_GetStringResult(getTclInterp()), "error"))
  {
    Tcl_DStringFree(lastErrorCode);
    Tcl_DStringAppend(lastErrorCode, "|motion|FAULT||||error:", -1);
    string err = Tcl_GetStringResult(getTclInterp());
    char err_code[3];
    err_code[0] = err[6];
    if (err[7] != '\n')
    {
      err_code[1] = err[7];
      err_code[2] = '\0';
    }
    else
    {
      err_code[1] = '\0';
    }
    Tcl_DStringAppend(lastErrorCode, err_code, -1);
    sendData(Tcl_DStringValue(lastErrorCode));
    ret = stoi(err_code);
    return ret;
  }

  if (strstr(Tcl_GetStringResult(getTclInterp()), "ALARM"))
  {
    Tcl_DStringFree(lastErrorCode);
    Tcl_DStringAppend(lastErrorCode, "|motion|FAULT||||", -1);
    string err = Tcl_GetStringResult(getTclInterp());
    char err_code[3];
    err_code[0] = err[6];
    if (err[7] != '\n')
    {
      err_code[1] = err[7];
      err_code[2] = '\0';
    }
    else
    {
      err_code[1] = '\0';
    }
    Tcl_DStringAppend(lastErrorCode, err_code, -1);
    sendData(Tcl_DStringValue(lastErrorCode));
    ret = stoi(err_code);
    return ret;
  }

  return ret;
}

/**
 * @brief Parse grbl status string to Tcl Variables
 * @param grblStatus grbl status string as returned by the ? command
 */

void gecoMTCGrblAdapter::parseGrblStatus(const char *grblStatus)
{
  // creates Tcl command to parse status data
  Tcl_DString *Tcl_Cmd = new Tcl_DString;
  Tcl_DStringInit(Tcl_Cmd);

  // removes < and >
  Tcl_DStringAppend(Tcl_Cmd, "set data [lindex [::split ", -1);
  Tcl_DStringAppend(Tcl_Cmd, grblStatus, -1);
  Tcl_DStringAppend(Tcl_Cmd, " <>] 1];", -1);
  // splits into fields according pipe sign
  Tcl_DStringAppend(Tcl_Cmd, "set fields [::split $data |];", -1);
  // gets machine status
  Tcl_DStringAppend(Tcl_Cmd, "set status [lindex $fields 0];", -1);
  // gets coordinates
  Tcl_DStringAppend(Tcl_Cmd, "set coords [lindex [::split [lindex $fields 1] :] 1];", -1);
  Tcl_DStringAppend(Tcl_Cmd, "set X [lindex [::split $coords ,] 0];", -1);
  Tcl_DStringAppend(Tcl_Cmd, "set Y [lindex [::split $coords ,] 1];", -1);
  Tcl_DStringAppend(Tcl_Cmd, "set Z [lindex [::split $coords ,] 2];", -1);
  // gets buffer state
  Tcl_DStringAppend(Tcl_Cmd, "set buff_state [lindex [::split [lindex $fields 2] :] 1];", -1);
  Tcl_DStringAppend(Tcl_Cmd, "set Bf [lindex [::split $buff_state ,] 0];", -1);
  // gets speeds
  Tcl_DStringAppend(Tcl_Cmd, "set speeds [lindex [::split [lindex $fields 3] :] 1];", -1);
  Tcl_DStringAppend(Tcl_Cmd, "set F [lindex [::split $speeds ,] 0];", -1);
  Tcl_DStringAppend(Tcl_Cmd, "set S [lindex [::split $speeds ,] 1];", -1);

  Tcl_ResetResult(interp);
  Tcl_Eval(interp, Tcl_DStringValue(Tcl_Cmd));
  Tcl_ResetResult(interp);

  if ((strcmp(Tcl_GetVar(interp, "status", 0), "Idle") == 0) && (lastGrblStatus != GRBL_IDLE))
  {
    lastGrblStatus = GRBL_IDLE;
    sendData("|execution|READY|");
    sendData("|motion|WARNING||||Idle");
  }

  if ((strcmp(Tcl_GetVar(interp, "status", 0), "Run") == 0) && (lastGrblStatus != GRBL_RUN))
  {
    lastGrblStatus = GRBL_RUN;
    sendData("|execution|ACTIVE|");
    sendData("|motion|NORMAL||||Run");
  }

  if ((strcmp(Tcl_GetVar(interp, "status", 0), "Alarm") == 0) && (lastGrblStatus != GRBL_ALARM))
  {
    lastGrblStatus = GRBL_ALARM;
    sendData("|execution|READY|");
    sendData("|motion|FAULT||||Alarm");
  }

  Tcl_DStringFree(Tcl_Cmd);
  delete Tcl_Cmd;
}

/**
 * @brief Starts a cycle
 */

void gecoMTCGrblAdapter::startCycle()
{
  cycle = true;
  lineNbr = 1;
  gcodeFile.open(Tcl_DStringValue(gcodeFileName));
  if (!gcodeFile.is_open())
  {
    Tcl_AppendResult(interp, "Could not open g-code program file.", NULL);
    cycle = false;
    return;
  }

  // message to agent
  Tcl_DString *str = new Tcl_DString;
  Tcl_DStringInit(str);
  Tcl_DStringAppend(str, "|mode|AUTOMATIC|program|", -1);
  Tcl_DStringAppend(str, getGCodeFileName(), -1);

  // sends first block
  string block;
  getline(gcodeFile, block);
  Tcl_DStringAppend(str, "|block|", -1);
  Tcl_DStringAppend(str, block.c_str(), -1);
  Tcl_DStringAppend(str, "|line|1", -1);
  Tcl_WriteChars(grblChan, block.c_str(), -1);
  Tcl_WriteChars(grblChan, "\n", -1);
  grblBf = grblBfsize - 1;

  // sends second block
  Tcl_DStringFree(nextBlock);
  if (getline(gcodeFile, block))
  {
    Tcl_WriteChars(grblChan, block.c_str(), -1);
    Tcl_WriteChars(grblChan, "\n", -1);
    Tcl_DStringAppend(nextBlock, block.c_str(), -1);
    grblBf = grblBfsize - 2;
  }
  else
  {
    Tcl_DStringAppend(str, "|execution|PROGRAM_COMPLETED\n", -1);
    gcodeFile.close();
    cycle = false;
  }

  Tcl_Flush(grblChan);
  sendData(Tcl_DStringValue(str));
  Tcl_DStringFree(str);
  delete str;

  // creates Tcl command to read answer from controller
  Tcl_DString *Tcl_Cmd = new Tcl_DString;
  Tcl_DStringInit(Tcl_Cmd);
  Tcl_DStringAppend(Tcl_Cmd, "read ", -1);
  Tcl_DStringAppend(Tcl_Cmd, Tcl_GetChannelName(grblChan), -1);

  // reads answer from socket
  Tcl_Eval(interp, Tcl_DStringValue(Tcl_Cmd));
  Tcl_DStringFree(Tcl_Cmd);
  delete Tcl_Cmd;
  Tcl_Flush(grblChan);
  Tcl_ResetResult(interp);
}

/**
 * @brief Sends g-code to the grbl controller
 * @param gcode g-code to send to the grbl controller
 * \return grbl error code or 0 if no error
 */

int gecoMTCGrblAdapter::sendGcode(const char *gcode)
{
  // send g-code to grbl controller
  Tcl_WriteChars(grblChan, gcode, -1);
  Tcl_WriteChars(grblChan, "\n", -1);
  Tcl_Flush(grblChan);

  // handles possible errors
  return handleGrblResponse();
}

/**
 * @brief Sets the Tcl_Channel to the grbl controller
 * @param chan Tcl_Channel of the grbl controller
 */

// TODO :  validate chan (is it indeed connected to a grbl controller?)

void gecoMTCGrblAdapter::setTclChannel(Tcl_Channel chan)
{
  grblChan = chan;
  grblConnected = true;
  Tcl_CreateCloseHandler(chan, GrblChannelClosed, (ClientData)this);
  // send firmware version to agent
  sendData("|firmeware|GRBL ", false);
  sendData(getGrblVersion().c_str());
  // sets correct status report mask of grbl
  // sendgcode("$10=3");
  // if active, sets avail
  if (status == Active)
    sendData("|avail|AVAILABLE");
}

/**
 * @brief Gets GRBL version
 * \return GRBL version of running firmeware
 */

string gecoMTCGrblAdapter::getGrblVersion()
{
  if (!grblChan)
    return "No Grbl controller connected";

  // send g-code to grbl controller
  Tcl_WriteChars(grblChan, "$I\n", -1);
  Tcl_Flush(grblChan);

  // creates Tcl command to read answer from controller
  Tcl_DString *Tcl_Cmd = new Tcl_DString;
  Tcl_DStringInit(Tcl_Cmd);
  Tcl_DStringAppend(Tcl_Cmd, "read ", -1);
  Tcl_DStringAppend(Tcl_Cmd, Tcl_GetChannelName(grblChan), -1);

  // reads answer from socket
  Tcl_ResetResult(interp);
  Tcl_Eval(interp, Tcl_DStringValue(Tcl_Cmd));
  Tcl_Flush(grblChan);
  Tcl_DStringFree(Tcl_Cmd);
  delete Tcl_Cmd;

  string res = Tcl_GetStringResult(getTclInterp());
  int j = 0;
  int i;
  for (i = 0; ((i < res.length()) && (j < 2)); i++)
    if (res[i] == '\n')
    {
      res[i] = ' ';
      j++;
    }
  res.erase(i - 1, std::string::npos);
  return res;
}
