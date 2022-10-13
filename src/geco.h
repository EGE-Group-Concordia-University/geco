// This may look like C code, but it is really -*- C++ -*-
// $Id: ECApp.h 15 2014-01-03 18:49:04Z wuthrich $
// ---------------------------------------------------------------- 
//                                                                  
// Header file for the geCo library
//
// (c) Rolf Wuthrich
//     2015-2020 Concordia University
//
// author:  Rolf Wuthrich
// email:   rolf.wuthrich@concordia.ca
// version: v2 ($Revision: 15 $)
//
// This software is copyright under the BSD license
//
// ---------------------------------------------------------------
// history:
// ---------------------------------------------------------------
// Date       Modification                     Author
// ---------------------------------------------------------------
// 12.10.2015 Creation                         R. Wuthrich
// 21.11.2020 Added DOxygen documentation      R. Wuthrich
// ---------------------------------------------------------------

#ifndef geco_SEEN_
#define geco_SEEN_

#include <tcl.h>
#include <tk.h>
#include "gecoHelp.h"
#include "gecoEvent.h"
#include "gecoObj.h"
#include "gecoProcess.h"
#include "gecoTrigger.h"
#include "gecoUProc.h"
#include "gecoGraph.h"
#include "gecoFileStream.h"
#include "gecoMemStream.h"
#include "gecoIOModule.h"
#include "gecoIO.h"
#include "gecoIOSocket.h"
#include "gecoIOTcp.h"
#include "gecoClock.h"
#include "gecoApp.h"
#include "gecoPkgHandle.h"
#include "gecoGenerator.h"
#include "gecoTriangle.h"
#include "gecoSawtooth.h"
#include "gecoStep.h"
#include "gecoPulse.h"
#include "gecoEnd.h"
#include "gecoTcpServer.h"



/*! \mainpage geCo - The general communication library
 *
 * \tableofcontents
 *
 * \section intro_sec Introduction
 *
 * The geCo library is a a collection of c++ classes to facilitate the communication with IoT devices.
 * It integrates a Tcl interpreter and provides a class for the construction of application running geCo.
 *
 * \section install_sec Installation
 *
 * \subsection Prerequisites 
 * Tcl/Tk development package version 8.6 
 *
 * \subsection Step1 
 * Build geCo library with 'make'
 * \subsection Step3 
 * Install the library with 'sudo make install'
 * This will create a dynamic library that can be linked later to your code.
 */

#endif /* geco_SEEN_ */
