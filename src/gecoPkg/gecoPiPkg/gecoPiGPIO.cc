// $Id: ECIOBoard.cc 37 2015-01-09 12:35:29Z wuthrich $
// ---------------------------------------------------------------
//
// Class gecoPiGPIO
//
// (c) Rolf Wuthrich
//     2016 - 2020 Concordia University
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

#include <tcl.h>
#include <fstream>
#include <iostream>
#include <string.h>
#include "gecoPiGPIO.h"
#include "geco.h"

using namespace std;


// ------------------------------------------------------------
//
// pi GPIO bus interface functions
//


// ---- GETDIRECTION : returns configuration of a GPIO pin
//

const char*  getDirection(int GPIO_pin)
{
  static char str[100];
  ifstream ifs;
  sprintf(str, "/sys/class/gpio/gpio%d/direction", GPIO_pin);
  ifs.open(str, std::ifstream::in);
  if (ifs.is_open()) ifs.getline(str,5); else strcpy(str,"NA");
  ifs.close();
  return str;
}


// ---- WRITE : writes a value to a GPIO pin
//

void writeGPIO(int GPIO_pin, double value)
{
  ofstream ofs;
  static char str[100];
  sprintf(str, "/sys/class/gpio/gpio%d/value", GPIO_pin);
  ofs.open(str, std::ofstream::out | std::ofstream::app);
  ofs << value;
  ofs.close();
}


// ---- READ : reads a value to a GPIO pin
//

int readGPIO(int GPIO_pin)
{
  ifstream ifs;
  static char str[100];
  sprintf(str,"/sys/class/gpio/gpio%d/value", GPIO_pin);
  ifs.open(str, std::ofstream::out | std::ofstream::app);
  int value;
  ifs >> value;
  ifs.close();
  return value;
}


// -----------------------------------------------------------------------


// -----------------------------------------------------------------------
//
// Class to store GPIO triggers 
//


// ---- CONSTRUCTOR
//

gecoPiGPIOTrigger::gecoPiGPIOTrigger(int gpioPin, int value, const char* Tcl_Script)
{
  TclScript = new Tcl_DString;
  Tcl_DStringInit(TclScript);
  Tcl_DStringAppend(TclScript, Tcl_Script, -1);
  pin=gpioPin;
  val=value;
  next=NULL;
}


// ---- DESTRUCTOR
//

gecoPiGPIOTrigger::~gecoPiGPIOTrigger()
{
  Tcl_DStringFree(TclScript);
  delete TclScript;
}


// ------------------------------------------------------------



// ---- GECO_PIGPIO_EVENTLOOP : used to monitor the GPIO bus
//
// A Tcl_TimerProc used to monitor the GPIO bus
//
// The Timer register itself again continuously
//

void geco_pigpio_eventLoop(ClientData clientData)
{
  gecoPiGPIObus* gpioBus = (gecoPiGPIObus *)clientData;
  gecoApp*       app = gpioBus->getApp();
    
  // loops over all triggers
  //int val;
  gecoPiGPIOTrigger* p = gpioBus->getFirstTrigger();
  gecoPiGPIOTrigger* q;
  while (p)
  {
    if (readGPIO(p->getPin())==p->getVal())
	{
	  // this complex way to delete trigger is needed 
	  // in order the trigger script can re-schedule 
	  // a trigger on same pin
	  q=p->getNext();
	  Tcl_DString* str = new Tcl_DString;
      Tcl_DStringInit(str);
	  Tcl_DStringAppend(str, Tcl_DStringValue(p->getTclScript()), -1);
	  gpioBus->removeTrigger(p);
	  Tcl_Eval(app->getInterp(), Tcl_DStringValue(str));
	  Tcl_DStringFree(str);
      delete str;
	  
	  Tcl_ResetResult(app->getInterp());
	  p=q;
	}
	else
      p=p->getNext();
  }

  //if (val) Tcl_SetVar(app->getInterp(), "pin18", "1", TCL_GLOBAL_ONLY);
  Tcl_CreateTimerHandler(10, geco_pigpio_eventLoop, clientData);
}


// ------------------------------------------------------------



// ------------------------------------------------------------
//
// Tcl interface
//

// Command to create a new gecoPiGPIO object
//

