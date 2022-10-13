// $Id: ECIOBoard.cc 37 2015-01-09 12:35:29Z wuthrich $
// ---------------------------------------------------------------
//
// Definition of the class gecoIOComedi
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

#include "gecoComediIOModule.h"
#include "geco.h"
#include <tcl.h>
#include <cstring>
#include <comedilib.h>
#include <comedi.h>

using namespace std;

#define gecoComediIOModuleVersion "1.0"

// ------------------------------------------------------------
//
// license
//

static CONST char* comediIOModule_license =
"   iocomedi a geco IOModule for communicating with the comedi library\n"
"   Copyright (c) 2015-2016, Rolf Wuthrich <rolf.wuthrich@concordia.ca>\n"
"   All rights reserved.\n "
"   \n"
"   Redistribution and use in source and binary forms, with or without\n"
"   modification, are permitted provided that the following conditions\n"
"   are met:\n"
"   \n"
"     * Redistributions of source code must retain the above copyright\n"
"       notice, this list of conditions and the following disclaimer.\n"
"     * Redistributions in binary form must reproduce the above copyright\n"
"       notice, this list of conditions and the following disclaimer in the\n"
"       documentation and/or other materials provided with the distribution.\n"
"     * Neither the name of Rolf Wuthrich nor the names of contributors\n"
"       to this software may be used to endorse or promote products derived\n"
"       from this software without specific prior written permission.\n"
"       \n"
"   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS\n"
"   ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT\n"
"   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR\n"
"   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR\n"
"   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,\n"
"   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,\n"
"   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR\n"
"   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF\n"
"   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING\n"
"   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS\n"
"   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.";


// ------------------------------------------------------------
//
// Tcl interface
//

extern "C" {

  Tcl_Namespace* comediIOModuleNsPtr;
  gecoPkgHandle* comediIOModulePkgHandle;

  // ---- Tcl package initalization procedure
  //

  int Comediiomodule_Init(Tcl_Interp *interp, gecoApp* app, 
			  gecoPkgHandle* pkgHandle)
  {
    comediIOModuleNsPtr=Tcl_CreateNamespace(interp,"iocomedi", NULL, NULL);
    comediIOModulePkgHandle=pkgHandle;
    Tcl_LinkVar(interp,"::iocomedi::license",
		(char *)&comediIOModule_license, 
		TCL_LINK_STRING | TCL_LINK_READ_ONLY);
    Tcl_SetVar(interp,"::iocomedi::version",gecoComediIOModuleVersion,0);
    Tcl_CreateObjCommand(interp,"iocomedi",geco_IOComediCmd, 
			 (ClientData) app, (Tcl_CmdDeleteProc *) NULL);

    Tcl_AppendResult(interp,
		     "+---------------------------------------------------+\n",
		     "|                comediIOModule                     |\n",
		     "|                  Version 1.0                      |\n",
		     "|          last modified January 2016               |\n",
		     "+---------------------------------------------------+\n",
		     "| Copyright (C) 2015-2016                           |\n",
		     "| Rolf Wuthrich, Concordia University, Canada       |\n",
		     "| This is free software                             |\n",
		     "| Type 'puts $::iocomedi::license' for more details |\n",
		     "| Type 'iocomedi -help' for help                    |\n",
		     "+---------------------------------------------------+",
		     NULL);
    return TCL_OK;
  }


  // ---- Tcl package unload procedure
  //

  int Comediiomodule_Unload(Tcl_Interp *interp)
  {
    Tcl_DeleteCommand(interp,"iocomedi"); 
    Tcl_DeleteNamespace(comediIOModuleNsPtr);

    Tcl_AppendResult(interp,"\n\tUnloaded comediIOModule\n",NULL); 

    return TCL_OK;
  }

} /* extern "C" */


// ---- Tcl iocomedi command
//

