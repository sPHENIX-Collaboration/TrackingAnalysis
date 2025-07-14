#!/bin/bash

source /opt/sphenix/core/bin/sphenix_setup.sh -n new
goodRunList=goodRun.list
partialRunList=partialRun.list

#Have some default run number in case there is no existing run list and we have no runs in the web page
defaultRun=68750

#Where is the plot webpage
webPage=/sphenix/WWW/subsystem/Silicon_MBD_Vertex_Comparison/

#First, check to see if good run list exists
if [ -f ${goodRunList} ]; then 
  echo ${goodRunList} exists!
else
  echo ${goodRunList} is missing! Finding latest run on webpage
  webRunArray=($(ls -1 ${webPage} | grep Run))
  #Now check if there are actually runs here
  if [ ${#webRunArray[@]} == 0 ]; then
    echo "There are no runs listed on our webpages, setting a default run number"
    echo ${defaultRun} > ${goodRunList}
  else
    #Send the last completed run to the good run list
    readarray -t sortedWebRunArray < <(for a in "${goodWebArray[@]}"; do echo "$a"; done | sort)
    lastRunNumber=${webRunArray[-1]//"Run"/}
    echo ${lastRunNumber} > ${goodRunList}
  fi
fi

#Lets get the latest run list and find the highest run number that was submitted
#Check that there are actually runs in the list
nRunsInList=`cat ${goodRunList} | wc -l`
if [ ${nRunsInList} == 0 ]; then
  echo ${defaultRun} > ${goodRunList}  
fi

readarray goodRunArray < ${goodRunList} 
readarray -t sortedGoodRunArray < <(for i in "${goodRunArray[@]}"; do echo "$i"; done | sort)
latestGoodRun=${sortedGoodRunArray[-1]}
echo The latest analysed run is ${latestGoodRun}

#Now get the last physics run
lastPhysicsRunQuery=`psql -h sphnxdaqdbreplica daq --csv -c "SELECT runnumber FROM run WHERE runtype='physics' ORDER BY runnumber DESC LIMIT 1"`
lastPhysicsRun=`echo ${lastPhysicsRunQuery} | awk '{print $2}'`
echo The latest physics run is ${lastPhysicsRun}

#Find what runs exist between the latest analysed run, and the last physics run
physicsRunRangeQuery=`psql -h sphnxdaqdbreplica daq --csv -c "SELECT runnumber FROM run WHERE runtype='physics' AND runnumber BETWEEN ${latestGoodRun} AND ${lastPhysicsRun} ORDER BY runnumber"`
runsToAnalyse=(${physicsRunRangeQuery})
runsToAnalyse=("${runsToAnalyse[@]:2}")

#Lets get the previous list of runs that havent finished tranferrring segment 0's yet
if [ -f ${partialRunList} ]; then
  readarray partialRunArray < ${partialRunList}
  runsToAnalyse+=(${partialRunArray[@]})
fi

sortedUniqueRunsToAnalyse=($(echo "${runsToAnalyse[@]}" | tr ' ' '\n' | sort -u | tr '\n' ' '))
echo New run list is ${sortedUniqueRunsToAnalyse[@]}

#We now have the new run list, we can delete the old lists
rm ${goodRunList}
if  [ -f ${partialRunList} ]; then rm ${partialRunList}; fi;

#Finally, search for runs and write them to the full or partial lists
declare -A pairs=(["gl1daq"]="/sphenix/lustre01/sphnxpro/physics/GL1/physics/GL1_physics_gl1daq-"
                  ["seb18"]="/sphenix/lustre01/sphnxpro/physics/mbd/physics/physics_seb18-"
                  ["mvtx_flx"]="/sphenix/lustre01/sphnxpro/physics/MVTX/physics/physics_mvtx"
                  ["intt_flx"]="/sphenix/lustre01/sphnxpro/physics/INTT/physics/physics_intt")

rawTrailer="-0000.evt"
mbdTrailer="-0000.prdf"

for runNumber in "${sortedUniqueRunsToAnalyse[@]}"; do
  fullRun="true"
  fullRunNumber=$(printf "%08d" ${runNumber})

  for i in "${!pairs[@]}"; do
    j=${pairs[$i]}
  
    if [ "$i" == "gl1daq" ] || [ "$i" == "seb18" ]; then
      file=${j}${fullRunNumber}${rawTrailer}
     
      if [ "$i" == "seb18" ]; then file=${j}${fullRunNumber}${mbdTrailer}; fi;
  
      if ! test -f ${file}; then
        fullRun="false"
      fi
    fi
  
    if [ "$i" == "mvtx_flx" ] || [ "$i" = "intt_flx" ]; then
      nLoops=5
      if  [ "$i" == "intt_flx" ]; then nLoops=7; fi;
  
      for k in $(seq 0 $nLoops); do
        file=${j}${k}-${fullRunNumber}${rawTrailer}
        if ! test -f ${file}; then
          fullRun="false"
        fi
      done
    fi
  
  done

if [ ${fullRun} == "true" ]; then
  echo Run ${runNumber} has all segment 0s
  echo ${runNumber} >> ${goodRunList}
else
  echo Run ${runNumber} is missing some segment 0s
  echo ${runNumber} >> ${partialRunList}
fi

done

#Submit the new run list if there are some to analyse
nNewGoodRuns=`cat ${goodRunList} | wc -l`
if [ ${nNewGoodRuns} != 0 ]; then
  condor_submit condor.job
fi
 
