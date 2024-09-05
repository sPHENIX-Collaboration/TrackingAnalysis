#!/bin/sh

if [ $# -eq 0 ]
then
  echo "No type and runnumber supplied"
  exit 0
fi

if [ $# -eq 1 ]
then
  echo "No type or runnumber supplied"
  exit 0
fi

sh gl1_makelist.sh $1 $2
sh tpc_makelist.sh $1 $2
