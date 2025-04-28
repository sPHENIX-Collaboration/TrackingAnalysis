#!/bin/bash
export USER="$(id -u -n)"
export LOGNAME=${USER}
export HOME=/sphenix/u/${LOGNAME}
baseDir=$6
echo $baseDir
this_script=$BASH_SOURCE
this_script=`readlink -f $this_script`
this_dir=`dirname $this_script`
echo rsyncing from $this_dir
echo running: $this_script $*

source /opt/sphenix/core/bin/sphenix_setup.sh -n new
#source /opt/sphenix/core/bin/setup_local.sh /sphenix/user/jdosbo/install


cd $baseDir/..
pwd
echo "Runnumber "$1
echo "type "$2
echo "Detector "$3
echo "ROC "$4
echo "nevents "$5
runnumber=$(printf "%08d" $1)

if [ ! -f gl1daq_$runnumber.list ]; then
    sh gl1_makelist.sh $2 $1
fi
if [ ! -f mvtx0_$runnumber.list ]; then
    sh mvtx_makelist.sh $2 $1
fi
if [ ! -f intt0_$runnumberlist ]; then
    sh intt_makelist.sh $2 $1
fi
#sh tpot_makelist.sh $2 $1

if [[ $3 == "MVTX" ]]
then
    root Fun4All_SingleStream_Combiner.C\($5,$1,\"./\",\"./\",\"$2-$3$4\",1000000,\"2025p001\",\"gl1daq_${runnumber}.list\",\"\",\"\",\"mvtx$4_${runnumber}.list\"\)
elif [[ $3 == "INTT" ]]
then
    root Fun4All_SingleStream_Combiner.C\($5,$1,\"./\",\"./\",\"$2-$3$4\",1000000,\"2025p001\",\"gl1daq_${runnumber}.list\",\"\",\"intt$4_${runnumber}.list\"\)
elif [[ $3 == "TPOT" ]]
then
    root Fun4All_SingleStream_Combiner.C\(0,$1,\"./\",\"./\",\"$2-$3$4\",1000000,\"2025p001\",\"gl1daq_${runnumber}.list\",\"\",\"\",\"\",\"tpot_${runnumber}.list\"\)
else
    echo "Can't do other subsystems with production code"
fi

echo "Finished script"
rm intt*_$runnumber.list
rm mvtx*_$runnumber.list
rm gl1*_$runnumber.list

if [[ ! -d output ]]
then
    mkdir output
fi

cd output
if [[ ! -d $runnumber ]]
then
    mkdir $runnumber
fi
cd ..
mv HIST*$3*$runnumber*.root output/$runnumber

rm cosmics*.root

echo "Finished clean up"