int geco_IOComediCmd(ClientData clientData, Tcl_Interp *interp, 
		     int objc,Tcl_Obj *const objv[])
{
  Tcl_ResetResult(interp);
  gecoApp*      app=(gecoApp *)clientData;
  gecoIOComedi* b;
  char          str[100];
  comedi_t*     device;

  if (objc==1)
    {
      Tcl_WrongNumArgs(interp,1,objv,"subcommand ?argument ...?");
      return TCL_ERROR;
    }

  int index;
  static CONST char* cmds[] = {"-help",
			       "-available",
			       "-open",
			       "-searchBoard",NULL};
  static CONST char* help[] = {"lists available boards",
			       "opens a board",
			       "returns the comedi device file associated to a board",
			       NULL};
  if (Tcl_GetIndexFromObj(interp,objv[1],cmds,"subcommand",'0',&index)!=TCL_OK)
    return TCL_ERROR;

  switch (index)
    {

    case 0: // -help
      if (objc!=2)
	{
	  Tcl_WrongNumArgs(interp,2,objv,NULL);
	  return TCL_ERROR;
	}
      gecoHelp(interp,"iocomedi","connects to IO-boards via the comedi library",
	       cmds,help);
      break;

    case 1: // -available
      if (objc!=2)
	{
	  Tcl_WrongNumArgs(interp,2,objv,NULL);
	  return TCL_ERROR;
	}
      Tcl_AppendResult(interp,
		       "COMEDI-FILE   BOARD             ",  
		       "AI-CHANNELS  AO-CHANNELS  DIO-CHANNELS\n",
		       NULL);
      char fileName[16];
      int i;
      i=0;
      strncpy(fileName,"/dev/comedi0",13);
      while (device=comedi_open(fileName))
        {
	  sprintf(str,"%-13s %-17s %-12d %-12d %-12d",
		  fileName,
		  comedi_get_board_name(device),
		  comedi_get_n_channels(device,
                      comedi_find_subdevice_by_type(device,COMEDI_SUBD_AI,0)),
		  comedi_get_n_channels(device,
                      comedi_find_subdevice_by_type(device,COMEDI_SUBD_AO,0)),
		  comedi_get_n_channels(device,
		      comedi_find_subdevice_by_type(device,COMEDI_SUBD_DIO,0)));
	  Tcl_AppendResult(interp,str,"\n",NULL);
	  comedi_close(device);
	  i++;
	  sprintf(fileName,"%s%d","/dev/comedi",i);
        }
      break;

    case 2: // -open
      if ((objc>4)||(objc<3))
	{
	  Tcl_WrongNumArgs(interp,2,objv,"comediFile ?boardName?");
	  return TCL_ERROR;
	}
      if (objc==3)
	b = new gecoIOComedi(app,Tcl_GetString(objv[2]),NULL);
      if (objc==4)
	b = new gecoIOComedi(app,Tcl_GetString(objv[2]),Tcl_GetString(objv[3]));
      if (!(b->getDevice())) 
	{
	  Tcl_AppendResult(interp,"could not open ",
			          Tcl_GetString(objv[2]),NULL);
	  delete b;
	  return TCL_ERROR;
	}
      Tcl_AppendResult(interp,b->getTclCmd(),NULL);
      break;

    case 3: // -searchBoard
      if (objc!=3)
	{
	  Tcl_WrongNumArgs(interp,2,objv,"boardName");
	  return TCL_ERROR;
	}
      char devName[16];
      i=0;
      strncpy(devName,"/dev/comedi0",13);
      while (device=comedi_open(devName))
        {
	  if (Tcl_StringMatch(comedi_get_board_name(device),Tcl_GetString(objv[2]))==1)
	    Tcl_AppendResult(interp,devName,NULL);
	  i++;
	  sprintf(devName,"%s%d","/dev/comedi",i);
	}
      break;

    }

  return TCL_OK;
}


// -----------------------------------------------------------------------
//
// Class to store linked Tcl variables and comedi instructions
//


// ---- CONSTRUCTOR
//

BoardInsn::BoardInsn(const char* Tcl_Var, 
                     int Type, 
                     int Chan, gecoIOComedi* Board) :
  IOModuleInsn(Tcl_Var,Type)
{
  chan=Chan;
  board=Board;
  instr=Board->instr_list.n_insns;
  Board->instr_list.n_insns++;

  // configures the comedi instruction
  if (Type==TclVarRead)
    {
      Board->instr[instr].insn=INSN_READ;
      Board->instr[instr].n=1;
      Board->instr[instr].data=&(Board->AI[Chan].sampl);
      Board->instr[instr].subdev=Board->inp_subdev;
      Board->instr[instr].chanspec=CR_PACK(Chan,Board->AI[Chan].range,
					   Board->AI[Chan].aref);
    }
  else
    {
      Board->instr[instr].insn=INSN_WRITE;
      Board->instr[instr].n=1;
      Board->instr[instr].data=&(Board->AO[Chan].sampl);
      Board->instr[instr].subdev=Board->out_subdev;
      Board->instr[instr].chanspec=CR_PACK(Chan,Board->AO[Chan].range,
					   Board->AO[Chan].aref);
    }
}


