#!/bin/bash

set -e
if [ -z "$1" ]; then 
  echo "Error: Run number argument is missing"
  exit 1
fi

runnumber=$1
runtype=$2

echo "Go to analysis and run auto submission for run $runnumber"
cd analysis || { echo "cd analysis didn't work... exiting.."; exit 1; }
bash runcondor.sh $runnumber $runtype

if [ $? -eq 0]; then
  echo "runcondor script executed successfully"
else
  echo "Error: smth went wrong"
  exit 1
fi
