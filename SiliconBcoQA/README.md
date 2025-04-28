# Silicon BCO QA module

###Instructions


To run this, you just need to run two scripts

1. Script to submit condor jobs and process data
2. Once all jobs are finished, run the plotting script


To do 1. you just need to 
```
cd scripts
sh runBcoChecks.sh <detector> <nevents> <runnumber>
```

As an example, you could do

```
sh runBcoChecks.sh INTT 0 63277
```

to run all events in run 63277. Once your jobs are finished, then you can run

```
sh makePlot.sh INTT 63277
```

For this module, you can swap out `INTT` with `MVTX` without any loss in functionality. This will produce some diagnostic plots of the GTM-GL1 BCO matching and (for the MVTX) strobe-GL1 BCO matching