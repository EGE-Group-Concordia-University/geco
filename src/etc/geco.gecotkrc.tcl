# $Id: ectk.ectkrc.tcl 15 2014-01-03 18:49:04Z wuthrich $
# ----------------------------------------------------------------
#
# /usr/local/etc/geco/geco.gecorc.tcl: system-wide gecorc file for geCo
#
# This file is sourced by gecoApp at start-up
#
# (c) Rolf Wuthrich
#     2015 Concordia University
#
# version: $Revision: 15 $
#
# ----------------------------------------------------------------


# ----------------------------------------------------------------
# 
# System specific configuration
# 
# Sources any tcl script in /usr/local/etc/geco/geco.d
#

set etc_dir /usr/local/etc/geco

foreach f [glob -directory $etc_dir/geco.d *.tcl] {
    source $f
}


# ----------------------------------------------------------------

