// ---------------------------------------------------------------
//
// Definition of the class gecoMTCBaseAdapter
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

#include <tcl.h>
#include <cstring>
#include "gecoMTCBaseAdapter.h"
#include "gecoApp.h"

using namespace std;


// -------------------------------------------------------------------------
//
// Callback procedures
//

// Needed internaly for callback procedures

struct AgentConnection
{
  gecoMTCBaseAdapter* adapter;
  Tcl_Channel         agentChan;
  gecoApp*            app;
};


// Callback procedure called when Tcl_Channel to agent is closed
//

void AgentTclChannelClosed(ClientData clientData)
{
  gecoMTCBaseAdapter* adapter = (gecoMTCBaseAdapter *)clientData;
  if (adapter->getVerbose())
    cout << "Connection to agent ("
	 << Tcl_DStringValue(adapter->getAgentHost())<< ") closed\n";
  adapter->removeAgent();
}


// Callback procedure called when adapter gets data from agent

void AgentTrsm(ClientData clientData, int mask)
{
  AgentConnection*    conn = (AgentConnection *)clientData;
  gecoApp*            app  = conn->app;
  Tcl_Channel         chan = conn->agentChan;
  gecoMTCBaseAdapter* adap = conn->adapter;
  
  // checks if connection was closed by agent
  if (Tcl_Eof(chan)) {
    Tcl_UnregisterChannel(adap->getTclInterp(), chan);  // will trigger AgentTclChannelClosed
    return;
  }
  
  Tcl_DString* str  = new Tcl_DString;
  Tcl_DStringInit(str);
  Tcl_Gets(chan, str);

  if (adap->getVerbose())
      cout << "Agent (" << Tcl_DStringValue(adap->getAgentHost())
	   << ") sent " << Tcl_DStringValue(str) << "\n";

  if (strcmp(Tcl_DStringValue(str), "* PING")==0) {
    Tcl_WriteChars(chan, "* PONG 60000\n", -1);
    Tcl_Flush(chan);
  }
  
  Tcl_DStringFree(str);
  delete str;
  
  // reschedule file handler 
  int chanID;
  Tcl_GetChannelHandle(chan, TCL_READABLE, (ClientData*)&chanID);
  Tcl_CreateFileHandler(chanID, TCL_READABLE, AgentTrsm, clientData);
}


// Callback procedure called when agent connects to adapter

void AgentConnect(ClientData clientData, Tcl_Channel channel, char *hostName, int port)
{  
  gecoMTCBaseAdapter* adapter = (gecoMTCBaseAdapter *)clientData;
  Tcl_RegisterChannel(adapter->getTclInterp(), channel);
  
  if (adapter->getVerbose())
    cout << "Connection from " << hostName << " on port " << port
	 << " with Tcl socket : " << Tcl_GetChannelName(channel) << "\n";
  
  if (adapter->connectedToAgent()) {
    Tcl_WriteChars(channel, "Adapter is already connected to an agent - Connection refused\n", -1);
    Tcl_UnregisterChannel(adapter->getTclInterp(), channel);
    return;
  }
  
  adapter->addAgent(channel, hostName);
	  
  // Ensure that each "puts" by the adapter results in a network transmission
  Tcl_SetChannelOption(adapter->getTclInterp(), channel, "-buffering", "line");

  // Force sending of SDHR data to agent (required by MTConnect standard)
  adapter->sendData("* adapterVersion: ", false);
  adapter->sendData(adapter->adapterVersion());
  adapter->sendData("* mtconnectVersion: 1.7");
  adapter->sendData("* shdrVersion: 1");
  adapter->initialSHDR();
  adapter->sendSHDR(true);
  
  // Gets Tcl channel ID of agent
  int chanID;
  Tcl_GetChannelHandle(channel, TCL_READABLE, (ClientData*)&chanID);
  
  AgentConnection* connection = new AgentConnection();
  connection->adapter = adapter;
  connection->agentChan = channel;
  connection->app = adapter->getGecoApp();

  // Set up a callback for when agent disconnects
  Tcl_CreateCloseHandler(channel, AgentTclChannelClosed, (ClientData)adapter);
  
  // Set up a callback for when agent sends data
  Tcl_CreateFileHandler(chanID, TCL_READABLE, AgentTrsm, (ClientData)connection);
}


// Callback procedure called when Tcl_Channel of the adapter is closed

