#include "geco.h"


// -----------------------------------------------------------------------
//
// class gecoSquare : a demo class for a gecoGenerator child 
//

//! [Class definition]
class gecoSquare : public gecoGenerator
{

protected: 

double duration;  // signal duration in sec

public:

  gecoSquare(gecoApp* App);
  
  virtual void handleEvent(gecoEvent* ev);
  virtual double signalFunction(double t);
};
//! [Class definition]


//! [Constructor]
// ---- CONSTRUCTOR
//

gecoSquare::gecoSquare(gecoApp* App) :
    gecoObj("Square signal", "square", App),
    gecoGenerator("Square signal", "square", App)
{
  duration = 0.0;
  addOption("-duration", &duration, "returns/sets signal duration (s)");
}
//! [Constructor]


//! [handleEvent]
// ---- HANDLEEVENT : defines how gecoSquare reacts to external events
//

void gecoSquare::handleEvent(gecoEvent* ev)
{
  gecoGenerator::handleEvent(ev);

  if (status!=Active) return;

  if (timeSinceActivated()>=duration) terminate(ev);
}
//! [handleEvent]



//! [function]
// ---- SIGNALFUNCTION : computes the square signal
//

double gecoSquare::signalFunction(double t)
{
  return t*t;
}
//! [function]



// -------------------------------------------------------------------------
//
// Tcl interface
//

// Command to create a new gecoSquare object
//

//! [Tcl interface]
int squareCmd(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[])
{
  gecoSquare* proc = new gecoSquare((gecoApp *)clientData);
  return geco_CreateGecoProcessCmd(proc, objc, objv);
}
//! [Tcl interface]

// -----------------------------------------------------------------------



// ---- MAIN : main function of C++
//

//! [main]
int main(int argc, char **argv)
{
  // creates a new gecoApp
  gecoApp* app = new gecoApp(argc, argv);
  
  // adds the new Tcl command to app
  Tcl_CreateObjCommand(app->getInterp(), "square", squareCmd, 
                       (ClientData) app, (Tcl_CmdDeleteProc *) NULL);
  
  // runs app in the CLI mode
  app->runCLI();
  
  // cleans up everything
  delete app;
  
  return 0;
}
//! [main]