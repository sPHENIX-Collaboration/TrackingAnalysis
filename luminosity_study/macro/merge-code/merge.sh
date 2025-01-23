un_finished_run=(
) #add bad runs here

for run in ./5*; 
do
 if [ -d "$run" ]; then 
   run_name=$(basename "$run")
   if [[ " ${un_finished_run[@]} " =~ " $run_name " ]]; then
            echo "Skipping unfinished run $run_name."
            continue
   fi

   cp mergeFiles.C "$run/"
   cd "$run" || exit
   root -b -q mergeFiles.C
   run_name=$(basename "$run")
   mv merged_output.root "../../rootfiles/run2/run${run_name}_output_gl1p_new.root"
   echo "Merge and move run $run to ../../rootfiles/run2/"
   cd ..
   rm -rf $run &
 fi
done
