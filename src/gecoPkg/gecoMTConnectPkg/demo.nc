; ============ Demo code for GRBL ==============
; Works with GRBL
; (c) 2022 Rolf Wuthrich, Concordia University
; ===============================================
G21             ; units are in mm
G01 X+2 F100
G01 X+4
G01 X+6
G01 X+8
G01 X-2 F40
M30             ; program end