int gecoPiGPIOCmd(ClientData clientData, Tcl_Interp *interp, 
	          int objc,Tcl_Obj *const objv[])
{
  Tcl_ResetResult(interp);
  gecoPiGPIObus* gpioBus = (gecoPiGPIObus *)clientData;
  gecoApp*       app = gpioBus->getApp();
  gecoPiGPIO*    gpio;

  if (objc==1)
    {
      Tcl_WrongNumArgs(interp, 1, objv, "subcommand ?argument ...?");
      return TCL_ERROR;
    }

  int index;
  static CONST char* cmds[] = {"-help", "-open", "-export", "-unexport", 
                               "-setdirection", "-getdirection", 
							   "-write", "-read", 
							   "-trigger", "-removeTrigger", "-listTrigger", NULL};
							   
  static CONST char* help[] = {"opens an io-module to connect to the GPIO bus", 
                               "exports a GPIO pin",
							   "unexports a GPIO pin", 
							   "sets the direction of a GPIO pin",
							   "gets the direction of a GPIO pin", 
							   "writes to a GPIO pin",
							   "reads from a GPIO pin",
							   "sets a trigger on a GPIO pin",
							   "removes a trigger set on a GPIO pin",
							   "lists all triggers set GPIO pins",
							   NULL};
							
  if (Tcl_GetIndexFromObj(interp,objv[1], cmds, "subcommand", '0', &index)!=TCL_OK)
    return TCL_ERROR;
	
  int pin;
  double val;
  static char str[100];
  ofstream ofs;
  gecoPiGPIOTrigger* trigg;

  switch (index)
    {

    case 0: // -help
      if ((objc>3)||(objc<2))
	{
	  Tcl_WrongNumArgs(interp,2, objv, "?cmdName?");
	  return TCL_ERROR;
	}
      gecoHelp(interp, "piGPIO", "Raspberry pi GPIO bus interface", cmds, help);  
      break;

    case 1: // -open
      if ((objc>3)||(objc<2))
	{
	  Tcl_WrongNumArgs(interp, 2, objv, "?cmdName?");
	  return TCL_ERROR;
	}

	  if (objc==2) gpio = new gecoPiGPIO(app, "gpio");
	  if (objc==3) gpio = new gecoPiGPIO(app, Tcl_GetString(objv[2]));

      break;
	
	case 2: // -export
	
	  if (objc!=4)
      	{
      	  Tcl_WrongNumArgs(interp, 2, objv, "GPIO_pin direction");
      	  return TCL_ERROR;
      	}

      if (Tcl_GetIntFromObj(interp, objv[2], &pin)!=TCL_OK) return TCL_ERROR;
	  if ((Tcl_StringMatch(Tcl_GetString(objv[3]), "in")==0)&&
	    (Tcl_StringMatch(Tcl_GetString(objv[3]), "out")==0))
	  {
	    Tcl_AppendResult(interp, "invalid direction \"",
		  		 Tcl_GetString(objv[3]),
			   "\": must be \"in\" or \"out\"",NULL);
	    return TCL_ERROR;
	  }
	  
	  ofs.open ("/sys/class/gpio/export",std::ofstream::out | std::ofstream::app);
      ofs << pin;
      ofs.close();
  
      sprintf(str, "/sys/class/gpio/gpio%d/direction", pin);
      ofs.open (str,std::ofstream::out | std::ofstream::app);
      ofs << Tcl_GetString(objv[3]);
      ofs.close();
	
	  break;
	
	case 3: // -unexport
	
	  if (objc!=3)
      	{
      	  Tcl_WrongNumArgs(interp, 2, objv, "GPIO_pin");
      	  return TCL_ERROR;
      	}

      if (Tcl_GetIntFromObj(interp, objv[2], &pin)!=TCL_OK) return TCL_ERROR;
      
      ofs.open ("/sys/class/gpio/unexport",std::ofstream::out);
      ofs << pin;
      ofs.close();
	  
	  // removes trigger in case there was a trigger on the pin
	  trigg = gpioBus->findTrigger(pin);
	  if (trigg)
	    gpioBus->removeTrigger(trigg);
	
	  break;
	  
	case 4: // -setdirection
	
	  if (objc!=4)
      	{
      	  Tcl_WrongNumArgs(interp, 2, objv, "GPIO_pin direction");
      	  return TCL_ERROR;
      	}

      if (Tcl_GetIntFromObj(interp, objv[2], &pin)!=TCL_OK) return -1;
      if ((Tcl_StringMatch(Tcl_GetString(objv[3]), "in")==0)&&
	    (Tcl_StringMatch(Tcl_GetString(objv[3]), "out")==0))
	  {
	    Tcl_AppendResult(interp, "invalid direction \"",
		  		 Tcl_GetString(objv[3]),
			   "\": must be \"in\" or \"out\"",NULL);
	    return TCL_ERROR;
	  }
      
	  ofs.open ("/sys/class/gpio/export",std::ofstream::out | std::ofstream::app);
      ofs << pin;
      ofs.close();
  
      sprintf(str, "/sys/class/gpio/gpio%d/direction", pin);
      ofs.open (str,std::ofstream::out | std::ofstream::app);
      ofs << Tcl_GetString(objv[3]);
      ofs.close();
	  
	  // removes trigger in case there was a trigger on a pin that got changed to OUT
	  trigg = gpioBus->findTrigger(pin);
	  if (trigg && Tcl_StringMatch(Tcl_GetString(objv[3]), "out"))
	    gpioBus->removeTrigger(trigg);
	
	  break;
	  
	case 5: // -getdirection
	
	  if (objc!=3)
      	{
      	  Tcl_WrongNumArgs(interp,2, objv, "GPIO_pin");
      	  return TCL_ERROR;
      	}

      if (Tcl_GetIntFromObj(interp,objv[2], &pin)!=TCL_OK) return TCL_ERROR;
	  
	  Tcl_AppendResult(interp, getDirection(pin), NULL);
	
	  break;
	  
	case 6: // -write
	
	  if (objc!=4)
      	{
      	  Tcl_WrongNumArgs(interp, 2, objv, "GPIO_pin value");
      	  return -1;
      	}

      if (Tcl_GetIntFromObj(interp, objv[2], &pin)!=TCL_OK) return TCL_ERROR;
      if (Tcl_GetDoubleFromObj(interp, objv[3], &val)!=TCL_OK) return TCL_ERROR;
      if (strcmp(getDirection(pin), "out")!=0)
	{
	  Tcl_AppendResult(interp,
		 "Can not write on this pin. Set direction to \"out\" with the \"piGPIO -setdirection\" command",NULL);
	  return -1;
	}
      writeGPIO(pin, val);
	
	  break;
	  
	case 7: //-read
	
	  if (objc!=3)
      	{
      	  Tcl_WrongNumArgs(interp, 2, objv, "GPIO_pin");
      	  return -1;
      	}

      if (Tcl_GetIntFromObj(interp, objv[2], &pin)!=TCL_OK) return TCL_ERROR;
      if (strcmp(getDirection(pin), "in")!=0)
	{
	  Tcl_AppendResult(interp,
		 "Can not read on this pin. Set direction to \"in\" with the \"piGPIO -setdirection\" command", NULL);
	  return TCL_ERROR;
	}
      sprintf(str, "%i", readGPIO(pin));
      Tcl_AppendResult(interp, str, NULL);
	
	  break;
	  
	case 8: //-trigger
	
	  if (objc!=5)
      	{
      	  Tcl_WrongNumArgs(interp, 2, objv, "GPIO_pin value TclScript");
      	  return -1;
      	}
		
	  if (Tcl_GetIntFromObj(interp, objv[2], &pin)!=TCL_OK) return TCL_ERROR;
	  if (Tcl_GetDoubleFromObj(interp, objv[3], &val)!=TCL_OK) return TCL_ERROR;
      if (strcmp(getDirection(pin), "in")!=0)
	{
	  Tcl_AppendResult(interp,
		 "Can not read on this pin. Set direction to \"in\" with the \"piGPIO -setdirection\" command", NULL);
	  return TCL_ERROR;
	}
	  if (gpioBus->findTrigger(pin))
	{
	  Tcl_AppendResult(interp, "This pin has already a trigger defined on it", NULL);
	  return TCL_ERROR;
	}	  
	  
	  if (gpioBus->getFirstTrigger()==NULL) Tcl_CreateTimerHandler(1 , geco_pigpio_eventLoop, (ClientData) gpioBus);
	  gpioBus->addTrigger(pin, val, Tcl_GetString(objv[4]));
	
	  break;
	  
	case 9: //-removeTrigger
	
	  if (objc!=3)
      	{
      	  Tcl_WrongNumArgs(interp, 2, objv, "GPIO_pin");
      	  return -1;
      	}
		
	  if (Tcl_GetIntFromObj(interp, objv[2], &pin)!=TCL_OK) return TCL_ERROR;	
	  trigg = gpioBus->findTrigger(pin);
	  if (trigg==NULL)
	{
	  Tcl_AppendResult(interp,
		 "This pin has no trigger assigned", NULL);
	  return TCL_ERROR;
	}
	  
	  gpioBus->removeTrigger(trigg);
	
	  break;
	  
	case 10: //-listTrigger
	
	  if (objc!=2)
      	{
      	  Tcl_WrongNumArgs(interp, 2, objv, "");
      	  return -1;
      	}
		
	  gpioBus->listTriggers();
	
	  break;
	  
	}

  return TCL_OK;
}