void MTCAdapterChannelClosed(ClientData clientData)
{
  gecoMTCBaseAdapter* adapter = (gecoMTCBaseAdapter *)clientData;
  delete adapter;
}


// -------------------------------------------------------------------------
//
// class gecoMTCBaseAdapter : an abstract class for an MTConnect adapter
//

/**
 * @brief Constructor
 * @param adaptName name of the gecoMTCBaseAdapter
 * @param adaptCmd Tcl command of the gecoMTCBaseAdapter
 * @param portID port to which the gecoMTCBaseAdapter is listening
 *
 * Creates an MTConnect adapter on port portID.
*/

gecoMTCBaseAdapter::gecoMTCBaseAdapter(const char* adaptName, const char* adaptCmd,
				       int portID, gecoApp* App) :
  gecoObj(adaptName, adaptCmd, App),
  gecoProcess(adaptName, "user", adaptCmd, App),
  saveTime(0.0),
  dtSend(0.1),
  connected(false)
{
  activateOnStart = 1;
  port = portID;
  SHDR_Str = new Tcl_DString;
  Tcl_DStringInit(SHDR_Str);
  
  adapterID = Tcl_OpenTcpServer(App->getInterp(), portID, NULL, AgentConnect, (ClientData)this);
  if (!adapterID) return;
  Tcl_RegisterChannel(App->getInterp(), adapterID);
  Tcl_CreateCloseHandler(adapterID, MTCAdapterChannelClosed, (ClientData)this);

  addOption("-dtSend", &dtSend, "returns/sets time interval between two SHDR data sending (s)");
  addOption("-send", "send data to the agent");
}


/**
 * @brief Destructor
*/

gecoMTCBaseAdapter::~gecoMTCBaseAdapter()
{
  char str[TCL_DOUBLE_SPACE];
  sprintf(str, "%i", port);
  cout << "Shutting down MTConnect adapter on port " << str << "\n";

  Tcl_DStringFree(SHDR_Str);
  delete SHDR_Str;
  if (connectedToAgent()) removeAgent();
  cout << "  Removed connection to Agent\n";

  Tcl_DeleteCloseHandler(adapterID, MTCAdapterChannelClosed, (ClientData)this);
  Tcl_UnregisterChannel(getTclInterp(), adapterID);   // This will as well close the Tcl_Channel
  cout << "  Closed Adapter\n  Success\n";
}


/*!
 * @copydoc gecoProcess::cmd 
 *
 * Compared to gecoProcess::cmd, gecoMTCBaseAdapter::cmd adds the processing of
 * the new subcommands of gecoMTCBaseAdapter.
 */

int gecoMTCBaseAdapter::cmd(int &i,int objc,Tcl_Obj *const objv[])
{
  // executes the command options defined in gecoProcess
  int index=gecoProcess::cmd(i,objc,objv);

  if (index==getOptionIndex("-send"))
    {
      if (i+1>=objc)
      	{
      	  Tcl_WrongNumArgs(interp, i+1, objv, "SHDR_data");
      	  return -1;
      	}
      if (!connectedToAgent())
	{
	  Tcl_AppendResult(interp, "No agent connected. Nothing was sent.", NULL);
	  return -1;
	}
      sendData(Tcl_GetString(objv[i+1]));
      i = i+2;
    }

  return index;
}


/**
 * @copydoc gecoProcess::handleEvent
 *
 * In addition to gecoProcess::handleEvent, gecoMTCBaseAdapter::handleEvent implements
 * the streaming of the SHDR data to the agent.
 */

void gecoMTCBaseAdapter::handleEvent(gecoEvent* ev)
{
  gecoProcess::handleEvent(ev);

  // If Active and connected to an agent sends SHDR data
  if ((status==Active) && ((ev->getT()-saveTime)>=dtSend))
    {
      saveTime=ev->getT();
      sendSHDR();
    }
}


/**
 * @copydoc gecoProcess::info
 *
 * In addition to gecoProcess::info, gecoMTCBaseAdapter::info adds the information
 * about the adapter and agent.
 */

Tcl_DString* gecoMTCBaseAdapter::info(const char* frontStr)
{
  gecoProcess::info(frontStr);
  addInfo(frontStr, "Port:                    ", port);   
  addInfo(frontStr, "SHDR send interval (s):  ", dtSend);
  if (connectedToAgent())
    addInfo(frontStr, "Connected agent:         ", getAgentHost());
  return infoStr;
}


