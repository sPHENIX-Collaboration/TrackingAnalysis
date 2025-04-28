#!/usr/bin/sh


echo "Detector "$1
echo "Run number "$2

cd $PWD/..

runnumber=$(printf "%08d" $2)
cd output/$runnumber/
hadd $1_$2.root HIST*$1*$2*.root
cd ../../

if [[ $1 == "MVTX" ]]
then
    root PlotRatios.C\(\"$1_$2.root\",$2,true\)
elif [[ $1 == "INTT" ]]
then
    root PlotRatios.C\(\"$1_$2.root\",$2,false\)
else
    echo "wrong detector type, try MVTX or INTT"
fi
