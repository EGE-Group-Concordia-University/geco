; ============ demo code for probe ==============
; Works with GRBL
; (c) 2022 Rolf Wuthrich, Concordia University
; ===============================================
G91                 ; All positions relative to current position
G21                 ; Units are in mm
G38.2 X-50 F100     ; Detect probe signal
G90                 ; All positions are absolute
G92 X0.0            ; Sets current position
M30                 ; Programm end
