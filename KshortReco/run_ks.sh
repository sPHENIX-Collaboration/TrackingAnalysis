#!/bin/bash

source /opt/sphenix/core/bin/sphenix_setup.sh -n new  # setup sPHENIX environment in the singularity container shell. Note the shell is bash by default

# Additional commands for my local environment
export SPHENIX=/sphenix/u/xyu3
#export MYINSTALL=$SPHENIX/install
export MYINSTALL=/sphenix/u/xyu3/workarea/PhotonConv/KsNew/background_shape/KshortReconstruction/debug/install

# Setup MYINSTALL to local directory and run sPHENIX setup local script
# to adjust PATH, LD LIBRARY PATH, ROOT INCLUDE PATH, etc
source /opt/sphenix/core/bin/setup_local.sh $MYINSTALL

echo "sPHENIX environment setup finished"

this_script=$BASH_SOURCE
this_script=`readlink -f $this_script`
this_dir=`dirname $this_script`
echo running: $this_script $*

nEvents=$1
InDst=$2
InPath=$3
OutPrefix=$4
RunNumber=$5
Segment=$6
Index=$7
StepSize=$8

root.exe -q -b Fun4All_FullReconstruction_Ks.C\($nEvents,\"${InDst}\",\"${InPath}\",\"${OutPrefix}\",$RunNumber,$Segment,$Index,$StepSize\)
echo Script done