// ---- DESTRUCTOR
//

BoardInsn::~BoardInsn()
{
  // updates the instruction list
  board->instr_list.n_insns--;
  for (int j=instr;j<31;j++)
    {
      board->instr[j].insn=board->instr[j+1].insn;
      board->instr[j].data=board->instr[j+1].data;
      board->instr[j].subdev=board->instr[j+1].subdev;
      board->instr[j].chanspec=board->instr[j+1].chanspec;
    }
}


// ---------------------------------------------------------------
//
// Class gecoIOComedi
//


// ---- Call-back procedure 
//      Called whenever the user modifies the range of a channel
//

char* reconfigureBoard(ClientData clientData, Tcl_Interp *interp,
	     const char* name1, const char* name2, int flags)
{
  gecoIOComedi* b=(gecoIOComedi *)clientData;

  for (int i=0;i<b->nAIchannels;i++)
    {
      b->AI[i].maxdata = comedi_get_maxdata(b->device,b->inp_subdev,i);
      b->AI[i].cr = comedi_get_range(b->device,b->inp_subdev,i,b->AI[i].range);
    }

  for (int i=0;i<b->nAOchannels;i++)
    {
      b->AO[i].maxdata = comedi_get_maxdata(b->device,b->inp_subdev,i);
      b->AO[i].cr = comedi_get_range(b->device,b->inp_subdev,i,b->AO[i].range);
    }

  BoardInsn* p=b->getFirstInsn();
  while (p)
    {
      if (b->instr[p->instr].insn==INSN_READ)
	b->instr[p->instr].chanspec=CR_PACK(p->chan,b->AI[p->chan].range,
				     b->AI[p->chan].aref);
      else
	b->instr[p->instr].chanspec=CR_PACK(p->chan,b->AO[p->chan].range,
				     b->AO[p->chan].aref);
      p=p->getNext();
    }

  // no defined return value = crash
  return NULL;
}


// ---- CONSTRUCTOR
//

