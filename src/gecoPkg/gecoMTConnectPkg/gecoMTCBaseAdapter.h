// This may look like C code, but it is really -*- C++ -*-
// ---------------------------------------------------------------- 
//                                                                  
// Header file for class gecoMTCBaseAdapter
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
// 27.04.2022 Creation                         R. Wuthrich
//
// ---------------------------------------------------------------
/*! \file */

#ifndef gecoMTCBaseAdapter_SEEN_
#define gecoMTCBaseAdapter_SEEN_

#include <tcl.h>
#include "gecoProcess.h"
#include "gecoApp.h"

using namespace std;


// -----------------------------------------------------------------------
//
// class gecoMTCBaseAdapter : an abstract class for an MTConnect adapter
//

/** 
 * @brief A gecoProcess to build MTConnect adapters
 * \author Rolf Wuthrich
 * \date 2022
 *
 * The gecoMTCBaseAdapter class allows to create MTConnect adapters.
 * This is an abstract class which is derived from the class gecoProcess.
 * It implements :
 *  * heartbeat between adapter and agent
 *  * SHDR data streaming to the agent
 *
 * The gecoMTCBaseAdapter class extends the subcommands from gecoObj and gecoProcess
 * by the following subcommands
 *
 * Sub-command       | Short description
 * ----------------- | ------------------
 * -dtSend           | returns/sets time interval between two SHDR data sending (s)
 * -send             | send data to the agent
 */

class gecoMTCBaseAdapter : public gecoProcess
{
private:

  double         saveTime;

protected:

  int            port;        /*!< port of adapter */
  Tcl_Channel    adapterID;   /*!< adapter TCP Tcl socket */
  double         dtSend;
  
  bool           connected;   /*!< true if connected to an agent */
  Tcl_Channel    agentID;     /*!< agent TCP Tcl socket */
  int            agentFileID; /*!< agent file identifier */
  Tcl_DString*   agentHost;   /*!< host name of connected agent */

  Tcl_DString*   SHDR_Str;    /*!< SHDR data in the gecoMTCBaseAdapter::SHDR method  */


public:

  gecoMTCBaseAdapter(const char* adaptName, const char* adaptCmd, int portID, gecoApp* App);
  ~gecoMTCBaseAdapter();

  virtual int  cmd(int &i,int objc,Tcl_Obj *const objv[]);
  virtual void handleEvent(gecoEvent* ev);
  virtual Tcl_DString* info(const char* frontStr = "");

  virtual void terminate(gecoEvent* ev);
  virtual void activate(gecoEvent* ev);

  virtual const char*  adapterVersion();
  virtual void         initialSHDR();
  virtual Tcl_DString* SHDR(bool forceSend = false);
  void sendSHDR(bool forceSend = false);

  void virtual addAgent(Tcl_Channel chan, const char* hostName);
  void removeAgent();

  void sendData(const char* SHDRdata, bool newLine = true);

  int              getPort() {return port;}
  bool             connectedToAgent() {return connected;}
  Tcl_DString*     getAgentHost() {return agentHost;}
};

#endif /* gecoMTCBaseAdapter_SEEN_ */
