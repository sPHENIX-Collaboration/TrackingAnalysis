#!/bin/bash
export USER="$(id -u -n)"
export LOGNAME=${USER}
export HOME=/sphenix/u/${USER}

hostname

this_script=$BASH_SOURCE
this_script=`readlink -f $this_script`
this_dir=`dirname $this_script`
echo rsyncing from $this_dir
echo running: $this_script $*

Run=$1
SERVER=$2

echo $Run
echo $SERVER

source /opt/sphenix/core/bin/sphenix_setup.sh -n new
source /opt/sphenix/core/bin/setup_local.sh /sphenix/user/jpark4/sPHENIX_software/online_distribution/newbasic/build/

printenv 

./anaroot $Run $SERVER

echo all done
echo "script done"
