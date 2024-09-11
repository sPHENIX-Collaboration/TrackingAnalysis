#!/bin/sh


source /opt/sphenix/core/bin/sphenix_setup.sh -n new
source /opt/sphenix/core/bin/setup_local.sh /sphenix/user/jdosbo/tpc_skip_build

#arguments for script
echo "Starting at event " $1
echo "that includes nevents= "$2
echo "run number "$3
echo "original gl1daq file "$4

gl1seg=$(ls /sphenix/lustre01/sphnxpro/physics/GL1/physics/$4 | grep -o "[0-9]\+.evt") 
gl1segment=${gl1seg%.evt}
echo "gl1segment is "$g11segment

runnumber=$(printf "%08d" $3)
gl1daqname="gl1daq-${runnumber}-skip${1}-${gl1segment}.evt"

#make the gl1 skipped prdf
echo $gl1daqname
eventcombiner -c $1 -n $2 $gl1daqname /sphenix/lustre01/sphnxpro/physics/GL1/physics/$4

# get the gl1 bco 64 bit
gl1bco=$(ddump -i $gl1daqname | grep "Beam Clock: " | awk '{print $3}')

# shift to 40 bit to match tracker bco
gl1bcotrb=$(($gl1bco & (1 << 40) -1))

gl1daqlist="gl1daq-${runnumber}-skip$1-${gl1segment}.list"
ls ${PWD}/$gl1daqname > $gl1daqlist

#make the tpc file list
sh make_tpc_skipped_prdflist.sh $3 $gl1bcotrb

outfile="DST_STREAMING_run2pp_new_2024p002_skip${1}-${runnumber}-${gl1segment}.root"

echo "Processing the job : "
echo "Nevents "$2
echo "run number "$runnumber
echo "outfile "$outfile
echo "with gl1list "$gl1daqlist
#run the show
root -l -b -q Fun4All_Stream_Combiner.C\($2,$3,\"$outfile\",\"$gl1daqlist\",\"tpc00-$runnumber-$gl1bcotrb.list\",\"tpc01-$runnumber-$gl1bcotrb.list\",\"tpc02-$runnumber-$gl1bcotrb.list\",\"tpc03-$runnumber-$gl1bcotrb.list\",\"tpc04-$runnumber-$gl1bcotrb.list\",\"tpc05-$runnumber-$gl1bcotrb.list\",\"tpc06-$runnumber-$gl1bcotrb.list\",\"tpc07-$runnumber-$gl1bcotrb.list\",\"tpc08-$runnumber-$gl1bcotrb.list\",\"tpc09-$runnumber-$gl1bcotrb.list\",\"tpc10-$runnumber-$gl1bcotrb.list\",\"tpc11-$runnumber-$gl1bcotrb.list\",\"tpc12-$runnumber-$gl1bcotrb.list\",\"tpc13-$runnumber-$gl1bcotrb.list\",\"tpc14-$runnumber-$gl1bcotrb.list\",\"tpc15-$runnumber-$gl1bcotrb.list\",\"tpc16-$runnumber-$gl1bcotrb.list\",\"tpc17-$runnumber-$gl1bcotrb.list\",\"tpc18-$runnumber-$gl1bcotrb.list\",\"tpc19-$runnumber-$gl1bcotrb.list\",\"tpc20-$runnumber-$gl1bcotrb.list\",\"tpc21-$runnumber-$gl1bcotrb.list\",\"tpc22-$runnumber-$gl1bcotrb.list\",\"tpc23-$runnumber-$gl1bcotrb.list\"\)
