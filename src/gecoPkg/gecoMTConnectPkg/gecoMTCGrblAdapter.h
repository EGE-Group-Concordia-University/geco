// This may look like C code, but it is really -*- C++ -*-
// ---------------------------------------------------------------- 
//                                                                  
// Header file for class gecoMTCGrblAdapter
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
/*! \file */

#ifndef gecoMTCGrblAdapter_SEEN_
#define gecoMTCGrblAdapter_SEEN_

#include <tcl.h>
#include <fstream>
#include "gecoProcess.h"
#include "gecoApp.h"
#include "gecoMTCAdapter.h"

using namespace std;


// -----------------------------------------------------------------------
//
// Tcl interface
//

/**
 * @brief C++ implementation of the Tcl command to create a gecoMTCGrblAdabter object
 * @param clientData pointer to the gecoApp in which the gecoMTCGrblAdabter instance will live
 * @param interp Tcl interpreter in which the Tcl command is executed
 * @param objc number of arguments of the Tcl command
 * @param objv arguments of the the Tcl command
 * \return TCL_OK if the execution of the Tcl command is successful and TCL_ERROR otherwise
 */

int geco_MTCGrblAdapterCmd(ClientData clientData, Tcl_Interp *interp, 
			   int objc,Tcl_Obj *const objv[]);


// -----------------------------------------------------------------------
//
// GRBL status types
//

const int
  GRBL_UNDEFINED = 0,
  GRBL_IDLE = 1,
  GRBL_RUN = 2,
  GRBL_ALARM = 3;
  


// ----------------------------------------------------------------------------
//
// class gecoMTCGrblAdapter : a class for streaming data to a MTConnect Agent
//

/** 
 * @brief A gecoProcess to stream data to an MTConnect agent
 * \author Rolf Wuthrich
 * \date 2022
 *
 * The gecoMTCGrblAdapter class allows to create a gecoProcess able to stream
 * data, from GRBL, to a MTConnet agent.
 *
 * The geco_MTCGrblAdapterCmd() is the C++ implementation for the Tcl command to
 * create gecoMTCGrblAdapter objects. This Tcl command is already available in 
 * the Tcl interpreter where the gecoMTConnectPkg is loaded under the name 'mtcgrbladapter'.
 *
 * Associated Tcl command
 * ----------------------
 * Every gecoMTCGrblAdapter is associated to a Tcl command. The associated Tcl command 
 * is created during the construction of an instance of gecoMTCGrblAdapter by its
 * parent class. The Tcl command is created in the Tcl interpreter
 * run by the gecoApp in which gecoMTCGrblAdapter lives.
 *
 * The gecoMTCGrblAdapter class extends the subcommands from gecoObj, gecoProcess
 * and gecoMTCBaseAdapter by the following subcommands
 *
 * Sub-command       | Short description
 * ----------------- | ------------------
 * -grbl             | returns/sets Tcl socket to grbl controller
 * -gcodefile        | returns/sets g-code file name to read from
 * -cycle            | manages execution of gcode program
 * -sendgcode        | sends a g-code to grbl controller
 */

class gecoMTCGrblAdapter : public gecoMTCAdapter
{

protected:

  Tcl_Channel   grblChan;         /*!< Tcl Socket to grbl controller */
  bool          grblConnected;    /*!< True if connected to a grbl controller */
  Tcl_DString*  gcodeFileName;    /*!< Full file name of g-code file to read */
  ifstream      gcodeFile;        /*!< ifstream to g-code file to read */
  long          lineNbr;          /*!< absolute line number wihtin gcode file under execution */
  bool          cycle;            /*!< True if cycle is running */
  Tcl_DString*  blocksBackLog;    /*!< blocks that where immediatly executed at upload to grbl */
  Tcl_DString*  nextBlock;        /*!< Next gcode block that will be exectuted */
  int           grblBfsize;       /*!< Grbl buffer stize */
  int           grblBf;           /*!< Grbl buffer status */
  Tcl_DString*  lastErrorCode;    /*!< Last error message of grbl */
  int           lastGrblStatus;   /*!< Last grbl status */
  int           n_fail;           /*!< Number of consecutive lost connections to Grbl */
  double        x_offset;         /*!< X-offset to apply to compute relative coordiante */
  double        y_offset;         /*!< Y-offset to apply to compute relative coordiante */
  double        z_offset;         /*!< Z-offset to apply to compute relative coordiante */


public:

  gecoMTCGrblAdapter(int portID, const char* adaptCmd, gecoApp* App, bool addIDtoCmd = true) :
    gecoObj("MTC GRBL Adapter", adaptCmd, App, addIDtoCmd),
    gecoMTCAdapter("MTC GRBL Adapter", adaptCmd, portID, App)
  {
    gcodeFileName = new Tcl_DString;
    Tcl_DStringInit(gcodeFileName);
    blocksBackLog = new Tcl_DString;
    Tcl_DStringInit(blocksBackLog);
    nextBlock = new Tcl_DString;
    Tcl_DStringInit(nextBlock);
    lastErrorCode = new Tcl_DString;
    Tcl_DStringInit(lastErrorCode);
    grblConnected = false;
    grblChan = NULL;
    cycle = false;
    lineNbr = 0;
    n_fail = 0;
    grblBfsize = 15;
    x_offset = 0.0;
    y_offset = 0.0;
    z_offset = 0.0;
    lastGrblStatus = GRBL_UNDEFINED;

    // links default variables
    addInsn(new SHDRInsn("status", "Status", App->getInterp()));
    addInsn(new SHDRInsn("X", "Xact", App->getInterp()));
    addInsn(new SHDRInsn("Y", "Yact", App->getInterp()));
    addInsn(new SHDRInsn("Z", "Zact", App->getInterp()));
    addInsn(new SHDRInsn("Xrel", "Xrel", App->getInterp()));
    addInsn(new SHDRInsn("Yrel", "Yrel", App->getInterp()));
    addInsn(new SHDRInsn("Zrel", "Zrel", App->getInterp()));
    addInsn(new SHDRInsn("S", "Sspeed", App->getInterp()));
    addInsn(new SHDRInsn("F", "Frt", App->getInterp()));
    Tcl_LinkVar(interp, "Bf", (char *)&grblBf, TCL_LINK_INT);
    
    addOption("-grbl", "returns/sets Tcl socket to grbl controller");
    addOption("-grblBufferSize", &grblBfsize, "returns/sets grbl buffer size");
    addOption("-homing-cycle", "runs a homing cycle");
    addOption("-sendgcode", "sends g-code to the grbl controller");
    addOption("-gcodefile", "returns/sets g-code program file name to read from");
    addOption("-cycle-start", "starts execution of gcode program");
    addOption("-lastError", "returns last error message from grbl controller");
  }

  ~gecoMTCGrblAdapter();

  virtual const char* adapterVersion() {return "gecoMTCGrblAdapter v1.0";}
  
  virtual int cmd(int &i,int objc,Tcl_Obj *const objv[]);
  virtual void handleEvent(gecoEvent* ev);
  virtual Tcl_DString* info(const char* frontStr = "");

  virtual void         initialSHDR();
  virtual Tcl_DString* SHDR(bool forceSend = false);

  void virtual addAgent(Tcl_Channel chan, const char* hostName);

  virtual void terminate(gecoEvent* ev);
  virtual void activate(gecoEvent* ev);

  int  handleGrblResponse();
  void parseGrblStatus(const char* grblStatus);
  void startCycle();
  void parseG92Command(const char *gcode);
  int  sendGcode(const char* gcode);
  void getG92Offsets();
  void updateState();
  int  loadNextBlock(int lineNbr);
  
  Tcl_Channel getTclChannel()  {return grblChan;}
  void        setTclChannel(Tcl_Channel chan);
  void        removeTclChannel() {grblConnected = false;}
  const char* getGCodeFileName() {return Tcl_DStringValue(gcodeFileName);}

  string getGrblVersion();

};

#endif /* gecoMTCGrblAdapter_SEEN_ */