gecoIOComedi::gecoIOComedi(gecoApp* App, const char* fileName,char* boardName) :
  gecoIOModule("IOBoard IO-module","tmp",App)
{
  setUnlinkedToGeco();
  comediFile = new Tcl_DString;
  Tcl_DStringInit(comediFile);
  char str[100];

  device=comedi_open(fileName);
  if (!device) return;

  addOption("-linkTclVariable","links a Tcl variable");
  addOption("-board","returns the board name");
  addOption("-driver","returns the board driver name");
  addOption("-deviceFile","returns the associated comedi device file");
  addOption("-read","reads from an AI channel");
  addOption("-write","writes to an AO channel");
  addOption("-DIOread","reads from a DIO channel");
  addOption("-DIOwrite","writes to a DIO channel");
  addOption("-getAIchannels","returns the number of AI channels");
  addOption("-getAOchannels","returns the number of AO channels");
  addOption("-getDIOchannels","returns the number of DIO channels");
  addOption("-getAIranges","returns the number of ranges for an AI channel");
  addOption("-getAOranges","returns the number of ranges for an AO channel");
  addOption("-getAIrangeInfo","returns the range of an AI channel");
  addOption("-getAOrangeInfo","returns the range of an AO channel");
  addOption("-lock","locks the board to the current user");
  addOption("-unlock","unlocks the board");

  inp_subdev = comedi_find_subdevice_by_type(device,COMEDI_SUBD_AI,0);
  out_subdev = comedi_find_subdevice_by_type(device,COMEDI_SUBD_AO,0);
  DIO_subdev = comedi_find_subdevice_by_type(device,COMEDI_SUBD_DIO,0);

  Tcl_DStringAppend(comediFile,fileName,-1);

  // to avoid that data=min or data=max gives NAN
  comedi_set_global_oor_behavior(COMEDI_OOR_NUMBER);

  // removes any non-valid characters from the board name 
  // and restricts the length to max 20 characters
  if (boardName==NULL)
    snprintf(str,100,"join [regexp -all -inline {[a-z,A-Z,0-9]} %s] \"\"",
	     comedi_get_board_name(device));
  else
    snprintf(str,100,"join [regexp -all -inline {[a-z,A-Z,0-9]} %s] \"\"",
	     boardName);
  Tcl_Eval(interp,str);
  snprintf(str,20,"%s",Tcl_GetStringResult(App->getInterp()));
  Tcl_DStringInit(TclCmd);
  Tcl_DStringAppend(TclCmd,str,-1);
  Tcl_ResetResult(interp);
  

  // checks if board was already connected and/or dublicated
  Tcl_CmdInfo infoPtr;
  if (Tcl_GetCommandInfo(interp,Tcl_DStringValue(TclCmd),&infoPtr))
    {
      Tcl_AppendResult(interp,
	 "board already open or board with identical names\n",NULL);
      comedi_close(device);
      device=NULL;
      return;
    }

  // registers the TclCmd
  comediIOModulePkgHandle->registerTclCmd(str);

  // configures the board
  TclNamespace=Tcl_CreateNamespace(interp,Tcl_DStringValue(TclCmd),NULL,NULL);
  nAIchannels=comedi_get_n_channels(device,inp_subdev);
  if (nAIchannels>16) nAIchannels=16;
  for (int i=0;i<nAIchannels;i++)
    {
      AI[i].gain    = 1.0;
      AI[i].offset  = 0.0;
      AI[i].range   = 0;
      AI[i].aref    = AREF_GROUND;
      AI[i].maxdata = comedi_get_maxdata(device,inp_subdev,i);
      AI[i].cr      = comedi_get_range(device,inp_subdev,i,AI[i].range);
      sprintf(str,"::%s::AI_gain(%i)",Tcl_DStringValue(TclCmd),i);
      Tcl_LinkVar(interp,str,(char *)&AI[i].gain,TCL_LINK_DOUBLE);
      sprintf(str,"::%s::AI_offset(%i)",Tcl_DStringValue(TclCmd),i);
      Tcl_LinkVar(interp,str,(char *)&AI[i].offset,TCL_LINK_DOUBLE);
      sprintf(str,"::%s::AI_range(%i)",Tcl_DStringValue(TclCmd),i);
      Tcl_LinkVar(interp,str,(char *)&AI[i].range,TCL_LINK_INT);
    }
  sprintf(str,"::%s::AI_range",Tcl_DStringValue(TclCmd));
  Tcl_TraceVar(interp,str,TCL_TRACE_WRITES,reconfigureBoard,(ClientData) this);

  nAOchannels=comedi_get_n_channels(device,out_subdev);
  if (nAOchannels>16) nAOchannels=16;
  for (int i=0;i<nAOchannels;i++)
    {
      AO[i].gain    = 1.0;
      AO[i].offset  = 0.0;
      AO[i].range   = 0;
      AO[i].aref    = AREF_GROUND;
      AO[i].maxdata = comedi_get_maxdata(device,out_subdev,i);
      AO[i].cr      = comedi_get_range(device,out_subdev,i,AO[i].range);
      sprintf(str,"::%s::AO_gain(%i)",Tcl_DStringValue(TclCmd),i);
      Tcl_LinkVar(interp,str,(char *)&AO[i].gain,TCL_LINK_DOUBLE);
      sprintf(str,"::%s::AO_offset(%i)",Tcl_DStringValue(TclCmd),i);
      Tcl_LinkVar(interp,str,(char *)&AO[i].offset,TCL_LINK_DOUBLE);
      sprintf(str,"::%s::AO_range(%i)",Tcl_DStringValue(TclCmd),i);
      Tcl_LinkVar(interp,str,(char *)&AO[i].range,TCL_LINK_INT);
    }
  sprintf(str,"::%s::AO_range",Tcl_DStringValue(TclCmd));
  Tcl_TraceVar(interp,str,TCL_TRACE_WRITES,reconfigureBoard,(ClientData) this);

  // initalises comedi instruction list
  instr_list.n_insns=0;
  instr_list.insns=instr;
}


// ---- DESTRUCTOR
//

gecoIOComedi::~gecoIOComedi()
{
  if (device) Tcl_DeleteCommand(interp,Tcl_DStringValue(TclCmd));
  Tcl_DStringFree(comediFile);
  delete comediFile;
  if (!device) return;
  comedi_close(device);
}


// ---- CMD : process a potential command option (# i of objv[])
//            searches the options table 
//            if a match is found processes the command option and
//            returns the index from the options table
//            if no match returns -1

