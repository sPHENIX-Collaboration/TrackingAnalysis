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

SEG1=$1
SEG2=$2

echo $SEG1 $SEG2

source /opt/sphenix/core/bin/sphenix_setup.sh -n new

printenv 

root -l -q -b "AnaTpcBco.C(\"${SEG2}\",\"${SEG1}\",1)"

echo all done
echo "script done"
