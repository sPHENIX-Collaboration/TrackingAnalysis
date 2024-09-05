#!/bin/sh
source /opt/sphenix/core/bin/sphenix_setup.sh -n
#print arguments for script
echo "Submitting condor job for :"
echo "run number "$1
# number of events to skip in the prdf
echo "nSkip "$2
# number of events to process after skipping nSkip events
echo "nEvent "$3
# gl1daq filename from lustre which contains the events you want to skip to
echo "Original gl1daq filename "$4
echo ${PWD}/logfiles
if [ ! -d ${PWD}/logfiles ]
then
    mkdir ${PWD}/logfiles
fi

# make condor file
echo "Universe = vanilla" > runCombiner.job
echo "Notification = never" >> runCombiner.job
echo "Initialdir = "${PWD} >> runCombiner.job
echo "Executable = \$(Initialdir)/skip_prdf.sh" >> runCombiner.job
echo "request_memory=20.0GB" >> runCombiner.job
echo "PeriodicHold = (NumJobStarts>=1 && JobStatus == 1)" >> runCombiner.job
echo "concurrency_limits=CONCURRENCY_LIMIT_DEFAULT:100" >> runCombiner.job
echo "runnumber = "$1 >> runCombiner.job
echo "nSkip = "$2 >> runCombiner.job
echo "nEvent = "$3 >> runCombiner.job
echo "gl1daqfilename = "$4 >> runCombiner.job
echo "Output = \$(Initialdir)/logfiles/log_\$(runnumber)_\$(nSkip)_\$(gl1daqfilename).out" >> runCombiner.job
echo "Error = \$(Initialdir)/logfiles/log_\$(runnumber)_\$(nSkip)_\$(gl1daqfilename).err" >> runCombiner.job
echo "Log = \$(Initialdir)/logfiles/log_\$(runnumber)_\$(nSkip)_\$(gl1daqfilename).log" >> runCombiner.job
echo "Arguments = \$(nSkip) \$(nEvent) \$(runnumber) \$(gl1daqfilename)" >> runCombiner.job
echo "Queue 1" >> runCombiner.job

chmod +x skip_prdf.sh
condor_submit runCombiner.job
