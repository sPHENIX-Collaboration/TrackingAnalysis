# sPHENIX_PmonTpcBcoCheck

## setup
Type your email address in ```email.config``` as ```EMAIL="<your_email_address>"```
You can also turn off the email alert by chaning ```SEND_EMAIL=true``` to ```SEND_EMAIL=false```
Build your environment
```
make 
```

## run
Pickup a valid run number (the script checks if files exist but also make sure files are copied to sdcc)
The script will set the initial dir of condor to the current one. 
Run the script below and wait until you get notifications through your email (if you disabled it check later in the output directories).
```
bash runTpcCondor.sh <RunType> <RunNumber>
```

An example : ```bash runTpcCondor.sh cosmics 62466``` 
