#!/usr/bin/bash

if [ -z "$1" ]; then
  echo "no run number argument"
  exit 1
fi

bash makecombined.sh $1
output_file="queuerun.list"
> "$output_file"
for i in {00..23}
do
  echo "$1 $i" >> "$output_file"
done

echo "-------- queuerun.list : -----------"
cat $output_file
echo "------------------------------------"

echo "....... now setting for condor jobs ......"
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
Arguments   = \$(filename1)
Arguments   = \$(run) \$(server)
Output      = $LOGDIR/\$(Cluster).\$(Process).out
Error       = $LOGDIR/\$(Cluster).\$(Process).err
Log         = $LOGDIR/\$(Cluster).\$(Process).log
Initialdir  = $INITIALDIR
PeriodicHold  = (NumJobStarts>=1 && JobStatus == 1)
request_memory = 4096MB
Queue run server from queuerun.list
EOL

if [[ -f condor_ana.job ]]; then
  if cmp -s "$TEMP_FILE" condor_ana.job; then
    echo "condor script already up-to-date. Move on."
  else
    echo "new condor script updated"
    mv "$TEMP_FILE" condor_ana.job
  fi
else
  echo "condor.job not exist. Creating new one"
  mv "$TEMP_FILE" condor_ana.job
fi

rm -f "$TEMP_FILE"

echo "------ Condor setup configuration : ------"
cat condor_ana.job
echo "------------------------------------------"


condor_submit condor_ana.job > ${MONITORLOGDIR}/job_submission_ana_${1}.log
CLUSTER_ID=$(grep -oP "submitted to cluster \K\d+" ${MONITORLOGDIR}/job_submission_ana_${1}.log)
NUM_JOBS=$(grep -oP "\d+(?= job\(s\) submitted)" ${MONITORLOGDIR}/job_submission_ana_${1}.log)

for i in $(seq 0 $(($NUM_JOBS-1))); do
  echo "$CLUSTER_ID.$i" >> ${MONITORLOGDIR}/job_ids_${1}.txt
done

echo "------ condor job for run ${1} submitted as : ------"
cat ${MONITORLOGDIR}/job_submission_ana_${1}.log
echo "----------------------------------------------------"

echo "....... now checking for condor log every 30 seconds ......"
echo "....... Please wait for notifications ....."

nohup bash ana_monitor_condor_jobs.sh ${MONITORLOGDIR}/job_ids_${1}.txt $LOGDIR > ${MONITORLOGDIR}/monitor_${1}.log 2>&1 &
