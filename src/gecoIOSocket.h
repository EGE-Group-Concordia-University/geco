// This may look like C code, but it is really -*- C++ -*-
// $Id: ECIOTcp.h 37 2015-01-09 12:35:29Z wuthrich $
// ---------------------------------------------------------------- 
//                                                                  
// Header file for the class gecoIOTcp
//
// (c) Rolf Wuthrich
//     2021 Concordia University
//
// author:    Rolf Wuthrich
// email:     rolf.wuthrich@concordia.ca
// version:   v2 ($Revision: 37 $)
//
// This software is copyright under the BSD license
//
// ---------------------------------------------------------------
// history:
// ---------------------------------------------------------------
// Date       Modification                     Author
// ---------------------------------------------------------------
// 26.01.2021 Creation                         R. Wuthrich
// 29.05.2021 Major revision                   R. Wuthrich
// ---------------------------------------------------------------

#ifndef gecoIOSocket_SEEN_
#define gecoIOSocket_SEEN_

#include <tcl8.6/tcl.h>
#include "gecoIOModule.h"

using namespace std;


// -----------------------------------------------------------------------
//
// Tcl interface
//

int geco_IOSocketCmd(ClientData clientData, Tcl_Interp *interp, 
		  int objc,Tcl_Obj *const objv[]);


// -----------------------------------------------------------------------
//
// Class to store linked Tcl variables and instructions 
// to be sent to the socket
//

class SocketInsn : public IOModuleInsn
{

  friend class gecoIOSocket;

protected:

  Tcl_DString*   SocketInstr;      // Instruction to be sent to socket
  Tcl_DString*   TclSocketInstr;   // Tcl command to send instruction to the socket
  Tcl_DString*   PostProcScript;

public:

  SocketInsn(const char* Tcl_Var, const char* insn, Tcl_Channel chanID, int Type, bool handshake);
  ~SocketInsn();

  SocketInsn* getNext() {return static_cast<SocketInsn*>(next);}
};


// -----------------------------------------------------------------------
//
// class gecoIOSocket
//

class gecoIOSocket : public gecoIOModule
{

private:

  bool           handshake;    // true if socket will reply with a handshake
  int            transDelay;   // delay in milliseconds between transmissions  

protected:

  Tcl_Channel    chanID;       // Tcl socket

public:

  gecoIOSocket(const char* modName, const char* moduleCmd, gecoApp* App);
  gecoIOSocket(Tcl_Channel socket, const char* moduleCmd, gecoApp* App);
  virtual ~gecoIOSocket();

  virtual int   cmd(int &i,int objc,Tcl_Obj *const objv[]);

  SocketInsn*   getFirstInsn() {return static_cast<SocketInsn*>(firstIOModuleInsn);}
  virtual void  listInstr();
  virtual int   doInstr();
  virtual int   update(const char* Tcl_Var);
  
  void          query(const char* queryInsn);
  void          write(const char* cmdTowrite);

  SocketInsn*   findLinkedTclVariable(const char* TclVar) 
    {return static_cast<SocketInsn*>(gecoIOModule::findLinkedTclVariable(TclVar));}

  Tcl_Channel  getTclChannel() {return chanID;}

  virtual Tcl_DString* info(const char* frontStr = "");
};


#endif /* gecoIOSocket_SEEN_ */
