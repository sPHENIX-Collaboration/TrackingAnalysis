#!/bin/bash

output_file="result2.txt"
> "$output_file"  # Clear the file if it exists

# Loop through directories matching the pattern 5xxxx
for dir in ./5*; do
    if [ -d "$dir" ]; then
        # Check if a.log exists in the directory
        if [ -f "$dir/a.log" ]; then
            echo "Processing $dir/a.log"
            # Append the content of a.log to result.txt
            tail -n 1 "$dir/a.log" >> "$output_file"
        else
            echo "Warning: $dir/a.log not found."
        fi
    fi
done


# Initialize arrays
run_number=()
total_events=()
luminosity=()
#lumi_density=()

# Counter to ensure we process only the first 142 lines
counter=0
max_lines=145

# Read and process each line from result.txt
while IFS= read -r line && [ $counter -lt $max_lines ]; do
    # Extract values from the line
    run=$(echo "$line" | awk '{print $2}')
    events=$(echo "$line" | awk '{print $4}')
    lumi=$(echo "$line" | awk '{print $8}')
 #   density=$(echo "$line" | awk '{print $9}')
    
    # Append values to arrays
    run_number+=("$run")
    total_events+=("$events")
    luminosity+=("$lumi")
  #  lumi_density+=("$density")
    
    # Increment counter
    ((counter++))
done < result2.txt

# Print arrays in the required format
echo "double run_number[nPoints] = {"
echo "${run_number[*]}" | sed 's/ /, /g'
echo "};"

echo "double total_events[nPoints] = {"
echo "${total_events[*]}" | sed 's/ /, /g'
echo "};"

echo "double luminosity[nPoints] = {"
echo "${luminosity[*]}" | sed 's/ /, /g'
echo "};"

#echo "double lumi_trigger_event[nPoints] = {"
#echo "  ${lumi_density[*]}" | sed 's/ /, /g'
#echo "};"

