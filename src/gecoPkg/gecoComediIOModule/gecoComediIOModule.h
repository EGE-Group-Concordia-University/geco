// This may look like C code, but it is really -*- C++ -*-
// $Id: ECIOBoard.h 37 2015-01-09 12:35:29Z wuthrich $
// ---------------------------------------------------------------- 
//                                                                  
// Header file for the class gecoComediIOModule
//
// (c) Rolf Wuthrich
//     2015-2016 Concordia University
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
// 13.11.2015 Creation                         R. Wuthrich
// ---------------------------------------------------------------

#ifndef GECOCOMEDIIOMODULE_SEEN_
#define GECOCOMEDIIOMODULE_SEEN_

#include <tcl.h>
#include <comedilib.h>
#include <comedi.h>
#include "geco.h"

using namespace std;


// -----------------------------------------------------------------------
//
// Tcl interface
//

extern "C" {

  int Comediiomodule_Init(Tcl_Interp *interp, gecoApp* app, 
			  gecoPkgHandle* pkgHandle);
  int Comediiomodule_Unload(Tcl_Interp *interp)

  int geco_IOComediCmd(ClientData clientData, Tcl_Interp *interp, 
		       int objc,Tcl_Obj *const objv[]);

} /* extern "C" */


// -------------------------------------------------------------------------
//
// Structure storing the configuration of IO channels

struct chanCfg
{
  double        gain;         // channel gain
  double        offset;       // channel offset
  int           range;        // channel range
  int           aref;         // channel reference
  lsampl_t      maxdata;      // channel maxdata
  comedi_range* cr;           // channel comedi range
  lsampl_t      sampl;        // comedi representation of data
};


// -----------------------------------------------------------------------
//
// Class to store linked Tcl variables and comedi instructions
//

class gecoIOComedi; // forward definition

class BoardInsn : public IOModuleInsn
{
  friend char* reconfigureBoard(ClientData clientData, Tcl_Interp *interp,
  			const char* name1, const char* name2, int flags);
  friend class gecoIOComedi;

protected:

  int             instr;        // associated comedi instruction (-1 = none)
  int             chan;         // associated comedi channel number
  gecoIOComedi*   board;        // associated comedi board

public:

  BoardInsn(const char* Tcl_Var, int Type, int Chan, gecoIOComedi* Board);
  ~BoardInsn();

  BoardInsn* getNext()  {return static_cast<BoardInsn*>(next);}
};


// -----------------------------------------------------------------------
//
// class gecoIOComedi
//

class gecoIOComedi : public gecoIOModule
{
  friend char* reconfigureBoard(ClientData clientData, Tcl_Interp* interp,
  			const char* name1, const char* name2, int flags);
  friend class gecoIO;
  friend class BoardInsn;

private:

  char nbr_str[TCL_DOUBLE_SPACE];  // needed as tmp variable for some functions
  char str[80];                    // needed as tmp variable for some functions

protected:

  Tcl_DString*   comediFile;         // associated comedi file
  comedi_t*      device;             // I-O device

  int inp_subdev;                    // AI subdevice
  int out_subdev;                    // AO subdevice
  int DIO_subdev;                    // DIO subdevice
  int nAIchannels;                   // nbr of AI channels
  int nAOchannels;                   // nbr of AO channels

  chanCfg AO[16];                    // AO channel configuration
  chanCfg AI[16];                    // AI channel configuration

  comedi_insn     instr[32];
  comedi_insnlist instr_list;
  
public:

  gecoIOComedi(gecoApp* App, const char* fileName, char* boardName);
  ~gecoIOComedi();

  virtual int cmd(int &i,int objc,Tcl_Obj *const objv[]);

  int         readData(int channel, double &data);
  int         writeData(int channel, double data);

  comedi_t*   getDevice()     {return device;}
  char*       getComediFile() {return Tcl_DStringValue(comediFile);}

  const char* getBoard()  {return comedi_get_board_name(device);}
  const char* getDriver() {return comedi_get_driver_name(device);}

  const char* getAIchannels();
  const char* getAOchannels();
  const char* getDIOchannels();
  int         getnAIchannels() {return nAIchannels;}
  int         getnAOchannels() {return nAOchannels;}
  const char* getAIranges(int channel);
  const char* getAOranges(int channel);
  const char* getAIrangeInfo(int channel, int range);
  const char* getAOrangeInfo(int channel, int range);

  int getInpSubdev() {return inp_subdev;}
  int getOutSubdev() {return out_subdev;}
  int getDIOSubdev() {return DIO_subdev;}
  
  int lock()   {return comedi_lock(device,out_subdev);}
  int unlock() {return comedi_unlock(device,out_subdev);}

  virtual void IOerror();
  virtual Tcl_DString* info(const char* frontStr = "");

  BoardInsn* getFirstInsn() {return static_cast<BoardInsn*>(firstIOModuleInsn);}
  virtual void listInstr();
  virtual int  doInstr();
  virtual int  update(const char* Tcl_Var);

  BoardInsn*   findLinkedTclVariable(const char* TclVar) 
    {return static_cast<BoardInsn*>(gecoIOModule::findLinkedTclVariable(TclVar));}

};

#endif /* GECOCOMEDIIOMODULE_SEEN_ */