int gecoIOComedi::cmd(int &i,int objc,Tcl_Obj *const objv[])
{
  // first executes the command options defined in gecoIOModule
  int index=gecoIOModule::cmd(i,objc,objv);

  int j,n;
  unsigned int bit;
  char str[TCL_DOUBLE_SPACE];
  double x;

  if (index==getOptionIndex("-linkTclVariable"))
  {
    if (i+2>=objc)
      {
	Tcl_WrongNumArgs(interp,i+1,objv,"Tcl_Variable chan#");
	return -1;
      }
    strncpy(str,Tcl_GetString(objv[i+2]),TCL_DOUBLE_SPACE);
    if (Tcl_RegExpMatch(interp,str,"A[I,O][0-9]")!=1)
      {
	Tcl_AppendResult(interp,"wrong format for chan#: ",
			 "should be \"AI#\" or \"AO#\"",NULL);
	return -1;
      }
    if (Tcl_GetInt(interp,&str[2],&n)!=TCL_OK) return -1;

    // checks if an instruction can be added
    if (instr_list.n_insns==16) 
      {
	Tcl_AppendResult(interp,"can't add a new insruction;", 
			 "maximal number of instructions reached.",NULL);
	return -1;
      }

    // creates a new entry and links it
    BoardInsn* isn;
    if (str[1]=='I')
      isn = new BoardInsn(Tcl_GetString(objv[i+1]),TclVarRead,n,this);
    else
      isn = new BoardInsn(Tcl_GetString(objv[i+1]),TclVarWrite,n,this);
    if (addInsn(isn)==TCL_ERROR) 
      {
	delete isn;
	return -1;
      }
    i=i+3;
  }

  if (index==getOptionIndex("-board"))
    {
      Tcl_AppendResult(interp,getBoard(),NULL);
      i++;
    }

  if (index==getOptionIndex("-driver"))
    {
      Tcl_AppendResult(interp,getDriver(),NULL);
      i++;
    }

  if (index==getOptionIndex("-deviceFile"))
    {
      Tcl_AppendResult(interp,getComediFile(),NULL);
      i++;
    }

  if (index==getOptionIndex("-read"))
    {
      if (i+1>=objc)
      	{
      	  Tcl_WrongNumArgs(interp,i+1,objv,"chan#");
      	  return -1;
      	}
      if (Tcl_GetIntFromObj(interp,objv[i+1],&n)!=TCL_OK) return -1;
      if (readData(n,x)!=1)
	{
	  IOerror();
	  return -1;
	}
      Tcl_PrintDouble(interp,x,str);
      Tcl_AppendResult(interp,str,NULL);
      i=i+2;
    }

  if (index==getOptionIndex("-write"))
    {
      if (i+2>=objc)
      	{
      	  Tcl_WrongNumArgs(interp,i+1,objv,"chan# data");
      	  return -1;
      	}
      if (Tcl_GetIntFromObj(interp,objv[i+1],&n)!=TCL_OK) return -1;
      if (Tcl_GetDoubleFromObj(interp,objv[i+2],&x)!=TCL_OK) return -1;
      if (writeData(n,x)!=1)
	{
	  IOerror();
	  return -1;
	}
      i=i+3;
    }

  if (index==getOptionIndex("-DIOread"))
    {
      if (getDIOSubdev()<0)
	{
	  Tcl_AppendResult(interp,"no DIO channels available",NULL);
	  return -1;
	}
      if (i+1>=objc)
      	{
      	  Tcl_WrongNumArgs(interp,i+1,objv,"chan#");
      	  return -1;
      	}
      if (Tcl_GetIntFromObj(interp,objv[i+1],&n)!=TCL_OK) return -1;
      comedi_dio_config(getDevice(),getDIOSubdev(),n,COMEDI_INPUT);
      comedi_dio_read(getDevice(),getDIOSubdev(),n,&bit);
      if (bit) 
	Tcl_AppendResult(interp,"1",NULL);
      else
	Tcl_AppendResult(interp,"0",NULL);
      i=i+2;
    }

  if (index==getOptionIndex("-DIOwrite"))
    {
      if (getDIOSubdev()<0)
	{
	  Tcl_AppendResult(interp,"no DIO channels available",NULL);
	  return -1;
	}
      if (i+2>=objc)
      	{
      	  Tcl_WrongNumArgs(interp,i+1,objv,"chan# bit");
      	  return -1;
      	}
      if (Tcl_GetIntFromObj(interp,objv[i+1],&n)!=TCL_OK) return -1;
      comedi_dio_config(getDevice(),getDIOSubdev(),n,COMEDI_OUTPUT);
      if (Tcl_GetIntFromObj(interp,objv[i+2],&j)!=TCL_OK) return -1;
      if (comedi_dio_write(getDevice(),getDIOSubdev(),n,j)!=1)
	{
	  IOerror();
	  return -1;
	}
      i=i+3;
    }

  if (index==getOptionIndex("-getAIchannels"))
    {
      Tcl_AppendResult(interp,getAIchannels(),NULL);
      i++;
    }

  if (index==getOptionIndex("-getAOchannels"))
    {
      Tcl_AppendResult(interp,getAOchannels(),NULL);
      i++;
    }

  if (index==getOptionIndex("-getDIOchannels"))
    {
      Tcl_AppendResult(interp,getDIOchannels(),NULL);
      i++;
    }

  if (index==getOptionIndex("-getAIranges"))
    {
      if (i+1>=objc)
      	{
      	  Tcl_WrongNumArgs(interp,i+1,objv,"chan#");
      	  return -1;
      	}
      if (Tcl_GetIntFromObj(interp,objv[i+1],&n)!=TCL_OK) return -1;
      Tcl_AppendResult(interp,getAIranges(n),NULL);
      i=i+2;
    }

  if (index==getOptionIndex("-getAOranges"))
    {
      if (i+1>=objc)
      	{
      	  Tcl_WrongNumArgs(interp,i+1,objv,"chan#");
      	  return -1;
      	}
      if (Tcl_GetIntFromObj(interp,objv[i+1],&n)!=TCL_OK) return -1;
      Tcl_AppendResult(interp,getAOranges(n),NULL);
      i=i+2;
    }

  if (index==getOptionIndex("-getAIrangeInfo"))
    {
      if (i+2>=objc)
      	{
      	  Tcl_WrongNumArgs(interp,i+2,objv,"chan# range");
      	  return -1;
      	}
      if (Tcl_GetIntFromObj(interp,objv[i+1],&n)!=TCL_OK) return TCL_ERROR;
      if (Tcl_GetIntFromObj(interp,objv[i+2],&j)!=TCL_OK) return TCL_ERROR;
      Tcl_AppendResult(interp,getAIrangeInfo(n,j),NULL);
      i=i+3;
    }

  if (index==getOptionIndex("-getAOrangeInfo"))
    {
      if (i+2>=objc)
      	{
      	  Tcl_WrongNumArgs(interp,i+2,objv,"chan# range");
      	  return -1;
      	}
      if (Tcl_GetIntFromObj(interp,objv[i+1],&n)!=TCL_OK) return TCL_ERROR;
      if (Tcl_GetIntFromObj(interp,objv[i+2],&j)!=TCL_OK) return TCL_ERROR;
      Tcl_AppendResult(interp,getAOrangeInfo(n,j),NULL);
      i=i+3;
    }

  if (index==getOptionIndex("-lock"))
    {
      lock();
      i++;
    }

  if (index==getOptionIndex("-unlock"))
    {
      unlock();
      i++;
    }

  return index;
}