// ------------------------------------------------------------
//
// class gecoPiGPIObus
//

// ---- CONSTRUCTOR
//

gecoPiGPIObus::gecoPiGPIObus(gecoApp* App)
{
  app=App;
  firstTrigger=NULL;
}


// ---- DESTRUCTOR
//

gecoPiGPIObus::~gecoPiGPIObus()
{
  // removes all triggers of the GPIO bus
  gecoPiGPIOTrigger* p;
  while (firstTrigger)
  {
    p=firstTrigger;
    firstTrigger=firstTrigger->getNext();
    delete p;
  }
}


// ---- ADDTRIGGER - Adds a trigger to the list
//

void gecoPiGPIObus::addTrigger(int pin, int value, const char* Tcl_Script)
{
  // creates the trigger
  gecoPiGPIOTrigger* trigg = new gecoPiGPIOTrigger(pin, value, Tcl_Script);
  
  // adds the trigger
  gecoPiGPIOTrigger* p=firstTrigger;
  if (p)
    {
      while (p->getNext())
	  p=p->getNext();    
      p->setNext(trigg);
    }
  else
    firstTrigger=trigg;
}


// ---- REMOVETRIGGER - removes and deletes a trigger from the list
//

void gecoPiGPIObus::removeTrigger(gecoPiGPIOTrigger* trigg)
{
  if (trigg==firstTrigger)
    {
      firstTrigger=trigg->getNext();
      delete trigg;
      return;
    }

  gecoPiGPIOTrigger* p=firstTrigger;
  while(p)
    {
      if (p->getNext()==trigg) break;
      p=p->getNext();
    }
  p->setNext(trigg->getNext());
  delete trigg;
}


