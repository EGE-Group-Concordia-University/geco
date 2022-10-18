# $Id: ectk_install.tcl 19 2014-01-04 17:43:15Z wuthrich $
# ---------------------------------------------------------------
#
# geCo install facility
#
# (c) Rolf Wuthrich
#     2015-2016 Concordia University
#
# author:  Rolf Wuthrich
# email:   rolf.wuthrich@concordia.ca
# version: $Revision: 19 $
#
# This software is copyright under the BSD license
#
# ---------------------------------------------------------------
# history:
# ---------------------------------------------------------------
# Date       Section       Modification             Author
# ---------------------------------------------------------------
# 17.10.15   all           creation                 R. Wuthrich 
# ---------------------------------------------------------------

# Version
set ver 1.0

# Important directories
set share_dir /usr/local/share/geco
set pkg_dir   /usr/local/share/geco/pkg
set etc_dir   /usr/local/etc/geco
set lib_dir   /usr/local/lib
set inc_dir   /usr/local/include/geco$ver
set man_dir   /usr/local/share/man

# Defnines an entry in /etc/ld.so.conf.d for the geco packages
file copy -force gecoPkg.conf /etc/ld.so.conf.d

# First create the needed directory structure

if {[file exists $share_dir]==0} {
    file mkdir $share_dir
}

if {[file exists $pkg_dir]==0} {
    file mkdir $pkg_dir
}

if {[file exists $etc_dir]==0} {
    file mkdir $etc_dir
}

if {[file exists $etc_dir/geco.d]==0} {
    file mkdir $etc_dir/geco.d
}

if {[file exists $man_dir/man1]==0} {
    file mkdir $man_dir/man1
}

if {[file exists $man_dir/man3]==0} {
    file mkdir $man_dir/man3
}

if {[file exists $inc_dir/examples]==0} {
    file mkdir $inc_dir/examples
}


# Second copy the various files

file copy -force gecolib.tcl plot.tcl $share_dir
file copy -force etc/geco.gecotkrc.tcl $etc_dir

cd ..
file copy -force AUTHORS $share_dir
file copy -force LICENSE $share_dir