// ---- readData : reads data from a channel
//
//      returns -1 in case of an error
//

int gecoIOComedi::readData(int channel, double &data)
{
  lsampl_t d;
  lsampl_t max = comedi_get_maxdata(device,inp_subdev,channel);
  comedi_range* r=comedi_get_range(device,inp_subdev,channel,AI[channel].range);
  int ret=comedi_data_read(device,
                           inp_subdev,
                           channel,
                           AI[channel].range,
                           AI[channel].aref,&d);
  data=AI[channel].gain*(comedi_to_phys(d,r,max)+AI[channel].offset);
  return ret;
}


// ---- writeData : writes data to a channel
//      
//      returns -1 in case of an error
//

int gecoIOComedi::writeData(int channel, double data)
{
  lsampl_t d;
  d=comedi_from_phys(AO[channel].gain*data+AO[channel].offset,
		  comedi_get_range(device,out_subdev,channel,AO[channel].range),
		  comedi_get_maxdata(device,out_subdev,channel));
  return comedi_data_write(device,
                           out_subdev,
			   channel,
                           AO[channel].range,
                           AO[channel].aref,d);
}


// ---- GETAICHANNELS : returns the number of AI channels
//

const char* gecoIOComedi::getAIchannels()
{
  sprintf(nbr_str,"%d",nAIchannels);
  return nbr_str;
}


