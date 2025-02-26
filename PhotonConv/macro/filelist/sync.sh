#!/bin/bash

runs=(53741 53742 53743 53744)

for ((k=0; k<${#runs[@]}; k++))
do
  # input list file
  input_trkr_seed_file="dst_trkr_seed_run2pp-000${runs[$k]}.list"
  input_trkr_cluster_file="dst_trkr_cluster_run2pp-000${runs[$k]}.list"
  input_calo_file="dst_calo_run2pp-000${runs[$k]}.list"

  # sync list file
  output_trkr_seed_cluster_calo_file="dst_sync_trkr_seed_cluster_calo_run2pp-000${runs[$k]}.list"

  # clear output file
  > "$output_trkr_seed_cluster_calo_file"

  # read trkr seed file and build synchronization map
  unset trkr_seed_map
  declare -A trkr_seed_map
  trkr_seed_prefix=0
  trkr_seed_run=0
  while IFS= read -r trkr_seed_line; do
    # read DST_TRKR_SEED_run2pp_ana468_2024p012_v002-00053741-00001.root
    # prefix: DST_TRKR_SEED_run2pp_ana468_2024p012_v002
    # run number: 00053741
    # segment number: 00001
    trkr_seed_prefix=$(echo "$trkr_seed_line" | cut -d'-' -f1)
    trkr_seed_run=$(echo "$trkr_seed_line" | cut -d'-' -f2)
    trkr_seed_segment=$(echo "$trkr_seed_line" | cut -d'-' -f3 | cut -d'.' -f1)

    # store in synchronization map, key is segment
    trkr_seed_map["$trkr_seed_segment"]="$trkr_seed_line"
  done < "$input_trkr_seed_file"

  # read calo file and build synchronization map
  unset calo_map
  declare -A calo_map
  calo_prefix=0
  calo_run=0
  while IFS= read -r calo_line; do
    # read DST_CALO_run2pp_ana462_2024p010_v001-00053741-00000.root
    # prefix: DST_CALO_run2pp_ana462_2024p010_v001
    # run number: 00053741
    # segment number: 00000
    calo_prefix=$(echo "$calo_line" | cut -d'-' -f1)
    calo_run=$(echo "$calo_line" | cut -d'-' -f2)
    calo_segment=$(echo "$calo_line" | cut -d'-' -f3 | cut -d'.' -f1)

    # store in synchronization map, key is segment
    calo_map["$calo_segment"]="$calo_line"
  done < "$input_calo_file"

  # read trkr cluster file and match with trkr seed file
  while IFS= read -r trkr_cluster_line; do
    # read DST_TRKR_CLUSTER_run2pp_ana466_2024p012_v001-00053741-00001.root
    # prefix: DST_TRKR_CLUSTER_run2pp_ana466_2024p012_v001
    # run number: 00053741
    # segment number: 00001
    trkr_cluster_prefix=$(echo "$trkr_cluster_line" | cut -d'-' -f1)
    trkr_cluster_run=$(echo "$trkr_cluster_line" | cut -d'-' -f2)
    trkr_cluster_segment=$(echo "$trkr_cluster_line" | cut -d'-' -f3 | cut -d'.' -f1)
    #echo trkr_cluster $trkr_cluster_segment

    # check if match with trkr seed file
    if [[ -n "${trkr_seed_map[$trkr_cluster_segment]}" ]]; then
      # if matched, then check calo file
      matched_calo_segment=$(printf "%05d" $((10#$trkr_cluster_segment / 10)))
      if [[ -n "${calo_map[$matched_calo_segment]}" ]]; then
        echo "$trkr_cluster_segment" "$matched_calo_segment" >> "$output_trkr_seed_cluster_calo_file"
      fi
    fi
  done < "$input_trkr_cluster_file"

  njobs=`cat $output_trkr_seed_cluster_calo_file | wc -l`
  echo "Run ${runs[$k]} match done! Writing to $output_trkr_seed_cluster_calo_file. ${njobs} Jobs in total"

done
