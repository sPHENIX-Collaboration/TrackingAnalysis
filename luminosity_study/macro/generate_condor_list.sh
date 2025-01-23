#!/bin/bash

# List of runs
#runs=(51830 52111 52641 53370 53744)

#mapfile -t runs < run2.list
mapfile -t runs < run_test.list

# Output file for condor list
condor_list="condor.list"
> $condor_list # Clear or create the file

# Base directories
base_dir="$PWD"
output_base_dir="$base_dir/output"
list_base_dir="$base_dir/list"
# Loop over each run
for run in "${runs[@]}"; do
    echo "Processing run $run..."

    # Step 1: Create $run directory and copy gl1_makelist.sh
    mkdir -p "$list_base_dir/$run"
    cp "$base_dir/gl1_makelist.sh" "$list_base_dir/$run"
    cd "$list_base_dir/$run" || exit

    # Run gl1_makelist.sh
    bash gl1_makelist.sh physics "$run"

    # Return to base directory
    cd "$base_dir" || exit

    # Step 2: Create $run directory under output
    mkdir -p "$output_base_dir/$run"

    # Step 3: Append to condor.list
    echo "$base_dir/run_analysisbco.sh $list_base_dir/$run/gl1daq.list output_streaming.root $output_base_dir/$run $output_base_dir/$run/condor-gl10.out $output_base_dir/$run/condor-gl10.err /tmp/xuzhiwan/condor0.log $base_dir" >> $condor_list
done

echo "All runs processed. Condor list saved to $condor_list."

