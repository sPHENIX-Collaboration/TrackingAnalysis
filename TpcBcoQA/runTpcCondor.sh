#!/usr/bin/bash

if [ -z "$1" ] || [ -z "$2" ]; then
  echo "Usage: $0 <run_type> <run_number>"
  exit 1
fi

run_type=$1
run_number=$2

echo "make file for run ${run_number} with type ${run_type}..."
bash make_list.sh "$run_type" "$run_number"

if [ ! -f "tpc.list" ] || [ ! -f "gl1daq.list" ]; then
  echo "Error: tpc.list or gl1daq.list do not exist... Exit the script"
  exit 1
fi
echo "Done... files are crated for gl1daq.list, tpc.list"


> queue.list
while IFS= read -r line; do
  echo "$run_type $line" >> queue.list
done < tpc.list
echo "Done... tpc condor queue.list created for ${run_type}"

echo "....... now running GL1 ......"
gl1filelist=`cat gl1daq.list` 
for file in $gl1filelist
do
  root -l -q -b "AnaTpcBco.C(\"${file}\",\"${run_type}\",0)"
done

echo "....... now submitting condor for TPC ......"
INITIALDIR=$(pwd)
LOGDIR="$INITIALDIR/log"
mkdir -p "$LOGDIR"
MONITORLOGDIR="$INITIALDIR/monitorlog"
mkdir -p "$MONITORLOGDIR"

TEMP_FILE=$(mktemp)
cat <<EOL > "$TEMP_FILE"
Executable  = condor_script.sh
Universe    = vanilla
Input       = /dev/null
Arguments   = \$(filename1) \$(filename2)
Output      = $LOGDIR/\$(Cluster).\$(Process).out
Error       = $LOGDIR/\$(Cluster).\$(Process).err
Log         = $LOGDIR/\$(Cluster).\$(Process).log
Initialdir  = $INITIALDIR
PeriodicHold  = (NumJobStarts>=1 && JobStatus == 1)
request_memory = 8192MB
Queue filename1 filename2 from queue.list
EOL

if [[ -f condor.job ]]; then
  if cmp -s "$TEMP_FILE" condor.job; then
    echo "condor script already up-to-date. Move on."
  else
    echo "new condor script updated"
    mv "$TEMP_FILE" condor.job
  fi
else
  echo "condor.job not exist. Creating new one"
  mv "$TEMP_FILE" condor.job
fi

rm -f "$TEMP_FILE"

echo "------ Condor setup configuration : ------"
cat condor.job
echo "------------------------------------------"


condor_submit condor.job > ${MONITORLOGDIR}/job_submission_${run_number}.log
CLUSTER_ID=$(grep -oP "submitted to cluster \K\d+" ${MONITORLOGDIR}/job_submission_${run_number}.log)
NUM_JOBS=$(grep -oP "\d+(?= job\(s\) submitted)" ${MONITORLOGDIR}/job_submission_${run_number}.log)

for i in $(seq 0 $(($NUM_JOBS-1))); do
  echo "$CLUSTER_ID.$i" >> ${MONITORLOGDIR}/job_ids_${run_number}.txt
done

echo "------ condor job for run ${run_number} submitted as : ------"
cat ${MONITORLOGDIR}/job_submission_${run_number}.log
echo "----------------------------------------------------"

echo "....... now checking for condor log every 30 seconds ......"
echo "....... Please wait for notifications ....."

nohup bash monitor_condor_jobs.sh ${MONITORLOGDIR}/job_ids_${run_number}.txt $LOGDIR > ${MONITORLOGDIR}/monitor_${run_number}.log 2>&1 &
