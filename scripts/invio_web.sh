#!/bin/bash

smbclient -U meteo_img%meteo  //172.16.1.6/meteo <<EOF
cd sat

del $2"_1.png"

rename $2"_2.png" $2"_1.png"
rename $2"_3.png" $2"_2.png"
rename $2"_4.png" $2"_3.png"
rename $2"_5.png" $2"_4.png"

put $1 $2"_5.png"

EOF

