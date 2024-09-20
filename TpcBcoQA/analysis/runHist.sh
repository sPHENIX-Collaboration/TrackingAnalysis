#!/bin/bash

if [ -z "$1" ]; then
  echo "no run number argument"
  exit 1
fi
histdir="histfiles"
plotdir="plots"
mkdir -p $histdir
mkdir -p $plotdir
root -l -q -b "HistAna.C($1,100,-1)"
root -l -q -b "drawHist.C($1,100,-1)"
