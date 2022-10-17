# geco
GECO the general communication library

The GECO library is a a collection of C++ classes to facilitate the communication with IoT devices. It integrates a Tcl interpreter and provides a class for the construction of applications running GECO.

GECO was designed and tested on Linux Ubuntu and Raspian but is probably working on any Linux distribution.

## Dependences
To install on your system the required libraries and software run
```
sudo apt-get install gnuplot
sudo apt-get install tcl8.6 tcl8.6-dev tk8.6 tk8.6-dev
sudo apt-get install tclreadline
sudo apt-get install g++
sudo apt-get install make
```
If you plan to use the ```gecoComediIOModule``` GECO package you further need to run
```
sudo apt-get install libcomedi-dev
```

## Building

Using make, in the directory containing the GECO Makfile:
```
make
```
To install the library system wide
```
sudo make install
```
This will copy the GECO library to ```/usr/local/lib/```.<br>
The header files of the library are copied to ```/usr/local/include/```.<br>
Shared files by GECO are copied to ```/usr/local/share/geco```<br>
The system-wide configuration files are copied to ```/usr/local/etc/geco```.

To generate the documentation using Doxygen:
```
make documentation
```
You may wish to check the Makefile in order to put a suitable directory to copy the generated HTML documents for your particular system.
The relevant line to check for is:
```
cp -r html/* /var/www/html/ege/geco/
```
Change as needed the destination folder.

To clean up the working directory
```
make clean
```

## Building GECO packages
To build a GECO package use ```make``` in the folder of the package:
```
make
```
To install the package system wide
```
sudo make install
```
This will copy the GECO package to ```/usr/local/share/geco/pkg/```.
