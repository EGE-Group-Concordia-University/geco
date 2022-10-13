loadGecoPkg ./libgecoMtcPkg1.0.so

package require http
set ::temp_handle [::http::geturl http://10.0.0.219/temp]
set ::humi_handle [::http::geturl http://10.0.0.219/humi]

clk -tick 10000

uproc -userProcess {set Temp [::http::data $::temp_handle]; set ::temp_handle [::http::geturl http://10.0.0.219/temp]} \
    -activateOnStart on \
    -comment "Temperature"
uproc -userProcess {set Humi [::http::data $::humi_handle]; set ::humi_handle [::http::geturl http://10.0.0.219/humi]} \
    -activateOnStart on \
    -comment "Humidity"

set mtc_adapt_pid [mtcadapter]
set mtc_adapt [ps -cmd $mtc_adapt_pid]
$mtc_adapt -linkTclVariable Temp Temp
$mtc_adapt -linkTclVariable Humi Humi

