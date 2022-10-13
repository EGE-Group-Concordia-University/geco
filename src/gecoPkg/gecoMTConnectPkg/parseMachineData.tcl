set a <Idle|MPos:0.000,0.000,0.000|FS:0,0>

set data [lindex [::split $a <>] 1]
set fields [::split $data "|"]
set status [lindex $fields 0]
set coords [lindex [::split [lindex $fields 1] ":"] 1]
set X [lindex [::split $coords ,] 0]
set Y [lindex [::split $coords ,] 1]
set Z [lindex [::split $coords ,] 2]


