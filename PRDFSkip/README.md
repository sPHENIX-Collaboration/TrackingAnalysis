## Quick start

There is a python script which will submit several jobs for you. Its usage is the following
```
submit_skipped_jobs.py [-h] [-n NEVENT] [-r RUNNUMBER] [-ngl NGL1DAQSEGMENTS] [-v]```
where `-n` is the number of events you want to process, `-r` is the run number you want to process, and `-ngl` is the number of GL1 segments that run number has in lustre from (e.g.) 

```
ls -lrt /sphenix/lustre01/sphnxpro/physics/GL1/physics/*runnumber*.evt
```

The submission script will run several condor jobs that skip 0, 1M, 2M, 3M...10M events in each GL1 segment and process `-n` events afterwards (i.e. the zeroth segment that will be produced contains events [0-nEvent], the first segment contains [1M-1M+nEvent], and so on and so forth)

This will produce the equivalent of event combined tracking DSTs, or `STREAMING` DSTs, that you can then run the track reconstruction over