// ---- LISTTRIGGERS - removes and deletes a trigger from the list
//

void gecoPiGPIObus::listTriggers()
{
  char str[100];

  Tcl_AppendResult(app->getInterp(), "GPIO PIN  VALUE  TCL SCRIPT\n",NULL);
  gecoPiGPIOTrigger* p=firstTrigger;
  while (p)
    {
      sprintf(str,"%-9i %-6i %s\n",
		  p->getPin(),
		  p->getVal(),
		  Tcl_DStringValue(p->getTclScript()));
      Tcl_AppendResult(app->getInterp(), str, NULL);
      p=p->getNext();
    }
}


// ---- FINDTRIGGER - finds a trigger in the list
//

gecoPiGPIOTrigger* gecoPiGPIObus::findTrigger(int gpioPin)
{
  gecoPiGPIOTrigger* p=firstTrigger;
  while(p)
    {
      if (p->getPin()==gpioPin) break;
      p=p->getNext();
    }
  return p;
}


// ------------------------------------------------------------



// ------------------------------------------------------------
//
// class gecoPiGPIO
//
// A geCo IO-module to connect to the GPIO bus
//

// ---- CONSTRUCTOR
//

gecoPiGPIO::gecoPiGPIO(gecoApp* App, const char* cmdName) :
  gecoIOModule("Raspberry Pi GPIO-module", cmdName, App)
{
  // add options
  addOption("-linkTclVariable", "links a Tcl variable");
}


