G91                 ; All moves are relative to current position
G21                 ; Units are mm
G38.2 X-50 F100     ; Detect probe signal
G90                 ; All positions are absolute
G92 X0.0            ; Sets current position
M30                 ; Programm end
