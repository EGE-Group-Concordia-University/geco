# geco
GECO the general communication library

The GECO library is a a collection of C++ classes to facilitate the communication with IoT devices. It integrates a Tcl interpreter and provides a class for the construction of applications running GECO.

GECO was designed and tested on Linux Ubuntu and Raspian but is probably working on any Linux distribution.

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
The relvant line to check for is:
```
cp -r html/* /var/www/html/ege/geco/
```
Change as needed the destination folder.

To clean up the working directory
```
make clean
```
