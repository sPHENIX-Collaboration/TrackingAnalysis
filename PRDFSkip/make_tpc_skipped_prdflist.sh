#!/bin/sh

runnumber=$1
nSkip=$2
sh tpc_makelist.sh physics $1

#iterate through ebdcs
for i in {0..23}
do
    filename=`printf "tpc%02i.list" $i`
    echo $filename
    MINDIFF=9999999999999999
    EBDCMINSEGMENT=9999999999999
    #read prdf filenames from the file list
    while read prdf; do
	# get the event number the prdf starts with
	eventnum=$(dlist -i $prdf | grep "Event " | awk '{print $3}')
	
	if [ "$((eventnum))" -gt "$nSkip" ]; then
	    # already found the lowest bco
	    break
	fi
	# if it is less than the number of events skip, check if it has the 
	# minimum distance to the skipped starting event
        if [ "$((eventnum))" -lt "$nSkip" ]; then
	    diff="$((nSkip-eventnum))"
	    if [ "$diff" -lt "$MINDIFF" ]; then
		MINDIFF=$diff
	        segment=$(ls $prdf | grep -o "[0-9]\+.evt") 
		EBDCMINSEGMENT=${segment%.evt}
	    fi
	fi
    done < $filename
    # then erase all segments from $filename less than EBDCMINSEGMENT
    sed -i 1,$EBDCMINSEGMENT\d $filename
done