// ---- GETAOCHANNELS : returns the number of AO channels
//

const char* gecoIOComedi::getAOchannels()
{
  sprintf(nbr_str,"%d",nAOchannels);
  return nbr_str;
}


// ---- GETDIOCHANNELS : returns the number of DIO channels
//

const char* gecoIOComedi::getDIOchannels()
{
  sprintf(nbr_str,"%d",comedi_get_n_channels(device,DIO_subdev));
  return nbr_str;
}


// ---- GETAIRANGES : returns the number of AI ranges
//

const char* gecoIOComedi::getAIranges(int channel)
{
  sprintf(nbr_str,"%d",comedi_get_n_ranges(device,inp_subdev,channel));
  return nbr_str;
}


// ---- GETAORANGES : returns the number of AO ranges
//

const char* gecoIOComedi::getAOranges(int channel)
{
  sprintf(nbr_str,"%d",comedi_get_n_ranges(device,out_subdev,channel));
  return nbr_str;
}


// ---- GETAIRANGEINFO : returns the range of a AI channel
//

const char* gecoIOComedi::getAIrangeInfo(int channel, int range)
{
  comedi_range* r=comedi_get_range(device,inp_subdev,channel,range);
  if (r==NULL)
    sprintf(str,"NA NA");
  else
    sprintf(str,"%4.3f %4.3f",r->min,r->max);
  return str;
}


// ---- GETAORANGEINFO : returns the range of a AO channel
//

const char* gecoIOComedi::getAOrangeInfo(int channel, int range)
{
  comedi_range* r=comedi_get_range(device,out_subdev,channel,range);
  if (r==NULL)
    sprintf(str,"NA NA");
  else
    sprintf(str,"%4.3f %4.3f",r->min,r->max);
  return str;
}


// ---- IOERROR : to be called whenever an IO error occurs
//

void gecoIOComedi::IOerror()
{
  Tcl_DString errorStr;
  Tcl_DStringInit(&errorStr);
  Tcl_DStringAppend(&errorStr,"error \"Comedi IO error : ",-1);
  Tcl_DStringAppend(&errorStr,comedi_strerror(comedi_errno()),-1);
  Tcl_DStringAppend(&errorStr,"\"",-1);
  Tcl_Eval(interp,Tcl_DStringValue(&errorStr));
  Tcl_DStringFree(&errorStr);
}


// ---- INFO : returns a Tcl_DString containing relevant info about board
//             

Tcl_DString* gecoIOComedi::info(const char* frontStr)
{
  Tcl_DStringFree(infoStr);

  int i;
  char str[100];
  const char* sep = "------------------------------------------------------------\n";
  comedi_range* range;

  Tcl_DStringAppend(infoStr,sep,-1);
  Tcl_DStringAppend(infoStr,"Board name:   ",-1);
  Tcl_DStringAppend(infoStr,getBoard(),-1);
  Tcl_DStringAppend(infoStr,"\n",-1);
  Tcl_DStringAppend(infoStr,"Driver:       ",-1);
  Tcl_DStringAppend(infoStr,getDriver(),-1);
  Tcl_DStringAppend(infoStr,"\n",-1);
  Tcl_DStringAppend(infoStr,"Comedi file:  ",-1);
  Tcl_DStringAppend(infoStr,getComediFile(),-1);
  Tcl_DStringAppend(infoStr,"\n",-1);
  
   // Analog output channels
  
  if (nAOchannels>1)
    {
      Tcl_DStringAppend(infoStr,sep,-1);
      Tcl_DStringAppend(infoStr,"Analog output channels:\n\n",-1);
      Tcl_DStringAppend(infoStr,"CHANNEL\tRANGE\t\t\tNBR OF BITS\n",-1);
  
      for (i=0;i<nAOchannels;i++)
	{
	  range=comedi_get_range(device,out_subdev,i,AO[i].range);
	  sprintf(str,"%d\t[%4.3f ; %4.3f]\t%d\t\n",i,range->min,range->max,
		  comedi_get_maxdata(device,out_subdev,i));
	  Tcl_DStringAppend(infoStr,str,-1);
	}
    }

  // Analog input channels

  if (nAIchannels>1)
    {
      Tcl_DStringAppend(infoStr,sep,-1);
      Tcl_DStringAppend(infoStr,"Analog input channels:\n\n",-1);
      Tcl_DStringAppend(infoStr,"CHANNEL\tRANGE\t\t\tNBR OF BITS\n",-1);
  
      for (i=0;i<nAIchannels;i++)
	{
	  range=comedi_get_range(device,inp_subdev,i,AI[i].range);
	  sprintf(str,"%d\t[%4.3f ; %4.3f]\t%d\t\n",i,range->min,range->max,
		  comedi_get_maxdata(device,inp_subdev,i));
	  Tcl_DStringAppend(infoStr,str,-1);
	}
    }

  // Digital channels

  int n = comedi_get_n_channels(device,DIO_subdev);
  if (n>0)
    {
      Tcl_DStringAppend(infoStr,sep,-1);
      Tcl_DStringAppend(infoStr,"Digital channels:\n\n",-1);

      for (i=0;i<n;i++)
	{
	  unsigned int dir;
	  comedi_dio_get_config(device,DIO_subdev,i,&dir);
	  unsigned int bit;
	  comedi_dio_read(device,DIO_subdev,i,&bit);
	  if (dir==COMEDI_INPUT)
	    sprintf(str,"%d\t INPUT\t%d\n",i,bit);
	  else
	    sprintf(str,"%d\t OUTPUT\t%d\n",i,bit);
	  Tcl_DStringAppend(infoStr,str,-1);
	}
    }

  return infoStr;
}


