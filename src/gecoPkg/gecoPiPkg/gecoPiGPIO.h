// This may look like C code, but it is really -*- C++ -*-
// $Id: ECIOBoard.h 37 2015-01-09 12:35:29Z wuthrich $
// ---------------------------------------------------------------- 
//                                                                  
// Header file for the gecoPiGPIO class
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

#ifndef GECOPIGPIO_SEEN_
#define GECOPIGPIO_SEEN_

#include <tcl.h>
#include "geco.h"

using namespace std;


// -----------------------------------------------------------------------
//
// General information
//
// Access to the pi GPIO bus is programmed via the sysfs interface
//
// The user must be member of the gpio group
//


// -----------------------------------------------------------------------
//
// Documentation on the pi GPIO bus
/*

https://www.raspberrypi.org/documentation/usage/gpio/
https://www.raspberrypi.org/documentation/hardware/raspberrypi/gpio/README.md

-------------------------------------------------------------------------*/


// -----------------------------------------------------------------------
//
// Tcl interface
//

int gecoPiGPIOCmd(ClientData clientData, Tcl_Interp *interp, 
		  int objc,Tcl_Obj *const objv[]);


// -----------------------------------------------------------------------
//
// GPIO bus characteristics
// PIN numbers are the number of the GPIO pin (e.g. 4 if GPIO4)
//

#define MAX_NBR_GPIO_CHAN 26



// -----------------------------------------------------------------------
//
// Class to store GPIO triggers 
//

class gecoPiGPIOTrigger
{

protected:

  int                 pin;
  int                 val;
  Tcl_DString*        TclScript;
  gecoPiGPIOTrigger*  next;

public:

  gecoPiGPIOTrigger(int gpioPin, int value, const char* Tcl_Script);
  virtual ~gecoPiGPIOTrigger();

  void               setNext(gecoPiGPIOTrigger* nextTrigger) {next=nextTrigger;}
  gecoPiGPIOTrigger* getNext()      {return next;}
  int                getPin()       {return pin;}
  int                getVal()       {return val;}
  Tcl_DString*       getTclScript() {return TclScript;}
};

// -----------------------------------------------------------------------



// -----------------------------------------------------------------------
//
// class gecoPiGPIObus - class to manipulate pi GPIO bus
//

class gecoPiGPIObus
{
protected:
  gecoPiGPIOTrigger* firstTrigger;
  gecoApp*           app;

public:

  gecoPiGPIObus(gecoApp* App);
  ~gecoPiGPIObus();
  
  void addTrigger(int pin, int value, const char* Tcl_Script);
  void removeTrigger(gecoPiGPIOTrigger* trigg);
  void listTriggers();
  gecoPiGPIOTrigger* findTrigger(int gpioPin);
  
  gecoPiGPIOTrigger* getFirstTrigger() {return firstTrigger;}
  gecoApp*           getApp() {return app;}
};


// -----------------------------------------------------------------------


// -----------------------------------------------------------------------
//
// class gecoPiGPIO - geco IO module for the GPIO bus
//

class gecoPiGPIO : public gecoIOModule
{
protected:

public:

  gecoPiGPIO(gecoApp* App, const char* cmdName);
  ~gecoPiGPIO();

  virtual int          cmd(int &i,int objc,Tcl_Obj *const objv[]);
  virtual Tcl_DString* info(const char* frontStr = "");

  virtual void listInstr();
  virtual int  doInstr();
  virtual int  update(const char* Tcl_Var);

};


// -----------------------------------------------------------------------


#endif /* GECOPIGPIO_SEEN_ */
