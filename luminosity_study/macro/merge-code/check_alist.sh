condor_q > a
grep run_analysisbco\.sh a | awk '{print $NF}' | grep -Eo '[0-9]+$' | while read -r dir; do
    if [ -d "$dir" ]; then
        if [ -f "$dir/condor-gl10.out" ]; then
#            echo "Directory: $dir"
#            tail -n 2 "$dir/condor-gl10.out" | head -n 1 | awk '{print $1}'
             num=$(tail -n 2 "$dir/condor-gl10.out" | head -n 1 | awk '{print $1}')
             echo "$dir $num"

        else
            echo "Warning: $dir/condor-gl10.out not found."
        fi
    else
        echo "Warning: Directory $dir does not exist."
    fi
done

