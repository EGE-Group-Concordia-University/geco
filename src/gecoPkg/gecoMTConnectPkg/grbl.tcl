loadGecoPkg ./libgecoMtcPkg1.0.so


# -----------------------------------------------------------------------------
# RS232 communication with Arduino type board
global ser
set ser [open /dev/ttyUSB0 r+]

# baud rate = 115200
# odd parity none
# 8 data bits
# 1 stop bit
fconfigure $ser -mode "115200,n,8,1"

# blocking on read (with timeout), don't buffer output, end-line character is CR
fconfigure $ser -blocking 1 -buffering none -timeout 200 

# prints welcome message
flush $ser
after 4000
puts [read $ser]

mtcgrbladapter -grbl $ser
mtcgrbladapter2 -gcodefile zero.nc
mtcgrbladapter2 -verbose off
start


