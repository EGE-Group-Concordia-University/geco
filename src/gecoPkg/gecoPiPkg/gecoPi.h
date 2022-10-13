// This may look like C code, but it is really -*- C++ -*-
// $Id: ECIOBoard.h 37 2015-01-09 12:35:29Z wuthrich $
// ---------------------------------------------------------------- 
//                                                                  
// Header file for the gecoPi class
//
// (c) Rolf Wuthrich
//     2016-2020 Concordia University
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
// 07.02.2016 Creation                         R. Wuthrich
// 01/11/2020 General update                   R. Wuthrich
// ---------------------------------------------------------------

#ifndef GECOPI_SEEN_
#define GECOPI_SEEN_

#include <tcl.h>
#include "geco.h"

using namespace std;


// -----------------------------------------------------------------------
//
// Tcl interface
//

//int gecoPiGPIOCmd(ClientData clientData, Tcl_Interp *interp, 
//		  int objc,Tcl_Obj *const objv[]);


// -----------------------------------------------------------------------
//
// class gecoPi
//

class gecoPi : public gecoObj
{
protected:

public:

  gecoPi(gecoApp* App);
  ~gecoPi();

  virtual int          cmd(int &i,int objc,Tcl_Obj *const objv[]);
  virtual Tcl_DString* info(const char* frontStr = "");

  double  getTemperature();
};

#endif /* GECOPI_SEEN_ */
