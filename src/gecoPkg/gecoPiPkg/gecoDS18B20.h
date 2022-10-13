// This may look like C code, but it is really -*- C++ -*-
// $Id: ECIOTcp.h 37 2015-01-09 12:35:29Z wuthrich $
// ---------------------------------------------------------------- 
//                                                                  
// Header file for the class gecoIODS18B20
//
// (c) Rolf Wuthrich
//     2020 Concordia University
//
// author:    Rolf Wuthrich
// email:     rolf.wuthrich@concordia.ca
// version:   v1 ($Revision: 37 $)
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

#ifndef gecoIODS18B20_SEEN_
#define gecoIODS18B20_SEEN_

#include <tcl.h>
#include "gecoSensor.h"

using namespace std;


// 1-Wire bus 
// Needs to be enabled with sudo raspi-config
#define gecoPiw1Bus   "/sys/bus/w1/devices/"
#define gecoPiw1Slave "/w1_slave"


// -----------------------------------------------------------------------
//
// Tcl interface
//

int gecoPiDS18B20Cmd(ClientData clientData, Tcl_Interp *interp, 
		     int objc,Tcl_Obj *const objv[]);



// -----------------------------------------------------------------------
//
// class gecoIODS18B20
//

class gecoDS18B20 : public gecoSensor
{

protected:

  Tcl_DString*   device;         // device file in /sys/bus/w1/devices/

public:

  gecoDS18B20(gecoApp* App);
  virtual ~gecoDS18B20();
  
  virtual int  cmd(int &i,int objc,Tcl_Obj *const objv[]);
  
  virtual Tcl_DString* info(const char* frontStr = "");

  virtual double readSensor();
};


#endif /* gecoIODS18B20_SEEN_ */
