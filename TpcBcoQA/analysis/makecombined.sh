#!/usr/bin/bash
set -e 

if [ -z "$1" ]; then
  echo "no run number argument"
  exit 1
fi

if [ -f "../utilsout.config" ]; then
  source ../utilsout.config
else
  echo "Error: utilsout.config in main dir not found."
  exit 1
fi

outdir=${OUTPUT_DIR}
echo "reading file from $outdir"
for i in ebdc{00..23}
do
  if ls ${outdir}/output_${i}_RunNumber_${1}_segment*.txt 1> /dev/null 2>&1; then
    ls ${outdir}/output_${i}_RunNumber_${1}_segment*.txt | sort -V | xargs cat > ${outdir}/combined_${i}_RunNumber${1}.txt
    sleep 0.1
    rm -rf ${outdir}/output_${i}_RunNumber_${1}_segment*.txt
  else
    echo "No files found for ${i} with RunNumber ${1}."
    exit 1
  fi
done
sleep 0.1

if ls ${outdir}/output_gl1daq_RunNumber_${1}_segment*.txt 1> /dev/null 2>&1; then
  ls ${outdir}/output_gl1daq_RunNumber_${1}_segment*.txt | sort -V | xargs cat > ${outdir}/combined_gl1daq_RunNumber${1}.txt
  rm -f ${outdir}/output_gl1daq_RunNumber_${1}_segment*.txt
else
  echo "No files found for gl1daq with RunNumber ${1}."
  exit 1
fi
