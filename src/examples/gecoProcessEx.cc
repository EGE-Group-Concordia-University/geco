#include "geco.h"


// -----------------------------------------------------------------------
//
// class gecoNetTraff : a demo class for a gecoProcess 
//
// Monitors network traffic based on entries of sysfs-class-net-statistics
// Official documentation: https://www.kernel.org/doc/Documentation/ABI/testing/sysfs-class-net-statistics
//

//! [Class definition]
class gecoNetTraff : public gecoProcess
{

protected:

  Tcl_DString* netDevice;  //  network interface
  
  ifstream devRX;
  ifstream devTX;
  
  long RXbytes;
  long TXbytes; 

  double prevT;  

public:

  gecoNetTraff(gecoApp* App);
  ~gecoNetTraff();

  virtual void handleEvent(gecoEvent* ev);
  virtual void activate(gecoEvent* ev);
  virtual void terminate(gecoEvent* ev);
  
  void setDevice(char* NetDevice) 
  {
    Tcl_DStringInit(netDevice); 
	Tcl_DStringAppend(netDevice, NetDevice, -1);
  }
  Tcl_DString* getDevice() {return netDevice;}
  
  
  long   RXtraffic();
  long   TXtraffic();
};
//! [Class definition]


//! [Constructor]
// ---- CONSTRUCTOR
//

gecoNetTraff::gecoNetTraff(gecoApp* App) :
    gecoObj("Network traffic", "netTraff", App),
    gecoProcess("Network traffic", "user","netTraff", App)
{
  netDevice  = new Tcl_DString;
  Tcl_DStringInit(netDevice);
  
  addOption("-networkInterface", netDevice, "returns/sets the network interface to monitor");
}
//! [Constructor]


//! [Destructor]
// ---- DESTRUCTOR
//

gecoNetTraff::~gecoNetTraff()
{
  Tcl_DStringFree(netDevice);
  delete netDevice;	 	
}
//! [Destructor]


//! [handleEvent]
// ---- HANDLEEVENT : define how gecoNetTraff reacts to external events
//

void gecoNetTraff::handleEvent(gecoEvent* ev)
{
  // first process the ev with gecoProcess
  gecoProcess::handleEvent(ev);
  
  // if the process is not active, nothing to do
  if (status!=Active) return;
  
  // specific tasks for gecoNetTraff 
  long RXnow = RXtraffic();
  long TXnow = TXtraffic(); 
  char str[80];
  
  // computes the traffic rates in kB/sec
  sprintf(str, "%f", (RXnow-RXbytes)/(1000.0*(ev->getT()-prevT)));
  Tcl_SetVar(interp, "RXrate", str, 0);
  sprintf(str, "%f", (TXnow-TXbytes)/(1000.0*(ev->getT()-prevT)));
  Tcl_SetVar(interp, "TXrate", str, 0);
  
  prevT = ev->getT();
  RXbytes = RXnow;
  TXbytes = TXnow;
}
//! [handleEvent]


//! [activate]
// ---- ACTIVATE : called when gecoNetTraff is activated:
//

void gecoNetTraff::activate(gecoEvent* ev)
{
  // first process the ev with gecoProcess
  gecoProcess::activate(ev);
  
  // open the files
  char str[80];
  sprintf(str, "/sys/class/net/%s/statistics/rx_bytes", Tcl_DStringValue(netDevice));;
  devRX.open(str, ios::in);
  sprintf(str, "/sys/class/net/%s/statistics/tx_bytes", Tcl_DStringValue(netDevice));;
  devTX.open(str, ios::in);
  
  // saves current network traffic
  RXbytes = RXtraffic();
  TXbytes = TXtraffic();
  
  // saves time of activation
  prevT = ev->getT();
}
//! [activate]


//! [terminate]
// ---- TERMINATE : called when gecoNetTraff is terminated:
//

void gecoNetTraff::terminate(gecoEvent* ev)
{
  // first process the ev with gecoProcess
  gecoProcess::terminate(ev);
  
  // closes the files
  devRX.close();
  devTX.close();
}
//! [terminate]



//! [auxiliary functions]
// ---- RXTRAFFIC : reads total bytes received so far
//

long gecoNetTraff::RXtraffic()
{
	long traff;
	devRX.seekg(ios_base::beg);
	devRX >> traff;
	return traff;
}


// ---- TXTRAFFIC : reads total bytes transmitted so far
//

long gecoNetTraff::TXtraffic()
{
	long traff;
	devTX.seekg(ios_base::beg);
	devTX >> traff;
	return traff;
}
//! [auxiliary functions]



// -------------------------------------------------------------------------
//
// Tcl interface
//

// Command to create a new gecoNetTraff object
//

//! [Tcl interface]
int netTraffCmd(ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const objv[])
{
  gecoNetTraff* proc = new gecoNetTraff((gecoApp *)clientData);
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
  
  // creates a new gecoNetTraff and adds it to the geco-process loop
  gecoNetTraff* newProc = new gecoNetTraff(app);
  newProc->setDevice("eno1");
  app->addGecoProcess(newProc);
  
  // creates a gecoEnd process and adds it to the geco-process loop
  gecoEnd* endProc = new gecoEnd(app);
  app->addGecoProcess(endProc);
  
  // configures the clock tick to 1 sec
  static_cast<gecoClock*>(app->getFirstGecoProcess())->setTick(1000);
  
  // runs app in the CLI mode
  app->runCLI();
  
  // cleans up everything
  delete app;
  
  return 0;
}
//! [main]