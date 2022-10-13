# CVS ID : $Id: plot.tcl 39 2015-01-09 13:09:42Z wuthrich $
# ---------------------------------------------------------------
#
# geCo interface for plotting
#
# (c) Rolf Wuthrich
#     2015-2016 Concordia University
#
# author:  Rolf Wuthrich
# email:   rolf.wuthrich@concordia.ca
# version: $Revision: 39 $
#
# ---------------------------------------------------------------
# history:
# ---------------------------------------------------------------
# Date       Section       Modification             Author
# ---------------------------------------------------------------
# 24.10.15   all           creation                 R. Wuthrich 
#                                                                  
# ---------------------------------------------------------------

global PlotPipe

# ----------------------------------------------------------------------
#
# openPlotPipe - opens a pipline to gnuplot
#
# usage : openPlotPipe
#
# -----------------------------------------------------------------------

proc openPlotPipe {} {
    global PlotPipe
    if [catch {open |gnuplot w} PlotPipe] {
	puts stderr "Cannot open gnuplot: $PlotPipe"
    }
}


# ----------------------------------------------------------------------
#
# closePlotPipe - closes the pipline to gnuplot
#
# usage : closePlotPipe
#
# -----------------------------------------------------------------------

proc closePlotPipe {} {
    global PlotPipe
    close $PlotPipe
}



# ----------------------------------------------------------------------
#
# plotTerminal - defines the plot terminal
#
# usage : plotTerminal terminal-type
#
# -----------------------------------------------------------------------

proc plotTerminal {type} {
    global PlotPipe

    if {[catch {flush $PlotPipe}] > 0} {
	openPlotPipe
    }

    puts $PlotPipe "set terminal $type"
    flush $PlotPipe
}


# ----------------------------------------------------------------------
#
# plotGnuplot - plots the content of a file
#
# usage : plotGnuplot file
#
# -----------------------------------------------------------------------

proc plotGnuplot {file {args ""}} {
    global PlotPipe

    if {[catch {flush $PlotPipe}] > 0} {
	openPlotPipe
    }

    puts $PlotPipe "plot \"$file\" $args"
    flush $PlotPipe
}
