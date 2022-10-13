// $Id: ECIOTcp.cc 37 2015-01-09 12:35:29Z wuthrich $
// ---------------------------------------------------------------
//
// Definition of the class gecoDS18B20
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
// 06.11.2020 Creation                         R. Wuthrich
// ---------------------------------------------------------------

#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <tcl.h>
#include "gecoHelp.h"
#include "gecoApp.h"
#include "gecoDS18B20.h"

using namespace std;


// ------------------------------------------------------------
//
// Tcl interface
//


// Command to create a new gecoDS18B20 object
//


int gecoPiDS18B20Cmd(ClientData clientData, Tcl_Interp *interp, 
		     int objc,Tcl_Obj *const objv[])
{
  gecoDS18B20* proc  = new gecoDS18B20((gecoApp *)clientData);
  return geco_CreateGecoProcessCmd(proc, objc, objv);
}



// ---------------------------------------------------------------
//
// class gecoDS18B20
//


// ---- CONSTRUCTOR
//

gecoDS18B20::gecoDS18B20(gecoApp* App) :
  gecoObj("DS18B20 temp sensor", "ds18B20_", App),
  gecoSensor("DS18B20 temp sensor", "ds18B20_", App)
{
  device = new Tcl_DString;
  Tcl_DStringInit(device);
  
  //system("sudo modprobe w1-gpio");
  //system("sudo modprobe w1-therm");
  
  // finds the first device file of a connected DS18B20 sensor
  DIR *dir;
  struct dirent *dirEntry;
  dir = opendir(gecoPiw1Bus);
  while((dirEntry = readdir(dir))) 
    if (strncmp(dirEntry->d_name, "28", 2) == 0)
	{
      Tcl_DStringAppend(device, dirEntry->d_name, -1);
	  break;
    }
  
  addOption("-deviceFile", device, "returns/sets device file");
  addOption("-readSensor", "reads sensor value");
  addOption("-listSensors", "lists physically connected DS18B20 sensors");
}


// ---- DESTRUCTOR
//

gecoDS18B20::~gecoDS18B20()
{
  Tcl_DStringFree(device);
  delete device;
}


// ---- CMD : process a potential command option (# i of objv[])
//            searches the options table 
//            if a match is found processes the command option and
//            returns the index from the options table
//            if no match returns -1

int gecoDS18B20::cmd(int &i, int objc,Tcl_Obj *const objv[])
{
  // first executes the command options defined in gecoSensor
  int index=gecoSensor::cmd(i, objc, objv);
  
  if (index==getOptionIndex("-listSensors"))
    {
	  DIR *dir;
	  struct dirent *dirEntry;
	  dir = opendir(gecoPiw1Bus);
	  while((dirEntry = readdir(dir)))
      {        
        if (strncmp(dirEntry->d_name, "28", 2) == 0)
		  cout << dirEntry->d_name << "\n";
      }
	  i++;
	}
	
  if (index==getOptionIndex("-readSensor"))
    {
      char str[80];
      sprintf(str,"%f", readSensor());
      Tcl_AppendResult(interp, str, NULL);
	  i++;
	}

  return index;
}


// ---- INFO : returns a Tcl_DString containing relevant info
//             

Tcl_DString* gecoDS18B20::info(const char* frontStr)
{
  gecoSensor::info(frontStr);
  addInfo(frontStr,"Device file = ", Tcl_DStringValue(device));
  return infoStr;
}


// ---- READSENSOR : reads and returns the sensor value
//                   Has to be defined by child
//

double gecoDS18B20::readSensor()
{
  FILE *fp = NULL;
  Tcl_DString* name = new Tcl_DString;
  Tcl_DStringInit(name);
  Tcl_DStringAppend(name, gecoPiw1Bus, -1);
  Tcl_DStringAppend(name, Tcl_DStringValue(device), -1);
  Tcl_DStringAppend(name, gecoPiw1Slave, -1);
  fp = fopen(Tcl_DStringValue(name), "r");
  
  fseek(fp, 0, SEEK_END);
  long deviceFileSize = ftell(fp);
  fseek(fp, 0, SEEK_SET);
  
  char* buffer = (char*) calloc(deviceFileSize, sizeof(char));
  fread(buffer, sizeof(char), deviceFileSize, fp);
  char *tempCh = strstr(buffer, "t=");
  tempCh +=2; //move pointer 2 spaces to compensate for t=
  float temp = atof(tempCh);
  
  free(buffer);
  fclose(fp);
  
  return temp/1000.0;
}
