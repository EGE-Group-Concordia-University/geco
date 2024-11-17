// This may look like C code, but it is really -*- C++ -*-
// $Id: ECIOTcp.h 37 2015-01-09 12:35:29Z wuthrich $
// ---------------------------------------------------------------- 
//                                                                  
// Header file for the class gecoIOTcp
//
// (c) Rolf Wuthrich
//     2015-2016 Concordia University
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
// 09.11.2015 Creation                         R. Wuthrich
// ---------------------------------------------------------------

#ifndef gecoIOTCP_SEEN_
#define gecoIOTCP_SEEN_

#include <tcl8.6/tcl.h>
#include "gecoIOSocket.h"

using namespace std;


// -----------------------------------------------------------------------
//
// Tcl interface
//

int geco_IOTcpCmd(ClientData clientData, Tcl_Interp *interp, 
		  int objc,Tcl_Obj *const objv[]);


// -----------------------------------------------------------------------
//
// class gecoIOTcp
//

class gecoIOTcp : public gecoIOSocket
{

protected:

  Tcl_DString*   host;         // host name or IP address
  int            port;         // port number where host is listening

public:

  gecoIOTcp(const char* moduleCmd, const char* Host, int portID, gecoApp* App);
  virtual ~gecoIOTcp();

  virtual int   cmd(int &i,int objc,Tcl_Obj *const objv[]);

  virtual Tcl_DString* info(const char* frontStr = "");
};


#endif /* gecoIOTCP_SEEN_ */
