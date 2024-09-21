# sPHENIX_PmonTpcBcoCheck
Note this is a Pmonitor check not through Fun4All and only works in SDCC.

## setup
Change the email address in the ```email.config``` file to yours
Build your environment
```
make 
```

## run
Pickup a valid run number (the script checks if files exist but also make sure files are copied to sdcc)
The script will set the initial dir of condor to the current one. 
Run the script below and wait until you get notifications through your email.
```
bash runTpcCondor.sh <RunNumber>
```
