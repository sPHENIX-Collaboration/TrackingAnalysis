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

raw_totalfilenum=$(grep 'const int nFiles' utils.h | cut -d'=' -f2 | tr -d ' ;')
totalfilenum=$((raw_totalfilenum-1))
outdir=${OUTPUT_DIR}
echo "reading file from $outdir"

for hostnum in $(seq -w 0 $totalfilenum); do
  for suffix in 0 1; do
    prefix="ebdc${hostnum}_${suffix}"
    if ls ${outdir}/output_${prefix}*RunNumber_${1}_segment*.txt 1> /dev/null 2>&1; then
      ls ${outdir}/output_${prefix}*RunNumber_${1}_segment*.txt | sort -V | xargs cat > ${outdir}/combined_${prefix}_RunNumber${1}.txt
      sleep 0.1
      rm -f ${outdir}/output_${prefix}*RunNumber_${1}_segment*.txt
    else
      echo "No files found for ${prefix} with RunNumber ${1}."
      exit 1
    fi
  done
done

sleep 0.1

if ls ${outdir}/output_gl1daq_RunNumber_${1}_segment*.txt 1> /dev/null 2>&1; then
  ls ${outdir}/output_gl1daq_RunNumber_${1}_segment*.txt | sort -V | xargs cat > ${outdir}/combined_gl1daq_RunNumber${1}.txt
  rm -f ${outdir}/output_gl1daq_RunNumber_${1}_segment*.txt
else
  echo "No files found for gl1daq with RunNumber ${1}."
  exit 1
fi
