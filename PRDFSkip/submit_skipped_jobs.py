import os
import subprocess
import argparse


# event number to skip to within the GL1 prdf
starting_event_numbers = [0,1000000,2000000,3000000,4000000,5000000,6000000,7000000,8000000,9000000,10000000]


parser = argparse.ArgumentParser(description="Submit condor jobs to skip around prdfs")
parser.add_argument("-n","--nevent",type=int,help="number of events to process")
parser.add_argument("-r","--runnumber",type=int,help="run number to process")
parser.add_argument("-ngl","--ngl1daqsegments",type=int,help="number of gl1 segments for this run")
parser.add_argument("-v","--verbose",help="verbose",action="store_true")

args = parser.parse_args()
runnumber = args.runnumber
ngl1segments = args.ngl1daqsegments
nEvent = args.nevent
verbose = args.verbose

if verbose:
    print(starting_event_numbers)
for gl1segment in range(ngl1segments):
    gl1daqname = "GL1_physics_gl1daq-{:0>8}-{:0>4}.evt".format(runnumber, gl1segment)
    if verbose:
        print("Gl1 name " + gl1daqname)
    for skip in starting_event_numbers:
        command = ["sh","submit_condor.sh"]
        command.append(str(runnumber))
        command.append(str(skip))
        command.append(str(nEvent))
        command.append(gl1daqname)
        
        if verbose:
            print(command)
        if not verbose:
            subprocess.run(command)
