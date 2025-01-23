#!/bin/bash

base_dir="../rootfiles"
total_lumi=0
lumi_per_entries=()
# Loop through each directory under the base directory
for file in "$base_dir"/*; do
    if [ -f "$file" ]; then
        run_number=$(basename "$file" | sed -E 's/^run([0-9]+)_output_gl1p_new\.root$/\1/')

        # Create a corresponding directory in the current plotting directory
        mkdir -p "$run_number"

        # Copy the AnalyzeLumiFile.C file into the newly created directory
        cp AnalyzeLumiFile.C "$run_number/"

        # Navigate to the new directory
        cd "$run_number" || exit

        # Modify the input run_number in the .C file
        #sed -i "s/int run_number = [0-9]\+/int run_number = $run_number/" AnalyzeLumiFile.C

        # Run the .C file using ROOT
	root -b -q "AnalyzeLumiFile.C($run_number)" > a.log
        output=$(grep "Run " a.log)

        lumi=$(echo "$output" | grep -oP 'Total Lumi raw \K[0-9.]+')
        entries=$(echo "$output" | awk '{print $7}')
	lumi_per_entries+=$(echo "scale=10; $lumi / $entries/1000000." | bc)
	echo "$output $(echo "scale=10; $lumi / ($entries/1000000.)"| bc)"

        if [ -n "$lumi" ]; then
            total_lumi=$(echo "$total_lumi + $lumi" | bc)
        fi
        cd ..

    else
        echo "Skipping $dir as it is not a directory."
    fi
done

echo "lumi_per_entries: ${lumi_per_entries[*]}" | sed 's/ /, /g'
echo "Total Lumi: $total_lumi"
