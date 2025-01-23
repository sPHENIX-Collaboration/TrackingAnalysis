#!/usr/bin/bash

export USER="$(id -u -n)"
export LOGNAME=${USER}
# check if we run an sphenix account or legacy phenix
if [[ -d /sphenix/u/${USER} ]]
then
  export HOME=/sphenix/u/${USER}
else
  export HOME=/phenix/u/${USER}
fi

hostname

this_script=$BASH_SOURCE
this_script=`readlink -f $this_script`
this_dir=`dirname $this_script`
echo rsyncing from $this_dir
echo running: $this_script $*

anabuild=new
 
source /cvmfs/sphenix.sdcc.bnl.gov/gcc-12.1.0/opt/sphenix/core/bin/sphenix_setup.sh -n $anabuild
#source /cvmfs/sphenix.sdcc.bnl.gov/gcc-12.1.0/opt/sphenix/core/bin/setup_local.sh $MYINSTALL
export SPHENIX=/sphenix/u/xuzhiwan/sPHENIX
export MYINSTALL=$SPHENIX/install
export LD_LIBRARY_PATH=$MYINSTALL/lib:$LD_LIBRARY_PATH
export ROOT_INCLUDE_PATH=$MYINSTALL/include:$ROOT_INCLUDE_PATH
source /cvmfs/sphenix.sdcc.bnl.gov/gcc-12.1.0/opt/sphenix/core/bin/setup_local.sh $MYINSTALL

echo local lib: $LD_LIBRARY_PATH
echo root: $ROOT_INCLUDE_PATH
echo opt_sphenix: $OPT_SPHENIX

#if [[ ! -z "$_CONDOR_SCRATCH_DIR" && -d $_CONDOR_SCRATCH_DIR ]]
#then
#    cd $_CONDOR_SCRATCH_DIR
#    rsync -av $this_dir/* .
#else
#    echo condor scratch NOT set
#    exit -1
#fi

if [ -z "$CONDOR_SCRATCH_DIR" ]; then
    echo "CONDOR_SCRATCH_DIR not set. Setting to a local temporary directory."
    export CONDOR_SCRATCH_DIR=/tmp/$USER/condor_scratch
    mkdir -p $CONDOR_SCRATCH_DIR
fi


echo 'here comes your environment'
printenv
echo arg1 \(input file\) : $1
echo arg2 \(output file\): $2
echo arg3 \(output dir\): $3
runnum=52050
event=10
#26935862
echo running root.exe -q -b Fun4All_SingleStream_Combiner.C\(10,\"$1\"\)

#root.exe -q -b Fun4All_SingleStream_Combiner.C\($event,$runnum,\"$3/$2\", \"gl1daq.list\",\"$1\"\)
root.exe -q -b 'Fun4All_SingleStream_Combiner.C('"$event"', '"$runnum"', "'"$3/$2"'", "streaming", "'"$1"'")'

echo "script done"
