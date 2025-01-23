finished=()
unfinished=()
skipped=()
# Use find to locate the latest .root file in each directory
while IFS= read -r dir; do

    dir_name=$(basename "$dir")

    # Check if the directory name matches any entry in a.list
    if grep -qx "$dir_name" a; then
        echo "Skipping directory $dir_name as it matches an entry in a.list."
	skipped+=$dir_name
        continue
    fi
    # Get the latest .root file in the directory
    latest_file=$(find "$dir" -maxdepth 1 -name "*.root" -type f -printf "%T@ %p\n" 2>/dev/null | sort -nr | head -n 1 | cut -d' ' -f2-)
    
    if [ -n "$latest_file" ]; then
        # Check if the latest file ends with '000.root'
        if [[ "$latest_file" =~ 000\.root$ ]]; then
            unfinished+=("$(basename "$dir")")
        else
            finished+=("$(basename "$dir")")
        fi
    else
        unfinished+=("$(basename "$dir")")  # No .root files in the directory
    fi
done < <(find . -mindepth 1 -maxdepth 1 -type d)

# Output the results
echo "skipped: ${skipped[*]}"
echo "Finished (latest root file not ending in 000.root): ${finished[*]}"
echo "Unfinished (latest root file ends in 000.root or no root files): ${unfinished[*]}"