// ---- DESTRUCTOR
//

gecoPiGPIO::~gecoPiGPIO()
{
}


// ---- CMD : process a potential command option (# i of objv[])
//            searches the options table 
//            if a match is found processes the command option and
//            returns the index from the options table
//            if no match returns -1

int gecoPiGPIO::cmd(int &i, int objc,Tcl_Obj *const objv[])
{
  // first executes the command options defined in gecoIOModule
  int index=gecoIOModule::cmd(i,objc,objv);

  int pin;

  if (index==getOptionIndex("-linkTclVariable"))
  {
    if (objc!=4)
      {
	Tcl_WrongNumArgs(interp, i+1, objv, "Tcl_Variable pin");
	return -1;
      }

    if (Tcl_GetIntFromObj(interp, objv[i+2], &pin)!=TCL_OK) return -1;

    // creates a new entry and links it
    IOModuleInsn* isn;
    isn = new IOModuleInsn(Tcl_GetString(objv[i+1]), 0, pin);
    if (addInsn(isn)==TCL_ERROR)
      {
	delete isn;
	return -1;
      }
    
    i=i+3;
  }
  
  return index;
}


// ---- INFO : returns a Tcl_DString containing relevant info about the GPIO
//             

Tcl_DString* gecoPiGPIO::info(const char* frontStr)
{

  gecoIOModule::info(frontStr);
  Tcl_DStringAppend(infoStr,"\n\n",-1);
  Tcl_DStringAppend(infoStr,"Pin\t Direction\n",-1);
  
  char str[100];
  
  for (int i=0;i<27;i++)
    if (strcmp(getDirection(i),"NA")!=0)
      {
	sprintf(str,"%d\t %s\n",i,getDirection(i));
	Tcl_DStringAppend(infoStr,str,-1);
      }

  return infoStr;
}


// ---- LISTINSTR : lists the instruction list
//

void gecoPiGPIO::listInstr()
{
  char str[100];
  Tcl_AppendResult(interp,"NUM  OPERATION  GPIO PIN   TCL VARIABLE\n",NULL);

  IOModuleInsn* p=getFirstInsn();
  int i=1;
  while (p)
    {
      sprintf(str,"%-4d %-5s      %-3d        %s\n",
	      i,
	      getDirection(p->getChanID()),
	      p->getChanID(),
	      Tcl_DStringValue(p->getTclVar()));
      Tcl_AppendResult(interp,str,NULL);
      p=p->getNext();
      i++;
    }
}


// ---- update : executes the IO instruction associated to the Tcl_Var
//
//      returns  1 if successful
//               0 if Tcl_Var is not linked
//

int gecoPiGPIO::update(const char* Tcl_Var)
{
  char str[100];
  IOModuleInsn* p=findLinkedTclVariable(Tcl_Var);
  if (p==NULL) return 0;
  if (strcmp("in", getDirection(p->getChanID()))==0) 
  {
	sprintf(str, "%d", readGPIO(p->getChanID()));
    Tcl_SetVar(interp, Tcl_Var, str, 0);
  }
    //p->setData(readGPIO(p->getChanID()));
  else
  {
	if (strcmp("0", Tcl_GetVar(interp, Tcl_Var, 0))==0)
	  writeGPIO(p->getChanID(), 0);
	else
	  writeGPIO(p->getChanID(), 1);
  }
    //writeGPIO(p->getChanID(), p->getData());
  return 1;
}


// ---- DOINSTR : executes the instruction list
//
//      returns number of successful operations done
//

int gecoPiGPIO::doInstr()
{
  int i=0;
  char str[100];
  IOModuleInsn* p=getFirstInsn();
  while (p)
    {
      if (strcmp("in",getDirection(p->getChanID()))==0)
	  {
	    sprintf(str, "%d", readGPIO(p->getChanID()));
        Tcl_SetVar(interp, Tcl_DStringValue(p->getTclVar()), str, 0);
      }
      else
	  {
	    if (strcmp("0", Tcl_GetVar(interp, Tcl_DStringValue(p->getTclVar()), 0))==0)
	      writeGPIO(p->getChanID(), 0);
	    else
	      writeGPIO(p->getChanID(), 1);
      }
      p=p->getNext();
      i++;
    }
  return i;
}
