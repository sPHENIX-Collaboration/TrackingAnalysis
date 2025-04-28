#!/usr/bin/sh

echo "Silicon subsystem "$1
echo "nevents "$2
echo "runnumber "$3

if [[ ! -d $PWD/logfiles ]]
then
   mkdir $PWD/logfiles
fi

if [[ $1 == "MVTX" ]]
   then
   echo "Running for MVTX"
   elif [[ $1 == "INTT" ]]
then
    echo "Running for INTT"
else
    echo "Undefined subsystem, try MVTX or INTT"
    exit 0
fi

echo "Universe        = vanilla" > runProd_$1_$3.job

echo "Notification    = Never" >> runProd_$1_$3.job
echo "Priority        = 1" >> runProd_$1_$3.job

echo "PeriodicHold = (NumJobStarts>=1 && JobStatus == 1)" >> runProd_$1_$3.job
echo "request_memory = 2.0GB" >> runProd_$1_$3.job

echo "Initialdir      ="$PWD >> runProd_$1_$3.job
echo "Executable      = "$PWD"/runCombiner.sh" >> runProd_$1_$3.job

echo "runnumber = "$3 >> runProd_$1_$3.job
echo "Detector = "$1 >> runProd_$1_$3.job
if [[ $1 == "MVTX" ]]
then
       echo "NROCS = 6" >> runProd_$1_$3.job
elif [[ $1 == "INTT" ]]
then
    echo "NROCS=8" >> runProd_$1_$3.job
fi
echo "nevents = "$2 >> runProd_$1_$3.job
echo "type = cosmics" >> runProd_$1_$3.job
echo "ana = \$(type)_\$(runnumber)_\$(Detector)_\$(Process)" >> runProd_$1_$3.job

echo "Output          =  \$(InitialDir)/logfiles/\$(ana).out" >> runProd_$1_$3.job
echo "Error           =  \$(InitialDir)/logfiles/\$(ana).err" >> runProd_$1_$3.job
echo "Log             =  \$(InitialDir)/logfiles/\$(ana).log" >> runProd_$1_$3.job


echo "Arguments       = \$(runnumber) \$(type) \$(Detector) \$(Process) \$(nevents) \$(Initialdir)" >> runProd_$1_$3.job
echo "Queue \$(NROCS)" >> runProd_$1_$3.job

condor_submit runProd_$1_$3.job