// ---- LISTINSTR : lists the comedi instruction list
//

void gecoIOComedi::listInstr()
{
  char str[100];
  Tcl_AppendResult(interp,"NUM  OPERATION  CHANNEL   TCL VARIABLE\n",NULL);

  BoardInsn* p=getFirstInsn();
  while (p)
    {
      int i=p->instr;
      if (instr[i].insn==INSN_READ)
	sprintf(str,"%-4d read       AI%-3d     %s\n",i,p->chan,
		Tcl_DStringValue(p->TclVar));
      else
	sprintf(str,"%-4d write      AO%-3d     %s\n",i,p->chan,
		Tcl_DStringValue(p->TclVar));
      Tcl_AppendResult(interp,str,NULL);
      p=p->getNext();
    }
}


// ---- DOINSTR : executes the comedi instruction list
//
//      returns number of succesfull operations done
//

int gecoIOComedi::doInstr()
{
  // computes the comedi sample values for write instructions
  BoardInsn* p=getFirstInsn();
  while (p)
    {
      if (instr[p->instr].insn==INSN_WRITE)
	AO[p->chan].sampl=
                comedi_from_phys(AO[p->chan].gain*p->data+AO[p->chan].offset,
  				 AO[p->chan].cr,AO[p->chan].maxdata);
      p=p->getNext();
    }


  // executes the comedi instruction list
  int ret=comedi_do_insnlist(device,&instr_list);

  // computes the physical input values
  p=getFirstInsn();
  while (p)
    {
      if (instr[p->instr].insn==INSN_READ)
	p->data=AI[p->chan].gain*(
	   comedi_to_phys(AI[p->chan].sampl,AI[p->chan].cr,AI[p->chan].maxdata)
           +AI[p->chan].offset);	
      p=p->getNext();
    }

  return ret;
}


// ---- update : executes the comedi instruction associated to the Tcl_Var
//
//      returns  1 if succesfull
//               0 if Tcl_Var is not linked
//              -1 if a comedi error occured
//

int gecoIOComedi::update(const char* Tcl_Var)
{
  BoardInsn* p=findLinkedTclVariable(Tcl_Var);
  if (p==NULL) return 0;
  int i=p->chan;

  // computes the comedi sample values for output in case it was an AO channel
  if (instr[p->instr].insn==INSN_WRITE)
    AO[i].sampl=comedi_from_phys(AO[i].gain*p->data+AO[i].offset,
  				 AO[i].cr,AO[i].maxdata);

  // executes the comedi instruction
  int ret=comedi_do_insn(device,&instr[p->instr]);

  // computes the physical input values in case it was an AI channel
  if (instr[p->instr].insn==INSN_READ)
    p->data=AI[i].gain*(comedi_to_phys(AI[i].sampl,AI[i].cr,AI[i].maxdata)
		   +AI[i].offset);
  return ret;
}
