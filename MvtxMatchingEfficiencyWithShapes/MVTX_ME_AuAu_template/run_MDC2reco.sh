#!/bin/bash

export HOME=/sphenix/u/${LOGNAME}
export MYINSTALL=/sphenix/u/jkvapil/sw/install
source /opt/sphenix/core/bin/sphenix_setup.sh -n new
source $OPT_SPHENIX/bin/setup_local.sh $MYINSTALL

nEvents=5000

echo running: Fun4All_FullJetFinder.sh $*
ls /sphenix/lustre01/sphnxpro/production/run2auau/physics/ana464_nocdbtag_v001/DST_STREAMING_EVENT_*/run_00054800_00054900/dst/*000$2-$1.root > list_$1.list
root.exe -q -b Fun4All_TrackSeeding_25326.C\($nEvents,\"list_$1.list\"\)
echo Script done
