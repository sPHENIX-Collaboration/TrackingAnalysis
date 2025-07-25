#!/bin/sh


source /opt/sphenix/core/bin/sphenix_setup.sh -n new
ROOT_INCLUDE_PATH=${OPT_SPHENIX}/include:${ROOT_INCLUDE_PATH}
logfile="transfer_checking.log"
yesterday=$(date --date="yesterday" +"%Y-%m-%d")

echo "Date to check " ${yesterday}
root -b -q CheckFileTransfer.C\(\"${yesterday}\"\) >& ${logfile} 2>&1
fullruns=$(grep  'Complete runs (all segments):' ${logfile})
allseginbraces=$(echo "${fullruns}" | sed -n 's/.*{ *\([^}]*\)*}.*/\1/p')
fullruns0=$(grep "Complete runs (first segment):" ${logfile})
zeroseginbraces=$(echo "${fullruns0}" | sed -n 's/.*{ *\([^}]*\) *}.*/\1/p')

echo "Full runs:     "${allseginbraces}"
0th segment runs "${zeroseginbraces}"
successfully transferred in the last 24 hours since "${yesterday} | s-nail -s "Transferred run summary" osbornjd91@gmail.com
