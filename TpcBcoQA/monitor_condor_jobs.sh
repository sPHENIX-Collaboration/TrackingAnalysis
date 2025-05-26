#!/bin/bash

source email.config
JOB_IDS_FILE=$1
LOG_DIR=$2
RUN_TYPE=$3

job_file_basename=$(basename "$JOB_IDS_FILE")
run_number="${job_file_basename#job_ids_}"
run_number="${run_number%.txt}"

nJobsDonePrev=0
lastCompletedTime=$(date +%s)

check_jobs_logs() {
  local job_ids=$(cat "$JOB_IDS_FILE")
  local all_done=true
  local error_found=false
  local nTotalJobs=$(wc -l < "$JOB_IDS_FILE")
  local nJobsDone=0

  for job_id in $job_ids; do
    log_file="$LOG_DIR/$job_id.log"
    err_file="$LOG_DIR/$job_id.err"

    if grep -q "Job terminated" "$log_file"; then
      if [[ -s "$err_file" ]]; then
        echo "Error found in $err_file for job $job_id"
        error_found=true
        break
      else
        ((nJobsDone++))
      fi
      continue
    fi

    if grep -qE "Job held|Job failed|Error" "$log_file"; then
      error_found=true
      break
    fi
    all_done=false
  done
  
  echo "$nJobsDone done out of ${nTotalJobs}..."

  if [ "$nJobsDone" -gt "$nJobsDonePrev" ]; then
    # If more jobs are done since last check, update the last completed time
    lastCompletedTime=$(date +%s)
    nJobsDonePrev=$nJobsDone
  fi


  if [ "$error_found" == "true" ]; then
    echo "Error found in one of the jobs. Exiting."
    return 2
  elif [ "$all_done" == "true" ]; then
    return 0
  else
    return 1 
  fi

}

while true; do
  check_jobs_logs
  status=$?

  currentTime=$(date +%s)
  timeSinceLastComplete=$(( (currentTime - lastCompletedTime) / 60 ))

  if [[ $status -eq 0 ]]; then
    echo "All jobs are done!"
    if [ "$SEND_EMAIL" == "true" ]; then
      echo -e "[BcoDump] All jobs completed for run ${run_number}.\nMonitor log file saved in ${JOB_IDS_FILE}.\nWill do automatic condor submission for analysis.\nPlease wait for the next email.." | mail -s "[BcoDump] Condor Jobs done for run ${run_number}" $EMAIL 
    fi
    nohup bash auto_analysis_condor.sh $run_number ${RUN_TYPE} > monitorlog/auto_analysis_monitor_${run_number}.log 2>&1 &
    break
  elif [[ $status -eq 2 ]]; then
    if [ "$SEND_EMAIL" == "true" ]; then
      echo -e "[BcoDump] One or more jobs failed for run ${run_number}.\nCheck ${JOB_IDS_FILE} and logs in ${LOG_DIR} for details.." | mail -s "[BcoDump] Problems found for condor jobs in run ${run_number}" $EMAIL
    fi
    break
  else
    if [[ $timeSinceLastComplete -ge 60 ]]; then
      if [ "$SEND_EMAIL" == "true" ]; then 
        echo -e "[BcoDump] No new completed jobs for run ${run_number} over 1 hour.\nCheck condor details and logs in ${JOB_IDS_FILE} and ${LOG_DIR}." | mail -s "[BcoDump] No update jobs for run ${run_number}" $EMAIL
      fi
      lastCompletedTime=$(date +%s)
    fi
    echo "Still running! wait another 30 seconds..." 
  fi  
  sleep 30
done

