import os
import subprocess

starting_event_numbers = [0,1000000,2000000,3000000,4000000,5000000,6000000,7000000,8000000,9000000,10000000]
runnumber = 52593
nEvent = 1000
ngl1segments = 2

print(starting_event_numbers)
for gl1segment in range(ngl1segments):
    gl1daqname = "GL1_physics_gl1daq-{:0>8}-{:0>4}.root".format(runnumber, gl1segment)
    print("Gl1 name " + gl1daqname)
    for skip in starting_event_numbers:
        command = ["sh","submit_condor.sh"]
        command.append(str(runnumber))
        command.append(str(skip))
        command.append(str(nEvent))
        command.append(gl1daqname)
        #command = "sh submit_condor.sh " + str(runnumber) + " " + str(skip) + " " + str(nEvent) + " " + gl1daqname
        print(command)
        subprocess.run("pwd")
        subprocess.run(command)
