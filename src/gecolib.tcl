# ---------------------------------------------------------
#
# This is the geCo Tcl library
#
# This file is sourced by gecoApp at start-up
#
# (c) Rolf Wuthrich
#     2015-2022 Concordia University
#
# author:  Rolf Wuthrich
# email:   rolf.wuthrich@concordia.ca
# version: $Revision: 15 $
#
# This software is copyright under the BSD license
#
# ---------------------------------------------------------
# history:
# ---------------------------------------------------------
# Date       Modification                     Author
#----------------------------------------------------------
# 10.12.2015 Creation                         R. Wuthrich
# 28.05.2022 Moved Tk stuff to end            R. Wuthrich
#----------------------------------------------------------

fconfigure stdin -blocking 0

# number of significant digits to be used in display operations
set tcl_precision 4


# -----------------------------------------------------------------------
#
# defines global variables of geCo
#
# -----------------------------------------------------------------------

# assigns the name of the logged user to the Tcl variable user
set ::geco::user $tcl_platform(user)

# assigns "# " as the standard comment string to be added
# in front of comments in files
set ::geco::commentStr "# "


# ----------------------------------------------------------------------
#
# wait - waits during N seconds
#
# usage : wait N
#
# -----------------------------------------------------------------------

proc wait {N} {
    after [expr {int($N * 1000)}]
}


# -----------------------------------------------------------------------
#
# Tk related operations
#
# -----------------------------------------------------------------------

# hide the main window of the tk application
wm state . withdrawn

# standard colour palette to be used
tk_setPalette white

# load additional modules
source /usr/local/share/geco/plot.tcl


# ----------------------------------------------------------------------
#
# Console
#
# -----------------------------------------------------------------------

toplevel .console
text .console.text -relief sunken -borderwidth 2 \
                   -yscrollcommand ".console.scroll set" \
                   -height 7 \
                   -width 80 \
                   -setgrid true
scrollbar .console.scroll -command ".console.text yview"
pack .console.scroll -side right -fill y
pack .console.text -side left -fill both -expand 1

wm protocol .console WM_DELETE_WINDOW {wm state .console withdrawn}
wm title .console "Console"
wm state .console withdrawn


# ----------------------------------------------------------------------
#
# cons - sends data to the console
#
# usage : con data
#
# -----------------------------------------------------------------------

proc cons {data} {
    .console.text insert end $data
    .console.text insert end "\n"
    .console.text see end
} 


# ----------------------------------------------------------------------
#
# console - manipulates the console
#
# usage : console ?option?
#
# valid options are:
#
# -clear : clears the content of the console
# -hide  : hides the console
# -show  : shows the console
#
# -----------------------------------------------------------------------

proc console {{subcmd "-show"}} {
    switch -exact -- $subcmd {
	-clear {.console.text delete 1.0 end}
	-hide {wm state .console withdrawn}
	-show {wm state .console normal}
	default {
	    puts "bad option \"$subcmd\": must be -show, -hide or -clear"}
    }
}

