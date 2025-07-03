#!/bin/bash

currentDir=$(pwd)
outDir=/sphenix/tg/tg01/hf/cdean/Silicon_MBD_Comparisons

nEvents=3000

source /opt/sphenix/core/bin/sphenix_setup.sh -n new

export MYINSTALL=$currentDir/module/install
export LD_LIBRARY_PATH=$MYINSTALL/lib:$LD_LIBRARY_PATH
export ROOT_INCLUDE_PATH=$MYINSTALL/include:$ROOT_INCLUDE_PATH

source /opt/sphenix/core/bin/setup_local.sh $MYINSTALL

runNumber=$1
fullRunNumber=$(printf "%08d" ${runNumber})
dataTopDir=$outDir/VertexCompare_run_${runNumber}

declare -a folders=("files" "lists" "plots")

for folder in "${folders[@]}"; do
  mkdir -p ${dataTopDir}/${folder}
done

declare -A pairs=(["gl1daq"]="/sphenix/lustre01/sphnxpro/physics/GL1/physics/GL1_physics_gl1daq-"
                  ["seb18"]="/sphenix/lustre01/sphnxpro/physics/mbd/physics/physics_seb18-"
                  ["mvtx_flx"]="/sphenix/lustre01/sphnxpro/physics/MVTX/physics/physics_mvtx"
                  ["intt_flx"]="/sphenix/lustre01/sphnxpro/physics/INTT/physics/physics_intt")

rawTrailer="-0000.evt"
mbdTrailer="-0000.prdf"
listTrailer=".list"

for i in "${!pairs[@]}"; do
  j=${pairs[$i]}

  if [ "$i" == "gl1daq" ] || [ "$i" == "seb18" ]; then
    file=${j}${fullRunNumber}${rawTrailer}
   
    if [ "$i" == "seb18" ]; then file=${j}${fullRunNumber}${mbdTrailer}; fi;

    if [ -f ${file} ]; then
      ls -1 ${file} > $dataTopDir/${folders[1]}/${i}${listTrailer}
    else
      echo "Missing file"
      exit 1
    fi
  fi

  if [ "$i" == "mvtx_flx" ] || [ "$i" = "intt_flx" ]; then
    nLoops=5
    if  [ "$i" == "intt_flx" ]; then nLoops=7; fi;

    for k in $(seq 0 $nLoops); do
      file=${j}${k}-${fullRunNumber}${rawTrailer}
      if [ -f ${file} ]; then
        ls -1 ${file} > $dataTopDir/${folders[1]}/${i}${k}${listTrailer}
      else
        echo "Missing file"
        exit 1
      fi
    done
  fi

done

root -l -q -b Fun4All_VertexCompare.C\(\"${runNumber}\",\"${dataTopDir}\",${nEvents},true,false,false\)
root -l -q -b Fun4All_VertexCompare.C\(\"${runNumber}\",\"${dataTopDir}\",${nEvents},false,true,false\)
root -l -q -b Fun4All_VertexCompare.C\(\"${runNumber}\",\"${dataTopDir}\",${nEvents},false,false,true\)

if [ -f ${dataTopDir}/${folders[0]}/outputVTX.root ]; then

  webDir=/sphenix/WWW/subsystem/offline/Silicon_MBD_Vertex_Comparison
  plotDir=${webDir}/Run${runNumber}
  
  mkdir -p ${plotDir}
  
  root -l -q -b vertexCompare.C\(\"${runNumber}\",\"${dataTopDir}\"\)
  
  cp ${webDir}/index.php ${plotDir}
  cp -r ${webDir}/res ${plotDir}
  cp $dataTopDir/${folders[2]}/*.pdf ${plotDir}
  cp $dataTopDir/${folders[2]}/*.png ${plotDir}
  
  rm -rf $dataTopDir
else
  exit 1
fi
