#!/bin/sh

runnumber=$1
#n gl1 bcos to skip, i.e. whatever the gl1 bco is
nSkip=$2
echo "Skipping to bco "$nSkip
sh tpc_makelist.sh physics $1 $nSkip

#iterate through ebdcs
for i in {0..23}
do
    filename=`printf "tpc%02i-${nSkip}.list" $i`
    echo $filename
    MINDIFF=9999999999999999
    EBDCMINSEGMENT=9999999999999
    #read prdf filenames from the file list
    while read prdf; do
	
	seg=$(ls $prdf | grep -o "[0-9]\+.evt") 
	segment=${seg%.evt}
	segmentnozeros=$(echo $segment | sed 's/^0*//')
        
	# get the bco the prdf starts with
	bco=$(ddump -i $prdf | head -n 7 | grep -m 1 "(MDBIT) " | awk '{print $3}')
	if [ "$((segmentnozeros))" -gt 0 ]; then
	    bco=$(ddump -i $prdf | head -n 7 | grep -m 1 "(LVL1 ) " | awk '{print $4}')
	fi
	# get the actual bco
	eventnum=$(bc <<< "ibase=16;${bco^^}")
	# no valid bco is less than 5 digits at this point
	if [ ${#eventnum} -lt 5 ]; then
	    # if we are in the middle of an rcdaq event just
	    # assume the previous prdf is the best segment
	    continue
	fi
	if [ "$((eventnum))" -gt "$nSkip" ]; then
	    # already found the lowest bco
	    break
	fi
	# if it is less than the number of events skip, check if it has the 
	# minimum distance to the skipped starting event
        if [ "$((eventnum))" -lt "$nSkip" ]; then
	    diff="$((nSkip-eventnum))"
	    echo "tpc bco "$eventnum
	    echo "gl1 bco "$nSkip
	    echo "diff is "$diff
	    if [ "$diff" -lt "$MINDIFF" ]; then
		MINDIFF=$diff
		EBDCMINSEGMENT=${seg%.evt}
	    fi
	fi
    done < $filename
    if [ $EBDCMINSEGMENT -lt 1 ]; then
	echo "0th segment sufficient, not erasing anything"
	continue
    fi
    # then erase all segments from $filename less than EBDCMINSEGMENT
    echo "For segments in "$filename
    echo "Erasing segments prior to "$EBDCMINSEGMENT
    sed -i 1,$EBDCMINSEGMENT\d $filename
done
