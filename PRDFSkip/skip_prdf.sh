#!/bin/sh


source /opt/sphenix/core/bin/sphenix_setup.sh -n new
source /opt/sphenix/core/bin/setup_local.sh /sphenix/user/jdosbo/tpc_skip_build

#arguments for script
echo "Starting at event " $1
echo "that includes nevents= "$2
echo "run number "$3
echo "original gl1daq file "$4

runnumber=$(printf "%08d" $3)
gl1daqname="gl1daq-${runnumber}-skip${1}.evt"

#make the gl1 skipped prdf
echo $gl1daqname
eventcombiner -c $1 -n $2 $gl1daqname /sphenix/lustre01/sphnxpro/physics/GL1/physics/$4

#make the tpc file list
sh tpc_makelist.sh physics $1
#sh make_tpc_skipped_prdflist.sh $3 $1

ls ${PWD}/$gl1daqname > gl1daq_skip$1.list

outfile="DST_STREAMING_run2pp_new_2024p002_skip${1}-${runnumber}-0000.root"

echo "Processing the job : "
echo "Nevents "$2
echo "run number "$3
echo "outfile "$outfile
echo "with gl1list "gl1daq_skip$1.list
#run the show
root -l -b -q Fun4All_Stream_Combiner.C\($2,$3,\"$outfile\",\"gl1daq_skip$1.list\"\)