/**
 * @copydoc gecoProcess::terminate
 */

void gecoMTCBaseAdapter::terminate(gecoEvent* ev)
{
  gecoProcess::terminate(ev);
  sendData("|avail|UNAVAILABLE");
}


/**
 * @copydoc gecoProcess::activate
 *
 * In addition to gecoProcess::activate, gecoMTCBaseAdapter::activate 
 * sends result of SHDR(true) to a connected agent
 */

void gecoMTCBaseAdapter::activate(gecoEvent* ev)
{
  gecoProcess::activate(ev);
  
  saveTime=ev->getT();
  sendData("|avail|AVAILABLE");
  sendSHDR(true);
}



/**
 * @brief Version of the adapter
 * \return Version number of the adapter to be sent to agent at connection
 *
 * This method can be overiden by the children of gecoMTCBaseAdapter.
 */

const char* gecoMTCBaseAdapter::adapterVersion()
{
  return "gecoMTCBaseAdapter v1.0";
}


/**
 * @brief Inital data to be sent to agent when agent connects first time
 *
 * Sends user to agent as |operator|$::geco::user
 *
 * This method can be overiden by the children of gecoMTCBaseAdapter.
 * Typical example:
 * sendData("some data");
 */

void gecoMTCBaseAdapter::initialSHDR()
{
  sendData("|operator|", false);
  sendData(Tcl_GetVar(interp, "::geco::user", 0));
}


/**
 * @brief Defines the SHDR data to be sent to agent in each loop of the gecoEventLoop
 * @param forceSend if true returns SHDR data even if values did not change since last sending
 *                  if false returns SHDR data only if values changed since last sending
 * \return SHDR data to be sent to agent
 *
 * This method has to be implemented by the children of gecoMTCBaseAdapter.
 */

Tcl_DString* gecoMTCBaseAdapter::SHDR(bool forceSend)
{
  // Template for implementation in children

  Tcl_DStringFree(SHDR_Str);
  // Compute SHDR data to be sent
  return SHDR_Str;
}



/**
 * @brief Sends the return value of SHDR() to the connected agent
 * @param forceSend if true sends SDHR data even if values did not change since last sending
 */

void gecoMTCBaseAdapter::sendSHDR(bool forceSend)
{
  Tcl_DString* shdr;
  shdr = SHDR(forceSend);
  if ((connectedToAgent() == false) || (status!=Active)) return;
  if (strcmp(Tcl_DStringValue(shdr), "")!=0)
    {
      Tcl_WriteChars(agentID, Tcl_DStringValue(shdr), -1);
      Tcl_WriteChars(agentID, "\n", -1);
      Tcl_Flush(agentID);
    }
}


/**
 * @brief Adds info of connected agent
 * @param chan Tcl_Channel to the connected agent
 * @param hostName host name on which runs the connected agent
 */

void gecoMTCBaseAdapter::addAgent(Tcl_Channel chan, const char* hostName)
{
  Tcl_GetChannelHandle(chan, TCL_READABLE, (ClientData*)&agentFileID);
  agentID = chan;
  agentHost = new Tcl_DString;
  Tcl_DStringInit(agentHost);
  Tcl_DStringAppend(agentHost, hostName, -1);
  connected = true;
  if (status==Active) sendData("|avail|AVAILABLE");
}


/**
 * @brief Removes info of connected agent
 */

void gecoMTCBaseAdapter::removeAgent()
{
  // removes callback procedures
  Tcl_DeleteCloseHandler(agentID, AgentTclChannelClosed, (ClientData)this);
  Tcl_DeleteFileHandler(agentFileID);
  
  // frees Tcl_DStrings
  Tcl_DStringFree(agentHost);
  delete agentHost;
  connected = false;
}


/**
 * @brief Sends data to the connected agent
 * @param SHDRdata data to send to agent
 * @param newLine if true adds '\n' to SHDRdata
 */

void gecoMTCBaseAdapter::sendData(const char* SHDRdata, bool newLine)
{
  if (connectedToAgent() == false) return;
  Tcl_WriteChars(agentID, SHDRdata, -1);
  if (newLine) Tcl_WriteChars(agentID, "\n", -1);
  Tcl_Flush(agentID);
}
